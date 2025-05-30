// Module 8 header
#pragma once
#include <vector>
#include <string>

class Module8 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module8();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
