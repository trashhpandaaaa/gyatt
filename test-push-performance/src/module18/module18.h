// Module 18 header
#pragma once
#include <vector>
#include <string>

class Module18 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module18();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
