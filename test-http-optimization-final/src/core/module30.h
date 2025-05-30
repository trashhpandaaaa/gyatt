#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 30 interface for high-performance operations
 */
void initializeModule30();

/**
 * @brief Module 30 performance benchmarking
 */
class Module30Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
