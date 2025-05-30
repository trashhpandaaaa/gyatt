#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 23 interface for high-performance operations
 */
void initializeModule23();

/**
 * @brief Module 23 performance benchmarking
 */
class Module23Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
