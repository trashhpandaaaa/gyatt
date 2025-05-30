// Module 10 header
#pragma once
#include <vector>
#include <string>

class Module10 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module10();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
