#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace gyatt {

class ProjectMapper {
public:
    struct FileNode {
        std::string name;
        std::string path;
        std::string type; // "file", "directory"
        size_t size;
        int lineCount;
        std::vector<std::shared_ptr<FileNode>> children;
        std::vector<std::string> dependencies;
        double testCoverage;
    };

    struct FunctionInfo {
        std::string name;
        std::string file;
        size_t startLine;
        size_t endLine;
        std::vector<std::string> calledBy;
        std::vector<std::string> calls;
        std::string signature;
    };

    ProjectMapper(const std::string& repoPath);
    
    // Generate project maps
    std::shared_ptr<FileNode> generateFileHierarchy();
    std::map<std::string, FunctionInfo> generateFunctionMap();
    std::map<std::string, std::vector<std::string>> generateDependencyGraph();
    std::map<std::string, double> generateTestCoverage();
    
    // Visual outputs
    bool exportHierarchyAsTree(const std::string& filename = "");
    bool exportHierarchyAsHTML(const std::string& filename);
    bool exportDependencyGraphAsDOT(const std::string& filename);
    bool exportFunctionMapAsJSON(const std::string& filename);
    
    // Interactive exploration
    void interactiveExplorer();
    
    // Statistics
    struct ProjectStats {
        size_t totalFiles;
        size_t totalLines;
        size_t totalFunctions;
        size_t totalClasses;
        double averageTestCoverage;
        std::map<std::string, size_t> languageBreakdown;
    };
    
    ProjectStats getProjectStats();
    void showProjectSummary();
    
private:
    std::string repoPath;
    
    // Analysis helpers
    std::shared_ptr<FileNode> analyzeDirectory(const std::string& dirPath);
    std::vector<FunctionInfo> extractFunctions(const std::string& filepath);
    std::vector<std::string> extractDependencies(const std::string& filepath);
    double calculateTestCoverage(const std::string& filepath);
    
    // Visualization helpers
    std::string generateTreeString(const std::shared_ptr<FileNode>& node, int depth = 0);
    std::string generateHTMLTree(const std::shared_ptr<FileNode>& node);
    std::string generateDOTGraph(const std::map<std::string, std::vector<std::string>>& deps);
    
    // Language detection
    std::string getFileLanguage(const std::string& filepath);
    bool isTestFile(const std::string& filepath);
};

} // namespace gyatt
