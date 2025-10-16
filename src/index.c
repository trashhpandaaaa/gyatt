#include "index.h"
#include "hash.h"
#include "object.h"
#include "utils.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef PATH_MAX
    #define PATH_MAX 4096
#endif

#define INDEX_SIGNATURE "GYAT"
#define INDEX_VERSION 1

index_t *index_create(void) {
    index_t *index = calloc(1, sizeof(index_t));
    if (!index) return NULL;
    
    index->entries = NULL;
    index->entry_count = 0;
    index->capacity = 0;
    
    return index;
}

void index_free(index_t *index) {
    if (!index) return;
    free(index->entries);
    free(index);
}

// Sort comparison function for index entries (by path)
static int entry_compare(const void *a, const void *b) {
    const index_entry_t *ea = (const index_entry_t *)a;
    const index_entry_t *eb = (const index_entry_t *)b;
    return strcmp(ea->path, eb->path);
}

int index_read(index_t *index) {
    if (!index) return -1;
    
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return -1;
    
    char *index_path = path_join(gyatt_dir, "index");
    free(gyatt_dir);
    
    // If index doesn't exist, that's okay - start with empty index
    if (!file_exists(index_path)) {
        free(index_path);
        return 0;
    }
    
    size_t file_size;
    void *data = read_file(index_path, &file_size);
    free(index_path);
    
    if (!data) return -1;
    
    char *ptr = (char *)data;
    
    // Check signature
    if (file_size < 8 || memcmp(ptr, INDEX_SIGNATURE, 4) != 0) {
        free(data);
        return -1;
    }
    ptr += 4;
    
    // Check version
    uint32_t version = *(uint32_t *)ptr;
    ptr += 4;
    
    if (version != INDEX_VERSION) {
        free(data);
        return -1;
    }
    
    // Read entry count
    if ((size_t)(ptr - (char *)data) + 4 > file_size) {
        free(data);
        return -1;
    }
    
    uint32_t entry_count = *(uint32_t *)ptr;
    ptr += 4;
    
    // Allocate entries
    index->entries = malloc(entry_count * sizeof(index_entry_t));
    if (!index->entries && entry_count > 0) {
        free(data);
        return -1;
    }
    
    index->entry_count = entry_count;
    index->capacity = entry_count;
    
    // Read entries
    for (size_t i = 0; i < entry_count; i++) {
        index_entry_t *entry = &index->entries[i];
        
        // Check bounds for path length
        if ((size_t)(ptr - (char *)data) + 2 > file_size) {
            free(data);
            return -1;
        }
        
        // Read path length
        uint16_t path_len = *(uint16_t *)ptr;
        ptr += 2;
        
        if (path_len >= sizeof(entry->path)) {
            free(data);
            return -1;
        }
        
        // Check bounds for path + hash + metadata (20 + 4 + 8 + 8 + 4 = 44 bytes)
        if ((size_t)(ptr - (char *)data) + path_len + 44 > file_size) {
            free(data);
            return -1;
        }
        
        // Read path
        memcpy(entry->path, ptr, path_len);
        entry->path[path_len] = '\0';
        ptr += path_len;
        
        // Read hash
        memcpy(entry->hash.hash, ptr, HASH_SIZE);
        ptr += HASH_SIZE;
        
        // Read mode, size, mtime, flags
        entry->mode = *(uint32_t *)ptr;
        ptr += 4;
        
        entry->size = *(uint64_t *)ptr;
        ptr += 8;
        
        entry->mtime = *(uint64_t *)ptr;
        ptr += 8;
        
        entry->flags = *(uint32_t *)ptr;
        ptr += 4;
    }
    
    free(data);
    return 0;
}

int index_write(index_t *index) {
    if (!index) return -1;
    
    // Sort entries by path for consistent ordering
    if (index->entry_count > 0) {
        qsort(index->entries, index->entry_count, sizeof(index_entry_t), entry_compare);
    }
    
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return -1;
    
    char *index_path = path_join(gyatt_dir, "index");
    free(gyatt_dir);
    
    buffer_t *buf = buffer_create(4096);
    
    // Write signature and version
    buffer_append(buf, INDEX_SIGNATURE, 4);
    uint32_t version = INDEX_VERSION;
    buffer_append(buf, &version, 4);
    
    // Write entry count
    uint32_t count = (uint32_t)index->entry_count;
    buffer_append(buf, &count, 4);
    
    // Write entries
    for (size_t i = 0; i < index->entry_count; i++) {
        index_entry_t *entry = &index->entries[i];
        
        // Write path length and path
        uint16_t path_len = (uint16_t)strlen(entry->path);
        buffer_append(buf, &path_len, 2);
        buffer_append(buf, entry->path, path_len);
        
        // Write hash
        buffer_append(buf, entry->hash.hash, HASH_SIZE);
        
        // Write mode, size, mtime, flags
        buffer_append(buf, &entry->mode, 4);
        
        uint64_t size64 = entry->size;
        buffer_append(buf, &size64, 8);
        
        uint64_t mtime64 = entry->mtime;
        buffer_append(buf, &mtime64, 8);
        
        buffer_append(buf, &entry->flags, 4);
    }
    
    int result = write_file(index_path, buf->data, buf->len);
    
    free(index_path);
    buffer_free(buf);
    
    return result;
}

void index_add_entry(index_t *index, const char *path, const gyatt_hash_t *hash,
                     uint32_t mode, size_t size, time_t mtime) {
    if (!index || !path || !hash) return;
    
    // Check if entry already exists
    index_entry_t *existing = index_find_entry(index, path);
    if (existing) {
        // Update existing entry
        hash_copy(&existing->hash, hash);
        existing->mode = mode;
        existing->size = size;
        existing->mtime = mtime;
        return;
    }
    
    // Add new entry
    if (index->entry_count >= index->capacity) {
        size_t new_capacity = index->capacity == 0 ? 16 : index->capacity * 2;
        index_entry_t *new_entries = realloc(index->entries, 
                                              new_capacity * sizeof(index_entry_t));
        if (!new_entries) return;
        
        index->entries = new_entries;
        index->capacity = new_capacity;
    }
    
    index_entry_t *entry = &index->entries[index->entry_count++];
    strncpy(entry->path, path, sizeof(entry->path) - 1);
    entry->path[sizeof(entry->path) - 1] = '\0';
    hash_copy(&entry->hash, hash);
    entry->mode = mode;
    entry->size = size;
    entry->mtime = mtime;
    entry->flags = 0;
}

index_entry_t *index_find_entry(index_t *index, const char *path) {
    if (!index || !path) return NULL;
    
    for (size_t i = 0; i < index->entry_count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            return &index->entries[i];
        }
    }
    
    return NULL;
}

int index_remove_entry(index_t *index, const char *path) {
    if (!index || !path) return -1;
    
    for (size_t i = 0; i < index->entry_count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            // Shift remaining entries
            memmove(&index->entries[i], &index->entries[i + 1],
                    (index->entry_count - i - 1) * sizeof(index_entry_t));
            index->entry_count--;
            return 0;
        }
    }
    
    return -1;
}

// Get absolute path (simple implementation)
static char *get_absolute_path(const char *path) {
    char *result = malloc(PATH_MAX);
    if (!result) return NULL;
    
    // If path is already absolute
    if (path[0] == '/') {
        strncpy(result, path, PATH_MAX - 1);
        result[PATH_MAX - 1] = '\0';
        return result;
    }
    
    // Get current directory and join
    char *cwd = get_current_dir();
    if (!cwd) {
        free(result);
        return NULL;
    }
    
    snprintf(result, PATH_MAX, "%s/%s", cwd, path);
    free(cwd);
    
    return result;
}

int index_add_file(index_t *index, const char *path) {
    if (!index || !path) return -1;
    
    // Get file stats
    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Error: Cannot stat file '%s'\n", path);
        return -1;
    }
    
    // Check if it's a regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file\n", path);
        return -1;
    }
    
    // Create blob from file and write to object store
    blob_object_t *blob = blob_from_file(path);
    if (!blob) {
        fprintf(stderr, "Error: Failed to read file '%s'\n", path);
        return -1;
    }
    
    if (blob_write(blob) != 0) {
        fprintf(stderr, "Error: Failed to write blob for '%s'\n", path);
        blob_free(blob);
        return -1;
    }
    
    // Get relative path from repo root
    char *repo_root = find_repo_root();
    if (!repo_root) {
        blob_free(blob);
        return -1;
    }
    
    char *abs_path = get_absolute_path(path);
    if (!abs_path) {
        free(repo_root);
        blob_free(blob);
        return -1;
    }
    
    // Calculate relative path
    size_t root_len = strlen(repo_root);
    const char *rel_path = abs_path;
    
    if (strncmp(abs_path, repo_root, root_len) == 0) {
        rel_path = abs_path + root_len;
        if (*rel_path == '/' || *rel_path == '\\') rel_path++;
    }
    
    // Add to index
    index_add_entry(index, rel_path, &blob->header.hash, 
                    st.st_mode & 0777, blob->header.size, st.st_mtime);
    
    free(abs_path);
    free(repo_root);
    blob_free(blob);
    
    return 0;
}
