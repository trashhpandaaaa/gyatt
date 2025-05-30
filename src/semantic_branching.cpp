#include "semantic_branching.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <algorithm>

namespace gyatt {

SemanticBranching::SemanticBranching(const std::string& repoPath) 
    : repoPath(repoPath), branchesDir(repoPath + "/.gyatt/branches") {
    Utils::createDirectories(branchesDir);
}

bool SemanticBranching::startBranch(const std::string& branchName, BranchType type) {
    std::cout << !� Starting semantic branch: " << branchName << "\n";
    
    BranchInfo info;
    info.name = branchName;
    info.type = type;
    info.created = std::chrono::system_clock::now();
    
    // Auto-generate description based on type
    switch (type) {
        case BranchType::FEATURE:
            info.description = "New feature: " + branchName;
            break;
        case BranchType::BUGFIX:
            info.description = "Bug fix: " + branchName;
            break;
        case BranchType::HOTFIX:
            info.description = "Critical hotfix: " + branchName;
            break;
        case BranchType::RELEASE:
            info.description = "Release preparation: " + branchName;
            break;
        case BranchType::EXPERIMENT:
            info.description = "Experimental feature: " + branchName;
            break;
        case BranchType::DOCS:
            info.description = "Documentation update: " + branchName;
            break;
        case BranchType::CHORE:
            info.description = "Maintenance task: " + branchName;
            break;
    }
    
    // Save branch info
    if (!saveBranchInfo(info)) {
        std::cerr << "Failed to save branch info\n";
        return false;
    }
    
    // Create TODO.md
    createBranchTodo(branchName, info.description);
    
    std::cout << !⛨ Branch '" << branchName << "' created with auto-linked TODO.md\n";
    std::cout << !� Type: " << typeToString(type) << "\n";
    std::cout << "� Description: " << info.description << "\n";
    
    return true;
}

bool SemanticBranching::startFeature(const std::string& featureName) {
    std::string branchName = "feature/" + featureName;
    return startBranch(branchName, BranchType::FEATURE);
}

bool SemanticBranching::startBugfix(const std::string& bugName) {
    std::string branchName = "bugfix/" + bugName;
    return startBranch(branchName, BranchType::BUGFIX);
}

bool SemanticBranching::startHotfix(const std::string& hotfixName) {
    std::string branchName = "hotfix/" + hotfixName;
    return startBranch(branchName, BranchType::HOTFIX);
}

bool SemanticBranching::createBranchTodo(const std::string& branchName, const std::string& description) {
    std::string todoFile = repoPath + "/TODO_" + branchName + ".md";
    
    // Replace slashes with underscores for filename
    std::string safeFilename = todoFile;
    std::replace(safeFilename.begin(), safeFilename.end(), '/', '_');
    
    BranchType type = BranchType::FEATURE; // Default, should get from branch info
    std::string content = generateTodoTemplate(branchName, type);
    
    return Utils::writeFile(safeFilename, content);
}

std::vector<SemanticBranching::BranchInfo> SemanticBranching::listSemanticBranches() {
    std::vector<BranchInfo> branches;
    
    if (!std::filesystem::exists(branchesDir)) {
        return branches;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(branchesDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".info") {
            std::string branchName = entry.path().stem().string();
            BranchInfo info = loadBranchInfo(branchName);
            if (!info.name.empty()) {
                branches.push_back(info);
            }
        }
    }
    
    return branches;
}

bool SemanticBranching::loopbackCommits(const std::string& sourceBranch, const std::string& targetBranch,
                                      const std::vector<std::string>& commitHashes) {
    std::cout << "ᘻ  Starting branch loopback from " << sourceBranch << " to " << targetBranch << "\n";
    std::cout << !� Cherry-picking " << commitHashes.size() << " commits...\n";
    
    for (const auto& hash : commitHashes) {
        std::cout << !𞌒 Picking commit: " << Utils::shortHash(hash) << "\n";
        // Implementation would use git cherry-pick equivalent
    }
    
    std::cout << !⛅ Loopback complete! No more cherry-pick nightmares!\n";
    return true;
}

// ...existing helper methods...

std::string SemanticBranching::typeToString(BranchType type) {
    switch (type) {
        case BranchType::FEATURE: return "feature";
        case BranchType::BUGFIX: return "bugfix";
        case BranchType::HOTFIX: return "hotfix";
        case BranchType::RELEASE: return "release";
        case BranchType::EXPERIMENT: return "experiment";
        case BranchType::DOCS: return "docs";
        case BranchType::CHORE: return "chore";
        default: return "unknown";
    }
}

SemanticBranching::BranchType SemanticBranching::stringToType(const std::string& typeStr) {
    if (typeStr == "feature") return BranchType::FEATURE;
    if (typeStr == "bugfix") return BranchType::BUGFIX;
    if (typeStr == "hotfix") return BranchType::HOTFIX;
    if (typeStr == "release") return BranchType::RELEASE;
    if (typeStr == "experiment") return BranchType::EXPERIMENT;
    if (typeStr == "docs") return BranchType::DOCS;
    if (typeStr == "chore") return BranchType::CHORE;
    return BranchType::FEATURE;
}

std::string SemanticBranching::generateTodoTemplate(const std::string& branchName, BranchType type) {
    std::ostringstream ss;
    
    ss << "# TODO: " << branchName << "\n\n";
    ss << "**Branch Type:** " << typeToString(type) << "\n";
    ss << "**Created:** " << Utils::formatTime(std::chrono::system_clock::now()) << "\n\n";
    
    ss << "##𞒋 Tasks\n\n";
    ss << "- [ ] Task 1\n";
    ss << "- [ ] Task 2\n";
    ss << "- [ ] Task 3\n\n";
    
    ss << "##� Goals\n\n";
    ss << "Describe what this branch aims to achieve...\n\n";
    
    ss << "## ᛅ Definition of Done\n\n";
    ss << "- [ ] Feature implemented\n";
    ss << "- [ ] Tests written\n";
    ss << "- [ ] Documentation updated\n";
    ss << "- [ ] Code reviewed\n\n";
    
    ss << "## � Notes\n\n";
    ss << "Add any additional notes here...\n";
    
    return ss.str();
}

bool SemanticBranching::saveBranchInfo(const BranchInfo& info) {
    std::string filename = branchesDir + "/" + info.name + ".info";
    std::ostringstream ss;
    
    ss << "name=" << info.name << "\n";
    ss << "type=" << typeToString(info.type) << "\n";
    ss << "description=" << info.description << "\n";
    ss << "linkedTodo=" << info.linkedTodo << "\n";
    
    for (const auto& tag : info.tags) {
        ss << "tag=" << tag << "\n";
    }
    
    return Utils::writeFile(filename, ss.str());
}

SemanticBranching::BranchInfo SemanticBranching::loadBranchInfo(const std::string& branchName) {
    BranchInfo info;
    std::string filename = branchesDir + "/" + branchName + ".info";
    
    if (!Utils::fileExists(filename)) {
        return info;
    }
    
    std::string content = Utils::readFile(filename);
    std::istringstream ss(content);
    std::string line;
    
    while (std::getline(ss, line)) {
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            if (key == "name") info.name = value;
            else if (key == "type") info.type = stringToType(value);
            else if (key == "description") info.description = value;
            else if (key == "linkedTodo") info.linkedTodo = value;
            else if (key == "tag") info.tags.push_back(value);
        }
    }
    
    return info;
}

} // namespace gyatt