#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace gyatt {

class TerminalUI {
public:
    enum class Color {
        BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
        BRIGHT_BLACK, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
        BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
    };

    enum class Style {
        NORMAL, BOLD, DIM, ITALIC, UNDERLINE, BLINK, REVERSE, STRIKETHROUGH
    };

    TerminalUI();
    
    // Color and styling
    std::string colorize(const std::string& text, Color fg, Color bg = Color::BLACK, Style style = Style::NORMAL);
    void setColorScheme(const std::string& scheme); // "neobrutalist", "dracula", "monokai", "solarized"
    
    // UI Components
    void showProgressBar(const std::string& label, int progress, int total);
    void showSpinner(const std::string& message);
    void stopSpinner();
    
    bool showConfirmDialog(const std::string& message, bool defaultYes = true);
    std::string showInputDialog(const std::string& prompt, const std::string& defaultValue = "");
    int showMenuDialog(const std::string& title, const std::vector<std::string>& options);
    std::vector<int> showMultiSelect(const std::string& prompt, const std::vector<std::string>& options);
    
    // Animations
    void showLoadingAnimation(const std::string& message, int durationMs = 2000);
    void showSuccessAnimation(const std::string& message);
    void showErrorAnimation(const std::string& message);
    
    // Layout and formatting
    void clearScreen();
    void moveCursor(int row, int col);
    void showBanner(const std::string& title, const std::string& subtitle = "");
    void showSeparator(char character = '=', int length = 80);
    
    // Interactive widgets
    class Slider {
    public:
        Slider(const std::string& label, int min, int max, int initial);
        int show();
    private:
        std::string label;
        int minVal, maxVal, currentVal;
    };
    
    class Toggle {
    public:
        Toggle(const std::string& label, bool initial = false);
        bool show();
    private:
        std::string label;
        bool value;
    };
    
    // Advanced layouts
    void showTwoColumnLayout(const std::vector<std::string>& leftContent,
                           const std::vector<std::string>& rightContent);
    void showTabledData(const std::vector<std::vector<std::string>>& data,
                       const std::vector<std::string>& headers = {});
    
private:
    std::string currentColorScheme;
    bool spinnerActive;
    
    std::string getColorCode(Color color, bool background = false);
    std::string getStyleCode(Style style);
    void resetColors();
    std::map<std::string, std::map<Color, std::string>> colorSchemes;
    void initColorSchemes();
};

class CommandAliases {
public:
    CommandAliases(); // Default constructor
    CommandAliases(const std::string& repoPath);
    
    // Vibe commands
    bool executeYeet(const std::vector<std::string>& args);     // add
    bool executeRegret(const std::vector<std::string>& args);   // reset/revert
    bool executeVibe(const std::vector<std::string>& args);     // status with style
    bool executeSummon(const std::vector<std::string>& args);   // checkout branch
    bool executeDamnit(const std::vector<std::string>& args);   // init
    bool executeFr(const std::vector<std::string>& args);       // commit (for real)
    bool executeNoCap(const std::vector<std::string>& args);    // push (no lies)
    bool executeBussin(const std::vector<std::string>& args);   // successful operation
    bool executeSlay(const std::vector<std::string>& args);     // force push
    bool executeSpill(const std::vector<std::string>& args);    // show detailed log
    bool executeGhostMode(const std::vector<std::string>& args); // work on detached head
    
    // Alias management
    bool addCustomAlias(const std::string& alias, const std::string& command);
    void addAlias(const std::string& alias, const std::string& command);
    bool isAlias(const std::string& command);
    std::string resolveAlias(const std::string& alias);
    bool removeAlias(const std::string& alias);
    std::map<std::string, std::string> getAllAliases();
    void showAliasHelp();
    std::vector<std::pair<std::string, std::string>> listAliases();
    
private:
    std::string repoPath;
    std::string aliasesFile;
    std::map<std::string, std::string> customAliases;
    
    void initAliases();
    bool saveAliases();
    bool loadAliases();
    std::string translateVibeCommand(const std::string& command);
};

class NeobrutalistTheme {
public:
    NeobrutalistTheme();
    
    // Theme colors (bold, high contrast)
    struct Colors {
        std::string primary;      // "#FF6B6B" - bold red
        std::string secondary;    // "#4ECDC4" - bold cyan
        std::string accent;       // "#FFE66D" - bold yellow
        std::string success;      // "#51CF66" - bold green
        std::string warning;      // "#FF922B" - bold orange
        std::string error;        // "#FA5252" - bold red
        std::string text;         // "#2D3748" - dark gray
        std::string background;   // "#F7FAFC" - light gray
        std::string border;       // "#000000" - black
    };
    
    Colors getColors();
    
    // Neobrutalist UI elements
    std::string createBorderedBox(const std::string& content, int width = 60);
    std::string createHeader(const std::string& title);
    std::string createButton(const std::string& text, bool highlighted = false);
    std::string createProgressBar(int progress, int total, int width = 40);
    
    // ASCII art and decorations
    std::string getGyattLogo();
    std::string getSuccessIcon();
    std::string getErrorIcon();
    std::string getWarningIcon();
    void showLogo();
    void showWelcomeMessage();
    
    // Animations
    void showGyattSplash();
    void showBrutalistAnimation(const std::string& message);
    
    // Helper methods
    char getChar();
    int getTerminalWidth();
    
private:
    Colors colors;
    std::vector<std::string> brutalBorders;
    
    void initTheme();
    std::string applyNeobrutalistStyle(const std::string& text, const std::string& color);
    std::string createShadowEffect(const std::string& text);
};

} // namespace gyatt
