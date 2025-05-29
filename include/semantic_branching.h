#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace gyatt {

class SemanticBranching {
public:
    enum class BranchType {
        FEATURE,
        BUGFIX,
        HOTFIX,
        RELEASE,
        EXPERIMENT,
        DOCS,
        CHORE
    };

    struct BranchInfo {
        std::string name;
        BranchType type;
        std::string description;
        std::string linkedTodo;
        std::vector<std::string> tags;
        std::chrono::system_clock::time_point created;
    };

    SemanticBranching(const std::string& repoPath);
    
    // Semantic branch creation
    bool startBranch(const std::string& branchName, BranchType type = BranchType::FEATURE);
    bool startFeature(const std::string& featureName);
    bool startBugfix(const std::string& bugName);
    bool startHotfix(const std::string& hotfixName);
    
    // Auto-generate TODO.md for branches
    bool createBranchTodo(const std::string& branchName, const std::string& description = "");
    
    // Branch management
    std::vector<BranchInfo> listSemanticBranches();
    bool addBranchTag(const std::string& branchName, const std::string& tag);
    bool setBranchDescription(const std::string& branchName, const std::string& description);
    
    // Branch merging with summaries
    bool mergeWithSummary(const std::string& sourceBranch, const std::string& targetBranch, 
                         const std::string& summary = "");
    
    // Branch loopback - selective commit merging
    bool loopbackCommits(const std::string& sourceBranch, const std::string& targetBranch,
                        const std::vector<std::string>& commitHashes);
    
private:
    std::string repoPath;
    std::string branchesDir;
    
    std::string typeToString(BranchType type);
    BranchType stringToType(const std::string& typeStr);
    std::string generateTodoTemplate(const std::string& branchName, BranchType type);
    bool saveBranchInfo(const BranchInfo& info);
    BranchInfo loadBranchInfo(const std::string& branchName);
};

} // namespace gyatt
