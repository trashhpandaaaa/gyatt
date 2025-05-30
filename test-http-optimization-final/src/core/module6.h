#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 6 interface for high-performance operations
 */
void initializeModule6();

/**
 * @brief Module 6 performance benchmarking
 */
class Module6Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
