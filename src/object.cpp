#include "object.h"
#include "utils.h"
#include <sstream>
#include <stdexcept>

namespace gyatt {

GitObject::GitObject(const std::string& repoPath) 
    : repoPath(repoPath), objectsDir(Utils::joinPath(repoPath, ".gyatt/objects")) {
}

std::string GitObject::createBlob(const std::string& content) {
    std::string hash = computeHash(content, ObjectType::BLOB);
    storeObject(hash, content, ObjectType::BLOB);
    return hash;
}

std::string GitObject::createTree(const std::string& content) {
    std::string hash = computeHash(content, ObjectType::TREE);
    storeObject(hash, content, ObjectType::TREE);
    return hash;
}

std::string GitObject::createCommit(const std::string& content) {
    std::string hash = computeHash(content, ObjectType::COMMIT);
    storeObject(hash, content, ObjectType::COMMIT);
    return hash;
}

std::string GitObject::readObject(const std::string& hash, ObjectType& type) {
    std::string content = loadObject(hash);
    auto [objType, objContent] = parseObjectContent(content);
    type = objType;
    return objContent;
}

std::string GitObject::readBlob(const std::string& hash) {
    ObjectType type;
    std::string content = readObject(hash, type);
    if (type != ObjectType::BLOB) {
        throw std::runtime_error("Object is not a blob: " + hash);
    }
    return content;
}

std::string GitObject::readTree(const std::string& hash) {
    ObjectType type;
    std::string content = readObject(hash, type);
    if (type != ObjectType::TREE) {
        throw std::runtime_error("Object is not a tree: " + hash);
    }
    return content;
}

std::string GitObject::readCommit(const std::string& hash) {
    ObjectType type;
    std::string content = readObject(hash, type);
    if (type != ObjectType::COMMIT) {
        throw std::runtime_error("Object is not a commit: " + hash);
    }
    return content;
}

bool GitObject::objectExists(const std::string& hash) {
    return Utils::fileExists(getObjectPath(hash));
}

ObjectType GitObject::getObjectType(const std::string& hash) {
    if (!objectExists(hash)) {
        throw std::runtime_error("Object does not exist: " + hash);
    }
    
    std::string content = loadObject(hash);
    auto [type, _] = parseObjectContent(content);
    return type;
}

std::vector<std::string> GitObject::listObjects() {
    std::vector<std::string> objects;
    
    if (!Utils::directoryExists(objectsDir)) {
        return objects;
    }
    
    auto dirs = Utils::listDirectory(objectsDir);
    for (const auto& dir : dirs) {
        if (dir.length() == 2) {
            std::string dirPath = Utils::joinPath(objectsDir, dir);
            auto files = Utils::listDirectory(dirPath);
            
            for (const auto& file : files) {
                if (file.length() == 38) { // 40 - 2 = 38
                    objects.push_back(dir + file);
                }
            }
        }
    }
    
    return objects;
}

std::string GitObject::computeHash(const std::string& content, ObjectType type) {
    std::string typeStr = objectTypeToString(type);
    std::string formatted = typeStr + " " + std::to_string(content.length()) + "\0" + content;
    return Utils::sha1Hash(formatted);
}

std::string GitObject::objectTypeToString(ObjectType type) {
    switch (type) {
        case ObjectType::BLOB: return "blob";
        case ObjectType::TREE: return "tree";
        case ObjectType::COMMIT: return "commit";
        default: throw std::runtime_error("Unknown object type");
    }
}

ObjectType GitObject::stringToObjectType(const std::string& typeStr) {
    if (typeStr == "blob") return ObjectType::BLOB;
    if (typeStr == "tree") return ObjectType::TREE;
    if (typeStr == "commit") return ObjectType::COMMIT;
    throw std::runtime_error("Unknown object type: " + typeStr);
}

std::string GitObject::getObjectPath(const std::string& hash) {
    if (hash.length() < 2) {
        throw std::runtime_error("Invalid hash length: " + hash);
    }
    
    std::string dir = hash.substr(0, 2);
    std::string file = hash.substr(2);
    return Utils::joinPath(Utils::joinPath(objectsDir, dir), file);
}

std::string GitObject::getObjectDir(const std::string& hash) {
    if (hash.length() < 2) {
        throw std::runtime_error("Invalid hash length: " + hash);
    }
    
    std::string dir = hash.substr(0, 2);
    return Utils::joinPath(objectsDir, dir);
}

bool GitObject::storeObject(const std::string& hash, const std::string& content, ObjectType type) {
    std::string objectPath = getObjectPath(hash);
    std::string objectDir = getObjectDir(hash);
    
    // Create object directory if it doesn't exist
    if (!Utils::createDirectories(objectDir)) {
        return false;
    }
    
    // Don't overwrite existing objects
    if (Utils::fileExists(objectPath)) {
        return true;
    }
    
    std::string formatted = formatObjectContent(content, type);
    return Utils::writeFile(objectPath, formatted);
}

std::string GitObject::loadObject(const std::string& hash) {
    std::string objectPath = getObjectPath(hash);
    
    if (!Utils::fileExists(objectPath)) {
        throw std::runtime_error("Object not found: " + hash);
    }
    
    return Utils::readFile(objectPath);
}

std::string GitObject::formatObjectContent(const std::string& content, ObjectType type) {
    std::string typeStr = objectTypeToString(type);
    return typeStr + " " + std::to_string(content.length()) + "\0" + content;
}

std::pair<ObjectType, std::string> GitObject::parseObjectContent(const std::string& content) {
    size_t nullPos = content.find('\0');
    if (nullPos == std::string::npos) {
        throw std::runtime_error("Invalid object format");
    }
    
    std::string header = content.substr(0, nullPos);
    std::string objectContent = content.substr(nullPos + 1);
    
    size_t spacePos = header.find(' ');
    if (spacePos == std::string::npos) {
        throw std::runtime_error("Invalid object header format");
    }
    
    std::string typeStr = header.substr(0, spacePos);
    std::string sizeStr = header.substr(spacePos + 1);
    
    ObjectType type = stringToObjectType(typeStr);
    size_t expectedSize = std::stoull(sizeStr);
    
    if (objectContent.length() != expectedSize) {
        throw std::runtime_error("Object content size mismatch");
    }
    
    return {type, objectContent};
}

} // namespace gyatt
