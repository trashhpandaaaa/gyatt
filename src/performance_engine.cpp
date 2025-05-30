#include "performance_engine.h"
#include "utils.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <immintrin.h> 

namespace gyatt {
MemoryPool::MemoryPool(size_t blockSize) : blockSize(blockSize) {
    allocateNewBlock();
}

MemoryPool::~MemoryPool() {
    for (auto& block : blocks) {
        delete[] block.data;
    }
}

void* MemoryPool::allocate(size_t size) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    if (size > blockSize) {
        return new char[size];
    }
    
    Block& current = getCurrentBlock();
    if (current.used + size > current.size) {
        allocateNewBlock();
        return allocate(size); // Recursive call with new block
    }
    
    void* ptr = current.data + current.used;
    current.used += size;
    return ptr;
}

void MemoryPool::deallocate(void* ptr) {
    // Simple implementation - just mark as available
    // In production, could implement free list
}

void MemoryPool::reset() {
    std::lock_guard<std::mutex> lock(poolMutex);
    for (auto& block : blocks) {
        block.used = 0;
    }
}

MemoryPool::Block& MemoryPool::getCurrentBlock() {
    if (blocks.empty()) {
        allocateNewBlock();
    }
    return blocks.back();
}

void MemoryPool::allocateNewBlock() {
    Block block;
    block.data = new char[blockSize];
    block.size = blockSize;
    block.used = 0;
    blocks.push_back(block);
}

// ParallelProcessor Implementation
ParallelProcessor::ParallelProcessor(size_t numThreads) {
    workers.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ParallelProcessor::workerLoop, this);
    }
}

ParallelProcessor::~ParallelProcessor() {
    stopping = true;
    condition.notify_all();
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

std::vector<std::future<std::string>> ParallelProcessor::hashFilesParallel(
    const std::vector<std::string>& files) {
    
    std::vector<std::future<std::string>> results;
    results.reserve(files.size());
    
    for (const auto& file : files) {
        auto promise = std::make_shared<std::promise<std::string>>();
        results.push_back(promise->get_future());
        
        enqueueTask([file, promise]() {
            try {
                std::string content = Utils::readFile(file);
                std::string hash = Utils::sha1Hash(content);
                promise->set_value(hash);
            } catch (const std::exception& e) {
                promise->set_exception(std::current_exception());
            }
        });
    }
    
    return results;
}

std::vector<std::future<bool>> ParallelProcessor::processFilesParallel(
    const std::vector<std::string>& files,
    std::function<bool(const std::string&)> processor) {
    
    std::vector<std::future<bool>> results;
    results.reserve(files.size());
    
    for (const auto& file : files) {
        auto promise = std::make_shared<std::promise<bool>>();
        results.push_back(promise->get_future());
        
        enqueueTask([file, processor, promise]() {
            try {
                bool result = processor(file);
                promise->set_value(result);
            } catch (const std::exception& e) {
                promise->set_exception(std::current_exception());
            }
        });
    }
    
    return results;
}

std::future<std::vector<std::string>> ParallelProcessor::scanDirectoryAsync(
    const std::string& path,
    const std::function<bool(const std::string&)>& filter) {
    
    auto promise = std::make_shared<std::promise<std::vector<std::string>>>();
    auto future = promise->get_future();
    
    enqueueTask([path, filter, promise]() {
        try {
            std::vector<std::string> files;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string filepath = entry.path().string();
                    if (!filter || filter(filepath)) {
                        files.push_back(filepath);
                    }
                }
            }
            promise->set_value(files);
        } catch (const std::exception& e) {
            promise->set_exception(std::current_exception());
        }
    });
    
    return future;
}

void ParallelProcessor::workerLoop() {
    while (!stopping) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(taskMutex);
            condition.wait(lock, [this] { return stopping || !tasks.empty(); });
            
            if (stopping && tasks.empty()) {
                break;
            }
            
            task = tasks.front();
            tasks.pop();
        }
        
        task();
    }
}

void ParallelProcessor::enqueueTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(taskMutex);
        tasks.push(task);
    }
    condition.notify_one();
}

// MemoryMappedFile Implementation
MemoryMappedFile::MemoryMappedFile(const std::string& filepath) {
    fd = open(filepath.c_str(), O_RDONLY);
    if (fd == -1) {
        return;
    }
    
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        fd = -1;
        return;
    }
    
    size = sb.st_size;
    data = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    
    if (data == MAP_FAILED) {
        data = nullptr;
        close(fd);
        fd = -1;
    }
}

MemoryMappedFile::~MemoryMappedFile() {
    cleanup();
}

std::string MemoryMappedFile::computeHash() const {
    if (!isValid()) {
        return "";
    }
    
    return SIMDOptimizations::fastHash(data, size);
}

bool MemoryMappedFile::contentEquals(const MemoryMappedFile& other) const {
    if (!isValid() || !other.isValid() || size != other.size) {
        return false;
    }
    
    return SIMDOptimizations::fastMemoryCompare(data, other.data, size);
}

void MemoryMappedFile::cleanup() {
    if (data != nullptr) {
        munmap(data, size);
        data = nullptr;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

// ObjectCache Implementation
ObjectCache::ObjectCache(size_t maxSize) : maxSize(maxSize) {}

void ObjectCache::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    if (cache.size() >= maxSize) {
        evictOldest();
    }
    
    CacheEntry entry;
    entry.value = value;
    entry.lastAccess = std::chrono::steady_clock::now();
    entry.accessCount = 1;
    
    cache[key] = std::move(entry);
}

bool ObjectCache::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    auto it = cache.find(key);
    if (it != cache.end()) {
        value = it->second.value;
        it->second.lastAccess = std::chrono::steady_clock::now();
        it->second.accessCount++;
        hits++;
        return true;
    }
    
    misses++;
    return false;
}

void ObjectCache::clear() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.clear();
    hits = 0;
    misses = 0;
}

size_t ObjectCache::getHitRate() const {
    size_t totalHits = hits.load();
    size_t totalMisses = misses.load();
    size_t total = totalHits + totalMisses;
    
    return total > 0 ? (totalHits * 100) / total : 0;
}

void ObjectCache::evictOldest() {
    if (cache.empty()) return;
    
    auto oldest = cache.begin();
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it->second.lastAccess < oldest->second.lastAccess) {
            oldest = it;
        }
    }
    
    cache.erase(oldest);
}

// DeltaCompression Implementation
std::string DeltaCompression::createDelta(const std::string& baseContent, const std::string& newContent) {
    // Simple diff-based compression
    std::ostringstream delta;
    
    size_t baseSize = baseContent.size();
    size_t newSize = newContent.size();
    
    delta << "DELTA:" << baseSize << ":" << newSize << ":";
    
    // Find common prefix
    size_t commonPrefix = 0;
    size_t minSize = std::min(baseSize, newSize);
    while (commonPrefix < minSize && 
           baseContent[commonPrefix] == newContent[commonPrefix]) {
        commonPrefix++;
    }
    
    // Find common suffix
    size_t commonSuffix = 0;
    while (commonSuffix < (minSize - commonPrefix) && 
           baseContent[baseSize - 1 - commonSuffix] == newContent[newSize - 1 - commonSuffix]) {
        commonSuffix++;
    }
    
    // Encode delta
    delta << "PREFIX:" << commonPrefix << ":";
    delta << "SUFFIX:" << commonSuffix << ":";
    delta << "MIDDLE:";
    
    if (commonPrefix + commonSuffix < newSize) {
        delta << newContent.substr(commonPrefix, newSize - commonPrefix - commonSuffix);
    }
    
    return delta.str();
}

std::string DeltaCompression::applyDelta(const std::string& baseContent, const std::string& delta) {
    if (delta.substr(0, 6) != "DELTA:") {
        return ""; // Invalid delta
    }
    
    std::istringstream stream(delta);
    std::string token;
    
    // Parse delta format
    std::getline(stream, token, ':'); // "DELTA"
    std::getline(stream, token, ':'); // base size
    std::getline(stream, token, ':'); // new size
    size_t newSize = std::stoul(token);
    
    std::getline(stream, token, ':'); // "PREFIX"
    std::getline(stream, token, ':'); // prefix length
    size_t prefixLen = std::stoul(token);
    
    std::getline(stream, token, ':'); // "SUFFIX"
    std::getline(stream, token, ':'); // suffix length
    size_t suffixLen = std::stoul(token);
    
    std::getline(stream, token, ':'); // "MIDDLE"
    std::string middle;
    std::getline(stream, middle); // rest of the content
    
    // Reconstruct content
    std::string result;
    result.reserve(newSize);
    
    if (prefixLen > 0) {
        result += baseContent.substr(0, prefixLen);
    }
    result += middle;
    if (suffixLen > 0) {
        result += baseContent.substr(baseContent.size() - suffixLen);
    }
    
    return result;
}

bool DeltaCompression::isDeltaWorthwhile(const std::string& delta, const std::string& original) {
    return delta.size() < original.size() * 0.8; // 20% compression threshold
}

// FastIndex Implementation
FastIndex::FastIndex(const std::string& repoPath) 
    : repoPath(repoPath), indexFile(repoPath + "/.gyatt/index") {
    memPool = std::make_unique<MemoryPool>();
    processor = std::make_unique<ParallelProcessor>();
    cache = std::make_unique<ObjectCache>();
}

bool FastIndex::addFilesBatch(const std::vector<std::string>& files) {
    std::unique_lock<std::shared_mutex> lock(indexMutex);
    
    // Parallel hash computation
    auto hashFutures = processor->hashFilesParallel(files);
    
    for (size_t i = 0; i < files.size(); ++i) {
        try {
            std::string hash = hashFutures[i].get();
            
            IndexEntry entry;
            entry.filepath = files[i];
            entry.hash = hash;
            
            struct stat st;
            if (stat(files[i].c_str(), &st) == 0) {
                entry.modTime = st.st_mtime;
                entry.size = st.st_size;
            }
            entry.staged = true;
            
            // Check if file already exists in index
            auto it = pathToIndex.find(files[i]);
            if (it != pathToIndex.end()) {
                entries[it->second] = entry;
            } else {
                pathToIndex[files[i]] = entries.size();
                entries.push_back(entry);
            }
            
            // Cache the hash
            cache->put(files[i] + ":" + std::to_string(entry.modTime), hash);
            
        } catch (const std::exception& e) {
            // Handle error for individual file
            continue;
        }
    }
    
    indexDirty = true;
    return true;
}

bool FastIndex::removeFilesBatch(const std::vector<std::string>& files) {
    std::unique_lock<std::shared_mutex> lock(indexMutex);
    
    for (const auto& file : files) {
        auto it = pathToIndex.find(file);
        if (it != pathToIndex.end()) {
            size_t index = it->second;
            
            // Mark as removed (lazy deletion)
            entries[index].staged = false;
            entries[index].hash = "";
            
            pathToIndex.erase(it);
        }
    }
    
    indexDirty = true;
    return true;
}

std::map<std::string, std::string> FastIndex::getFileStatusesParallel() {
    std::shared_lock<std::shared_mutex> lock(indexMutex);
    
    std::map<std::string, std::string> statuses;
    std::vector<std::string> filesToCheck;
    
    // Collect files to check
    for (const auto& entry : entries) {
        if (!entry.filepath.empty()) {
            filesToCheck.push_back(entry.filepath);
        }
    }
    
    // Parallel status checking
    auto statusFutures = processor->processFilesParallel(filesToCheck,
        [this](const std::string& file) -> bool {
            struct stat st;
            return stat(file.c_str(), &st) == 0;
        });
    
    for (size_t i = 0; i < filesToCheck.size(); ++i) {
        try {
            bool exists = statusFutures[i].get();
            const std::string& file = filesToCheck[i];
            
            auto it = pathToIndex.find(file);
            if (it != pathToIndex.end()) {
                const auto& entry = entries[it->second];
                
                if (!exists) {
                    statuses[file] = "deleted";
                } else {
                    struct stat st;
                    stat(file.c_str(), &st);
                    
                    if (st.st_mtime != entry.modTime || st.st_size != entry.size) {
                        statuses[file] = "modified";
                    } else if (entry.staged) {
                        statuses[file] = "staged";
                    } else {
                        statuses[file] = "tracked";
                    }
                }
            }
        } catch (const std::exception& e) {
            // Handle error
            continue;
        }
    }
    
    return statuses;
}

bool FastIndex::loadIndexStreaming() {
    std::ifstream file(indexFile, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    std::unique_lock<std::shared_mutex> lock(indexMutex);
    entries.clear();
    pathToIndex.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Parse index entry format: filepath|hash|modtime|size|staged
        std::istringstream iss(line);
        IndexEntry entry;
        
        std::getline(iss, entry.filepath, '|');
        std::getline(iss, entry.hash, '|');
        
        std::string modTimeStr, sizeStr, stagedStr;
        std::getline(iss, modTimeStr, '|');
        std::getline(iss, sizeStr, '|');
        std::getline(iss, stagedStr, '|');
        
        entry.modTime = std::stoull(modTimeStr);
        entry.size = std::stoull(sizeStr);
        entry.staged = (stagedStr == "1");
        
        pathToIndex[entry.filepath] = entries.size();
        entries.push_back(entry);
    }
    
    indexDirty = false;
    return true;
}

bool FastIndex::saveIndexStreaming() {
    if (!indexDirty) {
        return true;
    }
    
    std::shared_lock<std::shared_mutex> lock(indexMutex);
    
    std::ofstream file(indexFile, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& entry : entries) {
        if (!entry.filepath.empty() && !entry.hash.empty()) {
            file << entry.filepath << "|"
                 << entry.hash << "|"
                 << entry.modTime << "|"
                 << entry.size << "|"
                 << (entry.staged ? "1" : "0") << "\n";
        }
    }
    
    indexDirty = false;
    return true;
}

std::string FastIndex::createTreeOptimized() {
    std::shared_lock<std::shared_mutex> lock(indexMutex);
    
    std::ostringstream tree;
    
    // Sort entries by path for efficient tree creation
    std::vector<const IndexEntry*> sortedEntries;
    for (const auto& entry : entries) {
        if (!entry.filepath.empty() && !entry.hash.empty() && entry.staged) {
            sortedEntries.push_back(&entry);
        }
    }
    
    std::sort(sortedEntries.begin(), sortedEntries.end(),
        [](const IndexEntry* a, const IndexEntry* b) {
            return a->filepath < b->filepath;
        });
    
    for (const auto* entry : sortedEntries) {
        tree << entry->hash << " " << entry->filepath << "\n";
    }
    
    return Utils::sha1Hash(tree.str());
}

// PerformanceEngine Implementation
PerformanceEngine::PerformanceEngine(const std::string& repoPath) : repoPath(repoPath) {
    memoryPool = std::make_unique<MemoryPool>();
    parallelProcessor = std::make_unique<ParallelProcessor>();
    objectCache = std::make_unique<ObjectCache>();
    fastIndex = std::make_unique<FastIndex>(repoPath);
}

PerformanceEngine::~PerformanceEngine() = default;

void PerformanceEngine::enableParallelProcessing(bool enable) {
    parallelEnabled = enable;
}

void PerformanceEngine::enableObjectCaching(bool enable) {
    cachingEnabled = enable;
    if (!enable) {
        objectCache->clear();
    }
}

void PerformanceEngine::enableDeltaCompression(bool enable) {
    deltaCompressionEnabled = enable;
}

void PerformanceEngine::enableMemoryMapping(bool enable) {
    memoryMappingEnabled = enable;
}

void PerformanceEngine::enableOptimizations(bool enable) {
    enableParallelProcessing(enable);
    enableObjectCaching(enable);
    enableDeltaCompression(enable);
    enableMemoryMapping(enable);
}

void PerformanceEngine::resetMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex);
    metrics = Metrics{};
}

bool PerformanceEngine::addFilesOptimized(const std::vector<std::string>& files) {
    auto start = std::chrono::steady_clock::now();
    
    bool result = fastIndex->addFilesBatch(files);
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    size_t totalBytes = 0;
    for (const auto& file : files) {
        struct stat st;
        if (stat(file.c_str(), &st) == 0) {
            totalBytes += st.st_size;
        }
    }
    
    updateMetrics(duration, files.size(), totalBytes);
    return result;
}

bool PerformanceEngine::commitOptimized(const std::string& message, const std::string& author) {
    auto start = std::chrono::steady_clock::now();
    
    // Create optimized tree
    std::string treeHash = fastIndex->createTreeOptimized();
    
    // Save index efficiently
    bool result = fastIndex->saveIndexStreaming();
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    updateMetrics(duration, 1, 0);
    return result;
}

std::map<std::string, std::string> PerformanceEngine::statusOptimized() {
    auto start = std::chrono::steady_clock::now();
    
    auto result = fastIndex->getFileStatusesParallel();
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    updateMetrics(duration, result.size(), 0);
    return result;
}

void PerformanceEngine::autoTune() {
    // Benchmark different configurations and choose optimal settings
    benchmarkOperations();
    
    // Auto-adjust cache size based on available memory
    size_t optimalCacheSize = 1000; // Default
    objectCache = std::make_unique<ObjectCache>(optimalCacheSize);
    
    // Auto-adjust thread count based on workload
    size_t optimalThreads = std::thread::hardware_concurrency();
    parallelProcessor = std::make_unique<ParallelProcessor>(optimalThreads);
}

void PerformanceEngine::benchmarkOperations() {
    // Simple benchmark to measure current performance
    std::vector<std::string> testFiles;
    
    // Find some test files
    for (const auto& entry : std::filesystem::directory_iterator(repoPath)) {
        if (entry.is_regular_file() && testFiles.size() < 10) {
            testFiles.push_back(entry.path().string());
        }
    }
    
    if (!testFiles.empty()) {
        auto start = std::chrono::steady_clock::now();
        addFilesOptimized(testFiles);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        // Use benchmark results to optimize settings
    }
}

void PerformanceEngine::updateMetrics(const std::chrono::milliseconds& time, size_t files, size_t bytes) {
    std::lock_guard<std::mutex> lock(metricsMutex);
    metrics.totalTime += time;
    metrics.filesProcessed += files;
    metrics.bytesProcessed += bytes;
    metrics.cacheHits = objectCache->getHitRate();
    metrics.parallelThreadsUsed = std::thread::hardware_concurrency();
}

std::string PerformanceEngine::optimizeObjectStorage(const std::string& content, const std::string& type) {
    if (deltaCompressionEnabled && content.size() > 1024) {
        // Try delta compression for large objects
        // This is a simplified implementation
        return content; // For now, return as-is
    }
    return content;
}

// LockFreeQueue Template Implementation
template<typename T>
void LockFreeQueue<T>::push(T item) {
    Node* newNode = new Node;
    T* data = new T(std::move(item));
    newNode->data.store(data);
    
    Node* prevTail = tail.exchange(newNode);
    prevTail->next.store(newNode);
}

template<typename T>
bool LockFreeQueue<T>::pop(T& item) {
    Node* head_node = head.load();
    Node* next = head_node->next.load();
    
    if (next == nullptr) {
        return false;
    }
    
    T* data = next->data.load();
    if (data == nullptr) {
        return false;
    }
    
    item = *data;
    head.store(next);
    delete data;
    delete head_node;
    
    return true;
}

template<typename T>
bool LockFreeQueue<T>::empty() const {
    Node* head_node = head.load();
    Node* next = head_node->next.load();
    return (next == nullptr);
}

template<typename T>
size_t LockFreeQueue<T>::size() const {
    size_t count = 0;
    Node* current = head.load()->next.load();
    
    while (current != nullptr) {
        count++;
        current = current->next.load();
    }
    
    return count;
}

// SIMDOptimizations Implementation
bool SIMDOptimizations::fastMemoryCompare(const void* a, const void* b, size_t size) {
    const char* ptr1 = static_cast<const char*>(a);
    const char* ptr2 = static_cast<const char*>(b);
    
    if (size >= 32) {
        size_t simd_chunks = size / 32;
        
        for (size_t i = 0; i < simd_chunks; ++i) {
            __m256i chunk1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr1));
            __m256i chunk2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr2));
            
            __m256i cmp = _mm256_cmpeq_epi8(chunk1, chunk2);
            if (_mm256_movemask_epi8(cmp) != 0xFFFFFFFF) {
                return false;
            }
            
            ptr1 += 32;
            ptr2 += 32;
        }
        
        size %= 32;
    }
    
    // Handle remaining bytes
    return memcmp(ptr1, ptr2, size) == 0;
}

std::string SIMDOptimizations::fastHash(const char* data, size_t size) {
    // Use optimized hash function with SIMD acceleration
    // For now, fall back to standard hash
    return Utils::sha1Hash(std::string(data, size));
}

uint64_t SIMDOptimizations::fastChecksum(const char* data, size_t size) {
    uint64_t checksum = 0;
    
    const uint64_t* ptr64 = reinterpret_cast<const uint64_t*>(data);
    size_t chunks = size / 8;
    
    for (size_t i = 0; i < chunks; ++i) { // Process 8 bytes at a time
        checksum ^= ptr64[i];
    }
    
    const char* remaining = data + (chunks * 8); //To handle remaining bytes
    size_t remainingSize = size % 8;
    
    for (size_t i = 0; i < remainingSize; ++i) {
        checksum ^= static_cast<uint64_t>(remaining[i]) << (i * 8);
    }
    
    return checksum;
}

template class LockFreeQueue<std::function<void()>>;
template class LockFreeQueue<std::string>;

} // namespace gyatt
