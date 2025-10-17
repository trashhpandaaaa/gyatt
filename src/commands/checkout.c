#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gyatt.h"
#include "../utils.h"
#include "../object.h"
#include "../index.h"
#include "../hash.h"

#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

// Helper to check if working directory is clean
static int is_working_directory_clean(void) {
    // For now, just check if index is empty
    // In a full implementation, we'd check for uncommitted changes
    index_t *index = index_create();
    if (!index) return 0;
    
    index_read(index);
    int is_clean = (index->entry_count == 0);
    index_free(index);
    
    return is_clean;
}

// Helper to update HEAD to point to a branch
static int update_head_to_branch(const char *branch_name) {
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return -1;
    
    char head_path[PATH_MAX];
    snprintf(head_path, sizeof(head_path), "%s/HEAD", gyatt_dir);
    free(gyatt_dir);
    
    char ref_content[PATH_MAX];
    snprintf(ref_content, sizeof(ref_content), "ref: refs/heads/%s\n", branch_name);
    
    if (write_file(head_path, ref_content, strlen(ref_content)) != 0) {
        return -1;
    }
    
    return 0;
}

// Helper to restore files from a commit
static int restore_files_from_commit(const gyatt_hash_t *commit_hash) {
    // Read commit
    commit_object_t *commit = commit_read(commit_hash);
    if (!commit) {
        fprintf(stderr, "Error: Could not read commit\n");
        return -1;
    }
    
    // Read tree
    tree_object_t *tree = tree_read(&commit->tree);
    commit_free(commit);
    
    if (!tree) {
        fprintf(stderr, "Error: Could not read tree\n");
        return -1;
    }
    
    // For each entry in tree, restore the file
    for (size_t i = 0; i < tree->entry_count; i++) {
        tree_entry_t *entry = &tree->entries[i];
        
        if (entry->type != OBJ_BLOB) {
            continue;  // Skip non-blobs for now
        }
        
        // Read blob
        blob_object_t *blob = blob_read(&entry->hash);
        if (!blob) {
            fprintf(stderr, "Warning: Could not read blob for '%s'\n", entry->name);
            continue;
        }
        
        // Write file
        if (write_file(entry->name, blob->data, blob->header.size) != 0) {
            fprintf(stderr, "Warning: Could not write file '%s'\n", entry->name);
        }
        
        blob_free(blob);
    }
    
    tree_free(tree);
    
    return 0;
}

int cmd_checkout(int argc, char *argv[]) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Error: Not a Gyatt repository\n");
        return 1;
    }
    
    if (argc < 2) {
        fprintf(stderr, "Usage: gyatt checkout <branch-name>\n");
        return 1;
    }
    
    const char *branch_name = argv[1];
    
    // Check if branch exists
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) {
        fprintf(stderr, "Error: Not a gyatt repository\n");
        return 1;
    }
    
    char branch_path[PATH_MAX];
    snprintf(branch_path, sizeof(branch_path), "%s/refs/heads/%s", gyatt_dir, branch_name);
    
    if (!file_exists(branch_path)) {
        fprintf(stderr, "Error: Branch '%s' does not exist\n", branch_name);
        free(gyatt_dir);
        return 1;
    }
    
    // Check if working directory is clean
    if (!is_working_directory_clean()) {
        fprintf(stderr, "Error: You have uncommitted changes\n");
        fprintf(stderr, "Please commit or stash them before switching branches\n");
        free(gyatt_dir);
        return 1;
    }
    
    // Read branch commit hash
    size_t size = 0;
    char *hash_str = read_file(branch_path, &size);
    if (!hash_str) {
        fprintf(stderr, "Error: Could not read branch ref\n");
        free(gyatt_dir);
        return 1;
    }
    
    gyatt_hash_t commit_hash;
    hex_to_hash(hash_str, &commit_hash);
    free(hash_str);
    
    // Restore files from the commit
    if (restore_files_from_commit(&commit_hash) != 0) {
        fprintf(stderr, "Error: Failed to restore files\n");
        free(gyatt_dir);
        return 1;
    }
    
    // Update HEAD to point to the new branch
    if (update_head_to_branch(branch_name) != 0) {
        fprintf(stderr, "Error: Failed to update HEAD\n");
        free(gyatt_dir);
        return 1;
    }
    
    printf("Switched to branch '%s'\n", branch_name);
    
    free(gyatt_dir);
    
    return 0;
}
