#include "include/http_optimization.h"
#include "include/utils.h"
#include <iostream>

int main() {
    std::cout << "Testing HTTP optimization with GitHub blob creation...\n";
    
    // Test standard utils HTTP functions first
    std::cout << "\n1. Testing standard Utils::httpGet...\n";
    auto response = gyatt::Utils::httpGet("https://api.github.com/zen");
    std::cout << "Response code: " << response.responseCode << "\n";
    std::cout << "Success: " << (response.success ? "true" : "false") << "\n";
    std::cout << "Content: " << response.content.substr(0, 100) << "...\n";
    std::cout << "Error: " << response.error << "\n";
    
    // Test HTTP optimization
    std::cout << "\n2. Testing HttpOptimization::httpGet...\n";
    gyatt::HttpOptimization httpOpt;
    auto optResponse = httpOpt.httpGet("https://api.github.com/zen");
    std::cout << "Response code: " << optResponse.responseCode << "\n";
    std::cout << "Success: " << (optResponse.success ? "true" : "false") << "\n";
    std::cout << "Content: " << optResponse.content.substr(0, 100) << "...\n";
    std::cout << "Error: " << optResponse.error << "\n";
    std::cout << "Transfer time: " << optResponse.transferTime << "s\n";
    
    // Test GitHub API with token
    std::string token;
    try {
        token = gyatt::Utils::readFile("/home/trashhpandaaaa/Documents/Code/Cpp/gyatt/.gyatt/github_token");
        token = gyatt::Utils::trim(token);
    } catch (const std::exception& e) {
        std::cout << "Could not read GitHub token: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\n3. Testing GitHub API with token...\n";
    std::vector<std::string> headers = {
        "Authorization: token " + token,
        "Accept: application/vnd.github.v3+json"
    };
    
    auto ghResponse = httpOpt.httpGet("https://api.github.com/user", headers);
    std::cout << "Response code: " << ghResponse.responseCode << "\n";
    std::cout << "Success: " << (ghResponse.success ? "true" : "false") << "\n";
    std::cout << "Content: " << ghResponse.content.substr(0, 200) << "...\n";
    std::cout << "Error: " << ghResponse.error << "\n";
    
    // Test blob creation
    std::cout << "\n4. Testing GitHub blob creation...\n";
    std::vector<std::string> blobHeaders = {
        "Authorization: token " + token,
        "Accept: application/vnd.github.v3+json",
        "Content-Type: application/json"
    };
    
    std::string blobData = "{\"content\":\"SGVsbG8gZnJvbSBneWF0dCE=\",\"encoding\":\"base64\"}";
    auto blobResponse = httpOpt.httpPost("https://api.github.com/repos/trashhpandaaaa/gyatt/git/blobs", blobData, blobHeaders);
    
    std::cout << "Response code: " << blobResponse.responseCode << "\n";
    std::cout << "Success: " << (blobResponse.success ? "true" : "false") << "\n";
    std::cout << "Content: " << blobResponse.content.substr(0, 200) << "...\n";
    std::cout << "Error: " << blobResponse.error << "\n";
    
    return 0;
}
