#!/usr/bin/env node
// Test runner plugin for gyatt
const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');

function runTests() {
    const cwd = process.cwd();
    
    // Check for different test frameworks
    if (fs.existsSync(path.join(cwd, 'package.json'))) {
        const pkg = JSON.parse(fs.readFileSync(path.join(cwd, 'package.json')));
        
        if (pkg.scripts && pkg.scripts.test) {
            console.log('🧪 Running npm test...');
            exec('npm test', (error, stdout, stderr) => {
                if (error) {
                    console.log('❌ Tests failed:', error.message);
                    return;
                }
                console.log('✅ Tests passed');
                console.log(stdout);
            });
            return;
        }
    }
    
    // Check for Python tests
    if (fs.existsSync(path.join(cwd, 'requirements.txt')) || 
        fs.existsSync(path.join(cwd, 'setup.py'))) {
        console.log('🧪 Running Python tests...');
        exec('python -m pytest', (error, stdout, stderr) => {
            if (error) {
                console.log('❌ Tests failed:', error.message);
                return;
            }
            console.log('✅ Tests passed');
            console.log(stdout);
        });
        return;
    }
    
    // Check for C++ tests
    if (fs.existsSync(path.join(cwd, 'Makefile')) || 
        fs.existsSync(path.join(cwd, 'CMakeLists.txt'))) {
        console.log('🧪 Running C++ tests...');
        exec('make test', (error, stdout, stderr) => {
            if (error) {
                console.log('❌ Tests failed:', error.message);
                return;
            }
            console.log('✅ Tests passed');
            console.log(stdout);
        });
        return;
    }
    
    console.log('⚠️  No test framework detected');
}

runTests();
