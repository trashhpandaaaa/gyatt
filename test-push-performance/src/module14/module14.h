// Module 14 header
#pragma once
#include <vector>
#include <string>

class Module14 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module14();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
