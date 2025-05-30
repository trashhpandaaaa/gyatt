#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 28 interface for high-performance operations
 */
void initializeModule28();

/**
 * @brief Module 28 performance benchmarking
 */
class Module28Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
