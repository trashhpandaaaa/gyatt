#include "../include/section_staging.h"
#include "../include/utils.h"
#include <fstream>
#include <regex>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace gyatt {

SectionBasedStaging::SectionBasedStaging(const std::string& repoPath) 
    : repoPath(repoPath), stagingDir(repoPath + "/.gyatt/staging") {
    std::filesystem::create_directories(stagingDir);
    loadStagedSections();
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::analyzeFunctions(const std::string& filepath) {
    std::string language = detectLanguage(filepath);
    
    if (language == "cpp" || language == "c" || language == "hpp") {
        return parseCppFile(filepath);
    } else if (language == "py") {
        return parsePythonFile(filepath);
    } else if (language == "js" || language == "ts") {
        return parseJavaScriptFile(filepath);
    }
    
    return {};
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::analyzeClasses(const std::string& filepath) {
    std::vector<CodeSection> sections;
    std::ifstream file(filepath);
    std::string line;
    size_t lineNum = 0;
    
    // Simple class detection for multiple languages
    std::regex cppClassRegex(R"(^\s*class\s+(\w+).*\{)");
    std::regex pythonClassRegex(R"(^\s*class\s+(\w+).*:)");
    std::regex jsClassRegex(R"(^\s*class\s+(\w+).*\{)");
    
    std::smatch match;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        if (std::regex_search(line, match, cppClassRegex) ||
            std::regex_search(line, match, pythonClassRegex) ||
            std::regex_search(line, match, jsClassRegex)) {
            
            CodeSection section;
            section.filepath = filepath;
            section.sectionType = "class";
            section.name = match[1].str();
            section.startLine = lineNum;
            section.staged = false;
            
            // Find end of class (simplified)
            size_t braceCount = 1;
            size_t endLine = lineNum;
            std::string endMarker = (line.find(':') != std::string::npos) ? "" : "}";
            
            while (std::getline(file, line) && braceCount > 0) {
                endLine++;
                if (!endMarker.empty()) {
                    for (char c : line) {
                        if (c == '{') braceCount++;
                        else if (c == '}') braceCount--;
                    }
                } else {
                    // Python class - find next class or EOF
                    if (line.find("class ") == 0 || line.find("def ") == 0) {
                        if (line[0] != ' ' && line[0] != '\t') break;
                    }
                }
            }
            
            section.endLine = endLine;
            sections.push_back(section);
        }
    }
    
    return sections;
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::analyzeLogicalBlocks(const std::string& filepath) {
    std::vector<CodeSection> sections;
    std::ifstream file(filepath);
    std::string line;
    size_t lineNum = 0;
    
    // Detect logical blocks like if statements, loops, etc.
    std::regex blockRegex(R"(^\s*(if|for|while|try|switch)\s*\()");
    std::smatch match;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        if (std::regex_search(line, match, blockRegex)) {
            CodeSection section;
            section.filepath = filepath;
            section.sectionType = "block";
            section.name = match[1].str() + "_block_" + std::to_string(lineNum);
            section.startLine = lineNum;
            section.staged = false;
            
            // Find matching closing brace
            size_t braceCount = 0;
            for (char c : line) {
                if (c == '{') braceCount++;
            }
            
            if (braceCount > 0) {
                size_t endLine = lineNum;
                while (std::getline(file, line) && braceCount > 0) {
                    endLine++;
                    for (char c : line) {
                        if (c == '{') braceCount++;
                        else if (c == '}') braceCount--;
                    }
                }
                section.endLine = endLine;
                sections.push_back(section);
            }
        }
    }
    
    return sections;
}

bool SectionBasedStaging::stageFunction(const std::string& filepath, const std::string& functionName) {
    auto functions = analyzeFunctions(filepath);
    
    for (auto& func : functions) {
        if (func.name == functionName) {
            func.staged = true;
            
            // Remove if already staged
            stagedSections.erase(
                std::remove_if(stagedSections.begin(), stagedSections.end(),
                    [&](const CodeSection& s) {
                        return s.filepath == filepath && s.name == functionName;
                    }),
                stagedSections.end()
            );
            
            stagedSections.push_back(func);
            saveStagedSections();
            std::cout << "🎯 Staged function: " << functionName << " from " << filepath << std::endl;
            return true;
        }
    }
    
    std::cout << "❌ Function not found: " << functionName << std::endl;
    return false;
}

bool SectionBasedStaging::stageClass(const std::string& filepath, const std::string& className) {
    auto classes = analyzeClasses(filepath);
    
    for (auto& cls : classes) {
        if (cls.name == className) {
            cls.staged = true;
            
            // Remove if already staged
            stagedSections.erase(
                std::remove_if(stagedSections.begin(), stagedSections.end(),
                    [&](const CodeSection& s) {
                        return s.filepath == filepath && s.name == className;
                    }),
                stagedSections.end()
            );
            
            stagedSections.push_back(cls);
            saveStagedSections();
            std::cout << "🎯 Staged class: " << className << " from " << filepath << std::endl;
            return true;
        }
    }
    
    std::cout << "❌ Class not found: " << className << std::endl;
    return false;
}

bool SectionBasedStaging::stageSection(const std::string& filepath, size_t startLine, size_t endLine) {
    std::ifstream file(filepath);
    std::string content;
    std::string line;
    size_t currentLine = 1;
    
    while (std::getline(file, line)) {
        if (currentLine >= startLine && currentLine <= endLine) {
            content += line + "\n";
        }
        currentLine++;
    }
    
    CodeSection section;
    section.filepath = filepath;
    section.sectionType = "custom";
    section.name = "lines_" + std::to_string(startLine) + "_" + std::to_string(endLine);
    section.startLine = startLine;
    section.endLine = endLine;
    section.content = content;
    section.staged = true;
    
    stagedSections.push_back(section);
    saveStagedSections();
    
    std::cout << "🎯 Staged lines " << startLine << "-" << endLine << " from " << filepath << std::endl;
    return true;
}

bool SectionBasedStaging::unstageSection(const std::string& filepath, const std::string& sectionName) {
    auto it = std::remove_if(stagedSections.begin(), stagedSections.end(),
        [&](const CodeSection& s) {
            return s.filepath == filepath && s.name == sectionName;
        });
    
    if (it != stagedSections.end()) {
        stagedSections.erase(it, stagedSections.end());
        saveStagedSections();
        std::cout << "⏪ Unstaged section: " << sectionName << " from " << filepath << std::endl;
        return true;
    }
    
    std::cout << "❌ Section not found in staging area: " << sectionName << std::endl;
    return false;
}

void SectionBasedStaging::interactiveSectionStaging(const std::string& filepath) {
    auto functions = analyzeFunctions(filepath);
    auto classes = analyzeClasses(filepath);
    auto blocks = analyzeLogicalBlocks(filepath);
    
    std::cout << "\n🎯 Interactive Section Staging for: " << filepath << std::endl;
    std::cout << "═══════════════════════════════════════════════\n";
    
    std::cout << "\n📋 Available Functions:" << std::endl;
    for (size_t i = 0; i < functions.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << functions[i].name 
                  << " (lines " << functions[i].startLine << "-" << functions[i].endLine << ")" << std::endl;
    }
    
    std::cout << "\n🏗️ Available Classes:" << std::endl;
    for (size_t i = 0; i < classes.size(); i++) {
        std::cout << "  " << (functions.size() + i + 1) << ". " << classes[i].name 
                  << " (lines " << classes[i].startLine << "-" << classes[i].endLine << ")" << std::endl;
    }
    
    std::cout << "\nEnter section numbers to stage (comma-separated, q to quit): ";
    std::string input;
    std::getline(std::cin, input);
    
    if (input == "q") return;
    
    std::stringstream ss(input);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        try {
            int choice = std::stoi(token);
            if (choice > 0 && choice <= functions.size()) {
                stageFunction(filepath, functions[choice - 1].name);
            } else if (choice > functions.size() && choice <= functions.size() + classes.size()) {
                stageClass(filepath, classes[choice - functions.size() - 1].name);
            }
        } catch (...) {
            std::cout << "Invalid choice: " << token << std::endl;
        }
    }
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::getStagedSections() {
    return stagedSections;
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::getUnstagedSections() {
    std::vector<CodeSection> unstaged;
    
    // This would require scanning all files and comparing with staged sections
    // For now, return empty vector
    return unstaged;
}

void SectionBasedStaging::showSectionDiff(const CodeSection& section) {
    std::cout << "\n📝 Section Diff: " << section.name << std::endl;
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "File: " << section.filepath << std::endl;
    std::cout << "Type: " << section.sectionType << std::endl;
    std::cout << "Lines: " << section.startLine << "-" << section.endLine << std::endl;
    std::cout << "Status: " << (section.staged ? "🟢 Staged" : "⚪ Unstaged") << std::endl;
    std::cout << "\nContent Preview:\n";
    std::cout << "─────────────────────────\n";
    
    // Show first few lines of content
    std::ifstream file(section.filepath);
    std::string line;
    size_t currentLine = 1;
    size_t previewLines = 0;
    
    while (std::getline(file, line) && previewLines < 10) {
        if (currentLine >= section.startLine && currentLine <= section.endLine) {
            std::cout << currentLine << ": " << line << std::endl;
            previewLines++;
        }
        currentLine++;
    }
    
    if (section.endLine - section.startLine + 1 > 10) {
        std::cout << "... (" << (section.endLine - section.startLine + 1 - 10) << " more lines)\n";
    }
}

void SectionBasedStaging::showStagingSummary() {
    std::cout << "\n📊 Staging Summary" << std::endl;
    std::cout << "═══════════════════\n";
    std::cout << "Total staged sections: " << stagedSections.size() << std::endl;
    
    std::map<std::string, int> typeCounts;
    for (const auto& section : stagedSections) {
        typeCounts[section.sectionType]++;
    }
    
    for (const auto& [type, count] : typeCounts) {
        std::cout << "  " << type << ": " << count << std::endl;
    }
    
    std::cout << "\nStaged sections:\n";
    for (const auto& section : stagedSections) {
        std::cout << "  🎯 " << section.name << " (" << section.sectionType << ") - " << section.filepath << std::endl;
    }
}

// Private helper methods

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::parseCppFile(const std::string& filepath) {
    std::vector<CodeSection> sections;
    std::ifstream file(filepath);
    std::string line;
    size_t lineNum = 0;
    
    // Regex for C++ functions
    std::regex functionRegex(R"(^\s*(?:(?:inline|static|virtual|explicit|friend)\s+)*(?:[\w:&*\s]+\s+)?(\w+)\s*\([^;]*\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?\{)");
    std::smatch match;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        if (std::regex_search(line, match, functionRegex)) {
            CodeSection section;
            section.filepath = filepath;
            section.sectionType = "function";
            section.name = match[1].str();
            section.startLine = lineNum;
            section.staged = false;
            
            // Find matching closing brace
            size_t braceCount = 1;
            size_t endLine = lineNum;
            
            while (std::getline(file, line) && braceCount > 0) {
                endLine++;
                for (char c : line) {
                    if (c == '{') braceCount++;
                    else if (c == '}') braceCount--;
                }
            }
            
            section.endLine = endLine;
            sections.push_back(section);
        }
    }
    
    return sections;
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::parsePythonFile(const std::string& filepath) {
    std::vector<CodeSection> sections;
    std::ifstream file(filepath);
    std::string line;
    size_t lineNum = 0;
    
    std::regex functionRegex(R"(^\s*def\s+(\w+)\s*\()");
    std::smatch match;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        if (std::regex_search(line, match, functionRegex)) {
            CodeSection section;
            section.filepath = filepath;
            section.sectionType = "function";
            section.name = match[1].str();
            section.startLine = lineNum;
            section.staged = false;
            
            // Find end of function by indentation
            size_t baseIndent = line.find_first_not_of(" \t");
            size_t endLine = lineNum;
            
            while (std::getline(file, line)) {
                endLine++;
                size_t currentIndent = line.find_first_not_of(" \t");
                
                if (!line.empty() && currentIndent <= baseIndent && 
                    (line.find("def ") != std::string::npos || 
                     line.find("class ") != std::string::npos)) {
                    endLine--;
                    break;
                }
            }
            
            section.endLine = endLine;
            sections.push_back(section);
        }
    }
    
    return sections;
}

std::vector<SectionBasedStaging::CodeSection> SectionBasedStaging::parseJavaScriptFile(const std::string& filepath) {
    std::vector<CodeSection> sections;
    std::ifstream file(filepath);
    std::string line;
    size_t lineNum = 0;
    
    std::regex functionRegex(R"(^\s*(?:function\s+(\w+)|(?:const|let|var)\s+(\w+)\s*=\s*(?:async\s+)?(?:function|\(.*\)\s*=>)|\s*(\w+)\s*:\s*(?:async\s+)?function))");
    std::smatch match;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        if (std::regex_search(line, match, functionRegex)) {
            CodeSection section;
            section.filepath = filepath;
            section.sectionType = "function";
            
            // Extract function name from different patterns
            for (size_t i = 1; i < match.size(); i++) {
                if (!match[i].str().empty()) {
                    section.name = match[i].str();
                    break;
                }
            }
            
            section.startLine = lineNum;
            section.staged = false;
            
            // Find matching closing brace
            size_t braceCount = 0;
            for (char c : line) {
                if (c == '{') braceCount++;
            }
            
            if (braceCount > 0) {
                size_t endLine = lineNum;
                while (std::getline(file, line) && braceCount > 0) {
                    endLine++;
                    for (char c : line) {
                        if (c == '{') braceCount++;
                        else if (c == '}') braceCount--;
                    }
                }
                section.endLine = endLine;
                sections.push_back(section);
            }
        }
    }
    
    return sections;
}

std::string SectionBasedStaging::detectLanguage(const std::string& filepath) {
    size_t lastDot = filepath.find_last_of('.');
    if (lastDot == std::string::npos) return "unknown";
    
    std::string extension = filepath.substr(lastDot + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "cpp" || extension == "cc" || extension == "cxx") return "cpp";
    if (extension == "c") return "c";
    if (extension == "h" || extension == "hpp" || extension == "hxx") return "hpp";
    if (extension == "py") return "py";
    if (extension == "js") return "js";
    if (extension == "ts") return "ts";
    if (extension == "java") return "java";
    
    return "unknown";
}

bool SectionBasedStaging::saveStagedSections() {
    std::ofstream file(stagingDir + "/sections.gyatt");
    if (!file.is_open()) return false;
    
    for (const auto& section : stagedSections) {
        file << section.filepath << "|" 
             << section.sectionType << "|"
             << section.name << "|"
             << section.startLine << "|"
             << section.endLine << "|"
             << generateSectionHash(section) << std::endl;
    }
    
    return true;
}

bool SectionBasedStaging::loadStagedSections() {
    std::ifstream file(stagingDir + "/sections.gyatt");
    if (!file.is_open()) return false;
    
    stagedSections.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        CodeSection section;
        
        if (std::getline(iss, section.filepath, '|') &&
            std::getline(iss, section.sectionType, '|') &&
            std::getline(iss, section.name, '|') &&
            std::getline(iss, token, '|')) {
            
            section.startLine = std::stoul(token);
            if (std::getline(iss, token, '|')) {
                section.endLine = std::stoul(token);
                section.staged = true;
                stagedSections.push_back(section);
            }
        }
    }
    
    return true;
}

std::string SectionBasedStaging::generateSectionHash(const CodeSection& section) {
    // Simple hash based on filepath, name, and line numbers
    std::string data = section.filepath + section.name + 
                      std::to_string(section.startLine) + 
                      std::to_string(section.endLine);
    
    std::hash<std::string> hasher;
    return std::to_string(hasher(data));
}

} // namespace gyatt