#include "../include/plugin_system.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <json/json.h>
#include <chrono>

namespace gyatt {

PluginManager::PluginManager(const std::string& repoPath) 
    : repoPath(repoPath), pluginsDir(repoPath + "/.gyatt/plugins") {
    std::filesystem::create_directories(pluginsDir);
    loadPluginConfig();
}

bool PluginManager::installPlugin(const std::string& pluginName, const std::string& source) {
    std::cout << "📦 Installing plugin: " << pluginName << std::endl;
    
    if (source.empty()) {
        // Install from built-in registry
        return downloadPlugin(pluginName, getPluginRegistry());
    } else {
        // Install from custom source
        return downloadPlugin(pluginName, source);
    }
}

bool PluginManager::uninstallPlugin(const std::string& pluginName) {
    std::string pluginPath = pluginsDir + "/" + pluginName;
    if (std::filesystem::exists(pluginPath)) {
        std::filesystem::remove_all(pluginPath);
        
        // Remove from config
        auto it = std::find_if(installedPlugins.begin(), installedPlugins.end(),
                              [&](const Plugin& p) { return p.name == pluginName; });
        if (it != installedPlugins.end()) {
            installedPlugins.erase(it);
            savePluginConfig();
        }
        
        std::cout << "✅ Plugin uninstalled: " << pluginName << std::endl;
        return true;
    }
    return false;
}

bool PluginManager::enablePlugin(const std::string& pluginName) {
    auto it = std::find_if(installedPlugins.begin(), installedPlugins.end(),
                          [&](Plugin& p) { return p.name == pluginName; });
    if (it != installedPlugins.end()) {
        it->enabled = true;
        savePluginConfig();
        std::cout << "✅ Plugin enabled: " << pluginName << std::endl;
        return true;
    }
    return false;
}

bool PluginManager::disablePlugin(const std::string& pluginName) {
    auto it = std::find_if(installedPlugins.begin(), installedPlugins.end(),
                          [&](Plugin& p) { return p.name == pluginName; });
    if (it != installedPlugins.end()) {
        it->enabled = false;
        savePluginConfig();
        std::cout << "⚠️  Plugin disabled: " << pluginName << std::endl;
        return true;
    }
    return false;
}

std::vector<PluginManager::Plugin> PluginManager::listPlugins() {
    return installedPlugins;
}

bool PluginManager::executePlugin(const std::string& pluginName, const std::vector<std::string>& args) {
    auto it = std::find_if(installedPlugins.begin(), installedPlugins.end(),
                          [&](const Plugin& p) { return p.name == pluginName && p.enabled; });
    
    if (it == installedPlugins.end()) {
        std::cerr << "Plugin not found or disabled: " << pluginName << std::endl;
        return false;
    }
    
    std::string command = it->executable;
    for (const auto& arg : args) {
        command += " " + arg;
    }
    
    int result = std::system(command.c_str());
    return result == 0;
}

std::string PluginManager::getPluginOutput(const std::string& pluginName, const std::vector<std::string>& args) {
    // Simple implementation - could be improved with proper process management
    std::string command = pluginName;
    for (const auto& arg : args) {
        command += " " + arg;
    }
    command += " 2>&1";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
}

bool PluginManager::installChangelogGenerator() {
    std::cout << "📝 Installing built-in changelog generator..." << std::endl;
    
    std::string pluginDir = pluginsDir + "/changelog-gen";
    std::filesystem::create_directories(pluginDir);
    
    // Create simple Python script for changelog generation
    std::string scriptContent = R"GYATT_SCRIPT(#!/usr/bin/env python3
import os
import sys
import subprocess
import json
from datetime import datetime

def generate_changelog():
    """Generate changelog from git commits"""
    try:
        # Get commit history
        result = subprocess.run(['git', 'log', '--oneline', '--decorate'], 
                              capture_output=True, text=True)
        
        if result.returncode != 0:
            print("Error: Not in a git repository")
            return False
            
        commits = result.stdout.strip().split('\n')
        
        # Create changelog
        changelog = "# Changelog\n\n"
        changelog += f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        for commit in commits:
            if commit.strip():
                changelog += f"- {commit}\n"
        
        # Write to file
        with open('CHANGELOG.md', 'w') as f:
            f.write(changelog)
            
        print("✅ Changelog generated successfully")
        return True
        
    except Exception as e:
        print(f"❌ Error generating changelog: {e}")
        return False

if __name__ == "__main__":
    generate_changelog()
)GYATT_SCRIPT";
    
    std::ofstream scriptFile(pluginDir + "/changelog-gen.py");
    scriptFile << scriptContent;
    scriptFile.close();
    
    // Make script executable
    std::filesystem::permissions(pluginDir + "/changelog-gen.py", 
                               std::filesystem::perms::owner_exec, 
                               std::filesystem::perm_options::add);
    
    // Create plugin metadata
    Plugin plugin;
    plugin.name = "changelog-gen";
    plugin.version = "1.0.0";
    plugin.description = "Generate changelog from commit history";
    plugin.language = PluginLanguage::PYTHON;
    plugin.executable = pluginDir + "/changelog-gen.py";
    plugin.commands = {"generate"};
    plugin.enabled = true;
    
    installedPlugins.push_back(plugin);
    savePluginConfig();
    
    return true;
}

bool PluginManager::installUndoCommits() {
    std::cout << "↩️  Installing built-in undo commits plugin..." << std::endl;
    
    std::string pluginDir = pluginsDir + "/undo-commits";
    std::filesystem::create_directories(pluginDir);
    
    // Create simple bash script for undoing commits
    std::string scriptContent = R"GYATT_SCRIPT(#!/bin/bash
# Undo commits plugin for gyatt

undo_commits() {
    local count=${1:-1}
    
    echo "⚠️  About to undo $count commit(s)"
    echo "This will reset HEAD~$count and stage the changes"
    
    read -p "Continue? (y/N): " confirm
    if [[ $confirm =~ ^[Yy]$ ]]; then
        git reset --soft HEAD~$count
        echo "✅ Successfully undone $count commit(s)"
        echo "💡 Changes are now staged, ready to recommit"
    else
        echo "❌ Operation cancelled"
        return 1
    fi
}

case "$1" in
    "undo")
        undo_commits ${2:-1}
        ;;
    *)
        echo "Usage: $0 undo [number_of_commits]"
        exit 1
        ;;
esac
)GYATT_SCRIPT";
    
    std::ofstream scriptFile(pluginDir + "/undo.sh");
    scriptFile << scriptContent;
    scriptFile.close();
    
    // Make script executable
    std::filesystem::permissions(pluginDir + "/undo.sh", 
                               std::filesystem::perms::owner_exec, 
                               std::filesystem::perm_options::add);
    
    Plugin plugin;
    plugin.name = "undo-commits";
    plugin.version = "1.0.0";
    plugin.description = "Safely undo commits while preserving changes";
    plugin.language = PluginLanguage::BASH;
    plugin.executable = pluginDir + "/undo.sh";
    plugin.commands = {"undo"};
    plugin.enabled = true;
    
    installedPlugins.push_back(plugin);
    savePluginConfig();
    
    return true;
}

bool PluginManager::installCodeFormatter() {
    std::cout << "🎨 Installing built-in code formatter..." << std::endl;
    
    std::string pluginDir = pluginsDir + "/code-formatter";
    std::filesystem::create_directories(pluginDir);
    
    // Create simple Python script for code formatting
    std::string scriptContent = R"GYATT_SCRIPT(#!/usr/bin/env python3
import os
import sys
import subprocess

def format_code(filepath):
    """Format code based on file extension"""
    try:
        ext = os.path.splitext(filepath)[1].lower()
        
        if ext == '.py':
            subprocess.run(['black', filepath], check=True)
            print(f"✅ Formatted Python file: {filepath}")
        elif ext in ['.js', '.ts', '.json']:
            subprocess.run(['prettier', '--write', filepath], check=True)
            print(f"✅ Formatted JS/TS file: {filepath}")
        elif ext in ['.cpp', '.h', '.hpp', '.cc']:
            subprocess.run(['clang-format', '-i', filepath], check=True)
            print(f"✅ Formatted C++ file: {filepath}")
        else:
            print(f"⚠️  No formatter available for: {filepath}")
            
    except subprocess.CalledProcessError:
        print(f"❌ Failed to format: {filepath}")
    except FileNotFoundError:
        print(f"❌ Formatter not installed for: {filepath}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: code-formatter.py <file>")
        sys.exit(1)
        
    format_code(sys.argv[1])
)GYATT_SCRIPT";
    
    std::ofstream scriptFile(pluginDir + "/formatter.py");
    scriptFile << scriptContent;
    scriptFile.close();
    
    // Make script executable
    std::filesystem::permissions(pluginDir + "/formatter.py", 
                               std::filesystem::perms::owner_exec, 
                               std::filesystem::perm_options::add);
    
    Plugin plugin;
    plugin.name = "code-formatter";
    plugin.version = "1.0.0";
    plugin.description = "Format code files with appropriate tools";
    plugin.language = PluginLanguage::PYTHON;
    plugin.executable = pluginDir + "/formatter.py";
    plugin.commands = {"format"};
    plugin.enabled = true;
    
    installedPlugins.push_back(plugin);
    savePluginConfig();
    
    return true;
}

bool PluginManager::installTestRunner() {
    std::cout << "🧪 Installing built-in test runner..." << std::endl;
    
    std::string pluginDir = pluginsDir + "/test-runner";
    std::filesystem::create_directories(pluginDir);
    
    // Create simple test runner script
    std::string scriptContent = R"GYATT_SCRIPT(#!/usr/bin/env node
// Test runner plugin for gyatt
const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');

function runTests() {
    const cwd = process.cwd();
    
    // Check for different test frameworks
    if (fs.existsSync(path.join(cwd, 'package.json'))) {
        const pkg = JSON.parse(fs.readFileSync(path.join(cwd, 'package.json')));
        
        if (pkg.scripts && pkg.scripts.test) {
            exec('npm test', (error, stdout, stderr) => {
                if (error) {
                    return;
                }
            });
            return;
        }
    }
    
    // Check for Python tests
    if (fs.existsSync(path.join(cwd, 'requirements.txt')) || 
        fs.existsSync(path.join(cwd, 'setup.py'))) {
        exec('python -m pytest', (error, stdout, stderr) => {
            if (error) {
                return;
            }
        });
        return;
    }
    
    // Check for C++ tests
    if (fs.existsSync(path.join(cwd, 'Makefile')) || 
        fs.existsSync(path.join(cwd, 'CMakeLists.txt'))) {
        exec('make test', (error, stdout, stderr) => {
            if (error) {
                return;
            }
        });
        return;
    }
    
}

runTests();
)GYATT_SCRIPT";
    
    std::ofstream scriptFile(pluginDir + "/test-runner.js");
    scriptFile << scriptContent;
    scriptFile.close();
    
    // Make script executable
    std::filesystem::permissions(pluginDir + "/test-runner.js", 
                               std::filesystem::perms::owner_exec, 
                               std::filesystem::perm_options::add);
    
    Plugin plugin;
    plugin.name = "test-runner";
    plugin.version = "1.0.0";
    plugin.description = "Run tests for various project types";
    plugin.language = PluginLanguage::JAVASCRIPT;
    plugin.executable = pluginDir + "/test-runner.js";
    plugin.commands = {"test", "run"};
    plugin.enabled = true;
    
    installedPlugins.push_back(plugin);
    savePluginConfig();
    
    return true;
}

bool PluginManager::createPluginTemplate(const std::string& pluginName, PluginLanguage language) {
    std::string pluginDir = pluginsDir + "/" + pluginName;
    std::filesystem::create_directories(pluginDir);
    
    std::string extension, shebang;
    switch (language) {
        case PluginLanguage::PYTHON:
            extension = ".py";
            shebang = "#!/usr/bin/env python3";
            break;
        case PluginLanguage::BASH:
            extension = ".sh";
            shebang = "#!/bin/bash";
            break;
        case PluginLanguage::JAVASCRIPT:
            extension = ".js";
            shebang = "#!/usr/bin/env node";
            break;
        default:
            return false;
    }
    
    std::string scriptContent = shebang + "\n# " + pluginName + " plugin\n\necho \"Hello from " + pluginName + "\"\n";
    
    std::ofstream scriptFile(pluginDir + "/" + pluginName + extension);
    scriptFile << scriptContent;
    scriptFile.close();
    
    return true;
}

bool PluginManager::downloadPlugin(const std::string& pluginName, const std::string& source) {
    // Simplified implementation - in practice would download from registry
    std::cout << "📦 Downloading " << pluginName << " from " << source << std::endl;
    return true;
}

std::string PluginManager::getPluginRegistry() {
    return "https://gyatt-plugins.dev/registry";
}

PluginManager::Plugin PluginManager::parsePluginManifest(const std::string& manifestPath) {
    Plugin plugin;
    // Simplified - would parse JSON manifest
    return plugin;
}

bool PluginManager::validatePlugin(const Plugin& plugin) {
    if (plugin.name.empty() || plugin.version.empty()) {
        return false;
    }
    
    if (!std::filesystem::exists(plugin.executable)) {
        return false;
    }
    
    return true;
}

void PluginManager::loadPluginConfig() {
    std::string configFile = repoPath + "/.gyatt/plugins.json";
    if (!std::filesystem::exists(configFile)) {
        return;
    }
    
    // Simplified - would parse JSON config file
    // For now just create empty list
}

void PluginManager::savePluginConfig() {
    std::string configFile = repoPath + "/.gyatt/plugins.json";
    
    // Simplified - would save JSON config
    std::ofstream file(configFile);
    file << "{\n  \"plugins\": [\n";
    
    for (size_t i = 0; i < installedPlugins.size(); ++i) {
        const auto& plugin = installedPlugins[i];
        file << "    {\n";
        file << "      \"name\": \"" << plugin.name << "\",\n";
        file << "      \"version\": \"" << plugin.version << "\",\n";
        file << "      \"enabled\": " << (plugin.enabled ? "true" : "false") << "\n";
        file << "    }";
        if (i < installedPlugins.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n}\n";
    file.close();
}

PluginManager::Plugin PluginManager::parsePluginFromJson(const std::string& json) {
    Plugin plugin;
    // Simplified - would parse JSON string
    return plugin;
}

// SessionRecorder implementations
SessionRecorder::SessionRecorder(const std::string& repoPath) 
    : repoPath(repoPath), sessionsDir(repoPath + "/.gyatt/sessions"), isRecording(false) {
    std::filesystem::create_directories(sessionsDir);
}

bool SessionRecorder::startRecording(const std::string& sessionName) {
    if (isRecording) {
        std::cout << "⚠️  Already recording session: " << currentSession << std::endl;
        return false;
    }
    
    currentSession = sessionName;
    currentEntries.clear();
    isRecording = true;
    
    std::cout << "🎥 Started recording session: " << sessionName << std::endl;
    return true;
}

void SessionRecorder::recordCommand(const std::string& command, const std::string& output) {
    if (!isRecording) return;
    
    SessionEntry entry;
    entry.command = command;
    entry.output = output;
    entry.timestamp = std::chrono::system_clock::now();
    entry.workingDir = std::filesystem::current_path();
    
    currentEntries.push_back(entry);
}

bool SessionRecorder::stopRecording() {
    if (!isRecording) {
        std::cout << "⚠️  No active recording session" << std::endl;
        return false;
    }
    
    isRecording = false;
    bool success = saveSession(currentSession);
    
    if (success) {
        std::cout << "✅ Session saved: " << currentSession << std::endl;
    } else {
        std::cout << "❌ Failed to save session" << std::endl;
    }
    
    return success;
}

bool SessionRecorder::playSession(const std::string& sessionName) {
    auto entries = loadSession(sessionName);
    if (entries.empty()) {
        std::cout << "❌ Session not found: " << sessionName << std::endl;
        return false;
    }
    
    std::cout << "▶️  Playing session: " << sessionName << std::endl;
    
    for (const auto& entry : entries) {
        auto timePoint = std::chrono::system_clock::to_time_t(entry.timestamp);
        std::cout << "[" << std::ctime(&timePoint) << "] " << entry.command << std::endl;
        std::cout << entry.output << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return true;
}

std::vector<std::string> SessionRecorder::listSessions() {
    std::vector<std::string> sessions;
    
    for (const auto& entry : std::filesystem::directory_iterator(sessionsDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            sessions.push_back(entry.path().stem().string());
        }
    }
    
    return sessions;
}

bool SessionRecorder::saveSession(const std::string& sessionName) {
    std::string sessionFile = sessionsDir + "/" + sessionName + ".json";
    
    std::ofstream file(sessionFile);
    if (!file) return false;
    
    file << "{\n";
    file << "  \"name\": \"" << sessionName << "\",\n";
    file << "  \"entries\": [\n";
    
    for (size_t i = 0; i < currentEntries.size(); ++i) {
        const auto& entry = currentEntries[i];
        file << "    {\n";
        file << "      \"command\": \"" << entry.command << "\",\n";
        file << "      \"output\": \"" << entry.output << "\",\n";
        file << "      \"workingDir\": \"" << entry.workingDir << "\"\n";
        file << "    }";
        if (i < currentEntries.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    return true;
}

std::vector<SessionRecorder::SessionEntry> SessionRecorder::loadSession(const std::string& sessionName) {
    std::vector<SessionEntry> entries;
    std::string sessionFile = sessionsDir + "/" + sessionName + ".json";
    
    if (!std::filesystem::exists(sessionFile)) {
        return entries;
    }
    
    // Simplified - would parse JSON file
    // For now return empty vector
    return entries;
}

void SessionRecorder::recordFileDiff(const std::vector<std::string>& files) {
    // Implementation for recording file diffs
    (void)files; // Suppress unused parameter warning
}

} // namespace gyatt
