#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace gyatt {

class GuardrailSystem {
public:
    enum class GuardrailType {
        PREVENT_MAIN_PUSH,
        BLOCK_DEBUG_CODE,
        REQUIRE_FORMAT,
        REQUIRE_LINT,
        REQUIRE_TESTS,
        COMMIT_MESSAGE_FORMAT,
        FILE_SIZE_LIMIT,
        CUSTOM
    };

    struct GuardrailRule {
        GuardrailType type;
        std::string name;
        std::string description;
        bool enabled;
        std::map<std::string, std::string> config;
        std::function<bool(const std::string&)> validator;
    };

    GuardrailSystem(const std::string& repoPath);
    
    // Guardrail management
    bool addGuardrail(const GuardrailRule& rule);
    bool removeGuardrail(const std::string& name);
    bool enableGuardrail(const std::string& name);
    bool disableGuardrail(const std::string& name);
    std::vector<GuardrailRule> listGuardrails();
    
    // Pre-commit checks
    bool runPreCommitChecks(const std::vector<std::string>& files);
    bool runPrePushChecks(const std::string& branch);
    
    // Override system
    bool commitWithOverride(const std::string& message, const std::vector<std::string>& overrides);
    bool pushWithOverride(const std::string& branch, const std::vector<std::string>& overrides);
    
    // Built-in guardrails
    bool checkForDebugCode(const std::vector<std::string>& files);
    bool checkFormattingRequirement(const std::vector<std::string>& files);
    bool checkLintRequirement(const std::vector<std::string>& files);
    bool checkMainBranchProtection(const std::string& branch);
    bool checkCommitMessageFormat(const std::string& message);
    
    // Configuration
    bool saveGuardrailConfig();
    bool loadGuardrailConfig();
    
private:
    std::string repoPath;
    std::string guardrailConfigFile;
    std::vector<GuardrailRule> rules;
    
    // Pattern matching
    bool containsDebugPatterns(const std::string& filepath);
    bool isFormatted(const std::string& filepath);
    bool passesLint(const std::string& filepath);
    std::vector<std::string> getDebugPatterns();
};

class ConfigOverrides {
public:
    ConfigOverrides(const std::string& repoPath);
    
    // Inline config toggles
    bool setNoVerify(bool enabled);
    bool setNoFormat(bool enabled);
    bool setNoLint(bool enabled);
    bool setForceMode(bool enabled);
    
    // Temporary overrides
    bool temporaryOverride(const std::string& setting, const std::string& value, int durationMinutes = 60);
    std::map<std::string, std::string> getActiveOverrides();
    bool clearOverrides();
    
    // Command-specific overrides
    bool commitWithFlags(const std::string& message, 
                        bool noVerify = false, 
                        bool noFormat = false, 
                        bool noLint = false);
    
private:
    std::string repoPath;
    std::string overridesFile;
    std::map<std::string, std::string> activeOverrides;
    
    bool saveOverrides();
    bool loadOverrides();
    void cleanupExpiredOverrides();
};

} // namespace gyatt
