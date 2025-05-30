#!/usr/bin/env python3
import os
import sys
import subprocess
import json
from datetime import datetime

def generate_changelog():
    """Generate changelog from git commits"""
    try:
        # Get commit history
        result = subprocess.run(['git', 'log', '--oneline', '--decorate'], 
                              capture_output=True, text=True)
        
        if result.returncode != 0:
            print("Error: Not in a git repository")
            return False
            
        commits = result.stdout.strip().split('\n')
        
        # Create changelog
        changelog = "# Changelog\n\n"
        changelog += f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        for commit in commits:
            if commit.strip():
                changelog += f"- {commit}\n"
        
        # Write to file
        with open('CHANGELOG.md', 'w') as f:
            f.write(changelog)
            
        print("✅ Changelog generated successfully")
        return True
        
    except Exception as e:
        print(f"❌ Error generating changelog: {e}")
        return False

if __name__ == "__main__":
    generate_changelog()
