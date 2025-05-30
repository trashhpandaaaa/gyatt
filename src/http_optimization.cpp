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
    
    // Sort requests by priority (higher priority first)
    std::vector<size_t> indices(requests.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&requests](size_t a, size_t b) {
        return requests[a].priority > requests[b].priority;
    });
    
    // Execute requests with optimal parallelism
    const size_t maxParallel = std::min(config_.maxConnections, requests.size());
    std::vector<std::future<OptimizedHttpResponse>> futures;
    
    for (size_t i = 0; i < requests.size(); ++i) {
        if (futures.size() >= maxParallel) {
            // Wait for at least one to complete
            for (auto it = futures.begin(); it != futures.end(); ++it) {
                if (it->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                    futures.erase(it);
                    break;
                }
            }
            if (futures.size() >= maxParallel) {
                futures.front().wait();
                futures.erase(futures.begin());
            }
        }
        
        size_t reqIdx = indices[i];
        const auto& req = requests[reqIdx];
        
        auto future = std::async(std::launch::async, [this, req]() {
            return performRequest(req.method, req.url, req.data, req.headers);
        });
        
        futures.push_back(std::move(future));
    }
    
    // Wait for all remaining requests
    for (auto& future : futures) {
        future.wait();
    }
    
    return responses;
}

std::map<std::string, std::string> HttpOptimization::createBlobsBatch(
    const std::string& repoName,
    const std::vector<std::pair<std::string, std::string>>& files,
    const std::string& token,
    std::function<void(size_t completed, size_t total, const std::string& currentFile)> progressCallback) {
    
    std::map<std::string, std::string> results;
    std::mutex resultsMutex;
    std::atomic<size_t> completed{0};
    
    // Prepare batch requests
    std::vector<BatchRequest> batchRequests;
    for (const auto& [filePath, fileContent] : files) {
        BatchRequest req;
        req.url = "https://api.github.com/repos/" + repoName + "/git/blobs";
        req.method = "POST";
        
        // Prepare blob data with compression if beneficial
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
    
    // Execute optimized parallel requests
    const size_t numThreads = std::min(static_cast<size_t>(8), 
                                      std::max(static_cast<size_t>(2), 
                                      static_cast<size_t>(std::thread::hardware_concurrency())));
    const size_t chunkSize = std::max(static_cast<size_t>(1), 
                                     (files.size() + numThreads - 1) / numThreads);
    
    std::vector<std::future<void>> futures;
    
    for (size_t i = 0; i < files.size(); i += chunkSize) {
        size_t endIdx = std::min(i + chunkSize, files.size());
        
        auto future = std::async(std::launch::async, [=, &files, &results, &resultsMutex, 
                                                     &completed, &progressCallback]() {
            for (size_t j = i; j < endIdx; ++j) {
                const auto& [filePath, fileContent] = files[j];
                
                // Use optimized HTTP request
                OptimizedHttpResponse response = httpPost(
                    "https://api.github.com/repos/" + repoName + "/git/blobs",
                    "{\"content\":\"" + Utils::base64Encode(fileContent) + "\",\"encoding\":\"base64\"}",
                    {
                        "Authorization: token " + token,
                        "Accept: application/vnd.github.v3+json",
                        "Content-Type: application/json"
                    }
                );
                
                if (response.success) {
                    // Parse SHA from response
                    size_t shaPos = response.content.find("\"sha\":");
                    if (shaPos != std::string::npos) {
                        shaPos = response.content.find("\"", shaPos + 6);
                        if (shaPos != std::string::npos) {
                            size_t shaEnd = response.content.find("\"", shaPos + 1);
                            if (shaEnd != std::string::npos) {
                                std::string blobSha = response.content.substr(shaPos + 1, shaEnd - shaPos - 1);
                                
                                {
                                    std::lock_guard<std::mutex> lock(resultsMutex);
                                    results[filePath] = blobSha;
                                }
                            }
                        }
                    }
                }
                
                size_t currentCompleted = ++completed;
                if (progressCallback) {
                    progressCallback(currentCompleted, files.size(), filePath);
                }
            }
        });
        
        futures.push_back(std::move(future));
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
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
    
    std::lock_guard<std::mutex> lock(poolMutex_);
    
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
    
    // No connections available, create a temporary one
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
    
    return nullptr;
}

void HttpOptimization::releaseConnection(CurlHandle* handle) {
    if (handle) {
        handle->inUse = false;
        handle->requestCount++;
        --activeConnections_;
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
        response.error = "Failed to acquire connection";
        return response;
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Configure request
    curl_easy_setopt(handle->curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle->curl, CURLOPT_WRITEDATA, &response.content);
    
    // Clear previous response data
    response.content.clear();
    
    // Set method and data
    if (method == "POST") {
        curl_easy_setopt(handle->curl, CURLOPT_POST, 1L);
        curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, data.c_str());
    } else if (method == "PUT") {
        curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, data.c_str());
    } else if (method == "PATCH") {
        curl_easy_setopt(handle->curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        curl_easy_setopt(handle->curl, CURLOPT_POSTFIELDS, data.c_str());
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
    
    // Perform request
    CURLcode res = curl_easy_perform(handle->curl);
    
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
    }
    
    // Clean up
    if (headerList) {
        curl_slist_free_all(headerList);
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
