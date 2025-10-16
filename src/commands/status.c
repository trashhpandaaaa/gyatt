#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../gyatt.h"
#include "../index.h"
#include "../object.h"
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

// Helper to check if file should be ignored
static int should_ignore(const char *path) {
    // Ignore .gyatt directory
    if (strstr(path, ".gyatt") != NULL) {
        return 1;
    }
    
    // Ignore .git directory
    if (strstr(path, ".git") != NULL) {
        return 1;
    }
    
    // Ignore common build artifacts
    if (strstr(path, "/bin/") != NULL || strstr(path, "/build/") != NULL) {
        return 1;
    }
    
    // TODO: Read from .gyattignore
    return 0;
}

// Helper to scan directory recursively
typedef struct {
    char **files;
    size_t count;
    size_t capacity;
} file_list_t;

static void file_list_init(file_list_t *list) {
    list->files = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void file_list_add(file_list_t *list, const char *path) {
    if (list->count >= list->capacity) {
        list->capacity = (list->capacity == 0) ? 16 : list->capacity * 2;
        list->files = realloc(list->files, list->capacity * sizeof(char *));
    }
    list->files[list->count++] = str_duplicate(path);
}

static void file_list_free(file_list_t *list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->files[i]);
    }
    free(list->files);
}

static void scan_directory_recursive(const char *dir_path, file_list_t *list) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (should_ignore(full_path)) {
            continue;
        }
        
        struct stat st;
        if (stat(full_path, &st) != 0) {
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            scan_directory_recursive(full_path, list);
        } else if (S_ISREG(st.st_mode)) {
            // Store relative path
            const char *rel_path = full_path;
            if (strncmp(rel_path, "./", 2) == 0) {
                rel_path += 2;
            }
            file_list_add(list, rel_path);
        }
    }
    
    closedir(dir);
}

// Helper to compute file hash (as blob)
static void compute_file_hash(const char *path, gyatt_hash_t *hash) {
    // Read file
    size_t size;
    void *data = read_file(path, &size);
    if (!data) {
        memset(hash, 0, sizeof(gyatt_hash_t));
        return;
    }
    
    // Create object header: "blob size\0"
    char header[64];
    int header_len = snprintf(header, sizeof(header), "blob %zu", size);
    header[header_len++] = '\0';
    
    // Combine header and data for hashing
    size_t total_size = header_len + size;
    void *combined = malloc(total_size);
    if (!combined) {
        free(data);
        memset(hash, 0, sizeof(gyatt_hash_t));
        return;
    }
    
    memcpy(combined, header, header_len);
    memcpy((char *)combined + header_len, data, size);
    
    // Calculate hash
    sha1_hash(combined, total_size, hash);
    
    free(combined);
    free(data);
}

// Helper to check if file is in index
static index_entry_t *find_in_index(index_t *index, const char *path) {
    for (size_t i = 0; i < index->entry_count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            return &index->entries[i];
        }
    }
    return NULL;
}

// Helper to get last commit tree
static tree_object_t *get_head_tree(void) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return NULL;
    
    // Read HEAD
    char head_path[PATH_MAX];
    snprintf(head_path, sizeof(head_path), "%s/HEAD", gyatt_dir);
    
    size_t size = 0;
    char *head_content = read_file(head_path, &size);
    if (!head_content) {
        free(gyatt_dir);
        return NULL;
    }
    
    // Parse branch ref
    char *ref_start = strstr(head_content, "refs/heads/");
    if (!ref_start) {
        free(head_content);
        free(gyatt_dir);
        return NULL;
    }
    ref_start += strlen("refs/heads/");
    char *newline = strchr(ref_start, '\n');
    if (newline) *newline = '\0';
    
    // Read branch ref file
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/refs/heads/%s", gyatt_dir, ref_start);
    free(head_content);
    free(gyatt_dir);
    
    size_t hash_size = 0;
    char *hash_str = read_file(branch_path, &hash_size);
    if (!hash_str) {
        // No commits yet
        return NULL;
    }
    
    // Parse commit hash
    gyatt_hash_t commit_hash;
    hex_to_hash(hash_str, &commit_hash);
    free(hash_str);
    
    // Read commit object
    commit_object_t *commit = commit_read(&commit_hash);
    if (!commit) return NULL;
    
    // Read tree object
    tree_object_t *tree = tree_read(&commit->tree);
    commit_free(commit);
    
    return tree;
}

int cmd_status(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Error: Not a Gyatt repository\n");
        return 1;
    }
    
    // Get current branch
    char *branch = get_current_branch();
    if (!branch) {
        fprintf(stderr, "Error: Could not determine current branch\n");
        return 1;
    }
    
    printf("On branch %s\n", branch);
    free(branch);
    
    // Load index
    index_t *index = index_create();
    if (!index) {
        fprintf(stderr, "Error: Failed to create index\n");
        return 1;
    }
    
    // Try to read index (it's okay if it doesn't exist yet)
    index_read(index);
    
    // Get HEAD tree (last commit)
    tree_object_t *head_tree = get_head_tree();
    
    // Scan working directory
    file_list_t working_files;
    file_list_init(&working_files);
    scan_directory_recursive(".", &working_files);
    
    // Categorize files
    file_list_t staged_new, staged_modified, staged_deleted;
    file_list_t modified_not_staged, deleted_not_staged;
    file_list_t untracked;
    
    file_list_init(&staged_new);
    file_list_init(&staged_modified);
    file_list_init(&staged_deleted);
    file_list_init(&modified_not_staged);
    file_list_init(&deleted_not_staged);
    file_list_init(&untracked);
    
    // Check staged files (in index)
    for (size_t i = 0; i < index->entry_count; i++) {
        index_entry_t *entry = &index->entries[i];
        
        // Check if file exists in HEAD tree
        int in_head = 0;
        if (head_tree) {
            tree_entry_t *head_entry = tree_find_entry(head_tree, entry->path);
            if (head_entry) {
                in_head = 1;
                // Check if modified from HEAD
                if (hash_compare(&entry->hash, &head_entry->hash) != 0) {
                    file_list_add(&staged_modified, entry->path);
                }
            }
        }
        
        if (!in_head) {
            // New file staged
            file_list_add(&staged_new, entry->path);
        }
        
        // Check if file still exists in working directory
        if (!file_exists(entry->path)) {
            file_list_add(&deleted_not_staged, entry->path);
        } else {
            // Check if modified in working directory
            gyatt_hash_t current_hash;
            compute_file_hash(entry->path, &current_hash);
            if (hash_compare(&current_hash, &entry->hash) != 0) {
                file_list_add(&modified_not_staged, entry->path);
            }
        }
    }
    
    // Check files in HEAD but not in working directory or index
    if (head_tree) {
        for (size_t i = 0; i < head_tree->entry_count; i++) {
            tree_entry_t *head_entry = &head_tree->entries[i];
            index_entry_t *idx_entry = find_in_index(index, head_entry->name);
            
            // If file is in HEAD but not in index
            if (!idx_entry) {
                // Check if file still exists in working directory
                if (file_exists(head_entry->name)) {
                    // File exists but not staged - this is normal, not a "staged deletion"
                    // Don't add to staged_deleted
                } else {
                    // File was in HEAD, not in index, and doesn't exist - it's deleted but not staged
                    file_list_add(&deleted_not_staged, head_entry->name);
                }
            }
        }
    }
    
    // Check untracked files and modified files (not in index)
    for (size_t i = 0; i < working_files.count; i++) {
        const char *path = working_files.files[i];
        if (!find_in_index(index, path)) {
            // Not in index - check if in HEAD
            int in_head = 0;
            tree_entry_t *head_entry = NULL;
            if (head_tree) {
                head_entry = tree_find_entry(head_tree, path);
                if (head_entry) in_head = 1;
            }
            
            if (!in_head) {
                // Not in HEAD and not in index - truly untracked
                file_list_add(&untracked, path);
            } else {
                // In HEAD but not in index - check if modified
                gyatt_hash_t current_hash;
                compute_file_hash(path, &current_hash);
                if (hash_compare(&current_hash, &head_entry->hash) != 0) {
                    // Modified from HEAD but not staged
                    file_list_add(&modified_not_staged, path);
                }
                // If not modified, it's clean - don't report anything
            }
        }
    }
    
    // Print status
    int has_changes = 0;
    
    if (staged_new.count > 0 || staged_modified.count > 0 || staged_deleted.count > 0) {
        has_changes = 1;
        printf("\nChanges to be committed:\n");
        printf("  (use \"gyatt restore --staged <file>...\" to unstage)\n\n");
        
        for (size_t i = 0; i < staged_new.count; i++) {
            printf("\t\033[32mnew file:   %s\033[0m\n", staged_new.files[i]);
        }
        for (size_t i = 0; i < staged_modified.count; i++) {
            printf("\t\033[32mmodified:   %s\033[0m\n", staged_modified.files[i]);
        }
        for (size_t i = 0; i < staged_deleted.count; i++) {
            printf("\t\033[32mdeleted:    %s\033[0m\n", staged_deleted.files[i]);
        }
    }
    
    if (modified_not_staged.count > 0 || deleted_not_staged.count > 0) {
        has_changes = 1;
        printf("\nChanges not staged for commit:\n");
        printf("  (use \"gyatt add <file>...\" to update what will be committed)\n");
        printf("  (use \"gyatt restore <file>...\" to discard changes in working directory)\n\n");
        
        for (size_t i = 0; i < modified_not_staged.count; i++) {
            printf("\t\033[31mmodified:   %s\033[0m\n", modified_not_staged.files[i]);
        }
        for (size_t i = 0; i < deleted_not_staged.count; i++) {
            printf("\t\033[31mdeleted:    %s\033[0m\n", deleted_not_staged.files[i]);
        }
    }
    
    if (untracked.count > 0) {
        has_changes = 1;
        printf("\nUntracked files:\n");
        printf("  (use \"gyatt add <file>...\" to include in what will be committed)\n\n");
        
        for (size_t i = 0; i < untracked.count; i++) {
            printf("\t\033[31m%s\033[0m\n", untracked.files[i]);
        }
    }
    
    if (!has_changes) {
        if (head_tree) {
            printf("\nnothing to commit, working tree clean\n");
        } else {
            printf("\nNo commits yet\n");
            printf("\nnothing to commit (create/copy files and use \"gyatt add\" to track)\n");
        }
    } else {
        if (staged_new.count == 0 && staged_modified.count == 0 && staged_deleted.count == 0) {
            printf("\nno changes added to commit (use \"gyatt add\" and/or \"gyatt commit -a\")\n");
        }
    }
    
    // Cleanup
    if (head_tree) tree_free(head_tree);
    index_free(index);
    file_list_free(&working_files);
    file_list_free(&staged_new);
    file_list_free(&staged_modified);
    file_list_free(&staged_deleted);
    file_list_free(&modified_not_staged);
    file_list_free(&deleted_not_staged);
    file_list_free(&untracked);
    
    return 0;
}
