#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../gyatt.h"
#include "../utils.h"
#include "../hash.h"

#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

// Helper to get current branch name
static char *get_current_branch(void) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return NULL;
    
    char head_path[PATH_MAX];
    snprintf(head_path, sizeof(head_path), "%s/HEAD", gyatt_dir);
    
    size_t size = 0;
    char *head_content = read_file(head_path, &size);
    free(gyatt_dir);
    
    if (!head_content) return NULL;
    
    // Parse "ref: refs/heads/branch_name"
    char *ref_start = strstr(head_content, "refs/heads/");
    if (!ref_start) {
        free(head_content);
        return NULL;
    }
    
    ref_start += strlen("refs/heads/");
    char *newline = strchr(ref_start, '\n');
    if (newline) *newline = '\0';
    
    char *branch = str_duplicate(ref_start);
    free(head_content);
    
    return branch;
}

// Helper to get current HEAD commit hash
static int get_head_commit_hash(gyatt_hash_t *hash) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return -1;
    
    char *current_branch = get_current_branch();
    if (!current_branch) {
        free(gyatt_dir);
        return -1;
    }
    
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/refs/heads/%s", gyatt_dir, current_branch);
    free(current_branch);
    free(gyatt_dir);
    
    size_t size = 0;
    char *hash_str = read_file(branch_path, &size);
    if (!hash_str) {
        // No commits yet
        memset(hash, 0, sizeof(gyatt_hash_t));
        return 0;
    }
    
    hex_to_hash(hash_str, hash);
    free(hash_str);
    
    return 0;
}

// List all branches
static int list_branches(void) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        fprintf(stderr, "Error: Not a gyatt repository\n");
        return 1;
    }
    
    char refs_path[PATH_MAX];
    snprintf(refs_path, sizeof(refs_path), "%s/refs/heads", gyatt_dir);
    free(gyatt_dir);
    
    DIR *dir = opendir(refs_path);
    if (!dir) {
        fprintf(stderr, "Error: Could not open refs/heads\n");
        return 1;
    }
    
    char *current_branch = get_current_branch();
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Check if it's a regular file
        char branch_path[PATH_MAX];
        snprintf(branch_path, sizeof(branch_path), "%s/%s", refs_path, entry->d_name);
        
        struct stat st;
        if (stat(branch_path, &st) != 0 || !S_ISREG(st.st_mode)) {
            continue;
        }
        
        // Print branch name with * for current branch
        if (current_branch && strcmp(entry->d_name, current_branch) == 0) {
            printf("\033[32m* %s\033[0m\n", entry->d_name);
        } else {
            printf("  %s\n", entry->d_name);
        }
    }
    
    closedir(dir);
    free(current_branch);
    
    return 0;
}

// Create a new branch
static int create_branch(const char *branch_name) {
    if (!branch_name || strlen(branch_name) == 0) {
        fprintf(stderr, "Error: Branch name required\n");
        return 1;
    }
    
    // Validate branch name
    if (strchr(branch_name, '/') || strchr(branch_name, ' ') || 
        strchr(branch_name, '\\') || strchr(branch_name, '\t')) {
        fprintf(stderr, "Error: Invalid branch name '%s'\n", branch_name);
        return 1;
    }
    
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        fprintf(stderr, "Error: Not a gyatt repository\n");
        return 1;
    }
    
    // Check if branch already exists
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/refs/heads/%s", gyatt_dir, branch_name);
    
    if (file_exists(branch_path)) {
        fprintf(stderr, "Error: Branch '%s' already exists\n", branch_name);
        free(gyatt_dir);
        return 1;
    }
    
    // Get current HEAD commit
    gyatt_hash_t head_hash;
    if (get_head_commit_hash(&head_hash) != 0) {
        fprintf(stderr, "Error: Could not get HEAD commit\n");
        free(gyatt_dir);
        return 1;
    }
    
    // Check if we have any commits
    int has_commits = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (head_hash.hash[i] != 0) {
            has_commits = 1;
            break;
        }
    }
    
    if (!has_commits) {
        fprintf(stderr, "Error: Cannot create branch without any commits\n");
        fprintf(stderr, "Create your first commit before creating branches\n");
        free(gyatt_dir);
        return 1;
    }
    
    // Write the commit hash to the new branch
    char hash_hex[HASH_HEX_SIZE + 2];
    hash_to_hex(&head_hash, hash_hex);
    strcat(hash_hex, "\n");
    
    if (write_file(branch_path, hash_hex, strlen(hash_hex)) != 0) {
        fprintf(stderr, "Error: Failed to create branch '%s'\n", branch_name);
        free(gyatt_dir);
        return 1;
    }
    
    printf("Branch '%s' created\n", branch_name);
    free(gyatt_dir);
    
    return 0;
}

// Delete a branch
static int delete_branch(const char *branch_name) {
    if (!branch_name || strlen(branch_name) == 0) {
        fprintf(stderr, "Error: Branch name required\n");
        return 1;
    }
    
    char *current_branch = get_current_branch();
    if (current_branch && strcmp(branch_name, current_branch) == 0) {
        fprintf(stderr, "Error: Cannot delete current branch '%s'\n", branch_name);
        free(current_branch);
        return 1;
    }
    free(current_branch);
    
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        fprintf(stderr, "Error: Not a gyatt repository\n");
        return 1;
    }
    
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/refs/heads/%s", gyatt_dir, branch_name);
    free(gyatt_dir);
    
    if (!file_exists(branch_path)) {
        fprintf(stderr, "Error: Branch '%s' does not exist\n", branch_name);
        return 1;
    }
    
    if (remove(branch_path) != 0) {
        fprintf(stderr, "Error: Failed to delete branch '%s'\n", branch_name);
        return 1;
    }
    
    printf("Deleted branch '%s'\n", branch_name);
    
    return 0;
}

int cmd_branch(int argc, char *argv[]) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Error: Not a Gyatt repository\n");
        return 1;
    }
    
    // No arguments - list branches
    if (argc == 1) {
        return list_branches();
    }
    
    // Parse options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
            // Delete branch
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Branch name required after %s\n", argv[i]);
                return 1;
            }
            return delete_branch(argv[i + 1]);
        } else {
            // Create branch
            return create_branch(argv[i]);
        }
    }
    
    return 0;
}
