#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 2 interface for high-performance operations
 */
void initializeModule2();

/**
 * @brief Module 2 performance benchmarking
 */
class Module2Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
