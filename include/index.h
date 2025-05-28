#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace gyatt {

class Index {
public:
    struct IndexEntry {
        std::string filepath;
        std::string hash;
        std::filesystem::file_time_type modTime;
        uintmax_t size;
        bool staged;
    };
    
    Index(const std::string& repoPath);
    
    // Index operations
    bool addFile(const std::string& filepath);
    bool removeFile(const std::string& filepath);
    bool isFileStaged(const std::string& filepath);
    
    // Get staged files
    std::vector<IndexEntry> getStagedFiles();
    std::vector<IndexEntry> getAllFiles();
    
    // Index file operations
    bool loadIndex();
    bool saveIndex();
    
    // Create tree from index
    std::string createTree();
    
    // Get file status
    enum class FileStatus {
        UNTRACKED,
        MODIFIED,
        STAGED,
        COMMITTED
    };
    
    std::map<std::string, FileStatus> getFileStatuses();
    
private:
    std::string repoPath;
    std::string indexFile;
    std::string objectsDir;
    std::map<std::string, IndexEntry> entries;
    
    std::string hashFile(const std::string& filepath);
    bool storeBlob(const std::string& filepath, const std::string& hash);
    std::string createTreeObject(const std::map<std::string, IndexEntry>& treeEntries);
};

} // namespace gyatt
