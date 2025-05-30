#include "http_optimization.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <regex>
#include <numeric>
#include <zlib.h>

namespace gyatt {

HttpOptimization::HttpOptimization() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Initialize connection pool
    for (size_t i = 0; i < config_.maxConnections; ++i) {
        auto handle = std::make_unique<CurlHandle>();
        handle->curl = curl_easy_init();
        if (handle->curl) {
            initializeCurlHandle(handle->curl);
            connectionPool_.push_back(std::move(handle));
        }
    }
}

HttpOptimization::~HttpOptimization() {
    connectionPool_.clear();
    curl_global_cleanup();
}

void HttpOptimization::setConfig(const ConnectionPoolConfig& config) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    config_ = config;
    
    // Resize connection pool if needed
    if (connectionPool_.size() < config_.maxConnections) {
        for (size_t i = connectionPool_.size(); i < config_.maxConnections; ++i) {
            auto handle = std::make_unique<CurlHandle>();
            handle->curl = curl_easy_init();
            if (handle->curl) {
                initializeCurlHandle(handle->curl);
                connectionPool_.push_back(std::move(handle));
            }
        }
    }
}

void HttpOptimization::enableCompression(bool enable) {
    compressionEnabled_ = enable;
}

void HttpOptimization::setCacheExpiry(std::chrono::seconds expiry) {
    cacheExpiry_ = expiry;
}

void HttpOptimization::setRateLimit(std::chrono::milliseconds minInterval) {
    minRequestInterval_ = minInterval;
}

HttpOptimization::OptimizedHttpResponse HttpOptimization::httpGet(
    const std::string& url, const std::vector<std::string>& headers) {
    return performRequest("GET", url, "", headers);
}

HttpOptimization::OptimizedHttpResponse HttpOptimization::httpPost(
    const std::string& url, const std::string& data, const std::vector<std::string>& headers) {
    return performRequest("POST", url, data, headers);
}

HttpOptimization::OptimizedHttpResponse HttpOptimization::httpPatch(
    const std::string& url, const std::string& data, const std::vector<std::string>& headers) {
    return performRequest("PATCH", url, data, headers);
}

HttpOptimization::OptimizedHttpResponse HttpOptimization::httpPut(
    const std::string& url, const std::string& data, const std::vector<std::string>& headers) {
    return performRequest("PUT", url, data, headers);
}

std::vector<HttpOptimization::OptimizedHttpResponse> HttpOptimization::executeRequestBatch(
    const std::vector<BatchRequest>& requests) {
    
    std::vector<OptimizedHttpResponse> responses(requests.size());
    
    if (requests.empty()) {
        return responses;
    }
    
    // Sort requests by priority (higher priority first)
    std::vector<size_t> indices(requests.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&requests](size_t a, size_t b) {
        return requests[a].priority > requests[b].priority;
    });
    
    // Adaptive parallelism based on connection pool size and request count
    const size_t optimalParallel = std::min({
        config_.maxConnections,
        requests.size(),
        static_cast<size_t>(std::thread::hardware_concurrency() * 2)
    });
    
    std::vector<std::pair<std::future<OptimizedHttpResponse>, size_t>> futures;
    std::atomic<size_t> completedRequests{0};
    
    for (size_t i = 0; i < requests.size(); ++i) {
        // Dynamic throttling: wait if we have too many concurrent requests
        while (futures.size() >= optimalParallel) {
            bool foundCompleted = false;
            
            // Check for completed futures with timeout
            for (auto it = futures.begin(); it != futures.end(); ++it) {
                if (it->first.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
                    try {
                        responses[it->second] = it->first.get();
                        ++completedRequests;
                    } catch (const std::exception& e) {
                        // Handle future exceptions
                        OptimizedHttpResponse errorResponse;
                        errorResponse.success = false;
                        errorResponse.responseCode = 0;
                        errorResponse.error = "Future exception: " + std::string(e.what());
                        responses[it->second] = errorResponse;
                    }
                    futures.erase(it);
                    foundCompleted = true;
                    break;
                }
            }
            
            if (!foundCompleted) {
                // Force wait on the oldest future to prevent deadlock
                if (!futures.empty()) {
                    try {
                        responses[futures.front().second] = futures.front().first.get();
                        ++completedRequests;
                    } catch (const std::exception& e) {
                        OptimizedHttpResponse errorResponse;
                        errorResponse.success = false;
                        errorResponse.responseCode = 0;
                        errorResponse.error = "Future exception: " + std::string(e.what());
                        responses[futures.front().second] = errorResponse;
                    }
                    futures.erase(futures.begin());
                }
            }
        }
        
        size_t reqIdx = indices[i];
        const auto& req = requests[reqIdx];
        
        // Launch async request with proper error handling
        auto future = std::async(std::launch::async, [this, req]() -> OptimizedHttpResponse {
            try {
                return performRequest(req.method, req.url, req.data, req.headers);
            } catch (const std::exception& e) {
                OptimizedHttpResponse errorResponse;
                errorResponse.success = false;
                errorResponse.responseCode = 0;
                errorResponse.error = "Request exception: " + std::string(e.what());
                return errorResponse;
            } catch (...) {
                OptimizedHttpResponse errorResponse;
                errorResponse.success = false;
                errorResponse.responseCode = 0;
                errorResponse.error = "Unknown request exception";
                return errorResponse;
            }
        });
        
        futures.push_back(std::make_pair(std::move(future), reqIdx));
        
        // Small delay to prevent overwhelming the connection pool
        if (futures.size() > optimalParallel / 2) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    // Wait for all remaining requests and collect their results
    for (auto& future_pair : futures) {
        try {
            responses[future_pair.second] = future_pair.first.get();
            ++completedRequests;
        } catch (const std::exception& e) {
            OptimizedHttpResponse errorResponse;
            errorResponse.success = false;
            errorResponse.responseCode = 0;
            errorResponse.error = "Final future exception: " + std::string(e.what());
            responses[future_pair.second] = errorResponse;
        }
    }
    
    return responses;
}

std::map<std::string, std::string> HttpOptimization::createBlobsBatch(
    const std::string& repoName,
    const std::vector<std::pair<std::string, std::string>>& files,
    const std::string& token,
    std::function<void(size_t completed, size_t total, const std::string& currentFile)> progressCallback) {
    
    std::map<std::string, std::string> results;
    
    // Prepare batch requests using the corrected batch system
    std::vector<BatchRequest> batchRequests;
    for (const auto& [filePath, fileContent] : files) {
        BatchRequest req;
        req.url = "https://api.github.com/repos/" + repoName + "/git/blobs";
        req.method = "POST";
        
        // Prepare blob data with base64 encoding
        std::string encodedContent = Utils::base64Encode(fileContent);
        req.data = "{\"content\":\"" + encodedContent + "\",\"encoding\":\"base64\"}";
        
        req.headers = {
            "Authorization: token " + token,
            "Accept: application/vnd.github.v3+json",
            "Content-Type: application/json"
        };
        
        // Prioritize larger files (they benefit more from parallel processing)
        req.priority = fileContent.size();
        
        batchRequests.push_back(req);
    }
    
    // Execute batch requests using the corrected batch system
    std::vector<OptimizedHttpResponse> responses = executeRequestBatch(batchRequests);
    
    // Process responses and extract blob SHAs
    for (size_t i = 0; i < files.size() && i < responses.size(); ++i) {
        const auto& [filePath, fileContent] = files[i];
        const auto& response = responses[i];
        
        if (response.success && response.responseCode >= 200 && response.responseCode < 300) {
            // Parse SHA from response
            size_t shaPos = response.content.find("\"sha\":");
            if (shaPos != std::string::npos) {
                shaPos = response.content.find("\"", shaPos + 6);
                if (shaPos != std::string::npos) {
                    size_t shaEnd = response.content.find("\"", shaPos + 1);
                    if (shaEnd != std::string::npos) {
                        std::string blobSha = response.content.substr(shaPos + 1, shaEnd - shaPos - 1);
                        results[filePath] = blobSha;
                    }
                }
            }
            
            if (results.find(filePath) == results.end()) {
                std::cerr << "Warning: Failed to parse blob SHA for file: " << filePath << std::endl;
                std::cerr << "Response content: " << response.content << std::endl;
            }
        } else {
            std::cerr << "Error creating blob for file: " << filePath 
                      << " - HTTP " << response.responseCode;
            if (!response.error.empty()) {
                std::cerr << " (" << response.error << ")";
            }
            std::cerr << std::endl;
            
            if (!response.content.empty()) {
                std::cerr << "Response content: " << response.content << std::endl;
            }
        }
        
        // Update progress
        if (progressCallback) {
            progressCallback(i + 1, files.size(), filePath);
        }
    }
    
    return results;
}

HttpOptimization::PerformanceStats HttpOptimization::getStats() const {
    PerformanceStats stats;
    stats.totalRequests = totalRequests_.load();
    stats.cacheHits = cacheHits_.load();
    stats.cacheHitRate = stats.totalRequests > 0 ? 
        static_cast<double>(stats.cacheHits) / stats.totalRequests : 0.0;
    stats.activeConnections = activeConnections_.load();
    stats.poolSize = connectionPool_.size();
    // TODO: Implement average response time tracking
    stats.averageResponseTime = 0.0;
    stats.totalBytesTransferred = 0; // TODO: Implement
    
    return stats;
}

void HttpOptimization::resetStats() {
    totalRequests_ = 0;
    cacheHits_ = 0;
}

void HttpOptimization::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    responseCache_.clear();
}

HttpOptimization::CurlHandle* HttpOptimization::acquireConnection(const std::string& host) {
    applyRateLimit();
    
    std::unique_lock<std::mutex> lock(poolMutex_);
    
    // Use condition variable for better waiting instead of busy polling
    auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(30); // Increased timeout
    
    while (std::chrono::steady_clock::now() < timeout) {
        // Try to find an available connection for the same host (connection reuse)
        for (auto& handle : connectionPool_) {
            if (!handle->inUse && handle->lastHost == host) {
                handle->inUse = true;
                handle->lastUsed = std::chrono::steady_clock::now();
                ++activeConnections_;
                return handle.get();
            }
        }
        
        // Find any available connection
        for (auto& handle : connectionPool_) {
            if (!handle->inUse) {
                handle->inUse = true;
                handle->lastHost = host;
                handle->lastUsed = std::chrono::steady_clock::now();
                ++activeConnections_;
                return handle.get();
            }
        }
        
        // No connections available, create a temporary one if under limit
        if (connectionPool_.size() < config_.maxConnections * 3) { // Increased expansion allowance
            auto tempHandle = std::make_unique<CurlHandle>();
            tempHandle->curl = curl_easy_init();
            if (tempHandle->curl) {
                initializeCurlHandle(tempHandle->curl);
                tempHandle->inUse = true;
                tempHandle->lastHost = host;
                tempHandle->lastUsed = std::chrono::steady_clock::now();
                ++activeConnections_;
                
                CurlHandle* result = tempHandle.get();
                connectionPool_.push_back(std::move(tempHandle));
                return result;
            }
        }
        
        // Wait briefly before retrying
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Increased wait time
        lock.lock();
    }
    
    // If we still can't get a connection, force create one as last resort
    auto emergencyHandle = std::make_unique<CurlHandle>();
    emergencyHandle->curl = curl_easy_init();
    if (emergencyHandle->curl) {
        initializeCurlHandle(emergencyHandle->curl);
        emergencyHandle->inUse = true;
        emergencyHandle->lastHost = host;
        emergencyHandle->lastUsed = std::chrono::steady_clock::now();
        ++activeConnections_;
        
        CurlHandle* result = emergencyHandle.get();
        connectionPool_.push_back(std::move(emergencyHandle));
        return result;
    }
    
    return nullptr;
}

void HttpOptimization::releaseConnection(CurlHandle* handle) {
    if (handle) {
        std::lock_guard<std::mutex> lock(poolMutex_);
        handle->inUse = false;
        handle->requestCount++;
        --activeConnections_;
        
        // Clean up stale or overused connections
        if (handle->requestCount > 100 || 
            (std::chrono::steady_clock::now() - handle->lastUsed) > std::chrono::minutes(5)) {
            
            // Find and remove this handle from the pool
            auto it = std::find_if(connectionPool_.begin(), connectionPool_.end(),
                [handle](const std::unique_ptr<CurlHandle>& ptr) {
                    return ptr.get() == handle;
                });
            
            if (it != connectionPool_.end()) {
                connectionPool_.erase(it);
                
                // Create a replacement connection if we're below the minimum
                if (connectionPool_.size() < config_.maxConnections / 2) {
                    auto newHandle = std::make_unique<CurlHandle>();
                    newHandle->curl = curl_easy_init();
                    if (newHandle->curl) {
                        initializeCurlHandle(newHandle->curl);
                        connectionPool_.push_back(std::move(newHandle));
                    }
                }
            }
        }
    }
}

void HttpOptimization::initializeCurlHandle(CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_.requestTimeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, config_.connectionTimeout);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gyatt/1.0-optimized");
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    
    if (config_.enableKeepAlive) {
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
    }
    
    if (config_.enableHttp2) {
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    }
    
    if (config_.enableCompression) {
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    }
}

std::string HttpOptimization::getCacheKey(const std::string& method, const std::string& url, 
                                         const std::string& data) const {
    return method + "|" + url + "|" + std::to_string(std::hash<std::string>{}(data));
}

bool HttpOptimization::getCachedResponse(const std::string& cacheKey, OptimizedHttpResponse& response) {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    
    auto it = responseCache_.find(cacheKey);
    if (it != responseCache_.end()) {
        auto& [cachedResponse, timestamp] = it->second;
        
        // Check if cache entry is still valid
        auto now = std::chrono::steady_clock::now();
        if (now - timestamp < cacheExpiry_) {
            response = cachedResponse;
            response.fromCache = true;
            ++cacheHits_;
            return true;
        } else {
            // Cache entry expired, remove it
            responseCache_.erase(it);
        }
    }
    
    return false;
}

void HttpOptimization::cacheResponse(const std::string& cacheKey, const OptimizedHttpResponse& response) {
    if (response.success && response.responseCode == 200) {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        responseCache_[cacheKey] = {response, std::chrono::steady_clock::now()};
    }
}

void HttpOptimization::applyRateLimit() {
    std::lock_guard<std::mutex> lock(rateLimitMutex_);
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastRequest = now - lastRequest_;
    
    if (timeSinceLastRequest < minRequestInterval_) {
        auto sleepTime = minRequestInterval_ - timeSinceLastRequest;
        std::this_thread::sleep_for(sleepTime);
    }
    
    lastRequest_ = std::chrono::steady_clock::now();
}

HttpOptimization::OptimizedHttpResponse HttpOptimization::performRequest(
    const std::string& method, const std::string& url, const std::string& data,
    const std::vector<std::string>& headers) {
    
    ++totalRequests_;
    
    // Check cache for GET requests
    if (method == "GET") {
        std::string cacheKey = getCacheKey(method, url, data);
        OptimizedHttpResponse cachedResponse;
        if (getCachedResponse(cacheKey, cachedResponse)) {
            return cachedResponse;
        }
    }
    
    OptimizedHttpResponse response;
    response.success = false;
    response.responseCode = 0;
    response.fromCache = false;
    
    // Extract host from URL for connection reuse
    std::regex urlRegex(R"(https?://([^/]+))");
    std::smatch matches;
    std::string host;
    if (std::regex_search(url, matches, urlRegex)) {
        host = matches[1].str();
    }
    
    CurlHandle* handle = acquireConnection(host);
    if (!handle || !handle->curl) {
        response.error = "Failed to acquire connection - pool exhausted";
        response.responseCode = 0;
        
        // Add exponential backoff for connection failures
        static thread_local int failureCount = 0;
        failureCount++;
        int delayMs = std::min(1000, 50 * (1 << std::min(failureCount, 4))); // Cap at ~800ms
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        
        // Try one more time with emergency connection
        handle = acquireConnection(host);
        if (!handle || !handle->curl) {
            response.error = "Failed to acquire connection after retry";
            return response;
        }
        failureCount = 0; // Reset on success
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        // Configure request
        curl_easy_setopt(handle->curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, &response.content);
        
        // Clear previous response data
        response.content.clear();
        
        // Reset curl handle to clean state
        curl_easy_reset(handle->curl);
        initializeCurlHandle(handle->curl);
        curl_easy_setopt(handle->curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, &response.content);
        
        // Set method and data
        if (method == "POST") {
            curl_easy_setopt(handle->curl, CURLOPT_POST, 1L);
            curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDSIZE, data.length());
        } else if (method == "PUT") {
            curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDSIZE, data.length());
        } else if (method == "PATCH") {
            curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDSIZE, data.length());
        } else {
            curl_easy_setopt(handle->curl, CURLOPT_HTTPGET, 1L);
        }
        
        // Set headers
        struct curl_slist* headerList = nullptr;
        for (const auto& header : headers) {
            headerList = curl_slist_append(headerList, header.c_str());
        }
        if (headerList) {
            curl_easy_setopt(handle->curl, CURLOPT_HTTPHEADER, headerList);
        }
        
        // Perform request with retries for transient errors
        CURLcode res = CURLE_FAILED_INIT;
        int retryCount = 0;
        const int maxRetries = 3;
        
        while (retryCount < maxRetries) {
            res = curl_easy_perform(handle->curl);
            
            if (res == CURLE_OK) {
                break;
            }
            
            // Check if this is a retryable error
            if (res == CURLE_COULDNT_CONNECT || 
                res == CURLE_COULDNT_RESOLVE_HOST ||
                res == CURLE_OPERATION_TIMEDOUT ||
                res == CURLE_SEND_ERROR ||
                res == CURLE_RECV_ERROR) {
                
                retryCount++;
                if (retryCount < maxRetries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100 * retryCount));
                    response.content.clear(); // Clear any partial content
                }
            } else {
                break; // Non-retryable error
            }
        }
        
        auto endTime = std::chrono::steady_clock::now();
        response.transferTime = std::chrono::duration<double>(endTime - startTime).count();
        
        if (res == CURLE_OK) {
            curl_easy_getinfo(handle->curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
            response.success = (response.responseCode >= 200 && response.responseCode < 300);
            
            curl_off_t downloadSize;
            curl_easy_getinfo(handle->curl, CURLINFO_SIZE_DOWNLOAD_T, &downloadSize);
            response.bytesTransferred = static_cast<size_t>(downloadSize);
        } else {
            response.error = curl_easy_strerror(res);
            response.responseCode = 0;
        }
        
        // Clean up
        if (headerList) {
            curl_slist_free_all(headerList);
        }
        
    } catch (const std::exception& e) {
        response.error = "Exception during HTTP request: " + std::string(e.what());
        response.responseCode = 0;
    } catch (...) {
        response.error = "Unknown exception during HTTP request";
        response.responseCode = 0;
    }
    
    releaseConnection(handle);
    
    // Cache successful GET responses
    if (method == "GET" && response.success) {
        std::string cacheKey = getCacheKey(method, url, data);
        cacheResponse(cacheKey, response);
    }
    
    return response;
}

size_t HttpOptimization::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

int HttpOptimization::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                                      curl_off_t ultotal, curl_off_t ulnow) {
    // Suppress unused parameter warnings
    (void)clientp;
    (void)dltotal;
    (void)dlnow;
    (void)ultotal;
    (void)ulnow;
    
    // Progress callback implementation if needed
    return 0;
}

} // namespace gyatt
