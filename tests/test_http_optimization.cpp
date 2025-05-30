#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <random>
#include <sstream>
#include "http_optimization.h"

using namespace gyatt;

// Test data generator
class TestDataGenerator {
public:
    static std::string generateRandomContent(size_t size) {
        static const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
        
        std::string result;
        result.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            result += chars[dis(gen)];
        }
        return result;
    }
    
    static std::vector<std::pair<std::string, std::string>> generateTestFiles(size_t count, size_t avgSize = 1024) {
        std::vector<std::pair<std::string, std::string>> files;
        files.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            std::ostringstream filename;
            filename << "test_file_" << i << ".txt";
            
            std::string content = generateRandomContent(avgSize + (i % 512));
            files.emplace_back(filename.str(), content);
        }
        
        return files;
    }
};

// Performance benchmark
class HttpOptimizationBenchmark {
private:
    HttpOptimization httpOpt_;
    
public:
    HttpOptimizationBenchmark() {
        // Configure for optimal performance
        HttpOptimization::ConnectionPoolConfig config;
        config.maxConnections = 20;
        config.maxConnectionsPerHost = 10;
        config.connectionTimeout = 30L;
        config.requestTimeout = 60L;
        config.enableCompression = true;
        config.enableKeepAlive = true;
        config.enableHttp2 = true;
        config.maxRetries = 2;
        
        httpOpt_.setConfig(config);
        httpOpt_.enableCompression(true);
        httpOpt_.setCacheExpiry(std::chrono::seconds(300));
        httpOpt_.setRateLimit(std::chrono::milliseconds(10)); // 100 requests per second
    }
    
    void benchmarkSingleRequests(size_t numRequests) {
        std::cout << "\n=== Single Request Benchmark ===\n";
        std::cout << "Testing " << numRequests << " individual HTTP requests...\n";
        
        auto startTime = std::chrono::steady_clock::now();
        
        for (size_t i = 0; i < numRequests; ++i) {
            std::string testData = TestDataGenerator::generateRandomContent(512);
            std::vector<std::string> headers = {"Content-Type: application/json"};
            
            // Test with httpbin.org echo service
            auto response = httpOpt_.httpPost("https://httpbin.org/post", testData, headers);
            
            if (i % 10 == 0) {
                std::cout << "  Completed " << (i + 1) << "/" << numRequests 
                          << " requests (success: " << (response.success ? "yes" : "no") 
                          << ", from cache: " << (response.fromCache ? "yes" : "no") << ")\n";
            }
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        auto stats = httpOpt_.getStats();
        
        std::cout << "Single request benchmark completed in " << duration.count() << "ms\n";
        std::cout << "Average time per request: " << (duration.count() / static_cast<double>(numRequests)) << "ms\n";
        std::cout << "Requests per second: " << (numRequests * 1000.0 / duration.count()) << "\n";
        printStats(stats);
    }
    
    void benchmarkBatchRequests(size_t numBatches, size_t batchSize) {
        std::cout << "\n=== Batch Request Benchmark ===\n";
        std::cout << "Testing " << numBatches << " batches of " << batchSize << " requests...\n";
        
        auto startTime = std::chrono::steady_clock::now();
        
        for (size_t batch = 0; batch < numBatches; ++batch) {
            std::vector<HttpOptimization::BatchRequest> requests;
            requests.reserve(batchSize);
            
            for (size_t i = 0; i < batchSize; ++i) {
                HttpOptimization::BatchRequest request;
                request.url = "https://httpbin.org/post";
                request.method = "POST";
                request.data = TestDataGenerator::generateRandomContent(256);
                request.headers = {"Content-Type: application/json"};
                request.priority = i % 3; // Vary priority
                
                requests.push_back(request);
            }
            
            auto responses = httpOpt_.executeRequestBatch(requests);
            
            size_t successCount = 0;
            size_t cacheHits = 0;
            for (const auto& resp : responses) {
                if (resp.success) successCount++;
                if (resp.fromCache) cacheHits++;
            }
            
            std::cout << "  Batch " << (batch + 1) << "/" << numBatches 
                      << " completed: " << successCount << "/" << batchSize 
                      << " successful, " << cacheHits << " cache hits\n";
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        auto stats = httpOpt_.getStats();
        size_t totalRequests = numBatches * batchSize;
        
        std::cout << "Batch request benchmark completed in " << duration.count() << "ms\n";
        std::cout << "Average time per request: " << (duration.count() / static_cast<double>(totalRequests)) << "ms\n";
        std::cout << "Requests per second: " << (totalRequests * 1000.0 / duration.count()) << "\n";
        printStats(stats);
    }
    
    void benchmarkConnectionReuse() {
        std::cout << "\n=== Connection Reuse Benchmark ===\n";
        std::cout << "Testing connection pooling efficiency...\n";
        
        // Reset stats to get clean measurements
        httpOpt_.resetStats();
        
        auto startTime = std::chrono::steady_clock::now();
        
        // Make multiple requests to the same host to test connection reuse
        std::string baseUrl = "https://httpbin.org";
        std::vector<std::string> endpoints = {"/get", "/post", "/put", "/patch", "/delete"};
        
        for (int round = 0; round < 5; ++round) {
            for (const auto& endpoint : endpoints) {
                std::string url = baseUrl + endpoint;
                auto response = httpOpt_.httpGet(url);
                
                std::cout << "  Round " << (round + 1) << ", endpoint " << endpoint 
                          << " (success: " << (response.success ? "yes" : "no") 
                          << ", transfer time: " << response.transferTime << "ms)\n";
            }
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        auto stats = httpOpt_.getStats();
        
        std::cout << "Connection reuse benchmark completed in " << duration.count() << "ms\n";
        printStats(stats);
        
        // Calculate connection efficiency
        if (stats.totalRequests > 0) {
            double reuseRatio = 1.0 - (stats.poolSize / static_cast<double>(stats.totalRequests));
            std::cout << "Connection reuse efficiency: " << (reuseRatio * 100.0) << "%\n";
        }
    }
    
    void benchmarkCacheEfficiency() {
        std::cout << "\n=== Cache Efficiency Benchmark ===\n";
        std::cout << "Testing response caching...\n";
        
        httpOpt_.resetStats();
        
        auto startTime = std::chrono::steady_clock::now();
        
        // Make repeated requests to the same URL to test caching
        std::string url = "https://httpbin.org/get";
        
        for (int i = 0; i < 20; ++i) {
            auto response = httpOpt_.httpGet(url);
            
            std::cout << "  Request " << (i + 1) << "/20 "
                      << "(success: " << (response.success ? "yes" : "no") 
                      << ", from cache: " << (response.fromCache ? "yes" : "no") 
                      << ", transfer time: " << response.transferTime << "ms)\n";
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        auto stats = httpOpt_.getStats();
        
        std::cout << "Cache efficiency benchmark completed in " << duration.count() << "ms\n";
        printStats(stats);
    }
    
    void printStats(const HttpOptimization::PerformanceStats& stats) {
        std::cout << "\nïž“Š Performance Statistics:\n";
        std::cout << "  á€¢ Total requests: " << stats.totalRequests << "\n";
        std::cout << "  â¢ Cache hits: " << stats.cacheHits << " (" << stats.cacheHitRate << "%)\n";
        std::cout << "   á¢ Average response time: " << stats.averageResponseTime << "ms\n";
        std::cout << "  á€¢ Active connections: " << stats.activeConnections << "\n";
        std::cout << "  á€¢ Pool size: " << stats.poolSize << "\n";
        std::cout << "  á€¢ Total data transferred: " << (stats.totalBytesTransferred / 1024.0) << " KB\n";
    }
};

int main() {
    std::cout << "ïžš€ HTTP Optimization Performance Test Suite\n";
    std::cout << "==========================================\n";
    
    try {
        HttpOptimizationBenchmark benchmark;
        
        // Run comprehensive benchmarks
        benchmark.benchmarkSingleRequests(20);  // Fewer requests for real testing
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        benchmark.benchmarkBatchRequests(3, 5);  // Smaller batches for real testing
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        benchmark.benchmarkConnectionReuse();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        benchmark.benchmarkCacheEfficiency();
        
        std::cout << "\máœ… HTTP Optimization benchmark completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "áœŒ Benchmark failed: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
