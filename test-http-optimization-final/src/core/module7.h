#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 7 interface for high-performance operations
 */
void initializeModule7();

/**
 * @brief Module 7 performance benchmarking
 */
class Module7Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
