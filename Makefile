# Gyatt Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = 

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/utils.c \
          $(SRC_DIR)/commands/init.c \
          $(SRC_DIR)/commands/add.c \
          $(SRC_DIR)/commands/commit.c \
          $(SRC_DIR)/commands/status.c \
          $(SRC_DIR)/commands/log.c \
          $(SRC_DIR)/commands/branch.c \
          $(SRC_DIR)/commands/checkout.c \
          $(SRC_DIR)/commands/push.c \
          $(SRC_DIR)/commands/pull.c \
          $(SRC_DIR)/commands/server.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/gyatt

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/gyatt.exe
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
    FIXPATH = $(subst /,\,$1)
else
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
    FIXPATH = $1
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo Build complete: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@$(MKDIR) $(call FIXPATH,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	@$(MKDIR) $(call FIXPATH,$(BIN_DIR))

$(BUILD_DIR):
	@$(MKDIR) $(call FIXPATH,$(BUILD_DIR))
	@$(MKDIR) $(call FIXPATH,$(BUILD_DIR)/commands)

clean:
ifeq ($(OS),Windows_NT)
	@if exist $(call FIXPATH,$(BUILD_DIR)) $(RMDIR) $(call FIXPATH,$(BUILD_DIR))
	@if exist $(call FIXPATH,$(BIN_DIR)) $(RMDIR) $(call FIXPATH,$(BIN_DIR))
else
	@$(RMDIR) $(BUILD_DIR) $(BIN_DIR)
endif
	@echo Clean complete

run: $(TARGET)
	@$(TARGET)

help:
	@echo Gyatt Build System
	@echo.
	@echo Targets:
	@echo   all     - Build the project (default)
	@echo   clean   - Remove build artifacts
	@echo   run     - Build and run gyatt
	@echo   help    - Show this help message
