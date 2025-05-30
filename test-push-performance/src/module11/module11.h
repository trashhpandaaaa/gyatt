// Module 11 header
#pragma once
#include <vector>
#include <string>

class Module11 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module11();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
