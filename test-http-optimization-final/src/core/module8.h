#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 8 interface for high-performance operations
 */
void initializeModule8();

/**
 * @brief Module 8 performance benchmarking
 */
class Module8Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
