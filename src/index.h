#ifndef INDEX_H
#define INDEX_H

#include "gyatt.h"
#include <time.h>

// Index entry representing a staged file
typedef struct {
    char path[1024];           // File path relative to repo root
    gyatt_hash_t hash;         // SHA-1 hash of file content
    uint32_t mode;             // File mode/permissions
    size_t size;               // File size in bytes
    time_t mtime;              // Modification time
    uint32_t flags;            // Status flags
} index_entry_t;

// Index structure (staging area)
typedef struct {
    index_entry_t *entries;
    size_t entry_count;
    size_t capacity;
} index_t;

// Index operations
index_t *index_create(void);
void index_free(index_t *index);

int index_read(index_t *index);
int index_write(index_t *index);

void index_add_entry(index_t *index, const char *path, const gyatt_hash_t *hash,
                     uint32_t mode, size_t size, time_t mtime);
index_entry_t *index_find_entry(index_t *index, const char *path);
int index_remove_entry(index_t *index, const char *path);

// Add file to index
int index_add_file(index_t *index, const char *path);

#endif // INDEX_H
