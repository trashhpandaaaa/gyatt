#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TEST_DIR="/tmp/gyatt_test_$(date +%s)"

function run_single_file_test() {
    local file_path=$1
    local file_size=$2
    
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR" || exit 1
    
    ./gyatt init
    
    echo -e "${BLUE}Testing with file: ${NC}$file_path (Size: $file_size bytes)"
    
    dd if=/dev/urandom of="test_file" bs=$file_size count=1 2>/dev/null
    
    ./gyatt add test_file
    ./gyatt commit -m "Test commit"
    
    echo -e "${GREEN}Test completed${NC}"
    
    cd - > /dev/null
    rm -rf "$TEST_DIR"
}

function show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -f, --file FILE    Test with specific file"
    echo "  -s, --size SIZE    Test with random file of specified size (in bytes)"
    echo "  -b, --benchmark    Run full benchmark"
    echo "  -h, --help        Show this help message"
}

case "$1" in
    -f|--file)
        if [ -f "$2" ]; then
            run_single_file_test "$2" "$(stat -f %z "$2")"
        else
            echo -e "${RED}File not found: $2${NC}"
            exit 1
        fi
        ;;
    -s|--size)
        if [[ "$2" =~ ^[0-9]+$ ]]; then
            run_single_file_test "random_test_file" "$2"
        else
            echo -e "${RED}Invalid size: $2${NC}"
            exit 1
        fi
        ;;
    -b|--benchmark)
        ./benchmark_github_push.sh
        ;;
    -h|--help|*)
        show_help
        ;;
esac 