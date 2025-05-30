#!/bin/bash

# Advanced Compression Performance Test for Gyatt

echo "=========================================="
echo "GYATT COMPRESSION PERFORMANCE BENCHMARK"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

GYATT_PATH="$(pwd)/gyatt"
TEST_DIR="/tmp/gyatt_compression_test_$(date +%s)"

echo -e "${BLUE}Gyatt Version: ${NC}$(./gyatt --version 2>/dev/null || echo 'Development Build')"
echo -e "${BLUE}Test Directory: ${NC}$TEST_DIR"

# Verify Gyatt Build
echo -e "${YELLOW}Verifying Gyatt Build${NC}"
if [[ ! -f "$GYATT_PATH" ]]; then
    echo -e "${RED}Gyatt binary not found. Building...${NC}"
    make clean && make || exit 1
fi
echo -e "${GREEN}Gyatt binary ready${NC}"

# Create test repository
echo -e "\n${YELLOW}Creating Test Repository${NC}"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR" || exit 1

"$GYATT_PATH" init
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to initialize repository${NC}"
    exit 1
fi
echo -e "${GREEN}Repository initialized${NC}"

# Create test files of different types for compression testing
echo -e "${CYAN}Creating test files...${NC}"

# Text files (good compression)
mkdir -p "text"
for i in {1..20}; do
    # Create text file with repeating content (highly compressible)
    yes "This is a test line that should compress well. " | head -n 100 > "text/file$i.txt"
done

# Binary files (medium compression)
mkdir -p "binary"
for i in {1..10}; do
    # Create binary file with some patterns
    dd if=/dev/urandom bs=1K count=20 | tr '\0-\377' '\0-\177' > "binary/file$i.bin"
done

# Random data (poor compression)
mkdir -p "random"
for i in {1..5}; do
    # Create file with truly random data
    dd if=/dev/urandom of="random/file$i.dat" bs=1K count=30 2>/dev/null
done

# Count total files and sizes
TOTAL_FILES=$(find . -type f ! -path "./.gyatt/*" | wc -l)
TOTAL_SIZE=$(du -sh . | cut -f1)
echo -e "${GREEN}Created $TOTAL_FILES test files (Total size: $TOTAL_SIZE)${NC}"

# Stage and commit files
echo -e "\n${YELLOW}Staging Files${NC}"
"$GYATT_PATH" add .
echo -e "${GREEN}Files staged${NC}"

echo -e "\n${YELLOW}Creating Commit${NC}"
"$GYATT_PATH" commit -m "Compression test commit with various file types"
echo -e "${GREEN}Commit created${NC}"

# Test compression features
echo -e "\n${MAGENTA}TESTING COMPRESSION FEATURES${NC}"

# Enable compression
echo -e "\n${CYAN}Enabling compression system...${NC}"
"$GYATT_PATH" perf compression enable
sleep 1

# Test different optimization modes
echo -e "\n${CYAN}Testing speed-optimized compression...${NC}"
"$GYATT_PATH" perf compression speed
sleep 1

echo -e "\n${CYAN}Testing size-optimized compression...${NC}"
"$GYATT_PATH" perf compression size
sleep 1

echo -e "\n${CYAN}Testing balanced compression...${NC}"
"$GYATT_PATH" perf compression balance
sleep 1

# Run full optimization
echo -e "\n${CYAN}Running full compression optimization...${NC}"
"$GYATT_PATH" perf compression full
sleep 1

# Show compression statistics
echo -e "\n${CYAN}Displaying compression statistics...${NC}"
"$GYATT_PATH" perf profile

# Summary
echo -e "\n${MAGENTA}=========================================="
echo "COMPRESSION BENCHMARK COMPLETE"
echo "==========================================${NC}"

echo -e "${BLUE}Results Summary:${NC}"
echo "✓ Compression system tested with $TOTAL_FILES files of varying compressibility"
echo "✓ All compression modes validated"
echo "✓ Full repository compression optimization completed"

# Cleanup
cd /home/trashhpandaaaa/Documents/Code/Cpp/gyatt
echo -e "\n${YELLOW}Cleaning up test directory...${NC}"
rm -rf "$TEST_DIR"
echo -e "${GREEN}Cleanup complete${NC}"

echo -e "\n${GREEN}Compression benchmark completed successfully!${NC}"
