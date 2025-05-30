#include <iostream>
#include "http_optimization.h"

using namespace gyatt;

int main() {
    std::cout << "Testing basic HTTP optimization functionality...\n";
    
    try {
        HttpOptimization httpOpt;
        
        // Configure
        HttpOptimization::ConnectionPoolConfig config;
        config.maxConnections = 5;
        config.maxConnectionsPerHost = 3;
        config.connectionTimeout = 10L;
        config.requestTimeout = 30L;
        
        httpOpt.setConfig(config);
        
        std::cout << "✅ HTTP optimization object created and configured successfully\n";
        
        // Test a simple GET request
        std::cout << "Testing simple GET request...\n";
        auto response = httpOpt.httpGet("https://httpbin.org/get");
        
        std::cout << "Response received:\n";
        std::cout << "  Success: " << (response.success ? "yes" : "no") << "\n";
        std::cout << "  Response code: " << response.responseCode << "\n";
        std::cout << "  Content length: " << response.content.length() << "\n";
        std::cout << "  Transfer time: " << response.transferTime << "ms\n";
        std::cout << "  From cache: " << (response.fromCache ? "yes" : "no") << "\n";
        
        if (!response.success) {
            std::cout << "  Error: " << response.error << "\n";
        }
        
        // Get stats
        auto stats = httpOpt.getStats();
        std::cout << "\nStats:\n";
        std::cout << "  Total requests: " << stats.totalRequests << "\n";
        std::cout << "  Cache hits: " << stats.cacheHits << "\n";
        std::cout << "  Pool size: " << stats.poolSize << "\n";
        
        std::cout << "\n✅ Basic test completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
