#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include "ignore.h"

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
    
    // Remote operations
    bool clone(const std::string& sourceUrl, const std::string& targetDir = "");
    bool push(const std::string& remoteName = "origin", const std::string& branchName = "");
    bool addRemote(const std::string& name, const std::string& url);
    bool listRemotes();
    
    // Ignore file operations
    bool createIgnoreFile();
    bool isIgnored(const std::string& filepath) const;
    bool addIgnorePattern(const std::string& pattern);
    
    // GitHub-specific operations
    bool cloneFromGitHub(const std::string& repoUrl, const std::string& targetDir = "");
    bool pushToGitHub(const std::string& remoteName = "origin", const std::string& branchName = "");
    bool downloadGitHubRepo(const std::string& repoName, const std::string& targetDir);
    bool uploadToGitHub(const std::string& repoName, const std::string& branch = "main");
    bool setGitHubToken(const std::string& token);
    
    // Getters
    std::string getRepoPath() const { return repoPath; }
    std::string getCurrentBranch() const;
    bool isRepository() const;
    
private:
    std::string repoPath;
    std::string gyattDir;
    std::unique_ptr<IgnoreList> ignoreList;
    std::string objectsDir;
    std::string refsDir;
    std::string headsDir;
    std::string remotesDir;
    std::string configFile;
    std::string indexFile;
    std::string headFile;
    
    bool createDirectoryStructure();
    bool writeHead(const std::string& ref);
    std::string readHead() const;
    std::string getBranchCommit(const std::string& branchName);
    bool writeBranchCommit(const std::string& branchName, const std::string& commitHash);
    
    // Remote operations helpers
    bool copyRepository(const std::string& source, const std::string& target);
    bool syncObjects(const std::string& source, const std::string& target);
    bool syncRefs(const std::string& source, const std::string& target);
    std::map<std::string, std::string> parseConfig();
    
    // GitHub helpers
    bool isGitHubUrl(const std::string& url);
    std::string getGitHubApiUrl(const std::string& repoName);
    std::string getGitHubDownloadUrl(const std::string& repoName, const std::string& branch = "main");
    std::string getGitHubToken();
    bool createGitHubRepo(const std::string& repoName);
    bool uploadFilesToGitHub(const std::string& repoName, const std::string& branch);
    bool uploadToEmptyGitHubRepo(const std::string& repoName, const std::string& branch, const std::string& token);
};

} // namespace gyatt
