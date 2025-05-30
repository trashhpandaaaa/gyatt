#!/bin/bash

# gyatt_test.sh - Unified Test Framework for Gyatt
# Provides a centralized interface for running various tests

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

GYATT_PATH="$(pwd)/gyatt"
TEST_DIR="/tmp/gyatt_test_$(date +%s)"

# Function to print usage information
print_usage() {
    echo -e "${CYAN}Gyatt Test Framework${NC}"
    echo -e "Usage: $0 [OPTION]"
    echo -e "Run various tests for the Gyatt version control system\n"
    echo -e "Options:"
    echo -e "  ${YELLOW}basic${NC}                Run basic functionality tests"
    echo -e "  ${YELLOW}compression${NC}          Test advanced compression performance"
    echo -e "  ${YELLOW}http${NC}                 Test HTTP optimization"
    echo -e "  ${YELLOW}parallel${NC}             Test parallel processing performance"
    echo -e "  ${YELLOW}memory${NC}               Test memory optimization"
    echo -e "  ${YELLOW}github${NC}               Test GitHub integration"
    echo -e "  ${YELLOW}all${NC}                  Run all tests"
    echo -e "  ${YELLOW}help${NC}                 Display this help message"
    echo -e "\nExample: $0 compression"
}

# Function to check if gyatt binary exists
check_gyatt_binary() {
    if [[ ! -f "$GYATT_PATH" ]]; then
        echo -e "${YELLOW}Building gyatt...${NC}"
        make clean && make -j8 || { 
            echo -e "${RED}Failed to build gyatt${NC}"
            exit 1
        }
    fi
    echo -e "${GREEN}✓ Gyatt binary ready${NC}"
}

# Function to run compression tests
run_compression_test() {
    echo -e "\n${MAGENTA}===== RUNNING COMPRESSION OPTIMIZATION TESTS =====${NC}"
    check_gyatt_binary

    # Create test repository
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR" || exit 1
    "$GYATT_PATH" init
    
    # Create test files
    for i in {1..50}; do
        echo "Test file $i content" > "file$i.txt"
    done
    
    # Add, commit, and run compression
    "$GYATT_PATH" add .
    "$GYATT_PATH" commit -m "Test commit for compression"
    
    # Test compression commands
    echo -e "\n${CYAN}Testing compression enable/disable${NC}"
    "$GYATT_PATH" perf compression enable
    "$GYATT_PATH" perf compression disable
    "$GYATT_PATH" perf compression enable
    
    echo -e "\n${CYAN}Testing compression optimization modes${NC}"
    "$GYATT_PATH" perf compression speed
    "$GYATT_PATH" perf compression size
    "$GYATT_PATH" perf compression balance
    
    echo -e "\n${CYAN}Testing full compression optimization${NC}"
    "$GYATT_PATH" perf compression full
    
    echo -e "\n${CYAN}Testing compression profiling${NC}"
    "$GYATT_PATH" perf profile
    
    # Cleanup
    cd - > /dev/null
    rm -rf "$TEST_DIR"
    echo -e "\n${GREEN}Compression tests completed successfully${NC}"
}

# Function to run HTTP optimization tests
run_http_test() {
    echo -e "\n${MAGENTA}===== RUNNING HTTP OPTIMIZATION TESTS =====${NC}"
    check_gyatt_binary
    
    # Run HTTP optimization test
    if [[ -f "test_http_optimization" ]]; then
        echo -e "${CYAN}Executing HTTP optimization test...${NC}"
        ./test_http_optimization
    else
        echo -e "${YELLOW}Building HTTP optimization test...${NC}"
        g++ -std=c++17 -o test_http_optimization test_http_optimization.cpp -lcurl -pthread
        ./test_http_optimization
    fi
    
    echo -e "\n${GREEN}HTTP optimization tests completed${NC}"
}

# Function to run parallel processing tests
run_parallel_test() {
    echo -e "\n${MAGENTA}===== RUNNING PARALLEL PROCESSING TESTS =====${NC}"
    check_gyatt_binary
    
    # Create test repository with varying file sizes
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR" || exit 1
    "$GYATT_PATH" init
    
    # Create test files
    echo -e "${CYAN}Creating test files...${NC}"
    for i in {1..100}; do
        mkdir -p "dir$((i % 10))"
        dd if=/dev/urandom of="dir$((i % 10))/file$i.dat" bs=1K count=$((i % 50 + 1)) 2>/dev/null
    done
    
    # Add and commit files
    "$GYATT_PATH" add .
    "$GYATT_PATH" commit -m "Test commit for parallel processing"
    
    # Run parallel performance test
    cd - > /dev/null
    if [[ -f "test_parallel_performance" ]]; then
        echo -e "${CYAN}Executing parallel performance test...${NC}"
        ./test_parallel_performance
    else
        echo -e "${YELLOW}Building parallel performance test...${NC}"
        g++ -std=c++17 -o test_parallel_performance test_parallel_performance.cpp -pthread
        ./test_parallel_performance
    fi
    
    # Cleanup
    rm -rf "$TEST_DIR"
    echo -e "\n${GREEN}Parallel processing tests completed${NC}"
}

# Function to run memory optimization tests
run_memory_test() {
    echo -e "\n${MAGENTA}===== RUNNING MEMORY OPTIMIZATION TESTS =====${NC}"
    check_gyatt_binary
    
    # Compile memory test if needed
    if [[ ! -f "test_memory_optimization" ]]; then
        echo -e "${YELLOW}Building memory optimization test...${NC}"
        g++ -std=c++17 -o test_memory_optimization test_memory_optimization.cpp -pthread
    fi
    
    echo -e "${CYAN}Executing memory optimization test...${NC}"
    ./test_memory_optimization
    
    echo -e "\n${GREEN}Memory optimization tests completed${NC}"
}

# Function to run GitHub integration tests
run_github_test() {
    echo -e "\n${MAGENTA}===== RUNNING GITHUB INTEGRATION TESTS =====${NC}"
    check_gyatt_binary
    
    # Create test repository
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR" || exit 1
    "$GYATT_PATH" init
    
    # Create test files
    echo -e "${CYAN}Creating test files...${NC}"
    for i in {1..10}; do
        echo "Test file $i" > "file$i.txt"
    done
    
    # Add and commit files
    "$GYATT_PATH" add .
    "$GYATT_PATH" commit -m "Test commit for GitHub integration"
    
    # Run GitHub simulation
    cd - > /dev/null
    if [[ -f "test_github_simulation" ]]; then
        echo -e "${CYAN}Executing GitHub simulation test...${NC}"
        ./test_github_simulation
    else
        echo -e "${YELLOW}Building GitHub simulation test...${NC}"
        g++ -std=c++17 -o test_github_simulation test_github_simulation.cpp -lcurl -pthread
        ./test_github_simulation
    fi
    
    # Cleanup
    rm -rf "$TEST_DIR"
    echo -e "\n${GREEN}GitHub integration tests completed${NC}"
}

# Function to run basic functionality tests
run_basic_test() {
    echo -e "\n${MAGENTA}===== RUNNING BASIC FUNCTIONALITY TESTS =====${NC}"
    check_gyatt_binary
    
    # Create test repository
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR" || exit 1
    
    echo -e "${CYAN}Testing repository initialization${NC}"
    "$GYATT_PATH" init
    
    echo -e "${CYAN}Testing file staging${NC}"
    echo "Test file" > test.txt
    "$GYATT_PATH" add test.txt
    
    echo -e "${CYAN}Testing commit creation${NC}"
    "$GYATT_PATH" commit -m "Test commit"
    
    echo -e "${CYAN}Testing status command${NC}"
    "$GYATT_PATH" status
    
    echo -e "${CYAN}Testing log command${NC}"
    "$GYATT_PATH" log
    
    # Cleanup
    cd - > /dev/null
    rm -rf "$TEST_DIR"
    echo -e "\n${GREEN}Basic functionality tests completed${NC}"
}

# Function to run all tests
run_all_tests() {
    run_basic_test
    run_compression_test
    run_http_test
    run_parallel_test
    run_memory_test
    run_github_test
    
    echo -e "\n${GREEN}All tests completed successfully${NC}"
}

# Main execution logic
case "$1" in
    basic)
        run_basic_test
        ;;
    compression)
        run_compression_test
        ;;
    http)
        run_http_test
        ;;
    parallel)
        run_parallel_test
        ;;
    memory)
        run_memory_test
        ;;
    github)
        run_github_test
        ;;
    all)
        run_all_tests
        ;;
    help|--help|-h|"")
        print_usage
        ;;
    *)
        echo -e "${RED}Unknown option: $1${NC}"
        print_usage
        exit 1
        ;;
esac

exit 0
