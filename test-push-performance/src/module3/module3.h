// Module 3 header
#pragma once
#include <vector>
#include <string>

class Module3 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module3();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
