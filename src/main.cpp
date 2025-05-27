#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "repository.h"
#include "utils.h"

void printUsage() {
    std::cout << "Gyatt - A Git Implementation in C++\n\n";
    std::cout << "Usage: gyatt <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  init                     Initialize a new repository\n";
    std::cout << "  add <file>              Add file to staging area\n";
    std::cout << "  commit -m <message>     Commit staged changes\n";
    std::cout << "  status                  Show repository status\n";
    std::cout << "  log                     Show commit history\n";
    std::cout << "  diff                    Show differences\n";
    std::cout << "  branch [name]           List or create branches\n";
    std::cout << "  checkout <branch>       Switch to a branch\n";
    std::cout << "  show <object>           Show object content\n";
    std::cout << "  help                    Show this help message\n";
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
    
    try {
        if (command == "init") {
            if (repo.init()) {
                std::cout << "Initialized empty Gyatt repository in " << repo.getRepoPath() << "/.gyatt\n";
                return 0;
            } else {
                std::cerr << "Failed to initialize repository\n";
                return 1;
            }
        }
        
        else if (command == "help" || command == "--help" || command == "-h") {
            printUsage();
            return 0;
        }
        
        // For all other commands, we need an existing repository
        if (!repo.isRepository()) {
            std::cerr << "fatal: not a gyatt repository (or any of the parent directories): .gyatt\n";
            return 1;
        }
        
        if (command == "add") {
            if (args.empty()) {
                std::cerr << "Nothing specified, nothing added.\n";
                return 1;
            }
            
            for (const auto& file : args) {
                if (!repo.add(file)) {
                    std::cerr << "Failed to add file: " << file << "\n";
                    return 1;
                }
            }
            return 0;
        }
        
        else if (command == "commit") {
            if (args.size() < 2 || args[0] != "-m") {
                std::cerr << "Usage: gyatt commit -m <message>\n";
                return 1;
            }
            
            std::string message = args[1];
            if (repo.commit(message)) {
                return 0;
            } else {
                std::cerr << "Failed to create commit\n";
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
                    std::cout << "Created branch: " << args[0] << "\n";
                } else {
                    std::cerr << "Failed to create branch: " << args[0] << "\n";
                    return 1;
                }
            }
            return 0;
        }
        
        else if (command == "checkout") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt checkout <branch>\n";
                return 1;
            }
            
            if (repo.checkout(args[0])) {
                std::cout << "Switched to branch '" << args[0] << "'\n";
            } else {
                std::cerr << "Failed to checkout branch: " << args[0] << "\n";
                return 1;
            }
            return 0;
        }
        
        else if (command == "show") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt show <object>\n";
                return 1;
            }
            
            if (repo.show(args[0])) {
                return 0;
            } else {
                std::cerr << "Failed to show object: " << args[0] << "\n";
                return 1;
            }
        }
        
        else {
            std::cerr << "Unknown command: " << command << "\n";
            std::cerr << "Use 'gyatt help' for usage information.\n";
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
