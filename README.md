# Gyatt - A Git Implementation in C++

Gyatt is a custom implementation of Git version control system written in C++. It implements core Git functionality including repositories, commits, branches, and basic file tracking.

## Features

- Initialize repositories (`gyatt init`)
- Add files to staging area (`gyatt add`)
- Commit changes (`gyatt commit`)
- View repository status (`gyatt status`)
- View commit history (`gyatt log`)
- Create and switch branches (`gyatt branch`, `gyatt checkout`)
- Show differences (`gyatt diff`)
- View file contents (`gyatt show`)

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
```

## Architecture

The project is structured as follows:

- `src/main.cpp` - Entry point and command parsing
- `src/repository.cpp` - Repository management
- `src/commit.cpp` - Commit handling
- `src/index.cpp` - Staging area management
- `src/object.cpp` - Git object handling
- `src/utils.cpp` - Utility functions
- `include/` - Header files

## License

MIT License
