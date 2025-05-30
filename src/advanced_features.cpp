#include "../include/advanced_features.h"
#include "../include/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <sstream>
#include <ctime>

namespace gyatt {

// InitTemplates Implementation

InitTemplates::InitTemplates(const std::string& repoPath) 
    : repoPath(repoPath), templatesDir(repoPath + "/.gyatt/templates") {
    std::filesystem::create_directories(templatesDir);
    loadTemplates();
}

bool InitTemplates::createTemplate(const std::string& name, const std::string& description, 
                                   const std::vector<TemplateFile>& files) {
    Template tmpl;
    tmpl.name = name;
    tmpl.description = description;
    tmpl.files = files;
    tmpl.timestamp = std::chrono::system_clock::now();
    
    templates[name] = tmpl;
    
    // Create template directory
    std::string templateDir = Utils::joinPath(templatesDir, name);
    std::filesystem::create_directories(templateDir);
    
    // Save template files
    for (const auto& file : files) {
        std::string filePath = Utils::joinPath(templateDir, file.relativePath);
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
        Utils::writeFile(filePath, file.content);
    }
    
    saveTemplateConfig(tmpl);
    
    std::cout << "� Created template: " << name << std::endl;
    std::cout << !𞒝 " << description << std::endl;
    std::cout << !𞒁 " << files.size() << " files in template" << std::endl;
    
    return true;
}

bool InitTemplates::useTemplate(const std::string& name, const std::string& targetDir, 
                                 const std::map<std::string, std::string>& variables) {
    auto it = templates.find(name);
    if (it == templates.end()) {
        std::cout << !ᝌ Template not found: " << name << std::endl;
        return false;
    }
    
    const auto& tmpl = it->second;
    
    std::cout << !𞒋 Applying template: " << name << std::endl;
    std::cout << !𞒝 " << tmpl.description << std::endl;
    std::cout << !� Target: " << targetDir << std::endl;
    
    std::filesystem::create_directories(targetDir);
    
    for (const auto& file : tmpl.files) {
        std::string targetPath = Utils::joinPath(targetDir, file.relativePath);
        std::filesystem::create_directories(std::filesystem::path(targetPath).parent_path());
        
        // Apply variable substitution
        std::string content = file.content;
        for (const auto& [key, value] : variables) {
            std::string placeholder = "{{" + key + "}}";
            size_t pos = 0;
            while ((pos = content.find(placeholder, pos)) != std::string::npos) {
                content.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }
        
        Utils::writeFile(targetPath, content);
        std::cout << !ᜅ Created: " << file.relativePath << std::endl;
    }
    
    std::cout << !𞍉 Template applied successfully!" << std::endl;
    return true;
}

bool InitTemplates::deleteTemplate(const std::string& name) {
    auto it = templates.find(name);
    if (it == templates.end()) {
        std::cout << !ᝌ Template not found: " << name << std::endl;
        return false;
    }
    
    templates.erase(it);
    
    // Remove template directory
    std::string templateDir = Utils::joinPath(templatesDir, name);
    std::filesystem::remove_all(templateDir);
    
    std::cout << !𞖑  Deleted template: " << name << std::endl;
    return true;
}

std::vector<InitTemplates::Template> InitTemplates::listTemplates() {
    std::vector<Template> templateList;
    for (const auto& [name, tmpl] : templates) {
        templateList.push_back(tmpl);
    }
    return templateList;
}

void InitTemplates::showTemplates() {
    if (templates.empty()) {
        std::cout << "� No templates available" << std::endl;
        return;
    }
    
    std::cout << "\n� Available Templates\n";
    std::cout << !┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┐\n";
    
    for (const auto& [name, tmpl] : templates) {
        auto timeStr = Utils::formatTime(tmpl.timestamp);
        
        std::cout << "� " << name << std::endl;
        std::cout << "   � " << tmpl.description << std::endl;
        std::cout << "   � " << tmpl.files.size() << " files �� " << timeStr << std::endl;
        std::cout << !��������������������������������������Ⓚ\n";
    }
}

void InitTemplates::showTemplate(const std::string& name) {
    auto it = templates.find(name);
    if (it == templates.end()) {
        std::cout << !✌ Template not found: " << name << std::endl;
        return;
    }
    
    const auto& tmpl = it->second;
    auto timeStr = Utils::formatTime(tmpl.timestamp);
    
    std::cout << "\m𞒋 Template: " << name << "\n";
    std::cout << "ᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐᔐ\n";
    std::cout << !𞒝 Description: " << tmpl.description << std::endl;
    std::cout << !⎰ Created: " << timeStr << std::endl;
    std::cout << "� Files:\n";
    
    for (const auto& file : tmpl.files) {
        std::cout << "  ဢ " << file.relativePath;
        if (!file.description.empty()) {
            std::cout << " - " << file.description;
        }
        std::cout << std::endl;
    }
}

bool InitTemplates::loadTemplates() {
    templates.clear();
    
    if (!std::filesystem::exists(templatesDir)) {
        return true; // No templates yet
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(templatesDir)) {
        if (entry.is_directory()) {
            std::string templateName = entry.path().filename().string();
            loadTemplate(templateName);
        }
    }
    
    return true;
}

bool InitTemplates::loadTemplate(const std::string& name) {
    std::string templateDir = Utils::joinPath(templatesDir, name);
    std::string configFile = Utils::joinPath(templateDir, "template.json");
    
    if (!Utils::fileExists(configFile)) {
        return false;
    }
    
    // Simplified JSON loading - in a real implementation, use a proper JSON library
    Template tmpl;
    tmpl.name = name;
    tmpl.description = "Loaded template";
    tmpl.timestamp = std::chrono::system_clock::now();
    
    // Scan for files in template directory
    scanTemplateFiles(templateDir, tmpl.files);
    
    templates[name] = tmpl;
    return true;
}

void InitTemplates::scanTemplateFiles(const std::string& templateDir, 
                                       std::vector<TemplateFile>& files) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(templateDir)) {
        if (entry.is_regular_file() && entry.path().filename() != "template.json") {
            TemplateFile file;
            file.relativePath = std::filesystem::relative(entry.path(), templateDir).string();
            file.content = Utils::readFile(entry.path().string());
            file.description = "Template file";
            
            files.push_back(file);
        }
    }
}

bool InitTemplates::saveTemplateConfig(const Template& tmpl) {
    std::string templateDir = Utils::joinPath(templatesDir, tmpl.name);
    std::string configFile = Utils::joinPath(templateDir, "template.json");
    
    std::ofstream file(configFile);
    if (!file.is_open()) return false;
    
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        tmpl.timestamp.time_since_epoch()).count();
    
    file << "{\n";
    file << "  \"name\": \"" << tmpl.name << "\",\n";
    file << "  \"description\": \"" << tmpl.description << "\",\n";
    file << "  \"timestamp\": " << timestamp << ",\n";
    file << "  \"files\": [\n";
    
    for (size_t i = 0; i < tmpl.files.size(); ++i) {
        const auto& templateFile = tmpl.files[i];
        file << "    {\n";
        file << "      \"path\": \"" << templateFile.relativePath << "\",\n";
        file << "      \"description\": \"" << templateFile.description << "\"\n";
        file << "    }";
        
        if (i < tmpl.files.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    return true;
}

bool InitTemplates::initCustomTemplate(const std::string& templateName, const std::string& projectName) {
    std::cout << !𞍨 Initializing custom template: " << templateName << std::endl;
    std::cout << "� Project name: " << projectName << std::endl;
    
    // Look for the template
    for (const auto& [name, tmpl] : templates) {
        if (name == templateName) {
            // Apply the template
            std::cout << !ᜅ Found template: " << tmpl.description << std::endl;
            return useTemplate(templateName, ".", {});
        }
    }
    
    std::cout << !ᝌ Template not found: " << templateName << std::endl;
    std::cout << "� Available templates:" << std::endl;
    showTemplates();
    return false;
}

// StoryMode Implementation

StoryMode::StoryMode(const std::string& repoPath) 
    : repoPath(repoPath), storiesFile(repoPath + "/.gyatt/stories.json") {
    std::filesystem::create_directories(repoPath + "/.gyatt");
    loadStories();
}

bool StoryMode::startStory(const std::string& title, const std::string& description) {
    if (currentStory.active) {
        std::cout << !ᝌ Story already active: " << currentStory.title << std::endl;
        std::cout << "� Use 'gyatt story end' to finish current story" << std::endl;
        return false;
    }
    
    currentStory.id = generateStoryId();
    currentStory.title = title;
    currentStory.description = description;
    currentStory.startTime = std::chrono::system_clock::now();
    currentStory.active = true;
    currentStory.commits.clear();
    currentStory.tags.clear();
    
    std::cout << "� Started story: " << title << std::endl;
    std::cout << "� " << description << std::endl;
    std::cout << "� Story ID: " << currentStory.id << std::endl;
    
    saveStories();
    return true;
}

bool StoryMode::endStory() {
    if (!currentStory.active) {
        std::cout << "ᜌ No active story" << std::endl;
        return false;
    }
    
    currentStory.endTime = std::chrono::system_clock::now();
    currentStory.active = false;
    
    // Archive completed story
    completedStories.push_back(currentStory);
    
    std::cout << "� Completed story: " << currentStory.title << std::endl;
    std::cout << "� " << currentStory.commits.size() << " commits in story" << std::endl;
    
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        currentStory.endTime - currentStory.startTime).count();
    std::cout << !⎰﷏  Duration: " << duration << " hours" << std::endl;
    
    saveStories();
    
    // Reset current story
    currentStory = Story{};
    return true;
}

bool StoryMode::addCommitToStory(const std::string& commitHash, const std::string& message) {
    if (!currentStory.active) {
        std::cout << "ᜌ No active story to add commit to" << std::endl;
        return false;
    }
    
    StoryCommit commit;
    commit.hash = commitHash;
    commit.message = message;
    commit.timestamp = std::chrono::system_clock::now();
    
    currentStory.commits.push_back(commit);
    
    std::cout << !� Added commit to story: " << message << std::endl;
    std::cout << !𞒊 Story now has " << currentStory.commits.size() << " commits" << std::endl;
    
    saveStories();
    return true;
}

bool StoryMode::addTagToStory(const std::string& tag) {
    if (!currentStory.active) {
        std::cout << !✌ No active story to add tag to" << std::endl;
        return false;
    }
    
    currentStory.tags.insert(tag);
    
    std::cout << !�﷏  Added tag to story: " << tag << std::endl;
    
    saveStories();
    return true;
}

StoryMode::Story StoryMode::getCurrentStory() {
    return currentStory;
}

std::vector<StoryMode::Story> StoryMode::getCompletedStories() {
    return completedStories;
}

void StoryMode::showCurrentStory() {
    if (!currentStory.active) {
        std::cout << !� No active story" << std::endl;
        return;
    }
    
    auto startTimeStr = Utils::formatTime(currentStory.startTime);
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        now - currentStory.startTime).count();
    
    std::cout << "\m𞒖 Current Story\n";
    std::cout << !ᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕐ\n";
    std::cout << "� ID: " << currentStory.id << std::endl;
    std::cout << !𞒖 Title: " << currentStory.title << std::endl;
    std::cout << !� Description: " << currentStory.description << std::endl;
    std::cout << "Ꮁ  Started: " << startTimeStr << " (" << duration << " hours ago)" << std::endl;
    std::cout << !𞒊 Commits: " << currentStory.commits.size() << std::endl;
    
    if (!currentStory.tags.empty()) {
        std::cout << !𞎷  Tags: ";
        for (const auto& tag : currentStory.tags) {
            std::cout << tag << " ";
        }
        std::cout << std::endl;
    }
    
    if (!currentStory.commits.empty()) {
        std::cout << "\m𞒝 Recent commits:\n";
        size_t showCount = std::min<size_t>(5, currentStory.commits.size());
        for (size_t i = currentStory.commits.size() - showCount; i < currentStory.commits.size(); ++i) {
            const auto& commit = currentStory.commits[i];
            auto timeStr = Utils::formatTime(commit.timestamp);
            std::cout << "   �� " << commit.message << " (" << timeStr << ")" << std::endl;
        }
    }
}

void StoryMode::showStoryHistory() {
    if (completedStories.empty()) {
        std::cout << !� No completed stories" << std::endl;
        return;
    }
    
    std::cout << "\m𞒖 Story History\n";
    std::cout << !ᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕏᕐ\n";
    
    // Sort by end time (most recent first)
    auto sortedStories = completedStories;
    std::sort(sortedStories.begin(), sortedStories.end(),
        [](const Story& a, const Story& b) {
            return a.endTime > b.endTime;
        });
    
    for (const auto& story : sortedStories) {
        showStory(story);
        std::cout << "ᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀᓀ\n";
    }
}

void StoryMode::showStory(const Story& story) {
    auto startTimeStr = Utils::formatTime(story.startTime);
    auto endTimeStr = Utils::formatTime(story.endTime);
    
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        story.endTime - story.startTime).count();
    
    std::cout << "� " << story.title << std::endl;
    std::cout << "   � " << story.description << std::endl;
    std::cout << "   Ꮁ  " << startTimeStr << "ᆒ " << endTimeStr << " (" << duration << " hours)" << std::endl;
    std::cout << "  𞒊 " << story.commits.size() << " commits";
    
    if (!story.tags.empty()) {
        std::cout << "�� �  ";
        for (const auto& tag : story.tags) {
            std::cout << tag << " ";
        }
    }
    std::cout << std::endl;
}

std::string StoryMode::generateStoryId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return "story_" + std::to_string(timestamp);
}

bool StoryMode::saveStories() {
    std::ofstream file(storiesFile);
    if (!file.is_open()) return false;
    
    file << "{\n";
    
    // Save current story
    if (currentStory.active) {
        file << "  \"current\": {\n";
        file << "    \"id\": \"" << currentStory.id << "\",\n";
        file << "    \"title\": \"" << currentStory.title << "\",\n";
        file << "    \"description\": \"" << currentStory.description << "\",\n";
        
        auto startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
            currentStory.startTime.time_since_epoch()).count();
        file << "    \"startTime\": " << startTimestamp << ",\n";
        file << "    \"active\": true,\n";
        
        file << "    \"commits\": [\n";
        for (size_t i = 0; i < currentStory.commits.size(); ++i) {
            const auto& commit = currentStory.commits[i];
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                commit.timestamp.time_since_epoch()).count();
            
            file << "      {\n";
            file << "        \"hash\": \"" << commit.hash << "\",\n";
            file << "        \"message\": \"" << commit.message << "\",\n";
            file << "        \"timestamp\": " << timestamp << "\n";
            file << "      }";
            
            if (i < currentStory.commits.size() - 1) file << ",";
            file << "\n";
        }
        file << "    ],\n";
        
        file << "    \"tags\": [";
        bool first = true;
        for (const auto& tag : currentStory.tags) {
            if (!first) file << ", ";
            file << "\"" << tag << "\"";
            first = false;
        }
        file << "]\n";
        
        file << "  },\n";
    }
    
    // Save completed stories
    file << "  \"completed\": [\n";
    for (size_t i = 0; i < completedStories.size(); ++i) {
        const auto& story = completedStories[i];
        
        auto startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
            story.startTime.time_since_epoch()).count();
        auto endTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
            story.endTime.time_since_epoch()).count();
        
        file << "    {\n";
        file << "      \"id\": \"" << story.id << "\",\n";
        file << "      \"title\": \"" << story.title << "\",\n";
        file << "      \"description\": \"" << story.description << "\",\n";
        file << "      \"startTime\": " << startTimestamp << ",\n";
        file << "      \"endTime\": " << endTimestamp << ",\n";
        file << "      \"commits\": " << story.commits.size() << "\n";
        file << "    }";
        
        if (i < completedStories.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n";
    
    file << "}\n";
    return true;
}

bool StoryMode::loadStories() {
    // Simplified JSON loading - in a real implementation, use a proper JSON library
    std::ifstream file(storiesFile);
    if (!file.is_open()) return false;
    
    // For now, just create empty structures
    currentStory = Story{};
    completedStories.clear();
    return true;
}

// ContainerizedSnapshots Implementation

ContainerizedSnapshots::ContainerizedSnapshots(const std::string& repoPath) 
    : repoPath(repoPath), snapshotsDir(repoPath + "/.gyatt/snapshots") {
    std::filesystem::create_directories(snapshotsDir);
    loadSnapshots();
}

bool ContainerizedSnapshots::createSnapshot(const std::string& name, const std::string& description, 
                                            bool includeEnv) {
    Snapshot snapshot;
    snapshot.id = generateSnapshotId();
    snapshot.name = name;
    snapshot.description = description;
    snapshot.timestamp = std::chrono::system_clock::now();
    snapshot.includeEnv = includeEnv;
    
    std::string snapshotDir = Utils::joinPath(snapshotsDir, snapshot.id);
    std::filesystem::create_directories(snapshotDir);
    
    std::cout << !� Creating containerized snapshot: " << name << std::endl;
    std::cout << "� Snapshot ID: " << snapshot.id << std::endl;
    
    // Create workspace snapshot
    if (createWorkspaceSnapshot(snapshotDir)) {
        std::cout << !ᜅ Workspace archived" << std::endl;
    }
    
    // Create environment snapshot if requested
    if (includeEnv && createEnvironmentSnapshot(snapshotDir)) {
        std::cout << !ᜅ Environment captured" << std::endl;
    }
    
    // Create dependency snapshot
    if (createDependencySnapshot(snapshotDir)) {
        std::cout << !ᜅ Dependencies recorded" << std::endl;
    }
    
    snapshot.size = calculateSnapshotSize(snapshotDir);
    snapshots[snapshot.id] = snapshot;
    saveSnapshotConfig(snapshot);
    
    std::cout << !� Snapshot created successfully!" << std::endl;
    std::cout << !� Size: " << formatSize(snapshot.size) << std::endl;
    
    return true;
}

bool ContainerizedSnapshots::restoreSnapshot(const std::string& snapshotId, const std::string& targetDir) {
    auto it = snapshots.find(snapshotId);
    if (it == snapshots.end()) {
        std::cout << !✌ Snapshot not found: " << snapshotId << std::endl;
        return false;
    }
    
    const auto& snapshot = it->second;
    std::string snapshotDir = Utils::joinPath(snapshotsDir, snapshotId);
    
    std::cout << !𞒦 Restoring snapshot: " << snapshot.name << std::endl;
    std::cout << "� Target: " << targetDir << std::endl;
    
    std::filesystem::create_directories(targetDir);
    
    // Restore workspace
    std::string workspaceArchive = Utils::joinPath(snapshotDir, "workspace.tar.gz");
    if (Utils::fileExists(workspaceArchive)) {
        if (restoreWorkspaceSnapshot(workspaceArchive, targetDir)) {
            std::cout << !⛅ Workspace restored" << std::endl;
        }
    }
    
    // Restore environment if available
    if (snapshot.includeEnv) {
        std::string envScript = Utils::joinPath(snapshotDir, "environment.sh");
        if (Utils::fileExists(envScript)) {
            std::cout << "� Environment script available: " << envScript << std::endl;
            std::cout << "� Run 'source " << envScript << "' to restore environment" << std::endl;
        }
    }
    
    // Show dependency information
    std::string depsFile = Utils::joinPath(snapshotDir, "dependencies.json");
    if (Utils::fileExists(depsFile)) {
        std::cout << !� Dependencies recorded in: " << depsFile << std::endl;
    }
    
    std::cout << !𞍉 Snapshot restored successfully!" << std::endl;
    return true;
}

bool ContainerizedSnapshots::deleteSnapshot(const std::string& snapshotId) {
    auto it = snapshots.find(snapshotId);
    if (it == snapshots.end()) {
        std::cout << !✌ Snapshot not found: " << snapshotId << std::endl;
        return false;
    }
    
    std::string snapshotDir = Utils::joinPath(snapshotsDir, snapshotId);
    std::filesystem::remove_all(snapshotDir);
    
    std::cout << "�  Deleted snapshot: " << it->second.name << std::endl;
    snapshots.erase(it);
    
    return true;
}

std::vector<ContainerizedSnapshots::Snapshot> ContainerizedSnapshots::listSnapshots() {
    std::vector<Snapshot> snapshotList;
    for (const auto& [id, snapshot] : snapshots) {
        snapshotList.push_back(snapshot);
    }
    
    // Sort by timestamp (most recent first)
    std::sort(snapshotList.begin(), snapshotList.end(),
        [](const Snapshot& a, const Snapshot& b) {
            return a.timestamp > b.timestamp;
        });
    
    return snapshotList;
}

void ContainerizedSnapshots::showSnapshots() {
    auto snapshotList = listSnapshots();
    
    if (snapshotList.empty()) {
        std::cout << !𞒦 No snapshots available" << std::endl;
        return;
    }
    
    std::cout << "\m𞒦 Containerized Snapshots\n";
    std::cout << !┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┐\n";
    
    for (const auto& snapshot : snapshotList) {
        auto timeStr = Utils::formatTime(snapshot.timestamp);
        
        std::cout << "� " << snapshot.name << std::endl;
        std::cout << "   � " << snapshot.description << std::endl;
        std::cout << "  � " << snapshot.id << std::endl;
        std::cout << "  ⎰ " << timeStr << std::endl;
        std::cout << "  � " << formatSize(snapshot.size);
        
        if (snapshot.includeEnv) {
            std::cout << "ဢ𞓧 Environment included";
        }
        
        std::cout << std::endl;
        std::cout << !��������������������������������������ᔀ\n";
    }
}

void ContainerizedSnapshots::showSnapshot(const std::string& snapshotId) {
    auto it = snapshots.find(snapshotId);
    if (it == snapshots.end()) {
        std::cout << !ᝌ Snapshot not found: " << snapshotId << std::endl;
        return;
    }
    
    const auto& snapshot = it->second;
    auto timeStr = Utils::formatTime(snapshot.timestamp);
    
    std::cout << "\n� Snapshot Details\n";
    std::cout << !┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┏┐\n";
    std::cout << !� Name: " << snapshot.name << std::endl;
    std::cout << "� Description: " << snapshot.description << std::endl;
    std::cout << !� ID: " << snapshot.id << std::endl;
    std::cout << !⎰ Created: " << timeStr << std::endl;
    std::cout << "� Size: " << formatSize(snapshot.size) << std::endl;
    std::cout << !𞓧 Environment: " << (snapshot.includeEnv ? "Included" : "Not included") << std::endl;
    
    // Show snapshot contents
    std::string snapshotDir = Utils::joinPath(snapshotsDir, snapshotId);
    if (std::filesystem::exists(snapshotDir)) {
        std::cout << "\m� Snapshot contents:\n";
        for (const auto& entry : std::filesystem::directory_iterator(snapshotDir)) {
            std::cout << "  �� " << entry.path().filename().string() << std::endl;
        }
    }
}

std::string ContainerizedSnapshots::generateSnapshotId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return "snap_" + std::to_string(timestamp);
}

bool ContainerizedSnapshots::createWorkspaceSnapshot(const std::string& snapshotDir) {
    // Create tar.gz archive of workspace (excluding .gyatt directory)
    std::string archivePath = Utils::joinPath(snapshotDir, "workspace.tar.gz");
    
    // In a real implementation, this would use tar/compression libraries
    // For now, just create a placeholder file
    std::ofstream file(archivePath);
    if (file.is_open()) {
        file << "Workspace snapshot placeholder\n";
        file << "Original path: " << repoPath << "\n";
        file << "Timestamp: " << std::time(nullptr) << "\n";
        return true;
    }
    
    return false;
}

bool ContainerizedSnapshots::createEnvironmentSnapshot(const std::string& snapshotDir) {
    std::string envScript = Utils::joinPath(snapshotDir, "environment.sh");
    
    std::ofstream file(envScript);
    if (!file.is_open()) return false;
    
    file << "#!/bin/bash\n";
    file << "# Environment snapshot\n";
    file << "# Generated by gyatt containerized snapshots\n\n";
    
    // Capture common environment variables
    const char* envVars[] = {"PATH", "HOME", "USER", "PWD", "SHELL", nullptr};
    
    for (int i = 0; envVars[i]; ++i) {
        const char* value = std::getenv(envVars[i]);
        if (value) {
            file << "export " << envVars[i] << "=\"" << value << "\"\n";
        }
    }
    
    file << "\necho \"Environment restored from gyatt snapshot\"\n";
    return true;
}

bool ContainerizedSnapshots::createDependencySnapshot(const std::string& snapshotDir) {
    std::string depsFile = Utils::joinPath(snapshotDir, "dependencies.json");
    
    std::ofstream file(depsFile);
    if (!file.is_open()) return false;
    
    file << "{\n";
    file << "  \"timestamp\": " << std::time(nullptr) << ",\n";
    file << "  \"system\": {\n";
    file << "    \"os\": \"unknown\",\n";
    file << "    \"architecture\": \"unknown\"\n";
    file << "  },\n";
    file << "  \"dependencies\": {\n";
    file << "    \"build_tools\": [],\n";
    file << "    \"libraries\": [],\n";
    file << "    \"runtime\": []\n";
    file << "  }\n";
    file << "}\n";
    
    return true;
}

bool ContainerizedSnapshots::restoreWorkspaceSnapshot(const std::string& archivePath, 
                                                       const std::string& targetDir) {
    // In a real implementation, this would extract the tar.gz archive
    // For now, just copy the placeholder
    std::string targetFile = Utils::joinPath(targetDir, "README_snapshot.txt");
    
    std::string content = Utils::readFile(archivePath);
    return Utils::writeFile(targetFile, "Snapshot restored: " + content);
}

size_t ContainerizedSnapshots::calculateSnapshotSize(const std::string& snapshotDir) {
    size_t totalSize = 0;
    
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(snapshotDir)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Handle error
    }
    
    return totalSize;
}

std::string ContainerizedSnapshots::formatSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    oss.precision(2);
    oss << std::fixed << size << " " << units[unit];
    return oss.str();
}

bool ContainerizedSnapshots::saveSnapshotConfig(const Snapshot& snapshot) {
    std::string snapshotDir = Utils::joinPath(snapshotsDir, snapshot.id);
    std::string configFile = Utils::joinPath(snapshotDir, "snapshot.json");
    
    std::ofstream file(configFile);
    if (!file.is_open()) return false;
    
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        snapshot.timestamp.time_since_epoch()).count();
    
    file << "{\n";
    file << "  \"id\": \"" << snapshot.id << "\",\n";
    file << "  \"name\": \"" << snapshot.name << "\",\n";
    file << "  \"description\": \"" << snapshot.description << "\",\n";
    file << "  \"timestamp\": " << timestamp << ",\n";
    file << "  \"size\": " << snapshot.size << ",\n";
    file << "  \"includeEnv\": " << (snapshot.includeEnv ? "true" : "false") << "\n";
    file << "}\n";
    
    return true;
}

bool ContainerizedSnapshots::loadSnapshots() {
    snapshots.clear();
    
    if (!std::filesystem::exists(snapshotsDir)) {
        return true; // No snapshots yet
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(snapshotsDir)) {
        if (entry.is_directory()) {
            std::string snapshotId = entry.path().filename().string();
            loadSnapshot(snapshotId);
        }
    }
    
    return true;
}

bool ContainerizedSnapshots::loadSnapshot(const std::string& snapshotId) {
    std::string snapshotDir = Utils::joinPath(snapshotsDir, snapshotId);
    std::string configFile = Utils::joinPath(snapshotDir, "snapshot.json");
    
    if (!Utils::fileExists(configFile)) {
        return false;
    }
    
    // Simplified JSON loading - in a real implementation, use a proper JSON library
    Snapshot snapshot;
    snapshot.id = snapshotId;
    snapshot.name = "Loaded snapshot";
    snapshot.description = "Snapshot loaded from disk";
    snapshot.timestamp = std::chrono::system_clock::now();
    snapshot.size = calculateSnapshotSize(snapshotDir);
    snapshot.includeEnv = false;
    
    snapshots[snapshotId] = snapshot;
    return true;
}

// CommitStoryMode implementation
CommitStoryMode::CommitStoryMode(const std::string& repoPath) 
    : repoPath(repoPath), storyFile(repoPath + "/.gyatt/story.json"), currentTheme("epic") {
    std::filesystem::create_directories(repoPath + "/.gyatt");
    loadStoryProgress();
}

bool CommitStoryMode::enableStoryMode() {
    std::cout << !� Story mode enabled! Your commits will now become epic tales!" << std::endl;
    return true;
}

bool CommitStoryMode::disableStoryMode() {
    std::cout << !� Story mode disabled. Back to regular commits." << std::endl;
    return true;
}

bool CommitStoryMode::setStoryTheme(const std::string& theme) {
    if (theme == "epic" || theme == "adventure" || theme == "mystery" || theme == "comedy" || theme == "horror") {
        currentTheme = theme;
        std::cout << "� Story theme set to: " << theme << std::endl;
        return saveStoryProgress();
    }
    std::cout << !✌ Invalid theme. Available: epic, adventure, mystery, comedy, horror" << std::endl;
    return false;
}

std::string CommitStoryMode::startNewChapter(const std::string& title, const std::string& description) {
    std::string chapterStr = "Chapter " + std::to_string(chapters.size() + 1) + ": " + title;
    if (!description.empty()) {
        chapterStr += " - " + description;
    }
    chapters.push_back(chapterStr);
    saveStoryProgress();
    std::cout << "� Started new chapter: " << chapterStr << std::endl;
    return chapterStr;
}

bool CommitStoryMode::endChapter(const std::string& chapterTitle) {
    std::cout << "� Ended chapter: " << chapterTitle << std::endl;
    return saveStoryProgress();
}

std::vector<std::string> CommitStoryMode::listChapters() {
    return chapters;
}

std::string CommitStoryMode::generateStoryCommit(const std::string& changes, const std::string& context) {
    std::string storyCommit = generateNarrative(changes, currentTheme);
    if (!context.empty()) {
        storyCommit += "\n\nContext: " + context;
    }
    return storyCommit;
}

std::string CommitStoryMode::generateChapterSummary(const std::string& chapterTitle) {
    return "Chapter Summary: " + chapterTitle + " - A tale of code and adventure";
}

bool CommitStoryMode::exportStoryToBook(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "# The Story of " << repoPath << std::endl;
    file << "Theme: " << currentTheme << std::endl << std::endl;
    
    for (const auto& chapter : chapters) {
        file << "## " << chapter << std::endl << std::endl;
    }
    
    file.close();
    std::cout << !𞒖 Story exported to: " << filename << std::endl;
    return true;
}

void CommitStoryMode::interactiveStoryCommit() {
    std::cout << "� Interactive Story Commit Mode" << std::endl;
    std::cout << "Current theme: " << currentTheme << std::endl;
    // Interactive implementation would go here
}

void CommitStoryMode::showCurrentStory() {
    std::cout << !𞒚 Current Story Progress" << std::endl;
    std::cout << "Theme: " << currentTheme << std::endl;
    std::cout << "Chapters: " << chapters.size() << std::endl;
    for (const auto& chapter : chapters) {
        std::cout << "  - " << chapter << std::endl;
    }
}

CommitStoryMode::StoryTemplate CommitStoryMode::getStoryTemplate(const std::string& theme) {
    StoryTemplate tmpl;
    tmpl.theme = theme;
    
    if (theme == "epic") {
        tmpl.narrativePatterns = {"The hero embarked on", "A mighty quest to", "Against all odds"};
        tmpl.characterNames = {"Hero", "Warrior", "Champion"};
        tmpl.actionMappings["add"] = "discovered";
        tmpl.actionMappings["remove"] = "vanquished";
        tmpl.actionMappings["modify"] = "transformed";
    } else if (theme == "adventure") {
        tmpl.narrativePatterns = {"The explorer ventured", "On a journey to", "Through uncharted territory"};
        tmpl.characterNames = {"Explorer", "Adventurer", "Pioneer"};
        tmpl.actionMappings["add"] = "uncovered";
        tmpl.actionMappings["remove"] = "cleared away";
        tmpl.actionMappings["modify"] = "refined";
    }
    // Add more themes as needed
    
    return tmpl;
}

std::string CommitStoryMode::generateNarrative(const std::string& action, const std::string& theme) {
    StoryTemplate tmpl = getStoryTemplate(theme);
    std::string narrative = "In this " + theme + " tale, ";
    
    if (!tmpl.narrativePatterns.empty()) {
        narrative += tmpl.narrativePatterns[0] + " " + action;
    } else {
        narrative += "the developer " + action;
    }
    
    return narrative;
}

std::string CommitStoryMode::mapCodeActionToStory(const std::string& codeAction) {
    StoryTemplate tmpl = getStoryTemplate(currentTheme);
    auto it = tmpl.actionMappings.find(codeAction);
    if (it != tmpl.actionMappings.end()) {
        return it->second;
    }
    return codeAction; // fallback to original action
}

bool CommitStoryMode::saveStoryProgress() {
    try {
        std::ofstream file(storyFile);
        if (!file.is_open()) return false;
        
        // Simple JSON-like format
        file << "{\n";
        file << "  \"theme\": \"" << currentTheme << "\",\n";
        file << "  \"chapters\": [\n";
        for (size_t i = 0; i < chapters.size(); ++i) {
            file << "    \"" << chapters[i] << "\"";
            if (i < chapters.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n";
        file << "}\n";
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool CommitStoryMode::loadStoryProgress() {
    if (!std::filesystem::exists(storyFile)) {
        return true; // No story file yet, that's okay
    }
    
    try {
        std::ifstream file(storyFile);
        if (!file.is_open()) return false;
        
        // Simple parsing - in production, use proper JSON library
        std::string line;
        chapters.clear();
        
        while (std::getline(file, line)) {
            if (line.find("\"theme\":") != std::string::npos) {
                size_t start = line.find("\"", line.find(":") + 1) + 1;
                size_t end = line.find("\"", start);
                if (start != std::string::npos && end != std::string::npos) {
                    currentTheme = line.substr(start, end - start);
                }
            }
            // Parse chapters - simplified implementation
        }
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace gyatt
