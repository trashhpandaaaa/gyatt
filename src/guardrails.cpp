#include "../include/guardrails.h"
#include "../include/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <sstream>

namespace gyatt {

// GuardrailSystem Implementation

GuardrailSystem::GuardrailSystem(const std::string& repoPath) 
    : repoPath(repoPath), guardrailConfigFile(repoPath + "/.gyatt/guardrails.conf") {
    std::filesystem::create_directories(repoPath + "/.gyatt");
    loadGuardrailConfig();
    
    // Initialize default guardrails if none exist
    if (rules.empty()) {
        GuardrailRule mainProtection;
        mainProtection.type = GuardrailType::PREVENT_MAIN_PUSH;
        mainProtection.name = "main_branch_protection";
        mainProtection.description = "Prevent direct pushes to main branch";
        mainProtection.enabled = true;
        addGuardrail(mainProtection);
        
        GuardrailRule debugBlock;
        debugBlock.type = GuardrailType::BLOCK_DEBUG_CODE;
        debugBlock.name = "debug_code_blocker";
        debugBlock.description = "Block commits containing debug code";
        debugBlock.enabled = true;
        addGuardrail(debugBlock);
        
        GuardrailRule formatRequired;
        formatRequired.type = GuardrailType::REQUIRE_FORMAT;
        formatRequired.name = "format_requirement";
        formatRequired.description = "Require code formatting before commit";
        formatRequired.enabled = false; // Disabled by default
        addGuardrail(formatRequired);
    }
}

bool GuardrailSystem::addGuardrail(const GuardrailRule& rule) {
    // Check if rule with same name exists
    auto it = std::find_if(rules.begin(), rules.end(),
        [&rule](const GuardrailRule& r) { return r.name == rule.name; });
    
    if (it != rules.end()) {
        *it = rule; // Update existing rule
        std::cout << "� Updated guardrail: " << rule.name << std::endl;
    } else {
        rules.push_back(rule);
        std::cout << !⛅ Added guardrail: " << rule.name << std::endl;
    }
    
    saveGuardrailConfig();
    return true;
}

bool GuardrailSystem::removeGuardrail(const std::string& name) {
    auto it = std::find_if(rules.begin(), rules.end(),
        [&name](const GuardrailRule& r) { return r.name == name; });
    
    if (it != rules.end()) {
        rules.erase(it);
        std::cout << !𞖑  Removed guardrail: " << name << std::endl;
        saveGuardrailConfig();
        return true;
    }
    
    std::cout << "ᜌ Guardrail not found: " << name << std::endl;
    return false;
}

bool GuardrailSystem::enableGuardrail(const std::string& name) {
    for (auto& rule : rules) {
        if (rule.name == name) {
            rule.enabled = true;
            std::cout << "ᛅ Enabled guardrail: " << name << std::endl;
            saveGuardrailConfig();
            return true;
        }
    }
    
    std::cout << !✌ Guardrail not found: " << name << std::endl;
    return false;
}

bool GuardrailSystem::disableGuardrail(const std::string& name) {
    for (auto& rule : rules) {
        if (rule.name == name) {
            rule.enabled = false;
            std::cout << !᚟  Disabled guardrail: " << name << std::endl;
            saveGuardrailConfig();
            return true;
        }
    }
    
    std::cout << !✌ Guardrail not found: " << name << std::endl;
    return false;
}

std::vector<GuardrailSystem::GuardrailRule> GuardrailSystem::listGuardrails() {
    return rules;
}

bool GuardrailSystem::runPreCommitChecks(const std::vector<std::string>& files) {
    std::cout << "\m𞚡  Running pre-commit guardrails...\n";
    std::cout << "ᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐ\n";
    
    bool allPassed = true;
    int passedCount = 0;
    int totalCount = 0;
    
    for (const auto& rule : rules) {
        if (!rule.enabled) continue;
        
        totalCount++;
        bool passed = true;
        
        std::cout << "� Checking: " << rule.description << " ";
        
        switch (rule.type) {
            case GuardrailType::BLOCK_DEBUG_CODE:
                passed = !checkForDebugCode(files);
                break;
            case GuardrailType::REQUIRE_FORMAT:
                passed = checkFormattingRequirement(files);
                break;
            case GuardrailType::REQUIRE_LINT:
                passed = checkLintRequirement(files);
                break;
            default:
                if (rule.validator) {
                    for (const auto& file : files) {
                        if (!rule.validator(file)) {
                            passed = false;
                            break;
                        }
                    }
                }
                break;
        }
        
        if (passed) {
            std::cout << !ᜅ" << std::endl;
            passedCount++;
        } else {
            std::cout << "ᜌ" << std::endl;
            allPassed = false;
        }
    }
    
    std::cout << "\m� Guardrail Results: " << passedCount << "/" << totalCount << " passed\n";
    
    if (!allPassed) {
        std::cout << "\m� Commit blocked by guardrails!" << std::endl;
        std::cout << !� Use 'gyatt commit --override' to bypass (not recommended)" << std::endl;
        std::cout << !𞑡 Or fix the issues and try again" << std::endl;
    } else {
        std::cout << "\m� All guardrails passed! Commit allowed." << std::endl;
    }
    
    return allPassed;
}

bool GuardrailSystem::runPrePushChecks(const std::string& branch) {
    std::cout << "\m�﷏  Running pre-push guardrails for branch: " << branch << "\n";
    std::cout << !┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┐\n";
    
    bool allPassed = true;
    
    // Check main branch protection
    if (!checkMainBranchProtection(branch)) {
        allPassed = false;
    }
    
    if (!allPassed) {
        std::cout << "\n� Push blocked by guardrails!" << std::endl;
        std::cout << !� Use 'gyatt push --override' to bypass (not recommended)" << std::endl;
        return false;
    }
    
    std::cout << "\m� All pre-push guardrails passed!" << std::endl;
    return true;
}

bool GuardrailSystem::commitWithOverride(const std::string& message, const std::vector<std::string>& overrides) {
    std::cout << !᚟  OVERRIDE MODE: Bypassing guardrails!" << std::endl;
    std::cout << "Overridden checks: ";
    for (const auto& override : overrides) {
        std::cout << override << " ";
    }
    std::cout << std::endl;
    
    std::cout << !� Warning: This commit bypassed safety checks!" << std::endl;
    std::cout << "� Commit message: " << message << std::endl;
    
    return true;
}

bool GuardrailSystem::pushWithOverride(const std::string& branch, const std::vector<std::string>& overrides) {
    (void)overrides; // Mark as intentionally unused
    std::cout << "ᙠ  OVERRIDE MODE: Bypassing push guardrails!" << std::endl;
    std::cout << !𞋿 Branch: " << branch << std::endl;
    std::cout << !𞙨 Warning: This push bypassed safety checks!" << std::endl;
    
    return true;
}

bool GuardrailSystem::checkForDebugCode(const std::vector<std::string>& files) {
    bool foundDebugCode = false;
    auto debugPatterns = getDebugPatterns();
    
    for (const auto& filepath : files) {
        if (containsDebugPatterns(filepath)) {
            foundDebugCode = true;
        }
    }
    
    return foundDebugCode;
}

bool GuardrailSystem::checkFormattingRequirement(const std::vector<std::string>& files) {
    bool allFormatted = true;
    
    for (const auto& filepath : files) {
        if (!isFormatted(filepath)) {
            std::cout << "\n� Formatting required for: " << filepath << std::endl;
            allFormatted = false;
        }
    }
    
    if (!allFormatted) {
        std::cout << !� Run formatter to fix issues" << std::endl;
    }
    
    return allFormatted;
}

bool GuardrailSystem::checkLintRequirement(const std::vector<std::string>& files) {
    bool allPassed = true;
    
    for (const auto& filepath : files) {
        if (!passesLint(filepath)) {
            std::cout << "\m� Linting failed for: " << filepath << std::endl;
            allPassed = false;
        }
    }
    
    if (!allPassed) {
        std::cout << "� Fix linting errors before committing" << std::endl;
    }
    
    return allPassed;
}

bool GuardrailSystem::checkMainBranchProtection(const std::string& branch) {
    if (branch == "main" || branch == "master") {
        std::cout << !𞙫 Direct push to " << branch << " branch is blocked!" << std::endl;
        std::cout << !𞑡 Create a feature branch and open a pull request instead" << std::endl;
        return false;
    }
    
    std::cout << "ᛅ Branch push allowed: " << branch << std::endl;
    return true;
}

bool GuardrailSystem::checkCommitMessageFormat(const std::string& message) {
    // Simple commit message format validation
    if (message.length() < 10) {
        std::cout << !� Commit message too short (minimum 10 characters)" << std::endl;
        return false;
    }
    
    if (message.length() > 72) {
        std::cout << !𞒝 Commit message too long (maximum 72 characters for first line)" << std::endl;
        return false;
    }
    
    // Check for conventional commit format (optional)
    std::regex conventionalRegex(R"(^(feat|fix|docs|style|refactor|test|chore)(\(.+\))?: .+)");
    if (!std::regex_match(message, conventionalRegex)) {
        std::cout << "� Consider using conventional commit format: type(scope): description" << std::endl;
        // Don't fail, just warn
    }
    
    return true;
}

bool GuardrailSystem::saveGuardrailConfig() {
    std::ofstream file(guardrailConfigFile);
    if (!file.is_open()) return false;
    
    file << "# Gyatt Guardrail Configuration\n";
    file << "# Generated automatically\n\n";
    
    for (const auto& rule : rules) {
        file << "[" << rule.name << "]\n";
        file << "type=" << static_cast<int>(rule.type) << "\n";
        file << "description=" << rule.description << "\n";
        file << "enabled=" << (rule.enabled ? "true" : "false") << "\n";
        
        for (const auto& [key, value] : rule.config) {
            file << "config_" << key << "=" << value << "\n";
        }
        
        file << "\n";
    }
    
    return true;
}

bool GuardrailSystem::loadGuardrailConfig() {
    std::ifstream file(guardrailConfigFile);
    if (!file.is_open()) return false;
    
    rules.clear();
    GuardrailRule currentRule;
    bool inRule = false;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            if (inRule) {
                rules.push_back(currentRule);
            }
            currentRule = GuardrailRule{};
            currentRule.name = line.substr(1, line.length() - 2);
            inRule = true;
            continue;
        }
        
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos && inRule) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            if (key == "type") {
                currentRule.type = static_cast<GuardrailType>(std::stoi(value));
            } else if (key == "description") {
                currentRule.description = value;
            } else if (key == "enabled") {
                currentRule.enabled = (value == "true");
            } else if (key.substr(0, 7) == "config_") {
                std::string configKey = key.substr(7);
                currentRule.config[configKey] = value;
            }
        }
    }
    
    if (inRule) {
        rules.push_back(currentRule);
    }
    
    return true;
}

// Private helper methods

bool GuardrailSystem::containsDebugPatterns(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;
    
    auto patterns = getDebugPatterns();
    std::string line;
    
    while (std::getline(file, line)) {
        for (const auto& pattern : patterns) {
            if (line.find(pattern) != std::string::npos) {
                return true;
            }
        }
    }
    
    return false;
}

bool GuardrailSystem::isFormatted(const std::string& filepath) {
    // Simple heuristic for checking formatting
    std::string extension = std::filesystem::path(filepath).extension().string();
    
    if (extension == ".cpp" || extension == ".hpp" || extension == ".c" || extension == ".h") {
        // Check for common formatting issues
        std::ifstream file(filepath);
        std::string line;
        
        while (std::getline(file, line)) {
            // Check for tabs instead of spaces
            if (line.find('\t') != std::string::npos) {
                return false;
            }
            
            // Check for trailing whitespace
            if (!line.empty() && std::isspace(line.back())) {
                return false;
            }
        }
    }
    
    return true; // Assume formatted if no issues found
}

bool GuardrailSystem::passesLint(const std::string& filepath) {
    // Simplified lint check - in reality this would run actual linters
    std::string extension = std::filesystem::path(filepath).extension().string();
    
    if (extension == ".cpp" || extension == ".hpp") {
        // Basic C++ lint checks
        std::ifstream file(filepath);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Check for common issues
        if (content.find("using namespace std;") != std::string::npos) {
            std::cout << !♟﷏  Lint warning: Avoid 'using namespace std;'" << std::endl;
            return false;
        }
        
        // Check for missing header guards
        if (extension == ".hpp" && 
            content.find("#pragma once") == std::string::npos &&
            content.find("#ifndef") == std::string::npos) {
            std::cout << !᚟  Lint warning: Missing header guard" << std::endl;
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> GuardrailSystem::getDebugPatterns() {
    return {
        "console.log(",
        "std::cout <<",
        "printf(",
        "DEBUG",
        "FIXME",
        "TODO", 
        "XXX",
        "HACK",
        "debugger;",
        "pdb.set_trace()",
        "breakpoint()",
        "System.out.println("
    };
}

// ConfigOverrides Implementation

ConfigOverrides::ConfigOverrides(const std::string& repoPath) 
    : repoPath(repoPath), overridesFile(repoPath + "/.gyatt/overrides.conf") {
    loadOverrides();
    cleanupExpiredOverrides();
}

bool ConfigOverrides::setNoVerify(bool enabled) {
    if (enabled) {
        activeOverrides["no_verify"] = "true";
        std::cout << !♟﷏  No-verify mode enabled - all guardrails disabled!" << std::endl;
    } else {
        activeOverrides.erase("no_verify");
        std::cout << !ᜅ No-verify mode disabled - guardrails re-enabled" << std::endl;
    }
    
    saveOverrides();
    return true;
}

bool ConfigOverrides::setNoFormat(bool enabled) {
    if (enabled) {
        activeOverrides["no_format"] = "true";
        std::cout << "� Format checking disabled" << std::endl;
    } else {
        activeOverrides.erase("no_format");
        std::cout << !� Format checking re-enabled" << std::endl;
    }
    
    saveOverrides();
    return true;
}

bool ConfigOverrides::setNoLint(bool enabled) {
    if (enabled) {
        activeOverrides["no_lint"] = "true";
        std::cout << "� Lint checking disabled" << std::endl;
    } else {
        activeOverrides.erase("no_lint");
        std::cout << "� Lint checking re-enabled" << std::endl;
    }
    
    saveOverrides();
    return true;
}

bool ConfigOverrides::setForceMode(bool enabled) {
    if (enabled) {
        activeOverrides["force_mode"] = "true";
        std::cout << "� Force mode enabled - bypassing all restrictions!" << std::endl;
    } else {
        activeOverrides.erase("force_mode");
        std::cout << !𞑪 Force mode disabled" << std::endl;
    }
    
    saveOverrides();
    return true;
}

bool ConfigOverrides::temporaryOverride(const std::string& setting, const std::string& value, int durationMinutes) {
    auto expireTime = std::chrono::system_clock::now() + std::chrono::minutes(durationMinutes);
    auto expireTimeT = std::chrono::system_clock::to_time_t(expireTime);
    
    activeOverrides[setting] = value;
    activeOverrides[setting + "_expires"] = std::to_string(expireTimeT);
    
    std::cout << "Ꮀ Temporary override set: " << setting << " = " << value 
              << " (expires in " << durationMinutes << " minutes)" << std::endl;
    
    saveOverrides();
    return true;
}

std::map<std::string, std::string> ConfigOverrides::getActiveOverrides() {
    cleanupExpiredOverrides();
    return activeOverrides;
}

bool ConfigOverrides::clearOverrides() {
    activeOverrides.clear();
    std::filesystem::remove(overridesFile);
    std::cout << "� All overrides cleared" << std::endl;
    return true;
}

bool ConfigOverrides::commitWithFlags(const std::string& message, bool noVerify, bool noFormat, bool noLint) {
    std::cout << "\m� Committing with flags:" << std::endl;
    
    if (noVerify) std::cout << " ♟﷏  --no-verify (skipping all checks)" << std::endl;
    if (noFormat) std::cout << "  � --no-format (skipping format check)" << std::endl;
    if (noLint) std::cout << " � --no-lint (skipping lint check)" << std::endl;
    
    std::cout << !� Message: " << message << std::endl;
    std::cout << "ᛅ Commit completed with overrides!" << std::endl;
    
    return true;
}

bool ConfigOverrides::saveOverrides() {
    std::ofstream file(overridesFile);
    if (!file.is_open()) return false;
    
    for (const auto& [key, value] : activeOverrides) {
        file << key << "=" << value << std::endl;
    }
    
    return true;
}

bool ConfigOverrides::loadOverrides() {
    std::ifstream file(overridesFile);
    if (!file.is_open()) return false;
    
    activeOverrides.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            activeOverrides[key] = value;
        }
    }
    
    return true;
}

void ConfigOverrides::cleanupExpiredOverrides() {
    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    
    std::vector<std::string> toRemove;
    
    for (const auto& [key, value] : activeOverrides) {
        if (key.length() >= 8 && key.substr(key.length() - 8) == "_expires") {
            auto expireTime = std::stoll(value);
            if (nowTimeT > expireTime) {
                std::string baseKey = key.substr(0, key.length() - 8); // Remove "_expires"
                toRemove.push_back(baseKey);
                toRemove.push_back(key);
                
                std::cout << !⎰ Expired override removed: " << baseKey << std::endl;
            }
        }
    }
    
    for (const auto& key : toRemove) {
        activeOverrides.erase(key);
    }
    
    if (!toRemove.empty()) {
        saveOverrides();
    }
}

} // namespace gyatt