#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>

namespace gyatt {

class Commit {
public:
    struct CommitInfo {
        std::string hash;
        std::string treeHash;
        std::string parentHash;
        std::string author;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
    };
    
    Commit(const std::string& repoPath);
    
    // Create a new commit
    std::string createCommit(const std::string& message, const std::string& author,
                           const std::string& treeHash, const std::string& parentHash = "");
    
    // Read commit information
    CommitInfo readCommit(const std::string& commitHash);
    
    // Get commit history
    std::vector<CommitInfo> getCommitHistory(const std::string& startCommit = "");
    
    // Validate commit hash
    bool isValidCommit(const std::string& commitHash);
    
private:
    std::string repoPath;
    std::string objectsDir;
    
    std::string formatCommitContent(const std::string& treeHash, const std::string& parentHash,
                                  const std::string& author, const std::string& message,
                                  const std::chrono::system_clock::time_point& timestamp);
    
    CommitInfo parseCommitContent(const std::string& content, const std::string& hash);
    std::string getObjectPath(const std::string& hash);
};

} // namespace gyatt
