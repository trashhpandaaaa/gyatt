// Module 12 header
#pragma once
#include <vector>
#include <string>

class Module12 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module12();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
