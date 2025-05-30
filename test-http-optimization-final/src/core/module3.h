#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 3 interface for high-performance operations
 */
void initializeModule3();

/**
 * @brief Module 3 performance benchmarking
 */
class Module3Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
