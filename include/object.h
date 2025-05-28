#pragma once

#include <string>
#include <vector>

namespace gyatt {

enum class ObjectType {
    BLOB,
    TREE,
    COMMIT
};

class GitObject {
public:
    GitObject(const std::string& repoPath);
    
    std::string createBlob(const std::string& content);
    std::string createTree(const std::string& content);
    std::string createCommit(const std::string& content);
    
    std::string readObject(const std::string& hash, ObjectType& type);
    std::string readBlob(const std::string& hash);
    std::string readTree(const std::string& hash);
    std::string readCommit(const std::string& hash);
    
    bool objectExists(const std::string& hash);
    ObjectType getObjectType(const std::string& hash);
    std::vector<std::string> listObjects();
    
    static std::string computeHash(const std::string& content, ObjectType type);
    static std::string objectTypeToString(ObjectType type);
    static ObjectType stringToObjectType(const std::string& typeStr);
    
private:
    std::string repoPath;
    std::string objectsDir;
    
    std::string getObjectPath(const std::string& hash);
    std::string getObjectDir(const std::string& hash);
    bool storeObject(const std::string& hash, const std::string& content, ObjectType type);
    std::string loadObject(const std::string& hash);
    std::string formatObjectContent(const std::string& content, ObjectType type);
    std::pair<ObjectType, std::string> parseObjectContent(const std::string& content);
};

} // namespace gyatt
