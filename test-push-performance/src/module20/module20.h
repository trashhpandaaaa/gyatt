// Module 20 header
#pragma once
#include <vector>
#include <string>

class Module20 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module20();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
