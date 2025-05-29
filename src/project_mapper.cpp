#include "../include/project_mapper.h"
#include "../include/utils.h"
#include <fstream>
#include <regex>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <set>

namespace gyatt {

ProjectMapper::ProjectMapper(const std::string& repoPath) : repoPath(repoPath) {
    // Initialize project mapper
}

std::shared_ptr<ProjectMapper::FileNode> ProjectMapper::generateFileHierarchy() {
    return analyzeDirectory(repoPath);
}

std::map<std::string, ProjectMapper::FunctionInfo> ProjectMapper::generateFunctionMap() {
    std::map<std::string, FunctionInfo> functionMap;
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
        if (entry.is_regular_file()) {
            std::string filepath = entry.path().string();
            std::string language = getFileLanguage(filepath);
            
            if (language != "unknown") {
                auto functions = extractFunctions(filepath);
                for (const auto& func : functions) {
                    std::string key = func.file + "::" + func.name;
                    functionMap[key] = func;
                }
            }
        }
    }
    
    // Build call relationships
    for (auto& [key, func] : functionMap) {
        std::ifstream file(func.file);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Simple function call detection
        for (const auto& [otherKey, otherFunc] : functionMap) {
            if (key != otherKey && content.find(otherFunc.name + "(") != std::string::npos) {
                func.calls.push_back(otherFunc.name);
                functionMap[otherKey].calledBy.push_back(func.name);
            }
        }
    }
    
    return functionMap;
}

std::map<std::string, std::vector<std::string>> ProjectMapper::generateDependencyGraph() {
    std::map<std::string, std::vector<std::string>> dependencies;
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
        if (entry.is_regular_file()) {
            std::string filepath = entry.path().string();
            std::string language = getFileLanguage(filepath);
            
            if (language != "unknown") {
                auto deps = extractDependencies(filepath);
                dependencies[filepath] = deps;
            }
        }
    }
    
    return dependencies;
}

std::map<std::string, double> ProjectMapper::generateTestCoverage() {
    std::map<std::string, double> coverage;
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
        if (entry.is_regular_file()) {
            std::string filepath = entry.path().string();
            std::string language = getFileLanguage(filepath);
            
            if (language != "unknown" && !isTestFile(filepath)) {
                coverage[filepath] = calculateTestCoverage(filepath);
            }
        }
    }
    
    return coverage;
}

bool ProjectMapper::exportHierarchyAsTree(const std::string& filename) {
    auto hierarchy = generateFileHierarchy();
    std::string treeString = generateTreeString(hierarchy);
    
    if (filename.empty()) {
        std::cout << "\n🌲 Project File Hierarchy\n";
        std::cout << "══════════════════════════\n";
        std::cout << treeString << std::endl;
        return true;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "Project File Hierarchy\n";
    file << "======================\n\n";
    file << treeString;
    
    std::cout << "✅ Exported hierarchy tree to: " << filename << std::endl;
    return true;
}

bool ProjectMapper::exportHierarchyAsHTML(const std::string& filename) {
    auto hierarchy = generateFileHierarchy();
    std::string htmlContent = generateHTMLTree(hierarchy);
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "<!DOCTYPE html>\n<html>\n<head>\n";
    file << "<title>Project Hierarchy</title>\n";
    file << "<style>\n";
    file << "body { font-family: 'Courier New', monospace; margin: 20px; }\n";
    file << ".tree { list-style-type: none; margin: 0; padding: 0; }\n";
    file << ".tree li { margin: 2px 0; }\n";
    file << ".folder { color: #0066cc; font-weight: bold; }\n";
    file << ".file { color: #333; }\n";
    file << ".stats { color: #666; font-size: 0.9em; }\n";
    file << "</style>\n</head>\n<body>\n";
    file << "<h1>🌲 Project File Hierarchy</h1>\n";
    file << htmlContent;
    file << "</body>\n</html>";
    
    std::cout << "✅ Exported HTML hierarchy to: " << filename << std::endl;
    return true;
}

bool ProjectMapper::exportDependencyGraphAsDOT(const std::string& filename) {
    auto dependencies = generateDependencyGraph();
    std::string dotContent = generateDOTGraph(dependencies);
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << dotContent;
    
    std::cout << "✅ Exported DOT graph to: " << filename << std::endl;
    std::cout << "💡 Use: dot -Tpng " << filename << " -o dependency_graph.png" << std::endl;
    return true;
}

bool ProjectMapper::exportFunctionMapAsJSON(const std::string& filename) {
    auto functionMap = generateFunctionMap();
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "{\n  \"functions\": [\n";
    
    bool first = true;
    for (const auto& [key, func] : functionMap) {
        if (!first) file << ",\n";
        first = false;
        
        file << "    {\n";
        file << "      \"name\": \"" << func.name << "\",\n";
        file << "      \"file\": \"" << func.file << "\",\n";
        file << "      \"startLine\": " << func.startLine << ",\n";
        file << "      \"endLine\": " << func.endLine << ",\n";
        file << "      \"signature\": \"" << func.signature << "\",\n";
        file << "      \"calls\": [";
        
        for (size_t i = 0; i < func.calls.size(); i++) {
            if (i > 0) file << ", ";
            file << "\"" << func.calls[i] << "\"";
        }
        file << "],\n";
        
        file << "      \"calledBy\": [";
        for (size_t i = 0; i < func.calledBy.size(); i++) {
            if (i > 0) file << ", ";
            file << "\"" << func.calledBy[i] << "\"";
        }
        file << "]\n    }";
    }
    
    file << "\n  ]\n}";
    
    std::cout << "✅ Exported function map to: " << filename << std::endl;
    return true;
}

void ProjectMapper::interactiveExplorer() {
    auto hierarchy = generateFileHierarchy();
    auto functionMap = generateFunctionMap();
    auto dependencies = generateDependencyGraph();
    
    while (true) {
        std::cout << "\n🗺️  Interactive Project Explorer\n";
        std::cout << "════════════════════════════════\n";
        std::cout << "1. 📂 Show file hierarchy\n";
        std::cout << "2. 🔍 Search functions\n";
        std::cout << "3. 📊 Show dependencies\n";
        std::cout << "4. 📈 Project statistics\n";
        std::cout << "5. 🎯 Function call graph\n";
        std::cout << "6. 🧪 Test coverage\n";
        std::cout << "7. 💾 Export all\n";
        std::cout << "0. 🚪 Exit\n\n";
        std::cout << "Choice: ";
        
        std::string choice;
        std::getline(std::cin, choice);
        
        if (choice == "0") break;
        
        if (choice == "1") {
            exportHierarchyAsTree();
        } else if (choice == "2") {
            std::cout << "🔍 Enter function name to search: ";
            std::string searchTerm;
            std::getline(std::cin, searchTerm);
            
            std::cout << "\n📋 Matching functions:\n";
            for (const auto& [key, func] : functionMap) {
                if (func.name.find(searchTerm) != std::string::npos) {
                    std::cout << "  📍 " << func.name << " in " << func.file 
                              << " (lines " << func.startLine << "-" << func.endLine << ")\n";
                    std::cout << "    Calls: ";
                    for (const auto& call : func.calls) {
                        std::cout << call << " ";
                    }
                    std::cout << "\n    Called by: ";
                    for (const auto& caller : func.calledBy) {
                        std::cout << caller << " ";
                    }
                    std::cout << "\n\n";
                }
            }
        } else if (choice == "3") {
            std::cout << "\n🔗 Dependency Analysis:\n";
            for (const auto& [file, deps] : dependencies) {
                if (!deps.empty()) {
                    std::cout << "📄 " << file << " depends on:\n";
                    for (const auto& dep : deps) {
                        std::cout << "  └─ " << dep << "\n";
                    }
                    std::cout << "\n";
                }
            }
        } else if (choice == "4") {
            showProjectSummary();
        } else if (choice == "5") {
            std::cout << "🔍 Enter function name for call graph: ";
            std::string funcName;
            std::getline(std::cin, funcName);
            
            for (const auto& [key, func] : functionMap) {
                if (func.name == funcName) {
                    std::cout << "\n🎯 Call Graph for " << funcName << ":\n";
                    std::cout << "Called by:\n";
                    for (const auto& caller : func.calledBy) {
                        std::cout << "  ← " << caller << "\n";
                    }
                    std::cout << "\nCalls:\n";
                    for (const auto& callee : func.calls) {
                        std::cout << "  → " << callee << "\n";
                    }
                    break;
                }
            }
        } else if (choice == "6") {
            auto coverage = generateTestCoverage();
            std::cout << "\n🧪 Test Coverage Report:\n";
            for (const auto& [file, cov] : coverage) {
                std::cout << "📄 " << file << ": " 
                          << std::fixed << std::setprecision(1) << (cov * 100) << "%\n";
            }
        } else if (choice == "7") {
            exportHierarchyAsTree("hierarchy.txt");
            exportHierarchyAsHTML("hierarchy.html");
            exportDependencyGraphAsDOT("dependencies.dot");
            exportFunctionMapAsJSON("functions.json");
            std::cout << "✅ All exports completed!\n";
        }
    }
}

ProjectMapper::ProjectStats ProjectMapper::getProjectStats() {
    ProjectStats stats = {};
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
        if (entry.is_regular_file()) {
            std::string filepath = entry.path().string();
            std::string language = getFileLanguage(filepath);
            
            if (language != "unknown") {
                stats.totalFiles++;
                stats.languageBreakdown[language]++;
                
                // Count lines
                std::ifstream file(filepath);
                std::string line;
                while (std::getline(file, line)) {
                    stats.totalLines++;
                }
                
                // Count functions (simplified)
                auto functions = extractFunctions(filepath);
                stats.totalFunctions += functions.size();
                
                // Count classes (simplified)
                std::ifstream file2(filepath);
                std::string content((std::istreambuf_iterator<char>(file2)),
                                   std::istreambuf_iterator<char>());
                
                std::regex classRegex(R"(class\s+\w+)");
                auto begin = std::sregex_iterator(content.begin(), content.end(), classRegex);
                auto end = std::sregex_iterator();
                stats.totalClasses += std::distance(begin, end);
            }
        }
    }
    
    // Calculate average test coverage
    auto coverage = generateTestCoverage();
    if (!coverage.empty()) {
        double total = 0.0;
        for (const auto& [file, cov] : coverage) {
            total += cov;
        }
        stats.averageTestCoverage = total / coverage.size();
    }
    
    return stats;
}

void ProjectMapper::showProjectSummary() {
    auto stats = getProjectStats();
    
    std::cout << "\n📊 Project Summary\n";
    std::cout << "═══════════════════\n";
    std::cout << "📁 Total Files: " << stats.totalFiles << "\n";
    std::cout << "📝 Total Lines: " << stats.totalLines << "\n";
    std::cout << "⚙️  Total Functions: " << stats.totalFunctions << "\n";
    std::cout << "🏗️  Total Classes: " << stats.totalClasses << "\n";
    std::cout << "🧪 Average Test Coverage: " 
              << std::fixed << std::setprecision(1) << (stats.averageTestCoverage * 100) << "%\n\n";
    
    std::cout << "🔤 Language Breakdown:\n";
    for (const auto& [lang, count] : stats.languageBreakdown) {
        double percentage = (double)count / stats.totalFiles * 100;
        std::cout << "  " << lang << ": " << count << " files (" 
                  << std::fixed << std::setprecision(1) << percentage << "%)\n";
    }
}

// Private helper methods

std::shared_ptr<ProjectMapper::FileNode> ProjectMapper::analyzeDirectory(const std::string& dirPath) {
    auto node = std::make_shared<FileNode>();
    node->name = std::filesystem::path(dirPath).filename().string();
    node->path = dirPath;
    node->type = "directory";
    node->size = 0;
    node->lineCount = 0;
    node->testCoverage = 0.0;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.path().filename().string()[0] == '.') continue; // Skip hidden files
            
            if (entry.is_directory()) {
                auto childNode = analyzeDirectory(entry.path().string());
                node->children.push_back(childNode);
                node->size += childNode->size;
                node->lineCount += childNode->lineCount;
            } else if (entry.is_regular_file()) {
                auto fileNode = std::make_shared<FileNode>();
                fileNode->name = entry.path().filename().string();
                fileNode->path = entry.path().string();
                fileNode->type = "file";
                fileNode->size = entry.file_size();
                
                // Count lines
                std::ifstream file(fileNode->path);
                std::string line;
                while (std::getline(file, line)) {
                    fileNode->lineCount++;
                }
                
                fileNode->dependencies = extractDependencies(fileNode->path);
                fileNode->testCoverage = calculateTestCoverage(fileNode->path);
                
                node->children.push_back(fileNode);
                node->size += fileNode->size;
                node->lineCount += fileNode->lineCount;
            }
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error accessing directory: " << dirPath << " - " << ex.what() << std::endl;
    }
    
    return node;
}

std::vector<ProjectMapper::FunctionInfo> ProjectMapper::extractFunctions(const std::string& filepath) {
    std::vector<FunctionInfo> functions;
    std::string language = getFileLanguage(filepath);
    
    std::ifstream file(filepath);
    std::string line;
    size_t lineNum = 0;
    
    if (language == "cpp" || language == "c" || language == "hpp") {
        std::regex functionRegex(R"(^\s*(?:(?:inline|static|virtual|explicit|friend)\s+)*(?:[\w:&*\s]+\s+)?(\w+)\s*\([^;]*\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?\{)");
        std::smatch match;
        
        while (std::getline(file, line)) {
            lineNum++;
            
            if (std::regex_search(line, match, functionRegex)) {
                FunctionInfo func;
                func.name = match[1].str();
                func.file = filepath;
                func.startLine = lineNum;
                func.signature = line;
                
                // Find end of function
                size_t braceCount = 1;
                size_t endLine = lineNum;
                
                while (std::getline(file, line) && braceCount > 0) {
                    endLine++;
                    for (char c : line) {
                        if (c == '{') braceCount++;
                        else if (c == '}') braceCount--;
                    }
                }
                
                func.endLine = endLine;
                functions.push_back(func);
            }
        }
    } else if (language == "py") {
        std::regex functionRegex(R"(^\s*def\s+(\w+)\s*\()");
        std::smatch match;
        
        while (std::getline(file, line)) {
            lineNum++;
            
            if (std::regex_search(line, match, functionRegex)) {
                FunctionInfo func;
                func.name = match[1].str();
                func.file = filepath;
                func.startLine = lineNum;
                func.signature = line;
                
                // Find end by indentation
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
                
                func.endLine = endLine;
                functions.push_back(func);
            }
        }
    }
    
    return functions;
}

std::vector<std::string> ProjectMapper::extractDependencies(const std::string& filepath) {
    std::vector<std::string> dependencies;
    std::string language = getFileLanguage(filepath);
    
    std::ifstream file(filepath);
    std::string line;
    
    if (language == "cpp" || language == "c" || language == "hpp") {
        std::regex includeRegex(R"(^\s*#include\s*[<"]([^">]+)[">])");
        std::smatch match;
        
        while (std::getline(file, line)) {
            if (std::regex_search(line, match, includeRegex)) {
                dependencies.push_back(match[1].str());
            }
        }
    } else if (language == "py") {
        std::regex importRegex(R"(^\s*(?:import|from)\s+(\w+))");
        std::smatch match;
        
        while (std::getline(file, line)) {
            if (std::regex_search(line, match, importRegex)) {
                dependencies.push_back(match[1].str());
            }
        }
    } else if (language == "js" || language == "ts") {
        std::regex requireRegex(R"(require\(['"]([^'"]+)['"]\))");
        std::regex importRegex(R"(import.+from\s+['"]([^'"]+)['"])");
        std::smatch match;
        
        while (std::getline(file, line)) {
            if (std::regex_search(line, match, requireRegex) ||
                std::regex_search(line, match, importRegex)) {
                dependencies.push_back(match[1].str());
            }
        }
    }
    
    return dependencies;
}

double ProjectMapper::calculateTestCoverage(const std::string& filepath) {
    // Simple heuristic: if there's a corresponding test file, assume 70% coverage
    std::string language = getFileLanguage(filepath);
    if (language == "unknown") return 0.0;
    
    std::filesystem::path path(filepath);
    std::string basename = path.stem().string();
    std::string dir = path.parent_path().string();
    
    // Look for test files
    std::vector<std::string> testPatterns = {
        dir + "/test_" + basename + ".*",
        dir + "/" + basename + "_test.*",
        dir + "/tests/" + basename + ".*",
        dir + "/../test/" + basename + ".*",
        dir + "/../tests/" + basename + ".*"
    };
    
    for (const auto& pattern : testPatterns) {
        if (std::filesystem::exists(pattern)) {
            return 0.7; // 70% coverage if test file exists
        }
    }
    
    return 0.1; // 10% default coverage
}

std::string ProjectMapper::generateTreeString(const std::shared_ptr<FileNode>& node, int depth) {
    std::string result;
    std::string indent(depth * 2, ' ');
    
    if (depth > 0) {
        result += indent + "├─ ";
    }
    
    if (node->type == "directory") {
        result += "📁 " + node->name + "/\n";
        for (const auto& child : node->children) {
            result += generateTreeString(child, depth + 1);
        }
    } else {
        std::string icon = "📄";
        std::string ext = std::filesystem::path(node->name).extension().string();
        
        if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") icon = "⚙️";
        else if (ext == ".h" || ext == ".hpp") icon = "🔧";
        else if (ext == ".py") icon = "🐍";
        else if (ext == ".js" || ext == ".ts") icon = "🟨";
        else if (ext == ".md") icon = "📝";
        else if (ext == ".json") icon = "📋";
        
        result += icon + " " + node->name;
        result += " (" + std::to_string(node->lineCount) + " lines, ";
        result += std::to_string(node->size) + " bytes)\n";
    }
    
    return result;
}

std::string ProjectMapper::generateHTMLTree(const std::shared_ptr<FileNode>& node) {
    std::string result = "<ul class=\"tree\">\n";
    
    for (const auto& child : node->children) {
        result += "<li>";
        
        if (child->type == "directory") {
            result += "<span class=\"folder\">📁 " + child->name + "/</span>\n";
            result += generateHTMLTree(child);
        } else {
            result += "<span class=\"file\">📄 " + child->name + "</span>";
            result += "<span class=\"stats\"> (" + std::to_string(child->lineCount) + " lines)</span>";
        }
        
        result += "</li>\n";
    }
    
    result += "</ul>\n";
    return result;
}

std::string ProjectMapper::generateDOTGraph(const std::map<std::string, std::vector<std::string>>& deps) {
    std::string result = "digraph Dependencies {\n";
    result += "  rankdir=LR;\n";
    result += "  node [shape=box, style=rounded];\n\n";
    
    // Add nodes
    std::set<std::string> allFiles;
    for (const auto& [file, dependencies] : deps) {
        allFiles.insert(file);
        for (const auto& dep : dependencies) {
            allFiles.insert(dep);
        }
    }
    
    for (const auto& file : allFiles) {
        std::string label = std::filesystem::path(file).filename().string();
        result += "  \"" + file + "\" [label=\"" + label + "\"];\n";
    }
    
    result += "\n";
    
    // Add edges
    for (const auto& [file, dependencies] : deps) {
        for (const auto& dep : dependencies) {
            result += "  \"" + file + "\" -> \"" + dep + "\";\n";
        }
    }
    
    result += "}\n";
    return result;
}

std::string ProjectMapper::getFileLanguage(const std::string& filepath) {
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
    if (extension == "go") return "go";
    if (extension == "rs") return "rust";
    
    return "unknown";
}

bool ProjectMapper::isTestFile(const std::string& filepath) {
    std::string filename = std::filesystem::path(filepath).filename().string();
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    
    return filename.find("test") != std::string::npos ||
           filename.find("spec") != std::string::npos ||
           filepath.find("/test/") != std::string::npos ||
           filepath.find("/tests/") != std::string::npos;
}

} // namespace gyatt