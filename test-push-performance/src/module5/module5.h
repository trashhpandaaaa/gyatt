// Module 5 header
#pragma once
#include <vector>
#include <string>

class Module5 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module5();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
