#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include <chrono>
#include <functional>
#include "ignore.h"

// Enhanced remote repository management structures
namespace gyatt {

enum class RemoteProtocol {
    HTTP,
    HTTPS,
    SSH,
    LOCAL,
    UNKNOWN
};

enum class AuthMethod {
    NONE,
    TOKEN,
    SSH_KEY,
    USERNAME_PASSWORD,
    OAUTH
};

enum class SyncMode {
    FULL,
    SELECTIVE,
    INCREMENTAL,
    SMART
};

struct RemoteCredentials {
    AuthMethod method = AuthMethod::NONE;
    std::string username;
    std::string token;
    std::string sshKeyPath;
    std::string oauthToken;
    bool rememberCredentials = false;
    std::chrono::system_clock::time_point expiryTime;
};

struct RemoteRepository {
    std::string name;
    std::string url;
    RemoteProtocol protocol;
    bool isGyattRepo = false;
    bool isHealthy = true;
    std::string lastError;
    std::chrono::system_clock::time_point lastSync;
    RemoteCredentials credentials;
    std::map<std::string, std::string> branches; // local -> remote mapping
    std::vector<std::string> syncProfiles;
};

struct SyncProfile {
    std::string name;
    SyncMode mode;
    std::vector<std::string> includePaths;
    std::vector<std::string> excludePaths;
    bool autoSync = false;
    std::chrono::minutes syncInterval{60};
    std::function<void(const std::string&)> progressCallback;
};

struct PushProgress {
    std::string phase;
    std::string message;
    size_t current = 0;
    size_t total = 0;
    size_t totalFiles = 0;
    size_t processedFiles = 0;
    size_t totalBytes = 0;
    size_t transferredBytes = 0;
    double percentComplete = 0.0;
    std::string currentOperation;
    bool isComplete = false;
    std::string errorMessage;
    std::chrono::system_clock::time_point startTime;
    std::chrono::milliseconds elapsedTime{0};
};

}

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
    
        // Enhanced Remote operations
    bool clone(const std::string& sourceUrl, const std::string& targetDir = "");
    bool push(const std::string& remoteName = "origin", const std::string& branchName = "");
    bool addRemote(const std::string& name, const std::string& url);
    std::vector<RemoteRepository> listRemotes();
    void printRemotes();  // For CLI display
    
    // Enhanced remote repository methods
    bool addRemoteWithAuth(const std::string& name, const std::string& url, const RemoteCredentials& credentials);
    std::vector<RemoteRepository> getRemoteRepositories() const;
    
    // GitHub-specific methods
    bool cloneFromGitHub(const std::string& repoUrl, const std::string& targetDir);
    bool pushToGitHub(const std::string& remoteName, const std::string& branchName);
    bool downloadGitHubRepo(const std::string& repoName, const std::string& targetDir);
    bool uploadToGitHub(const std::string& repoName, const std::string& branch);
    
    // Protocol Detection and URL Validation
    RemoteProtocol detectProtocol(const std::string& url);
    bool validateRemoteUrl(const std::string& url);
    std::string normalizeRemoteUrl(const std::string& url);
    
    // Enhanced Push/Pull with Progress
    bool pushWithProgress(const std::string& remoteName, const std::string& branchName, 
                         std::function<void(const PushProgress&)> progressCallback = nullptr);
    bool pullWithProgress(const std::string& remoteName, const std::string& branchName,
                         std::function<void(const PushProgress&)> progressCallback = nullptr);
    bool pushWithRetry(const std::string& remoteName, const std::string& branchName, 
                      int maxRetries = 3, int delaySeconds = 5);
    
    // Branch Tracking and Synchronization
    bool setBranchUpstream(const std::string& localBranch, const std::string& remoteName, 
                          const std::string& remoteBranch);
    std::string getBranchUpstream(const std::string& branchName);
    std::vector<std::string> getRemoteBranches(const std::string& remoteName);
    bool synchronizeBranches(const std::string& remoteName, bool dryRun = false);
    
    // Conflict Detection and Resolution
    bool detectPushConflicts(const std::string& remoteName, const std::string& branchName);
    std::vector<std::string> getPushConflicts(const std::string& remoteName, const std::string& branchName);
    bool stashAndPull(const std::string& remoteName, const std::string& branchName);
    bool popStashAfterPull();
    
    // Sync Profiles and Automation
    bool createSyncProfile(const SyncProfile& profile);
    SyncProfile createSyncProfile(const std::string& name, SyncMode mode, 
                                const std::vector<std::string>& includePaths,
                                const std::vector<std::string>& excludePaths);
    bool deleteSyncProfile(const std::string& profileName);
    std::vector<SyncProfile> getSyncProfiles() const;
    bool applySyncProfile(const std::string& profileName, const std::string& remoteName);
    bool enableAutoSync(const std::string& remoteName, const std::string& profileName);
    bool disableAutoSync(const std::string& remoteName);
    
    // Missing methods needed for CLI
    bool setGitHubToken(const std::string& token);
    bool createIgnoreFile();
    bool isIgnored(const std::string& filepath);
    bool addIgnorePattern(const std::string& pattern);
    
    // Helper methods for UI
    std::string getProtocolName(RemoteProtocol protocol);
    std::string getAuthMethodName(AuthMethod method);
    std::string getSyncModeName(SyncMode mode);
    
    // Enhanced remote repository management with overloaded methods
    bool checkRemoteHealth(const RemoteRepository& remote);
    bool authenticateWithRemote(const RemoteRepository& remote);
    bool uploadFileToRemote(const RemoteRepository& remote, const std::string& filePath);
    std::vector<std::string> getModifiedFiles();
    RemoteRepository loadRemoteConfig(const std::string& name);
    void updateLastSyncTime(const std::string& remoteName);
    std::time_t getLastSyncTime(const std::string& remoteName);
    
    // HTTP/SSH connection testing
    bool testHttpConnection(const std::string& url);
    bool testSshConnection(const std::string& url, const RemoteCredentials& credentials);
    std::string extractHostFromSshUrl(const std::string& url);
    
    // Sync profile persistence
    void saveSyncProfile(const SyncProfile& profile, const std::string& path);
    SyncProfile loadSyncProfile(const std::string& path) const;
    
    // Additional methods for enhanced push functionality
    bool pushToRemoteWithProgress(const std::string& remoteName, const std::string& branch,
                                 std::function<void(const PushProgress&)> progressCallback);
    bool hasConflicts(const RemoteRepository& remote, const std::string& branch);
    
    // Public getter for repo path
    std::string getRepoPath() const { return repoPath; }
    CommentThread* getCommentSystem() const { return commentSystem.get(); }
    StickyNotes* getStickyNotes() const { return stickyNotes.get(); }
    LabelSystem* getLabelSystem() const { return labelSystem.get(); }
    InitTemplates* getInitTemplates() const { return initTemplates.get(); }
    CommitStoryMode* getStoryMode() const { return storyMode.get(); }
    ContainerizedSnapshots* getSnapshots() const { return snapshots.get(); }
    TerminalUI* getTerminalUI() const { return terminalUI.get(); }
    
    // Other getters
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
