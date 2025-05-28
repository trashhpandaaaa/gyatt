#include "commit.h"
#include "object.h"
#include "utils.h"
#include <sstream>
#include <stdexcept>

namespace gyatt {

Commit::Commit(const std::string& repoPath) 
    : repoPath(repoPath), objectsDir(Utils::joinPath(repoPath, ".gyatt/objects")) {
}

std::string Commit::createCommit(const std::string& message, const std::string& author,
                                const std::string& treeHash, const std::string& parentHash) {
    auto timestamp = std::chrono::system_clock::now();
    std::string commitContent = formatCommitContent(treeHash, parentHash, author, message, timestamp);
    
    GitObject gitObj(repoPath);
    return gitObj.createCommit(commitContent);
}

Commit::CommitInfo Commit::readCommit(const std::string& commitHash) {
    GitObject gitObj(repoPath);
    std::string content = gitObj.readCommit(commitHash);
    return parseCommitContent(content, commitHash);
}

std::vector<Commit::CommitInfo> Commit::getCommitHistory(const std::string& startCommit) {
    std::vector<CommitInfo> history;
    
    if (startCommit.empty()) {
        return history;
    }
    
    std::string currentCommit = startCommit;
    
    while (!currentCommit.empty() && isValidCommit(currentCommit)) {
        try {
            CommitInfo info = readCommit(currentCommit);
            history.push_back(info);
            currentCommit = info.parentHash;
        } catch (const std::exception&) {
            break;
        }
    }
    
    return history;
}

bool Commit::isValidCommit(const std::string& commitHash) {
    if (!Utils::isValidHash(commitHash)) {
        return false;
    }
    
    GitObject gitObj(repoPath);
    try {
        return gitObj.objectExists(commitHash) && 
               gitObj.getObjectType(commitHash) == ObjectType::COMMIT;
    } catch (const std::exception&) {
        return false;
    }
}

std::string Commit::formatCommitContent(const std::string& treeHash, const std::string& parentHash,
                                       const std::string& author, const std::string& message,
                                       const std::chrono::system_clock::time_point& timestamp) {
    std::ostringstream ss;
    
    ss << "tree " << treeHash << "\n";
    
    if (!parentHash.empty()) {
        ss << "parent " << parentHash << "\n";
    }
    
    std::string authorStr = author.empty() ? Utils::getAuthorString() : author;
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    
    ss << "author " << authorStr << " " << time_t << "\n";
    ss << "committer " << authorStr << " " << time_t << "\n";
    ss << "\n";
    ss << message << "\n";
    
    return ss.str();
}

Commit::CommitInfo Commit::parseCommitContent(const std::string& content, const std::string& hash) {
    CommitInfo info;
    info.hash = hash;
    
    std::istringstream ss(content);
    std::string line;
    bool inMessage = false;
    std::ostringstream messageBuilder;
    
    while (std::getline(ss, line)) {
        if (inMessage) {
            if (!messageBuilder.str().empty()) {
                messageBuilder << "\n";
            }
            messageBuilder << line;
        } else if (line.empty()) {
            inMessage = true;
        } else if (line.substr(0, 5) == "tree ") {
            info.treeHash = line.substr(5);
        } else if (line.substr(0, 7) == "parent ") {
            info.parentHash = line.substr(7);
        } else if (line.substr(0, 7) == "author ") {
            std::string authorLine = line.substr(7);
            size_t lastSpace = authorLine.find_last_of(' ');
            if (lastSpace != std::string::npos) {
                info.author = authorLine.substr(0, lastSpace);
                try {
                    auto timestamp = std::stoll(authorLine.substr(lastSpace + 1));
                    info.timestamp = std::chrono::system_clock::from_time_t(timestamp);
                } catch (const std::exception&) {
                    info.timestamp = std::chrono::system_clock::now();
                }
            } else {
                info.author = authorLine;
                info.timestamp = std::chrono::system_clock::now();
            }
        }
        // Skip committer line for now
    }
    
    info.message = messageBuilder.str();
    return info;
}

std::string Commit::getObjectPath(const std::string& hash) {
    if (hash.length() < 2) {
        throw std::runtime_error("Invalid hash length: " + hash);
    }
    
    std::string dir = hash.substr(0, 2);
    std::string file = hash.substr(2);
    return Utils::joinPath(Utils::joinPath(objectsDir, dir), file);
}

} // namespace gyatt
