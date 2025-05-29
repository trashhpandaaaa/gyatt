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
```

## Usage

```bash
# Initialize a new repository
gyatt init

# Add files to staging area
gyatt add <file>
gyatt add .

# Commit changes
gyatt commit -m "Your commit message"

# View status
gyatt status

# View commit history
gyatt log

# Create a new branch
gyatt branch <branch-name>

# Switch to a branch
gyatt checkout <branch-name>

# Show differences
gyatt diff

# Show file content
gyatt show <commit-hash>:<file>

# Create .gyattignore file
gyatt gyattignore

# Check if a file is ignored
gyatt check-ignore <file>

# Add a pattern to .gyattignore
gyatt add-ignore <pattern>

# Clone a repository (local or GitHub)
gyatt clone <url> [directory]

# Add a remote repository
gyatt remote add <name> <url>

# List remote repositories
gyatt remote -v

# Push changes to a remote repository
gyatt push [remote] [branch]

# Set GitHub personal access token
gyatt github-token <token>
```

## GitHub Integration

Gyatt includes support for GitHub repositories:

1. **Cloning GitHub repositories**:
   ```bash
   gyatt clone https://github.com/username/repo.git
   ```

2. **Setting up GitHub authentication**:
   ```bash
   gyatt github-token <your-github-token>
   ```
   To create a token, visit: https://github.com/settings/tokens

3. **Pushing to GitHub**:
   ```bash
   gyatt push origin main
   ```

## Ignore System

Gyatt provides a `.gyattignore` system similar to Git's `.gitignore`:

1. A default `.gyattignore` file is created automatically when initializing a repository
2. Use `gyatt gyattignore` to create a default ignore file if one doesn't exist
3. Use `gyatt check-ignore <file>` to check if a file is ignored
4. Use `gyatt add-ignore <pattern>` to add patterns to the ignore file

Default ignored patterns include:
- `.gyatt/` directory
- Compiled files (*.o, *.exe, etc.)
- Build directories (build/, bin/, lib/)
- Log and database files
- OS-specific files

## Architecture

The project is structured as follows:

- `src/main.cpp` - Entry point and command parsing
- `src/repository.cpp` - Repository management
- `src/commit.cpp` - Commit handling
- `src/index.cpp` - Staging area management
- `src/object.cpp` - Git object handling
- `src/utils.cpp` - Utility functions
- `src/ignore.cpp` - Ignore file handling
- `include/` - Header files

### Core Components

1. **Repository**: Manages the overall repository structure and operations
2. **Index**: Handles the staging area and tracking of file changes
3. **Commit**: Creates and manages commit objects
4. **Object**: Handles Git-style object storage (blobs, trees, commits)
5. **Utils**: Provides utility functions for file operations, hashing, etc.
6. **Ignore**: Manages the `.gyattignore` system for ignoring files

## License

MIT License

## Contributing

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

