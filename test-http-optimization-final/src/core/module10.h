#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 10 interface for high-performance operations
 */
void initializeModule10();

/**
 * @brief Module 10 performance benchmarking
 */
class Module10Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
