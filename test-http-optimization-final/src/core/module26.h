#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 26 interface for high-performance operations
 */
void initializeModule26();

/**
 * @brief Module 26 performance benchmarking
 */
class Module26Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
