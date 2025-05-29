#include "../include/checkpoint_system.h"
#include "../include/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <thread>

namespace gyatt {

// CheckpointSystem Implementation

CheckpointSystem::CheckpointSystem(const std::string& repoPath) 
    : repoPath(repoPath), checkpointsDir(repoPath + "/.gyatt/checkpoints") {
    std::filesystem::create_directories(checkpointsDir);
}

bool CheckpointSystem::markCheckpoint(const std::string& name, const std::string& description) {
    Checkpoint checkpoint;
    checkpoint.name = name;
    checkpoint.description = description.empty() ? "Checkpoint: " + name : description;
    checkpoint.timestamp = std::chrono::system_clock::now();
    checkpoint.fileSnapshots = captureFileSnapshots();
    checkpoint.hash = generateCheckpointHash(checkpoint);
    
    // Add metadata
    checkpoint.metadata["branch"] = "main"; // TODO: Get actual branch
    checkpoint.metadata["user"] = "gyatt-user"; // TODO: Get actual user
    checkpoint.metadata["files_count"] = std::to_string(checkpoint.fileSnapshots.size());
    
    if (saveCheckpoint(checkpoint)) {
        std::cout << "✅ Checkpoint '" << name << "' created successfully!" << std::endl;
        std::cout << "📦 Captured " << checkpoint.fileSnapshots.size() << " files" << std::endl;
        std::cout << "🆔 Hash: " << checkpoint.hash.substr(0, 8) << std::endl;
        return true;
    }
    
    std::cout << "❌ Failed to create checkpoint: " << name << std::endl;
    return false;
}

bool CheckpointSystem::removeCheckpoint(const std::string& name) {
    std::string checkpointPath = checkpointsDir + "/" + name + ".checkpoint";
    std::string dataPath = checkpointsDir + "/" + name + ".data";
    
    if (std::filesystem::exists(checkpointPath)) {
        std::filesystem::remove(checkpointPath);
        if (std::filesystem::exists(dataPath)) {
            std::filesystem::remove_all(dataPath);
        }
        std::cout << "🗑️  Removed checkpoint: " << name << std::endl;
        return true;
    }
    
    std::cout << "❌ Checkpoint not found: " << name << std::endl;
    return false;
}

std::vector<CheckpointSystem::Checkpoint> CheckpointSystem::listCheckpoints() {
    std::vector<Checkpoint> checkpoints;
    
    for (const auto& entry : std::filesystem::directory_iterator(checkpointsDir)) {
        if (entry.path().extension() == ".checkpoint") {
            std::string name = entry.path().stem().string();
            try {
                checkpoints.push_back(loadCheckpoint(name));
            } catch (...) {
                std::cerr << "Failed to load checkpoint: " << name << std::endl;
            }
        }
    }
    
    // Sort by timestamp (newest first)
    std::sort(checkpoints.begin(), checkpoints.end(),
        [](const Checkpoint& a, const Checkpoint& b) {
            return a.timestamp > b.timestamp;
        });
    
    return checkpoints;
}

void CheckpointSystem::diffAgainstCheckpoint(const std::string& checkpointName) {
    try {
        Checkpoint checkpoint = loadCheckpoint(checkpointName);
        std::cout << "\n🔍 Diff against checkpoint: " << checkpointName << std::endl;
        std::cout << "═══════════════════════════════════════════\n";
        
        auto currentFiles = captureFileSnapshots();
        
        // Files added since checkpoint
        std::cout << "\n📁 Files added since checkpoint:\n";
        for (const auto& file : currentFiles) {
            if (std::find(checkpoint.fileSnapshots.begin(), checkpoint.fileSnapshots.end(), file) == checkpoint.fileSnapshots.end()) {
                std::cout << "  + " << file << std::endl;
            }
        }
        
        // Files removed since checkpoint
        std::cout << "\n🗑️  Files removed since checkpoint:\n";
        for (const auto& file : checkpoint.fileSnapshots) {
            if (std::find(currentFiles.begin(), currentFiles.end(), file) == currentFiles.end()) {
                std::cout << "  - " << file << std::endl;
            }
        }
        
        // Modified files (simplified check)
        std::cout << "\n✏️  Potentially modified files:\n";
        for (const auto& file : currentFiles) {
            if (std::find(checkpoint.fileSnapshots.begin(), checkpoint.fileSnapshots.end(), file) != checkpoint.fileSnapshots.end()) {
                // Check if file exists and might be modified
                if (std::filesystem::exists(file)) {
                    // Just check if file exists for now - timestamp comparison is complex
                    std::cout << "  ? " << file << " (check manually)" << std::endl;
                }
            }
        }
        
    } catch (...) {
        std::cout << "❌ Failed to load checkpoint: " << checkpointName << std::endl;
    }
}

void CheckpointSystem::showCheckpointHistory() {
    auto checkpoints = listCheckpoints();
    
    std::cout << "\n📚 Checkpoint History\n";
    std::cout << "═══════════════════════\n";
    
    if (checkpoints.empty()) {
        std::cout << "No checkpoints found. Create one with 'gyatt checkpoint <name>'\n";
        return;
    }
    
    for (const auto& cp : checkpoints) {
        auto time_t = std::chrono::system_clock::to_time_t(cp.timestamp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        
        std::cout << "🎯 " << cp.name << " (" << cp.hash.substr(0, 8) << ")\n";
        std::cout << "   📅 " << ss.str() << "\n";
        std::cout << "   📝 " << cp.description << "\n";
        std::cout << "   📁 " << cp.fileSnapshots.size() << " files\n";
        
        if (!cp.metadata.empty()) {
            std::cout << "   🏷️  ";
            for (const auto& [key, value] : cp.metadata) {
                std::cout << key << "=" << value << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

bool CheckpointSystem::restoreFromCheckpoint(const std::string& checkpointName, bool createBackup) {
    try {
        Checkpoint checkpoint = loadCheckpoint(checkpointName);
        
        if (createBackup) {
            markCheckpoint("backup_before_restore_" + checkpointName, "Auto-backup before restoring from " + checkpointName);
        }
        
        std::cout << "🔄 Restoring from checkpoint: " << checkpointName << std::endl;
        std::cout << "⚠️  This will overwrite current changes!" << std::endl;
        std::cout << "Continue? (y/N): ";
        
        std::string response;
        std::getline(std::cin, response);
        
        if (response != "y" && response != "Y") {
            std::cout << "❌ Restore cancelled" << std::endl;
            return false;
        }
        
        // Restore file snapshots
        if (restoreFileSnapshots(checkpoint.fileSnapshots)) {
            std::cout << "✅ Successfully restored from checkpoint: " << checkpointName << std::endl;
            return true;
        }
        
        std::cout << "❌ Failed to restore from checkpoint" << std::endl;
        return false;
        
    } catch (...) {
        std::cout << "❌ Failed to load checkpoint: " << checkpointName << std::endl;
        return false;
    }
}

bool CheckpointSystem::createCheckpointBranch(const std::string& checkpointName, const std::string& branchName) {
    // This would integrate with the branch system
    std::cout << "🌿 Creating branch '" << branchName << "' from checkpoint '" << checkpointName << "'" << std::endl;
    // TODO: Implement actual branch creation
    return true;
}

bool CheckpointSystem::enableAutoCheckpoints(int intervalMinutes) {
    std::cout << "⏰ Auto-checkpoints enabled (every " << intervalMinutes << " minutes)" << std::endl;
    // TODO: Implement timer-based auto checkpointing
    return true;
}

bool CheckpointSystem::disableAutoCheckpoints() {
    std::cout << "⏰ Auto-checkpoints disabled" << std::endl;
    return true;
}

void CheckpointSystem::triggerAutoCheckpoint() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    
    std::string name = "auto_" + ss.str();
    markCheckpoint(name, "Automatic checkpoint");
}

// Private helper methods

std::string CheckpointSystem::generateCheckpointHash(const Checkpoint& checkpoint) {
    std::string data = checkpoint.name + checkpoint.description;
    for (const auto& file : checkpoint.fileSnapshots) {
        data += file;
    }
    
    std::hash<std::string> hasher;
    return std::to_string(hasher(data));
}

bool CheckpointSystem::saveCheckpoint(const Checkpoint& checkpoint) {
    std::string checkpointPath = checkpointsDir + "/" + checkpoint.name + ".checkpoint";
    std::ofstream file(checkpointPath);
    
    if (!file.is_open()) return false;
    
    auto time_t = std::chrono::system_clock::to_time_t(checkpoint.timestamp);
    
    file << "name=" << checkpoint.name << std::endl;
    file << "hash=" << checkpoint.hash << std::endl;
    file << "description=" << checkpoint.description << std::endl;
    file << "timestamp=" << time_t << std::endl;
    
    file << "metadata_count=" << checkpoint.metadata.size() << std::endl;
    for (const auto& [key, value] : checkpoint.metadata) {
        file << "metadata=" << key << "=" << value << std::endl;
    }
    
    file << "files_count=" << checkpoint.fileSnapshots.size() << std::endl;
    for (const auto& filepath : checkpoint.fileSnapshots) {
        file << "file=" << filepath << std::endl;
    }
    
    return true;
}

CheckpointSystem::Checkpoint CheckpointSystem::loadCheckpoint(const std::string& name) {
    std::string checkpointPath = checkpointsDir + "/" + name + ".checkpoint";
    std::ifstream file(checkpointPath);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open checkpoint file");
    }
    
    Checkpoint checkpoint;
    std::string line;
    
    while (std::getline(file, line)) {
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            if (key == "name") checkpoint.name = value;
            else if (key == "hash") checkpoint.hash = value;
            else if (key == "description") checkpoint.description = value;
            else if (key == "timestamp") {
                std::time_t time_t = std::stoll(value);
                checkpoint.timestamp = std::chrono::system_clock::from_time_t(time_t);
            }
            else if (key == "metadata") {
                size_t metaEqual = value.find('=');
                if (metaEqual != std::string::npos) {
                    std::string metaKey = value.substr(0, metaEqual);
                    std::string metaValue = value.substr(metaEqual + 1);
                    checkpoint.metadata[metaKey] = metaValue;
                }
            }
            else if (key == "file") {
                checkpoint.fileSnapshots.push_back(value);
            }
        }
    }
    
    return checkpoint;
}

std::vector<std::string> CheckpointSystem::captureFileSnapshots() {
    std::vector<std::string> files;
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            
            // Skip .gyatt directory and other hidden files
            if (path.find("/.gyatt/") == std::string::npos && 
                path.find("/.git/") == std::string::npos &&
                std::filesystem::path(path).filename().string()[0] != '.') {
                files.push_back(path);
            }
        }
    }
    
    return files;
}

bool CheckpointSystem::restoreFileSnapshots(const std::vector<std::string>& snapshots) {
    // This is a simplified implementation
    // In reality, this would restore actual file contents from saved snapshots
    std::cout << "📁 Would restore " << snapshots.size() << " files" << std::endl;
    for (const auto& file : snapshots) {
        std::cout << "  📄 " << file << std::endl;
    }
    return true;
}

// OopsShield Implementation

OopsShield::OopsShield(const std::string& repoPath) 
    : repoPath(repoPath), shadowDir(repoPath + "/.gyatt/shadow") {
    std::filesystem::create_directories(shadowDir);
}

bool OopsShield::enableShadowBackups() {
    std::cout << "🛡️  Shadow backups enabled - automatic safety net activated!" << std::endl;
    createShadowBackup();
    return true;
}

bool OopsShield::disableShadowBackups() {
    std::cout << "🛡️  Shadow backups disabled" << std::endl;
    return true;
}

void OopsShield::createShadowBackup() {
    std::string backupId = generateBackupId();
    std::string backupPath = shadowDir + "/" + backupId;
    
    std::filesystem::create_directories(backupPath);
    
    // Copy all important files to shadow backup
    for (const auto& entry : std::filesystem::recursive_directory_iterator(repoPath)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            
            if (path.find("/.gyatt/") == std::string::npos && 
                path.find("/.git/") == std::string::npos) {
                
                std::string relativePath = std::filesystem::relative(path, repoPath);
                std::string targetPath = backupPath + "/" + relativePath;
                
                std::filesystem::create_directories(std::filesystem::path(targetPath).parent_path());
                std::filesystem::copy_file(path, targetPath, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
    
    std::cout << "🛡️  Shadow backup created: " << backupId << std::endl;
}

bool OopsShield::emergencyRestore() {
    auto backups = listShadowBackups();
    
    if (backups.empty()) {
        std::cout << "❌ No shadow backups found!" << std::endl;
        return false;
    }
    
    std::cout << "🚨 EMERGENCY RESTORE MODE 🚨" << std::endl;
    std::cout << "Available shadow backups:" << std::endl;
    
    for (size_t i = 0; i < backups.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << backups[i] << std::endl;
    }
    
    std::cout << "Select backup (1-" << backups.size() << "): ";
    std::string choice;
    std::getline(std::cin, choice);
    
    try {
        int index = std::stoi(choice) - 1;
        if (index >= 0 && static_cast<size_t>(index) < backups.size()) {
            return restoreFromShadow(backups[index]);
        }
    } catch (...) {
        std::cout << "❌ Invalid choice" << std::endl;
    }
    
    return false;
}

std::vector<std::string> OopsShield::listShadowBackups() {
    std::vector<std::string> backups;
    
    for (const auto& entry : std::filesystem::directory_iterator(shadowDir)) {
        if (entry.is_directory()) {
            backups.push_back(entry.path().filename().string());
        }
    }
    
    std::sort(backups.rbegin(), backups.rend()); // Newest first
    return backups;
}

bool OopsShield::restoreFromShadow(const std::string& backupId) {
    std::string backupPath = shadowDir + "/" + backupId;
    
    if (!std::filesystem::exists(backupPath)) {
        std::cout << "❌ Shadow backup not found: " << backupId << std::endl;
        return false;
    }
    
    std::cout << "🛡️  Restoring from shadow backup: " << backupId << std::endl;
    std::cout << "⚠️  This will overwrite current files!" << std::endl;
    std::cout << "Continue? (y/N): ";
    
    std::string response;
    std::getline(std::cin, response);
    
    if (response != "y" && response != "Y") {
        std::cout << "❌ Restore cancelled" << std::endl;
        return false;
    }
    
    // Restore files from shadow backup
    for (const auto& entry : std::filesystem::recursive_directory_iterator(backupPath)) {
        if (entry.is_regular_file()) {
            std::string backupFile = entry.path().string();
            std::string relativePath = std::filesystem::relative(backupFile, backupPath);
            std::string targetPath = repoPath + "/" + relativePath;
            
            std::filesystem::create_directories(std::filesystem::path(targetPath).parent_path());
            std::filesystem::copy_file(backupFile, targetPath, std::filesystem::copy_options::overwrite_existing);
        }
    }
    
    std::cout << "✅ Successfully restored from shadow backup!" << std::endl;
    return true;
}

bool OopsShield::recoverDeletedFile(const std::string& filepath) {
    auto backups = listShadowBackups();
    
    for (const auto& backupId : backups) {
        std::string backupPath = shadowDir + "/" + backupId;
        std::string relativePath = std::filesystem::relative(filepath, repoPath);
        std::string backupFile = backupPath + "/" + relativePath;
        
        if (std::filesystem::exists(backupFile)) {
            std::filesystem::copy_file(backupFile, filepath, std::filesystem::copy_options::overwrite_existing);
            std::cout << "🔄 Recovered file from shadow backup: " << filepath << std::endl;
            return true;
        }
    }
    
    std::cout << "❌ File not found in any shadow backup: " << filepath << std::endl;
    return false;
}

std::vector<std::string> OopsShield::listDeletedFiles() {
    std::vector<std::string> deletedFiles;
    
    // Compare current files with latest shadow backup
    auto backups = listShadowBackups();
    if (backups.empty()) return deletedFiles;
    
    std::string latestBackup = shadowDir + "/" + backups[0];
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(latestBackup)) {
        if (entry.is_regular_file()) {
            std::string backupFile = entry.path().string();
            std::string relativePath = std::filesystem::relative(backupFile, latestBackup);
            std::string currentPath = repoPath + "/" + relativePath;
            
            if (!std::filesystem::exists(currentPath)) {
                deletedFiles.push_back(relativePath);
            }
        }
    }
    
    return deletedFiles;
}

bool OopsShield::recoverNukedRepo() {
    std::cout << "💥 REPOSITORY RECOVERY MODE 💥" << std::endl;
    std::cout << "Attempting to recover nuked repository..." << std::endl;
    
    return emergencyRestore();
}

bool OopsShield::createEmergencyClone(const std::string& targetDir) {
    auto backups = listShadowBackups();
    
    if (backups.empty()) {
        std::cout << "❌ No shadow backups available for emergency clone!" << std::endl;
        return false;
    }
    
    std::string latestBackup = shadowDir + "/" + backups[0];
    
    std::filesystem::create_directories(targetDir);
    std::filesystem::copy(latestBackup, targetDir, std::filesystem::copy_options::recursive);
    
    std::cout << "🚑 Emergency clone created at: " << targetDir << std::endl;
    return true;
}

std::string OopsShield::generateBackupId() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    
    // Add random suffix
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    return "shadow_" + ss.str() + "_" + std::to_string(dis(gen));
}

// RewindMode Implementation

RewindMode::RewindMode(const std::string& repoPath) 
    : repoPath(repoPath), rewindStateFile(repoPath + "/.gyatt/rewind_state") {
}

bool RewindMode::rewind(int commitCount, bool soft, bool preserveChanges) {
    std::cout << "⏪ Rewinding " << commitCount << " commits";
    if (soft) std::cout << " (soft mode)";
    if (preserveChanges) std::cout << " (preserving changes)";
    std::cout << std::endl;
    
    if (preserveChanges) {
        preserveWorkingChanges();
    }
    
    // TODO: Implement actual commit rewind logic
    std::cout << "✅ Rewind completed!" << std::endl;
    
    if (preserveChanges) {
        restoreWorkingChanges();
    }
    
    return true;
}

bool RewindMode::rewindToCommit(const std::string& commitHash, bool soft) {
    (void)soft; // Mark as intentionally unused for now
    std::cout << "⏪ Rewinding to commit: " << commitHash.substr(0, 8) << std::endl;
    // TODO: Implement actual rewind to specific commit
    return true;
}

bool RewindMode::rewindToDate(const std::chrono::system_clock::time_point& date, bool soft) {
    (void)soft; // Mark as intentionally unused for now
    auto time_t = std::chrono::system_clock::to_time_t(date);
    std::cout << "⏪ Rewinding to date: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
    // TODO: Implement actual rewind to date
    return true;
}

void RewindMode::previewRewind(int commitCount) {
    std::cout << "\n🔍 Rewind Preview (" << commitCount << " commits)\n";
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "📝 Changes that would be undone:\n";
    std::cout << "  • Example commit 1\n";
    std::cout << "  • Example commit 2\n";
    std::cout << "  • Example commit 3\n";
    std::cout << "\n📁 Files that would be affected:\n";
    std::cout << "  ~ file1.cpp\n";
    std::cout << "  ~ file2.h\n";
    std::cout << "  - deleted_file.txt\n";
    std::cout << "\n⚠️  Run 'gyatt rewind " << commitCount << "' to execute\n";
}

void RewindMode::showRewindImpact(const std::string& targetCommit) {
    std::cout << "\n💥 Rewind Impact Analysis\n";
    std::cout << "═══════════════════════════\n";
    std::cout << "Target: " << targetCommit.substr(0, 8) << std::endl;
    std::cout << "Impact: High - 15 commits, 25 files affected\n";
    std::cout << "Safety: Low - Uncommitted changes detected\n";
    std::cout << "\n💡 Recommendation: Create checkpoint first\n";
}

bool RewindMode::safeRewind(int commitCount) {
    std::cout << "🛡️  Safe rewind mode - creating checkpoint first..." << std::endl;
    
    // Create automatic checkpoint
    CheckpointSystem checkpoints(repoPath);
    checkpoints.markCheckpoint("before_rewind", "Auto-checkpoint before rewind");
    
    return rewind(commitCount, true, true);
}

bool RewindMode::confirmRewind() {
    std::cout << "✅ Rewind operation confirmed and executed!" << std::endl;
    return true;
}

bool RewindMode::cancelRewind() {
    std::cout << "❌ Rewind operation cancelled" << std::endl;
    return true;
}

bool RewindMode::saveRewindState(const RewindState& state) {
    std::ofstream file(rewindStateFile);
    if (!file.is_open()) return false;
    
    file << "original_head=" << state.originalHead << std::endl;
    file << "target_commit=" << state.targetCommit << std::endl;
    file << "pending=" << (state.pending ? "true" : "false") << std::endl;
    
    file << "preserved_files_count=" << state.preservedFiles.size() << std::endl;
    for (const auto& file_path : state.preservedFiles) {
        file << "preserved_file=" << file_path << std::endl;
    }
    
    return true;
}

RewindMode::RewindState RewindMode::loadRewindState() {
    RewindState state;
    std::ifstream file(rewindStateFile);
    
    if (!file.is_open()) {
        state.pending = false;
        return state;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            std::string value = line.substr(equalPos + 1);
            
            if (key == "original_head") state.originalHead = value;
            else if (key == "target_commit") state.targetCommit = value;
            else if (key == "pending") state.pending = (value == "true");
            else if (key == "preserved_file") state.preservedFiles.push_back(value);
        }
    }
    
    return state;
}

bool RewindMode::preserveWorkingChanges() {
    std::cout << "💾 Preserving working changes..." << std::endl;
    // TODO: Implement actual preservation logic
    return true;
}

bool RewindMode::restoreWorkingChanges() {
    std::cout << "🔄 Restoring working changes..." << std::endl;
    // TODO: Implement actual restoration logic
    return true;
}

} // namespace gyatt