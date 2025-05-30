// Module 19 header
#pragma once
#include <vector>
#include <string>

class Module19 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module19();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
