// Module 9 header
#pragma once
#include <vector>
#include <string>

class Module9 {
private:
    std::vector<std::string> data;
    int moduleId;
    
public:
    Module9();
    void processData();
    int getModuleId() const;
    size_t getDataSize() const;
};
