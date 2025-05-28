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
    std::cout << "  clone <url> [dir]       Clone a repository\n";
    std::cout << "  add <file>              Add file to staging area\n";
    std::cout << "  commit -m <message>     Commit staged changes\n";
    std::cout << "  status                  Show repository status\n";
    std::cout << "  log                     Show commit history\n";
    std::cout << "  diff                    Show differences\n";
    std::cout << "  branch [name]           List or create branches\n";
    std::cout << "  checkout <branch>       Switch to a branch\n";
    std::cout << "  push [remote] [branch]  Push changes to remote\n";
    std::cout << "  remote add <n> <url> Add a remote repository\n";
    std::cout << "  remote -v               List remote repositories\n";
    std::cout << "  show <object>           Show object content\n";
    std::cout << "  github-token <token>    Set GitHub personal access token\n";
    std::cout << "  gyattignore              Create a default .gyattignore file\n";
    std::cout << "  check-ignore <file>      Check if a file is ignored\n";
    std::cout << "  add-ignore <pattern>     Add a pattern to .gyattignore\n";
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
        
        else if (command == "clone") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt clone <repository> [directory]\n";
                return 1;
            }
            
            std::string sourceUrl = args[0];
            std::string targetDir = args.size() > 1 ? args[1] : "";
            
            gyatt::Repository cloneRepo(".");
            if (cloneRepo.clone(sourceUrl, targetDir)) {
                return 0;
            } else {
                std::cerr << "Failed to clone repository\n";
                return 1;
            }
        }
        
        else if (command == "help" || command == "--help" || command == "-h") {
            printUsage();
            return 0;
        }
        
        if (!repo.isRepository()) {
            std::cerr << "fatal: not a gyatt repository (or any of the parent directories): .gyatt\n";
            return 1;
        }
        
        if (command == "add" || command == "yeet") {
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
        
        else if (command == "commit" || command == "fr") {
            if (args.size() < 2 || args[0] != "-m") {
                std::cerr << "Usage: gyatt " << command << " -m <message>\n";
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
        
        else if (command == "push") {
            std::string remoteName = args.size() > 0 ? args[0] : "origin";
            std::string branchName = args.size() > 1 ? args[1] : "";
            
            if (repo.push(remoteName, branchName)) {
                return 0;
            } else {
                std::cerr << "Failed to push\n";
                return 1;
            }
        }
        
        else if (command == "remote") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt remote add <n> <url> | gyatt remote -v\n";
                return 1;
            }
            
            if (args[0] == "add") {
                if (args.size() < 3) {
                    std::cerr << "Usage: gyatt remote add <n> <url>\n";
                    return 1;
                }
                
                if (repo.addRemote(args[1], args[2])) {
                    return 0;
                } else {
                    std::cerr << "Failed to add remote\n";
                    return 1;
                }
            }
            else if (args[0] == "-v") {
                repo.listRemotes();
                return 0;
            }
            else {
                std::cerr << "Unknown remote command: " << args[0] << "\n";
                return 1;
            }
        }
        
        else if (command == "github-token") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt github-token <token>\n";
                std::cerr << "Use 'gyatt github-token clear' to remove the token\n";
                return 1;
            }
            
            std::string token = args[0];
            if (token == "clear") {
                token = "";
            }
            
            if (repo.setGitHubToken(token)) {
                if (token.empty()) {
                    std::cout << "GitHub token cleared\n";
                } else {
                    std::cout << "GitHub token set successfully\n";
                }
                return 0;
            } else {
                std::cerr << "Failed to set GitHub token\n";
                return 1;
            }
        }
        
        else if (command == "gyattignore") {
            if (repo.createIgnoreFile()) {
                std::cout << "Created .gyattignore file in " << repo.getRepoPath() << "\n";
                std::cout << "This file specifies patterns for files that gyatt should ignore.\n";
                return 0;
            } else {
                std::cerr << "Failed to create .gyattignore file\n";
                return 1;
            }
        }
        
        else if (command == "check-ignore") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt check-ignore <file>\n";
                return 1;
            }
            
            std::string filepath = args[0];
            if (repo.isIgnored(filepath)) {
                std::cout << filepath << " is ignored\n";
            } else {
                std::cout << filepath << " is not ignored\n";
            }
            return 0;
        }
        
        else if (command == "add-ignore") {
            if (args.empty()) {
                std::cerr << "Usage: gyatt add-ignore <pattern>\n";
                return 1;
            }
            
            std::string pattern = args[0];
            try {
                if (repo.addIgnorePattern(pattern)) {
                    std::cout << "Added pattern to .gyattignore: " << pattern << "\n";
                    return 0;
                } else {
                    std::cerr << "Failed to add pattern to .gyattignore\n";
                    return 1;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
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
