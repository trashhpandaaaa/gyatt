// Module 17 header
#pragma once
#include <vector>
#include <string>

class Module17 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module17();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
