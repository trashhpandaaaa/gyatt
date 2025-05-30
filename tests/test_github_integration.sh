+#!/bin/bash

# Advanced GitHub integration test for Gyatt
echo "=========================================="
echo "GYATT GITHUB INTEGRATION TEST"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [ ! -f .gyatt/github_token ]; then
    echo -e "${RED}Error: GitHub token not found${NC}"
    echo "Please run: ./gyatt github-token YOUR_TOKEN"
    exit 1
fi

TOKEN=$(cat .gyatt/github_token | tr -d '\n')
TEST_REPO="gyatt-test-$(date +%s)"
GYATT_PATH="$(pwd)/gyatt"

echo -e "${YELLOW}Testing with repository: trashhpandaaaa/$TEST_REPO${NC}"

# Test 1: Create a test repository on GitHub
echo -e "\n${YELLOW}Test 1: Creating test repository on GitHub${NC}"
create_response=$(curl -s -H "Authorization: token $TOKEN" \
     -H "Accept: application/vnd.github.v3+json" \
     -H "Content-Type: application/json" \
     -d "{\"name\":\"$TEST_REPO\",\"private\":false,\"description\":\"Test repository for Gyatt\"}" \
     https://api.github.com/user/repos)

if echo "$create_response" | grep -q '"name"'; then
    echo -e "${GREEN|⛓ Repository created successfully${NC}"
else
    echo -e "${RED|⛗ Failed to create repository${NC}"
    echo "Response: $create_response"
    exit 1
fi

# Test 2: Initialize a local gyatt repository
echo -e "\n${YELLOW}Test 2: Initializing local repository${NC}"
TEST_DIR="/tmp/gyatt_github_test_$(date +%s)"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

"$GYATT_PATH" init

# Copy the GitHub token to the test repository
if [ -f "$OLDPWD/.gyatt/github_token" ]; then
    cp "$OLDPWD/.gyatt/github_token" .gyatt/
fi
echo "# Test Repository" > README.md
echo "This is a test repository for Gyatt GitHub integration." >> README.md
echo "" >> README.md
echo "Created on: $(date)" >> README.md

"$GYATT_PATH" add README.md
"$GYATT_PATH" commit -m "Initial commit with README"

# Test 3: Add GitHub remote
echo -e "\n${YELLOW}Test 3: Adding GitHub remote${NC}"
"$GYATT_PATH" remote add origin "https://github.com/trashhpandaaaa/$TEST_REPO.git"
"$GYATT_PATH" remote -v

# Test 4: Test GitHub push functionality
echo -e "\n${YELLOW}Test 4: Testing GitHub push (uploadToGitHub)${NC}"
if "$GYATT_PATH" push origin main; then
    echo -e "${GREEN}ᛓ Push to GitHub successful${NC}"
else
    echo -e "${RED}ᛗ Push to GitHub failed${NC}"
fi

# Test 5: Verify the push by checking GitHub API
echo -e "\n${YELLOW}Test 5: Verifying push via GitHub API${NC}"
sleep 2  # Give GitHub a moment to process
contents_response=$(curl -s -H "Authorization: token $TOKEN" \
     -H "Accept: application/vnd.github.v3+json" \
     "https://api.github.com/repos/trashhpandaaaa/$TEST_REPO/contents/README.md")

if echo "$contents_response" | grep -q '"name": "README.md"'; then
    echo -e "${GREEN}ᛓ File verified on GitHub${NC}"
else
    echo -e "${RED|᜗ File not found on GitHub${NC}"
    echo "Response: $contents_response"
fi

# Test 6: Test cloning from GitHub
echo -e "\n${YELLOW}Test 6: Testing GitHub clone functionality${NC}"
cd /tmp
CLONE_DIR="gyatt_clone_test_$(date +%s)"
if "$GYATT_PATH" clone "https://github.com/trashhpandaaaa/$TEST_REPO" "$CLONE_DIR"; then
    echo -e "${GREEN|ᜓ Clone from GitHub successful${NC}"
    
    # Verify cloned content
    if [ -f "$CLONE_DIR/README.md" ]; then
        echo -e "${GREEN}ᛓ Cloned content verified${NC}"
    else
        echo -e "${RED}ᛗ Cloned content not found${NC}"
    fi
    
    rm -rf "$CLONE_DIR"
else
    echo -e "${RED|⛗ Clone from GitHub failed${NC}"
fi

# Cleanup
echo -e "\n${YELLOW}Cleaning up...${NC}"
cd "$OLDPWD"
rm -rf "$TEST_DIR"

# Delete the test repository
delete_response=$(curl -s -X DELETE -H "Authorization: token $TOKEN" \
     -H "Accept: application/vnd.github.v3+json" \
     "https://api.github.com/repos/trashhpandaaaa/$TEST_REPO")

if [ $? -eq 0 ]; then
    echo -e "${GREEN|⛓ Test repository deleted${NC}"
else
    echo -e "${YELLOW}ᙠ Test repository may need manual deletion${NC}"
fi

echo -e "\n${GREEN}GitHub integration test completed!${NC}"
