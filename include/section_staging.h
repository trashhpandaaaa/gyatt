#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace gyatt {

class SectionBasedStaging {
public:
    struct CodeSection {
        std::string filepath;
        std::string sectionType; // "function", "class", "block", "custom"
        std::string name;
        size_t startLine;
        size_t endLine;
        std::string content;
        bool staged;
    };

    SectionBasedStaging(const std::string& repoPath);
    
    // Analyze files for sections
    std::vector<CodeSection> analyzeFunctions(const std::string& filepath);
    std::vector<CodeSection> analyzeClasses(const std::string& filepath);
    std::vector<CodeSection> analyzeLogicalBlocks(const std::string& filepath);
    
    // Section-based staging
    bool stageFunction(const std::string& filepath, const std::string& functionName);
    bool stageClass(const std::string& filepath, const std::string& className);
    bool stageSection(const std::string& filepath, size_t startLine, size_t endLine);
    bool unstageSection(const std::string& filepath, const std::string& sectionName);
    
    // Interactive staging
    void interactiveSectionStaging(const std::string& filepath);
    
    // Get staged sections
    std::vector<CodeSection> getStagedSections();
    std::vector<CodeSection> getUnstagedSections();
    
    // Generate smart diffs
    void showSectionDiff(const CodeSection& section);
    void showStagingSummary();
    
private:
    std::string repoPath;
    std::string stagingDir;
    std::vector<CodeSection> stagedSections;
    
    // Language-specific parsers
    std::vector<CodeSection> parseCppFile(const std::string& filepath);
    std::vector<CodeSection> parsePythonFile(const std::string& filepath);
    std::vector<CodeSection> parseJavaScriptFile(const std::string& filepath);
    
    // Helper functions
    std::string detectLanguage(const std::string& filepath);
    bool saveStagedSections();
    bool loadStagedSections();
    std::string generateSectionHash(const CodeSection& section);
};

} // namespace gyatt
