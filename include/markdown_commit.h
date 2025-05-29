#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace gyatt {

class MarkdownCommit {
public:
    struct MarkdownCommitInfo {
        std::string hash;
        std::string title;
        std::string description;
        std::vector<std::string> emojis;
        std::vector<std::string> codeBlocks;
        std::string author;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, std::string> metadata;
    };

    MarkdownCommit(const std::string& repoPath);
    
    // Create markdown-powered commits
    std::string createMarkdownCommit(const std::string& title, const std::string& description,
                                   const std::vector<std::string>& emojis = {},
                                   const std::vector<std::string>& codeBlocks = {},
                                   const std::map<std::string, std::string>& metadata = {});
    
    // Interactive commit prompt
    MarkdownCommitInfo interactiveCommitPrompt();
    
    // Parse markdown commit content
    MarkdownCommitInfo parseMarkdownCommit(const std::string& commitHash);
    
    // Generate human-readable log
    void showMarkdownLog(int limit = -1);
    
    // Export commit history to various formats
    bool exportToMarkdown(const std::string& filename);
    bool exportToPDF(const std::string& filename);
    bool exportToTimeline(const std::string& filename);
    
private:
    std::string repoPath;
    std::string objectsDir;
    
    std::string formatMarkdownCommit(const MarkdownCommitInfo& info);
    std::string renderEmoji(const std::string& emoji);
    std::string formatCodeBlock(const std::string& code, const std::string& language = "");
};

} // namespace gyatt
