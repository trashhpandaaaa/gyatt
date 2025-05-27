#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace gyatt {

namespace Utils {
    // String utilities
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
    
    // File utilities
    bool fileExists(const std::string& filepath);
    bool directoryExists(const std::string& dirpath);
    bool createDirectory(const std::string& dirpath);
    bool createDirectories(const std::string& dirpath);
    std::string readFile(const std::string& filepath);
    bool writeFile(const std::string& filepath, const std::string& content);
    std::vector<std::string> listDirectory(const std::string& dirpath);
    bool isDirectory(const std::string& path);
    std::string getFileName(const std::string& filepath);
    std::string getParentPath(const std::string& filepath);
    
    // Hash utilities
    std::string sha1Hash(const std::string& content);
    std::string bytesToHex(const std::vector<unsigned char>& bytes);
    
    // Time utilities
    std::string formatTime(const std::chrono::system_clock::time_point& time);
    std::chrono::system_clock::time_point parseTime(const std::string& timeStr);
    
    // Path utilities
    std::string joinPath(const std::string& path1, const std::string& path2);
    std::string relativePath(const std::string& from, const std::string& to);
    std::string absolutePath(const std::string& path);
    std::string normalizePath(const std::string& path);
    
    // Git-specific utilities
    bool isValidHash(const std::string& hash);
    std::string shortHash(const std::string& hash, size_t length = 7);
    
    // User utilities
    std::string getUserName();
    std::string getUserEmail();
    std::string getAuthorString();
    
    // HTTP utilities
    struct HttpResponse {
        std::string content;
        long responseCode;
        bool success;
        std::string error;
    };
    
    bool isHttpUrl(const std::string& url);
    bool isGitHubUrl(const std::string& url);
    std::string parseGitHubRepoName(const std::string& url);
    HttpResponse httpGet(const std::string& url, const std::vector<std::string>& headers = {});
    HttpResponse httpPost(const std::string& url, const std::string& data, const std::vector<std::string>& headers = {});
    std::string urlEncode(const std::string& str);
    
    // Encoding utilities
    std::string base64Encode(const std::string& data);
    std::string base64Decode(const std::string& encoded);
    
    // Archive utilities
    bool extractZipData(const std::string& zipData, const std::string& targetDir);
    bool writeDataToFile(const std::string& filepath, const std::string& data);
}

} // namespace gyatt
