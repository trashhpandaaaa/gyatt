#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

namespace gyatt {

class Repository {
public:
    Repository(const std::string& path = ".");
    
    // Core operations
    bool init();
    bool add(const std::string& filepath);
    bool commit(const std::string& message, const std::string& author = "");
    bool status();
    bool log();
    bool diff();
    
    // Branch operations
    bool createBranch(const std::string& branchName);
    bool checkout(const std::string& branchName);
    bool listBranches();
    
    // File operations
    bool show(const std::string& objectRef);
    
    // Getters
    std::string getRepoPath() const { return repoPath; }
    std::string getCurrentBranch() const;
    bool isRepository() const;
    
private:
    std::string repoPath;
    std::string gyattDir;
    std::string objectsDir;
    std::string refsDir;
    std::string headsDir;
    std::string indexFile;
    std::string headFile;
    
    bool createDirectoryStructure();
    bool writeHead(const std::string& ref);
    std::string readHead() const;
    std::string getBranchCommit(const std::string& branchName);
    bool writeBranchCommit(const std::string& branchName, const std::string& commitHash);
};

} // namespace gyatt
