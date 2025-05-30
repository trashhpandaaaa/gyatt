// Module 1 header
#pragma once
#include <vector>
#include <string>

class Module1 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module1();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
