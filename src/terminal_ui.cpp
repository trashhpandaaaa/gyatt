#include "../include/terminal_ui.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <json/json.h>

namespace gyatt {

TerminalUI::TerminalUI() : currentColorScheme("neobrutalist"), spinnerActive(false) {
    initColorSchemes();
}

std::string TerminalUI::colorize(const std::string& text, Color fg, Color bg, Style style) {
    std::string result = "\033[";
    
    // Add style
    result += getStyleCode(style);
    result += ";";
    
    // Add foreground color
    result += getColorCode(fg, false);
    
    if (bg != Color::BLACK) {
        result += ";";
        result += getColorCode(bg, true);
    }
    
    result += "m" + text + "\033[0m";
    return result;
}

void TerminalUI::setColorScheme(const std::string& scheme) {
    currentColorScheme = scheme;
}

void TerminalUI::showProgressBar(const std::string& label, int progress, int total) {
    int barWidth = 50;
    float percentage = static_cast<float>(progress) / total;
    int filled = static_cast<int>(barWidth * percentage);
    
    std::cout << "\r" << colorize(label, Color::CYAN, Color::BLACK, Style::BOLD) << " [";
    
    for (int i = 0; i < barWidth; i++) {
        if (i < filled) {
            std::cout << colorize("█", Color::GREEN);
        } else {
            std::cout << colorize("░", Color::WHITE);
        }
    }
    
    std::cout << "] " << colorize(std::to_string(static_cast<int>(percentage * 100)) + "%", 
                                Color::YELLOW, Color::BLACK, Style::BOLD);
    std::cout.flush();
    
    if (progress == total) {
        std::cout << std::endl;
    }
}

void TerminalUI::showSpinner(const std::string& message) {
    if (spinnerActive) return;
    
    spinnerActive = true;
    std::thread([this, message]() {
        const std::vector<std::string> frames = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
        int frame = 0;
        
        while (spinnerActive) {
            std::cout << "\r" << colorize(frames[frame], Color::CYAN) << " " << message;
            std::cout.flush();
            
            frame = (frame + 1) % frames.size();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "\r" << std::string(message.length() + 10, ' ') << "\r";
        std::cout.flush();
    }).detach();
}

void TerminalUI::stopSpinner() {
    spinnerActive = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

bool TerminalUI::showConfirmDialog(const std::string& message, bool defaultYes) {
    std::string prompt = defaultYes ? " (Y/n): " : " (y/N): ";
    std::cout << colorize("❓ " + message + prompt, Color::YELLOW, Color::BLACK, Style::BOLD);
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.empty()) {
        return defaultYes;
    }
    
    char firstChar = std::tolower(input[0]);
    return firstChar == 'y';
}

std::string TerminalUI::showInputDialog(const std::string& prompt, const std::string& defaultValue) {
    std::string fullPrompt = colorize("📝 " + prompt, Color::CYAN, Color::BLACK, Style::BOLD);
    
    if (!defaultValue.empty()) {
        fullPrompt += colorize(" [" + defaultValue + "]", Color::WHITE);
    }
    
    fullPrompt += ": ";
    std::cout << fullPrompt;
    
    std::string input;
    std::getline(std::cin, input);
    
    return input.empty() ? defaultValue : input;
}

std::vector<int> TerminalUI::showMultiSelect(const std::string& prompt, const std::vector<std::string>& options) {
    std::vector<int> selected;
    std::vector<bool> choices(options.size(), false);
    int currentOption = 0;
    
    // Hide cursor
    std::cout << "\033[?25l";
    
    while (true) {
        // Clear screen and show header
        std::cout << "\033[2J\033[H";
        std::cout << colorize("🔽 " + prompt, Color::CYAN, Color::BLACK, Style::BOLD) << std::endl;
        std::cout << colorize("Use ↑↓ to navigate, SPACE to select, ENTER to confirm", Color::YELLOW) << std::endl << std::endl;
        
        // Show options
        for (size_t i = 0; i < options.size(); i++) {
            std::string prefix = (i == currentOption) ? "► " : "  ";
            std::string checkbox = choices[i] ? "[✓] " : "[ ] ";
            
            Color color = (i == currentOption) ? Color::YELLOW : Color::WHITE;
            Style style = (i == currentOption) ? Style::BOLD : Style::NORMAL;
            
            std::cout << colorize(prefix + checkbox + options[i], color, Color::BLACK, style) << std::endl;
        }
        
        // Get key input
        char key = getchar();
        
        switch (key) {
            case 'A': // Up arrow
                currentOption = (currentOption - 1 + options.size()) % options.size();
                break;
            case 'B': // Down arrow
                currentOption = (currentOption + 1) % options.size();
                break;
            case ' ': // Space - toggle selection
                choices[currentOption] = !choices[currentOption];
                break;
            case '\n': // Enter - confirm
            case '\r':
                goto done;
            case 'q': // Quit
                selected.clear();
                goto done;
        }
    }
    
done:
    // Show cursor again
    std::cout << "\033[?25h";
    
    // Collect selected indices
    for (size_t i = 0; i < choices.size(); i++) {
        if (choices[i]) {
            selected.push_back(i);
        }
    }
    
    return selected;
}

void TerminalUI::showBanner(const std::string& title, const std::string& subtitle) {
    int terminalWidth = 80; // Default width
    std::string border(terminalWidth, '█');
    
    std::cout << colorize(border, Color::GREEN, Color::BLACK, Style::BOLD) << std::endl;
    
    // Center the title
    int padding = (terminalWidth - title.length()) / 2;
    std::string centeredTitle = std::string(padding, ' ') + title;
    
    std::cout << colorize("█" + centeredTitle + std::string(terminalWidth - centeredTitle.length() - 1, ' ') + "█", 
                         Color::GREEN, Color::BLACK, Style::BOLD) << std::endl;
    
    // Show subtitle if provided
    if (!subtitle.empty()) {
        int subtitlePadding = (terminalWidth - subtitle.length()) / 2;
        std::string centeredSubtitle = std::string(subtitlePadding, ' ') + subtitle;
        std::cout << colorize("█" + centeredSubtitle + std::string(terminalWidth - centeredSubtitle.length() - 1, ' ') + "█", 
                             Color::GREEN, Color::BLACK, Style::BOLD) << std::endl;
    }
    
    std::cout << colorize(border, Color::GREEN, Color::BLACK, Style::BOLD) << std::endl;
}

void TerminalUI::showTabledData(const std::vector<std::vector<std::string>>& data, const std::vector<std::string>& headers) {
    if (data.empty()) return;
    
    // Calculate column widths
    std::vector<size_t> colWidths;
    if (!headers.empty()) {
        for (const auto& header : headers) {
            colWidths.push_back(header.length());
        }
    } else {
        colWidths.resize(data[0].size(), 0);
    }
    
    // Find maximum width for each column
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size() && i < colWidths.size(); i++) {
            colWidths[i] = std::max(colWidths[i], row[i].length());
        }
    }
    
    // Add padding
    for (auto& width : colWidths) {
        width += 2;
    }
    
    // Print headers if provided
    if (!headers.empty()) {
        std::cout << colorize("┌", Color::WHITE);
        for (size_t i = 0; i < headers.size(); i++) {
            std::cout << colorize(std::string(colWidths[i], '─'), Color::WHITE);
            if (i < headers.size() - 1) {
                std::cout << colorize("┬", Color::WHITE);
            }
        }
        std::cout << colorize("┐", Color::WHITE) << std::endl;
        
        std::cout << colorize("│", Color::WHITE);
        for (size_t i = 0; i < headers.size(); i++) {
            std::cout << colorize(" " + headers[i] + std::string(colWidths[i] - headers[i].length() - 1, ' '), 
                                Color::CYAN, Color::BLACK, Style::BOLD);
            std::cout << colorize("│", Color::WHITE);
        }
        std::cout << std::endl;
        
        std::cout << colorize("├", Color::WHITE);
        for (size_t i = 0; i < headers.size(); i++) {
            std::cout << colorize(std::string(colWidths[i], '─'), Color::WHITE);
            if (i < headers.size() - 1) {
                std::cout << colorize("┼", Color::WHITE);
            }
        }
        std::cout << colorize("┤", Color::WHITE) << std::endl;
    }
    
    // Print data rows
    for (const auto& row : data) {
        std::cout << colorize("│", Color::WHITE);
        for (size_t i = 0; i < row.size() && i < colWidths.size(); i++) {
            std::cout << " " << colorize(row[i], Color::WHITE) 
                     << std::string(colWidths[i] - row[i].length() - 1, ' ');
            std::cout << colorize("│", Color::WHITE);
        }
        std::cout << std::endl;
    }
    
    // Bottom border
    std::cout << colorize("└", Color::WHITE);
    for (size_t i = 0; i < colWidths.size(); i++) {
        std::cout << colorize(std::string(colWidths[i], '─'), Color::WHITE);
        if (i < colWidths.size() - 1) {
            std::cout << colorize("┴", Color::WHITE);
        }
    }
    std::cout << colorize("┘", Color::WHITE) << std::endl;
}

// Color and style helper methods
std::string TerminalUI::getColorCode(Color color, bool background) {
    int base = background ? 40 : 30;
    
    switch (color) {
        case Color::BLACK: return std::to_string(base + 0);
        case Color::RED: return std::to_string(base + 1);
        case Color::GREEN: return std::to_string(base + 2);
        case Color::YELLOW: return std::to_string(base + 3);
        case Color::BLUE: return std::to_string(base + 4);
        case Color::MAGENTA: return std::to_string(base + 5);
        case Color::CYAN: return std::to_string(base + 6);
        case Color::WHITE: return std::to_string(base + 7);
        default: return std::to_string(base + 7);
    }
}

std::string TerminalUI::getStyleCode(Style style) {
    switch (style) {
        case Style::NORMAL: return "0";
        case Style::BOLD: return "1";
        case Style::DIM: return "2";
        case Style::ITALIC: return "3";
        case Style::UNDERLINE: return "4";
        case Style::BLINK: return "5";
        case Style::REVERSE: return "7";
        case Style::STRIKETHROUGH: return "9";
        default: return "0";
    }
}

void TerminalUI::initColorSchemes() {
    // Neobrutalist theme  
    std::map<Color, std::string> neobrutalist;
    neobrutalist[Color::GREEN] = "primary";
    neobrutalist[Color::YELLOW] = "secondary";
    neobrutalist[Color::MAGENTA] = "accent";
    neobrutalist[Color::RED] = "error";
    neobrutalist[Color::YELLOW] = "warning";
    neobrutalist[Color::GREEN] = "success";
    neobrutalist[Color::CYAN] = "info";
    colorSchemes["neobrutalist"] = neobrutalist;
    
    // Classic theme
    std::map<Color, std::string> classic;
    classic[Color::BLUE] = "primary";
    classic[Color::WHITE] = "secondary";
    classic[Color::CYAN] = "accent";
    classic[Color::RED] = "error";
    classic[Color::YELLOW] = "warning";
    classic[Color::GREEN] = "success";
    classic[Color::BLUE] = "info";
    colorSchemes["classic"] = classic;
}

// Neobrutalist Theme Implementation
NeobrutalistTheme::NeobrutalistTheme() {
    initTheme();
}

void gyatt::NeobrutalistTheme::showLogo() {
    std::string logo = R"(
    ████████╗ ██╗   ██╗ ████████╗ ████████╗
    ██╔═════╝ ╚██╗ ██╔╝ ██╔═══██║ ╚══██╔══╝
    ██║  ████╗ ╚████╔╝  ██║   ██║    ██║   
    ██║   ███║  ╚██╔╝   ██║▄▄▄██║    ██║   
    ╚███████╔╝   ██║    ╚██████╔╝    ██║   
     ╚══════╝    ╚═╝     ╚═════╝     ╚═╝   
    
    ═══════════════════════════════════════
    🔥 THE MOST BRUTAL VERSION CONTROL 🔥
    ═══════════════════════════════════════
    )";
    
    std::cout << TerminalUI().colorize(logo, TerminalUI::Color::GREEN, TerminalUI::Color::BLACK, TerminalUI::Style::BOLD) << std::endl;
}

void gyatt::NeobrutalistTheme::showWelcomeMessage() {
    std::cout << TerminalUI().colorize("🚀 WELCOME TO GYATT - WHERE CODE MEETS CHAOS! 🚀", 
                                     TerminalUI::Color::YELLOW, TerminalUI::Color::BLACK, TerminalUI::Style::BOLD) << std::endl;
    std::cout << TerminalUI().colorize("💀 PREPARE FOR THE MOST BRUTAL VERSION CONTROL EXPERIENCE 💀", 
                                     TerminalUI::Color::RED, TerminalUI::Color::BLACK, TerminalUI::Style::BOLD) << std::endl;
    std::cout << std::endl;
}

void gyatt::NeobrutalistTheme::showBrutalistAnimation(const std::string& message) {
    const std::vector<std::string> frames = {
        "▰▱▱▱▱▱▱▱▱▱",
        "▰▰▱▱▱▱▱▱▱▱",
        "▰▰▰▱▱▱▱▱▱▱",
        "▰▰▰▰▱▱▱▱▱▱",
        "▰▰▰▰▰▱▱▱▱▱",
        "▰▰▰▰▰▰▱▱▱▱",
        "▰▰▰▰▰▰▰▱▱▱",
        "▰▰▰▰▰▰▰▰▱▱",
        "▰▰▰▰▰▰▰▰▰▱",
        "▰▰▰▰▰▰▰▰▰▰"
    };
    
    for (const auto& frame : frames) {
        std::cout << "\r" << TerminalUI().colorize("🔥 " + message + " ", TerminalUI::Color::RED, TerminalUI::Color::BLACK, TerminalUI::Style::BOLD);
        std::cout << TerminalUI().colorize(frame, TerminalUI::Color::YELLOW, TerminalUI::Color::BLACK, TerminalUI::Style::BOLD);
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << " " << TerminalUI().colorize("DONE!", TerminalUI::Color::GREEN, TerminalUI::Color::BLACK, TerminalUI::Style::BOLD) << std::endl;
}

void gyatt::NeobrutalistTheme::initTheme() {
    // Initialize brutal ASCII art and styling
    brutalBorders = {
        "████████████████████████████████████████",
        "▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰▰",
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━",
        "═══════════════════════════════════════"
    };
}

// Command Aliases Implementation
gyatt::CommandAliases::CommandAliases() {
    initAliases();
}

gyatt::CommandAliases::CommandAliases(const std::string& repoPath) 
    : repoPath(repoPath), aliasesFile(repoPath + "/.gyatt/aliases.json") {
    initAliases();
    loadAliases();
}

void gyatt::CommandAliases::initAliases() {
    // GYATT-specific command aliases
    customAliases["yeet"] = "add";
    customAliases["regret"] = "reset";
    customAliases["vibe"] = "status";
    customAliases["summon"] = "checkout";
    customAliases["fr"] = "commit";
    customAliases["nocap"] = "push";
    customAliases["slay"] = "push --force";
    customAliases["flex"] = "log --oneline --graph";
    customAliases["ghost"] = "stash";
    customAliases["haunt"] = "stash pop";
    customAliases["sus"] = "diff";
    customAliases["based"] = "branch";
    customAliases["cringe"] = "revert";
    customAliases["ratio"] = "merge";
    customAliases["cope"] = "reset --hard";
    customAliases["touch-grass"] = "clean -fd";
    customAliases["no-bitches"] = "status --porcelain";
    customAliases["sigma"] = "tag";
    customAliases["alpha"] = "remote";
    customAliases["beta"] = "fetch";
    customAliases["gigachad"] = "rebase";
    customAliases["midjourney"] = "log --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset'";
}

std::string gyatt::CommandAliases::resolveAlias(const std::string& alias) {
    auto it = customAliases.find(alias);
    if (it != customAliases.end()) {
        return it->second;
    }
    return alias;
}

bool gyatt::CommandAliases::isAlias(const std::string& command) {
    return customAliases.find(command) != customAliases.end();
}

void gyatt::CommandAliases::addAlias(const std::string& alias, const std::string& command) {
    customAliases[alias] = command;
    saveAliases();
}

bool gyatt::CommandAliases::removeAlias(const std::string& alias) {
    customAliases.erase(alias);
    saveAliases();
    return true;
}

std::vector<std::pair<std::string, std::string>> gyatt::CommandAliases::listAliases() {
    std::vector<std::pair<std::string, std::string>> aliasList;
    for (const auto& pair : customAliases) {
        aliasList.push_back({pair.first, pair.second});
    }
    return aliasList;
}

void gyatt::CommandAliases::showAliasHelp() {
    TerminalUI ui;
    
    ui.showBanner("🔥 GYATT COMMAND ALIASES 🔥");
    
    std::cout << ui.colorize("Most brutal aliases in the game:", ui.Color::YELLOW, ui.Color::BLACK, ui.Style::BOLD) << std::endl << std::endl;
    
    std::vector<std::vector<std::string>> aliasTable = {
        {"gyatt yeet", "gyatt add", "Add files like throwing them into the void"},
        {"gyatt regret", "gyatt reset", "Undo your poor life choices"},
        {"gyatt vibe", "gyatt status", "Check what's good in your repo"},
        {"gyatt summon", "gyatt checkout", "Bring branches back from the dead"},
        {"gyatt fr", "gyatt commit", "For real commit your changes"},
        {"gyatt nocap", "gyatt push", "Push without lying about it"},
        {"gyatt slay", "gyatt push --force", "Absolutely destroy the remote"},
        {"gyatt flex", "gyatt log --oneline --graph", "Show off your commit history"},
        {"gyatt ghost", "gyatt stash", "Make changes disappear mysteriously"},
        {"gyatt haunt", "gyatt stash pop", "Bring back haunting changes"},
        {"gyatt sus", "gyatt diff", "Something's not right here..."},
        {"gyatt based", "gyatt branch", "Create branches like a sigma"},
        {"gyatt cringe", "gyatt revert", "That commit was pure cringe"},
        {"gyatt ratio", "gyatt merge", "Merge and ratio the competition"},
        {"gyatt cope", "gyatt reset --hard", "Hard reset when you can't cope"},
        {"gyatt touch-grass", "gyatt clean -fd", "Clean up and go outside"},
        {"gyatt gigachad", "gyatt rebase", "Rebase like an absolute unit"},
        {"gyatt sigma", "gyatt tag", "Tag releases with sigma energy"},
        {"gyatt alpha", "gyatt remote", "Manage remotes with alpha vibes"},
        {"gyatt beta", "gyatt fetch", "Fetch updates (beta behavior)"}
    };
    
    std::vector<std::string> headers = {"Alias", "Command", "Description"};
    ui.showTabledData(aliasTable, headers);
    
    std::cout << std::endl;
    std::cout << ui.colorize("💡 Pro tip: Use 'gyatt alias add <alias> <command>' to create custom aliases!", 
                           ui.Color::CYAN, ui.Color::BLACK, ui.Style::ITALIC) << std::endl;
}

bool gyatt::CommandAliases::saveAliases() {
    // Save aliases to config file
    std::ofstream file(".gyatt/aliases.json");
    Json::Value aliasJson;
    
    for (const auto& pair : customAliases) {
        aliasJson[pair.first] = pair.second;
    }
    
    file << aliasJson;
    return true;
}

bool CommandAliases::loadAliases() {
    // Load aliases from config file
    std::ifstream file(".gyatt/aliases.json");
    if (file.is_open()) {
        Json::Value aliasJson;
        file >> aliasJson;
        
        for (const auto& key : aliasJson.getMemberNames()) {
            customAliases[key] = aliasJson[key].asString();
        }
    }
    return true;
}

bool gyatt::CommandAliases::addCustomAlias(const std::string& alias, const std::string& command) {
    customAliases[alias] = command;
    return saveAliases();
}

std::map<std::string, std::string> gyatt::CommandAliases::getAllAliases() {
    return customAliases;
}

void gyatt::NeobrutalistTheme::showGyattSplash() {
    std::cout << R"(
    ██████  ██    ██  █████  ████████ ████████ 
   ██        ██  ██  ██   ██    ██       ██    
   ██   ███   ████   ███████    ██       ██    
   ██    ██    ██    ██   ██    ██       ██    
    ██████     ██    ██   ██    ██       ██    
                                              
    🔥 BRUTAL VERSION CONTROL 🔥
    )" << std::endl;
}

} // namespace gyatt