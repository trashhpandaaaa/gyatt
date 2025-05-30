#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 9 interface for high-performance operations
 */
void initializeModule9();

/**
 * @brief Module 9 performance benchmarking
 */
class Module9Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
