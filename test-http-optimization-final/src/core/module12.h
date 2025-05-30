#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 12 interface for high-performance operations
 */
void initializeModule12();

/**
 * @brief Module 12 performance benchmarking
 */
class Module12Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
