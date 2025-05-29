#!/bin/bash

# Edge case and error handling tests for Gyatt
echo "=========================================="
echo "GYATT EDGE CASE & ERROR HANDLING TESTS"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

# Test function that expects failure
expect_failure() {
    local test_name="$1"
    local test_command="$2"
    
    echo -e "\n${YELLOW}Testing (expect failure): $test_name${NC}"
    
    if eval "$test_command" >/dev/null 2>&1; then
        echo -e "${RED}✗ UNEXPECTED SUCCESS${NC}: $test_name"
        ((FAILED++))
    else
        echo -e "${GREEN}✓ EXPECTED FAILURE${NC}: $test_name"
        ((PASSED++))
    fi
}

# Test function that expects success
expect_success() {
    local test_name="$1"
    local test_command="$2"
    
    echo -e "\n${YELLOW}Testing: $test_name${NC}"
    
    if eval "$test_command" >/dev/null 2>&1; then
        echo -e "${GREEN}✓ PASSED${NC}: $test_name"
        ((PASSED++))
    else
        echo -e "${RED}✗ FAILED${NC}: $test_name"
        ((FAILED++))
    fi
}

echo -e "\n${YELLOW}=== ERROR HANDLING TESTS ===${NC}"

# Test commands outside of repository
TEST_DIR="/tmp/gyatt_error_test_$(date +%s)"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

expect_failure "Add file outside repository" "$OLDPWD/gyatt add nonexistent.txt"
expect_failure "Commit outside repository" "$OLDPWD/gyatt commit -m 'test'"
expect_failure "Status outside repository" "$OLDPWD/gyatt status"
expect_failure "Branch outside repository" "$OLDPWD/gyatt branch test"

# Test with initialized repository
$OLDPWD/gyatt init >/dev/null 2>&1

expect_failure "Add nonexistent file" "$OLDPWD/gyatt add nonexistent_file.txt"
expect_failure "Commit with no staged files" "$OLDPWD/gyatt commit -m 'empty commit'"
expect_failure "Checkout nonexistent branch" "$OLDPWD/gyatt checkout nonexistent-branch"
expect_failure "Invalid commit message format" "$OLDPWD/gyatt commit 'missing -m flag'"

echo -e "\n${YELLOW}=== EDGE CASE TESTS ===${NC}"

# Test with special characters
echo "content" > "file with spaces.txt"
expect_success "Add file with spaces" "$OLDPWD/gyatt add 'file with spaces.txt'"

echo "content" > "file_with_unicode_🎉.txt"
expect_success "Add file with unicode" "$OLDPWD/gyatt add 'file_with_unicode_🎉.txt'"

# Test with very long filename
LONG_NAME="this_is_a_very_long_filename_that_should_still_work_fine_even_though_it_is_quite_long_indeed.txt"
echo "content" > "$LONG_NAME"
expect_success "Add file with long name" "$OLDPWD/gyatt add '$LONG_NAME'"

# Test with empty file
touch empty_file.txt
expect_success "Add empty file" "$OLDPWD/gyatt add empty_file.txt"

# Test large file (1MB)
dd if=/dev/zero of=large_file.txt bs=1024 count=1024 >/dev/null 2>&1
expect_success "Add large file (1MB)" "$OLDPWD/gyatt add large_file.txt"

# Test with binary file
echo -e '\x00\x01\x02\x03\x04\x05' > binary_file.bin
expect_success "Add binary file" "$OLDPWD/gyatt add binary_file.bin"

# Test commit with all files
expect_success "Commit all files" "$OLDPWD/gyatt commit -m 'Test commit with various file types'"

echo -e "\n${YELLOW}=== IGNORE PATTERN TESTS ===${NC}"

# Test ignore patterns
echo "*.log" > test_patterns.txt
expect_success "Add ignore pattern *.log" "$OLDPWD/gyatt add-ignore '*.log'"

echo "test log content" > test.log
expect_success "Check that .log file is ignored" "$OLDPWD/gyatt check-ignore test.log"

echo "temp" > temp.txt
expect_success "Add non-ignored file" "$OLDPWD/gyatt add temp.txt"

echo -e "\n${YELLOW}=== REMOTE OPERATION TESTS ===${NC}"

expect_failure "Push to nonexistent remote" "$OLDPWD/gyatt push nonexistent main"
expect_failure "Clone nonexistent repository" "$OLDPWD/gyatt clone /nonexistent/path"
expect_failure "Add invalid remote URL" "$OLDPWD/gyatt remote add test 'not-a-valid-url'"

echo -e "\n${YELLOW}=== GITHUB TOKEN TESTS ===${NC}"

# Test token operations
expect_success "Set test GitHub token" "$OLDPWD/gyatt github-token 'test-token-123'"
expect_success "Clear GitHub token" "$OLDPWD/gyatt github-token clear"

echo -e "\n${YELLOW}=== CONCURRENT ACCESS TESTS ===${NC}"

# Test concurrent operations (basic)
echo "concurrent test" > concurrent1.txt
echo "concurrent test" > concurrent2.txt

# Add files simultaneously (in background)
$OLDPWD/gyatt add concurrent1.txt &
$OLDPWD/gyatt add concurrent2.txt &
wait

expect_success "Check status after concurrent adds" "$OLDPWD/gyatt status"

echo -e "\n${YELLOW}=== REPOSITORY CORRUPTION TESTS ===${NC}"

# Test with corrupted objects (create invalid object)
mkdir -p .gyatt/objects/ab
echo "invalid object content" > .gyatt/objects/ab/cdef1234567890
expect_success "Status with corrupted object" "$OLDPWD/gyatt status"

# Clean up
cd "$OLDPWD"
rm -rf "$TEST_DIR"

echo -e "\n=========================================="
echo "EDGE CASE TEST RESULTS SUMMARY"
echo "=========================================="
echo -e "Tests passed: ${GREEN}$PASSED${NC}"
echo -e "Tests failed: ${RED}$FAILED${NC}"
echo -e "Total tests: $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 ALL EDGE CASE TESTS PASSED! 🎉${NC}"
    exit 0
else
    echo -e "\n${RED}❌ SOME EDGE CASE TESTS FAILED ❌${NC}"
    exit 1
fi
