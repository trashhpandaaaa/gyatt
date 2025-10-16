#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define mkdir(path, mode) _mkdir(path)
    #define stat _stat
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#else
    #include <unistd.h>
#endif

int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && !S_ISDIR(st.st_mode);
}

int dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int mkdir_recursive(const char *path) {
    char *tmp = str_duplicate(path);
    char *p = NULL;
    size_t len;

    len = strlen(tmp);
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
            if (!dir_exists(tmp)) {
                if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                    free(tmp);
                    return -1;
                }
            }
            *p = '/';
        }
    }

    if (!dir_exists(tmp)) {
        if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
            free(tmp);
            return -1;
        }
    }

    free(tmp);
    return 0;
}

char *read_file(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    if (!content) {
        fclose(f);
        return NULL;
    }

    size_t read_size = fread(content, 1, fsize, f);
    fclose(f);

    content[read_size] = 0;
    if (size) *size = read_size;

    return content;
}

int write_file(const char *path, const void *data, size_t size) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    size_t written = fwrite(data, 1, size, f);
    fclose(f);

    return (written == size) ? 0 : -1;
}

char *str_concat(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    if (!result) return NULL;

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);

    return result;
}

char *str_duplicate(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *dup = malloc(len + 1);
    if (!dup) return NULL;
    memcpy(dup, str, len + 1);
    return dup;
}

void str_trim(char *str) {
    if (!str) return;
    
    // Trim leading whitespace
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r'))
        start++;
    
    if (start != str)
        memmove(str, start, strlen(start) + 1);
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
}

char *path_join(const char *p1, const char *p2) {
    size_t len1 = strlen(p1);
    size_t len2 = strlen(p2);
    
    // Check if p1 ends with separator
    int needs_sep = (len1 > 0 && p1[len1-1] != '/' && p1[len1-1] != '\\');
    
    char *result = malloc(len1 + len2 + 2);
    if (!result) return NULL;
    
    strcpy(result, p1);
    if (needs_sep) {
        result[len1] = '/';
        strcpy(result + len1 + 1, p2);
    } else {
        strcpy(result + len1, p2);
    }
    
    return result;
}

char *get_current_dir(void) {
    char *buffer = malloc(4096);
    if (!buffer) return NULL;
    
#ifdef _WIN32
    if (!_getcwd(buffer, 4096)) {
#else
    if (!getcwd(buffer, 4096)) {
#endif
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

// Find the root of the Gyatt repository
char *find_repo_root(void) {
    char *current = get_current_dir();
    if (!current) return NULL;
    
    char *search = str_duplicate(current);
    free(current);
    
    while (1) {
        char *gyatt_dir = path_join(search, ".gyatt");
        
        if (dir_exists(gyatt_dir)) {
            free(gyatt_dir);
            return search;
        }
        
        free(gyatt_dir);
        
        // Go up one directory
        char *parent = search;
        char *last_sep = strrchr(parent, '/');
        
        #ifdef _WIN32
        char *last_sep_win = strrchr(parent, '\\');
        if (last_sep_win > last_sep) last_sep = last_sep_win;
        #endif
        
        if (!last_sep || last_sep == parent) {
            // Reached root without finding .gyatt
            free(search);
            return NULL;
        }
        
        *last_sep = '\0';
    }
}

// Check if we're in a Gyatt repository
int is_gyatt_repo(void) {
    char *root = find_repo_root();
    if (root) {
        free(root);
        return 1;
    }
    return 0;
}

// Get the .gyatt directory path
char *get_gyatt_dir(void) {
    char *root = find_repo_root();
    if (!root) return NULL;
    
    char *gyatt_dir = path_join(root, ".gyatt");
    free(root);
    
    return gyatt_dir;
}
