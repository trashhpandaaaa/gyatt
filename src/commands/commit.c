#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../gyatt.h"
#include "../index.h"
#include "../object.h"
#include "../utils.h"
#include "../hash.h"

#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

// Helper to build tree from index
static tree_object_t *build_tree_from_index(index_t *index) {
    tree_object_t *tree = tree_create();
    if (!tree) return NULL;
    
    // Add each index entry as a tree entry
    for (size_t i = 0; i < index->entry_count; i++) {
        index_entry_t *idx_entry = &index->entries[i];
        
        // Determine if it's a file or directory
        // For now, we treat everything as blobs (files)
        tree_add_entry(tree, idx_entry->path, idx_entry->mode, 
                      &idx_entry->hash, OBJ_BLOB);
    }
    
    return tree;
}

// Helper to read current HEAD commit hash
static int get_head_commit(gyatt_hash_t *head_hash) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        return -1;
    }
    
    char head_path[PATH_MAX];
    snprintf(head_path, sizeof(head_path), "%s/HEAD", gyatt_dir);
    
    // Read HEAD to get current branch ref
    size_t ref_size = 0;
    char *ref_path = read_file(head_path, &ref_size);
    if (!ref_path) {
        return -1;
    }
    
    // HEAD should contain "ref: refs/heads/branch_name"
    char *ref_start = strstr(ref_path, "ref:");
    if (!ref_start) {
        free(ref_path);
        return -1;
    }
    ref_start += 4;
    while (*ref_start == ' ' || *ref_start == '\t') ref_start++;
    
    // Remove newline
    char *newline = strchr(ref_start, '\n');
    if (newline) *newline = '\0';
    
    // Read the branch ref file
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/%s", gyatt_dir, ref_start);
    
    size_t hash_size = 0;
    char *hash_str = read_file(branch_path, &hash_size);
    if (!hash_str) {
        // No commits yet
        memset(head_hash, 0, sizeof(gyatt_hash_t));
        free(ref_path);
        return 0;
    }
    
    // Parse hash
    hex_to_hash(hash_str, head_hash);
    
    free(hash_str);
    free(ref_path);
    free(gyatt_dir);
    return 0;
}

// Helper to update HEAD to point to new commit
static int update_head(const gyatt_hash_t *commit_hash) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        return -1;
    }
    
    char head_path[PATH_MAX];
    snprintf(head_path, sizeof(head_path), "%s/HEAD", gyatt_dir);
    
    // Read HEAD to get current branch ref
    size_t ref_size = 0;
    char *ref_content = read_file(head_path, &ref_size);
    if (!ref_content) {
        return -1;
    }
    
    // HEAD should contain "ref: refs/heads/branch_name"
    char *ref_start = strstr(ref_content, "ref:");
    if (!ref_start) {
        free(ref_content);
        return -1;
    }
    ref_start += 4;
    while (*ref_start == ' ' || *ref_start == '\t') ref_start++;
    
    // Remove newline
    char *newline = strchr(ref_start, '\n');
    if (newline) *newline = '\0';
    
    // Write commit hash to branch ref
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/%s", gyatt_dir, ref_start);
    
    char hash_hex[HASH_HEX_SIZE + 2];
    hash_to_hex(commit_hash, hash_hex);
    strcat(hash_hex, "\n");
    
    if (write_file(branch_path, hash_hex, strlen(hash_hex)) != 0) {
        free(ref_content);
        free(gyatt_dir);
        return -1;
    }
    
    free(ref_content);
    free(gyatt_dir);
    return 0;
}

int cmd_commit(int argc, char *argv[]) {
    // Parse arguments - looking for -m "message"
    char *message = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            message = argv[i + 1];
            i++;
        }
    }
    
    if (!message) {
        fprintf(stderr, "Error: Commit message required. Use: gyatt commit -m \"message\"\n");
        return 1;
    }
    
    // Find gyatt directory
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        fprintf(stderr, "Error: Not a gyatt repository\n");
        return 1;
    }
    
    // Read index
    index_t *index = index_create();
    if (!index) {
        fprintf(stderr, "Error: Failed to create index\n");
        free(gyatt_dir);
        return 1;
    }
    
    if (index_read(index) != 0) {
        fprintf(stderr, "Error: Could not read index\n");
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    if (index->entry_count == 0) {
        fprintf(stderr, "Error: Nothing to commit (staging area is empty)\n");
        fprintf(stderr, "Use 'gyatt add <file>' to stage files for commit\n");
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    // Build tree from index
    tree_object_t *tree = build_tree_from_index(index);
    if (!tree) {
        fprintf(stderr, "Error: Failed to build tree\n");
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    // Write tree object
    if (tree_write(tree) != 0) {
        fprintf(stderr, "Error: Failed to write tree object\n");
        tree_free(tree);
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    gyatt_hash_t tree_hash = tree->header.hash;
    tree_free(tree);
    
    // Get parent commit
    gyatt_hash_t parent_hash;
    get_head_commit(&parent_hash);
    
    // Create commit object
    commit_object_t *commit = commit_create();
    if (!commit) {
        fprintf(stderr, "Error: Failed to create commit\n");
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    // Set commit data
    commit->tree = tree_hash;
    commit->parent = parent_hash;
    
    // Set info (for now, I'll use a placeholder)
    // TODO: Read from config
    strncpy(commit->author.name, "Gyatt User", sizeof(commit->author.name) - 1);
    strncpy(commit->author.email, "user@gyatt.local", sizeof(commit->author.email) - 1);
    commit->author.timestamp = time(NULL);
    commit->author.timezone = 0;
    
    commit->committer = commit->author;
    
    strncpy(commit->message, message, sizeof(commit->message) - 1);
    
    // Write commit object
    if (commit_write(commit) != 0) {
        fprintf(stderr, "Error: Failed to write commit object\n");
        commit_free(commit);
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    gyatt_hash_t commit_hash = commit->header.hash;
    commit_free(commit);
    
    // Update HEAD
    if (update_head(&commit_hash) != 0) {
        fprintf(stderr, "Error: Failed to update HEAD\n");
        index_free(index);
        free(gyatt_dir);
        return 1;
    }
    
    // Save file count before clearing
    size_t file_count = index->entry_count;
    
    // Clear the index after successful commit
    index->entry_count = 0;
    index_write(index);
    
    // Print success
    char hash_hex[HASH_HEX_SIZE + 1];
    hash_to_hex(&commit_hash, hash_hex);
    
    printf("[");
    
    // Get current branch name from HEAD
    char head_path[PATH_MAX];
    snprintf(head_path, sizeof(head_path), "%s/HEAD", gyatt_dir);
    size_t head_size = 0;
    char *head_content = read_file(head_path, &head_size);
    if (head_content) {
        char *branch_start = strstr(head_content, "refs/heads/");
        if (branch_start) {
            branch_start += strlen("refs/heads/");
            char *newline = strchr(branch_start, '\n');
            if (newline) *newline = '\0';
            printf("%s", branch_start);
        }
        free(head_content);
    }
    
    printf(" %.7s] %s\n", hash_hex, message);
    printf(" %zu file(s) changed\n", file_count);
    
    index_free(index);
    free(gyatt_dir);
    return 0;
}
