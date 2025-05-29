#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace gyatt {

class CheckpointSystem {
public:
    struct Checkpoint {
        std::string name;
        std::string hash;
        std::string description;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, std::string> metadata;
        std::vector<std::string> fileSnapshots;
    };

    CheckpointSystem(const std::string& repoPath);
    
    // Checkpoint operations
    bool markCheckpoint(const std::string& name, const std::string& description = "");
    bool removeCheckpoint(const std::string& name);
    std::vector<Checkpoint> listCheckpoints();
    
    // Diff against checkpoints
    void diffAgainstCheckpoint(const std::string& checkpointName);
    void showCheckpointHistory();
    
    // Checkpoint restoration
    bool restoreFromCheckpoint(const std::string& checkpointName, bool createBackup = true);
    bool createCheckpointBranch(const std::string& checkpointName, const std::string& branchName);
    
    // Auto-checkpointing
    bool enableAutoCheckpoints(int intervalMinutes = 30);
    bool disableAutoCheckpoints();
    void triggerAutoCheckpoint();
    
private:
    std::string repoPath;
    std::string checkpointsDir;
    
    std::string generateCheckpointHash(const Checkpoint& checkpoint);
    bool saveCheckpoint(const Checkpoint& checkpoint);
    Checkpoint loadCheckpoint(const std::string& name);
    std::vector<std::string> captureFileSnapshots();
    bool restoreFileSnapshots(const std::vector<std::string>& snapshots);
};

class OopsShield {
public:
    OopsShield(const std::string& repoPath);
    
    // Shadow backup system
    bool enableShadowBackups();
    bool disableShadowBackups();
    void createShadowBackup();
    
    // Emergency recovery
    bool emergencyRestore();
    std::vector<std::string> listShadowBackups();
    bool restoreFromShadow(const std::string& backupId);
    
    // File recovery
    bool recoverDeletedFile(const std::string& filepath);
    std::vector<std::string> listDeletedFiles();
    
    // Repository recovery
    bool recoverNukedRepo();
    bool createEmergencyClone(const std::string& targetDir);
    
private:
    std::string repoPath;
    std::string shadowDir;
    
    void scheduleBackup();
    std::string generateBackupId();
};

class RewindMode {
public:
    RewindMode(const std::string& repoPath);
    
    // Rewind operations
    bool rewind(int commitCount, bool soft = true, bool preserveChanges = true);
    bool rewindToCommit(const std::string& commitHash, bool soft = true);
    bool rewindToDate(const std::chrono::system_clock::time_point& date, bool soft = true);
    
    // Preview rewind
    void previewRewind(int commitCount);
    void showRewindImpact(const std::string& targetCommit);
    
    // Safe rewind with staging
    bool safeRewind(int commitCount);
    bool confirmRewind();
    bool cancelRewind();
    
private:
    std::string repoPath;
    std::string rewindStateFile;
    
    struct RewindState {
        std::string originalHead;
        std::string targetCommit;
        std::vector<std::string> preservedFiles;
        bool pending;
    };
    
    bool saveRewindState(const RewindState& state);
    RewindState loadRewindState();
    bool preserveWorkingChanges();
    bool restoreWorkingChanges();
};

} // namespace gyatt
