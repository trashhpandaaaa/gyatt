// Module 2 header
#pragma once
#include <vector>
#include <string>

class Module2 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module2();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
