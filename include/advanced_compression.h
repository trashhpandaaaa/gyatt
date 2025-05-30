#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <atomic>
#include <map>
#include <functional>
#include <iomanip>
#include <sstream>

namespace gyatt {

// CompressionType and CompressionStats must be accessible for other classes
enum class CompressionType {
    NONE,
    LZ4_FAST,
    LZ4_STANDARD,
    LZ4_HIGH,
    ZLIB_FAST,
    ZLIB_BALANCED,
    ZLIB_BEST,
    CUSTOM_DELTA,
    ADAPTIVE
};

struct CompressionStats {
    std::atomic<size_t> totalCompressions{0};
    std::atomic<size_t> totalDecompressions{0};
    std::atomic<size_t> totalBytesCompressed{0};
    std::atomic<size_t> totalBytesDecompressed{0};
    std::atomic<size_t> totalSavedBytes{0};
    std::atomic<double> averageCompressionRatio{0.0};
    std::atomic<std::chrono::milliseconds::rep> totalCompressionTime{0};
    std::atomic<std::chrono::milliseconds::rep> totalDecompressionTime{0};
    CompressionStats() = default;
    CompressionStats(const CompressionStats& other) {
        totalCompressions.store(other.totalCompressions.load());
        totalDecompressions.store(other.totalDecompressions.load());
        totalBytesCompressed.store(other.totalBytesCompressed.load());
        totalBytesDecompressed.store(other.totalBytesDecompressed.load());
        totalSavedBytes.store(other.totalSavedBytes.load());
        averageCompressionRatio.store(other.averageCompressionRatio.load());
        totalCompressionTime.store(other.totalCompressionTime.load());
        totalDecompressionTime.store(other.totalDecompressionTime.load());
    }
    CompressionStats& operator=(const CompressionStats& other) {
        if (this != &other) {
            totalCompressions.store(other.totalCompressions.load());
            totalDecompressions.store(other.totalDecompressions.load());
            totalBytesCompressed.store(other.totalBytesCompressed.load());
            totalBytesDecompressed.store(other.totalBytesDecompressed.load());
            totalSavedBytes.store(other.totalSavedBytes.load());
            averageCompressionRatio.store(other.averageCompressionRatio.load());
            totalCompressionTime.store(other.totalCompressionTime.load());
            totalDecompressionTime.store(other.totalDecompressionTime.load());
        }
        return *this;
    }
};

class AdvancedCompressionEngine {
public:
    struct CompressionResult {
        std::vector<uint8_t> data;
        CompressionType usedType;
        size_t originalSize;
        size_t compressedSize;
        double compressionRatio;
        std::chrono::milliseconds compressionTime;
        bool success;
    };
    
    AdvancedCompressionEngine();
    ~AdvancedCompressionEngine();
    
    // Main compression/decompression interface
    CompressionResult compress(const std::vector<uint8_t>& data, CompressionType type = CompressionType::ADAPTIVE);
    CompressionResult compress(const std::string& data, CompressionType type = CompressionType::ADAPTIVE);
    
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressedData, CompressionType type);
    std::string decompressToString(const std::vector<uint8_t>& compressedData, CompressionType type);
    
    // Streaming compression for large files
    class StreamingCompressor {
    public:
        StreamingCompressor(CompressionType type, size_t bufferSize = 64 * 1024);
        ~StreamingCompressor();
        
        bool addData(const uint8_t* data, size_t size);
        bool addData(const std::string& data);
        std::vector<uint8_t> finalize();
        
        double getCurrentRatio() const;
        size_t getBytesProcessed() const;
        
    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
    
    // Adaptive compression - automatically chooses best algorithm
    CompressionType selectOptimalCompression(const std::vector<uint8_t>& sample) const;
    
    // Git-specific compression optimizations
    CompressionResult compressGitObject(const std::string& objectType, const std::vector<uint8_t>& data);
    CompressionResult compressDelta(const std::vector<uint8_t>& base, const std::vector<uint8_t>& target);
    
    // Configuration
    void setCompressionLevel(CompressionType type, int level); // 1-9 scale
    void enableParallelCompression(bool enable = true);
    void setThreadCount(size_t threads);
    
    // Statistics and monitoring
    CompressionStats getStats() const;
    void resetStats();
    double getOverallCompressionRatio() const;
    size_t getMemorySavings() const;
    
    // Cache management for compression ratios
    void cacheCompressionProfile(const std::string& filePattern, CompressionType bestType);
    CompressionType getCachedCompressionType(const std::string& filePattern) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

// ============================================================================
// PACK FILE OPTIMIZATION WITH COMPRESSION
// ============================================================================

class PackFileOptimizer {
public:
    struct PackConfig {
        size_t maxPackSize = 256 * 1024 * 1024;  // 256MB max pack files
        size_t targetPackSize = 64 * 1024 * 1024; // 64MB target
        double compressionThreshold = 0.1;        // 10% minimum compression ratio
        bool enableDeltaCompression = true;
        bool enableParallelPacking = true;
        size_t deltaWindowSize = 16;              // Delta compression window size
    };
    
    struct PackStats {
        size_t totalPacks = 0;
        size_t totalObjects = 0;
        size_t totalSizeUncompressed = 0;
        size_t totalSizeCompressed = 0;
        double averageCompressionRatio = 0.0;
        std::chrono::milliseconds packingTime{0};
    };
    
    PackFileOptimizer(const std::string& repoPath);
    PackFileOptimizer(const std::string& repoPath, const PackConfig& config);
    ~PackFileOptimizer();
    
    // Pack file operations
    bool createPackFile(const std::vector<std::string>& objectHashes);
    bool optimizeExistingPacks();
    bool repackRepository();
    
    // Delta compression for similar objects
    bool enableSmartDeltaCompression(bool enable = true);
    void setDeltaWindow(size_t windowSize);
    
    // Pack file maintenance
    bool garbageCollectPacks();
    bool verifyPackIntegrity();
    std::vector<std::string> findCorruptedPacks();
    
    PackStats getPackStats() const;
    void printPackStatistics() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

// ============================================================================
// COMPRESSION-AWARE STORAGE MANAGER
// ============================================================================

class CompressionAwareStorage {
public:
    struct StorageConfig {
        std::string storageRoot;
        CompressionType defaultCompression = CompressionType::LZ4_STANDARD;
        bool enableInlineCompression = true;
        bool enableCompressionCache = true;
        size_t compressionCacheSize = 128 * 1024 * 1024; // 128MB
        double compressionThreshold = 0.05; // 5% minimum savings
    };
    
    CompressionAwareStorage(const StorageConfig& config);
    ~CompressionAwareStorage();
    
    // Object storage with automatic compression
    bool storeObject(const std::string& hash, const std::vector<uint8_t>& data);
    bool storeObject(const std::string& hash, const std::string& data);
    
    std::vector<uint8_t> retrieveObject(const std::string& hash);
    std::string retrieveObjectAsString(const std::string& hash);
    
    // Bulk operations with optimized compression
    bool storeBulkObjects(const std::map<std::string, std::vector<uint8_t>>& objects);
    std::map<std::string, std::vector<uint8_t>> retrieveBulkObjects(const std::vector<std::string>& hashes);
    
    // Storage optimization
    bool optimizeStorage();
    bool compactStorage();
    size_t getStorageEfficiency() const;
    
    // Storage statistics
    struct StorageStats {
        size_t totalObjects = 0;
        size_t totalSizeRaw = 0;
        size_t totalSizeCompressed = 0;
        double compressionRatio = 0.0;
        size_t spaceSaved = 0;
        std::map<CompressionType, size_t> compressionTypeUsage;
    };
    
    StorageStats getStorageStats() const;
    void printStorageReport() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

// ============================================================================
// INTEGRATED COMPRESSION MANAGER
// ============================================================================

class IntegratedCompressionManager {
public:
    struct CompressionProfile {
        std::string repositoryPath;
        std::map<std::string, CompressionType> fileTypeMapping;
        PackFileOptimizer::PackConfig packConfig;
        CompressionAwareStorage::StorageConfig storageConfig;
        bool enableRealTimeCompression = true;
        bool enableBackgroundOptimization = true;
    };
    
    IntegratedCompressionManager(const CompressionProfile& profile);
    IntegratedCompressionManager(const std::string& repositoryPath);  // Convenience constructor
    ~IntegratedCompressionManager();
    
    // High-level compression operations
    bool enableCompression(bool enable = true);
    bool optimizeForSpeed();
    bool optimizeForSize();
    bool optimizeForBalance();
    
    // Repository-wide optimization
    bool performFullOptimization();
    bool scheduleBackgroundOptimization();
    bool isOptimizationRunning() const;
    
    // Integration with memory optimization
    void integrateWithMemoryPool(class AdvancedMemoryPool* memoryPool);
    void setCompressionCallback(std::function<void(const std::string&, double)> callback);
    
    // Performance monitoring
    struct OverallStats {
        CompressionStats compressionStats;
        PackFileOptimizer::PackStats packStats;
        CompressionAwareStorage::StorageStats storageStats;
        double overallSpaceSavings = 0.0;
        double performanceGain = 0.0;
        std::chrono::milliseconds totalOptimizationTime{0};
        OverallStats() = default;
        OverallStats(const OverallStats& other)
            : compressionStats(other.compressionStats),
              packStats(other.packStats),
              storageStats(other.storageStats),
              overallSpaceSavings(other.overallSpaceSavings),
              performanceGain(other.performanceGain),
              totalOptimizationTime(other.totalOptimizationTime) {}
        OverallStats& operator=(const OverallStats& other) {
            if (this != &other) {
                compressionStats = other.compressionStats;
                packStats = other.packStats;
                storageStats = other.storageStats;
                overallSpaceSavings = other.overallSpaceSavings;
                performanceGain = other.performanceGain;
                totalOptimizationTime = other.totalOptimizationTime;
            }
            return *this;
        }
    };
    
    OverallStats getOverallStats() const;
    void printComprehensiveReport() const;
    
    // Configuration management
    bool saveCompressionProfile(const std::string& profileName) const;
    bool loadCompressionProfile(const std::string& profileName);
    std::vector<std::string> listCompressionProfiles() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace gyatt
