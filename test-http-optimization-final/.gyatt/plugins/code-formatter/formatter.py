#!/usr/bin/env python3
import os
import sys
import subprocess

def format_code(filepath):
    """Format code based on file extension"""
    try:
        ext = os.path.splitext(filepath)[1].lower()
        
        if ext == '.py':
            subprocess.run(['black', filepath], check=True)
            print(f"✅ Formatted Python file: {filepath}")
        elif ext in ['.js', '.ts', '.json']:
            subprocess.run(['prettier', '--write', filepath], check=True)
            print(f"✅ Formatted JS/TS file: {filepath}")
        elif ext in ['.cpp', '.h', '.hpp', '.cc']:
            subprocess.run(['clang-format', '-i', filepath], check=True)
            print(f"✅ Formatted C++ file: {filepath}")
        else:
            print(f"⚠️  No formatter available for: {filepath}")
            
    except subprocess.CalledProcessError:
        print(f"❌ Failed to format: {filepath}")
    except FileNotFoundError:
        print(f"❌ Formatter not installed for: {filepath}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: code-formatter.py <file>")
        sys.exit(1)
        
    format_code(sys.argv[1])
