// Module 16 header
#pragma once
#include <vector>
#include <string>

class Module16 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module16();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
