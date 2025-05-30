// Module 15 header
#pragma once
#include <vector>
#include <string>

class Module15 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module15();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
