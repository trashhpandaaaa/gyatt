#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

namespace gyatt {

class PluginManager {
public:
    enum class PluginLanguage {
        PYTHON,
        BASH,
        RUST,
        JAVASCRIPT,
        NATIVE_CPP
    };

    struct Plugin {
        std::string name;
        std::string version;
        std::string description;
        PluginLanguage language;
        std::string executable;
        std::vector<std::string> commands;
        std::map<std::string, std::string> config;
        bool enabled;
    };

    PluginManager(const std::string& repoPath);
    
    // Plugin installation and management
    bool installPlugin(const std::string& pluginName, const std::string& source = "");
    bool uninstallPlugin(const std::string& pluginName);
    bool enablePlugin(const std::string& pluginName);
    bool disablePlugin(const std::string& pluginName);
    std::vector<Plugin> listPlugins();
    
    // Execute plugin commands
    bool executePlugin(const std::string& pluginName, const std::vector<std::string>& args);
    std::string getPluginOutput(const std::string& pluginName, const std::vector<std::string>& args);
    
    // Built-in plugins
    bool installChangelogGenerator();
    bool installUndoCommits();
    bool installCodeFormatter();
    bool installTestRunner();
    
    // Plugin development
    bool createPluginTemplate(const std::string& pluginName, PluginLanguage language);
    bool registerLocalPlugin(const std::string& pluginPath);
    
    // Plugin discovery
    std::vector<std::string> discoverAvailablePlugins();
    Plugin getPluginInfo(const std::string& pluginName);
    
private:
    std::string repoPath;
    std::string pluginsDir;
    std::vector<Plugin> installedPlugins;
    
    bool downloadPlugin(const std::string& pluginName, const std::string& source);
    bool validatePlugin(const Plugin& plugin);
    std::string getPluginRegistry();
    void savePluginConfig();
    void loadPluginConfig();
    std::string generatePluginManifest(const Plugin& plugin);
    Plugin parsePluginManifest(const std::string& manifestPath);
    Plugin parsePluginFromJson(const std::string& json);
};

class SessionRecorder {
public:
    SessionRecorder(const std::string& repoPath);
    
    // Recording control
    bool startRecording(const std::string& sessionName = "");
    bool stopRecording();
    bool pauseRecording();
    bool resumeRecording();
    
    // Session management
    std::vector<std::string> listSessions();
    bool deleteSession(const std::string& sessionName);
    bool exportSession(const std::string& sessionName, const std::string& format = "asciinema");
    
    // Playback
    bool playSession(const std::string& sessionName);
    bool replayCommands(const std::string& sessionName);
    
    // Analysis
    struct SessionStats {
        std::string sessionName;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        size_t commandCount;
        std::vector<std::string> filesModified;
        std::map<std::string, size_t> commandFrequency;
    };
    
    SessionStats analyzeSession(const std::string& sessionName);
    
private:
    std::string repoPath;
    std::string sessionsDir;
    std::string currentSession;
    bool isRecording;
    
    struct SessionEntry {
        std::chrono::system_clock::time_point timestamp;
        std::string command;
        std::string output;
        std::string workingDir;
        std::vector<std::string> filesDiff;
    };
    
    std::vector<SessionEntry> currentEntries;
    
    void recordCommand(const std::string& command, const std::string& output);
    void recordFileDiff(const std::vector<std::string>& files);
    bool saveSession(const std::string& sessionName);
    std::vector<SessionEntry> loadSession(const std::string& sessionName);
};

} // namespace gyatt
