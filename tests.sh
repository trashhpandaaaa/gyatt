#!/bin/bash

# Comprehensive test suite for Gyatt functionality
# Remove set -e to allow tests to continue even if some fail

echo "=========================================="
echo "GYATT COMPREHENSIVE TEST SUITE"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

# Test function
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    echo -e "\n${YELLOW}Testing: $test_name${NC}"
    
    # Capture both stdout and stderr, and check exit code
    if output=$(eval "$test_command" 2>&1) && [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ PASSED${NC}: $test_name"
        ((PASSED++))
    else
        echo -e "${RED}✗ FAILED${NC}: $test_name"
        echo -e "${RED}  Error: $output${NC}"
        ((FAILED++))
    fi
}

# Test basic functionality
echo -e "\n${YELLOW}=== BASIC FUNCTIONALITY TESTS ===${NC}"

run_test "Binary exists and runs" "./gyatt help"
run_test "GitHub token test script exists" "test -f test_token.sh"
run_test "Token test script is executable" "test -x test_token.sh"

# Test GitHub integration
echo -e "\n${YELLOW}=== GITHUB INTEGRATION TESTS ===${NC}"

if [ -f .gyatt/github_token ]; then
    echo "GitHub token found, running API tests..."
    
    # Test token validity
    TOKEN=$(cat .gyatt/github_token | tr -d '\n')
    
    run_test "GitHub user authentication" "curl -s -f -H 'Authorization: token $TOKEN' https://api.github.com/user"
    run_test "GitHub repository access" "curl -s -f -H 'Authorization: token $TOKEN' https://api.github.com/repos/trashhpandaaaa/gyatt"
    run_test "Repository permissions check" "curl -s -f -H 'Authorization: token $TOKEN' https://api.github.com/user/repos"
else
    echo -e "${YELLOW}GitHub token not found, skipping API tests${NC}"
fi

# Test repository functionality
echo -e "\n${YELLOW}=== REPOSITORY FUNCTIONALITY TESTS ===${NC}"

# Create a temporary test directory
TEST_DIR="/tmp/gyatt_test_$(date +%s)"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

run_test "Repository initialization" "$OLDPWD/gyatt init"
run_test "Check .gyatt directory creation" "test -d .gyatt"
run_test "Check HEAD file creation" "test -f .gyatt/HEAD"
run_test "Check objects directory creation" "test -d .gyatt/objects"
run_test "Check refs directory creation" "test -d .gyatt/refs"

# Test file operations
echo "test content" > test_file.txt
run_test "Add file to repository" "$OLDPWD/gyatt add test_file.txt"
run_test "Check repository status" "$OLDPWD/gyatt status"

# Test ignore functionality
run_test "Create ignore file" "$OLDPWD/gyatt gyattignore || test -f .gyattignore"
run_test "Check ignore file exists" "test -f .gyattignore"
echo "*.tmp" > temp_pattern.txt
run_test "Add ignore pattern" "$OLDPWD/gyatt add-ignore '*.tmp'"

# Test commit first (needed for branch operations)
echo "test content" >> test_file.txt
$OLDPWD/gyatt add test_file.txt >/dev/null 2>&1 || true
run_test "Create initial commit" "$OLDPWD/gyatt commit -m 'Initial commit'"

# Test branch operations (after commit)
run_test "List branches" "$OLDPWD/gyatt branch"
run_test "Create new branch" "$OLDPWD/gyatt branch test-branch"
run_test "Checkout branch" "$OLDPWD/gyatt checkout test-branch"

# Test commit (this might fail if no staged files)
echo "more content" >> test_file.txt
$OLDPWD/gyatt add test_file.txt >/dev/null 2>&1 || true
run_test "Create second commit" "$OLDPWD/gyatt commit -m 'Test commit'"

# Test log and diff
run_test "Show commit log" "$OLDPWD/gyatt log"
run_test "Show differences" "$OLDPWD/gyatt diff"

# Test remote operations (dry run)
run_test "Add remote repository" "$OLDPWD/gyatt remote add origin https://github.com/test/test.git"
run_test "List remotes" "$OLDPWD/gyatt remote -v"

# Clean up test directory
cd "$OLDPWD"
rm -rf "$TEST_DIR"

# Test build and compilation
echo -e "\n${YELLOW}=== BUILD AND COMPILATION TESTS ===${NC}"

run_test "Clean build" "make clean && make"
run_test "Debug build exists" "test -f gyatt_debug"
run_test "Release build exists" "test -f gyatt"

# Test dependencies and libraries
echo -e "\n${YELLOW}=== DEPENDENCY TESTS ===${NC}"

run_test "libcurl available" "pkg-config --exists libcurl"
run_test "OpenSSL available" "pkg-config --exists openssl"

# Test alias functionality
echo -e "\n${YELLOW}=== ALIAS FUNCTIONALITY TESTS ===${NC}"

# The aliases are implemented in main.cpp (like "yeet" for "add", "fr" for "commit")
TEST_DIR2="/tmp/gyatt_alias_test_$(date +%s)"
mkdir -p "$TEST_DIR2"
cd "$TEST_DIR2"

$OLDPWD/gyatt init >/dev/null 2>&1
echo "alias test" > alias_test.txt

run_test "Alias 'yeet' for 'add'" "$OLDPWD/gyatt yeet alias_test.txt"
run_test "Alias 'fr' for 'commit'" "$OLDPWD/gyatt fr -m 'Alias test commit'"

cd "$OLDPWD"
rm -rf "$TEST_DIR2"

# Final results
echo -e "\n=========================================="
echo "TEST RESULTS SUMMARY"
echo "=========================================="
echo -e "Tests passed: ${GREEN}$PASSED${NC}"
echo -e "Tests failed: ${RED}$FAILED${NC}"
echo -e "Total tests: $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
    exit 0
else
    echo -e "\n${RED}❌ SOME TESTS FAILED ❌${NC}"
    exit 1
fi