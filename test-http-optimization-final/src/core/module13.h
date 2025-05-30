#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 13 interface for high-performance operations
 */
void initializeModule13();

/**
 * @brief Module 13 performance benchmarking
 */
class Module13Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
