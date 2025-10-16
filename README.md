# Gyatt - A Better Version Control System

Gyatt is a modern, fast, and feature-rich version control system built from scratch in C. It's designed to be better than Git with innovative features and superior performance.

## 🚀 Features

- **Lightning Fast**: Optimized C implementation for maximum performance
- **Built-in Private Server**: Host your own repositories without external services
- **Git Compatible**: Push/pull to GitHub, GitLab, Bitbucket, etc.
- **Better UX**: Intuitive commands and helpful error messages
- **Team Collaboration**: Real-time notifications and presence awareness
- **Cross-Platform**: Works on Windows, Linux, and macOS

## 📦 Building

### Prerequisites
- GCC or Clang compiler
- Make

### Build Instructions

```bash
# Build the project
make

# Clean build artifacts
make clean

# Build and run
make run
```

## 🎯 Usage

```bash
# Initialize a new repository
gyatt init

# Add files to staging
gyatt add <file>

# Commit changes
gyatt commit -m "Your message"

# View status
gyatt status

# View commit history
gyatt log

# Branching
gyatt branch <name>
gyatt checkout <branch>

# Remote operations
gyatt push <remote>
gyatt pull <remote>

# Start server mode
gyatt server start
```

## 🏗️ Development Progress

This project is being built in 10 parts:

- [x] Part 1: Project setup and basic structure
- [ ] Part 2: Core data structures and utilities
- [ ] Part 3: Repository initialization
- [ ] Part 4: Object storage system
- [ ] Part 5: Index/staging area
- [ ] Part 6: Commit functionality
- [ ] Part 7: Status and diff
- [ ] Part 8: Branch management
- [ ] Part 9: Network protocol foundation
- [ ] Part 10: Push/Pull with private server

## 📝 License

MIT License - See LICENSE file for details

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## ⚡ Why Gyatt?

- **Private by default**: Host on your own infrastructure
- **Faster operations**: Optimized algorithms and data structures
- **Better collaboration**: Built-in team features
- **Modern architecture**: Designed for today's workflows
- **Git interoperability**: Don't abandon your existing repositories

---

Built with ❤️ in C
