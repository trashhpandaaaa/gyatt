// Module 6 header
#pragma once
#include <vector>
#include <string>

class Module6 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module6();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
