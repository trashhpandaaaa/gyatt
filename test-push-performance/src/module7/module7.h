// Module 7 header
#pragma once
#include <vector>
#include <string>

class Module7 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module7();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
