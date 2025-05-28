#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>

namespace gyatt {

class IgnoreList {
public:
    IgnoreList(const std::string& repoPath);
    
    // Check if a file should be ignored
    bool isIgnored(const std::string& filepath) const;
    
    // Create a default .gyattignore file
    static bool createDefaultIgnoreFile(const std::string& dirPath);

    // Load the ignore file
    void load();

    // Add a pattern to the ignore list
    void addPattern(const std::string& pattern);

private:
    std::string repoPath;
    std::string ignorePath;
    std::vector<std::string> patterns;
    
    // Map for cached results
    mutable std::map<std::string, bool> ignoreCache;
    
    // Parse a pattern and determine if it matches a file
    bool matchesPattern(const std::string& filepath, const std::string& pattern) const;

    // Pattern matching helpers
    bool matchGlobPattern(const std::string& str, const std::string& pattern) const;
    bool matchWildcard(const std::string& str, const std::string& pattern, size_t strIdx, size_t patternIdx) const;
};

} // namespace gyatt
