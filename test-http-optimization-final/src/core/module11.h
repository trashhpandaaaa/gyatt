#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 11 interface for high-performance operations
 */
void initializeModule11();

/**
 * @brief Module 11 performance benchmarking
 */
class Module11Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
