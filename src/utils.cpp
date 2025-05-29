#include "utils.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <pwd.h>
#include <openssl/sha.h>
#include <curl/curl.h>
#include <regex>
#include <iostream>

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

std::string getGitConfigValue(const std::string& key) {
    std::string command = "git config --get " + key + " 2>/dev/null";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

std::string getUserName() {
    // Try to read from git config first
    std::string gitName = getGitConfigValue("user.name");
    if (!gitName.empty()) {
        return gitName;
    }
    
    // Fall back to environment variable
    const char* name = std::getenv("USER");
    if (name) return std::string(name);
    
    // Fall back to system user
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    
    return "unknown";
}

std::string getUserEmail() {
    // Try to read from git config first
    std::string gitEmail = getGitConfigValue("user.email");
    if (!gitEmail.empty()) {
        return gitEmail;
    }
    
    // Fall back to environment variable
    const char* email = std::getenv("EMAIL");
    if (email) return std::string(email);
    
    // Fall back to default (but this should ideally not happen)
    return getUserName() + "@localhost";
}

std::string getAuthorString() {
    return getUserName() + " <" + getUserEmail() + ">";
}

static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

bool isHttpUrl(const std::string& url) {
    return url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://";
}

bool isGitHubUrl(const std::string& url) {
    if (url.find("github.com") == std::string::npos) {
        return false;
    }
    
    std::regex githubRegex(R"((?:https?://)?(?:www\.)?github\.com/[^/]+/[^/]+?(?:\.git)?/?$)");
    return std::regex_match(url, githubRegex);
}

std::string parseGitHubRepoName(const std::string& url) {
    std::regex githubRegex(R"((?:https?://)?(?:www\.)?github\.com/([^/]+/[^/]+?)(?:\.git)?/?$)");
    std::smatch match;
    
    if (std::regex_search(url, match, githubRegex)) {
        return match[1].str();
    }
    
    return "";
}

HttpResponse httpGet(const std::string& url, const std::vector<std::string>& headers) {
    HttpResponse response;
    response.success = false;
    response.responseCode = 0;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.content);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gyatt/1.0");
    
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
        response.success = (response.responseCode >= 200 && response.responseCode < 300);
    } else {
        response.error = curl_easy_strerror(res);
    }
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    
    return response;
}

HttpResponse httpPost(const std::string& url, const std::string& data, const std::vector<std::string>& headers) {
    HttpResponse response;
    response.success = false;
    response.responseCode = 0;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.content);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gyatt/1.0");
    
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
        response.success = (response.responseCode >= 200 && response.responseCode < 300);
    } else {
        response.error = curl_easy_strerror(res);
    }
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    
    return response;
}

HttpResponse httpPatch(const std::string& url, const std::string& data, const std::vector<std::string>& headers) {
    HttpResponse response;
    response.success = false;
    response.responseCode = 0;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.content);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gyatt/1.0");
    
    // Add custom headers
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
        response.success = (response.responseCode >= 200 && response.responseCode < 300);
    } else {
        response.error = curl_easy_strerror(res);
    }
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    
    return response;
}

HttpResponse httpPut(const std::string& url, const std::string& data, const std::vector<std::string>& headers) {
    HttpResponse response;
    response.success = false;
    response.responseCode = 0;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.content);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "gyatt/1.0");
    
    // Add custom headers
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
        response.success = (response.responseCode >= 200 && response.responseCode < 300);
    } else {
        response.error = curl_easy_strerror(res);
    }
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    
    return response;
}

std::string urlEncode(const std::string& str) {
    CURL* curl = curl_easy_init();
    if (!curl) return str;
    
    char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    
    return result;
}

std::string base64Encode(const std::string& data) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    
    for (size_t i = 0; i < data.length(); i += 3) {
        int n = (data[i] << 16) + ((i + 1 < data.length()) ? (data[i + 1] << 8) : 0) + ((i + 2 < data.length()) ? data[i + 2] : 0);
        
        result += chars[(n >> 18) & 63];
        result += chars[(n >> 12) & 63];
        result += (i + 1 < data.length()) ? chars[(n >> 6) & 63] : '=';
        result += (i + 2 < data.length()) ? chars[n & 63] : '=';
    }
    
    return result;
}

std::string base64Decode(const std::string& encoded) {
    static const int T[128] = {
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
        52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
        -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
        15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
        -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
        41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
    };
    
    std::string result;
    int pad = 0;
    
    for (size_t i = 0; i < encoded.length(); i += 4) {
        int n = 0;
        for (int j = 0; j < 4; ++j) {
            if (i + j < encoded.length()) {
                char c = encoded[i + j];
                if (c == '=') {
                    pad++;
                } else if (static_cast<unsigned char>(c) < 128) {
                    n = (n << 6) + T[static_cast<unsigned char>(c)];
                }
            }
        }
        
        if (pad < 2) result += char((n >> 16) & 255);
        if (pad < 1) result += char((n >> 8) & 255);
        if (pad < 1) result += char(n & 255);
    }
    
    return result;
}

// Archive utilities implementation
bool extractZipData(const std::string& zipData, const std::string& targetDir) {
    try {
        // Write ZIP data to a temporary file
        std::string tempZipPath = Utils::joinPath("/tmp", "gyatt_temp_" + std::to_string(std::time(nullptr)) + ".zip");
        
        std::cout << "Writing ZIP data to: " << tempZipPath << std::endl;
        if (!writeDataToFile(tempZipPath, zipData)) {
            std::cerr << "Failed to write ZIP data to temporary file" << std::endl;
            return false;
        }
        
        std::cout << "ZIP file written, size: " << zipData.size() << " bytes" << std::endl;
        
        // Create target directory if it doesn't exist
        if (!Utils::directoryExists(targetDir)) {
            std::cout << "Creating target directory: " << targetDir << std::endl;
            if (!Utils::createDirectories(targetDir)) {
                std::cerr << "Failed to create target directory: " << targetDir << std::endl;
                std::filesystem::remove(tempZipPath);
                return false;
            }
        }
        
        // Extract using system unzip command
        std::string command = "cd '" + targetDir + "' && unzip -q -o '" + tempZipPath + "' 2>&1";
        std::cout << "Running command: " << command << std::endl;
        
        // Capture the output of the command
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run unzip command" << std::endl;
            std::filesystem::remove(tempZipPath);
            return false;
        }
        
        char buffer[128];
        std::string commandOutput;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            commandOutput += buffer;
        }
        int result = pclose(pipe);
        
        if (result != 0) {
            std::cerr << "Unzip command failed with code " << result << std::endl;
            if (!commandOutput.empty()) {
                std::cerr << "Command output: " << commandOutput << std::endl;
            }
            
            // Try to list the content of the ZIP file to see what's inside
            std::string listCommand = "unzip -l '" + tempZipPath + "' 2>&1";
            std::cout << "Listing ZIP contents: " << listCommand << std::endl;
            
            FILE* listPipe = popen(listCommand.c_str(), "r");
            if (listPipe) {
                while (fgets(buffer, sizeof(buffer), listPipe) != nullptr) {
                    std::cout << buffer;
                }
                pclose(listPipe);
            }
            
            std::filesystem::remove(tempZipPath);
            return false;
        }
        
        std::cout << "ZIP extraction completed successfully" << std::endl;
        
        // Clean up temporary file
        std::filesystem::remove(tempZipPath);
        
        // Find the extracted directory (GitHub zips create a directory like "repo-main")
        auto directories = Utils::listDirectory(targetDir);
        std::string extractedDir;
        for (const auto& dir : directories) {
            std::string fullPath = Utils::joinPath(targetDir, dir);
            if (Utils::isDirectory(fullPath) && dir != "." && dir != ".." && dir != ".gyatt") {
                extractedDir = fullPath;
                std::cout << "Found extracted directory: " << extractedDir << std::endl;
                break;
            }
        }
        
        if (extractedDir.empty()) {
            std::cerr << "No extracted directory found in: " << targetDir << std::endl;
            // Try to list the directory contents
            std::cout << "Directory contents:" << std::endl;
            for (const auto& entry : directories) {
                std::cout << "- " << entry << std::endl;
            }
            
            // If there are files directly in the target directory, we don't need to move anything
            bool hasFiles = false;
            for (const auto& entry : directories) {
                if (entry != "." && entry != ".." && entry != ".gyatt") {
                    std::string fullPath = Utils::joinPath(targetDir, entry);
                    if (!Utils::isDirectory(fullPath)) {
                        hasFiles = true;
                        break;
                    }
                }
            }
            
            if (hasFiles) {
                std::cout << "Files found directly in target directory, skipping move step" << std::endl;
                return true;
            }
            
            return false;
        }
        
        // Move contents from extracted directory to target directory
        std::cout << "Moving contents from " << extractedDir << " to " << targetDir << std::endl;
        for (const auto& entry : std::filesystem::directory_iterator(extractedDir)) {
            std::string filename = entry.path().filename().string();
            std::string targetPath = Utils::joinPath(targetDir, filename);
            
            std::cout << "Moving: " << filename << " -> " << targetPath << std::endl;
            
            if (entry.is_directory()) {
                std::filesystem::copy(entry.path(), targetPath, 
                    std::filesystem::copy_options::recursive | 
                    std::filesystem::copy_options::overwrite_existing);
            } else {
                std::filesystem::copy_file(entry.path(), targetPath, 
                    std::filesystem::copy_options::overwrite_existing);
            }
        }
        
        // Remove the now-empty extracted directory
        std::filesystem::remove_all(extractedDir);
        std::cout << "Cleanup completed" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in extractZipData: " << e.what() << std::endl;
        return false;
    }
}

bool writeDataToFile(const std::string& filepath, const std::string& data) {
    try {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        file.write(data.c_str(), data.length());
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

} // namespace Utils
} // namespace gyatt
