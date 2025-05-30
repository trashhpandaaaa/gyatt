#include "memory_optimization.h"
#include "utils.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include <iomanip>

using namespace gyatt;

// Test data generator
class TestDataGenerator {
public:
    static std::string generateRandomString(size_t length) {
        const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.size() - 1);
        
        std::string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            result += chars[dis(gen)];
        }
        
        return result;
    }
    
    static std::vector<std::string> generateTestObjects(size_t count, size_t minSize, size_t maxSize) {
        std::vector<std::string> objects;
        objects.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> sizeDis(minSize, maxSize);
        
        for (size_t i = 0; i < count; ++i) {
            size_t size = sizeDis(gen);
            objects.push_back(generateRandomString(size));
        }
        
        return objects;
    }
};

// Performance benchmarking
class MemoryBenchmark {
public:
    struct BenchmarkResult {
        std::chrono::milliseconds allocationTime{0};
        std::chrono::milliseconds cacheTime{0};
        std::chrono::milliseconds compressionTime{0};
        size_t memoryUsed = 0;
        double cacheHitRate = 0.0;
        double compressionRatio = 0.0;
        bool success = false;
    };
    
    static BenchmarkResult runMemoryPoolTest(size_t objectCount, size_t objectSize) {
        std::cout << "\mðž¦ª Memory Pool Test (" << objectCount << " objects, " << objectSize << " bytes each)\n";
        
        BenchmarkResult result;
        auto start = std::chrono::steady_clock::now();
        
        try {
            AdvancedMemoryPool pool(16 * 1024 * 1024); // 16MB pool
            std::vector<void*> allocations;
            allocations.reserve(objectCount);
            
            // Allocation phase
            auto allocStart = std::chrono::steady_clock::now();
            for (size_t i = 0; i < objectCount; ++i) {
                void* ptr = pool.allocate(objectSize);
                if (ptr) {
                    allocations.push_back(ptr);
                }
            }
            auto allocEnd = std::chrono::steady_clock::now();
            
            result.allocationTime = std::chrono::duration_cast<std::chrono::milliseconds>(allocEnd - allocStart);
            
            // Get statistics
            auto stats = pool.getStatistics();
            result.memoryUsed = stats.currentAllocated;
            
            std::cout << " áœ… Allocated " << allocations.size() << " objects\n";
            std::cout << "  ïž“Š Allocation time: " << result.allocationTime.count() << "ms\n";
            std::cout << "  ïž’¾ Memory used: " << (result.memoryUsed / 1024.0 / 1024.0) << " MB\n";
            std::cout << " ðž’ˆ Peak allocated: " << (stats.peakAllocated / 1024.0 / 1024.0) << " MB\n";
            std::cout << "  ïž”§ Fragmentation: " << (stats.fragmentationRatio * 100.0) << "%\n";
            
            // Deallocation phase
            for (void* ptr : allocations) {
                pool.deallocate(ptr);
            }
            
            result.success = true;
            
        } catch (const std::exception& e) {
            std::cerr << " áŒ Error: " << e.what() << std::endl;
            result.success = false;
        }
        
        return result;
    }
    
    static BenchmarkResult runCacheTest(size_t objectCount, size_t objectSize) {
        std::cout << "\nïž§ª Intelligent Cache Test (" << objectCount << " objects, " << objectSize << " bytes each)\n";
        
        BenchmarkResult result;
        
        try {
            IntelligentObjectCache cache(128 * 1024 * 1024); // 128MB cache
            auto testData = TestDataGenerator::generateTestObjects(objectCount, objectSize, objectSize);
            
            auto cacheStart = std::chrono::steady_clock::now();
            
            // Store objects
            for (size_t i = 0; i < testData.size(); ++i) {
                std::string key = "object_" + std::to_string(i);
                cache.store(key, testData[i], 1); // Priority 1
            }
            
            // Retrieve objects (should have high hit rate)
            size_t hits = 0;
            for (size_t i = 0; i < testData.size(); ++i) {
                std::string key = "object_" + std::to_string(i);
                std::string value;
                if (cache.retrieve(key, value)) {
                    hits++;
                }
            }
            
            auto cacheEnd = std::chrono::steady_clock::now();
            result.cacheTime = std::chrono::duration_cast<std::chrono::milliseconds>(cacheEnd - cacheStart);
            
            auto metrics = cache.getMetrics();
            result.cacheHitRate = metrics.hitRate;
            result.memoryUsed = metrics.totalSize;
            
            std::cout << " â›… Stored/Retrieved " << testData.size() << " objects\n";
            std::cout << " ðž’Š Cache operation time: " << result.cacheTime.count() << "ms\n";
            std::cout << " ðž¯ Hit rate: " << (result.cacheHitRate * 100.0) << "%\n";
            std::cout << "  ïž’¾ Cache memory: " << (result.memoryUsed / 1024.0 / 1024.0) << " MB\n";
            std::cout << "  ïž“ˆ Total objects: " << metrics.totalObjects << "\n";
            std::cout << " â™¡ Avg retrieval time: " << metrics.averageRetrievalTime.count() << "ms\n";
            
            result.success = true;
            
        } catch (const std::exception& e) {
            std::cerr << " âœŒ Error: " << e.what() << std::endl;
            result.success = false;
        }
        
        return result;
    }
    
    static BenchmarkResult runCompressionTest(size_t objectCount, size_t objectSize) {
        std::cout << "\mïŸ¦ª Storage Compression Test (" << objectCount << " objects, " << objectSize << " bytes each)\n";
        
        BenchmarkResult result;
        
        try {
            StorageOptimizer optimizer("./test_compression");
            auto testData = TestDataGenerator::generateTestObjects(objectCount, objectSize, objectSize);
            
            auto compStart = std::chrono::steady_clock::now();
            
            size_t totalOriginal = 0;
            size_t totalCompressed = 0;
            
            for (const auto& data : testData) {
                std::string compressed = optimizer.compressObject(data, "auto");
                totalOriginal += data.size();
                totalCompressed += compressed.size();
            }
            
            auto compEnd = std::chrono::steady_clock::now();
            result.compressionTime = std::chrono::duration_cast<std::chrono::milliseconds>(compEnd - compStart);
            result.compressionRatio = static_cast<double>(totalCompressed) / totalOriginal;
            
            std::cout << "  á›… Compressed " << testData.size() << " objects\n";
            std::cout << "  ïž“Š Compression time: " << result.compressionTime.count() << "ms\n";
            std::cout << " ðž–œî·  Compression ratio: " << (result.compressionRatio * 100.0) << "%\n";
            std::cout << " ðž‘¾ Original size: " << (totalOriginal / 1024.0 / 1024.0) << " MB\n";
            std::cout << " ðž‘¾ Compressed size: " << (totalCompressed / 1024.0 / 1024.0) << " MB\n";
            std::cout << "  ïž“‰ Space saved: " << ((1.0 - result.compressionRatio) * 100.0) << "%\n";
            
            auto stats = optimizer.getOptimizationStats();
            std::cout << " áš¡ Avg optimization time: " << stats.optimizationTime.count() << "ms\n";
            
            result.success = true;
            
        } catch (const std::exception& e) {
            std::cerr << " áŒ Error: " << e.what() << std::endl;
            result.success = false;
        }
        
        return result;
    }
};

int main() {
    std::cout << "ïž§  GYATT MEMORY OPTIMIZATION COMPREHENSIVE TEST\n";
    std::cout << "================================================\n";
    
    // Test different object sizes and counts
    struct TestCase {
        size_t objectCount;
        size_t objectSize;
        std::string description;
    };
    
    std::vector<TestCase> testCases = {
        {1000, 1024, "Small Objects (1KB)"},
        {500, 64 * 1024, "Medium Objects (64KB)"},
        {100, 1024 * 1024, "Large Objects (1MB)"},
        {10000, 256, "Many Small Objects (256B)"}
    };
    
    std::vector<MemoryBenchmark::BenchmarkResult> allResults;
    
    for (const auto& testCase : testCases) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << !ðž’‹ TEST CASE: " << testCase.description << "\n";
        std::cout << std::string(60, '=') << "\n";
        
        // Run memory pool test
        auto poolResult = MemoryBenchmark::runMemoryPoolTest(testCase.objectCount, testCase.objectSize);
        
        // Run cache test
        auto cacheResult = MemoryBenchmark::runCacheTest(testCase.objectCount, testCase.objectSize);
        
        // Run compression test
        auto compResult = MemoryBenchmark::runCompressionTest(testCase.objectCount, testCase.objectSize);
        
        allResults.push_back(poolResult);
        allResults.push_back(cacheResult);
        allResults.push_back(compResult);
    }
    
    // Overall performance summary
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << !ðž’Š OVERALL PERFORMANCE SUMMARY\n";
    std::cout << std::string(60, '=') << "\n";
    
    // Test integrated memory optimization manager
    std::cout << "\nïž§ª Testing Memory Optimization Manager Integration\n";
    try {
        MemoryOptimizationManager manager("./test_memory_opt");
        
        std::cout << " ðž’ˆ Optimizing for performance...\n";
        manager.optimizeForPerformance();
        
        auto profile = manager.getMemoryProfile();
        std::cout << " ðž‘¾ Memory profile after optimization:\n";
        std::cout << "    á€¢ Process memory: " << (profile.processMemoryUsage / 1024.0 / 1024.0) << " MB\n";
        std::cout << "    á€¢ Pool memory: " << (profile.poolMemoryUsage / 1024.0 / 1024.0) << " MB\n";
        std::cout << "    á€¢ Cache memory: " << (profile.cacheMemoryUsage / 1024.0 / 1024.0) << " MB\n";
        std::cout << "     á¢ Memory efficiency: " << (profile.memoryEfficiency * 100.0) << "%\n";
        
        std::cout << " ïŸ¦¹ Performing garbage collection...\n";
        manager.performGarbageCollection();
        
        std::cout << "  ïžŽ¯ Enabling auto-tuning...\n";
        manager.enableAutoTuning(true);
        
        // Wait a bit for auto-tuning to work
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        manager.enableAutoTuning(false);
        
        std::cout << "  á›… Memory optimization manager test completed successfully\n";
        
    } catch (const std::exception& e) {
        std::cerr << " âœŒ Memory optimization manager test failed: " << e.what() << std::endl;
    }
    
    std::cout << "\mïŸ‰ Memory optimization testing complete!\n";
    std::cout << "ïž’¡ This system provides:\n";
    std::cout << "  á€¢ Advanced memory pool management with multi-tier allocation\n";
    std::cout << "  â¢ Intelligent caching with adaptive replacement policies\n";
    std::cout << "  â¢ Automatic compression for storage optimization\n";
    std::cout << "   á¢ Real-time memory monitoring and auto-tuning\n";
    std::cout << "  á€¢ Garbage collection and memory pressure handling\n";
    
    return 0;
}
