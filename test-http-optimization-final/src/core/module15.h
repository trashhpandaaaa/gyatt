#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 15 interface for high-performance operations
 */
void initializeModule15();

/**
 * @brief Module 15 performance benchmarking
 */
class Module15Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
