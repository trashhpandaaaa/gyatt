#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 1 interface for high-performance operations
 */
void initializeModule1();

/**
 * @brief Module 1 performance benchmarking
 */
class Module1Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
