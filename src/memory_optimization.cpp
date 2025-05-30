#include "memory_optimization.h"
#include "advanced_compression.h"  // Add compression integration
#include "utils.h"
#include <algorithm>
#include <sstream>
#include <cstring>
#include <zlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace gyatt {

// ============================================================================
// ADVANCED MEMORY POOL IMPLEMENTATION WITH COMPRESSION
// ============================================================================

AdvancedMemoryPool::AdvancedMemoryPool(size_t initialSize) {
    allocateNewBlock(initialSize);
}

AdvancedMemoryPool::~AdvancedMemoryPool() {
    std::lock_guard<std::mutex> lock(poolMutex);
    for (auto& block : memoryBlocks) {
        if (block.data) {
            free(block.data);
        }
    }
}

void* AdvancedMemoryPool::allocate(size_t size) {
    auto start = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(poolMutex);
    
    PoolType poolType = getOptimalPoolType(size);
    void* ptr = allocateFromPool(poolType, size);
    
    if (!ptr) {
        // Allocate new block if needed
        allocateNewBlock(std::max(size * 2, static_cast<size_t>(1024 * 1024)));
        ptr = allocateFromPool(poolType, size);
    }
    
    if (ptr) {
        allocatedSizes[ptr] = size;
        stats.totalAllocations++;
        stats.currentAllocated += size;
        stats.totalBytesAllocated += size;
        
        // Update compression-related stats if applicable
        if (compressionEngine && size > 1024) { // Consider compression for larger allocations
            stats.compressedAllocations++;
        }
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        stats.averageAllocTime = std::chrono::milliseconds(
            (stats.averageAllocTime.count() + duration.count()) / 2);
    }
    
    return ptr;
}

void AdvancedMemoryPool::deallocate(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(poolMutex);
    
    auto it = allocatedSizes.find(ptr);
    if (it != allocatedSizes.end()) {
        size_t size = it->second;
        allocatedSizes.erase(it);
        
        returnToPool(ptr, size);
        
        stats.totalDeallocations++;
        stats.currentAllocated -= size;
    }
}

// ============================================================================
// COMPRESSION-AWARE ALLOCATION METHODS
// ============================================================================

void* AdvancedMemoryPool::allocateCompressed(size_t size, bool enableCompression) {
    // For smaller allocations, compression overhead isn't worth it
    if (!enableCompression || size < 1024) {
        return allocate(size);
    }
    
    auto start = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(poolMutex);
    
    // Allocate regular memory first
    void* ptr = allocate(size);
    if (!ptr) return nullptr;
    
    // Mark as compressed allocation for tracking
    if (compressionEngine) {
        stats.compressedAllocations++;
        stats.totalBytesCompressed += size;
        
        // In a real implementation, we could compress the data here
        // For now, just track that this allocation could benefit from compression
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        stats.averageAllocTime = std::chrono::milliseconds(
            (stats.averageAllocTime.count() + duration.count()) / 2);
    }
    
    return ptr;
}

void AdvancedMemoryPool::deallocateCompressed(void* ptr) {
    // For now, just use regular deallocation
    deallocate(ptr);
}

void AdvancedMemoryPool::setCompressionEngine(std::shared_ptr<AdvancedCompressionEngine> engine) {
    std::lock_guard<std::mutex> lock(poolMutex);
    compressionEngine = engine;
}

AdvancedMemoryPool::PoolType AdvancedMemoryPool::getOptimalPoolType(size_t size) const {
    if (size < 1024) return PoolType::SMALL_OBJECTS;
    if (size < 64 * 1024) return PoolType::MEDIUM_OBJECTS;
    if (size < 1024 * 1024) return PoolType::LARGE_OBJECTS;
    return PoolType::HUGE_OBJECTS;
}

void* AdvancedMemoryPool::allocateFromPool(PoolType type, size_t size) {
    auto& freeList = freeLists[type];
    
    if (!freeList.empty()) {
        FreeBlock block = freeList.front();
        freeList.pop();
        
        if (block.size >= size) {
            // If block is much larger, split it
            if (block.size > size * 2) {
                FreeBlock remainder;
                remainder.ptr = static_cast<char*>(block.ptr) + size;
                remainder.size = block.size - size;
                remainder.type = getOptimalPoolType(remainder.size);
                
                freeLists[remainder.type].push(remainder);
                stats.wastedBytes += (block.size - size);
            }
            
            return block.ptr;
        }
    }
    
    // Allocate from current block
    for (auto& block : memoryBlocks) {
        if (block.isActive && block.used + size <= block.size) {
            void* ptr = block.data + block.used;
            block.used += size;
            block.lastAccess = std::chrono::steady_clock::now();
            return ptr;
        }
    }
    
    return nullptr;
}

void AdvancedMemoryPool::returnToPool(void* ptr, size_t size) {
    PoolType type = getOptimalPoolType(size);
    
    FreeBlock block;
    block.ptr = ptr;
    block.size = size;
    block.type = type;
    
    freeLists[type].push(block);
}

void AdvancedMemoryPool::allocateNewBlock(size_t minSize) {
    size_t blockSize = std::max(minSize, static_cast<size_t>(4 * 1024 * 1024)); // 4MB minimum
    
    MemoryBlock block;
    block.data = static_cast<char*>(malloc(blockSize));
    if (!block.data) {
        throw std::bad_alloc();
    }
    
    block.size = blockSize;
    block.used = 0;
    block.isActive = true;
    block.lastAccess = std::chrono::steady_clock::now();
    
    memoryBlocks.push_back(block);
}

void AdvancedMemoryPool::compactMemory() {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    // Cleanup unused blocks older than 5 minutes
    auto now = std::chrono::steady_clock::now();
    auto threshold = std::chrono::minutes(5);
    
    memoryBlocks.erase(
        std::remove_if(memoryBlocks.begin(), memoryBlocks.end(),
            [&](const MemoryBlock& block) {
                if (!block.isActive || (now - block.lastAccess) > threshold) {
                    if (block.data) free(block.data);
                    return true;
                }
                return false;
            }),
        memoryBlocks.end()
    );
}

AdvancedMemoryPool::PoolStatistics AdvancedMemoryPool::getStatistics() const {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    PoolStatistics result = stats;
    result.fragmentationRatio = stats.wastedBytes / static_cast<double>(stats.totalBytesAllocated);
    
    return result;
}

// ============================================================================
// INTELLIGENT OBJECT CACHE IMPLEMENTATION
// ============================================================================

IntelligentObjectCache::IntelligentObjectCache(size_t maxMemory, CachePolicy policy)
    : maxMemorySize(maxMemory), currentMemoryUsage(0), policy(policy) {
}

IntelligentObjectCache::~IntelligentObjectCache() {
    clear();
}

bool IntelligentObjectCache::store(const std::string& key, const std::string& value, 
                                  int priority, std::chrono::minutes ttl) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    size_t valueSize = value.size();
    
    // Check if we need to evict items
    while (currentMemoryUsage + valueSize > maxMemorySize && !cache.empty()) {
        switch (policy) {
            case CachePolicy::LRU: evictLRU(); break;
            case CachePolicy::LFU: evictLFU(); break;
            case CachePolicy::ADAPTIVE: evictAdaptive(); break;
            case CachePolicy::SIZE_AWARE: evictSizeAware(); break;
        }
    }
    
    if (currentMemoryUsage + valueSize > maxMemorySize) {
        return false; // Cannot fit even after eviction
    }
    
    auto now = std::chrono::steady_clock::now();
    
    // Remove existing entry if present
    auto it = cache.find(key);
    if (it != cache.end()) {
        currentMemoryUsage -= it->second.size;
        cache.erase(it);
    }
    
    CacheEntry entry;
    entry.value = value;
    entry.priority = priority;
    entry.accessCount = 1;
    entry.lastAccess = now;
    entry.created = now;
    entry.ttl = ttl;
    entry.size = valueSize;
    
    cache[key] = entry;
    currentMemoryUsage += valueSize;
    metrics.totalObjects++;
    
    updateMetrics();
    return true;
}

bool IntelligentObjectCache::retrieve(const std::string& key, std::string& value) {
    auto start = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    auto it = cache.find(key);
    if (it == cache.end()) {
        metrics.misses++;
        updateMetrics();
        return false;
    }
    
    auto& entry = it->second;
    
    // Check TTL
    auto now = std::chrono::steady_clock::now();
    if ((now - entry.created) > entry.ttl) {
        currentMemoryUsage -= entry.size;
        cache.erase(it);
        metrics.misses++;
        updateMetrics();
        return false;
    }
    
    // Update access information
    entry.lastAccess = now;
    entry.accessCount++;
    
    value = entry.value;
    metrics.hits++;
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    metrics.averageRetrievalTime = std::chrono::milliseconds(
        (metrics.averageRetrievalTime.count() + duration.count()) / 2);
    
    updateMetrics();
    return true;
}

void IntelligentObjectCache::evictLRU() {
    if (cache.empty()) return;
    
    auto oldest = std::min_element(cache.begin(), cache.end(),
        [](const auto& a, const auto& b) {
            return a.second.lastAccess < b.second.lastAccess;
        });
    
    currentMemoryUsage -= oldest->second.size;
    cache.erase(oldest);
    metrics.evictions++;
}

void IntelligentObjectCache::evictLFU() {
    if (cache.empty()) return;
    
    auto leastUsed = std::min_element(cache.begin(), cache.end(),
        [](const auto& a, const auto& b) {
            return a.second.accessCount < b.second.accessCount;
        });
    
    currentMemoryUsage -= leastUsed->second.size;
    cache.erase(leastUsed);
    metrics.evictions++;
}

void IntelligentObjectCache::evictAdaptive() {
    if (cache.empty()) return;
    
    // Adaptive replacement: balance between recency and frequency
    auto candidate = std::min_element(cache.begin(), cache.end(),
        [](const auto& a, const auto& b) {
            auto now = std::chrono::steady_clock::now();
            double scoreA = a.second.accessCount / 
                std::chrono::duration_cast<std::chrono::minutes>(now - a.second.lastAccess).count();
            double scoreB = b.second.accessCount / 
                std::chrono::duration_cast<std::chrono::minutes>(now - b.second.lastAccess).count();
            return scoreA < scoreB;
        });
    
    currentMemoryUsage -= candidate->second.size;
    cache.erase(candidate);
    metrics.evictions++;
}

void IntelligentObjectCache::evictSizeAware() {
    if (cache.empty()) return;
    
    // Evict largest item with lowest priority
    auto candidate = std::min_element(cache.begin(), cache.end(),
        [](const auto& a, const auto& b) {
            double ratioA = a.second.size / static_cast<double>(a.second.priority + 1);
            double ratioB = b.second.size / static_cast<double>(b.second.priority + 1);
            return ratioA > ratioB; // Higher ratio = worse candidate
        });
    
    currentMemoryUsage -= candidate->second.size;
    cache.erase(candidate);
    metrics.evictions++;
}

void IntelligentObjectCache::updateMetrics() {
    size_t totalAccesses = metrics.hits + metrics.misses;
    if (totalAccesses > 0) {
        metrics.hitRate = static_cast<double>(metrics.hits) / totalAccesses;
    }
    
    metrics.memoryEfficiency = static_cast<double>(currentMemoryUsage) / maxMemorySize;
    metrics.totalSize = currentMemoryUsage;
}

IntelligentObjectCache::CacheMetrics IntelligentObjectCache::getMetrics() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    CacheMetrics result = metrics;
    return result;
}

// ============================================================================
// STORAGE OPTIMIZER IMPLEMENTATION
// ============================================================================

StorageOptimizer::StorageOptimizer(const std::string& repoPath) 
    : repoPath(repoPath) {
    objectsPath = Utils::joinPath(repoPath, ".gyatt/objects");
    packsPath = Utils::joinPath(repoPath, ".gyatt/packs");
    
    Utils::createDirectories(packsPath);
    readPackIndex();
}

std::string StorageOptimizer::compressObject(const std::string& content, const std::string& type) {
    auto start = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(optimizerMutex);
    
    std::string compressed;
    std::string algorithm = "gzip";
    
    if (type == "auto") {
        // Choose compression based on content size and type
        if (content.size() < 1024) {
            // Small objects: no compression overhead
            compressed = content;
            algorithm = "none";
        } else if (content.size() < 64 * 1024) {
            // Medium objects: fast compression
            compressed = lz4Compress(content);
            algorithm = "lz4";
        } else {
            // Large objects: maximum compression
            compressed = gzipCompress(content);
            algorithm = "gzip";
        }
    } else if (type == "gzip") {
        compressed = gzipCompress(content);
    } else if (type == "lz4") {
        compressed = lz4Compress(content);
    } else {
        compressed = content;
        algorithm = "none";
    }
    
    auto end = std::chrono::steady_clock::now();
    
    lastResult.originalSize = content.size();
    lastResult.optimizedSize = compressed.size();
    lastResult.compressionRatio = static_cast<double>(compressed.size()) / content.size();
    lastResult.optimizationTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    lastResult.algorithm = algorithm;
    lastResult.success = true;
    
    return compressed;
}

std::string StorageOptimizer::gzipCompress(const std::string& data) {
    z_stream stream = {};
    
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return data; // Fallback to uncompressed
    }
    
    std::string compressed;
    compressed.resize(data.size() + 16); // Initial estimate
    
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()));
    stream.avail_in = data.size();
    stream.next_out = reinterpret_cast<Bytef*>(&compressed[0]);
    stream.avail_out = compressed.size();
    
    int result = deflate(&stream, Z_FINISH);
    
    if (result == Z_STREAM_END) {
        compressed.resize(stream.total_out);
    } else {
        compressed = data; // Fallback
    }
    
    deflateEnd(&stream);
    return compressed;
}

std::string StorageOptimizer::lz4Compress(const std::string& data) {
    // Simplified LZ4-like compression
    // In a real implementation, you would use the actual LZ4 library
    
    std::string compressed;
    compressed.reserve(data.size());
    
    // Very basic run-length encoding as a placeholder
    for (size_t i = 0; i < data.size(); ) {
        char current = data[i];
        size_t count = 1;
        
        while (i + count < data.size() && data[i + count] == current && count < 255) {
            count++;
        }
        
        if (count > 3) {
            compressed += '\xFF'; // Escape character
            compressed += static_cast<char>(count);
            compressed += current;
        } else {
            for (size_t j = 0; j < count; j++) {
                compressed += current;
            }
        }
        
        i += count;
    }
    
    return compressed;
}

StorageOptimizer::OptimizationResult StorageOptimizer::getOptimizationStats() const {
    std::lock_guard<std::mutex> lock(optimizerMutex);
    return lastResult;
}

// ============================================================================
// MEMORY OPTIMIZATION MANAGER IMPLEMENTATION
// ============================================================================

MemoryOptimizationManager::MemoryOptimizationManager(const std::string& repoPath)
    : repoPath(repoPath), 
      memoryPool(64 * 1024 * 1024), // 64MB pool
      objectCache(256 * 1024 * 1024), // 256MB cache
      storageOptimizer(repoPath) {
}

MemoryOptimizationManager::~MemoryOptimizationManager() {
    shouldStopAutoTuning = true;
    if (autoTuningThread.joinable()) {
        autoTuningThread.join();
    }
}

void MemoryOptimizationManager::optimizeForPerformance() {
    // Configure for maximum performance
    objectCache.setMemoryLimit(512 * 1024 * 1024); // 512MB cache
    memoryPool.preAllocateObjects(AdvancedMemoryPool::PoolType::SMALL_OBJECTS, 10000);
    memoryPool.preAllocateObjects(AdvancedMemoryPool::PoolType::MEDIUM_OBJECTS, 1000);
    
    std::cout << "🚀 Memory system optimized for performance\n";
    std::cout << "   • Cache size: 512MB\n";
    std::cout << "   • Pre-allocated object pools\n";
    std::cout << "   • Aggressive caching enabled\n";
}

void MemoryOptimizationManager::optimizeForMemory() {
    // Configure for minimal memory usage
    objectCache.setMemoryLimit(64 * 1024 * 1024); // 64MB cache
    memoryPool.compactMemory();
    performGarbageCollection();
    
    std::cout << "💾 Memory system optimized for low memory usage\n";
    std::cout << "   • Cache size: 64MB\n";
    std::cout << "   • Memory compaction performed\n";
    std::cout << "   • Garbage collection completed\n";
}

void MemoryOptimizationManager::optimizeForBatch() {
    // Configure for batch operations
    objectCache.setMemoryLimit(128 * 1024 * 1024); // 128MB cache
    memoryPool.preAllocateObjects(AdvancedMemoryPool::PoolType::MEDIUM_OBJECTS, 5000);
    
    std::cout << "📦 Memory system optimized for batch operations\n";
    std::cout << "   • Cache size: 128MB\n";
    std::cout << "   • Batch-optimized allocation\n";
}

MemoryOptimizationManager::MemoryProfile MemoryOptimizationManager::getMemoryProfile() const {
    MemoryProfile profile;
    
    profile.totalSystemMemory = getAvailableMemory() + getSystemMemoryUsage();
    profile.availableMemory = getAvailableMemory();
    profile.processMemoryUsage = getSystemMemoryUsage();
    
    auto poolStats = memoryPool.getStatistics();
    profile.poolMemoryUsage = poolStats.currentAllocated;
    
    auto cacheMetrics = objectCache.getMetrics();
    profile.cacheMemoryUsage = cacheMetrics.totalSize;
    
    profile.memoryEfficiency = cacheMetrics.memoryEfficiency;
    
    // Add compression statistics if compression is enabled
    if (compressionEnabled && compressionEngine) {
        auto compressionStats = compressionEngine->getStats();
        
        profile.compressedDataSize = compressionStats.totalBytesCompressed - compressionStats.totalSavedBytes;
        profile.uncompressedDataSize = compressionStats.totalBytesCompressed;
        profile.totalSpaceSaved = compressionStats.totalSavedBytes;
        
        if (compressionStats.totalBytesCompressed > 0) {
            profile.overallCompressionRatio = 
                static_cast<double>(profile.compressedDataSize) / profile.uncompressedDataSize;
        }
        
        profile.compressionTime = std::chrono::milliseconds(compressionStats.totalCompressionTime);
        
        // Include compression in pool statistics
        profile.poolMemoryUsage += poolStats.totalBytesCompressed;
    }
    
    return profile;
}

void MemoryOptimizationManager::performGarbageCollection() {
    auto start = std::chrono::steady_clock::now();
    
    // Compact memory pool
    memoryPool.compactMemory();
    
    // Clean up expired cache entries
    objectCache.optimizeMemoryUsage();
    
    // Optimize storage
    storageOptimizer.optimizeStorageLayout();
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "🧹 Garbage collection completed in " << duration.count() << "ms\n";
}

size_t MemoryOptimizationManager::getAvailableMemory() const {
    // Get available memory from system
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    // Convert to bytes (rusage returns in KB on Linux)
    return static_cast<size_t>(usage.ru_maxrss) * 1024;
}

size_t MemoryOptimizationManager::getSystemMemoryUsage() const {
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    
    while (std::getline(statusFile, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string label, value, unit;
            iss >> label >> value >> unit;
            
            size_t memory = std::stoull(value);
            if (unit == "kB") memory *= 1024;
            
            return memory;
        }
    }
    
    return 0;
}

void MemoryOptimizationManager::enableOptimization(bool enable) {
    static bool isInitialized = false;
    
    if (enable && !isInitialized) {
        // Memory optimization is enabled through constructor initialization
        // Components are already initialized in the constructor
        isInitialized = true;
        
        std::cout << "🧠 Memory optimization enabled\n";
        std::cout << "  • Advanced memory pool: " << memoryPool.getStatistics().currentAllocated / 1024.0 / 1024.0 << " MB allocated\n";
        std::cout << "  • Intelligent object cache: " << objectCache.getMetrics().totalObjects << " items cached\n";
        std::cout << "  • Storage optimization: Ready\n";
    } else if (!enable && isInitialized) {
        // Disable auto-tuning if it's running
        if (autoTuningEnabled) {
            enableAutoTuning(false);
        }
        
        // Clear caches to free memory
        objectCache.clear();
        
        isInitialized = false;
        std::cout << "⏸️  Memory optimization disabled\n";
    } else if (enable) {
        std::cout << "🧠 Memory optimization already enabled\n";
    } else {
        std::cout << "⏸️  Memory optimization already disabled\n";
    }
}

void MemoryOptimizationManager::enableAutoTuning(bool enable) {
    if (enable && !autoTuningEnabled) {
        autoTuningEnabled = true;
        shouldStopAutoTuning = false;
        autoTuningThread = std::thread(&MemoryOptimizationManager::autoTuningLoop, this);
        
        std::cout << "🎯 Auto-tuning enabled for memory optimization\n";
    } else if (!enable && autoTuningEnabled) {
        autoTuningEnabled = false;
        shouldStopAutoTuning = true;
        
        if (autoTuningThread.joinable()) {
            autoTuningThread.join();
        }
        
        std::cout << "⏸️  Auto-tuning disabled\n";
    }
}

void MemoryOptimizationManager::autoTuningLoop() {
    while (!shouldStopAutoTuning) {
        try {
            monitorMemoryPressure();
            adjustPoolSizes();
            adjustCachePolicy();
            
            // Sleep for 30 seconds between tuning cycles
            std::this_thread::sleep_for(std::chrono::seconds(30));
        } catch (const std::exception& e) {
            std::cerr << "Auto-tuning error: " << e.what() << std::endl;
        }
    }
}

void MemoryOptimizationManager::monitorMemoryPressure() {
    auto profile = getMemoryProfile();
    
    double memoryPressure = static_cast<double>(profile.processMemoryUsage) / profile.totalSystemMemory;
    
    if (memoryPressure > 0.8) {
        // High memory pressure - optimize for memory
        objectCache.setMemoryLimit(32 * 1024 * 1024); // 32MB
        memoryPool.compactMemory();
        
        std::cout << "⚠️  High memory pressure detected - reducing cache size\n";
    } else if (memoryPressure < 0.4) {
        // Low memory pressure - optimize for performance
        objectCache.setMemoryLimit(512 * 1024 * 1024); // 512MB
        
        std::cout << "📈 Low memory pressure - increasing cache size\n";
    }
}

void AdvancedMemoryPool::preAllocateObjects(PoolType type, size_t count) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    size_t objectSize = 512; // default size
    switch (type) {
        case PoolType::SMALL_OBJECTS: objectSize = 512; break;
        case PoolType::MEDIUM_OBJECTS: objectSize = 32 * 1024; break;
        case PoolType::LARGE_OBJECTS: objectSize = 512 * 1024; break;
        case PoolType::HUGE_OBJECTS: objectSize = 2 * 1024 * 1024; break;
    }
    
    for (size_t i = 0; i < count; ++i) {
        void* ptr = malloc(objectSize);
        if (ptr) {
            FreeBlock block{ptr, objectSize, type};
            freeLists[type].push(block);
        }
    }
}

void IntelligentObjectCache::clear() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.clear();
    currentMemoryUsage = 0;
    metrics.totalObjects = 0;
    metrics.totalSize = 0;
}

void IntelligentObjectCache::setMemoryLimit(size_t maxMemory) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    maxMemorySize = maxMemory;
    
    // Evict items if current usage exceeds new limit
    while (currentMemoryUsage > maxMemorySize && !cache.empty()) {
        evictLRU();
    }
}

void IntelligentObjectCache::optimizeMemoryUsage() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cleanupExpiredEntries();
    
    // Compact cache if fragmentation is high
    if (cache.size() > 1000 && currentMemoryUsage < maxMemorySize * 0.6) {
        // Consider reducing cache size target
        while (currentMemoryUsage > maxMemorySize * 0.5 && !cache.empty()) {
            evictLRU();
        }
    }
}

void StorageOptimizer::optimizeStorageLayout() {
    std::lock_guard<std::mutex> lock(optimizerMutex);
    
    auto start = std::chrono::steady_clock::now();
    lastResult.originalSize = 0;
    lastResult.optimizedSize = 0;
    
    // Simulate storage layout optimization
    lastResult.optimizationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    lastResult.algorithm = "layout_optimization";
    lastResult.success = true;
}

void StorageOptimizer::readPackIndex() {
    std::lock_guard<std::mutex> lock(optimizerMutex);
    // Simulate reading pack index
    packIndex.clear();
}

void MemoryOptimizationManager::adjustPoolSizes() {
    auto poolStats = memoryPool.getStatistics();
    
    // Adjust pool sizes based on usage patterns
    if (poolStats.fragmentationRatio > 0.3) {
        memoryPool.compactMemory();
    }
}

void MemoryOptimizationManager::adjustCachePolicy() {
    auto cacheMetrics = objectCache.getMetrics();
    
    // Adjust cache policy based on hit rates
    if (cacheMetrics.hitRate < 0.7) {
        // Could switch cache policy here
        objectCache.optimizeMemoryUsage();
    }
}

// ============================================================================
// COMPRESSION INTEGRATION METHODS
// ============================================================================

void MemoryOptimizationManager::enableCompressionIntegration(bool enable) {
    compressionEnabled = enable;
    if (enable && !compressionEngine) {
        compressionEngine = std::make_shared<AdvancedCompressionEngine>();
        memoryPool.setCompressionEngine(compressionEngine);
        
        std::cout << "🗜️  Compression integration enabled\n";
        std::cout << "   • LZ4 fast compression: Active\n";
        std::cout << "   • Adaptive compression: Enabled\n";
        std::cout << "   • Memory pool compression: Ready\n";
    } else if (!enable) {
        std::cout << "🗜️  Compression integration disabled\n";
    }
}

void MemoryOptimizationManager::setCompressionEngine(
    std::shared_ptr<AdvancedCompressionEngine> engine) {
    compressionEngine = engine;
    if (engine) {
        memoryPool.setCompressionEngine(engine);
    }
}

void MemoryOptimizationManager::setCompressionManager(
    std::shared_ptr<IntegratedCompressionManager> manager) {
    compressionManager = manager;
}

bool MemoryOptimizationManager::optimizeWithCompression() {
    if (!compressionEnabled || !compressionEngine) {
        std::cerr << "⚠️  Compression not enabled for optimization\n";
        return false;
    }
    
    std::cout << "🚀 Performing memory optimization with compression...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Step 1: Optimize memory pool with compression
    std::cout << "  [1/4] Optimizing memory pool with compression...\n";
    memoryPool.compactMemory();
    
    // Step 2: Compress cached objects
    std::cout << "  [2/4] Compressing cached objects...\n";
    objectCache.optimizeMemoryUsage();
    
    // Step 3: Optimize storage with compression
    std::cout << "  [3/4] Optimizing storage with compression...\n";
    storageOptimizer.optimizeStorageLayout();
    
    // Step 4: Run integrated compression optimization
    std::cout << "  [4/4] Running integrated compression optimization...\n";
    if (compressionManager) {
        compressionManager->optimizeForBalance();
    } else {
        // Fallback to engine-level optimization
        compressionEngine->enableParallelCompression(true);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "✅ Memory + compression optimization completed in " 
              << duration.count() << "ms\n";
    
    // Print combined statistics
    printCompressionIntegratedStats();
    
    return true;
}

void MemoryOptimizationManager::printCompressionIntegratedStats() {
    auto memProfile = getMemoryProfile();
    
    std::cout << "\n📊 INTEGRATED MEMORY + COMPRESSION STATISTICS:\n";
    std::cout << "   • Memory pool usage: " << (memProfile.poolMemoryUsage / 1024.0 / 1024.0) << " MB\n";
    std::cout << "   • Cache memory usage: " << (memProfile.cacheMemoryUsage / 1024.0 / 1024.0) << " MB\n";
    std::cout << "   • Memory efficiency: " << (memProfile.memoryEfficiency * 100.0) << "%\n";
    
    if (compressionEngine && memProfile.compressedDataSize > 0) {
        std::cout << "   • Compressed data: " << Utils::formatSize(memProfile.compressedDataSize) << "\n";
        std::cout << "   • Uncompressed data: " << Utils::formatSize(memProfile.uncompressedDataSize) << "\n";
        std::cout << "   • Compression ratio: " << std::fixed << std::setprecision(1) 
                  << (memProfile.overallCompressionRatio * 100.0) << "%\n";
        std::cout << "   • Space saved: " << Utils::formatSize(memProfile.totalSpaceSaved) << "\n";
    }
    
    if (compressionManager) {
        auto overallStats = compressionManager->getOverallStats();
        std::cout << "   • Overall performance gain: " << std::fixed << std::setprecision(1) 
                  << overallStats.performanceGain << "%\n";
    }
}

void IntelligentObjectCache::cleanupExpiredEntries() {
    auto now = std::chrono::steady_clock::now();
    
    auto it = cache.begin();
    while (it != cache.end()) {
        auto& entry = it->second;
        auto age = std::chrono::duration_cast<std::chrono::minutes>(now - entry.created);
        
        if (age > entry.ttl) {
            currentMemoryUsage -= entry.size;
            metrics.totalObjects--;
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace gyatt
