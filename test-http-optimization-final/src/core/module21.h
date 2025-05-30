#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 21 interface for high-performance operations
 */
void initializeModule21();

/**
 * @brief Module 21 performance benchmarking
 */
class Module21Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
