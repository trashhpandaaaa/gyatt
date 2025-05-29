#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include "ignore.h"

// Forward declarations for feature systems
namespace gyatt {
class MarkdownCommit;
class SemanticBranching;
class SectionBasedStaging;
class ProjectMapper;
class CheckpointSystem;
class OopsShield;
class RewindMode;
class GuardrailSystem;
class PluginManager;
class SessionRecorder;
class CommentThread;
class StickyNotes;
class LabelSystem;
class InitTemplates;
class CommitStoryMode;
class ContainerizedSnapshots;
class TerminalUI;
}

namespace gyatt {

class Repository {
public:
    Repository(const std::string& path = ".");
    ~Repository();
    
    // Core operations
    bool init();
    bool add(const std::string& filepath);
    bool commit(const std::string& message, const std::string& author = "");
    bool status();
    bool log();
    bool diff();
    
    // Branch operations
    bool createBranch(const std::string& branchName);
    bool checkout(const std::string& branchName);
    bool listBranches();
    
    // File operations
    bool show(const std::string& objectRef);
    
    // Remote operations
    bool clone(const std::string& sourceUrl, const std::string& targetDir = "");
    bool push(const std::string& remoteName = "origin", const std::string& branchName = "");
    bool addRemote(const std::string& name, const std::string& url);
    bool listRemotes();
    
    // Ignore file operations
    bool createIgnoreFile();
    bool isIgnored(const std::string& filepath) const;
    bool addIgnorePattern(const std::string& pattern);
    
    // GitHub-specific operations
    bool cloneFromGitHub(const std::string& repoUrl, const std::string& targetDir = "");
    bool pushToGitHub(const std::string& remoteName = "origin", const std::string& branchName = "");
    bool downloadGitHubRepo(const std::string& repoName, const std::string& targetDir);
    bool uploadToGitHub(const std::string& repoName, const std::string& branch = "main");
    bool setGitHubToken(const std::string& token);
    
    // Advanced feature system accessors
    MarkdownCommit* getMarkdownCommit() const { return markdownCommit.get(); }
    SemanticBranching* getSemanticBranching() const { return semanticBranching.get(); }
    SectionBasedStaging* getSectionStaging() const { return sectionStaging.get(); }
    ProjectMapper* getProjectMapper() const { return projectMapper.get(); }
    CheckpointSystem* getCheckpointSystem() const { return checkpointSystem.get(); }
    OopsShield* getOopsShield() const { return oopsShield.get(); }
    RewindMode* getRewindMode() const { return rewindMode.get(); }
    GuardrailSystem* getGuardrails() const { return guardrails.get(); }
    PluginManager* getPluginManager() const { return pluginManager.get(); }
    SessionRecorder* getSessionRecorder() const { return sessionRecorder.get(); }
    CommentThread* getCommentSystem() const { return commentSystem.get(); }
    StickyNotes* getStickyNotes() const { return stickyNotes.get(); }
    LabelSystem* getLabelSystem() const { return labelSystem.get(); }
    InitTemplates* getInitTemplates() const { return initTemplates.get(); }
    CommitStoryMode* getStoryMode() const { return storyMode.get(); }
    ContainerizedSnapshots* getSnapshots() const { return snapshots.get(); }
    TerminalUI* getTerminalUI() const { return terminalUI.get(); }
    
    // Getters
    std::string getRepoPath() const { return repoPath; }
    std::string getCurrentBranch() const;
    bool isRepository() const;
    
private:
    std::string repoPath;
    std::string gyattDir;
    std::unique_ptr<IgnoreList> ignoreList;
    std::string objectsDir;
    std::string refsDir;
    std::string headsDir;
    std::string remotesDir;
    std::string configFile;
    std::string indexFile;
    std::string headFile;
    
    // Advanced feature systems - initialized lazily
    mutable std::unique_ptr<MarkdownCommit> markdownCommit;
    mutable std::unique_ptr<SemanticBranching> semanticBranching;
    mutable std::unique_ptr<SectionBasedStaging> sectionStaging;
    mutable std::unique_ptr<ProjectMapper> projectMapper;
    mutable std::unique_ptr<CheckpointSystem> checkpointSystem;
    mutable std::unique_ptr<OopsShield> oopsShield;
    mutable std::unique_ptr<RewindMode> rewindMode;
    mutable std::unique_ptr<GuardrailSystem> guardrails;
    mutable std::unique_ptr<PluginManager> pluginManager;
    mutable std::unique_ptr<SessionRecorder> sessionRecorder;
    mutable std::unique_ptr<CommentThread> commentSystem;
    mutable std::unique_ptr<StickyNotes> stickyNotes;
    mutable std::unique_ptr<LabelSystem> labelSystem;
    mutable std::unique_ptr<InitTemplates> initTemplates;
    mutable std::unique_ptr<CommitStoryMode> storyMode;
    mutable std::unique_ptr<ContainerizedSnapshots> snapshots;
    mutable std::unique_ptr<TerminalUI> terminalUI;
    
    // Initialization methods for feature systems
    void initializeFeatureSystems() const;
    
    bool createDirectoryStructure();
    bool writeHead(const std::string& ref);
    std::string readHead() const;
    std::string getBranchCommit(const std::string& branchName);
    bool writeBranchCommit(const std::string& branchName, const std::string& commitHash);
    
    // Remote operations helpers
    bool copyRepository(const std::string& source, const std::string& target);
    bool syncObjects(const std::string& source, const std::string& target);
    bool syncRefs(const std::string& source, const std::string& target);
    std::map<std::string, std::string> parseConfig();
    
    // GitHub helpers
    bool isGitHubUrl(const std::string& url);
    std::string getGitHubApiUrl(const std::string& repoName);
    std::string getGitHubDownloadUrl(const std::string& repoName, const std::string& branch = "main");
    std::string getGitHubToken();
    bool createGitHubRepo(const std::string& repoName);
    bool uploadFilesToGitHub(const std::string& repoName, const std::string& branch);
    bool uploadToEmptyGitHubRepo(const std::string& repoName, const std::string& branch, const std::string& token);
    bool shouldExcludeFromGitHubUpload(const std::string& filePath);
};

} // namespace gyatt
