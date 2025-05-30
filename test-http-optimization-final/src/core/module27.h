#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 27 interface for high-performance operations
 */
void initializeModule27();

/**
 * @brief Module 27 performance benchmarking
 */
class Module27Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
