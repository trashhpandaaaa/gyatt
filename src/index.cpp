#include "index.h"
#include "object.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace gyatt {

Index::Index(const std::string& repoPath) 
    : repoPath(repoPath), 
      indexFile(Utils::joinPath(repoPath, ".gyatt/index")),
      objectsDir(Utils::joinPath(repoPath, ".gyatt/objects")) {
    loadIndex();
}

bool Index::addFile(const std::string& filepath) {
    if (!Utils::fileExists(filepath)) {
        return false;
    }
    
    // Calculate file hash
    std::string hash = hashFile(filepath);
    
    // Store as blob object
    if (!storeBlob(filepath, hash)) {
        return false;
    }
    
    // Add to index
    IndexEntry entry;
    entry.filepath = filepath;
    entry.hash = hash;
    entry.modTime = std::filesystem::last_write_time(filepath);
    entry.size = std::filesystem::file_size(filepath);
    entry.staged = true;
    
    entries[filepath] = entry;
    
    return saveIndex();
}

bool Index::removeFile(const std::string& filepath) {
    auto it = entries.find(filepath);
    if (it != entries.end()) {
        entries.erase(it);
        return saveIndex();
    }
    return false;
}

bool Index::isFileStaged(const std::string& filepath) {
    auto it = entries.find(filepath);
    return it != entries.end() && it->second.staged;
}

std::vector<Index::IndexEntry> Index::getStagedFiles() {
    std::vector<IndexEntry> staged;
    
    for (const auto& [path, entry] : entries) {
        if (entry.staged) {
            staged.push_back(entry);
        }
    }
    
    return staged;
}

std::vector<Index::IndexEntry> Index::getAllFiles() {
    std::vector<IndexEntry> all;
    
    for (const auto& [path, entry] : entries) {
        all.push_back(entry);
    }
    
    return all;
}

bool Index::loadIndex() {
    if (!Utils::fileExists(indexFile)) {
        return true; // Empty index is valid
    }
    
    try {
        std::string content = Utils::readFile(indexFile);
        std::istringstream ss(content);
        std::string line;
        
        entries.clear();
        
        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            
            auto parts = Utils::split(line, '\t');
            if (parts.size() >= 4) {
                IndexEntry entry;
                entry.filepath = parts[0];
                entry.hash = parts[1];
                entry.size = std::stoull(parts[2]);
                entry.staged = (parts[3] == "1");
                
                // Parse modification time if available
                if (parts.size() >= 5) {
                    try {
                        // Just validate the time format, use file system time
                        Utils::parseTime(parts[4]);
                        entry.modTime = std::filesystem::last_write_time(entry.filepath);
                    } catch (...) {
                        // Use current time if parsing fails
                        entry.modTime = std::filesystem::file_time_type::clock::now();
                    }
                } else {
                    entry.modTime = std::filesystem::file_time_type::clock::now();
                }
                
                entries[entry.filepath] = entry;
            }
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Index::saveIndex() {
    try {
        std::ostringstream ss;
        
        for (const auto& [path, entry] : entries) {
            std::string timeStr = Utils::formatTime(std::chrono::system_clock::now());
            
            ss << entry.filepath << "\t" 
               << entry.hash << "\t"
               << entry.size << "\t"
               << (entry.staged ? "1" : "0") << "\t"
               << timeStr << "\n";
        }
        
        return Utils::writeFile(indexFile, ss.str());
    } catch (const std::exception&) {
        return false;
    }
}

std::string Index::createTree() {
    std::ostringstream ss;
    
    auto staged = getStagedFiles();
    std::sort(staged.begin(), staged.end(), 
              [](const IndexEntry& a, const IndexEntry& b) {
                  return a.filepath < b.filepath;
              });
    
    for (const auto& entry : staged) {
        ss << "100644 " << Utils::getFileName(entry.filepath) << "\0" << entry.hash;
    }
    
    GitObject gitObj(repoPath);
    return gitObj.createTree(ss.str());
}

std::map<std::string, Index::FileStatus> Index::getFileStatuses() {
    std::map<std::string, FileStatus> statuses;
    
    // Check tracked files
    for (const auto& [path, entry] : entries) {
        if (Utils::fileExists(path)) {
            std::string currentHash = hashFile(path);
            if (currentHash == entry.hash) {
                statuses[path] = entry.staged ? FileStatus::STAGED : FileStatus::COMMITTED;
            } else {
                statuses[path] = FileStatus::MODIFIED;
            }
        }
    }
    
    // TODO: Check for untracked files in working directory
    // This would require scanning the working directory and comparing with tracked files
    
    return statuses;
}

std::string Index::hashFile(const std::string& filepath) {
    std::string content = Utils::readFile(filepath);
    GitObject gitObj(repoPath);
    return GitObject::computeHash(content, ObjectType::BLOB);
}

bool Index::storeBlob(const std::string& filepath, const std::string& /* hash */) {
    try {
        std::string content = Utils::readFile(filepath);
        GitObject gitObj(repoPath);
        gitObj.createBlob(content);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string Index::createTreeObject(const std::map<std::string, IndexEntry>& treeEntries) {
    std::ostringstream ss;
    
    std::vector<std::pair<std::string, IndexEntry>> sortedEntries;
    for (const auto& [path, entry] : treeEntries) {
        sortedEntries.push_back({path, entry});
    }
    
    std::sort(sortedEntries.begin(), sortedEntries.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
    
    for (const auto& [path, entry] : sortedEntries) {
        ss << "100644 " << Utils::getFileName(path) << "\0" << entry.hash;
    }
    
    GitObject gitObj(repoPath);
    return gitObj.createTree(ss.str());
}

} // namespace gyatt
