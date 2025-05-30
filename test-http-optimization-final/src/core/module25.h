#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 25 interface for high-performance operations
 */
void initializeModule25();

/**
 * @brief Module 25 performance benchmarking
 */
class Module25Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
