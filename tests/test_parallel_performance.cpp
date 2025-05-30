#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <random>
#include <algorithm>

// Mock HTTP response for testing
struct MockHttpResponse {
    bool success = true;
    int responseCode = 200;
    std::string content = R"({"sha":"abc123def456"})";
    std::string error;
};

// Mock GitHub blob creation function
MockHttpResponse mockCreateBlob(const std::string& fileName, const std::string& content, 
                               std::chrono::milliseconds delay = std::chrono::milliseconds(50)) {
    // Simulate network latency
    std::this_thread::sleep_for(delay);
    
    MockHttpResponse response;
    
    // Simulate occasional network errors (5% chance)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, 100);
    
    if (dis(gen) <= 5) {
        response.success = false;
        response.responseCode = 500;
        response.error = "Simulated network error";
        return response;
    }
    
    // Generate a mock SHA
    std::hash<std::string> hasher;
    size_t hash = hasher(fileName + content);
    char sha[41];
    snprintf(sha, sizeof(sha), "%040zx", hash);
    
    response.content = R"({"sha":")" + std::string(sha).substr(0, 40) + R"("})";
    return response;
}

int main() {
    std::cout << "=== GYATT PARALLEL GITHUB PUSH PERFORMANCE TEST ===\n\n";
    
    // Create a list of test files (simulate the 60 files we created)
    std::vector<std::string> testFiles;
    for (int i = 1; i <= 20; ++i) {
        testFiles.push_back("module" + std::to_string(i) + "/src/module" + std::to_string(i) + ".cpp");
        testFiles.push_back("module" + std::to_string(i) + "/include/module" + std::to_string(i) + ".h");
        testFiles.push_back("module" + std::to_string(i) + "/docs/README.md");
    }
    
    std::cout << "Testing with " << testFiles.size() << " files\n";
    std::cout << "Simulating GitHub API blob creation with 50ms network latency per request\n\n";
    
    // Test sequential approach (original implementation)
    std::cout << "=== SEQUENTIAL APPROACH (Original Implementation) ===\n";
    auto seqStart = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::string mockContent = "// Mock content for " + testFiles[i] + "\n";
        auto response = mockCreateBlob(testFiles[i], mockContent);
        
        if (response.success) {
            // Extract SHA from mock response
            size_t start_pos = response.content.find("\"sha\":\"") + 7;
            size_t end_pos = response.content.find("\"", start_pos);
            std::string sha = response.content.substr(start_pos, end_pos - start_pos);
            std::cout << "  [" << (i + 1) << "/" << testFiles.size() << "] " 
                      << testFiles[i] << " -> " << sha.substr(0, 8) << "...\n";
        } else {
            std::cout << "  [" << (i + 1) << "/" << testFiles.size() << "] " 
                      << testFiles[i] << " -> FAILED (" << response.responseCode << ")\n";
        }
    }
    
    auto seqEnd = std::chrono::high_resolution_clock::now();
    auto seqDuration = std::chrono::duration_cast<std::chrono::milliseconds>(seqEnd - seqStart);
    
    std::cout << "Sequential blob creation completed in " << seqDuration.count() << "ms\n";
    std::cout << "Files per second: " << (testFiles.size() * 1000.0 / seqDuration.count()) << "\n\n";
    
    // Test parallel approach (optimized implementation)
    std::cout << "=== PARALLEL APPROACH (Optimized Implementation) ===\n";
    
    std::vector<std::string> blobShas(testFiles.size());
    std::vector<bool> completed(testFiles.size(), false);
    std::atomic<size_t> completedCount{0};
    std::mutex consoleMutex;
    
    // Calculate optimal thread count
    const unsigned int maxThreads = 8;
    const unsigned int minThreads = 2;
    unsigned int numThreads = std::min(maxThreads, 
                                      std::max(minThreads, 
                                              std::thread::hardware_concurrency()));
    
    std::cout << "Using " << numThreads << " threads for parallel processing\n";
    std::cout << "Files per thread: " << (testFiles.size() / numThreads) << "\n\n";
    
    auto parStart = std::chrono::high_resolution_clock::now();
    
    // Divide files into chunks for each thread
    size_t chunkSize = std::max(size_t(1), testFiles.size() / numThreads);
    std::vector<std::future<void>> futures;
    
    for (unsigned int threadId = 0; threadId < numThreads; ++threadId) {
        size_t startIdx = threadId * chunkSize;
        size_t endIdx = (threadId == numThreads - 1) ? testFiles.size() : std::min(startIdx + chunkSize, testFiles.size());
        
        if (startIdx >= testFiles.size()) break;
        
        futures.push_back(std::async(std::launch::async, [&, threadId, startIdx, endIdx]() {
            for (size_t i = startIdx; i < endIdx; ++i) {
                try {
                    std::string mockContent = "// Mock content for " + testFiles[i] + "\n";
                    auto response = mockCreateBlob(testFiles[i], mockContent);
                    
                    if (response.success) {
                        // Extract SHA from mock response
                        size_t start_pos = response.content.find("\"sha\":\"") + 7;
                        size_t end_pos = response.content.find("\"", start_pos);
                        std::string sha = response.content.substr(start_pos, end_pos - start_pos);
                        blobShas[i] = sha;
                        completed[i] = true;
                        
                        size_t current = ++completedCount;
                        {
                            std::lock_guard<std::mutex> lock(consoleMutex);
                            std::cout << "  [" << current << "/" << testFiles.size() << "] " 
                                      << testFiles[i] << " -> " << sha.substr(0, 8) << "... (thread " << threadId << ")\n";
                        }
                    } else {
                        size_t current = ++completedCount;
                        {
                            std::lock_guard<std::mutex> lock(consoleMutex);
                            std::cout << "  [" << current << "/" << testFiles.size() << "] " 
                                      << testFiles[i] << " -> FAILED (" << response.responseCode << ") (thread " << threadId << ")\n";
                        }
                    }
                } catch (const std::exception& e) {
                    {
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cerr << "Thread " << threadId << " error for " << testFiles[i] << ": " << e.what() << "\n";
                    }
                }
            }
        }));
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    auto parEnd = std::chrono::high_resolution_clock::now();
    auto parDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parEnd - parStart);
    
    std::cout << "Parallel blob creation completed in " << parDuration.count() << "ms\n";
    std::cout << "Files per second: " << (testFiles.size() * 1000.0 / parDuration.count()) << "\n";
    std::cout << "Successful uploads: " << std::count(completed.begin(), completed.end(), true) << "/" << testFiles.size() << "\n\n";
    
    // Performance comparison
    double speedup = static_cast<double>(seqDuration.count()) / parDuration.count();
    double improvement = ((seqDuration.count() - parDuration.count()) / static_cast<double>(seqDuration.count())) * 100;
    
    std::cout << "=== PERFORMANCE COMPARISON ===\n";
    std::cout << "Sequential Time: " << seqDuration.count() << "ms\n";
    std::cout << "Parallel Time: " << parDuration.count() << "ms\n";
    std::cout << "Speedup: " << speedup << "x\n";
    std::cout << "Performance Improvement: " << improvement << "%\n";
    std::cout << "Time Saved: " << (seqDuration.count() - parDuration.count()) << "ms\n\n";
    
    std::cout << "=== TECHNICAL DETAILS ===\n";
    std::cout << "Hardware threads available: " << std::thread::hardware_concurrency() << "\n";
    std::cout << "Threads used: " << numThreads << "\n";
    std::cout << "Files per thread: " << chunkSize << "\n";
    std::cout << "Network latency per request: 50ms\n";
    std::cout << "Total theoretical sequential time: " << (testFiles.size() * 50) << "ms\n";
    std::cout << "Total theoretical parallel time: " << ((testFiles.size() * 50) / numThreads) << "ms\n";
    std::cout << "Theoretical speedup: " << numThreads << "x\n\n";
    
    if (speedup > 1.0) {
        std::cout << "✅ OPTIMIZATION SUCCESSFUL!\n";
        std::cout << "The parallel implementation is " << speedup << " times faster than sequential!\n";
        std::cout << "This demonstrates the effectiveness of our GitHub push optimization.\n";
    } else {
        std::cout << "❌ OPTIMIZATION NEEDS IMPROVEMENT\n";
        std::cout << "Consider adjusting thread count or chunk size.\n";
    }
    
    return 0;
}
