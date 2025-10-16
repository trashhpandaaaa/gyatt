#ifndef OBJECT_H
#define OBJECT_H

#include "gyatt.h"
#include <time.h>

// Object header structure
typedef struct {
    object_type_t type;
    size_t size;
    gyatt_hash_t hash;
} object_header_t;

// Blob object (file content)
typedef struct {
    object_header_t header;
    void *data;
} blob_object_t;

// Tree entry
typedef struct {
    char name[256];
    uint32_t mode;  // File permissions
    gyatt_hash_t hash;
    object_type_t type;
} tree_entry_t;

// Tree object (directory)
typedef struct {
    object_header_t header;
    size_t entry_count;
    tree_entry_t *entries;
} tree_object_t;

// Commit author/committer info
typedef struct {
    char name[256];
    char email[256];
    time_t timestamp;
    int timezone;
} author_info_t;

// Commit object
typedef struct {
    object_header_t header;
    gyatt_hash_t tree;
    gyatt_hash_t parent;  // Zero hash if no parent
    author_info_t author;
    author_info_t committer;
    char message[4096];
} commit_object_t;

// Object functions
blob_object_t *blob_create(const void *data, size_t size);
tree_object_t *tree_create(void);
commit_object_t *commit_create(void);

void blob_free(blob_object_t *blob);
void tree_free(tree_object_t *tree);
void commit_free(commit_object_t *commit);

// Tree manipulation
void tree_add_entry(tree_object_t *tree, const char *name, uint32_t mode, 
                    const gyatt_hash_t *hash, object_type_t type);
tree_entry_t *tree_find_entry(tree_object_t *tree, const char *name);

// Object storage functions
int object_write(const void *data, size_t size, object_type_t type, gyatt_hash_t *hash);
void *object_read(const gyatt_hash_t *hash, object_type_t *type, size_t *size);
int object_exists(const gyatt_hash_t *hash);
char *object_path(const gyatt_hash_t *hash);

// Blob storage
int blob_write(blob_object_t *blob);
blob_object_t *blob_read(const gyatt_hash_t *hash);
blob_object_t *blob_from_file(const char *path);

// Tree storage
int tree_write(tree_object_t *tree);
tree_object_t *tree_read(const gyatt_hash_t *hash);

// Commit storage
int commit_write(commit_object_t *commit);
commit_object_t *commit_read(const gyatt_hash_t *hash);

#endif // OBJECT_H
