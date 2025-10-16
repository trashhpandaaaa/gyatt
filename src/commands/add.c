#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../gyatt.h"
#include "../index.h"
#include "../utils.h"

static int should_ignore(const char *path) {
    // Always ignore .gyatt directory
    if (strstr(path, ".gyatt") != NULL) {
        return 1;
    }
    
    // Check .gyattignore patterns (simple implementation)
    // TODO: Implement full .gyattignore parsing
    
    return 0;
}

static int add_directory_recursive(index_t *index, const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Error: Cannot open directory '%s'\n", dir_path);
        return -1;
    }
    
    int added = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char *entry_path = path_join(dir_path, entry->d_name);
        
        if (should_ignore(entry_path)) {
            free(entry_path);
            continue;
        }
        
        struct stat st;
        if (stat(entry_path, &st) != 0) {
            free(entry_path);
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            // Recurse into subdirectory
            int result = add_directory_recursive(index, entry_path);
            if (result >= 0) {
                added += result;
            }
        } else if (S_ISREG(st.st_mode)) {
            // Add file
            if (index_add_file(index, entry_path) == 0) {
                printf("add '%s'\n", entry_path);
                added++;
            }
        }
        
        free(entry_path);
    }
    
    closedir(dir);
    return added;
}

int cmd_add(int argc, char *argv[]) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Error: Not a Gyatt repository\n");
        fprintf(stderr, "Run 'gyatt init' to create a repository\n");
        return 1;
    }
    
    if (argc < 2) {
        fprintf(stderr, "Error: No files specified\n");
        fprintf(stderr, "Usage: gyatt add <file>...\n");
        fprintf(stderr, "       gyatt add .           # Add all files\n");
        return 1;
    }
    
    // Load the index
    index_t *index = index_create();
    if (!index) {
        fprintf(stderr, "Error: Failed to create index\n");
        return 1;
    }
    
    if (index_read(index) != 0) {
        fprintf(stderr, "Warning: Could not read existing index, starting fresh\n");
    }
    
    int total_added = 0;
    
    // Process each argument
    for (int i = 1; i < argc; i++) {
        const char *path = argv[i];
        
        struct stat st;
        if (stat(path, &st) != 0) {
            fprintf(stderr, "Error: '%s' does not exist\n", path);
            continue;
        }
        
        if (should_ignore(path)) {
            printf("Ignoring '%s'\n", path);
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            // Add directory recursively
            int added = add_directory_recursive(index, path);
            if (added >= 0) {
                total_added += added;
            }
        } else if (S_ISREG(st.st_mode)) {
            // Add single file
            if (index_add_file(index, path) == 0) {
                printf("add '%s'\n", path);
                total_added++;
            }
        } else {
            fprintf(stderr, "Warning: Skipping '%s' (not a regular file)\n", path);
        }
    }
    
    // Write the updated index
    if (index_write(index) != 0) {
        fprintf(stderr, "Error: Failed to write index\n");
        index_free(index);
        return 1;
    }
    
    printf("\n%d file(s) staged for commit\n", total_added);
    
    index_free(index);
    return 0;
}
