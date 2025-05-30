#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <future>
#include <memory>
#include <atomic>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <queue>

namespace gyatt {

// High-performance memory pool for object allocation
class MemoryPool {
public:
    MemoryPool(size_t blockSize = 64 * 1024); // 64KB blocks
    ~MemoryPool();
    
    void* allocate(size_t size);
    void deallocate(void* ptr);
    void reset();
    
private:
    struct Block {
        char* data;
        size_t size;
        size_t used;
    };
    
    std::vector<Block> blocks;
    size_t blockSize;
    std::mutex poolMutex;
    
    Block& getCurrentBlock();
    void allocateNewBlock();
};

// Parallel file processing engine
class ParallelProcessor {
public:
    ParallelProcessor(size_t numThreads = std::thread::hardware_concurrency());
    ~ParallelProcessor();
    
    // Parallel file hashing
    std::vector<std::future<std::string>> hashFilesParallel(const std::vector<std::string>& files);
    
    // Parallel file operations
    std::vector<std::future<bool>> processFilesParallel(
        const std::vector<std::string>& files,
        std::function<bool(const std::string&)> processor
    );
    
    // Parallel directory scanning
    std::future<std::vector<std::string>> scanDirectoryAsync(
        const std::string& path,
        const std::function<bool(const std::string&)>& filter = nullptr
    );
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex taskMutex;
    std::condition_variable condition;
    std::atomic<bool> stopping{false};
    
    void workerLoop();
    void enqueueTask(std::function<void()> task);
};

// Memory-mapped file operations for large files
class MemoryMappedFile {
public:
    MemoryMappedFile(const std::string& filepath);
    ~MemoryMappedFile();
    
    bool isValid() const { return data != nullptr; }
    const char* getData() const { return data; }
    size_t getSize() const { return size; }
    
    // Fast hash computation using memory mapping
    std::string computeHash() const;
    
    // Fast content comparison
    bool contentEquals(const MemoryMappedFile& other) const;
    
private:
    char* data = nullptr;
    size_t size = 0;
    int fd = -1;
    
    void cleanup();
};

// High-performance object cache
class ObjectCache {
public:
    ObjectCache(size_t maxSize = 1000);
    
    void put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    void clear();
    
    // Statistics
    size_t getHitRate() const;
    size_t getCacheSize() const { return cache.size(); }
    
private:
    struct CacheEntry {
        std::string value;
        std::chrono::steady_clock::time_point lastAccess;
        size_t accessCount = 0;
    };
    
    std::unordered_map<std::string, CacheEntry> cache;
    size_t maxSize;
    std::mutex cacheMutex;
    std::atomic<size_t> hits{0};
    std::atomic<size_t> misses{0};
    
    void evictOldest();
};

// Fast delta compression for objects
class DeltaCompression {
public:
    static std::string createDelta(const std::string& baseContent, const std::string& newContent);
    static std::string applyDelta(const std::string& baseContent, const std::string& delta);
    static bool isDeltaWorthwhile(const std::string& delta, const std::string& original);
};

// Optimized index operations
class FastIndex {
public:
    FastIndex(const std::string& repoPath);
    
    // Batch operations for better performance
    bool addFilesBatch(const std::vector<std::string>& files);
    bool removeFilesBatch(const std::vector<std::string>& files);
    
    // Parallel status checking
    std::map<std::string, std::string> getFileStatusesParallel();
    
    // Memory-efficient index loading
    bool loadIndexStreaming();
    bool saveIndexStreaming();
    
    // Optimized tree creation
    std::string createTreeOptimized();
    
private:
    std::string repoPath;
    std::string indexFile;
    std::unique_ptr<MemoryPool> memPool;
    std::unique_ptr<ParallelProcessor> processor;
    std::unique_ptr<ObjectCache> cache;
    
    // Lock-free operations where possible
    std::atomic<bool> indexDirty{false};
    mutable std::shared_mutex indexMutex;
    
    struct IndexEntry {
        std::string filepath;
        std::string hash;
        uint64_t modTime;
        size_t size;
        bool staged;
    };
    
    std::vector<IndexEntry> entries; // Use vector for better cache locality
    std::unordered_map<std::string, size_t> pathToIndex; // Fast lookups
};

// Performance monitoring and optimization engine
class PerformanceEngine {
public:
    PerformanceEngine(const std::string& repoPath);
    ~PerformanceEngine();
    
    // Enable/disable optimizations
    void enableParallelProcessing(bool enable = true);
    void enableObjectCaching(bool enable = true);
    void enableDeltaCompression(bool enable = true);
    void enableMemoryMapping(bool enable = true);
    void enableOptimizations(bool enable = true);
    
    // Performance metrics
    struct Metrics {
        std::chrono::milliseconds totalTime{0};
        size_t filesProcessed = 0;
        size_t bytesProcessed = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        double compressionRatio = 0.0;
        size_t parallelThreadsUsed = 0;
    };
    
    Metrics getMetrics() const { return metrics; }
    void resetMetrics();
    
    // Optimized core operations
    bool addFilesOptimized(const std::vector<std::string>& files);
    bool commitOptimized(const std::string& message, const std::string& author = "");
    std::map<std::string, std::string> statusOptimized();
    
    // Automatic performance tuning
    void autoTune();
    void benchmarkOperations();
    
private:
    std::string repoPath;
    std::unique_ptr<MemoryPool> memoryPool;
    std::unique_ptr<ParallelProcessor> parallelProcessor;
    std::unique_ptr<ObjectCache> objectCache;
    std::unique_ptr<FastIndex> fastIndex;
    
    bool parallelEnabled = true;
    bool cachingEnabled = true;
    bool deltaCompressionEnabled = true;
    bool memoryMappingEnabled = true;
    
    mutable Metrics metrics;
    mutable std::mutex metricsMutex;
    
    void updateMetrics(const std::chrono::milliseconds& time, size_t files, size_t bytes);
    std::string optimizeObjectStorage(const std::string& content, const std::string& type);
};

// Lock-free concurrent data structures for high performance
template<typename T>
class LockFreeQueue {
public:
    void push(T item);
    bool pop(T& item);
    bool empty() const;
    size_t size() const;
    
private:
    struct Node {
        std::atomic<T*> data{nullptr};
        std::atomic<Node*> next{nullptr};
    };
    
    std::atomic<Node*> head{new Node};
    std::atomic<Node*> tail{head.load()};
};

// SIMD-optimized operations for large data processing
class SIMDOptimizations {
public:
    // Fast memory comparison using SIMD
    static bool fastMemoryCompare(const void* a, const void* b, size_t size);
    
    // Optimized hash computation
    static std::string fastHash(const char* data, size_t size);
    
    // Parallel checksum calculation
    static uint64_t fastChecksum(const char* data, size_t size);
};

} // namespace gyatt
