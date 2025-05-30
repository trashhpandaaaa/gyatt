// Module 13 header
#pragma once
#include <vector>
#include <string>

class Module13 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module13();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
