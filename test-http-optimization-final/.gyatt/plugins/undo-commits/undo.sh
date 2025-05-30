#!/bin/bash
# Undo commits plugin for gyatt

undo_commits() {
    local count=${1:-1}
    
    echo "⚠️  About to undo $count commit(s)"
    echo "This will reset HEAD~$count and stage the changes"
    
    read -p "Continue? (y/N): " confirm
    if [[ $confirm =~ ^[Yy]$ ]]; then
        git reset --soft HEAD~$count
        echo "✅ Successfully undone $count commit(s)"
        echo "💡 Changes are now staged, ready to recommit"
    else
        echo "❌ Operation cancelled"
        return 1
    fi
}

case "$1" in
    "undo")
        undo_commits ${2:-1}
        ;;
    *)
        echo "Usage: $0 undo [number_of_commits]"
        exit 1
        ;;
esac
