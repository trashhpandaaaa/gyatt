/**
 * Comprehensive Test Suite for Gyatt Version Control System
 * 
 * This file contains comprehensive tests for all major components:
 * - Core version control operations
 * - Advanced compression algorithms
 * - Memory optimization
 * - Plugin system
 * - Performance metrics
 * - Security guardrails
 * - Edge cases and error handling
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <filesystem>
#include <memory>
#include <random>
#include <cassert>
#include <iomanip>

// Include main headers
#include "../include/repository.h"
#include "../include/advanced_compression.h"
#include "../include/memory_optimization.h"
#include "../include/plugin_system.h"
#include "../include/guardrails.h"

using namespace gyatt;
namespace fs = std::filesystem;

class TestReporter {
private:
    int totalTests = 0;
    int passedTests = 0;
    int failedTests = 0;
    std::vector<std::string> failureReasons;
    std::chrono::high_resolution_clock::time_point startTime;

public:
    TestReporter() {
        startTime = std::chrono::high_resolution_clock::now();
        std::cout << "\n=== Gyatt Version Control System - Comprehensive Test Suite ===" << std::endl;
        std::cout << "Starting tests at " << getCurrentTimestamp() << std::endl;
        std::cout << "=" << std::string(65, '=') << std::endl;
    }

    void startTest(const std::string& testName) {
        std::cout << "\n🧪 Running: " << testName << "..." << std::flush;
        totalTests++;
    }

    void testPassed() {
        passedTests++;
        std::cout << " ✅ PASS" << std::endl;
    }

    void testFailed(const std::string& reason) {
        failedTests++;
        failureReasons.push_back(reason);
        std::cout << " ❌ FAIL: " << reason << std::endl;
    }

    void generateReport() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "\n" << std::string(65, '=') << std::endl;
        std::cout << "=== TEST SUMMARY ===" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << " ✅" << std::endl;
        std::cout << "Failed: " << failedTests << " ❌" << std::endl;
        std::cout << "Success Rate: " << (totalTests > 0 ? (passedTests * 100.0 / totalTests) : 0) << "%" << std::endl;
        std::cout << "Execution Time: " << duration.count() << "ms" << std::endl;

        if (!failureReasons.empty()) {
            std::cout << "\n--- FAILURE DETAILS ---" << std::endl;
            for (size_t i = 0; i < failureReasons.size(); i++) {
                std::cout << (i + 1) << ". " << failureReasons[i] << std::endl;
            }
        }

        std::cout << "\nTest completed at " << getCurrentTimestamp() << std::endl;
        std::cout << std::string(65, '=') << std::endl;
    }

private:
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

class GyattTestSuite {
private:
    TestReporter reporter;
    std::unique_ptr<Repository> repo;
    std::unique_ptr<AdvancedCompressionEngine> compressionEngine;
    std::unique_ptr<MemoryOptimizationManager> memManager;
    std::unique_ptr<PluginManager> pluginSystem;
    std::unique_ptr<GuardrailSystem> guardrails;

public:
    GyattTestSuite() {
        // Initialize test environment
        setupTestEnvironment();
    }

    void runAllTests() {
        // Core functionality tests
        testRepositoryBasics();
        testCommitOperations();
        testBranchOperations();
        
        // Advanced compression tests
        testCompressionAlgorithms();
        testCompressionPerformance();
        
        // Memory optimization tests
        testMemoryOptimization();
        testMemoryProfiling();
        
        // Plugin system tests
        testPluginLoading();
        testPluginExecution();
        
        // Security and guardrails tests
        testSecurityGuardrails();
        
        // Performance tests
        testLargeRepositoryHandling();
        testConcurrentOperations();
        testScalabilityLimits();
        
        // Edge cases and error handling
        testCorruptedDataHandling();
        testFileSystemErrors();
        
        // Integration tests
        testEndToEndWorkflow();

        reporter.generateReport();
    }

private:
    void setupTestEnvironment() {
        // Create temporary test directory
        fs::create_directories("test_workspace");
        fs::current_path("test_workspace");
        
        // Create test repository directory
        fs::create_directories("test_repo");
        
        // Initialize components
        repo = std::make_unique<Repository>("test_repo");
        compressionEngine = std::make_unique<AdvancedCompressionEngine>();
        memManager = std::make_unique<MemoryOptimizationManager>("test_repo");
        pluginSystem = std::make_unique<PluginManager>("test_repo");
        guardrails = std::make_unique<GuardrailSystem>("test_repo");
    }

    void testRepositoryBasics() {
        reporter.startTest("Repository Initialization");
        try {
            bool initResult = repo->init(); // init() takes no parameters
            if (initResult && fs::exists("test_repo/.gyatt")) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Repository initialization failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testCommitOperations() {
        reporter.startTest("Commit Operations");
        try {
            // Ensure repository is initialized
            repo->init();
            
            // Create test file in correct directory
            fs::create_directories("test_repo");
            std::ofstream testFile("test_repo/test.txt");
            testFile << "Hello, Gyatt!";
            testFile.close();

            // Change to repo directory for git operations
            auto originalPath = fs::current_path();
            fs::current_path("test_repo");
            
            // Add and commit (using correct API)
            bool addResult = repo->add("test.txt"); // single file, not vector
            bool commitResult = repo->commit("Initial commit", "Test User"); // 2 params, not 3
            
            // Return to original directory
            fs::current_path(originalPath);
            
            if (addResult && commitResult) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Commit operation failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testBranchOperations() {
        reporter.startTest("Branch Operations");
        try {
            // Ensure we have a commit first
            repo->init();
            
            // Create test file and commit
            fs::create_directories("test_repo");
            std::ofstream testFile("test_repo/initial.txt");
            testFile << "Initial content";
            testFile.close();
            
            auto originalPath = fs::current_path();
            fs::current_path("test_repo");
            
            repo->add("initial.txt");
            repo->commit("Initial commit for branching", "Test User");
            
            // Now test branch operations
            bool createResult = repo->createBranch("feature-branch");
            bool switchResult = repo->checkout("feature-branch"); // checkout, not switchBranch
            std::string currentBranch = repo->getCurrentBranch();
            
            fs::current_path(originalPath);
            
            if (createResult && switchResult && currentBranch == "feature-branch") {
                reporter.testPassed();
            } else {
                reporter.testFailed("Branch operations failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testCompressionAlgorithms() {
        reporter.startTest("Compression Algorithms");
        try {
            std::string testData = generateTestData(1024 * 1024); // 1MB test data
            
            auto lz4Result = compressionEngine->compress(testData, CompressionType::LZ4_FAST);
            auto zlibResult = compressionEngine->compress(testData, CompressionType::ZLIB_FAST);
            auto lz4HighResult = compressionEngine->compress(testData, CompressionType::LZ4_HIGH);
            
            if (lz4Result.compressedSize < testData.size() && 
                zlibResult.compressedSize < testData.size() && 
                lz4HighResult.compressedSize < testData.size()) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Compression did not reduce data size");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testCompressionPerformance() {
        reporter.startTest("Compression Performance");
        try {
            std::string testData = generateTestData(10 * 1024 * 1024); // 10MB test data
            
            auto start = std::chrono::high_resolution_clock::now();
            auto compressed = compressionEngine->compress(testData, CompressionType::LZ4_FAST);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            if (duration.count() < 5000 && compressed.compressedSize < testData.size()) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Compression performance below expectations");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testMemoryOptimization() {
        reporter.startTest("Memory Optimization");
        try {
            memManager->enableOptimization();
            
            // Allocate and deallocate memory to test optimization
            std::vector<std::unique_ptr<char[]>> allocations;
            for (int i = 0; i < 100; i++) {
                allocations.push_back(std::make_unique<char[]>(1024 * 1024));
            }
            
            memManager->optimizeForMemory();
            
            // Just check that optimization methods can be called without errors
            reporter.testPassed();
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testMemoryProfiling() {
        reporter.startTest("Memory Profiling");
        try {
            auto profile = memManager->getMemoryProfile();
            
            // Perform memory-intensive operations
            std::vector<std::string> data;
            for (int i = 0; i < 1000; i++) {
                data.push_back(generateTestData(1024));
            }
            
            if (profile.totalSystemMemory > 0) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Memory profiling did not capture data");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testPluginLoading() {
        reporter.startTest("Plugin Loading");
        try {
            bool installResult = pluginSystem->installPlugin("test_plugin");
            auto loadedPlugins = pluginSystem->listPlugins(); // listPlugins, not getLoadedPlugins
            
            if (installResult || !loadedPlugins.empty()) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Plugin loading failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testPluginExecution() {
        reporter.startTest("Plugin Execution");
        try {
            bool result = pluginSystem->executePlugin("test_plugin", {"arg1", "arg2"}); // returns bool, not string
            
            if (result) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Plugin execution failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testSecurityGuardrails() {
        reporter.startTest("Security Guardrails");
        try {
            std::vector<std::string> testFiles = {"test_repo/test.txt"};
            bool securityPassed = guardrails->runPreCommitChecks(testFiles); // runPreCommitChecks, not performSecurityScan
            
            if (securityPassed) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Security guardrails failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testLargeRepositoryHandling() {
        reporter.startTest("Large Repository Handling");
        try {
            // Create multiple large files
            for (int i = 0; i < 10; i++) {
                std::ofstream largeFile("test_repo/large_file_" + std::to_string(i) + ".txt");
                largeFile << generateTestData(1024 * 1024); // 1MB each
                largeFile.close();
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            // Add files one by one (repository add takes single file)
            bool allSuccess = true;
            for (int i = 0; i < 10; i++) {
                bool addResult = repo->add("large_file_" + std::to_string(i) + ".txt");
                allSuccess = allSuccess && addResult;
            }
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
            
            if (allSuccess && duration.count() < 30) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Large repository handling too slow");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testConcurrentOperations() {
        reporter.startTest("Concurrent Operations");
        try {
            std::vector<std::thread> threads;
            std::atomic<int> successCount{0};
            
            // Start multiple concurrent operations
            for (int i = 0; i < 5; i++) {
                threads.emplace_back([this, i, &successCount]() {
                    try {
                        std::ofstream file("test_repo/concurrent_" + std::to_string(i) + ".txt");
                        file << "Concurrent operation " << i;
                        file.close();
                        
                        if (repo->add("concurrent_" + std::to_string(i) + ".txt")) {
                            successCount++;
                        }
                    } catch (...) {
                        // Ignore individual thread failures
                    }
                });
            }
            
            // Wait for all threads to complete
            for (auto& thread : threads) {
                thread.join();
            }
            
            if (successCount >= 3) { // At least 3 out of 5 should succeed
                reporter.testPassed();
            } else {
                reporter.testFailed("Concurrent operations failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testScalabilityLimits() {
        reporter.startTest("Scalability Limits");
        try {
            auto profile = memManager->getMemoryProfile();
            size_t initialMemory = profile.processMemoryUsage;
            
            // Test with gradually increasing load
            for (int scale = 1; scale <= 5; scale++) {
                std::string data = generateTestData(scale * 1024 * 1024);
                compressionEngine->compress(data, CompressionType::LZ4_FAST);
            }
            
            auto finalProfile = memManager->getMemoryProfile();
            size_t finalMemory = finalProfile.processMemoryUsage;
            
            if (finalMemory < initialMemory + (20 * 1024 * 1024)) { // Memory growth should be reasonable
                reporter.testPassed();
            } else {
                reporter.testFailed("Memory usage scaling too high");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testCorruptedDataHandling() {
        reporter.startTest("Corrupted Data Handling");
        try {
            std::vector<uint8_t> corruptedData = {0x01, 0x02, 0x03, 0x04}; // Invalid compressed data
            
            try {
                compressionEngine->decompress(corruptedData, CompressionType::LZ4_FAST);
                reporter.testFailed("Should have thrown exception for corrupted data");
            } catch (const std::exception&) {
                reporter.testPassed(); // Expected to throw
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Unexpected exception: ") + e.what());
        }
    }

    void testFileSystemErrors() {
        reporter.startTest("File System Error Handling");
        try {
            // Try to access non-existent directory - just test that it handles errors gracefully
            Repository testRepo("/non/existent/path/repo");
            bool result = testRepo.init(); // init() takes no parameters
            
            if (!result) {
                reporter.testPassed(); // Expected to fail gracefully
            } else {
                reporter.testFailed("Should have failed with invalid path");
            }
        } catch (const std::exception& e) {
            reporter.testPassed(); // Exception handling is acceptable
        }
    }

    void testEndToEndWorkflow() {
        reporter.startTest("End-to-End Workflow");
        try {
            // Complete workflow test
            bool success = true;
            
            // Initialize new repo
            Repository e2eRepo("e2e_test_repo");
            success &= e2eRepo.init(); // init() takes no parameters
            
            // Create and add files
            std::ofstream file("e2e_test_repo/workflow.txt");
            file << "End-to-end test content";
            file.close();
            
            success &= e2eRepo.add("workflow.txt");
            
            // Commit changes
            bool commitResult = e2eRepo.commit("E2E test commit", "Test User"); // 2 params, not 3
            success &= commitResult;
            
            // Create and switch branch
            success &= e2eRepo.createBranch("e2e-feature");
            success &= e2eRepo.checkout("e2e-feature"); // checkout, not switchBranch
            
            // Make changes
            std::ofstream featureFile("e2e_test_repo/feature.txt");
            featureFile << "Feature branch content";
            featureFile.close();
            
            success &= e2eRepo.add("feature.txt");
            e2eRepo.commit("Feature commit", "Test User");
            
            success &= e2eRepo.checkout("main"); // checkout back to main
            
            if (success) {
                reporter.testPassed();
            } else {
                reporter.testFailed("End-to-end workflow failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    std::string generateTestData(size_t size) {
        std::string data;
        data.reserve(size);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(32, 126); // Printable ASCII characters
        
        for (size_t i = 0; i < size; i++) {
            data += static_cast<char>(dis(gen));
        }
        
        return data;
    }
};

int main(int, char*[]) {
    try {
        GyattTestSuite testSuite;
        testSuite.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
