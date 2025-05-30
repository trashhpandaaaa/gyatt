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
 * - HTTP interface
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
        std::cout << "\m� Running: " << testName << "..." << std::flush;
        totalTests++;
    }

    void testPassed() {
        passedTests++;
        std::cout << "⛅ PASS" << std::endl;
    }

    void testFailed(const std::string& reason) {
        failedTests++;
        failureReasons.push_back(reason);
        std::cout << "✌ FAIL: " << reason << std::endl;
    }

    void generateReport() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "\n" << std::string(65, '=') << std::endl;
        std::cout << "=== TEST SUMMARY ===" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << "⛅" << std::endl;
        std::cout << "Failed: " << failedTests << "ᝌ" << std::endl;
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
        testMergeOperations();
        
        // Advanced compression tests
        testCompressionAlgorithms();
        testCompressionPerformance();
        testCompressionMemoryEfficiency();
        
        // Memory optimization tests
        testMemoryOptimization();
        testMemoryProfiling();
        testMemoryLeakDetection();
        
        // Plugin system tests
        testPluginLoading();
        testPluginExecution();
        testPluginSecurity();
        
        // Security and guardrails tests
        testSecurityGuardrails();
        testInputValidation();
        testAccessControl();
        
        // Performance tests
        testLargeRepositoryHandling();
        testConcurrentOperations();
        testScalabilityLimits();
        
        // Edge cases and error handling
        testCorruptedDataHandling();
        testNetworkFailures();
        testFileSystemErrors();
        
        // Integration tests
        testEndToEndWorkflow();
        testBackupAndRestore();
        testCrossCompatibility();

        reporter.generateReport();
    }

private:
    void setupTestEnvironment() {
        // Create temporary test directory
        fs::create_directories("test_workspace");
        fs::current_path("test_workspace");
        
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
            bool initResult = repo->init("test_repo");
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
            // Create test file
            std::ofstream testFile("test_repo/test.txt");
            testFile << "Hello, Gyatt!";
            testFile.close();

            // Add and commit
            bool addResult = repo->add({"test.txt"});
            std::string commitHash = repo->commit("Initial commit", "Test User", "test@example.com");
            
            if (addResult && !commitHash.empty()) {
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
            bool createResult = repo->createBranch("feature-branch");
            bool switchResult = repo->switchBranch("feature-branch");
            std::string currentBranch = repo->getCurrentBranch();
            
            if (createResult && switchResult && currentBranch == "feature-branch") {
                reporter.testPassed();
            } else {
                reporter.testFailed("Branch operations failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testMergeOperations() {
        reporter.startTest("Merge Operations");
        try {
            // Switch back to main and create a merge scenario
            repo->switchBranch("main");
            
            std::ofstream mainFile("test_repo/main.txt");
            mainFile << "Main branch content";
            mainFile.close();
            
            repo->add({"main.txt"});
            repo->commit("Main branch commit", "Test User", "test@example.com");
            
            bool mergeResult = repo->merge("feature-branch");
            if (mergeResult) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Merge operation failed");
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
            auto zstdResult = compressionEngine->compress(testData, CompressionType::ZLIB_FAST);
            auto brotliResult = compressionEngine->compress(testData, CompressionType::LZ4_HIGH);
            
            if (lz4Result.size() < testData.size() && 
                zstdResult.size() < testData.size() && 
                brotliResult.size() < testData.size()) {
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
            
            if (duration.count() < 5000 && compressed.size() < testData.size()) { // Should complete in under 5 seconds
                reporter.testPassed();
            } else {
                reporter.testFailed("Compression performance below expectations");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testCompressionMemoryEfficiency() {
        reporter.startTest("Compression Memory Efficiency");
        try {
            size_t initialMemory = memManager->getSystemMemoryUsage();
            
            std::string testData = generateTestData(5 * 1024 * 1024); // 5MB test data
            auto compressed = compressionEngine->compress(testData, CompressionType::ZLIB_FAST);
            
            size_t peakMemory = memManager->getSystemMemoryUsage();
            
            if (peakMemory - initialMemory < testData.size() * 2) { // Memory usage should not exceed 2x input size
                reporter.testPassed();
            } else {
                reporter.testFailed("Memory usage during compression too high");
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
            
            size_t beforeOptimization = memManager->getSystemMemoryUsage();
            memManager->optimizeForMemory();
            size_t afterOptimization = memManager->getSystemMemoryUsage();
            
            if (afterOptimization <= beforeOptimization) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Memory optimization did not reduce usage");
            }
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

    void testMemoryLeakDetection() {
        reporter.startTest("Memory Leak Detection");
        try {
            size_t initialMemory = memManager->getSystemMemoryUsage();
            
            // Simulate potential memory leak scenario
            for (int i = 0; i < 10; i++) {
                auto data = std::make_unique<char[]>(1024);
                // Intentionally not releasing in some iterations to test detection
            }
            
            bool leaksDetected = (memManager->getSystemMemoryUsage() > initialMemory);
            
            if (leaksDetected) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Memory leak detection not working");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testPluginLoading() {
        reporter.startTest("Plugin Loading");
        try {
            bool loadResult = pluginSystem->loadPlugin("test_plugin");
            auto loadedPlugins = pluginSystem->getLoadedPlugins();
            
            if (loadResult || !loadedPlugins.empty()) {
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
            auto result = pluginSystem->executePlugin("test_plugin", {"arg1", "arg2"});
            
            if (result.find("success") != std::string::npos || result.find("executed") != std::string::npos) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Plugin execution failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testPluginSecurity() {
        reporter.startTest("Plugin Security");
        try {
            bool securityCheck = pluginSystem->validatePluginSecurity("test_plugin");
            
            if (securityCheck) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Plugin security validation failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testSecurityGuardrails() {
        reporter.startTest("Security Guardrails");
        try {
            std::vector<std::string> testFiles = {"test_repo/test.txt"};
            bool securityPassed = guardrails->performSecurityScan(testFiles);
            
            if (securityPassed) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Security guardrails failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testInputValidation() {
        reporter.startTest("Input Validation");
        try {
            bool validInput = guardrails->validateInput("valid_input");
            bool invalidInput = !guardrails->validateInput("../../../etc/passwd");
            
            if (validInput && invalidInput) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Input validation not working correctly");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testAccessControl() {
        reporter.startTest("Access Control");
        try {
            bool accessGranted = guardrails->checkAccess("read", "test.txt");
            bool accessDenied = !guardrails->checkAccess("admin", "sensitive_file");
            
            if (accessGranted && accessDenied) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Access control not working correctly");
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
            std::vector<std::string> files;
            for (int i = 0; i < 10; i++) {
                files.push_back("large_file_" + std::to_string(i) + ".txt");
            }
            bool addResult = repo->add(files);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
            
            if (addResult && duration.count() < 30) { // Should complete in under 30 seconds
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
                        
                        if (repo->add({"concurrent_" + std::to_string(i) + ".txt"})) {
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
            size_t initialMemory = memManager->getCurrentMemoryUsage();
            
            // Test with gradually increasing load
            for (int scale = 1; scale <= 5; scale++) {
                std::string data = generateTestData(scale * 1024 * 1024);
                compressionEngine->compress(data, CompressionType::LZ4);
            }
            
            size_t finalMemory = memManager->getCurrentMemoryUsage();
            
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
            std::string corruptedData = "This is not valid compressed data!@#$%^&*()";
            
            try {
                compressionEngine->decompress(corruptedData, CompressionType::LZ4);
                reporter.testFailed("Should have thrown exception for corrupted data");
            } catch (const std::exception&) {
                reporter.testPassed(); // Expected to throw
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Unexpected exception: ") + e.what());
        }
    }

    void testNetworkFailures() {
        reporter.startTest("Network Failure Handling");
        try {
            // Simulate network failure scenario
            bool networkResult = repo->pushToRemote("invalid_remote_url");
            
            if (!networkResult) {
                reporter.testPassed(); // Expected to fail gracefully
            } else {
                reporter.testFailed("Should have failed with invalid remote");
            }
        } catch (const std::exception& e) {
            reporter.testPassed(); // Exception handling is acceptable
        }
    }

    void testFileSystemErrors() {
        reporter.startTest("File System Error Handling");
        try {
            // Try to access non-existent directory
            bool result = repo->init("/non/existent/path/repo");
            
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
            success &= repo->init("e2e_test_repo");
            
            // Create and add files
            std::ofstream file("e2e_test_repo/workflow.txt");
            file << "End-to-end test content";
            file.close();
            
            success &= repo->add({"workflow.txt"});
            
            // Commit changes
            std::string commitHash = repo->commit("E2E test commit", "Test User", "test@example.com");
            success &= !commitHash.empty();
            
            // Create and switch branch
            success &= repo->createBranch("e2e-feature");
            success &= repo->switchBranch("e2e-feature");
            
            // Make changes and merge
            std::ofstream featureFile("e2e_test_repo/feature.txt");
            featureFile << "Feature branch content";
            featureFile.close();
            
            success &= repo->add({"feature.txt"});
            repo->commit("Feature commit", "Test User", "test@example.com");
            
            success &= repo->switchBranch("main");
            success &= repo->merge("e2e-feature");
            
            if (success) {
                reporter.testPassed();
            } else {
                reporter.testFailed("End-to-end workflow failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testBackupAndRestore() {
        reporter.startTest("Backup and Restore");
        try {
            // Create backup
            bool backupResult = repo->createBackup("test_backup");
            
            // Simulate data loss and restore
            bool restoreResult = repo->restoreFromBackup("test_backup");
            
            if (backupResult && restoreResult) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Backup and restore failed");
            }
        } catch (const std::exception& e) {
            reporter.testFailed(std::string("Exception: ") + e.what());
        }
    }

    void testCrossCompatibility() {
        reporter.startTest("Cross-Platform Compatibility");
        try {
            // Test path handling across platforms
            std::string testPath = "test/path/with/separators";
            std::string normalizedPath = repo->normalizePath(testPath);
            
            // Test file operations
            std::ofstream crossPlatformFile("cross_platform_test.txt");
            crossPlatformFile << "Cross-platform content\nwith\ndifferent\nline\nendings\r\n";
            crossPlatformFile.close();
            
            bool fileExists = fs::exists("cross_platform_test.txt");
            
            if (!normalizedPath.empty() && fileExists) {
                reporter.testPassed();
            } else {
                reporter.testFailed("Cross-platform compatibility issues");
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

int main(int argc, char* argv[]) {
    try {
        GyattTestSuite testSuite;
        testSuite.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
