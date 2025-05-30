#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 22 interface for high-performance operations
 */
void initializeModule22();

/**
 * @brief Module 22 performance benchmarking
 */
class Module22Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
