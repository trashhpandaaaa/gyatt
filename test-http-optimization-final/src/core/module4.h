#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 4 interface for high-performance operations
 */
void initializeModule4();

/**
 * @brief Module 4 performance benchmarking
 */
class Module4Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
