#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 5 interface for high-performance operations
 */
void initializeModule5();

/**
 * @brief Module 5 performance benchmarking
 */
class Module5Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
