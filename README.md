# Gyatt - The Modern Version Control System

Gyatt is a modern, feature-rich version control system designed to make development more intuitive, fun, and powerful. With a focus on developer experience, Gyatt enhances the traditional Git workflow with advanced features like human-readable commit logs, semantic branching, and interactive interfaces.

## Core Features

### 📂 Human-Readable Commit Logs
- Markdown-powered commits with rich formatting
- Emoji support and code blocks
- Commit logs that read like a development diary

### 🌳 Semantic Branching System
- Automatic naming conventions (`gyatt start feature/login`)
- Auto-linked TODO.md
- Merge tags and branch summaries

### ✂️ Section-Based Staging
- Stage code by function, class, or logical section
- Smarter control than line-by-line hunking

### 🗺️ Project Map Generator
- Visual or textual file hierarchy
- Function maps and dependency graphs
- Test coverage visualization

### ♻️ Branch Loopback
- Merge select commits from child branches back to parent
- Simpler than Git's cherry-pick operations

### 🔖 Checkpoint Snapshots
- Tag moments in time without making a commit
- Diff against snapshots easily

### 🛡️ Prebuilt Guardrails
- Prevents accidental main pushes
- Blocks pushing debug code
- Enforces format/lint rules (with overrides)

### ⚙️ Inline Config Overrides
- Toggle settings with command flags
- `gyatt commit --no-verify --no-format`

### 🧩 Plugin Ecosystem
- Install community plugins: `gyatt install changelog-gen`
- Build your own tools (Python, Bash, Rust)

### 📋 Interactive Commit Prompt
- Guided commit creation
- Category, scope, and description prompts

### 📦 Containerized Dev Snapshots
- Store runtime info and configurations
- Setup scripts for reproducible environments

### 🎥 Session Recording
- Log command history and file diffs
- Replayable development sessions

### 📜 Exportable Histories
- One-command export to CHANGELOG.md
- Visual timelines and PDF reports

### 💬 In-Code Comment Threads
- Local threaded comments that persist between commits
- Like GitHub reviews, but in your local environment

### 🔁 Rewind Mode
- Roll back commits without losing changes
- `gyatt rewind 3 --soft`

### 💣 Oops Shield™
- Shadow backups for accidental deletions
- Restore deleted files after rage-quits

### 🧱 Custom Init Templates
- Project-specific initializations
- `gyatt init react` sets up folder structure and starter files

### 🧑‍🏫 Commit Story Mode
- Narrative-based commit organization
- Chapter-based development phases

### 📇 Label-based File Tagging
- Tag files with custom labels
- Query changes by label: `gyatt log --label=core`

### 📎 Sticky Notes System
- Inline notes that don't pollute code
- Terminal dashboard for note management

## Standard Version Control Features

- Initialize repositories (`gyatt init` or `gyatt damnit`)
- Add files to staging area (`gyatt add` or `gyatt yeet`)
- Commit changes (`gyatt commit` or `gyatt fr`)
- View repository status (`gyatt status`)
- View commit history (`gyatt log`)
- Branch management (`gyatt branch`, `gyatt checkout`)
- Show differences (`gyatt diff`)
- View file contents (`gyatt show`)
- Ignore files with `.gyattignore`
- Clone repositories (local and GitHub)
- Push changes to remote repositories
- GitHub integration for remote operations

## Installation

```bash
mkdir build
cd build
cmake ..
make
make install

Dependencies
C++17 or later
CMake 3.15 or later
OpenSSL
libcurl
jsoncpp



# Initialize a new repository with a project template
gyatt init react

# Create a semantic branch
gyatt start feature/login

# Add files with fun commands
gyatt yeet .

# Interactive commit with markdown
gyatt fr

# Show project map
gyatt map

# Create a development checkpoint
gyatt mark alpha-release

# Rewind without losing changes
gyatt rewind 3 --soft

# Tag files by category
gyatt label core src/core/*.cpp

# View files by label
gyatt log --label=core

# Add a sticky note
gyatt note "TODO: Refactor this section"

# Export development history
gyatt export changelog


Terminal UI Features
Gyatt includes a modern terminal UI with:

Animations and visual elements
Color schemes and theming
Interactive menus and wizards
Progress indicators
Contributing
Contributions are welcome! Feel free to submit pull requests or open issues for bugs and feature requests.

License
MIT License




## Testing the System

To verify everything works correctly:

1. Rebuild the project with the fixed files:

```bash
mkdir -p build
cd build
cmake ..
make

2. Run basic commands to ensure functionality:

./gyatt --help
./gyatt init test-repo
cd test-repo
touch test.txt
./gyatt add test.txt
./gyatt fr -m "Initial commit"
./gyatt status
./gyatt log

