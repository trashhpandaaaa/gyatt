// Module 4 header
#pragma once
#include <vector>
#include <string>

class Module4 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module4();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
