# Gyatt - A Git Implementation in C++

Gyatt is a custom implementation of Git version control system written in C++. It implements core Git functionality including repositories, commits, branches, basic file tracking, and remote repository support with GitHub integration.

## Features

- Initialize repositories (`gyatt init`)
- Add files to staging area (`gyatt add`)
- Commit changes (`gyatt commit`)
- View repository status (`gyatt status`)
- View commit history (`gyatt log`)
- Create and switch branches (`gyatt branch`, `gyatt checkout`)
- Show differences (`gyatt diff`)
- View file contents (`gyatt show`)
- Ignore files with `.gyattignore` (similar to `.gitignore`)
- Clone repositories (local and GitHub)
- Push changes to remote repositories
- GitHub integration for remote operations

## Dependencies

- C++17 or later
- CMake 3.15 or later
- OpenSSL
- libcurl

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Installation

```bash
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

## Future Improvements

- Full GitHub API support for pushing changes
- Enhanced commit history visualization
- Support for more complex merge operations
- Git LFS-like functionality for large files
- More comprehensive diff functionality
