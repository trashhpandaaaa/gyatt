#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 14 interface for high-performance operations
 */
void initializeModule14();

/**
 * @brief Module 14 performance benchmarking
 */
class Module14Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
