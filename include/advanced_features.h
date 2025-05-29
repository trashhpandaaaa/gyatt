#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <unordered_map>

namespace gyatt {

class InitTemplates {
public:
    enum class ProjectType {
        REACT,
        VUE,
        ANGULAR,
        NODE_EXPRESS,
        PYTHON_FLASK,
        PYTHON_DJANGO,
        RUST_ACTIX,
        CPP_CMAKE,
        JAVA_MAVEN,
        GO_MODULE,
        UNITY_GAME,
        ELECTRON,
        MOBILE_REACT_NATIVE,
        MACHINE_LEARNING,
        DATA_SCIENCE,
        BASIC,
        CUSTOM
    };

    struct TemplateFile {
        std::string path;
        std::string content;
        bool isDirectory;
        std::string relativePath;
        std::string description;
    };

    struct Template {
        std::string name;
        std::string description;
        std::vector<TemplateFile> files;
        std::map<std::string, std::string> variables;
        std::chrono::system_clock::time_point timestamp;
    };

    struct TemplateConfig {
        std::string name;
        std::string description;
        std::vector<std::string> files;
        std::vector<std::string> directories;
        std::map<std::string, std::string> variables;
        std::vector<std::string> setupCommands;
        std::string gitignoreTemplate;
    };

    InitTemplates(const std::string& repoPath);
    
    // Template creation and management
    bool createTemplate(const std::string& name, const std::string& description,
                       const std::vector<TemplateFile>& files);
    bool useTemplate(const std::string& name, const std::string& targetDir,
                    const std::map<std::string, std::string>& variables);
    bool deleteTemplate(const std::string& name);
    std::vector<Template> listTemplates();
    void showTemplates();
    void showTemplate(const std::string& name);
    
    // Template initialization
    bool initWithTemplate(ProjectType type, const std::string& projectName = "");
    bool initReact(const std::string& projectName);
    bool initPythonFlask(const std::string& projectName);
    bool initCppCMake(const std::string& projectName);
    bool initCustomTemplate(const std::string& templateName, const std::string& projectName);
    
    // Template management
    bool createCustomTemplate(const std::string& templateName, const std::string& sourceDir);
    std::vector<std::string> listAvailableTemplates();
    bool downloadTemplate(const std::string& templateUrl, const std::string& templateName);
    
    TemplateConfig getTemplateConfig(ProjectType type);
    bool saveTemplateConfig(const std::string& templateName, const TemplateConfig& config);
    
private:
    std::string repoPath;
    std::string templatesDir;
    std::unordered_map<std::string, Template> templates;
    
    bool loadTemplates();
    bool loadTemplate(const std::string& name);
    void scanTemplateFiles(const std::string& templateDir, std::vector<TemplateFile>& files);
    bool saveTemplateConfig(const Template& tmpl);
    
    bool createDirectoryStructure(const std::vector<std::string>& directories);
    bool createTemplateFiles(const std::vector<std::string>& files, const std::map<std::string, std::string>& variables);
    bool runSetupCommands(const std::vector<std::string>& commands);
    std::string processTemplate(const std::string& templateContent, const std::map<std::string, std::string>& variables);
    
    // Built-in templates
    TemplateConfig getReactTemplate();
    TemplateConfig getPythonFlaskTemplate();
    TemplateConfig getCppCMakeTemplate();
    TemplateConfig getNodeExpressTemplate();
};

class StoryMode {
public:
    struct StoryCommit {
        std::string hash;
        std::string message;
        std::string storyText;
        std::chrono::system_clock::time_point timestamp;
    };

    struct Story {
        std::string id;
        std::string title;
        std::string description;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        std::vector<StoryCommit> commits;
        std::set<std::string> tags;
        bool active = false;
    };

    StoryMode(const std::string& repoPath);

    // Story management
    bool startStory(const std::string& title, const std::string& description);
    bool endStory();
    bool addCommitToStory(const std::string& commitHash, const std::string& message);
    bool addTagToStory(const std::string& tag);

    // Story display
    Story getCurrentStory();
    std::vector<Story> getCompletedStories();
    void showCurrentStory();
    void showStoryHistory();
    void showStory(const Story& story);

private:
    std::string repoPath;
    std::string storiesFile;
    Story currentStory;
    std::vector<Story> completedStories;

    std::string generateStoryId();
    bool saveStories();
    bool loadStories();
};

class CommitStoryMode {
public:
    CommitStoryMode(const std::string& repoPath);
    
    // Story mode configuration
    bool enableStoryMode();
    bool disableStoryMode();
    bool setStoryTheme(const std::string& theme); // "epic", "adventure", "mystery", "comedy", "horror"
    
    // Chapter management
    std::string startNewChapter(const std::string& title, const std::string& description = "");
    bool endChapter(const std::string& chapterTitle);
    std::vector<std::string> listChapters();
    
    // Story generation
    std::string generateStoryCommit(const std::string& changes, const std::string& context = "");
    std::string generateChapterSummary(const std::string& chapterTitle);
    bool exportStoryToBook(const std::string& filename);
    
    // Interactive story building
    void interactiveStoryCommit();
    void showCurrentStory();
    
    // Story templates
    struct StoryTemplate {
        std::string theme;
        std::vector<std::string> narrativePatterns;
        std::vector<std::string> characterNames;
        std::map<std::string, std::string> actionMappings; // code action -> story action
    };
    
    StoryTemplate getStoryTemplate(const std::string& theme);
    
private:
    std::string repoPath;
    std::string storyFile;
    std::string currentTheme;
    std::vector<std::string> chapters;
    
    std::string generateNarrative(const std::string& action, const std::string& theme);
    std::string mapCodeActionToStory(const std::string& codeAction);
    bool saveStoryProgress();
    bool loadStoryProgress();
};

class ContainerizedSnapshots {
public:
    struct Snapshot {
        std::string id;
        std::string name;
        std::string description;
        std::string commitHash;
        std::chrono::system_clock::time_point created;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, std::string> runtimeInfo;
        std::vector<std::string> dependencies;
        std::string dockerfilePath;
        std::string setupScriptPath;
        size_t size = 0;
        bool includeEnv = false;
    };

    ContainerizedSnapshots(const std::string& repoPath);
    
    // Snapshot creation and management
    bool createSnapshot(const std::string& name, const std::string& description,
                       bool includeRuntime = false);
    bool restoreSnapshot(const std::string& snapshotId, const std::string& targetDir);
    bool deleteSnapshot(const std::string& snapshotId);
    std::vector<Snapshot> listSnapshots();
    void showSnapshots();
    void showSnapshot(const std::string& snapshotId);

    // Snapshot creation
    bool createSnapshot(const std::string& commitHash, bool includeRuntime = false);
    bool createSnapshotWithDeps(const std::string& commitHash, const std::vector<std::string>& dependencies);
    
    // Runtime capture
    bool captureRuntimeInfo(const std::string& snapshotId);
    bool captureEnvironmentVars(const std::string& snapshotId);
    bool captureSystemConfigs(const std::string& snapshotId);
    
    // Snapshot management
    bool restoreFromSnapshot(const std::string& snapshotId);
    
    // Container operations
    bool buildContainer(const std::string& snapshotId);
    bool runContainer(const std::string& snapshotId, const std::vector<std::string>& commands = {});
    bool exportContainer(const std::string& snapshotId, const std::string& filename);
    
    // Setup script generation
    bool generateSetupScript(const std::string& snapshotId, const std::string& scriptType = "bash");
    bool testSnapshot(const std::string& snapshotId);
    
private:
    std::string repoPath;
    std::string snapshotsDir;
    std::unordered_map<std::string, Snapshot> snapshots;
    
    struct SnapshotInfo {
        std::string id;
        std::string commitHash;
        std::chrono::system_clock::time_point created;
        std::map<std::string, std::string> runtimeInfo;
        std::vector<std::string> dependencies;
        std::string dockerfilePath;
        std::string setupScriptPath;
    };

    // Private helper methods
    std::string generateSnapshotId();
    bool createWorkspaceSnapshot(const std::string& snapshotDir);
    bool createEnvironmentSnapshot(const std::string& snapshotDir);
    bool createDependencySnapshot(const std::string& snapshotDir);
    bool restoreWorkspaceSnapshot(const std::string& archivePath, const std::string& targetDir);
    size_t calculateSnapshotSize(const std::string& snapshotDir);
    std::string formatSize(size_t bytes);
    bool saveSnapshotConfig(const Snapshot& snapshot);
    bool loadSnapshots();
    bool loadSnapshot(const std::string& snapshotId);
    
    bool saveSnapshotInfo(const SnapshotInfo& info);
    SnapshotInfo loadSnapshotInfo(const std::string& snapshotId);
    std::string generateDockerfile(const SnapshotInfo& info);
    std::string generateSetupScript(const SnapshotInfo& info, const std::string& scriptType);
};

} // namespace gyatt
