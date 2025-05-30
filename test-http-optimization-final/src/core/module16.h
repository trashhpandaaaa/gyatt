#pragma once

#include <vector>
#include <string>

namespace gyatt {
namespace core {

/**
 * @brief Module 16 interface for high-performance operations
 */
void initializeModule16();

/**
 * @brief Module 16 performance benchmarking
 */
class Module16Benchmark {
public:
    static void runBenchmark();
    static double measurePerformance();
    static void printResults();
};

} // namespace core
} // namespace gyatt
