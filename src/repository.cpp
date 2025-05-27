#include "repository.h"
#include "index.h"
#include "commit.h"
#include "object.h"
#include "utils.h"
#include <iostream>
#include <filesystem>

namespace gyatt {

Repository::Repository(const std::string& path) 
    : repoPath(Utils::absolutePath(path)) {
    gyattDir = Utils::joinPath(repoPath, ".gyatt");
    objectsDir = Utils::joinPath(gyattDir, "objects");
    refsDir = Utils::joinPath(gyattDir, "refs");
    headsDir = Utils::joinPath(refsDir, "heads");
    indexFile = Utils::joinPath(gyattDir, "index");
    headFile = Utils::joinPath(gyattDir, "HEAD");
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
                    
                    // Skip .gyatt directory
                    if (relPath.substr(0, 6) != ".gyatt") {
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
    auto branches = Utils::listDirectory(headsDir);
    
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
           Utils::createDirectories(headsDir);
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

} // namespace gyatt
