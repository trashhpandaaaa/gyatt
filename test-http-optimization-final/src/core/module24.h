#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 24 interface for high-performance operations
 */
void initializeModule24();

/**
 * @brief Module 24 performance benchmarking
 */
class Module24Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
