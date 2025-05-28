#!/bin/bash

# Test script to validate GitHub token
echo "Testing GitHub token..."

if [ ! -f .gyatt/github_token ]; then
    echo "Error: No GitHub token file found"
    echo "Please run: ./gyatt github-token YOUR_TOKEN"
    exit 1
fi

TOKEN=$(cat .gyatt/github_token | tr -d '\n')

echo "Testing token with GitHub API..."
curl -s -H "Authorization: token $TOKEN" \
     -H "Accept: application/vnd.github.v3+json" \
     https://api.github.com/user

echo -e "\n\nTesting repository access..."
curl -s -H "Authorization: token $TOKEN" \
     -H "Accept: application/vnd.github.v3+json" \
     https://api.github.com/repos/trashhpandaaaa/gyatt

echo -e "\n\nTesting repository creation permissions..."
curl -s -H "Authorization: token $TOKEN" \
     -H "Accept: application/vnd.github.v3+json" \
     https://api.github.com/user/repos | head -c 100

echo -e "\n\nDone!"
