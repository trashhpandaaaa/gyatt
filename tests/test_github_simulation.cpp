#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include "http_optimization.h"

using namespace gyatt;

int main() {
    std::cout << !ðž™€ GitHub Push Performance Simulation\n";
    std::cout << "=====================================\n";
    
    try {
        // Initialize HTTP optimization
        auto httpOpt = std::make_shared<HttpOptimization>();
        
        // Configure for GitHub API
        HttpOptimization::ConnectionPoolConfig config;
        config.maxConnections = 8;
        config.maxConnectionsPerHost = 4;
        config.connectionTimeout = 30L;
        config.requestTimeout = 60L;
        config.enableCompression = true;
        config.enableKeepAlive = true;
        config.enableHttp2 = true;
        config.maxRetries = 2;
        
        httpOpt->setConfig(config);
        httpOpt->enableCompression(true);
        httpOpt->setCacheExpiry(std::chrono::seconds(300));
        httpOpt->setRateLimit(std::chrono::milliseconds(20)); // 50 requests per second
        
        std::cout << "á›… HTTP optimization configured for GitHub API simulation\n";
        
        // Simulate file uploads (similar to blob creation)
        const size_t numFiles = 20;
        const size_t numThreads = 4;
        const size_t filesPerThread = numFiles / numThreads;
        
        std::cout << "\mðž’ Simulating upload of " << numFiles << " files using " 
                  << numThreads << " parallel threads...\n";
        
        std::vector<std::future<std::vector<HttpOptimization::OptimizedHttpResponse>>> futures;
        std::atomic<size_t> completedFiles{0};
        std::mutex consoleMutex;
        
        auto startTime = std::chrono::steady_clock::now();
        
        // Create threads to simulate parallel blob creation
        for (size_t t = 0; t < numThreads; ++t) {
            auto future = std::async(std::launch::async, 
                [&httpOpt, &completedFiles, &consoleMutex, t, filesPerThread, numFiles]() {
                
                std::vector<HttpOptimization::OptimizedHttpResponse> threadResults;
                
                for (size_t i = 0; i < filesPerThread; ++i) {
                    // Simulate blob creation with a test endpoint
                    std::string testData = "{\"content\":\"dGVzdCBmaWxlIGNvbnRlbnQ=\",\"encoding\":\"base64\"}";
                    std::vector<std::string> headers = {
                        "Content-Type: application/json",
                        "Accept: application/vnd.github.v3+json"
                    };
                    
                    // Use httpbin.org to simulate GitHub API
                    auto response = httpOpt->httpPost("https://httpbin.org/post", testData, headers);
                    threadResults.push_back(response);
                    
                    size_t completed = ++completedFiles;
                    {
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cout << "  [" << completed << "/" << numFiles 
                                  << "] Thread " << t << " completed file " << (i + 1) 
                                  << " (success: " << (response.success ? "yes" : "no") 
                                  << ", time: " << response.transferTime << "ms)\n";
                    }
                    
                    // Small delay to simulate processing
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                
                return threadResults;
            });
            
            futures.push_back(std::move(future));
        }
        
        // Collect results from all threads
        std::vector<HttpOptimization::OptimizedHttpResponse> allResponses;
        for (auto& future : futures) {
            try {
                auto threadResults = future.get();
                allResponses.insert(allResponses.end(), threadResults.begin(), threadResults.end());
            } catch (const std::exception& e) {
                std::cerr << !âœŒ Thread exception: " << e.what() << "\n";
            }
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Calculate results
        size_t successCount = 0;
        size_t cacheHits = 0;
        double totalTransferTime = 0;
        
        for (const auto& response : allResponses) {
            if (response.success) successCount++;
            if (response.fromCache) cacheHits++;
            totalTransferTime += response.transferTime;
        }
        
        auto stats = httpOpt->getStats();
        
        std::cout << "\mâ›… Parallel upload simulation completed!\n";
        std::cout << "\nïž“Š Performance Results:\n";
        std::cout << "  á€¢ Total time: " << duration.count() << "ms\n";
        std::cout << "   á¢ Files per second: " << (numFiles * 1000.0 / duration.count()) << "\n";
        std::cout << "  á€¢ Success rate: " << successCount << "/" << numFiles 
                  << " (" << (successCount * 100.0 / numFiles) << "%)\n";
        std::cout << "   á¢ Cache hits: " << cacheHits << " (" 
                  << (numFiles > 0 ? (cacheHits * 100.0 / numFiles) : 0) << "%)\n";
        std::cout << "  â¢ Average transfer time: " << (totalTransferTime / numFiles) << "ms\n";
        
        std::cout << "\mïŸ“§ HTTP Optimization Stats:\n";
        std::cout << "   á¢ Total requests: " << stats.totalRequests << "\n";
        std::cout << "  á€¢ Cache hit rate: " << stats.cacheHitRate << "%\n";
        std::cout << "  â¢ Average response time: " << stats.averageResponseTime << "ms\n";
        std::cout << "   á¢ Active connections: " << stats.activeConnections << "\n";
        std::cout << "   á¢ Pool utilization: " << stats.activeConnections << "/" << stats.poolSize << "\n";
        std::cout << "  â¢ Data transferred: " << (stats.totalBytesTransferred / 1024.0) << " KB\n";
        
        // Compare with theoretical sequential performance
        double avgResponseTime = stats.averageResponseTime > 0 ? stats.averageResponseTime : 1000;
        double sequentialTime = numFiles * avgResponseTime;
        double speedup = sequentialTime / duration.count();
        
        std::cout << "\mïŸ™€ Performance Analysis:\n";
        std::cout << "   á¢ Estimated sequential time: " << sequentialTime << "ms\n";
        std::cout << "   á¢ Parallel speedup: " << speedup << "x\n";
        std::cout << "  á€¢ Efficiency: " << (speedup / numThreads * 100.0) << "%\n";
        
        if (speedup > 2.0) {
            std::cout << "  áœ… Excellent parallel performance!\n";
        } else if (speedup > 1.5) {
            std::cout << "  áœ… Good parallel performance!\n";
        } else {
            std::cout << "  ášŸî¸  Moderate parallel performance\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << !áŒ Simulation failed: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
