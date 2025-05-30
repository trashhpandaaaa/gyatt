#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 17 interface for high-performance operations
 */
void initializeModule17();

/**
 * @brief Module 17 performance benchmarking
 */
class Module17Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
