/*
    * Gyatt - A modern, human-readable Git CLI tool
    * 
    * This tool enhances the Git experience with features like:
    * - Human-readable commits with emojis
    * - Semantic branching and auto-naming
    * - Section-based staging
    * - Project mapping and visualization
    * - Checkpoints and snapshots
    * - Guardrails for safety
    * - Plugin ecosystem for extensibility
    * - Session recording and playback
    * 
    * Built with love and chaos energy.
*/


#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "../include/repository.h"
#include "../include/utils.h"
#include "../include/markdown_commit.h"
#include "../include/semantic_branching.h"
#include "../include/section_staging.h"
#include "../include/project_mapper.h"
#include "../include/checkpoint_system.h"
#include "../include/guardrails.h"
#include "../include/plugin_system.h"
#include "../include/enhanced_features.h"
#include "../include/terminal_ui.h"
#include "../include/advanced_features.h"

void printUsage() {
    gyatt::TerminalUI ui;
    gyatt::NeobrutalistTheme theme;
    
    theme.showGyattSplash();
    
    std::cout << ui.colorize("ο HUMAN-READABLE COMMITS", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("fr", ui.Color::GREEN) << " -m \!α¨ feat: markdown commits with emojis\"" << std::endl;
    std::cout << "  " << ui.colorize("commit-interactive", ui.Color::GREEN) << "     Interactive commit builder" << std::endl;
    std::cout << "  " << ui.colorize("story-mode", ui.Color::GREEN) << "             Enable narrative commits" << std::endl << std::endl;

    std::cout << ui.colorize(!π³ SEMANTIC BRANCHING", ui.Color::YELLOW, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("start feature/login", ui.Color::GREEN) << "    Auto-naming + TODO.md generation" << std::endl;
    std::cout << "  " << ui.colorize("start bugfix/auth", ui.Color::GREEN) << "     Semantic branch creation" << std::endl;
    std::cout << "  " << ui.colorize("loopback", ui.Color::GREEN) << "              Selective commit merging" << std::endl << std::endl;

    std::cout << ui.colorize(!βο· SECTION-BASED STAGING", ui.Color::MAGENTA, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("stage-function", ui.Color::GREEN) << "         Stage by function/class" << std::endl;
    std::cout << "  " << ui.colorize("stage-interactive", ui.Color::GREEN) << "     Smart section staging" << std::endl << std::endl;

    std::cout << ui.colorize(!οΉο· PROJECT MAPPING", ui.Color::BLUE, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("map", ui.Color::GREEN) << "                   Generate project overview" << std::endl;
    std::cout << "  " << ui.colorize("map --visual", ui.Color::GREEN) << "          Visual dependency graph" << std::endl << std::endl;

    std::cout << ui.colorize(!ο CHECKPOINTS & SNAPSHOTS", ui.Color::RED, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("mark alpha-release", ui.Color::GREEN) << "   Create checkpoint" << std::endl;
    std::cout << "  " << ui.colorize("rewind 3 --soft", ui.Color::GREEN) << "      Time travel safely" << std::endl;
    std::cout << "  " << ui.colorize("oops-shield", ui.Color::GREEN) << "          Emergency recovery" << std::endl << std::endl;

    std::cout << ui.colorize(!π‘ξ· GUARDRAILS & OVERRIDES", ui.Color::YELLOW, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("fr", ui.Color::GREEN) << " -m \"fix\" --no-verify --no-format" << std::endl;
    std::cout << "  " << ui.colorize("guardrails", ui.Color::GREEN) << "            Manage safety rules" << std::endl << std::endl;

    std::cout << ui.colorize("ο§© PLUGIN ECOSYSTEM", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("install changelog-gen", ui.Color::GREEN) << "  Auto changelog generation" << std::endl;
    std::cout << "  " << ui.colorize("install undo-commits", ui.Color::GREEN) << "   Smart commit undoing" << std::endl << std::endl;

    std::cout << ui.colorize("ο¬ COMMENTS & NOTES", ui.Color::MAGENTA, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("comment add", ui.Color::GREEN) << " file.cpp:42 \"Fix this bug\"" << std::endl;
    std::cout << "  " << ui.colorize("sticky-note", ui.Color::GREEN) << "           Drop persistent notes" << std::endl;
    std::cout << "  " << ui.colorize("label core", ui.Color::GREEN) << " file.cpp   Label-based organization" << std::endl << std::endl;

    std::cout << ui.colorize(!π₯ SESSION RECORDING", ui.Color::BLUE, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("record start", ui.Color::GREEN) << "           Begin session recording" << std::endl;
    std::cout << "  " << ui.colorize("export session", ui.Color::GREEN) << "        Export to .asciinema" << std::endl << std::endl;

    std::cout << ui.colorize("ο§αο« TEMPLATES & INIT", ui.Color::YELLOW, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("damnit react", ui.Color::GREEN) << "          React project template" << std::endl;
    std::cout << "  " << ui.colorize("damnit python-flask", ui.Color::GREEN) << "   Flask project template" << std::endl << std::endl;

    std::cout << ui.colorize("ο€ VIBE COMMANDS", ui.Color::RED, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  " << ui.colorize("yeet", ui.Color::GREEN) << " file.cpp         Add files (no cap)" << std::endl;
    std::cout << "  " << ui.colorize("regret", ui.Color::GREEN) << "              Undo changes" << std::endl;
    std::cout << "  " << ui.colorize("vibe", ui.Color::GREEN) << "                Status with style" << std::endl;
    std::cout << "  " << ui.colorize("summon", ui.Color::GREEN) << " branch       Checkout branch" << std::endl;
    std::cout << "  " << ui.colorize("nocap", ui.Color::GREEN) << "              Push (no lies)" << std::endl;
    std::cout << "  " << ui.colorize("slay", ui.Color::GREEN) << "               Force push" << std::endl;
    std::cout << "  " << ui.colorize("flex", ui.Color::GREEN) << "               Show off commits" << std::endl;
    std::cout << "  " << ui.colorize("ghost", ui.Color::GREEN) << "              Stash changes" << std::endl << std::endl;

    std::cout << ui.colorize(!ο CLASSIC COMMANDS", ui.Color::WHITE, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
    std::cout << "  init, clone, add, commit, status, log, diff, branch, checkout, push" << std::endl;
    std::cout << "  remote, show, github-token, gyattignore, help" << std::endl << std::endl;

    std::cout << ui.colorize("Built withο and pure chaos energy", ui.Color::MAGENTA) << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    std::string command = argv[1];
    std::vector<std::string> args;
    
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    
    gyatt::Repository repo;
    gyatt::TerminalUI ui;
    gyatt::CommandAliases aliases(repo.getRepoPath());
    gyatt::NeobrutalistTheme theme;
    
    std::string originalCommand = command;
    if (command == "yeet" || command == "regret" || command == "vibe" || command == "summon" || 
        command == "fr" || command == "nocap" || command == "slay" || command == "spill" || 
        command == "ghost-mode") {
    }
    
    try {
        // ============== INITIALIZATION COMMANDS ==============
        if (command == "init" || command == "damnit") {
            if (args.empty()) {
                if (repo.init()) {
                    theme.showBrutalistAnimation("INITIALIZING GYATT REPOSITORY");
                    std::cout << ui.colorize(!α Initialized empty Gyatt repository in " + repo.getRepoPath() + "/.gyatt", 
                                           ui.Color::GREEN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                    
                    gyatt::PluginManager plugins(repo.getRepoPath());
                    plugins.installChangelogGenerator();
                    plugins.installUndoCommits();
                    plugins.installCodeFormatter();
                    plugins.installTestRunner();
                    
                    return 0;
                } else {
                    std::cerr << ui.colorize(!β Failed to initialize repository", ui.Color::RED) << std::endl;
                    return 1;
                }
            } else {
                gyatt::InitTemplates templates(repo.getRepoPath());
                std::string templateName = args[0];
                
                if (templates.initCustomTemplate(templateName, templateName)) {
                    theme.showBrutalistAnimation("INITIALIZING " + templateName + " PROJECT");
                    std::cout << ui.colorize(!α Initialized " + templateName + " project with Gyatt", 
                                           ui.Color::GREEN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                    return 0;
                } else {
                    std::cerr << ui.colorize(!α Failed to initialize from template: " + templateName, ui.Color::RED) << std::endl;
                    return 1;
                }
            }
        }
        
        else if (command == "clone") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt clone <repository> [directory]" << std::endl;
                return 1;
            }
            
            std::string sourceUrl = args[0];
            std::string targetDir = args.size() > 1 ? args[1] : "";
            
            gyatt::Repository cloneRepo(".");
            if (cloneRepo.clone(sourceUrl, targetDir)) {
                return 0;
            } else {
                std::cerr << "Failed to clone repository" << std::endl;
                return 1;
            }
        }
        
        else if (command == "help" || command == "--help" || command == "-h") {
            printUsage();
            return 0;
        }
        
        if (!repo.isRepository()) {
            std::cerr << ui.colorize(!π fatal: not a gyatt repository (or any of the parent directories): .gyatt", 
                                   ui.Color::RED, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
            return 1;
        }
        
        gyatt::MarkdownCommit markdownCommit(repo.getRepoPath());
        gyatt::SemanticBranching semanticBranching(repo.getRepoPath());
        gyatt::SectionBasedStaging sectionStaging(repo.getRepoPath());
        gyatt::ProjectMapper projectMapper(repo.getRepoPath());
        gyatt::CheckpointSystem checkpoints(repo.getRepoPath());
        gyatt::OopsShield oopsShield(repo.getRepoPath());
        gyatt::RewindMode rewindMode(repo.getRepoPath());
        gyatt::GuardrailSystem guardrails(repo.getRepoPath());
        gyatt::PluginManager plugins(repo.getRepoPath());
        gyatt::SessionRecorder recorder(repo.getRepoPath());
        gyatt::CommentThread comments(repo.getRepoPath());
        gyatt::StickyNotes stickyNotes(repo.getRepoPath());
        gyatt::LabelSystem labels(repo.getRepoPath());
        gyatt::InitTemplates templates(repo.getRepoPath());
        gyatt::CommitStoryMode storyMode(repo.getRepoPath());
        gyatt::ContainerizedSnapshots snapshots(repo.getRepoPath());
        
        // ============== PERFORMANCE COMMANDS ==============
        if (command == "perf") {
            if (args.empty()) {
                // Show performance metrics
                auto metrics = repo.getPerformanceMetrics();
                std::cout << ui.colorize(!π Performance Metrics:", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                std::cout << "  Total time: " << metrics.totalTime.count() << "ms" << std::endl;
                std::cout << "  Files processed: " << metrics.filesProcessed << std::endl;
                std::cout << "  Bytes processed: " << metrics.bytesProcessed << std::endl;
                std::cout << "  Cache hit rate: " << metrics.cacheHits << "%" << std::endl;
                std::cout << "  Compression ratio: " << metrics.compressionRatio << std::endl;
                std::cout << "  Parallel threads: " << metrics.parallelThreadsUsed << std::endl;
                return 0;
            }
            
            std::string subcommand = args[0];
            if (subcommand == "enable") {
                repo.enablePerformanceOptimizations(true);
                std::cout << ui.colorize(!ο Performance optimizations enabled", ui.Color::GREEN) << std::endl;
            } else if (subcommand == "disable") {
                repo.enablePerformanceOptimizations(false);
                std::cout << ui.colorize("αΈξ·  Performance optimizations disabled", ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "parallel") {
                bool enable = args.size() > 1 ? (args[1] == "on") : true;
                repo.enableParallelProcessing(enable);
                std::cout << ui.colorize(!β‘ Parallel processing " + std::string(enable ? "enabled" : "disabled"), 
                                       enable ? ui.Color::GREEN : ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "cache") {
                bool enable = args.size() > 1 ? (args[1] == "on") : true;
                repo.enableObjectCaching(enable);
                std::cout << ui.colorize(!πΎ Object caching " + std::string(enable ? "enabled" : "disabled"), 
                                       enable ? ui.Color::GREEN : ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "compression") {
                bool enable = args.size() > 1 ? (args[1] == "on") : true;
                repo.enableDeltaCompression(enable);
                std::cout << ui.colorize(!οο·  Delta compression " + std::string(enable ? "enabled" : "disabled"), 
                                       enable ? ui.Color::GREEN : ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "mmap") {
                bool enable = args.size() > 1 ? (args[1] == "on") : true;
                repo.enableMemoryMapping(enable);
                std::cout << ui.colorize("ο Memory mapping " + std::string(enable ? "enabled" : "disabled"), 
                                       enable ? ui.Color::GREEN : ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "memory") {
                bool enable = args.size() > 1 ? (args[1] == "on") : true;
                repo.enableMemoryOptimization(enable);
                std::cout << ui.colorize(!π¦  Memory optimization " + std::string(enable ? "enabled" : "disabled"), 
                                       enable ? ui.Color::GREEN : ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "autotune") {
                bool enable = args.size() > 1 ? (args[1] == "on") : true;
                repo.enableAutoTuning(enable);
                std::cout << ui.colorize("ο― Auto-tuning " + std::string(enable ? "enabled" : "disabled"), 
                                       enable ? ui.Color::GREEN : ui.Color::YELLOW) << std::endl;
            } else if (subcommand == "optimize-performance") {
                repo.optimizeForPerformance();
                std::cout << ui.colorize("ο System optimized for maximum performance", ui.Color::GREEN) << std::endl;
            } else if (subcommand == "optimize-memory") {
                repo.optimizeForMemory();
                std::cout << ui.colorize(!πΎ System optimized for minimal memory usage", ui.Color::GREEN) << std::endl;
            } else if (subcommand == "optimize-batch") {
                repo.optimizeForBatch();
                std::cout << ui.colorize("ο¦ System optimized for batch operations", ui.Color::GREEN) << std::endl;
            } else if (subcommand == "gc") {
                std::cout << ui.colorize(!ο¦Ή Performing garbage collection...", ui.Color::CYAN) << std::endl;
                repo.performGarbageCollection();
                std::cout << ui.colorize("α Garbage collection complete", ui.Color::GREEN) << std::endl;
            } else if (subcommand == "profile") {
                auto profile = repo.getMemoryProfile();
                std::cout << ui.colorize("ο Memory Profile:", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                std::cout << "  System Memory: " << (profile.totalSystemMemory / 1024.0 / 1024.0) << " MB" << std::endl;
                std::cout << "  Available Memory: " << (profile.availableMemory / 1024.0 / 1024.0) << " MB" << std::endl;
                std::cout << "  Process Memory: " << (profile.processMemoryUsage / 1024.0 / 1024.0) << " MB" << std::endl;
                std::cout << "  Pool Memory: " << (profile.poolMemoryUsage / 1024.0 / 1024.0) << " MB" << std::endl;
                std::cout << "  Cache Memory: " << (profile.cacheMemoryUsage / 1024.0 / 1024.0) << " MB" << std::endl;
                std::cout << "  Memory Efficiency: " << (profile.memoryEfficiency * 100.0) << "%" << std::endl;
                
                // Show compression statistics if available
                if (profile.compressedDataSize > 0) {
                    std::cout << ui.colorize("\mοο·  Compression Statistics:", ui.Color::BLUE, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                    std::cout << "  Compressed Data: " << (profile.compressedDataSize / 1024.0 / 1024.0) << " MB" << std::endl;
                    std::cout << "  Uncompressed Data: " << (profile.uncompressedDataSize / 1024.0 / 1024.0) << " MB" << std::endl;
                    std::cout << "  Compression Ratio: " << std::fixed << std::setprecision(1) 
                              << (profile.overallCompressionRatio * 100.0) << "%" << std::endl;
                    std::cout << "  Space Saved: " << (profile.totalSpaceSaved / 1024.0 / 1024.0) << " MB" << std::endl;
                    std::cout << "  Compression Time: " << profile.compressionTime.count() << "ms" << std::endl;
                }
                
                return 0;
            } else if (subcommand == "compression") {
                if (args.size() < 2) {
                    std::cout << "Usage: gyatt perf compression [enable|disable|optimize|speed|size|balance|full]" << std::endl;
                    return 1;
                }
                
                std::string compressionCmd = args[1];
                if (compressionCmd == "enable" || compressionCmd == "on") {
                    repo.enableCompressionIntegration(true);
                    std::cout << ui.colorize(!πξ·  Compression integration enabled", ui.Color::GREEN) << std::endl;
                } else if (compressionCmd == "disable" || compressionCmd == "off") {
                    repo.enableCompressionIntegration(false);
                    std::cout << ui.colorize(!πξ·  Compression integration disabled", ui.Color::YELLOW) << std::endl;
                } else if (compressionCmd == "optimize") {
                    std::cout << ui.colorize("ο Running compression optimization...", ui.Color::CYAN) << std::endl;
                    if (repo.optimizeWithCompression()) {
                        std::cout << ui.colorize(!α Compression optimization completed", ui.Color::GREEN) << std::endl;
                    } else {
                        std::cout << ui.colorize(!β Compression optimization failed", ui.Color::RED) << std::endl;
                    }
                } else if (compressionCmd == "speed") {
                    repo.optimizeCompressionForSpeed();
                    std::cout << ui.colorize("α‘ Compression optimized for speed", ui.Color::GREEN) << std::endl;
                } else if (compressionCmd == "size") {
                    repo.optimizeCompressionForSize();
                    std::cout << ui.colorize("ο¦ Compression optimized for size", ui.Color::GREEN) << std::endl;
                } else if (compressionCmd == "balance") {
                    repo.optimizeCompressionForBalance();
                    std::cout << ui.colorize("αξ·  Compression optimized for balance", ui.Color::GREEN) << std::endl;
                } else if (compressionCmd == "full") {
                    std::cout << ui.colorize(!π Performing full compression optimization...", ui.Color::CYAN) << std::endl;
                    repo.performFullCompressionOptimization();
                    std::cout << ui.colorize("α Full compression optimization completed", ui.Color::GREEN) << std::endl;
                } else {
                    std::cout << "Unknown compression command: " << compressionCmd << std::endl;
                    return 1;
                }
                return 0;
            } else {
                std::cout << "Usage: gyatt perf [enable|disable|parallel|cache|compression|mmap|memory|autotune|optimize-performance|optimize-memory|optimize-batch|gc|profile|tune|reset]" << std::endl;
                return 1;
            }
            return 0;
        }
        
        else if (command == "fast-add") {
            if (args.empty()) {
                std::cerr << "Nothing specified, nothing added." << std::endl;
                return 1;
            }
            
            // Use performance-optimized batch add
            if (!repo.addOptimized(args)) {
                std::cerr << "Failed to add files" << std::endl;
                return 1;
            }
            std::cout << ui.colorize(!ο Files added with optimizations", ui.Color::GREEN) << std::endl;
            return 0;
        }
        
        else if (command == "fast-commit") {
            if (args.size() >= 2 && args[0] == "-m") {
                std::string message = args[1];
                
                if (repo.commitOptimized(message)) {
                    std::cout << ui.colorize(!π Optimized commit created successfully", ui.Color::GREEN) << std::endl;
                    return 0;
                } else {
                    std::cerr << ui.colorize(!β Failed to create commit", ui.Color::RED) << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Usage: gyatt fast-commit -m <message>" << std::endl;
                return 1;
            }
        }
        
        else if (command == "fast-status") {
            auto statuses = repo.statusOptimized();
            
            std::cout << "On branch " << repo.getCurrentBranch() << "\n";
            
            if (statuses.empty()) {
                std::cout << ui.colorize(!α Working tree clean", ui.Color::GREEN) << std::endl;
            } else {
                std::cout << ui.colorize("ο Optimized file status:", ui.Color::CYAN) << std::endl;
                for (const auto& [file, status] : statuses) {
                    std::string color_status;
                    if (status == "modified") {
                        color_status = ui.colorize("modified", ui.Color::RED);
                    } else if (status == "staged") {
                        color_status = ui.colorize("staged", ui.Color::GREEN);
                    } else if (status == "deleted") {
                        color_status = ui.colorize("deleted", ui.Color::RED);
                    } else {
                        color_status = ui.colorize(status, ui.Color::YELLOW);
                    }
                    std::cout << "  " << color_status << ": " << file << std::endl;
                }
            }
            return 0;
        }
        
        // ============== BASIC VERSION CONTROL ==============
        if (command == "add") {
            if (args.empty()) {
                std::cerr << "Nothing specified, nothing added." << std::endl;
                return 1;
            }
            
            for (const auto& file : args) {
                if (!repo.add(file)) {
                    std::cerr << "Failed to add file: " << file << std::endl;
                    return 1;
                }
            }
            std::cout << ui.colorize("α Files added successfully", ui.Color::GREEN) << std::endl;
            return 0;
        }
        
        else if (command == "commit") {
            if (args.size() >= 2 && args[0] == "-m") {
                std::string message = args[1];
                
                // Check for story mode
                // Note: CommitStoryMode doesn't have isEnabled/enhanceCommitMessage methods
                // Using direct commit for now
                
                std::vector<std::string> files; // Empty for now, could get from staging area
                if (!guardrails.runPreCommitChecks(files)) {
                    if (!ui.showConfirmDialog("Guardrails detected issues. Continue anyway?", false)) {
                        std::cout << ui.colorize("α Commit cancelled", ui.Color::YELLOW) << std::endl;
                        return 1;
                    }
                }
                
                if (repo.commit(message)) {
                    std::cout << ui.colorize(!β Commit created successfully", ui.Color::GREEN) << std::endl;
                    return 0;
                } else {
                    std::cerr << ui.colorize("α Failed to create commit", ui.Color::RED) << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Usage: gyatt commit -m <message>" << std::endl;
                return 1;
            }
        }
        
        else if (command == "status") {
            repo.status();
            return 0;
        }
        
        else if (command == "log") {
            repo.log();
            return 0;
        }
        
        else if (command == "diff") {
            repo.diff();
            return 0;
        }
        
        else if (command == "branch") {
            if (args.empty()) {
                repo.listBranches();
            } else {
                if (repo.createBranch(args[0])) {
                    std::cout << "Created branch: " << args[0] << std::endl;
                } else {
                    std::cerr << "Failed to create branch: " << args[0] << std::endl;
                    return 1;
                }
            }
            return 0;
        }
        
        else if (command == "checkout") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt checkout <branch>" << std::endl;
                return 1;
            }
            
            if (repo.checkout(args[0])) {
                std::cout << "Switched to branch '" << args[0] << "'" << std::endl;
            } else {
                std::cerr << "Failed to checkout branch: " << args[0] << std::endl;
                return 1;
            }
            return 0;
        }
        
        // ============== MARKDOWN COMMITS ==============
        else if (command == "commit-interactive") {
            auto commitInfo = markdownCommit.interactiveCommitPrompt();
            if (!commitInfo.title.empty()) {
                std::string message = commitInfo.title + "\n\n" + commitInfo.description;
                if (repo.commit(message)) {
                    std::cout << ui.colorize(!α Interactive commit created successfully", ui.Color::GREEN) << std::endl;
                    return 0;
                }
            }
            std::cerr << ui.colorize(!α Failed to create interactive commit", ui.Color::RED) << std::endl;
            return 1;
        }
        
        else if (command == "export-changelog") {
            if (markdownCommit.exportToMarkdown("CHANGELOG.md")) {
                std::cout << ui.colorize(!β Commit history exported to CHANGELOG.md", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize("α Failed to export commit history", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        // ============== SEMANTIC BRANCHING ==============
        else if (command == "start") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt start <branch-type>/<branch-name>" << std::endl;
                return 1;
            }
            
            if (semanticBranching.startBranch(args[0])) {
                std::cout << ui.colorize(!α Semantic branch created: " + args[0], ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize(!α Failed to create semantic branch", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "loopback") {
            if (semanticBranching.loopbackCommits("", "", {})) {
                std::cout << ui.colorize(!α Loopback completed successfully", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize("α Loopback failed", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        // ============== SECTION-BASED STAGING ==============
        else if (command == "stage-function") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt stage-function <file> [function-name]" << std::endl;
                return 1;
            }
            
            std::string functionName = args.size() > 1 ? args[1] : "";
            if (sectionStaging.stageFunction(args[0], functionName)) {
                std::cout << ui.colorize(!α Function staged successfully", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize("α Failed to stage function", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "stage-interactive") {
            if (args.empty()) {
                sectionStaging.interactiveSectionStaging("");
                std::cout << ui.colorize("α Interactive staging completed", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                sectionStaging.interactiveSectionStaging(args[0]);
                std::cout << ui.colorize(!α Interactive staging completed", ui.Color::GREEN) << std::endl;
                return 0;
            }
        }
        
        // ============== PROJECT MAPPING ==============
        else if (command == "map") {
            bool visual = std::find(args.begin(), args.end(), "--visual") != args.end();
            
            if (visual) {
                if (projectMapper.exportDependencyGraphAsDOT("project_map.dot")) {
                    std::cout << ui.colorize(!α Visual project map exported to project_map.dot", ui.Color::GREEN) << std::endl;
                } else {
                    std::cerr << ui.colorize("α Failed to export visual map", ui.Color::RED) << std::endl;
                }
            } else {
                projectMapper.showProjectSummary();
            }
            return 0;
        }
        
        else if (command == "explore") {
            projectMapper.interactiveExplorer();
            return 0;
        }
        
        // ============== CHECKPOINTS & SNAPSHOTS ==============
        else if (command == "mark") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt mark <checkpoint-name>" << std::endl;
                return 1;
            }
            
            if (checkpoints.markCheckpoint(args[0])) {
                std::cout << ui.colorize(!α Checkpoint created: " + args[0], ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize(!β Failed to create checkpoint", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "rewind") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt rewind <steps> [--soft|--hard]" << std::endl;
                return 1;
            }
            
            int steps = std::stoi(args[0]);
            bool soft = args.size() > 1 && args[1] == "--soft";
            
            if (rewindMode.rewind(steps, soft)) {
                std::cout << ui.colorize(!β Rewind completed", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize(!α Rewind failed", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "oops-shield") {
            if (oopsShield.emergencyRestore()) {
                std::cout << ui.colorize(!β Emergency recovery completed", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize(!α Emergency recovery failed", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        // ============== GUARDRAILS ==============
        else if (command == "guardrails") {
            if (args.empty()) {
                auto guardrailList = guardrails.listGuardrails();
                std::cout << ui.colorize("ο ξΈ Guardrails Status", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                for (const auto& rule : guardrailList) {
                    std::string status = rule.enabled ? !α" : "α";
                    std::cout << status << " " << ui.colorize(rule.name, ui.Color::GREEN) 
                             << " - " << rule.description << std::endl;
                }
                return 0;
            } else if (args[0] == "enable") {
                guardrails.enableGuardrail(args.size() > 1 ? args[1] : "all");
                return 0;
            } else if (args[0] == "disable") {
                guardrails.disableGuardrail(args.size() > 1 ? args[1] : "all");
                return 0;
            }
        }
        
        // ============== PLUGIN SYSTEM ==============
        else if (command == "install") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt install <plugin-name> [source]" << std::endl;
                return 1;
            }
            
            std::string source = args.size() > 1 ? args[1] : "";
            if (plugins.installPlugin(args[0], source)) {
                return 0;
            } else {
                return 1;
            }
        }
        
        else if (command == "uninstall") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt uninstall <plugin-name>" << std::endl;
                return 1;
            }
            
            if (plugins.uninstallPlugin(args[0])) {
                return 0;
            } else {
                return 1;
            }
        }
        
        else if (command == "plugins") {
            auto pluginList = plugins.listPlugins();
            ui.showBanner("Installed Plugins");
            
            for (const auto& plugin : pluginList) {
                std::string status = plugin.enabled ? !β" : !α";
                std::cout << status << " " << ui.colorize(plugin.name, ui.Color::CYAN) 
                         << " v" << plugin.version << " - " << plugin.description << std::endl;
            }
            return 0;
        }
        
        else if (command == "run") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt run <plugin-name> [args...]" << std::endl;
                return 1;
            }
            
            std::string pluginName = args[0];
            std::vector<std::string> pluginArgs(args.begin() + 1, args.end());
            
            if (plugins.executePlugin(pluginName, pluginArgs)) {
                return 0;
            } else {
                return 1;
            }
        }
        
        // ============== SESSION RECORDING ==============
        else if (command == "record") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt record <start|stop> [session-name]" << std::endl;
                return 1;
            }
            
            if (args[0] == "start") {
                std::string sessionName = args.size() > 1 ? args[1] : "session-" + std::to_string(time(nullptr));
                if (recorder.startRecording(sessionName)) {
                    return 0;
                } else {
                    return 1;
                }
            } else if (args[0] == "stop") {
                if (recorder.stopRecording()) {
                    return 0;
                } else {
                    return 1;
                }
            }
        }
        
        else if (command == "playback") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt playback <session-name> [--interactive]" << std::endl;
                return 1;
            }
            
            if (recorder.playSession(args[0])) {
                return 0;
            } else {
                return 1;
            }
        }
        
        // ============== ENHANCED FEATURES ==============
        else if (command == "comment") {
            if (args.size() < 3 || args[0] != "add") {
                std::cerr << "Usage: gyatt comment add <file:line> <message>" << std::endl;
                return 1;
            }
            
            std::string location = args[1];
            std::string message = args[2];
            
            size_t colonPos = location.find(':');
            if (colonPos == std::string::npos) {
                std::cerr << "Invalid location format. Use file:line" << std::endl;
                return 1;
            }
            
            std::string filepath = location.substr(0, colonPos);
            int lineNumber = std::stoi(location.substr(colonPos + 1));
            std::string author = "current-user"; 
            
            if (comments.addComment(filepath, lineNumber, message, author)) {
                std::cout << ui.colorize(!β Comment added", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize(!α Failed to add comment", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "sticky-note") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt sticky-note <message>" << std::endl;
                return 1;
            }
            
            std::string message = args[0];
            if (stickyNotes.addNote(message)) {
                std::cout << ui.colorize(!α Sticky note added", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize(!α Failed to add sticky note", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "label") {
            if (args.size() < 2) {
                std::cerr << "Usage: gyatt label <label-name> <file>" << std::endl;
                return 1;
            }
            
            if (labels.addFileLabel(args[0], args[1])) {
                std::cout << ui.colorize(!β File labeled", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize("α Failed to label file", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        // ============== STORY MODE ==============
        else if (command == "story-mode") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt story-mode <enable|disable|status>" << std::endl;
                return 1;
            }
            
            if (args[0] == "enable") {
                storyMode.enableStoryMode();
                std::cout << ui.colorize(!β Story mode enabled", ui.Color::GREEN) << std::endl;
            } else if (args[0] == "disable") {
                storyMode.disableStoryMode();
                std::cout << ui.colorize(!α Story mode disabled", ui.Color::YELLOW) << std::endl;
            } else if (args[0] == "status") {
                std::cout << "Story mode: " << "available" << std::endl;
            }
            return 0;
        }
        
        // ============== ALIAS MANAGEMENT ==============
        else if (command == "alias") {
            if (args.empty()) {
                aliases.showAliasHelp();
                return 0;
            } else if (args[0] == "add" && args.size() >= 3) {
                aliases.addCustomAlias(args[1], args[2]);
                std::cout << ui.colorize(!α Alias added: " + args[1] + " -> " + args[2], ui.Color::GREEN) << std::endl;
                return 0;
            } else if (args[0] == "remove" && args.size() >= 2) {
                aliases.removeAlias(args[1]);
                std::cout << ui.colorize("α Alias removed: " + args[1], ui.Color::GREEN) << std::endl;
                return 0;
            } else if (args[0] == "list") {
                auto aliasList = aliases.getAllAliases();
                for (const auto& pair : aliasList) {
                    std::cout << ui.colorize(pair.first, ui.Color::CYAN) << " -> " 
                             << ui.colorize(pair.second, ui.Color::GREEN) << std::endl;
                }
                return 0;
            }
        }
        
        // ============== REMAINING CLASSIC COMMANDS ==============
        else if (command == "push") {
            std::string remoteName = args.size() > 0 ? args[0] : "origin";
            std::string branchName = args.size() > 1 ? args[1] : "";
            
            if (repo.push(remoteName, branchName)) {
                return 0;
            } else {
                std::cerr << "Failed to push" << std::endl;
                return 1;
            }
        }
        
        else if (command == "remote") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt remote add <name> <url> | gyatt remote -v | gyatt remote --enhanced" << std::endl;
                return 1;
            }
            
            if (args[0] == "add") {
                if (args.size() < 3) {
                    std::cerr << "Usage: gyatt remote add <name> <url>" << std::endl;
                    return 1;
                }
                
                if (repo.addRemote(args[1], args[2])) {
                    return 0;
                } else {
                    std::cerr << "Failed to add remote" << std::endl;
                    return 1;
                }
            }
            else if (args[0] == "-v") {
                repo.printRemotes();
                return 0;
            }
            else if (args[0] == "--enhanced") {
                auto remotes = repo.getRemoteRepositories();
                std::cout << ui.colorize(!ο Enhanced Remote Repository Information", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                for (const auto& remote : remotes) {
                    std::cout << "\n" << ui.colorize("Remote: " + remote.name, ui.Color::GREEN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                    std::cout << "  URL: " << remote.url << std::endl;
                    std::cout << "  Protocol: " << repo.getProtocolName(remote.protocol) << std::endl;
                    std::cout << "  Auth: " << repo.getAuthMethodName(remote.credentials.method) << std::endl;
                    std::cout << "  Health: " << (remote.isHealthy ? !α Healthy" : !β Unhealthy") << std::endl;
                    auto syncTime = std::chrono::system_clock::to_time_t(remote.lastSync);
                    if (syncTime > 0) {
                        std::cout << "  Last Sync: " << std::ctime(&syncTime);
                    } else {
                        std::cout << "  Last Sync: Never" << std::endl;
                    }
                }
                return 0;
            }
            else {
                std::cerr << "Unknown remote command: " << args[0] << std::endl;
                return 1;
            }
        }
        
        // ============== ENHANCED REMOTE REPOSITORY MANAGEMENT ==============
        else if (command == "remote-add") {
            if (args.size() < 2) {
                std::cerr << "Usage: gyatt remote-add <name> <url> [--auth <method>] [--token <token>] [--ssh-key <path>]" << std::endl;
                return 1;
            }
            
            std::string name = args[0];
            std::string url = args[1];
            
            gyatt::RemoteCredentials credentials;
            credentials.method = gyatt::AuthMethod::NONE;
            
            for (size_t i = 2; i < args.size(); i += 2) {
                if (i + 1 < args.size()) {
                    if (args[i] == "--auth") {
                        std::string method = args[i + 1];
                        if (method == "token") {
                            credentials.method = gyatt::AuthMethod::TOKEN;
                        } else if (method == "ssh") {
                            credentials.method = gyatt::AuthMethod::SSH_KEY;
                        } else if (method == "password") {
                            credentials.method = gyatt::AuthMethod::USERNAME_PASSWORD;
                        }
                    } else if (args[i] == "--token") {
                        credentials.token = args[i + 1];
                        credentials.method = gyatt::AuthMethod::TOKEN;
                    } else if (args[i] == "--ssh-key") {
                        credentials.sshKeyPath = args[i + 1];
                        credentials.method = gyatt::AuthMethod::SSH_KEY;
                    } else if (args[i] == "--username") {
                        credentials.username = args[i + 1];
                        credentials.method = gyatt::AuthMethod::USERNAME_PASSWORD;
                    }
                }
            }
            
            if (repo.addRemoteWithAuth(name, url, credentials)) {
                std::cout << ui.colorize(!α Enhanced remote added successfully", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize("α Failed to add enhanced remote", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "remote-health") {
            if (args.empty()) {
                auto remotes = repo.listRemotes();
                std::cout << ui.colorize(!π₯ Remote Health Check", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                for (const auto& remote : remotes) {
                    bool healthy = repo.checkRemoteHealth(remote);
                    std::string status = healthy ? "α Healthy" : !α Unhealthy";
                    std::cout << remote.name << ": " << status << std::endl;
                }
            } else {
                auto remote = repo.loadRemoteConfig(args[0]);
                if (remote.name.empty()) {
                    std::cerr << "Remote '" << args[0] << "' not found" << std::endl;
                    return 1;
                }
                
                bool healthy = repo.checkRemoteHealth(remote);
                std::string status = healthy ? !β Healthy" : "α Unhealthy";
                std::cout << "Remote '" << args[0] << "': " << status << std::endl;
            }
            return 0;
        }
        
        else if (command == "push-enhanced") {
            std::string remoteName = args.size() > 0 ? args[0] : "origin";
            std::string branchName = args.size() > 1 ? args[1] : "main";
            
            std::cout << ui.colorize(!π Enhanced Push with Progress Tracking", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
            
            bool success = repo.pushWithProgress(remoteName, branchName, 
                [&ui](const gyatt::PushProgress& progress) {
                    std::cout << "\r" << ui.colorize(progress.phase, ui.Color::YELLOW) 
                             << ": " << progress.message 
                             << " [" << progress.current << "/" << progress.total << "]" << std::flush;
                });
            
            std::cout << std::endl; // New line after progress
            
            if (success) {
                std::cout << ui.colorize(!α Enhanced push completed successfully!", ui.Color::GREEN) << std::endl;
                return 0;
            } else {
                std::cerr << ui.colorize("α Enhanced push failed", ui.Color::RED) << std::endl;
                return 1;
            }
        }
        
        else if (command == "sync-profile") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt sync-profile create <name> | list | apply <name>" << std::endl;
                return 1;
            }
            
            if (args[0] == "create" && args.size() >= 2) {
                std::string profileName = args[1];
                std::vector<std::string> includePaths = {"src/", "include/", "*.cpp", "*.h"};
                std::vector<std::string> excludePaths = {"build/", "*.o", "*.tmp"};
                
                auto profile = repo.createSyncProfile(profileName, gyatt::SyncMode::SELECTIVE, includePaths, excludePaths);
                std::cout << ui.colorize(!β Sync profile '" + profileName + "' created", ui.Color::GREEN) << std::endl;
                return 0;
            }
            else if (args[0] == "list") {
                auto profiles = repo.getSyncProfiles();
                std::cout << ui.colorize(!ο Sync Profiles", ui.Color::CYAN, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
                for (const auto& profile : profiles) {
                    std::cout << "  " << ui.colorize(profile.name, ui.Color::GREEN) 
                             << " (" << repo.getSyncModeName(profile.mode) << ")" << std::endl;
                }
                return 0;
            }
            else {
                std::cerr << "Unknown sync-profile command: " << args[0] << std::endl;
                return 1;
            }
        }
        
        else if (command == "show") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt show <object>" << std::endl;
                return 1;
            }
            
            if (repo.show(args[0])) {
                return 0;
            } else {
                std::cerr << "Failed to show object: " << args[0] << std::endl;
                return 1;
            }
        }
        
        else if (command == "github-token") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt github-token <token>" << std::endl;
                std::cerr << "Use 'gyatt github-token clear' to remove the token" << std::endl;
                return 1;
            }
            
            std::string token = args[0];
            if (token == "clear") {
                token = "";
            }
            
            if (repo.setGitHubToken(token)) {
                if (token.empty()) {
                    std::cout << "GitHub token cleared" << std::endl;
                } else {
                    std::cout << "GitHub token set successfully" << std::endl;
                }
                return 0;
            } else {
                std::cerr << "Failed to set GitHub token" << std::endl;
                return 1;
            }
        }
        
        else if (command == "gyattignore") {
            if (repo.createIgnoreFile()) {
                std::cout << "Created .gyattignore file in " << repo.getRepoPath() << std::endl;
                std::cout << "This file specifies patterns for files that gyatt should ignore." << std::endl;
                return 0;
            } else {
                std::cerr << "Failed to create .gyattignore file" << std::endl;
                return 1;
            }
        }
        
        else if (command == "check-ignore") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt check-ignore <file>" << std::endl;
                return 1;
            }
            
            std::string filepath = args[0];
            if (repo.isIgnored(filepath)) {
                std::cout << filepath << " is ignored" << std::endl;
            } else {
                std::cout << filepath << " is not ignored" << std::endl;
            }
            return 0;
        }
        
        else if (command == "add-ignore") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt add-ignore <pattern>" << std::endl;
                return 1;
            }
            
            std::string pattern = args[0];
            try {
                if (repo.addIgnorePattern(pattern)) {
                    std::cout << "Added pattern to .gyattignore: " << pattern << std::endl;
                    return 0;
                } else {
                    std::cerr << "Failed to add pattern to .gyattignore" << std::endl;
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return 1;
            }
        }
        
        else {
            std::cerr << ui.colorize(!π Unknown command: " + command, ui.Color::RED, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
            std::cerr << ui.colorize(!π‘ Use 'gyatt help' for usage information or 'gyatt alias list' for available aliases.", 
                                   ui.Color::YELLOW) << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << ui.colorize(!ο₯ Error: " + std::string(e.what()), ui.Color::RED, ui.Color::BLACK, ui.Style::BOLD) << std::endl;
        return 1;
    }
    
    return 0;
}
