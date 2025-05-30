#𞓥 Gyatt - The BRUTAL Version Control System

Gyatt is a blazingly fast, modern version control system that reimagines the development workflow with **pure chaos energy**. Built in C++17, Gyatt delivers Git's core functionality while adding revolutionary features that make version control actually enjoyable and powerful.

##ᚡ Performance & Reliability
- **Lightning-fast C++17 implementation** with optimized file operations
- **Enhanced GitHub integration** with robust error handling and file filtering
- **Multi-protocol support** (HTTP/HTTPS/SSH/Local) with automatic protocol detection
- **Advanced authentication** including token-based, SSH keys, and OAuth support

##� Enhanced Remote Repository System

### � Advanced Remote Management
- **Multi-protocol support** with automatic detection (HTTP/HTTPS/SSH/Local)
- **Enhanced authentication** with token, SSH key, and OAuth support
- **Remote health monitoring** with connection testing and diagnostics
- **Progress tracking** for push operations with real-time feedback

###� Synchronization Profiles
- **Custom sync profiles** with configurable modes (FULL/SELECTIVE/INCREMENTAL/SMART)
- **Automated synchronization** with interval-based scheduling
- **Conflict detection** and resolution strategies
- **Profile persistence** with JSON-based configuration storage

### � Enhanced CLI Commands
```bash
# Add remote with authentication
gyatt remote-add origin https://github.com/user/repo.git --auth token --token YOUR_TOKEN

# Check remote health
gyatt remote-health origin

# Enhanced push with progress tracking
gyatt push-enhanced origin main

# Create and manage sync profiles
gyatt sync-profile create daily-sync --mode INCREMENTAL --interval 60
gyatt sync-profile list
gyatt sync-profile apply daily-sync
```

##𞍯 Core Features

###𞒂 Human-Readable Commit Logs
- Markdown-powered commits with rich formatting
- Emoji support and code blocks
- Commit logs that read like a development diary

### � Semantic Branching System
- Automatic naming conventions (`gyatt start feature/login`)
- Auto-linked TODO.md
- Merge tags and branch summaries

### ᛂ Section-Based Staging
- Stage code by function, class, or logical section
- Smarter control than line-by-line hunking

###�﷏ Project Map Generator
- Visual or textual file hierarchy
- Function maps and dependency graphs
- Test coverage visualization

###ᙺ Branch Loopback
- Merge select commits from child branches back to parent
- Simpler than Git's cherry-pick operations

###𞓖 Checkpoint Snapshots
- Tag moments in time without making a commit
- Diff against snapshots easily

### � Prebuilt Guardrails
- Prevents accidental main pushes
- Blocks pushing debug code
- Enforces format/lint rules (with overrides)

### ᙙ Inline Config Overrides
- Toggle settings with command flags
- `gyatt commit --no-verify --no-format`

### � Plugin Ecosystem
- Install community plugins: `gyatt install changelog-gen`
- Build your own tools (Python, Bash, Rust)

###� Interactive Commit Prompt
- Guided commit creation
- Category, scope, and description prompts

### � Containerized Dev Snapshots
- Store runtime info and configurations
- Setup scripts for reproducible environments

###� Session Recording
- Log command history and file diffs
- Replayable development sessions

###� Exportable Histories
- One-command export to CHANGELOG.md
- Visual timelines and PDF reports

###𞑬 In-Code Comment Threads
- Local threaded comments that persist between commits
- Like GitHub reviews, but in your local environment

### � Rewind Mode
- Roll back commits without losing changes
- `gyatt rewind 3 --soft`

###� Oops Shielc⃢
- Shadow backups for accidental deletions
- Restore deleted files after rage-quits

###� Custom Init Templates
- Project-specific initializations
- `gyatt init react` sets up folder structure and starter files

### �ဌ� Commit Story Mode
- Narrative-based commit organization
- Chapter-based development phases

### � VIBE COMMANDS (The Fun Stuff)
- **yeet** - Add files (no cap) 
- **regret** - Undo changes
- **vibe** - Status with style
- **summon** - Checkout branch
- **nocap** - Push (no lies)
- **slay** - Force push
- **flex** - Show off commits  
- **ghost** - Stash changes
- **fr** - Commit with attitude

###𞒇 Label-based File Tagging
- Tag files with custom labels
- Query changes by label: `gyatt log --label=core`

###𞒎 Sticky Notes System
- Inline notes that don't pollute code
- Terminal dashboard for note management

## � Quick Start

### Installation
```bash
# Clone and build
git clone https://github.com/username/gyatt.git
cd gyatt
make
```

### Basic Usage
```bash
# Initialize with chaos energy
gyatt init
# or use a template
gyatt damnit react

# Add files (with style)
gyatt yeet file.cpp
# or traditional
gyatt add file.cpp

# Commit with attitude
gyatt fr -m !ᜨ feat: added epic functionality"
# or interactive
gyatt commit-interactive

# Check the vibe
gyatt vibe
# or traditional
gyatt status

# Enhanced remote operations
gyatt remote-add origin https://github.com/user/repo.git --auth token --token YOUR_TOKEN
gyatt push-enhanced origin main
```

## � Advanced Features

### Enhanced Remote Repository Management
```bash
# Add authenticated remote
gyatt remote-add upstream https://github.com/original/repo.git --auth ssh-key --ssh-key ~/.ssh/id_rsa

# Health check remotes
gyatt remote-health --all

# Create sync profiles
gyatt sync-profile create daily --mode SMART --interval 30 --auto-sync

# Push with progress tracking
gyatt push-enhanced origin main --progress
```

### Interactive Development
```bash
# Start semantic branches
gyatt start feature/awesome-feature

# Interactive staging
gyatt stage-interactive

# Project mapping
gyatt map --visual

# Checkpoint management
gyatt mark milestone-v1.0
gyatt rewind 3 --soft
```

##� Technical Implementation

### Enhanced GitHub Integration
Gyatt implements robust GitHub integration with comprehensive error handling:

- **Smart file filtering** prevents upload of system files (`.git/`, `.gyatt/`, etc.)
- **Multi-protocol support** with automatic protocol detection and validation
- **Advanced authentication** supporting tokens, SSH keys, and OAuth
- **Connection health monitoring** with HTTP/SSH connection testing
- **Progress tracking** with real-time push operation feedback

### Architecture Overview
```
gyatt/	ⓛ�Ⓚ src/	Ⓜ  ⓛ�Ⓚ main.cpp              # CLI interface with enhanced commands	ᔂ  ᔛ�ᔀ repository.cpp        # Core repo management + enhanced remotes
ᓂ   ᓜᓀᓀ advanced_features.cpp # Enhanced remote repository system	Ⓜ  ⓛ�Ⓚ terminal_ui.cpp       # Neobrutalist terminal interface	Ⓜ  ⓓ�Ⓚ ...                   # Additional feature modules
ᓜᓀᓀ include/	ᔂ  ᔛ�ᔀ repository.h          # Enhanced with remote management structures
ᓂ   ᓔᓀᓀ ...                   # Feature headers	ⓓ�Ⓚ build/                    # Compiled objects and binary
```

### Key Components
1. **Enhanced Repository Management**: Core operations with advanced remote handling
2. **Multi-Protocol Support**: HTTP/HTTPS/SSH/Local with automatic detection
3. **Authentication System**: Token, SSH key, OAuth support with secure storage
4. **Progress Tracking**: Real-time feedback for long-running operations
5. **Sync Profiles**: Configurable synchronization with persistence
6. **Health Monitoring**: Connection testing and diagnostics

## � Remote Repository Features

### Authentication Methods
```bash
# Token-based authentication
gyatt remote-add origin https://github.com/user/repo.git --auth token --token YOUR_TOKEN

# SSH key authentication  
gyatt remote-add origin git@github.com:user/repo.git --auth ssh-key --ssh-key ~/.ssh/id_rsa

# Username/password authentication
gyatt remote-add origin https://github.com/user/repo.git --auth username-password
```

### Health Monitoring
```bash
# Check specific remote
gyatt remote-health origin

# Check all remotes
gyatt remote-health --all

# Detailed diagnostics
gyatt remote-health origin --verbose
```

### Synchronization Profiles
```bash
# Create profile with auto-sync
gyatt sync-profile create production --mode SMART --interval 15 --auto-sync

# List all profiles
gyatt sync-profile list

# Apply profile
gyatt sync-profile apply production

# Delete profile  
gyatt sync-profile delete production
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

##𞚠 Build & Development

### Dependencies
- **C++17 or later**
- **libcurl** for HTTP operations
- **OpenSSL** for cryptographic functions  
- **jsoncpp** for configuration management
- **CMake** or **Make** for building

### Building from Source
```bash
# Using Make (recommended)
git clone https://github.com/username/gyatt.git
cd gyatt
make clean && make

# Using CMake
mkdir build && cd build
cmake ..
make
```

### Development Testing
```bash
# Run the built binary
./gyatt --help

# Test basic functionality
./gyatt init test-repo
cd test-repo
echo "test" > file.txt
./gyatt yeet file.txt
./gyatt fr -m !� initial commit"
./gyatt vibe

# Test enhanced remote features
./gyatt remote-add test https://github.com/test/repo.git --auth token --token test123
./gyatt remote-health test
```

## � Architecture Details

### Enhanced Remote Repository System
The enhanced remote system includes:

**Core Structures:**
- `RemoteProtocol` enum: HTTP, HTTPS, SSH, LOCAL support
- `AuthMethod` enum: Multiple authentication strategies
- `RemoteCredentials` struct: Secure credential management
- `RemoteRepository` struct: Comprehensive remote tracking
- `SyncProfile` struct: Configurable synchronization settings
- `PushProgress` struct: Real-time operation tracking

**Key Methods:**
- `detectProtocol()`: Automatic protocol detection from URLs
- `validateRemoteUrl()`: URL validation and normalization
- `addRemoteWithAuth()`: Enhanced remote addition with authentication
- `checkRemoteHealth()`: Connection testing and diagnostics
- `pushWithProgress()`: Push operations with progress callbacks
- `createSyncProfile()`: Synchronization profile management

### File Filtering System
Gyatt implements smart file filtering to prevent uploading system files:
- Excludes `.git/`, `.gyatt/`, and system directories
- Filters common system files (`.DS_Store`, `Thumbs.db`, etc.)
- Ignores cache directories (`node_modules/`, `__pycache__/`, etc.)
- Prevents temporary and build artifacts from being uploaded

## � Why Gyatt?

### Developer Experience First
Gyatt isn't just another version control system - it's a **developer experience revolution**:

- **No more cryptic Git commands** - intuitive aliases that actually make sense
- **Enhanced remote management** with health monitoring and progress tracking  
- **Visual feedback** for all operations with real-time progress indicators
- **Smart automation** through sync profiles and checkpoint systems
- **Chaos energy** - because development should be fun, not frustrating

### Modern Architecture
- **C++17 performance** - blazingly fast operations
- **Multi-protocol support** - works with any remote repository
- **Advanced authentication** - secure, flexible credential management
- **Extensible design** - plugin ecosystem for custom workflows

### Built for Teams
- **Enhanced collaboration** with threaded comments and session recording
- **Project mapping** for better code understanding
- **Guardrails** to prevent common mistakes
- **Exportable histories** for documentation and reporting

##𞦪 Testing the System

To verify everything works correctly:

1. **Rebuild the project:**
```bash
mkdir -p build
cd build
cmake ..
make
```

2. **Test basic functionality:**
```bash
./gyatt --help
./gyatt init test-repo
cd test-repo
echo "console.log('Hello Gyatt!');" > app.js
./gyatt yeet app.js
./gyatt fr -m !𞙀 initial commit with style"
./gyatt vibe
```

3. **Test enhanced remote features:**
```bash
./gyatt remote-add origin https://github.com/user/repo.git --auth token --token YOUR_TOKEN
./gyatt remote-health origin
./gyatt push-enhanced origin main --progress
```

##� License

**MIT License** - Feel free to use Gyatt in your projects and contribute back to the community!

##� Contributing

We welcome contributions with **open arms and chaos energy**! 

### How to Contribute:
1. **Fork the repository** and create your feature branch
2. **Follow the coding style** - clean C++17 with meaningful comments
3. **Add tests** for new features using our test suite
4. **Update documentation** - keep the README fresh
5. **Submit a pull request** with a clear description

### Development Setup:
```bash
git clone https://github.com/username/gyatt.git
cd gyatt
make clean && make
./tests.sh  # Run the test suite
```

### Areas We Need Help:
-� **Performance optimizations** for large repositories
-� **Plugin ecosystem** development and documentation  
-� **UI/UX improvements** for terminal interface
- � **Cross-platform support** (Windows, macOS)
- � **Documentation** and tutorial creation

**Let's build the future of version control together!** �

