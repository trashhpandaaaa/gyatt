#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <atomic>
#include <future>
#include <curl/curl.h>
#include "utils.h"

namespace gyatt {

/**
 * @brief Advanced HTTP optimization manager for high-performance GitHub API operations
 * 
 * This class provides:
 * - Connection pooling and reuse
 * - Request compression (gzip)
 * - Adaptive request batching
 * - Network latency optimization
 * - Memory pool management
 * - Rate limiting compliance
 */
class HttpOptimization {
public:
    struct OptimizedHttpResponse {
        std::string content;
        long responseCode;
        bool success;
        std::string error;
        double transferTime;
        size_t bytesTransferred;
        bool fromCache;
    };

    struct ConnectionPoolConfig {
        size_t maxConnections = 10;
        size_t maxConnectionsPerHost = 6;
        long connectionTimeout = 30L;
        long requestTimeout = 60L;
        bool enableCompression = true;
        bool enableKeepAlive = true;
        bool enableHttp2 = true;
        size_t maxRetries = 3;
    };

    struct BatchRequest {
        std::string url;
        std::string method;
        std::string data;
        std::vector<std::string> headers;
        size_t priority = 0; // Higher priority = executed first
    };

private:
    struct CurlHandle {
        CURL* curl;
        bool inUse;
        std::string lastHost;
        std::chrono::steady_clock::time_point lastUsed;
        size_t requestCount;
        
        CurlHandle() : curl(nullptr), inUse(false), requestCount(0) {}
        ~CurlHandle() {
            if (curl) {
                curl_easy_cleanup(curl);
            }
        }
    };

    ConnectionPoolConfig config_;
    std::vector<std::unique_ptr<CurlHandle>> connectionPool_;
    std::mutex poolMutex_;
    std::atomic<size_t> activeConnections_{0};
    std::atomic<size_t> totalRequests_{0};
    std::atomic<size_t> cacheHits_{0};
    
    // Request compression
    bool compressionEnabled_ = true;
    
    // Memory pool for small allocations
    std::vector<std::unique_ptr<char[]>> memoryPool_;
    std::mutex memoryPoolMutex_;
    
    // Response cache for repeated requests
    std::map<std::string, std::pair<OptimizedHttpResponse, std::chrono::steady_clock::time_point>> responseCache_;
    std::mutex cacheMutex_;
    std::chrono::seconds cacheExpiry_{300}; // 5 minutes
    
    // Rate limiting
    std::chrono::steady_clock::time_point lastRequest_;
    std::mutex rateLimitMutex_;
    std::chrono::milliseconds minRequestInterval_{50}; // 20 requests per second max
    
public:
    HttpOptimization();
    ~HttpOptimization();
    
    // Configuration
    void setConfig(const ConnectionPoolConfig& config);
    void enableCompression(bool enable);
    void setCacheExpiry(std::chrono::seconds expiry);
    void setRateLimit(std::chrono::milliseconds minInterval);
    
    // Single requests with optimization
    OptimizedHttpResponse httpGet(const std::string& url, 
                                 const std::vector<std::string>& headers = {});
    OptimizedHttpResponse httpPost(const std::string& url, 
                                  const std::string& data,
                                  const std::vector<std::string>& headers = {});
    OptimizedHttpResponse httpPatch(const std::string& url, 
                                   const std::string& data,
                                   const std::vector<std::string>& headers = {});
    OptimizedHttpResponse httpPut(const std::string& url, 
                                 const std::string& data,
                                 const std::vector<std::string>& headers = {});
    
    // Batch request processing
    std::vector<OptimizedHttpResponse> executeRequestBatch(
        const std::vector<BatchRequest>& requests);
    
    // Parallel blob creation optimized for GitHub
    std::map<std::string, std::string> createBlobsBatch(
        const std::string& repoName,
        const std::vector<std::pair<std::string, std::string>>& files,
        const std::string& token,
        std::function<void(size_t completed, size_t total, const std::string& currentFile)> progressCallback = nullptr);
    
    // Statistics and monitoring
    struct PerformanceStats {
        size_t totalRequests;
        size_t cacheHits;
        double cacheHitRate;
        size_t activeConnections;
        size_t poolSize;
        double averageResponseTime;
        size_t totalBytesTransferred;
    };
    
    PerformanceStats getStats() const;
    void resetStats();
    void clearCache();
    
    // Memory management
    void* allocateMemory(size_t size);
    void deallocateMemory(void* ptr);
    void optimizeMemoryPool();

private:
    CurlHandle* acquireConnection(const std::string& host);
    void releaseConnection(CurlHandle* handle);
    void initializeCurlHandle(CURL* curl);
    
    std::string getCacheKey(const std::string& method, const std::string& url, 
                           const std::string& data) const;
    bool getCachedResponse(const std::string& cacheKey, OptimizedHttpResponse& response);
    void cacheResponse(const std::string& cacheKey, const OptimizedHttpResponse& response);
    
    void applyRateLimit();
    std::string compressData(const std::string& data);
    std::string decompressData(const std::string& data);
    
    OptimizedHttpResponse performRequest(const std::string& method,
                                       const std::string& url,
                                       const std::string& data,
                                       const std::vector<std::string>& headers);
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                               curl_off_t ultotal, curl_off_t ulnow);
};

} // namespace gyatt
