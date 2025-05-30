#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>

// Forward declarations to avoid circular dependencies
namespace gyatt {
    class AdvancedCompressionEngine;
    class PackFileOptimizer;
    class CompressionAwareStorage;
    class IntegratedCompressionManager;
}

namespace gyatt {
    class AdvancedCompressionEngine;
    class IntegratedCompressionManager;
}

namespace gyatt {

// ============================================================================
// ADVANCED MEMORY POOL SYSTEM WITH COMPRESSION INTEGRATION
// ============================================================================

class AdvancedMemoryPool {
public:
    enum class PoolType {
        SMALL_OBJECTS,     // < 1KB
        MEDIUM_OBJECTS,    // 1KB - 64KB
        LARGE_OBJECTS,     // 64KB - 1MB
        HUGE_OBJECTS       // > 1MB
    };
    
    struct PoolStatistics {
        size_t totalAllocations = 0;
        size_t totalDeallocations = 0;
        size_t currentAllocated = 0;
        size_t peakAllocated = 0;
        size_t totalBytesAllocated = 0;
        size_t wastedBytes = 0;
        double fragmentationRatio = 0.0;
        std::chrono::milliseconds averageAllocTime{0};
        
        // Compression integration stats
        size_t compressedAllocations = 0;
        size_t totalBytesCompressed = 0;
        size_t totalBytesSaved = 0;
        double compressionRatio = 0.0;
    };
    
    AdvancedMemoryPool(size_t initialSize = 16 * 1024 * 1024); // 16MB
    ~AdvancedMemoryPool();
    
    // Smart allocation based on size
    void* allocate(size_t size);
    void deallocate(void* ptr);
    
    // Pre-allocation for known patterns
    void preAllocateObjects(PoolType type, size_t count);
    
    // Compression-aware allocation
    void* allocateCompressed(size_t size, bool enableCompression = true);
    void deallocateCompressed(void* ptr);
    
    // Integration with compression engine
    void setCompressionEngine(std::shared_ptr<AdvancedCompressionEngine> engine);
    
    // Memory compaction and defragmentation
    void compactMemory();
    void defragment();
    
    // Statistics and monitoring
    PoolStatistics getStatistics() const;
    void resetStatistics();
    
    // Memory pressure handling
    void handleMemoryPressure();
    size_t getMemoryUsage() const;
    
private:
    struct MemoryBlock {
        char* data;
        size_t size;
        size_t used;
        bool isActive;
        std::chrono::steady_clock::time_point lastAccess;
    };
    
    struct FreeBlock {
        void* ptr;
        size_t size;
        PoolType type;
    };
    
    std::vector<MemoryBlock> memoryBlocks;
    std::map<PoolType, std::queue<FreeBlock>> freeLists;
    std::unordered_map<void*, size_t> allocatedSizes;
    
    mutable std::mutex poolMutex;
    PoolStatistics stats;
    std::shared_ptr<AdvancedCompressionEngine> compressionEngine;
    
    PoolType getOptimalPoolType(size_t size) const;
    void* allocateFromPool(PoolType type, size_t size);
    void returnToPool(void* ptr, size_t size);
    void allocateNewBlock(size_t minSize);
    void cleanupUnusedBlocks();
};

// ============================================================================
// INTELLIGENT OBJECT CACHE SYSTEM
// ============================================================================

class IntelligentObjectCache {
public:
    enum class CachePolicy {
        LRU,           // Least Recently Used
        LFU,           // Least Frequently Used
        ADAPTIVE,      // Adaptive Replacement Cache (ARC)
        SIZE_AWARE     // Size-aware eviction
    };
    
    struct CacheMetrics {
        std::atomic<size_t> hits{0};
        std::atomic<size_t> misses{0};
        std::atomic<size_t> evictions{0};
        std::atomic<size_t> totalObjects{0};
        std::atomic<size_t> totalSize{0};
        double hitRate = 0.0;
        double memoryEfficiency = 0.0;
        std::chrono::milliseconds averageRetrievalTime{0};
        
        // Copy constructor and assignment operator
        CacheMetrics() = default;
        CacheMetrics(const CacheMetrics& other) 
            : hits(other.hits.load())
            , misses(other.misses.load())
            , evictions(other.evictions.load())
            , totalObjects(other.totalObjects.load())
            , totalSize(other.totalSize.load())
            , hitRate(other.hitRate)
            , memoryEfficiency(other.memoryEfficiency)
            , averageRetrievalTime(other.averageRetrievalTime) {}
            
        CacheMetrics& operator=(const CacheMetrics& other) {
            if (this != &other) {
                hits = other.hits.load();
                misses = other.misses.load();
                evictions = other.evictions.load();
                totalObjects = other.totalObjects.load();
                totalSize = other.totalSize.load();
                hitRate = other.hitRate;
                memoryEfficiency = other.memoryEfficiency;
                averageRetrievalTime = other.averageRetrievalTime;
            }
            return *this;
        }
    };
    
    IntelligentObjectCache(size_t maxMemory = 256 * 1024 * 1024, // 256MB
                          CachePolicy policy = CachePolicy::ADAPTIVE);
    ~IntelligentObjectCache();
    
    // Cache operations
    bool store(const std::string& key, const std::string& value, 
              int priority = 0, std::chrono::minutes ttl = std::chrono::minutes(60));
    bool retrieve(const std::string& key, std::string& value);
    bool contains(const std::string& key) const;
    void remove(const std::string& key);
    void clear();
    
    // Bulk operations
    std::map<std::string, std::string> retrieveBatch(const std::vector<std::string>& keys);
    void storeBatch(const std::map<std::string, std::string>& items);
    
    // Cache warming and preloading
    void warmCache(const std::vector<std::string>& commonKeys);
    void preloadFromIndex();
    
    // Statistics and monitoring
    CacheMetrics getMetrics() const;
    void resetMetrics();
    
    // Memory management
    void optimizeMemoryUsage();
    void setMemoryLimit(size_t maxMemory);
    
private:
    struct CacheEntry {
        std::string value;
        int priority;
        size_t accessCount;
        std::chrono::steady_clock::time_point lastAccess;
        std::chrono::steady_clock::time_point created;
        std::chrono::minutes ttl;
        size_t size;
    };
    
    std::unordered_map<std::string, CacheEntry> cache;
    size_t maxMemorySize;
    size_t currentMemoryUsage;
    CachePolicy policy;
    mutable std::mutex cacheMutex;
    CacheMetrics metrics;
    
    // Policy implementations
    void evictLRU();
    void evictLFU();
    void evictAdaptive();
    void evictSizeAware();
    
    bool shouldEvict(const CacheEntry& entry) const;
    void updateMetrics();
    void cleanupExpiredEntries();
};

// ============================================================================
// STORAGE OPTIMIZATION SYSTEM
// ============================================================================

class StorageOptimizer {
public:
    struct OptimizationResult {
        size_t originalSize;
        size_t optimizedSize;
        double compressionRatio;
        std::chrono::milliseconds optimizationTime;
        std::string algorithm;
        bool success;
    };
    
    StorageOptimizer(const std::string& repoPath);
    
    // Compression strategies
    std::string compressObject(const std::string& content, const std::string& type = "auto");
    std::string decompressObject(const std::string& compressed);
    
    // Delta compression for similar objects
    std::string createDelta(const std::string& base, const std::string& target);
    std::string applyDelta(const std::string& base, const std::string& delta);
    
    // Pack file generation for efficient storage
    bool createPackFile(const std::vector<std::string>& objectHashes);
    bool extractFromPackFile(const std::string& hash, std::string& content);
    
    // Storage layout optimization
    void optimizeStorageLayout();
    void defragmentObjectStore();
    
    // Statistics
    OptimizationResult getOptimizationStats() const;
    
private:
    std::string repoPath;
    std::string objectsPath;
    std::string packsPath;
    
    mutable std::mutex optimizerMutex;
    OptimizationResult lastResult;
    
    // Compression algorithms
    std::string gzipCompress(const std::string& data);
    std::string gzipDecompress(const std::string& data);
    std::string lz4Compress(const std::string& data);
    std::string lz4Decompress(const std::string& data);
    
    // Delta compression helpers
    std::vector<uint8_t> computeBinaryDelta(const std::string& base, const std::string& target);
    std::string applyBinaryDelta(const std::string& base, const std::vector<uint8_t>& delta);
    
    // Pack file management
    struct PackEntry {
        std::string hash;
        size_t offset;
        size_t size;
        std::string type;
    };
    
    std::map<std::string, std::vector<PackEntry>> packIndex;
    void buildPackIndex();
    void writePackIndex();
    void readPackIndex();
};

// ============================================================================
// MEMORY OPTIMIZATION MANAGER WITH COMPRESSION INTEGRATION
// ============================================================================

class MemoryOptimizationManager {
public:
    struct MemoryProfile {
        size_t totalSystemMemory;
        size_t availableMemory;
        size_t processMemoryUsage;
        size_t poolMemoryUsage;
        size_t cacheMemoryUsage;
        double memoryEfficiency;
        std::chrono::milliseconds gcTime{0};
        
        // Compression integration metrics
        size_t compressedDataSize = 0;
        size_t uncompressedDataSize = 0;
        double overallCompressionRatio = 0.0;
        size_t totalSpaceSaved = 0;
        std::chrono::milliseconds compressionTime{0};
    };
    
    MemoryOptimizationManager(const std::string& repoPath);
    ~MemoryOptimizationManager();
    
    // High-level optimization interface
    void enableOptimization(bool enable = true);
    void optimizeForPerformance();
    void optimizeForMemory();
    void optimizeForBatch();
    
    // Component access
    AdvancedMemoryPool& getMemoryPool() { return memoryPool; }
    IntelligentObjectCache& getObjectCache() { return objectCache; }
    StorageOptimizer& getStorageOptimizer() { return storageOptimizer; }
    
        // Monitoring and statistics
    MemoryProfile getMemoryProfile() const;
    void performGarbageCollection();
    void runMemoryDiagnostics();
    
    // Auto-tuning
    void enableAutoTuning(bool enable = true);
    void autoTunePerformance();
    
    // Compression integration
    void enableCompressionIntegration(bool enable = true);
    void setCompressionEngine(std::shared_ptr<AdvancedCompressionEngine> engine);
    void setCompressionManager(std::shared_ptr<IntegratedCompressionManager> manager);
    bool optimizeWithCompression();
    void printCompressionIntegratedStats();
    
private:
    std::string repoPath;
    AdvancedMemoryPool memoryPool;
    IntelligentObjectCache objectCache;
    StorageOptimizer storageOptimizer;
    
    // Compression integration
    std::shared_ptr<AdvancedCompressionEngine> compressionEngine;
    std::shared_ptr<IntegratedCompressionManager> compressionManager;
    bool compressionEnabled = false;
    
    bool autoTuningEnabled = false;
    std::thread autoTuningThread;
    std::atomic<bool> shouldStopAutoTuning{false};
    
    void autoTuningLoop();
    void adjustPoolSizes();
    void adjustCachePolicy();
    void monitorMemoryPressure();
    
    // System memory monitoring
    size_t getSystemMemoryUsage() const;
    size_t getAvailableMemory() const;
    void handleMemoryPressure();
};

} // namespace gyatt
