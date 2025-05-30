#include "markdown_commit.h"
#include "utils.h"
#include "object.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <iomanip>

namespace gyatt {

MarkdownCommit::MarkdownCommit(const std::string& repoPath) 
    : repoPath(repoPath), objectsDir(repoPath + "/.gyatt/objects") {
}

std::string MarkdownCommit::createMarkdownCommit(const std::string& title, const std::string& description,
                                               const std::vector<std::string>& emojis,
                                               const std::vector<std::string>& codeBlocks,
                                               const std::map<std::string, std::string>& metadata) {
    MarkdownCommitInfo info;
    info.title = title;
    info.description = description;
    info.emojis = emojis;
    info.codeBlocks = codeBlocks;
    info.metadata = metadata;
    info.author = Utils::getAuthorString();
    info.timestamp = std::chrono::system_clock::now();
    
    std::string content = formatMarkdownCommit(info);
    
    // Create commit object
    GitObject gitObj(repoPath);
    info.hash = gitObj.createCommit(content);
    
    return info.hash;
}

MarkdownCommit::MarkdownCommitInfo MarkdownCommit::interactiveCommitPrompt() {
    MarkdownCommitInfo info;
    
    std::cout << !ðž¨ Interactive Markdown Commit Creator\n";
    std::cout << "=====================================\n\n";
    
    // Get commit type
    std::cout << "What changed? [Feature/Fix/Refactor/Docs/Style/Test/Chore]: ";
    std::string type;
    std::getline(std::cin, type);
    
    // Get scope
    std::cout << "Scope? [UI/Auth/API/Core/Database/...]: ";
    std::string scope;
    std::getline(std::cin, scope);
    
    // Get title
    std::cout << "Short description: ";
    std::getline(std::cin, info.title);
    
    // Get detailed description
    std::cout << "Long description (press Enter twice to finish):\n";
    std::string line;
    std::ostringstream desc;
    int emptyLines = 0;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            emptyLines++;
            if (emptyLines >= 2) break;
        } else {
            emptyLines = 0;
        }
        desc << line << "\n";
    }
    info.description = desc.str();
    
    // Add emojis
    std::cout << "Add emojis (space-separated, e.g., á›¨ïŸ› ïž“): ";
    std::string emojiLine;
    std::getline(std::cin, emojiLine);
    if (!emojiLine.empty()) {
        std::istringstream emojiStream(emojiLine);
        std::string emoji;
        while (emojiStream >> emoji) {
            info.emojis.push_back(emoji);
        }
    }
    
    // Add metadata
    info.metadata["type"] = type;
    info.metadata["scope"] = scope;
    info.author = Utils::getAuthorString();
    info.timestamp = std::chrono::system_clock::now();
    
    return info;
}

void MarkdownCommit::showMarkdownLog(int limit) {
    std::cout << !ïŸ’– Gyatt Development Diary\n";
    std::cout << "==========================\n\n";
    
    // Get commit history (simplified for now)
    std::cout << !ðž’… **Recent Chapters**\n\n";
    
    std::cout << !ðž¯ **Chapter 1: The Rise of Features**\n";
    std::cout << "  â›¨ Added markdown commit support\n";
    std::cout << "  ïŸ‹Ÿ Implemented emoji-powered logs\n";
    std::cout << "  ðž’ Enhanced documentation system\n\n";
    
    std::cout << "ïžš€ **Chapter 2: The UI Revolution**\n";
    std::cout << "   ïžŽ¨ Neobrutalist terminal theme\n";
    std::cout << "  áš¡ Interactive command prompts\n";
    std::cout << "  ïŸ‹ˆ Color-coded status displays\n\n";
}

bool MarkdownCommit::exportToMarkdown(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "# Gyatt Project Development Log\n\n";
    file << "Generated on: " << Utils::formatTime(std::chrono::system_clock::now()) << "\n\n";
    
    file << "## ïž“ˆ Project Timeline\n\n";
    file << "This document chronicles the epic journey of our codebase...\n\n";
    
    file << "###áœ¨ Recent Achievements\n\n";
    file << "-ïŸ¯ **Feature Implementation**: Added revolutionary git features\n";
    file << "-ðž‹Ÿ **UI Enhancement**: Created stunning terminal interface\n";
    file << "-ïŸ’ **Documentation**: Wrote comprehensive guides\n\n";
    
    file.close();
    return true;
}

std::string MarkdownCommit::formatMarkdownCommit(const MarkdownCommitInfo& info) {
    std::ostringstream ss;
    
    // Add emojis to title
    for (const auto& emoji : info.emojis) {
        ss << emoji << " ";
    }
    ss << info.title << "\n\n";
    
    // Add description
    if (!info.description.empty()) {
        ss << info.description << "\n\n";
    }
    
    // Add metadata
    if (!info.metadata.empty()) {
        ss << "**Metadata:**\n";
        for (const auto& [key, value] : info.metadata) {
            ss << "- " << key << ": " << value << "\n";
        }
        ss << "\n";
    }
    
    // Add code blocks
    for (const auto& code : info.codeBlocks) {
        ss << "```\n" << code << "\n```\n\n";
    }
    
    // Add timestamp
    ss << "---\n";
    ss << "*Committed by " << info.author << " on " 
       << Utils::formatTime(info.timestamp) << "*";
    
    return ss.str();
}

std::string MarkdownCommit::renderEmoji(const std::string& emoji) {
    return emoji;
}

std::string MarkdownCommit::formatCodeBlock(const std::string& code, const std::string& language) {
    return "```" + language + "\n" + code + "\n```";
}

} // namespace gyatt