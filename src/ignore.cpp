#include "ignore.h"
#include "utils.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace gyatt {

IgnoreList::IgnoreList(const std::string& repoPath) 
    : repoPath(repoPath) {
    // Construct the path to the .gyattignore file
    ignorePath = Utils::joinPath(repoPath, ".gyattignore");
    load();
}

void IgnoreList::load() {
    // Clear any existing patterns
    patterns.clear();
    ignoreCache.clear();
    
    // Add default patterns
    patterns.push_back(".gyatt/");  // Always ignore .gyatt directory
    
    // If the ignore file doesn't exist, nothing more to load
    if (!Utils::fileExists(ignorePath)) {
        return;
    }
    
    // Read the ignore file
    std::string content = Utils::readFile(ignorePath);
    std::vector<std::string> lines = Utils::split(content, '\n');
    
    for (const auto& line : lines) {
        // Skip empty lines and comments
        std::string trimmed = Utils::trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }
        
        // Add the pattern
        patterns.push_back(trimmed);
    }
}

bool IgnoreList::isIgnored(const std::string& filepath) const {
    // Convert to relative path within the repository
    std::string relativePath = filepath;
    if (filepath.find(repoPath) == 0) {
        relativePath = filepath.substr(repoPath.length());
        if (!relativePath.empty() && (relativePath[0] == '/' || relativePath[0] == '\\')) {
            relativePath = relativePath.substr(1);
        }
    }
    
    // Check cache first
    if (ignoreCache.find(relativePath) != ignoreCache.end()) {
        return ignoreCache[relativePath];
    }
    
    // Always ignore the .gyatt directory
    if (relativePath.find(".gyatt") == 0 || 
        relativePath.find(".gyatt/") == 0 || 
        (relativePath.find("/") != std::string::npos && relativePath.find("/.gyatt/") != std::string::npos)) {
        ignoreCache[relativePath] = true;
        return true;
    }
    
    // Always ignore the .gyattignore file itself
    if (relativePath == ".gyattignore") {
        ignoreCache[relativePath] = true;
        return true;
    }
    
    // Check each pattern
    for (const auto& pattern : patterns) {
        if (matchesPattern(relativePath, pattern)) {
            ignoreCache[relativePath] = true;
            return true;
        }
        
        // Check each directory component against the pattern
        size_t pos = 0;
        std::string pathToCheck = relativePath;
        while ((pos = pathToCheck.find('/')) != std::string::npos) {
            std::string directory = pathToCheck.substr(0, pos) + "/";
            if (matchesPattern(directory, pattern)) {
                ignoreCache[relativePath] = true;
                return true;
            }
            pathToCheck = pathToCheck.substr(pos + 1);
        }
    }
    
    // Not ignored
    ignoreCache[relativePath] = false;
    return false;
}

bool IgnoreList::matchesPattern(const std::string& filepath, const std::string& pattern) const {
    // Check if pattern is negated (starts with !)
    bool negated = false;
    std::string actualPattern = pattern;
    if (!pattern.empty() && pattern[0] == '!') {
        negated = true;
        actualPattern = pattern.substr(1);
    }
    
    // Check if it's a directory pattern (ends with /)
    bool dirPattern = false;
    if (!actualPattern.empty() && actualPattern.back() == '/') {
        dirPattern = true;
        actualPattern = actualPattern.substr(0, actualPattern.length() - 1);
    }
    
    // If it's a directory pattern, check if filepath is a directory
    if (dirPattern) {
        if (!Utils::isDirectory(Utils::joinPath(repoPath, filepath))) {
            return false;
        }
    }
    
    // Match the pattern
    bool matches = false;
    
    // Handle simple pattern matching
    if (actualPattern == filepath) {
        matches = true;
    }
    // Handle directory matching (e.g., "build/" should match "build/file.txt")
    else if (dirPattern && filepath.find(actualPattern + "/") == 0) {
        matches = true;
    }
    // Handle extension matching (e.g., "*.o" should match "file.o")
    else if (actualPattern[0] == '*' && actualPattern[1] == '.' && 
             filepath.length() >= actualPattern.length() - 1 &&
             filepath.substr(filepath.length() - (actualPattern.length() - 1)) == actualPattern.substr(1)) {
        matches = true;
    }
    // Handle general glob pattern matching
    else {
        matches = matchGlobPattern(filepath, actualPattern);
    }
    
    // If negated, flip the result
    return negated ? !matches : matches;
}

bool IgnoreList::matchGlobPattern(const std::string& str, const std::string& pattern) const {
    return matchWildcard(str, pattern, 0, 0);
}

bool IgnoreList::matchWildcard(const std::string& str, const std::string& pattern, 
                              size_t strIdx, size_t patternIdx) const {
    // Base case: If pattern is exhausted, check if string is also exhausted
    if (patternIdx == pattern.size()) {
        return strIdx == str.size();
    }
    
    // Handle ** pattern (matches any path)
    if (patternIdx + 1 < pattern.size() && 
        pattern[patternIdx] == '*' && 
        pattern[patternIdx + 1] == '*') {
        
        // Try matching the rest of the pattern at each position
        for (size_t i = strIdx; i <= str.size(); ++i) {
            if (matchWildcard(str, pattern, i, patternIdx + 2)) {
                return true;
            }
        }
        return false;
    }
    
    // Handle * pattern (matches any character sequence)
    if (pattern[patternIdx] == '*') {
        // Try matching the rest of the pattern at each position
        for (size_t i = strIdx; i <= str.size(); ++i) {
            if (matchWildcard(str, pattern, i, patternIdx + 1)) {
                return true;
            }
        }
        return false;
    }
    
    // Handle ? pattern (matches a single character)
    if (pattern[patternIdx] == '?' && strIdx < str.size()) {
        return matchWildcard(str, pattern, strIdx + 1, patternIdx + 1);
    }
    
    // Handle character match
    if (strIdx < str.size() && pattern[patternIdx] == str[strIdx]) {
        return matchWildcard(str, pattern, strIdx + 1, patternIdx + 1);
    }
    
    // No match
    return false;
}

void IgnoreList::addPattern(const std::string& pattern) {
    // Check if the pattern already exists
    for (const auto& existingPattern : patterns) {
        if (existingPattern == pattern) {
            // Pattern already exists, nothing to do
            return;
        }
    }
    
    patterns.push_back(pattern);
    
    // Update the ignore file
    std::ofstream file(ignorePath, std::ios::app);
    if (file.is_open()) {
        file << pattern << std::endl;
        file.close();
        std::cout << "Added ignore pattern: " << pattern << std::endl;
    } else {
        throw std::runtime_error("Failed to open .gyattignore file for writing");
    }
    
    // Clear cache
    ignoreCache.clear();
}

bool IgnoreList::createDefaultIgnoreFile(const std::string& dirPath) {
    std::string ignorePath = Utils::joinPath(dirPath, ".gyattignore");
    
    // Check if file already exists
    if (Utils::fileExists(ignorePath)) {
        std::cerr << "Warning: .gyattignore file already exists at " << ignorePath << std::endl;
        return false;
    }
    
    // Default patterns to ignore
    std::string defaultIgnore = 
        "# Gyatt ignore file\n"
        "# Similar to .gitignore\n"
        "\n"
        "# Gyatt directory\n"
        ".gyatt/\n"
        "\n"
        "# Compiled files\n"
        "*.o\n"
        "*.a\n"
        "*.so\n"
        "*.exe\n"
        "*.dll\n"
        "*.dylib\n"
        "*.out\n"
        "\n"
        "# Build directories\n"
        "build/\n"
        "bin/\n"
        "lib/\n"
        "\n"
        "# Logs and databases\n"
        "*.log\n"
        "*.sql\n"
        "*.sqlite\n"
        "\n"
        "# OS generated files\n"
        ".DS_Store\n"
        ".DS_Store?\n"
        "._*\n"
        ".Spotlight-V100\n"
        ".Trashes\n"
        "ehthumbs.db\n"
        "Thumbs.db\n";
    
    // Write the file
    if (!Utils::writeFile(ignorePath, defaultIgnore)) {
        std::cerr << "Error: Failed to write .gyattignore file" << std::endl;
        return false;
    }
    
    std::cout << "Created .gyattignore file with default patterns" << std::endl;
    return true;
}

} // namespace gyatt
