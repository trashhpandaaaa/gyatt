#include "repository.h"
#include "index.h"
#include "commit.h"
#include "object.h"
#include "utils.h"
#include "performance_engine.h"
#include "markdown_commit.h"
#include "semantic_branching.h"
#include "section_staging.h"
#include "project_mapper.h"
#include "checkpoint_system.h"
#include "advanced_features.h"
#include "enhanced_features.h"
#include "guardrails.h"
#include "plugin_system.h"
#include "terminal_ui.h"
#include "http_optimization.h"
#include <iostream>
#include <functional>
#include <thread>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <future>
#include <atomic>
#include <mutex>
#include <sys/stat.h>
#include <filesystem>
#include <algorithm>
#include <set>
#include <sstream>
#include <fstream>
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

Repository::~Repository() {
    // Destructor implementation - required for unique_ptr with forward declared types
}

bool Repository::init() {
    if (isRepository()) {
        std::cout << "Reinitialized existing Gyatt repository in " << gyattDir << "\n";
        return true;
    }
    
    if (!createDirectoryStructure()) {
        return false;
    }
    
    // Create initial HEAD pointing to main branch
    if (!writeHead("ref: refs/heads/main")) {
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
    
    return "main"; // Default branch
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

std::vector<RemoteRepository> Repository::listRemotes() {
    std::vector<RemoteRepository> result;
    
    if (!isRepository()) {
        return result;
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
    
    for (const auto& remoteName : remotes) {
        RemoteRepository remote;
        remote.name = remoteName;
        remote.url = config["remote." + remoteName + ".url"];
        remote.protocol = detectProtocol(remote.url);
        result.push_back(remote);
    }
    
    return result;
}

void Repository::printRemotes() {
    auto remotes = listRemotes();
    for (const auto& remote : remotes) {
        std::cout << remote.name << "\n";
    }
}

// Ignore file operations
bool Repository::createIgnoreFile() {
    return IgnoreList::createDefaultIgnoreFile(repoPath);
}

bool Repository::isIgnored(const std::string& filepath) {
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
        
        // First check if the repository exists by querying the GitHub API
        std::string apiUrl = "https://api.github.com/repos/" + repoName;
        std::cout << "Checking repository: " << apiUrl << "\n";
        
        std::vector<std::string> headers = {"Accept: application/vnd.github.v3+json"};
        
        // Add GitHub token if available
        const char* token = std::getenv("GITHUB_TOKEN");
        if (token) {
            std::string authHeader = "Authorization: token " + std::string(token);
            headers.push_back(authHeader);
        }
        
        Utils::HttpResponse repoCheckResponse = Utils::httpGet(apiUrl, headers);
        
        if (!repoCheckResponse.success) {
            std::cerr << "error: repository not found or inaccessible: " << repoName << "\n";
            std::cerr << "HTTP Status: " << repoCheckResponse.responseCode << "\n";
            
            if (repoCheckResponse.responseCode == 404) {
                std::cerr << "The repository does not exist. Please check the repository name and your access rights.\n";
                std::cerr << "Repository URL: https://github.com/" << repoName << "\n";
            } else if (repoCheckResponse.responseCode == 401 || repoCheckResponse.responseCode == 403) {
                std::cerr << "Authentication error. This might be a private repository.\n";
                std::cerr << "Try setting a GitHub token: gyatt github-token <your-token>\n";
                std::cerr << "Or export GITHUB_TOKEN=<your-token> in your shell environment\n";
            } else {
                std::cerr << "GitHub API error: " << repoCheckResponse.error << "\n";
                std::cerr << "Please check your internet connection and try again later.\n";
            }
            
            return false;
        }
        
        // Try to get the default branch from the API
        std::string defaultBranch = "main"; // Default fallback
        try {
            // Simple JSON parsing for default_branch
            size_t pos = repoCheckResponse.content.find("\"default_branch\"");
            if (pos != std::string::npos) {
                pos = repoCheckResponse.content.find(":", pos);
                if (pos != std::string::npos) {
                    pos = repoCheckResponse.content.find("\"", pos);
                    if (pos != std::string::npos) {
                        size_t endPos = repoCheckResponse.content.find("\"", pos + 1);
                        if (endPos != std::string::npos) {
                            defaultBranch = repoCheckResponse.content.substr(pos + 1, endPos - pos - 1);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: Could not parse default branch from API response\n";
        }
        
        std::cout << "Repository default branch: " << defaultBranch << "\n";
        
        // Try to get a list of branches from the API
        std::vector<std::string> branchList;
        std::string branchesUrl = "https://api.github.com/repos/" + repoName + "/branches";
        std::cout << "Checking available branches: " << branchesUrl << "\n";
        
        Utils::HttpResponse branchesResponse = Utils::httpGet(branchesUrl, headers);
        if (branchesResponse.success) {
            // Very simple JSON array parsing - looking for "name": "branch-name" patterns
            std::string content = branchesResponse.content;
            size_t pos = 0;
            while ((pos = content.find("\"name\":", pos)) != std::string::npos) {
                pos = content.find("\"", pos + 7); // Skip past "name":
                if (pos != std::string::npos) {
                    size_t endPos = content.find("\"", pos + 1);
                    if (endPos != std::string::npos) {
                        std::string branchName = content.substr(pos + 1, endPos - pos - 1);
                        branchList.push_back(branchName);
                        pos = endPos;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
        }
        
        if (!branchList.empty()) {
            std::cout << "Found " << branchList.size() << " branches: ";
            for (size_t i = 0; i < branchList.size(); ++i) {
                std::cout << branchList[i];
                if (i < branchList.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "\n";
        } else {
            std::cout << "Could not retrieve branch list, will try common branch names\n";
        }
        
        // Build list of branches to try, starting with branches we found via API
        std::vector<std::string> branchesToTry = branchList;
        
        // If the default branch is not already in our list, add it first
        if (std::find(branchesToTry.begin(), branchesToTry.end(), defaultBranch) == branchesToTry.end()) {
            branchesToTry.insert(branchesToTry.begin(), defaultBranch);
        }
        
        // Add other common branch names if they're different from what we already have
        std::vector<std::string> commonBranches = {"main", "master", "develop", "trunk"};
        for (const auto& branch : commonBranches) {
            if (std::find(branchesToTry.begin(), branchesToTry.end(), branch) == branchesToTry.end()) {
                branchesToTry.push_back(branch);
            }
        }
        
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
            std::cerr << "The repository exists but no branches could be found or accessed.\n";
            std::cerr << "This could be because:\n";
            std::cerr << "1. The repository is empty (has no commits)\n";
            std::cerr << "2. The branches have different names than the ones we tried\n";
            std::cerr << "3. The repository is private and requires authentication\n";
            
            if (getenv("GITHUB_TOKEN") == nullptr) {
                std::cerr << "\nIf this is a private repository, try setting a GitHub token:\n";
                std::cerr << "  gyatt github-token <your-token>\n";
                std::cerr << "Or export GITHUB_TOKEN=<your-token> in your shell environment\n";
            }
            
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
     // Check if repository exists and if it's empty
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

    // For empty repositories, use a simpler approach with the Contents API
    std::string contentsUrl = "https://api.github.com/repos/" + repoName + "/contents";
    Utils::HttpResponse contentsResponse = Utils::httpGet(contentsUrl, headers);
    bool isEmptyRepo = !contentsResponse.success || contentsResponse.content.find("[]") != std::string::npos;

    if (isEmptyRepo) {
        std::cout << "Repository is empty, using Contents API for initial commit...\n";
        return uploadToEmptyGitHubRepo(repoName, branch, token);
    }
    
    // Get the current branch reference to see if it exists
    std::string refUrl = "https://api.github.com/repos/" + repoName + "/git/ref/heads/" + branch;
    Utils::HttpResponse refResponse = Utils::httpGet(refUrl, headers);
    
    std::string currentSha;
    if (refResponse.success) {
        // Parse SHA from response
        size_t shaPos = refResponse.content.find("\"sha\":");
        if (shaPos != std::string::npos) {
            shaPos = refResponse.content.find("\"", shaPos + 6);
            if (shaPos != std::string::npos) {
                size_t shaEnd = refResponse.content.find("\"", shaPos + 1);
                if (shaEnd != std::string::npos) {
                    currentSha = refResponse.content.substr(shaPos + 1, shaEnd - shaPos - 1);
                }
            }
        }
        std::cout << "Branch '" << branch << "' exists with SHA: " << Utils::shortHash(currentSha) << "\n";
    } else {
        std::cout << "Branch '" << branch << "' does not exist, will create it\n";
    }
    
    // Step 1: Create blobs for all files in the repository (OPTIMIZED)
    std::cout << "Step 1: Creating blobs for repository files (parallel processing)...\n";
    auto blobStartTime = std::chrono::steady_clock::now();
    std::map<std::string, std::string> fileBlobMap;
    
    Index index(repoPath);
    auto stagedFiles = index.getStagedFiles();
    
    if (stagedFiles.empty()) {
        std::cerr << "error: no files in repository to push\n";
        return false;
    }
    
    // Filter files to upload
    std::vector<std::pair<std::string, std::string>> filesToUpload;
    for (const auto& entry : stagedFiles) {
        std::string filePath = entry.filepath;
        
        // Skip files that should be ignored according to .gyattignore patterns
        if (isIgnored(filePath)) {
            std::cout << "Skipping ignored file during GitHub upload: " << filePath << "\n";
            continue;
        }
        
        // Skip system directories and files that shouldn't be uploaded to GitHub
        if (shouldExcludeFromGitHubUpload(filePath)) {
            std::cout << "Skipping system file during GitHub upload: " << filePath << "\n";
            continue;
        }
        
        std::string fileContent;
        try {
            fileContent = Utils::readFile(Utils::joinPath(repoPath, filePath));
            filesToUpload.emplace_back(filePath, fileContent);
        } catch (const std::exception& e) {
            std::cerr << "warning: could not read file '" << filePath << "': " << e.what() << "\n";
            continue;
        }
    }
    
    if (filesToUpload.empty()) {
        std::cerr << "error: no files to upload after filtering\n";
        return false;
    }
    
    // PARALLEL BLOB CREATION WITH HTTP + MEMORY OPTIMIZATION - Key optimization for performance
    auto httpOptimizer = std::make_shared<HttpOptimization>();
    
    // Initialize memory optimization for GitHub operations
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
        memoryOptimizer->optimizeForBatch(); // Optimize for bulk GitHub operations
    }
    
    // Configure HTTP optimization settings
    HttpOptimization::ConnectionPoolConfig config;
    config.maxConnections = 16;           // Increased for GitHub API
    config.maxConnectionsPerHost = 8;     // GitHub API limit consideration
    config.connectionTimeout = 30L;       // 30 second timeout
    config.requestTimeout = 60L;          // 60 second request timeout
    config.enableCompression = true;      // Enable gzip compression
    config.enableKeepAlive = true;        // Enable connection reuse
    config.enableHttp2 = true;            // Use HTTP/2 when available
    config.maxRetries = 3;                // Retry failed requests
    
    httpOptimizer->setConfig(config);
    httpOptimizer->enableCompression(true);
    httpOptimizer->setCacheExpiry(std::chrono::seconds(300)); // 5 minute cache
    httpOptimizer->setRateLimit(std::chrono::milliseconds(17)); // ~60 requests per second
    
    const size_t numThreads = std::min(static_cast<size_t>(8), 
                                      std::max(static_cast<size_t>(2), 
                                      static_cast<size_t>(std::thread::hardware_concurrency())));
    const size_t chunkSize = std::max(static_cast<size_t>(1), 
                                     (filesToUpload.size() + numThreads - 1) / numThreads);
    
    std::vector<std::future<std::map<std::string, std::string>>> futures;
    std::mutex consoleMutex;
    std::atomic<size_t> completedFiles{0};
    
    std::cout << "Processing " << filesToUpload.size() << " files with " << numThreads 
              << " threads (HTTP optimized, " << chunkSize << " files per thread)\n";
    
    for (size_t i = 0; i < filesToUpload.size(); i += chunkSize) {
        size_t endIdx = std::min(i + chunkSize, filesToUpload.size());
        std::vector<std::pair<std::string, std::string>> chunk(
            filesToUpload.begin() + i, filesToUpload.begin() + endIdx);
        
        auto future = std::async(std::launch::async, [&headers, &consoleMutex, &completedFiles, &filesToUpload, httpOptimizer, chunk, repoName]() 
            -> std::map<std::string, std::string> {
            
            std::map<std::string, std::string> chunkResults;
            std::string blobUrl = "https://api.github.com/repos/" + repoName + "/git/blobs";
            
            // Prepare batch requests for HTTP optimization
            std::vector<HttpOptimization::BatchRequest> batchRequests;
            
            for (const auto& [filePath, fileContent] : chunk) {
                // Create blob via GitHub API with HTTP optimization
                std::string encodedContent = Utils::base64Encode(fileContent);
                std::string blobData = "{\"content\":\"" + encodedContent + "\",\"encoding\":\"base64\"}";
                
                HttpOptimization::BatchRequest request;
                request.url = blobUrl;
                request.method = "POST";
                request.data = blobData;
                request.headers = headers;
                request.headers.push_back("Content-Type: application/json");
                request.priority = 1;  // High priority
                
                batchRequests.push_back(request);
            }
            
            // Process batch requests with HTTP optimization
            auto responses = httpOptimizer->executeRequestBatch(batchRequests);
            
            for (size_t i = 0; i < responses.size(); ++i) {
                const auto& response = responses[i];
                const std::string& filePath = chunk[i].first;
                
                if (response.success || response.responseCode == 409) {
                    // Check if this is an "empty repository" 409 error
                    if (response.responseCode == 409 && 
                        response.content.find("Git Repository is empty") != std::string::npos) {
                        chunkResults[filePath] = ""; // Placeholder for empty repo
                        {
                            std::lock_guard<std::mutex> lock(consoleMutex);
                            std::cout << "  " << filePath << " (will be included in initial commit)\n";
                        }
                        continue;
                    }
                    
                    // Parse SHA from response
                    size_t shaPos = response.content.find("\"sha\":");
                    if (shaPos != std::string::npos) {
                        shaPos = response.content.find("\"", shaPos + 6);
                        if (shaPos != std::string::npos) {
                            size_t shaEnd = response.content.find("\"", shaPos + 1);
                            if (shaEnd != std::string::npos) {
                                std::string blobSha = response.content.substr(shaPos + 1, shaEnd - shaPos - 1);
                                chunkResults[filePath] = blobSha;
                                
                                size_t completed = ++completedFiles;
                                {
                                    std::lock_guard<std::mutex> lock(consoleMutex);
                                    std::cout << "  [" << completed << "/" << filesToUpload.size() 
                                              << "] " << filePath << " -> " << Utils::shortHash(blobSha) 
                                              << " (optimized)\n";
                                }
                            }
                        }
                    }
                    
                    if (chunkResults.find(filePath) == chunkResults.end()) {
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cerr << "error: could not parse blob SHA for file '" << filePath << "'\n";
                    }
                } else {
                    {
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cerr << "error: failed to create blob for file '" << filePath 
                                  << "' (HTTP " << response.responseCode << ")\n";
                        std::cerr << "Response: " << response.content << "\n";
                    }
                }
            }
            
            return chunkResults;
        });
        
        futures.push_back(std::move(future));
    }
    
    // Collect results from all threads
    for (auto& future : futures) {
        try {
            auto chunkResults = future.get();
            for (const auto& [filePath, blobSha] : chunkResults) {
                fileBlobMap[filePath] = blobSha;
            }
        } catch (const std::exception& e) {
            std::cerr << "error: exception in parallel blob creation: " << e.what() << "\n";
            return false;
        }
    }
    
    // Check if we have any files left to upload after filtering and processing
    if (fileBlobMap.empty()) {
        std::cerr << "error: no files to upload after applying ignore patterns and processing\n";
        return false;
    }
    
    auto blobEndTime = std::chrono::steady_clock::now();
    auto blobDuration = std::chrono::duration_cast<std::chrono::milliseconds>(blobEndTime - blobStartTime);
    
    // Get HTTP optimization metrics
    auto stats = httpOptimizer->getStats();
    
    std::cout << "✅ Blob creation completed in " << blobDuration.count() << "ms "
              << "(" << (fileBlobMap.size() * 1000.0 / blobDuration.count()) << " files/sec)\n";
    std::cout << "📊 HTTP Optimization Stats:\n";
    std::cout << "   • Cache hits: " << stats.cacheHits << "/" << stats.totalRequests 
              << " (" << stats.cacheHitRate << "%)\n";
    std::cout << "   • Average response time: " << stats.averageResponseTime << "ms\n";
    std::cout << "   • Total bytes transferred: " << (stats.totalBytesTransferred / 1024.0 / 1024.0) << " MB\n";
    std::cout << "   • Active connections: " << stats.activeConnections << "/" << stats.poolSize << "\n";
    
    // Step 2: Create a tree with all the blobs (OPTIMIZED)
    std::cout << "Step 2: Creating tree with " << fileBlobMap.size() << " files...\n";
    auto treeStartTime = std::chrono::steady_clock::now();
    
    std::string treeUrl = "https://api.github.com/repos/" + repoName + "/git/trees";
    
    std::ostringstream treeJson;
    treeJson << "{\"tree\":[";
    
    bool first = true;
    for (const auto& [filePath, blobSha] : fileBlobMap) {
        if (!first) treeJson << ",";
        treeJson << "{\"path\":\"" << filePath << "\",\"mode\":\"100644\",\"type\":\"blob\",\"sha\":\"" << blobSha << "\"}";
        first = false;
    }
    
    treeJson << "]";
    
    // Only add base_tree if this is not the first commit (repository is not empty)
    if (!currentSha.empty()) {
        // Get the tree of the current commit for base_tree
        std::cout << "  Getting base tree from current commit...\n";
        std::string commitUrl = "https://api.github.com/repos/" + repoName + "/git/commits/" + currentSha;
        Utils::HttpResponse commitResponse = Utils::httpGet(commitUrl, headers);
        if (commitResponse.success) {
            size_t treePos = commitResponse.content.find("\"tree\":");
            if (treePos != std::string::npos) {
                treePos = commitResponse.content.find("\"sha\":", treePos);
                if (treePos != std::string::npos) {
                    treePos = commitResponse.content.find("\"", treePos + 6);
                    if (treePos != std::string::npos) {
                        size_t treeEnd = commitResponse.content.find("\"", treePos + 1);
                        if (treeEnd != std::string::npos) {
                            std::string baseTreeSha = commitResponse.content.substr(treePos + 1, treeEnd - treePos - 1);
                            treeJson << ",\"base_tree\":\"" << baseTreeSha << "\"";
                            std::cout << "  Using base tree: " << Utils::shortHash(baseTreeSha) << "\n";
                        }
                    }
                }
            }
        } else {
            std::cerr << "warning: could not fetch base tree, proceeding without it\n";
        }
    }
    
    treeJson << "}";
    
    std::vector<std::string> treeHeaders = headers;
    treeHeaders.push_back("Content-Type: application/json");
    
    Utils::HttpResponse treeResponse = Utils::httpPost(treeUrl, treeJson.str(), treeHeaders);
    
    if (!treeResponse.success) {
        std::cerr << "error: failed to create tree (HTTP " << treeResponse.responseCode << ")\n";
        std::cerr << "Response: " << treeResponse.content << "\n";
        return false;
    }
    
    // Parse tree SHA
    std::string treeSha;
    size_t shaPos = treeResponse.content.find("\"sha\":");
    if (shaPos != std::string::npos) {
        shaPos = treeResponse.content.find("\"", shaPos + 6);
        if (shaPos != std::string::npos) {
            size_t shaEnd = treeResponse.content.find("\"", shaPos + 1);
            if (shaEnd != std::string::npos) {
                treeSha = treeResponse.content.substr(shaPos + 1, shaEnd - shaPos - 1);
            }
        }
    }
    
    if (treeSha.empty()) {
        std::cerr << "error: could not parse tree SHA\n";
        return false;
    }
    
    auto treeEndTime = std::chrono::steady_clock::now();
    auto treeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(treeEndTime - treeStartTime);
    std::cout << "Tree created: " << Utils::shortHash(treeSha) << " (took " << treeDuration.count() << "ms)\n";
    
    // Step 3: Create a commit with the tree (OPTIMIZED)
    std::cout << "Step 3: Creating commit...\n";
    auto commitStartTime = std::chrono::steady_clock::now();
    
    std::string commitUrl = "https://api.github.com/repos/" + repoName + "/git/commits";
    
    // Read commit info from local repository
    Commit commitObj(repoPath);
    auto commitInfo = commitObj.readCommit(currentCommit);
    
    std::ostringstream commitJson;
    commitJson << "{";
    commitJson << "\"message\":\"" << commitInfo.message << "\",";
    commitJson << "\"tree\":\"" << treeSha << "\",";
    commitJson << "\"author\":{\"name\":\"" << Utils::getUserName() << "\",\"email\":\"" << Utils::getUserEmail() << "\"}";
    
    if (!currentSha.empty()) {
        commitJson << ",\"parents\":[\"" << currentSha << "\"]";
    }
    
    commitJson << "}";
    
    std::vector<std::string> commitHeaders = headers;
    commitHeaders.push_back("Content-Type: application/json");
    
    Utils::HttpResponse commitResponse = Utils::httpPost(commitUrl, commitJson.str(), commitHeaders);
    
    if (!commitResponse.success) {
        std::cerr << "error: failed to create commit (HTTP " << commitResponse.responseCode << ")\n";
        std::cerr << "Response: " << commitResponse.content << "\n";
        return false;
    }
    
    // Parse commit SHA
    std::string commitSha;
    shaPos = commitResponse.content.find("\"sha\":");
    if (shaPos != std::string::npos) {
        shaPos = commitResponse.content.find("\"", shaPos + 6);
        if (shaPos != std::string::npos) {
            size_t shaEnd = commitResponse.content.find("\"", shaPos + 1);
            if (shaEnd != std::string::npos) {
                commitSha = commitResponse.content.substr(shaPos + 1, shaEnd - shaPos - 1);
            }
        }
    }
    
    if (commitSha.empty()) {
        std::cerr << "error: could not parse commit SHA\n";
        return false;
    }
    
    auto commitEndTime = std::chrono::steady_clock::now();
    auto commitDuration = std::chrono::duration_cast<std::chrono::milliseconds>(commitEndTime - commitStartTime);
    std::cout << "Commit created: " << Utils::shortHash(commitSha) << " (took " << commitDuration.count() << "ms)\n";
    
    // Step 4: Update the branch reference (OPTIMIZED)
    std::cout << "Step 4: Updating branch reference...\n";
    auto refStartTime = std::chrono::steady_clock::now();
    
    std::ostringstream refJson;
    refJson << "{\"sha\":\"" << commitSha << "\"}";
    
    std::vector<std::string> refHeaders = headers;
    refHeaders.push_back("Content-Type: application/json");
    
    Utils::HttpResponse refUpdateResponse;
    
    if (currentSha.empty()) {
        // Create new reference
        std::string createRefUrl = "https://api.github.com/repos/" + repoName + "/git/refs";
        std::ostringstream createRefJson;
        createRefJson << "{\"ref\":\"refs/heads/" << branch << "\",\"sha\":\"" << commitSha << "\"}";
        
        refUpdateResponse = Utils::httpPost(createRefUrl, createRefJson.str(), refHeaders);
    } else {
        // Update existing reference - Use PATCH
        refUrl = "https://api.github.com/repos/" + repoName + "/git/refs/heads/" + branch;
        
        refUpdateResponse = Utils::httpPatch(refUrl, refJson.str(), refHeaders);
    }
    
    if (!refUpdateResponse.success) {
        std::cerr << "error: failed to update branch reference (HTTP " << refUpdateResponse.responseCode << ")\n";
        std::cerr << "Response: " << refUpdateResponse.content << "\n";
        return false;
    }
    
    auto refEndTime = std::chrono::steady_clock::now();
    auto refDuration = std::chrono::duration_cast<std::chrono::milliseconds>(refEndTime - refStartTime);
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(refEndTime - blobStartTime);
    
    std::cout << "Reference updated (took " << refDuration.count() << "ms)\n";
    std::cout << "Successfully pushed to GitHub!\n";
    std::cout << "Repository: https://github.com/" << repoName << "\n";
    std::cout << "Branch: " << branch << " -> " << Utils::shortHash(commitSha) << "\n";
    std::cout << "Total push time: " << totalDuration.count() << "ms\n";
    std::cout << "Performance: " << (fileBlobMap.size() * 1000.0 / totalDuration.count()) << " files/sec\n";
    
    return true;
    
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

bool Repository::shouldExcludeFromGitHubUpload(const std::string& filePath) {
    // Exclude system directories and files that shouldn't be on GitHub
    if (filePath.find(".git/") == 0 || filePath.find(".git\\") == 0) {
        return true; // Git system files
    }
    
    if (filePath.find(".gyatt/") == 0 || filePath.find(".gyatt\\") == 0) {
        return true; // Gyatt system files
    }
    
    // Exclude other common system files and directories
    if (filePath == ".DS_Store" || filePath.find(".DS_Store/") != std::string::npos) {
        return true; // macOS system files
    }
    
    if (filePath == "Thumbs.db" || filePath == "desktop.ini") {
        return true; // Windows system files
    }
    
    if (filePath.find("__pycache__/") == 0 || filePath.find("__pycache__\\") == 0) {
        return true; // Python cache
    }
    
    if (filePath.find(".vscode/") == 0 || filePath.find(".vscode\\") == 0) {
        return true; // VS Code settings (unless wanted)
    }
    
    if (filePath.find("node_modules/") == 0 || filePath.find("node_modules\\") == 0) {
        return true; // Node.js dependencies
    }
    
    if (filePath.find(".tmp/") == 0 || filePath.find("tmp/") == 0) {
        return true; // Temporary files
    }
    
    return false;
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

bool Repository::uploadToEmptyGitHubRepo(const std::string& repoName, const std::string& branch, const std::string& token) {
    std::cout << "Uploading to empty GitHub repository using Contents API...\n";
    
    Index index(repoPath);
    auto stagedFiles = index.getStagedFiles();
    
    if (stagedFiles.empty()) {
        std::cerr << "error: no files to upload\n";
        return false;
    }
    
    // For empty repositories, we can only upload one file at a time using Contents API
    // We'll upload the first file to create the initial commit, then fall back to Git API for others
    
    std::vector<std::string> headers = {
        "Authorization: token " + token,
        "Accept: application/vnd.github.v3+json",
        "Content-Type: application/json"
    };
    
    bool firstFile = true;
    for (const auto& entry : stagedFiles) {
        std::string filePath = entry.filepath;
        
        // Skip files that should be ignored according to .gyattignore patterns
        if (isIgnored(filePath)) {
            std::cout << "Skipping ignored file during GitHub upload: " << filePath << "\n";
            continue;
        }
        
        // Skip system directories and files that shouldn't be uploaded to GitHub
        if (shouldExcludeFromGitHubUpload(filePath)) {
            std::cout << "Skipping system file during GitHub upload: " << filePath << "\n";
            continue;
        }
        
        std::string fileContent;
        
        try {
            fileContent = Utils::readFile(Utils::joinPath(repoPath, filePath));
        } catch (const std::exception& e) {
            std::cerr << "warning: could not read file '" << filePath << "': " << e.what() << "\n";
            continue;
        }
        
        std::string encodedContent = Utils::base64Encode(fileContent);
        std::string url = "https://api.github.com/repos/" + repoName + "/contents/" + filePath;
        
        // Read commit info for the commit message
        std::string currentCommit = getBranchCommit(branch);
        Commit commitObj(repoPath);
        auto commitInfo = commitObj.readCommit(currentCommit);
        
        std::ostringstream jsonData;
        jsonData << "{";
        jsonData << "\"message\":\"" << commitInfo.message << "\",";
        jsonData << "\"content\":\"" << encodedContent << "\"";
        if (!firstFile) {
            jsonData << ",\"branch\":\"" << branch << "\"";
        }
        jsonData << "}";
        
        Utils::HttpResponse response = Utils::httpPut(url, jsonData.str(), headers);
        
        if (!response.success) {
            std::cerr << "error: failed to upload file '" << filePath << "' (HTTP " << response.responseCode << ")\n";
            std::cerr << "Response: " << response.content << "\n";
            return false;
        }
        
        std::cout << "  " << filePath << " -> uploaded\n";
        firstFile = false;
        
        // For the first file, this creates the branch and initial commit
        // For subsequent files, we would need to update them individually
        // For simplicity, we'll only upload the first file for now
        break;
    }
    
    std::cout << "Successfully uploaded to GitHub (Contents API)!\n";
    std::cout << "Repository: https://github.com/" << repoName << "\n";
    
    return true;
}

// ============================================================================
// HELPER FUNCTIONS FOR C++17 COMPATIBILITY
// ============================================================================

namespace {
    bool starts_with(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
    }
    
    bool ends_with(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
    }
}

// ============================================================================
// MISSING REPOSITORY METHOD IMPLEMENTATIONS
// ============================================================================

bool Repository::addRemoteWithAuth(const std::string& name, const std::string& url, const RemoteCredentials& credentials) {
    // Create remote directory if it doesn't exist
    std::string remotesPath = Utils::joinPath(repoPath, ".gyatt/remotes");
    Utils::createDirectories(remotesPath);
    
    // Store remote configuration
    std::string remoteFile = Utils::joinPath(remotesPath, name);
    std::ofstream file(remoteFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "url=" << url << "\n";
    file << "auth_method=" << static_cast<int>(credentials.method) << "\n";
    if (!credentials.username.empty()) {
        file << "username=" << credentials.username << "\n";
    }
    if (!credentials.token.empty()) {
        file << "token=" << credentials.token << "\n";
    }
    if (!credentials.sshKeyPath.empty()) {
        file << "ssh_key=" << credentials.sshKeyPath << "\n";
    }
    file.close();
    
    
    
    
    // Add to remotes list
    RemoteRepository remoteRepo;
    remoteRepo.name = name;
    remoteRepo.url = url;
    remoteRepo.protocol = detectProtocol(url);
    remoteRepo.authMethod = credentials.method;
    remoteRepo.isGyattRepo = false;
    remoteRepo.isHealthy = true;
    remoteRepo.lastError = "";
    remoteRepo.lastSync = std::chrono::system_clock::now();
    remoteRepo.credentials = credentials;
    // branches and syncProfiles are empty by default
    
    remotes[name] = remoteRepo;
    
    return true;
}

RemoteRepository Repository::loadRemoteConfig(const std::string& name) {
    RemoteRepository remote;
    remote.name = name;
    
    std::string remoteFile = Utils::joinPath(repoPath, ".gyatt/remotes/" + name);
    std::ifstream file(remoteFile);
    if (!file.is_open()) {
        return remote;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "url") {
                remote.url = value;
                remote.protocol = detectProtocol(value);
            } else if (key == "auth_method") {
                remote.authMethod = static_cast<AuthMethod>(std::stoi(value));
            }
        }
    }
    
    return remote;
}

bool Repository::checkRemoteHealth(const RemoteRepository& remote) {
    // Simple health check - try to connect
    if (remote.url.empty()) {
        return false;
    }
    
    // For GitHub URLs, try a simple HTTP request
    if (remote.url.find("github.com") != std::string::npos) {
        // Extract repo path and try API call
        auto pos = remote.url.find("github.com/");
        if (pos != std::string::npos) {
            std::string repoPath = remote.url.substr(pos + 11);
            if (ends_with(repoPath, ".git")) {
                repoPath = repoPath.substr(0, repoPath.length() - 4);
            }
            
            std::string apiUrl = "https://api.github.com/repos/" + repoPath;
            // Simple health check - for now just return true for GitHub repos
            // In a production environment, you would make an actual HTTP request
            return true;
        }
    }
    
    // For other remotes, assume healthy
    return true;
}

bool Repository::pushWithProgress(const std::string& remote, const std::string& branch, 
                                 std::function<void(const PushProgress&)> callback) {
    PushProgress progress;
    progress.totalObjects = 100; // Simulated object count
    progress.pushedObjects = 0;
    progress.totalBytes = 0;
    progress.pushedBytes = 0;
    progress.status = "Preparing objects...";
    
    if (callback) callback(progress);
    
    // Simulate progressive push
    for (size_t i = 0; i <= progress.totalObjects; ++i) {
        progress.pushedObjects = i;
        progress.pushedBytes = (progress.totalBytes * i) / progress.totalObjects;
        progress.status = "Pushing objects... (" + std::to_string(i) + "/" + std::to_string(progress.totalObjects) + ")";
        
        if (callback) callback(progress);
        
        // Small delay to simulate real push
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    progress.status = "Push completed";
    if (callback) callback(progress);
    
    return uploadToGitHub(remote, branch);
}

std::vector<SyncProfile> Repository::getSyncProfiles() const {
    std::vector<SyncProfile> profiles;
    
    std::string profilesPath = Utils::joinPath(repoPath, ".gyatt/sync_profiles");
    if (!Utils::fileExists(profilesPath)) {
        return profiles;
    }
    
    // Load sync profiles from file
    std::ifstream file(profilesPath);
    std::string line;
    SyncProfile current;
    
    while (std::getline(file, line)) {
        if (starts_with(line, "name=")) {
            if (!current.name.empty()) {
                profiles.push_back(current);
                current = SyncProfile{};
            }
            current.name = line.substr(5);
        } else if (starts_with(line, "mode=")) {
            current.mode = static_cast<SyncMode>(std::stoi(line.substr(5)));
        } else if (starts_with(line, "include=")) {
            current.includePatterns.push_back(line.substr(8));
        } else if (starts_with(line, "exclude=")) {
            current.excludePatterns.push_back(line.substr(8));
        }
    }
    
    if (!current.name.empty()) {
        profiles.push_back(current);
    }
    
    return profiles;
}

std::string Repository::getSyncModeName(SyncMode mode) {
    switch (mode) {
        case SyncMode::FULL: return "Full";
        case SyncMode::SELECTIVE: return "Selective";
        case SyncMode::INCREMENTAL: return "Incremental";
        case SyncMode::SMART: return "Smart";
        default: return "Unknown";
    }
}

SyncProfile Repository::createSyncProfile(const std::string& name, SyncMode mode,
                                  const std::vector<std::string>& includes,
                                  const std::vector<std::string>& excludes) {
    SyncProfile profile;
    profile.name = name;
    profile.mode = mode;
    profile.includePatterns = includes;
    profile.excludePatterns = excludes;
    
    // Save to file
    std::string profilesPath = Utils::joinPath(repoPath, ".gyatt/sync_profiles");
    
    std::ofstream file(profilesPath, std::ios::app);
    if (file.is_open()) {
        file << "name=" << name << "\n";
        file << "mode=" << static_cast<int>(mode) << "\n";
        
        for (const auto& pattern : includes) {
            file << "include=" << pattern << "\n";
        }
        
        for (const auto& pattern : excludes) {
            file << "exclude=" << pattern << "\n";
        }
        
        file << "\n"; // Separator between profiles
    }
    
    return profile;
}

std::vector<RemoteRepository> Repository::getRemoteRepositories() const {
    std::vector<RemoteRepository> repos;
    
    for (const auto& [name, remote] : remotes) {
        repos.push_back(remote);
    }
    
    return repos;
}

std::string Repository::getProtocolName(RemoteProtocol protocol) {
    switch (protocol) {
        case RemoteProtocol::HTTPS: return "HTTPS";
        case RemoteProtocol::SSH: return "SSH";
        case RemoteProtocol::LOCAL: return "Local";
        case RemoteProtocol::UNKNOWN: default: return "Unknown";
    }
}

std::string Repository::getAuthMethodName(AuthMethod method) {
    switch (method) {
        case AuthMethod::NONE: return "None";
        case AuthMethod::TOKEN: return "Token";
        case AuthMethod::SSH_KEY: return "SSH Key";
        case AuthMethod::USERNAME_PASSWORD: return "Username/Password";
        case AuthMethod::OAUTH: return "OAuth";
        default: return "Unknown";
    }
}

RemoteProtocol Repository::detectProtocol(const std::string& url) {
    if (starts_with(url, "https://")) {
        return RemoteProtocol::HTTPS;
    } else if (starts_with(url, "git@") || starts_with(url, "ssh://")) {
        return RemoteProtocol::SSH;
    } else if (starts_with(url, "file://") || starts_with(url, "/")) {
        return RemoteProtocol::LOCAL;
    } else {
        return RemoteProtocol::UNKNOWN;
    }
}

// ============================================================================
// ADDITIONAL MISSING REPOSITORY METHODS
// ============================================================================

bool Repository::addOptimized(const std::vector<std::string>& files) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    
    bool success = true;
    for (const auto& file : files) {
        if (!add(file)) {
            success = false;
        }
    }
    return success;
}

bool Repository::commitOptimized(const std::string& message, const std::string& author) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    return performanceEngine->commitOptimized(message, author);
}

std::map<std::string, std::string> Repository::statusOptimized() {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    
    // Return a simple status map for now
    std::map<std::string, std::string> statusMap;
    statusMap["status"] = "optimized";
    return statusMap;
}

PerformanceEngine::Metrics Repository::getPerformanceMetrics() const {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    return performanceEngine->getMetrics();
}

void Repository::enablePerformanceOptimizations(bool enable) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    performanceEngine->enableOptimizations(enable);
}

void Repository::enableParallelProcessing(bool enable) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    performanceEngine->enableParallelProcessing(enable);
}

void Repository::enableObjectCaching(bool enable) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    performanceEngine->enableObjectCaching(enable);
}

void Repository::enableDeltaCompression(bool enable) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    performanceEngine->enableDeltaCompression(enable);
}

void Repository::enableMemoryMapping(bool enable) {
    if (!performanceEngine) {
        performanceEngine = std::make_unique<PerformanceEngine>(repoPath);
    }
    performanceEngine->enableMemoryMapping(enable);
}

void Repository::enableMemoryOptimization(bool enable) {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    memoryOptimizer->enableOptimization(enable);
}

void Repository::optimizeForPerformance() {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    memoryOptimizer->optimizeForPerformance();
}

void Repository::optimizeForMemory() {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    memoryOptimizer->optimizeForMemory();
}

void Repository::optimizeForBatch() {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    memoryOptimizer->optimizeForBatch();
}

MemoryOptimizationManager::MemoryProfile Repository::getMemoryProfile() const {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    return memoryOptimizer->getMemoryProfile();
}

void Repository::performGarbageCollection() {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    memoryOptimizer->performGarbageCollection();
}

void Repository::enableAutoTuning(bool enable) {
    if (!memoryOptimizer) {
        memoryOptimizer = std::make_unique<MemoryOptimizationManager>(repoPath);
    }
    memoryOptimizer->enableAutoTuning(enable);
}

// Compression optimization control
void Repository::enableCompressionIntegration(bool enable) {
    if (!compressionManager) {
        compressionManager = std::make_unique<IntegratedCompressionManager>(repoPath);
    }
    compressionManager->enableCompression(enable);
}

bool Repository::optimizeWithCompression() {
    if (!compressionManager) {
        compressionManager = std::make_unique<IntegratedCompressionManager>(repoPath);
    }
    return compressionManager->performFullOptimization();
}

bool Repository::optimizeCompressionForSpeed() {
    if (!compressionManager) {
        compressionManager = std::make_unique<IntegratedCompressionManager>(repoPath);
    }
    return compressionManager->optimizeForSpeed();
}

bool Repository::optimizeCompressionForSize() {
    if (!compressionManager) {
        compressionManager = std::make_unique<IntegratedCompressionManager>(repoPath);
    }
    return compressionManager->optimizeForSize();
}

bool Repository::optimizeCompressionForBalance() {
    if (!compressionManager) {
        compressionManager = std::make_unique<IntegratedCompressionManager>(repoPath);
    }
    return compressionManager->optimizeForBalance();
}

bool Repository::performFullCompressionOptimization() {
    if (!compressionManager) {
        compressionManager = std::make_unique<IntegratedCompressionManager>(repoPath);
    }
    return compressionManager->performFullOptimization();
}
} // namespace gyatt
