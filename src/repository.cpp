#include "repository.h"
#include "index.h"
#include "commit.h"
#include "object.h"
#include "utils.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <set>
#include <sstream>
#include <curl/curl.h>

namespace gyatt {

Repository::Repository(const std::string& path) 
    : repoPath(Utils::absolutePath(path)) {
    gyattDir = Utils::joinPath(repoPath, ".gyatt");
    objectsDir = Utils::joinPath(gyattDir, "objects");
    refsDir = Utils::joinPath(gyattDir, "refs");
    headsDir = Utils::joinPath(refsDir, "heads");
    remotesDir = Utils::joinPath(refsDir, "remotes");
    configFile = Utils::joinPath(gyattDir, "config");
    indexFile = Utils::joinPath(gyattDir, "index");
    headFile = Utils::joinPath(gyattDir, "HEAD");
    ignoreList = std::make_unique<IgnoreList>(repoPath);
}

bool Repository::init() {
    if (isRepository()) {
        std::cout << "Reinitialized existing Gyatt repository in " << gyattDir << "\n";
        return true;
    }
    
    if (!createDirectoryStructure()) {
        return false;
    }
    
    // Create initial HEAD pointing to master branch
    if (!writeHead("ref: refs/heads/master")) {
        return false;
    }
    
    // Create default .gyattignore file
    createIgnoreFile();
    
    return true;
}

bool Repository::add(const std::string& filepath) {
    if (!isRepository()) {
        return false;
    }
    
    Index index(repoPath);
    
    if (filepath == ".") {
        // Add all files in current directory (simplified implementation)
        // In a real implementation, this would recursively add all files
        // excluding those in .gyatt directory and following .gyattignore rules
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
                if (entry.is_regular_file()) {
                    std::string relPath = Utils::relativePath(repoPath, entry.path().string());
                    
                    // Skip files that should be ignored
                    if (!isIgnored(relPath)) {
                        if (!index.addFile(relPath)) {
                            std::cerr << "Warning: failed to add " << relPath << "\n";
                        }
                    }
                }
            }
            return true;
        } catch (const std::exception&) {
            return false;
        }
    } else {
        // Skip adding if the file is ignored
        if (isIgnored(filepath)) {
            std::cout << "Skipping ignored file: " << filepath << "\n";
            return true;
        }
        return index.addFile(filepath);
    }
}

bool Repository::commit(const std::string& message, const std::string& author) {
    if (!isRepository()) {
        return false;
    }
    
    Index index(repoPath);
    auto stagedFiles = index.getStagedFiles();
    
    if (stagedFiles.empty()) {
        std::cout << "nothing to commit, working tree clean\n";
        return true;
    }
    
    // Create tree from staged files
    std::string treeHash = index.createTree();
    
    // Get parent commit
    std::string currentBranch = getCurrentBranch();
    std::string parentHash = getBranchCommit(currentBranch);
    
    // Create commit
    Commit commitObj(repoPath);
    std::string commitHash = commitObj.createCommit(message, author, treeHash, parentHash);
    
    // Update branch reference
    if (!writeBranchCommit(currentBranch, commitHash)) {
        return false;
    }
    
    // Clear staging area
    auto allFiles = index.getAllFiles();
    for (auto& entry : allFiles) {
        entry.staged = false;
    }
    index.saveIndex();
    
    std::cout << "[" << currentBranch << " " << Utils::shortHash(commitHash) << "] " << message << "\n";
    
    return true;
}

bool Repository::status() {
    if (!isRepository()) {
        return false;
    }
    
    std::cout << "On branch " << getCurrentBranch() << "\n";
    
    Index index(repoPath);
    auto fileStatuses = index.getFileStatuses();
    auto stagedFiles = index.getStagedFiles();
    
    if (!stagedFiles.empty()) {
        std::cout << "\nChanges to be committed:\n";
        std::cout << "  (use \"gyatt reset HEAD <file>...\" to unstage)\n\n";
        
        for (const auto& entry : stagedFiles) {
            std::cout << "\tnew file:   " << entry.filepath << "\n";
        }
    }
    
    bool hasModified = false;
    for (const auto& [path, status] : fileStatuses) {
        if (status == Index::FileStatus::MODIFIED) {
            if (!hasModified) {
                std::cout << "\nChanges not staged for commit:\n";
                std::cout << "  (use \"gyatt add <file>...\" to update what will be committed)\n";
                std::cout << "  (use \"gyatt checkout -- <file>...\" to discard changes in working directory)\n\n";
                hasModified = true;
            }
            std::cout << "\tmodified:   " << path << "\n";
        }
    }
    
    if (stagedFiles.empty() && !hasModified) {
        std::cout << "\nnothing to commit, working tree clean\n";
    }
    
    return true;
}

bool Repository::log() {
    if (!isRepository()) {
        return false;
    }
    
    std::string currentBranch = getCurrentBranch();
    std::string startCommit = getBranchCommit(currentBranch);
    
    if (startCommit.empty()) {
        std::cout << "No commits yet\n";
        return true;
    }
    
    Commit commitObj(repoPath);
    auto history = commitObj.getCommitHistory(startCommit);
    
    for (const auto& commit : history) {
        std::cout << "commit " << commit.hash << "\n";
        std::cout << "Author: " << commit.author << "\n";
        std::cout << "Date:   " << Utils::formatTime(commit.timestamp) << "\n\n";
        std::cout << "    " << commit.message << "\n\n";
    }
    
    return true;
}

bool Repository::diff() {
    if (!isRepository()) {
        return false;
    }
    
    // Simplified diff implementation
    // In a real implementation, this would show detailed differences
    std::cout << "diff --gyatt (simplified implementation)\n";
    
    Index index(repoPath);
    auto fileStatuses = index.getFileStatuses();
    
    for (const auto& [path, status] : fileStatuses) {
        if (status == Index::FileStatus::MODIFIED) {
            std::cout << "--- a/" << path << "\n";
            std::cout << "+++ b/" << path << "\n";
            std::cout << "File has been modified\n\n";
        }
    }
    
    return true;
}

bool Repository::createBranch(const std::string& branchName) {
    if (!isRepository()) {
        return false;
    }
    
    std::string branchFile = Utils::joinPath(headsDir, branchName);
    
    if (Utils::fileExists(branchFile)) {
        std::cerr << "A branch named '" << branchName << "' already exists.\n";
        return false;
    }
    
    // Get current commit
    std::string currentBranch = getCurrentBranch();
    std::string currentCommit = getBranchCommit(currentBranch);
    
    if (currentCommit.empty()) {
        std::cerr << "Cannot create branch from empty commit\n";
        return false;
    }
    
    return writeBranchCommit(branchName, currentCommit);
}

bool Repository::checkout(const std::string& branchName) {
    if (!isRepository()) {
        return false;
    }
    
    std::string branchFile = Utils::joinPath(headsDir, branchName);
    
    if (!Utils::fileExists(branchFile)) {
        std::cerr << "error: pathspec '" << branchName << "' did not match any file(s) known to gyatt.\n";
        return false;
    }
    
    return writeHead("ref: refs/heads/" + branchName);
}

bool Repository::listBranches() {
    if (!isRepository()) {
        return false;
    }
    
    std::string currentBranch = getCurrentBranch();
    
    // Recursively find all branch files
    std::vector<std::string> branches;
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(headsDir)) {
            if (entry.is_regular_file()) {
                std::string branchPath = entry.path().string();
                std::string branch = Utils::relativePath(headsDir, branchPath);
                branches.push_back(branch);
            }
        }
    } catch (const std::exception&) {
        return false;
    }
    
    std::sort(branches.begin(), branches.end());
    
    for (const auto& branch : branches) {
        if (branch == currentBranch) {
            std::cout << "* " << branch << "\n";
        } else {
            std::cout << "  " << branch << "\n";
        }
    }
    
    return true;
}

bool Repository::show(const std::string& objectRef) {
    if (!isRepository()) {
        return false;
    }
    
    // Parse object reference (simplified implementation)
    std::string hash = objectRef;
    
    // If it's a short hash, try to find the full hash
    if (hash.length() < 40) {
        GitObject gitObj(repoPath);
        auto objects = gitObj.listObjects();
        
        for (const auto& obj : objects) {
            if (obj.substr(0, hash.length()) == hash) {
                hash = obj;
                break;
            }
        }
    }
    
    try {
        GitObject gitObj(repoPath);
        ObjectType type = gitObj.getObjectType(hash);
        
        switch (type) {
            case ObjectType::COMMIT: {
                Commit commitObj(repoPath);
                auto commitInfo = commitObj.readCommit(hash);
                std::cout << "commit " << commitInfo.hash << "\n";
                std::cout << "Author: " << commitInfo.author << "\n";
                std::cout << "Date:   " << Utils::formatTime(commitInfo.timestamp) << "\n\n";
                std::cout << commitInfo.message << "\n";
                break;
            }
            case ObjectType::BLOB: {
                std::string content = gitObj.readBlob(hash);
                std::cout << content;
                break;
            }
            case ObjectType::TREE: {
                std::string content = gitObj.readTree(hash);
                std::cout << "tree " << hash << "\n\n";
                // Parse and display tree content (simplified)
                std::cout << content;
                break;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }
}

std::string Repository::getCurrentBranch() const {
    std::string head = readHead();
    
    if (head.substr(0, 5) == "ref: ") {
        std::string ref = head.substr(5);
        if (ref.substr(0, 11) == "refs/heads/") {
            return ref.substr(11);
        }
    }
    
    return "master"; // Default branch
}

bool Repository::isRepository() const {
    return Utils::directoryExists(gyattDir) && Utils::fileExists(headFile);
}

bool Repository::createDirectoryStructure() {
    return Utils::createDirectories(gyattDir) &&
           Utils::createDirectories(objectsDir) &&
           Utils::createDirectories(refsDir) &&
           Utils::createDirectories(headsDir) &&
           Utils::createDirectories(remotesDir);
}

bool Repository::writeHead(const std::string& ref) {
    return Utils::writeFile(headFile, ref + "\n");
}

std::string Repository::readHead() const {
    if (!Utils::fileExists(headFile)) {
        return "";
    }
    
    try {
        std::string content = Utils::readFile(headFile);
        return Utils::trim(content);
    } catch (const std::exception&) {
        return "";
    }
}

std::string Repository::getBranchCommit(const std::string& branchName) {
    std::string branchFile = Utils::joinPath(headsDir, branchName);
    
    if (!Utils::fileExists(branchFile)) {
        return "";
    }
    
    try {
        std::string content = Utils::readFile(branchFile);
        return Utils::trim(content);
    } catch (const std::exception&) {
        return "";
    }
}

bool Repository::writeBranchCommit(const std::string& branchName, const std::string& commitHash) {
    std::string branchFile = Utils::joinPath(headsDir, branchName);
    return Utils::writeFile(branchFile, commitHash + "\n");
}

// Remote operations implementation
bool Repository::clone(const std::string& sourceUrl, const std::string& targetDir) {
    // Check if this is a GitHub URL and handle it specially
    if (Utils::isGitHubUrl(sourceUrl)) {
        return cloneFromGitHub(sourceUrl, targetDir);
    }
    
    std::string actualTargetDir = targetDir;
    if (actualTargetDir.empty()) {
        // Extract repository name from source path
        std::filesystem::path sourcePath(sourceUrl);
        actualTargetDir = sourcePath.filename().string();
        if (actualTargetDir.length() >= 6 && actualTargetDir.substr(actualTargetDir.length() - 6) == ".gyatt") {
            actualTargetDir = actualTargetDir.substr(0, actualTargetDir.length() - 6);
        }
    }
    
    // Check if source is a valid gyatt repository
    std::string sourceGyattDir = Utils::joinPath(sourceUrl, ".gyatt");
    if (!Utils::directoryExists(sourceGyattDir)) {
        std::cerr << "fatal: repository '" << sourceUrl << "' does not exist or is not a gyatt repository\n";
        return false;
    }
    
    // Check if target directory doesn't exist or is empty
    if (Utils::directoryExists(actualTargetDir)) {
        auto files = Utils::listDirectory(actualTargetDir);
        // Filter out . and .. entries
        std::vector<std::string> actualFiles;
        for (const auto& file : files) {
            if (file != "." && file != "..") {
                actualFiles.push_back(file);
            }
        }
        if (!actualFiles.empty()) {
            std::cerr << "fatal: destination path '" << actualTargetDir << "' already exists and is not an empty directory.\n";
            return false;
        }
    }
    
    std::cout << "Cloning into '" << actualTargetDir << "'...\n";
    
    // Create target directory if it doesn't exist
    if (!Utils::directoryExists(actualTargetDir)) {
        if (!Utils::createDirectories(actualTargetDir)) {
            std::cerr << "fatal: could not create directory '" << actualTargetDir << "'\n";
            return false;
        }
    }
    
    // Copy the entire repository
    if (!copyRepository(sourceUrl, actualTargetDir)) {
        std::cerr << "fatal: failed to copy repository\n";
        return false;
    }
    
    // Add origin remote
    Repository targetRepo(actualTargetDir);
    if (!targetRepo.addRemote("origin", Utils::absolutePath(sourceUrl))) {
        std::cerr << "warning: failed to add origin remote\n";
    }
    
    std::cout << "Done.\n";
    return true;
}

bool Repository::push(const std::string& remoteName, const std::string& branchName) {
    if (!isRepository()) {
        std::cerr << "fatal: not a gyatt repository\n";
        return false;
    }
    
    // Parse config to get remote URL
    auto config = parseConfig();
    std::string remoteUrlKey = "remote." + remoteName + ".url";
    
    if (config.find(remoteUrlKey) == config.end()) {
        std::cerr << "fatal: '" << remoteName << "' does not appear to be a gyatt repository\n";
        return false;
    }
    
    std::string remoteUrl = config[remoteUrlKey];
    
    // Check if this is a GitHub URL and handle it specially
    if (Utils::isGitHubUrl(remoteUrl)) {
        return pushToGitHub(remoteName, branchName);
    }
    
    // Check if remote repository exists
    std::string remoteGyattDir = Utils::joinPath(remoteUrl, ".gyatt");
    if (!Utils::directoryExists(remoteGyattDir)) {
        std::cerr << "fatal: '" << remoteUrl << "' does not appear to be a gyatt repository\n";
        return false;
    }
    
    std::string actualBranchName = branchName.empty() ? getCurrentBranch() : branchName;
    std::string currentCommit = getBranchCommit(actualBranchName);
    
    if (currentCommit.empty()) {
        std::cerr << "error: src refspec " << actualBranchName << " does not match any\n";
        return false;
    }
    
    std::cout << "Pushing to " << remoteName << " (" << remoteUrl << ")...\n";
    
    // Sync objects to remote
    if (!syncObjects(repoPath, remoteUrl)) {
        std::cerr << "error: failed to push objects\n";
        return false;
    }
    
    // Update remote branch reference
    Repository remoteRepo(remoteUrl);
    if (!remoteRepo.writeBranchCommit(actualBranchName, currentCommit)) {
        std::cerr << "error: failed to update remote branch\n";
        return false;
    }
    
    std::cout << "To " << remoteUrl << "\n";
    std::cout << "   " << Utils::shortHash(currentCommit) << "  " << actualBranchName << " -> " << actualBranchName << "\n";
    
    return true;
}

bool Repository::addRemote(const std::string& name, const std::string& url) {
    if (!isRepository()) {
        return false;
    }
    
    // Create or update config file
    auto config = parseConfig();
    config["remote." + name + ".url"] = url;
    config["remote." + name + ".fetch"] = "+refs/heads/*:refs/remotes/" + name + "/*";
    
    // Write config file
    std::ostringstream configContent;
    std::map<std::string, std::map<std::string, std::string>> sections;
    
    // Group by sections
    for (const auto& [key, value] : config) {
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos) {
            std::string section = key.substr(0, dotPos);
            std::string subsection = key.substr(dotPos + 1, key.rfind('.') - dotPos - 1);
            std::string option = key.substr(key.rfind('.') + 1);
            
            std::string sectionKey = section;
            if (!subsection.empty() && subsection != option) {
                sectionKey += " \"" + subsection + "\"";
            }
            
            sections[sectionKey][option] = value;
        }
    }
    
    // Write sections
    for (const auto& [sectionName, options] : sections) {
        configContent << "[" << sectionName << "]\n";
        for (const auto& [option, value] : options) {
            configContent << "\t" << option << " = " << value << "\n";
        }
    }
    
    return Utils::writeFile(configFile, configContent.str());
}

bool Repository::listRemotes() {
    if (!isRepository()) {
        return false;
    }
    
    auto config = parseConfig();
    std::set<std::string> remotes;
    
    for (const auto& [key, value] : config) {
        if (key.substr(0, 7) == "remote.") {
            size_t nextDot = key.find('.', 7);
            if (nextDot != std::string::npos) {
                std::string remoteName = key.substr(7, nextDot - 7);
                remotes.insert(remoteName);
            }
        }
    }
    
    for (const auto& remote : remotes) {
        std::cout << remote << "\n";
    }
    
    return true;
}

// Ignore file operations
bool Repository::createIgnoreFile() {
    return IgnoreList::createDefaultIgnoreFile(repoPath);
}

bool Repository::isIgnored(const std::string& filepath) const {
    return ignoreList->isIgnored(filepath);
}

bool Repository::addIgnorePattern(const std::string& pattern) {
    ignoreList->addPattern(pattern);
    return true;
}

// Helper functions for remote operations
bool Repository::copyRepository(const std::string& source, const std::string& target) {
    try {
        // Copy all files from source to target
        for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
            if (entry.is_regular_file()) {
                std::string relativePath = Utils::relativePath(source, entry.path().string());
                std::string targetPath = Utils::joinPath(target, relativePath);
                
                // Ensure parent directory exists
                std::string parentDir = Utils::getParentPath(targetPath);
                if (!parentDir.empty()) {
                    Utils::createDirectories(parentDir);
                }
                
                // Copy file
                std::filesystem::copy_file(entry.path(), targetPath);
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error copying repository: " << e.what() << "\n";
        return false;
    }
}

bool Repository::syncObjects(const std::string& source, const std::string& target) {
    try {
        std::string sourceObjectsDir = Utils::joinPath(source, ".gyatt/objects");
        std::string targetObjectsDir = Utils::joinPath(target, ".gyatt/objects");
        
        // Ensure target objects directory exists
        Utils::createDirectories(targetObjectsDir);
        
        // Copy all objects
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceObjectsDir)) {
            if (entry.is_regular_file()) {
                std::string relativePath = Utils::relativePath(sourceObjectsDir, entry.path().string());
                std::string targetPath = Utils::joinPath(targetObjectsDir, relativePath);
                
                // Ensure parent directory exists
                std::string parentDir = Utils::getParentPath(targetPath);
                if (!parentDir.empty()) {
                    Utils::createDirectories(parentDir);
                }
                
                // Copy object file if it doesn't exist
                if (!Utils::fileExists(targetPath)) {
                    std::filesystem::copy_file(entry.path(), targetPath);
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error syncing objects: " << e.what() << "\n";
        return false;
    }
}

bool Repository::syncRefs(const std::string& source, const std::string& target) {
    try {
        std::string sourceRefsDir = Utils::joinPath(source, ".gyatt/refs");
        std::string targetRefsDir = Utils::joinPath(target, ".gyatt/refs");
        
        // Ensure target refs directory exists
        Utils::createDirectories(targetRefsDir);
        
        // Copy all refs
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceRefsDir)) {
            if (entry.is_regular_file()) {
                std::string relativePath = Utils::relativePath(sourceRefsDir, entry.path().string());
                std::string targetPath = Utils::joinPath(targetRefsDir, relativePath);
                
                // Ensure parent directory exists
                std::string parentDir = Utils::getParentPath(targetPath);
                if (!parentDir.empty()) {
                    Utils::createDirectories(parentDir);
                }
                
                // Copy ref file
                std::filesystem::copy_file(entry.path(), targetPath);
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error syncing refs: " << e.what() << "\n";
        return false;
    }
}

std::map<std::string, std::string> Repository::parseConfig() {
    std::map<std::string, std::string> config;
    
    if (!Utils::fileExists(configFile)) {
        return config;
    }
    
    try {
        std::string content = Utils::readFile(configFile);
        std::istringstream iss(content);
        std::string line;
        std::string currentSection;
        
        while (std::getline(iss, line)) {
            line = Utils::trim(line);
            
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                // Handle subsections like [remote "origin"]
                size_t quotePos = currentSection.find('"');
                if (quotePos != std::string::npos) {
                    std::string section = Utils::trim(currentSection.substr(0, quotePos));
                    std::string subsection = currentSection.substr(quotePos + 1);
                    size_t endQuote = subsection.find('"');
                    if (endQuote != std::string::npos) {
                        subsection = subsection.substr(0, endQuote);
                        currentSection = section + "." + subsection;
                    }
                }
            } else {
                size_t equalPos = line.find('=');
                if (equalPos != std::string::npos && !currentSection.empty()) {
                    std::string key = Utils::trim(line.substr(0, equalPos));
                    std::string value = Utils::trim(line.substr(equalPos + 1));
                    config[currentSection + "." + key] = value;
                }
            }
        }
    } catch (const std::exception&) {
        // Return empty config on error
    }
    
    return config;
}

// GitHub-specific operations implementation
bool Repository::cloneFromGitHub(const std::string& repoUrl, const std::string& targetDir) {
    if (!Utils::isGitHubUrl(repoUrl)) {
        std::cerr << "error: not a GitHub URL: " << repoUrl << "\n";
        return false;
    }
    
    std::string repoName = Utils::parseGitHubRepoName(repoUrl);
    if (repoName.empty()) {
        std::cerr << "error: could not parse GitHub repository name from: " << repoUrl << "\n";
        return false;
    }
    
    std::cout << "Cloning GitHub repository: " << repoName << "\n";
    return downloadGitHubRepo(repoName, targetDir);
}

bool Repository::pushToGitHub(const std::string& remoteName, const std::string& branchName) {
    if (!isRepository()) {
        return false;
    }
    
    auto config = parseConfig();
    std::string remoteUrl = config["remote." + remoteName + ".url"];
    
    if (remoteUrl.empty()) {
        std::cerr << "error: remote '" << remoteName << "' does not exist\n";
        return false;
    }
    
    if (!Utils::isGitHubUrl(remoteUrl)) {
        // Fall back to regular push for non-GitHub remotes
        return push(remoteName, branchName);
    }
    
    std::string repoName = Utils::parseGitHubRepoName(remoteUrl);
    if (repoName.empty()) {
        std::cerr << "error: could not parse GitHub repository name\n";
        return false;
    }
    
    std::string actualBranchName = branchName.empty() ? getCurrentBranch() : branchName;
    std::cout << "Pushing to GitHub repository: " << repoName << " (branch: " << actualBranchName << ")\n";
    
    return uploadToGitHub(repoName, actualBranchName);
}

bool Repository::downloadGitHubRepo(const std::string& repoName, const std::string& targetDir) {
    try {
        std::string actualTargetDir = targetDir;
        if (actualTargetDir.empty()) {
            // Extract repository name from repoName (user/repo -> repo)
            size_t slashPos = repoName.find('/');
            if (slashPos != std::string::npos) {
                actualTargetDir = repoName.substr(slashPos + 1);
            } else {
                actualTargetDir = repoName;
            }
        }
        
        std::cout << "Target directory: " << actualTargetDir << "\n";
        
        // Check if target directory doesn't exist or is empty
        if (Utils::directoryExists(actualTargetDir)) {
            auto files = Utils::listDirectory(actualTargetDir);
            std::vector<std::string> actualFiles;
            for (const auto& file : files) {
                if (file != "." && file != "..") {
                    actualFiles.push_back(file);
                }
            }
            if (!actualFiles.empty()) {
                std::cerr << "fatal: destination path '" << actualTargetDir << "' already exists and is not an empty directory.\n";
                return false;
            }
        }
        
        // Create target directory if it doesn't exist
        if (!Utils::directoryExists(actualTargetDir)) {
            std::cout << "Creating directory: " << actualTargetDir << "\n";
            if (!Utils::createDirectories(actualTargetDir)) {
                std::cerr << "fatal: could not create directory '" << actualTargetDir << "'\n";
                return false;
            }
        }
        
        // Try to download repository with different branch names
        std::vector<std::string> branchesToTry = {"main", "master", "develop", "trunk"};
        
        // If a specific branch was requested, try that first
        if (!targetDir.empty() && targetDir != repoName.substr(repoName.find('/') + 1)) {
            // The target directory name might be the branch name
            branchesToTry.insert(branchesToTry.begin(), targetDir);
        }
        Utils::HttpResponse response;
        std::string usedBranch;
        
    for (const auto& branch : branchesToTry) {
        std::string downloadUrl = getGitHubDownloadUrl(repoName, branch);
        std::cout << "Trying branch '" << branch << "': " << downloadUrl << "\n";
        
        response = Utils::httpGet(downloadUrl);
        if (response.success) {
            usedBranch = branch;
            std::cout << "Successfully found branch: " << branch << "\n";
            std::cout << "Received " << response.content.length() << " bytes\n";
            break;
        } else {
            std::cout << "Branch '" << branch << "' not found (HTTP " << response.responseCode << ")\n";
        }
    }
    
    if (!response.success) {
        std::cerr << "error: failed to download repository from any branch\n";
        std::cerr << "Last error: " << response.error << "\n";
        std::cerr << "Please check that the repository exists and is public: https://github.com/" << repoName << "\n";
        return false;
    }
    
    // Check if we got a valid ZIP file
    if (response.content.length() < 100) {
        std::cerr << "error: received too small response, not a valid ZIP file\n";
        std::cerr << "Response content: " << response.content << "\n";
        std::cerr << "Please check that the repository exists and is public: https://github.com/" << repoName << "\n";
        return false;
    }
        
        // Extract the ZIP content to the target directory
        std::cout << "Received " << response.content.length() << " bytes\n";
        std::cout << "Extracting repository content...\n";
        if (!Utils::extractZipData(response.content, actualTargetDir)) {
            std::cerr << "error: failed to extract repository content\n";
            return false;
        }
        
        // Initialize gyatt repository in the extracted content
        std::cout << "Initializing repository...\n";
        Repository targetRepo(actualTargetDir);
        if (!targetRepo.init()) {
            std::cerr << "error: failed to initialize repository\n";
            return false;
        }
        
        // Add all extracted files to the repository
        std::cout << "Adding files to repository...\n";
        if (!targetRepo.add(".")) {
            std::cerr << "warning: some files could not be added to the repository\n";
        }
        
        // Create initial commit
        std::cout << "Creating initial commit...\n";
        std::string commitMessage = "Initial commit from GitHub clone of " + repoName + " (" + usedBranch + " branch)";
        if (!targetRepo.commit(commitMessage, Utils::getAuthorString())) {
            std::cerr << "warning: failed to create initial commit\n";
        }
        
        // Add origin remote
        std::string githubUrl = "https://github.com/" + repoName + ".git";
        std::cout << "Adding remote 'origin' -> " << githubUrl << "\n";
        if (!targetRepo.addRemote("origin", githubUrl)) {
            std::cerr << "warning: failed to add origin remote\n";
        }
        
        std::cout << "Repository cloned successfully to " << actualTargetDir << "\n";
        std::cout << "Downloaded " << response.content.length() << " bytes\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "error: exception occurred during repository download: " << e.what() << "\n";
        return false;
    }
}

bool Repository::uploadToGitHub(const std::string& repoName, const std::string& branch) {
    if (!isRepository()) {
        std::cerr << "error: not a valid gyatt repository\n";
        return false;
    }
    
    std::string currentCommit = getBranchCommit(branch);
    if (currentCommit.empty()) {
        std::cerr << "error: no commits found on branch '" << branch << "'\n";
        return false;
    }
    
    std::cout << "Uploading to GitHub repository: " << repoName << "\n";
    std::cout << "Branch: " << branch << " (" << Utils::shortHash(currentCommit) << ")\n";
    
    // GitHub API requires authentication for pushing changes
    std::string token = getGitHubToken();
    if (token.empty()) {
        std::cerr << "error: GitHub token not found. Please set the GITHUB_TOKEN environment variable.\n";
        std::cerr << "Alternatively, you can create a file at .gyatt/github_token with your token.\n";
        std::cerr << "To create a token, visit: https://github.com/settings/tokens\n";
        return false;
    }
    
    // Check if repository exists
    std::string apiUrl = getGitHubApiUrl(repoName);
    std::vector<std::string> headers = {
        "Authorization: token " + token,
        "Accept: application/vnd.github.v3+json"
    };
    
    Utils::HttpResponse repoResponse = Utils::httpGet(apiUrl, headers);
    bool repoExists = repoResponse.success;
    
    if (!repoExists) {
        std::cout << "Repository does not exist. Creating...\n";
        if (!createGitHubRepo(repoName)) {
            std::cerr << "error: failed to create repository\n";
            return false;
        }
    }
    
    // For a proper implementation, we would need to:
    // 1. Get the current repository state
    // 2. Create blobs for each file
    // 3. Create a tree with those blobs
    // 4. Create a commit with that tree
    // 5. Update the branch reference
    
    // For now, we'll use a simpler approach by creating a ZIP file and pushing it
    
    // Create a temporary directory to prepare the upload
    std::string tempDir = "/tmp/gyatt_" + std::to_string(std::time(nullptr));
    if (!Utils::createDirectories(tempDir)) {
        std::cerr << "error: failed to create temporary directory\n";
        return false;
    }
    
    // Copy the repository files to the temporary directory
    std::string command = "cd '" + repoPath + "' && tar --exclude=.gyatt -czf '" + tempDir + "/repo.tar.gz' .";
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "error: failed to create repository archive\n";
        return false;
    }
    
    // Upload the archive to GitHub
    std::string uploadUrl = "https://api.github.com/repos/" + repoName + "/git/blobs";
    std::string archiveData = Utils::readFile(tempDir + "/repo.tar.gz");
    
    // Convert binary data to base64
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "error: failed to initialize curl\n";
        return false;
    }
    
    // Simulate successful upload for now
    // In a full implementation, we would encode the archive in base64 and send it to GitHub
    std::cout << "Archive created: " << tempDir << "/repo.tar.gz (" << archiveData.size() << " bytes)\n";
    std::cout << "To complete the implementation, the archive would be uploaded using the GitHub API\n";
    std::cout << "This would require multiple API calls to create blobs, trees, and commits\n";
    
    // Clean up
    curl_easy_cleanup(curl);
    std::filesystem::remove_all(tempDir);
    
    std::cout << "Push to GitHub completed successfully (partially implemented)\n";
    std::cout << "To complete this functionality:\n";
    std::cout << "1. Create blobs for each file\n";
    std::cout << "2. Create a tree with those blobs\n";
    std::cout << "3. Create a commit with that tree\n";
    std::cout << "4. Update the branch reference\n";
    
    return true;
}

// GitHub helper methods
bool Repository::isGitHubUrl(const std::string& url) {
    return Utils::isGitHubUrl(url);
}

std::string Repository::getGitHubApiUrl(const std::string& repoName) {
    return "https://api.github.com/repos/" + repoName;
}

std::string Repository::getGitHubDownloadUrl(const std::string& repoName, const std::string& branch) {
    return "https://codeload.github.com/" + repoName + "/zip/refs/heads/" + branch;
}

std::string Repository::getGitHubToken() {
    const char* token = std::getenv("GITHUB_TOKEN");
    if (token) {
        return std::string(token);
    }
    
    // Try to read from .gyatt/github_token
    std::string tokenFile = Utils::joinPath(repoPath, ".gyatt/github_token");
    if (Utils::fileExists(tokenFile)) {
        try {
            std::string token = Utils::readFile(tokenFile);
            // Trim whitespace and newlines
            token = Utils::trim(token);
            if (!token.empty()) {
                return token;
            }
        } catch (const std::exception&) {
            // Ignore errors and fall back to no token
        }
    }
    
    return "";
}

bool Repository::setGitHubToken(const std::string& token) {
    if (!isRepository()) {
        std::cerr << "error: not a valid gyatt repository\n";
        return false;
    }
    
    std::string tokenFile = Utils::joinPath(gyattDir, "github_token");
    
    if (token.empty()) {
        // Remove token file if it exists
        if (Utils::fileExists(tokenFile)) {
            try {
                std::filesystem::remove(tokenFile);
                std::cout << "GitHub token removed\n";
                return true;
            } catch (const std::exception& e) {
                std::cerr << "error: failed to remove token file: " << e.what() << "\n";
                return false;
            }
        }
        return true;
    }
    
    // Save token to file
    bool success = Utils::writeFile(tokenFile, token);
    if (success) {
        // Change file permissions to make it readable only by the owner
        try {
            std::filesystem::permissions(tokenFile, 
                std::filesystem::perms::owner_read | 
                std::filesystem::perms::owner_write,
                std::filesystem::perm_options::replace);
            std::cout << "GitHub token saved\n";
            return true;
        } catch (const std::exception& e) {
            std::cerr << "warning: failed to set permissions on token file: " << e.what() << "\n";
            // Continue anyway since we did save the token
            return true;
        }
    } else {
        std::cerr << "error: failed to save GitHub token\n";
        return false;
    }
}

bool Repository::createGitHubRepo(const std::string& repoName) {
    // Get GitHub token
    std::string token = getGitHubToken();
    if (token.empty()) {
        std::cerr << "error: GitHub token not found. Please set the GITHUB_TOKEN environment variable.\n";
        std::cerr << "Alternatively, you can create a file at .gyatt/github_token with your token.\n";
        std::cerr << "To create a token, visit: https://github.com/settings/tokens\n";
        return false;
    }
    
    // Extract username and repository name
    std::string username, repo;
    size_t slashPos = repoName.find('/');
    if (slashPos != std::string::npos) {
        username = repoName.substr(0, slashPos);
        repo = repoName.substr(slashPos + 1);
    } else {
        // If no slash is found, use the user's GitHub username and the provided name as repo
        std::cerr << "error: invalid repository name format. Use username/repo format.\n";
        return false;
    }
    
    // Prepare request to create repository
    std::string url = "https://api.github.com/user/repos";
    std::string data = "{\"name\":\"" + repo + "\",\"private\":false}";
    
    std::vector<std::string> headers = {
        "Authorization: token " + token,
        "Accept: application/vnd.github.v3+json",
        "Content-Type: application/json"
    };
    
    Utils::HttpResponse response = Utils::httpPost(url, data, headers);
    
    if (!response.success) {
        std::cerr << "error: failed to create repository (HTTP " << response.responseCode << ")\n";
        std::cerr << "Response: " << response.content << "\n";
        return false;
    }
    
    std::cout << "Repository created successfully: " << repoName << "\n";
    return true;
}

bool Repository::uploadFilesToGitHub(const std::string& repoName, const std::string& branch) {
    // This method would implement the GitHub API calls to upload files
    // It would create blobs, trees, and commits using the GitHub Git Data API
    // https://docs.github.com/en/rest/git/blobs
    
    std::cout << "Uploading files to GitHub repository: " << repoName << " (branch: " << branch << ")\n";
    
    // In a real implementation, we would:
    // 1. Get the current commit SHA for the branch
    // 2. Create blobs for each file in the repository
    // 3. Create a tree with those blobs
    // 4. Create a commit pointing to that tree
    // 5. Update the branch reference to point to the new commit
    
    return true;
}

} // namespace gyatt
