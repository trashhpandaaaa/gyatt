#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 18 interface for high-performance operations
 */
void initializeModule18();

/**
 * @brief Module 18 performance benchmarking
 */
class Module18Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
