#include "utils.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <openssl/sha.h>

namespace gyatt {
namespace Utils {

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += delimiter + strings[i];
    }
    
    return result;
}

bool fileExists(const std::string& filepath) {
    return std::filesystem::exists(filepath) && std::filesystem::is_regular_file(filepath);
}

bool directoryExists(const std::string& dirpath) {
    return std::filesystem::exists(dirpath) && std::filesystem::is_directory(dirpath);
}

bool createDirectory(const std::string& dirpath) {
    try {
        return std::filesystem::create_directory(dirpath);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

bool createDirectories(const std::string& dirpath) {
    try {
        return std::filesystem::create_directories(dirpath);
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool writeFile(const std::string& filepath, const std::string& content) {
    try {
        // Create parent directories if they don't exist
        std::filesystem::path path(filepath);
        if (path.has_parent_path()) {
            createDirectories(path.parent_path().string());
        }
        
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        file << content;
        return file.good();
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<std::string> listDirectory(const std::string& dirpath) {
    std::vector<std::string> files;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dirpath)) {
            files.push_back(entry.path().filename().string());
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Return empty vector on error
    }
    
    std::sort(files.begin(), files.end());
    return files;
}

bool isDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

std::string getFileName(const std::string& filepath) {
    return std::filesystem::path(filepath).filename().string();
}

std::string getParentPath(const std::string& filepath) {
    return std::filesystem::path(filepath).parent_path().string();
}

std::string sha1Hash(const std::string& content) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.length(), hash);
    
    std::ostringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::ostringstream ss;
    for (unsigned char byte : bytes) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

std::string formatTime(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point parseTime(const std::string& timeStr) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    auto time_t = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time_t);
}

std::string joinPath(const std::string& path1, const std::string& path2) {
    std::filesystem::path p1(path1);
    std::filesystem::path p2(path2);
    return (p1 / p2).string();
}

std::string relativePath(const std::string& from, const std::string& to) {
    try {
        return std::filesystem::relative(to, from).string();
    } catch (const std::filesystem::filesystem_error&) {
        return to;
    }
}

std::string absolutePath(const std::string& path) {
    try {
        return std::filesystem::absolute(path).string();
    } catch (const std::filesystem::filesystem_error&) {
        return path;
    }
}

std::string normalizePath(const std::string& path) {
    try {
        return std::filesystem::weakly_canonical(path).string();
    } catch (const std::filesystem::filesystem_error&) {
        return path;
    }
}

bool isValidHash(const std::string& hash) {
    if (hash.length() != 40) return false;
    
    return std::all_of(hash.begin(), hash.end(), [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    });
}

std::string shortHash(const std::string& hash, size_t length) {
    return hash.substr(0, std::min(length, hash.length()));
}

std::string getUserName() {
    const char* name = std::getenv("USER");
    if (name) return std::string(name);
    
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    
    return "unknown";
}

std::string getUserEmail() {
    const char* email = std::getenv("EMAIL");
    if (email) return std::string(email);
    
    return getUserName() + "@localhost";
}

std::string getAuthorString() {
    return getUserName() + " <" + getUserEmail() + ">";
}

} // namespace Utils
} // namespace gyatt
