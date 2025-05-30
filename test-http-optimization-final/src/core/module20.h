#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 20 interface for high-performance operations
 */
void initializeModule20();

/**
 * @brief Module 20 performance benchmarking
 */
class Module20Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
