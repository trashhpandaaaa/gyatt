#include "object.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>

blob_object_t *blob_create(const void *data, size_t size) {
    blob_object_t *blob = calloc(1, sizeof(blob_object_t));
    if (!blob) return NULL;

    blob->header.type = OBJ_BLOB;
    blob->header.size = size;
    
    blob->data = malloc(size);
    if (!blob->data) {
        free(blob);
        return NULL;
    }
    
    memcpy(blob->data, data, size);
    
    // Calculate hash
    sha1_hash(data, size, &blob->header.hash);
    
    return blob;
}

tree_object_t *tree_create(void) {
    tree_object_t *tree = calloc(1, sizeof(tree_object_t));
    if (!tree) return NULL;

    tree->header.type = OBJ_TREE;
    tree->header.size = 0;
    tree->entry_count = 0;
    tree->entries = NULL;
    
    return tree;
}

commit_object_t *commit_create(void) {
    commit_object_t *commit = calloc(1, sizeof(commit_object_t));
    if (!commit) return NULL;

    commit->header.type = OBJ_COMMIT;
    
    // Initialize with zero hashes
    memset(&commit->tree, 0, sizeof(gyatt_hash_t));
    memset(&commit->parent, 0, sizeof(gyatt_hash_t));
    
    return commit;
}

void blob_free(blob_object_t *blob) {
    if (!blob) return;
    free(blob->data);
    free(blob);
}

void tree_free(tree_object_t *tree) {
    if (!tree) return;
    free(tree->entries);
    free(tree);
}

void commit_free(commit_object_t *commit) {
    if (!commit) return;
    free(commit);
}

void tree_add_entry(tree_object_t *tree, const char *name, uint32_t mode,
                    const gyatt_hash_t *hash, object_type_t type) {
    if (!tree || !name || !hash) return;

    // Reallocate entries array
    tree_entry_t *new_entries = realloc(tree->entries, 
                                        (tree->entry_count + 1) * sizeof(tree_entry_t));
    if (!new_entries) return;
    
    tree->entries = new_entries;
    tree_entry_t *entry = &tree->entries[tree->entry_count];
    
    // Set entry data
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';
    entry->mode = mode;
    hash_copy(&entry->hash, hash);
    entry->type = type;
    
    tree->entry_count++;
}

tree_entry_t *tree_find_entry(tree_object_t *tree, const char *name) {
    if (!tree || !name) return NULL;
    
    for (size_t i = 0; i < tree->entry_count; i++) {
        if (strcmp(tree->entries[i].name, name) == 0) {
            return &tree->entries[i];
        }
    }
    
    return NULL;
}
