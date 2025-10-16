#include "object.h"
#include "hash.h"
#include "utils.h"
#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include <sys/stat.h>

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

// ==================== Object Storage ====================

// Get the path for an object based on its hash
char *object_path(const gyatt_hash_t *hash) {
    char hex[HASH_HEX_SIZE];
    hash_to_hex(hash, hex);
    
    char *gyatt_dir = get_gyatt_dir();
    if (!gyatt_dir) return NULL;
    
    // Use Git's sharding: first 2 hex chars as directory, rest as filename
    char dir_name[3] = {hex[0], hex[1], '\0'};
    char *file_name = hex + 2;
    
    char *objects_dir = path_join(gyatt_dir, "objects");
    char *shard_dir = path_join(objects_dir, dir_name);
    char *obj_path = path_join(shard_dir, file_name);
    
    free(gyatt_dir);
    free(objects_dir);
    free(shard_dir);
    
    return obj_path;
}

// Check if an object exists
int object_exists(const gyatt_hash_t *hash) {
    char *path = object_path(hash);
    if (!path) return 0;
    
    int exists = file_exists(path);
    free(path);
    
    return exists;
}

// Compress data using zlib
static void *compress_data(const void *data, size_t size, size_t *compressed_size) {
    uLongf dest_len = compressBound(size);
    Bytef *dest = malloc(dest_len);
    if (!dest) return NULL;
    
    int result = compress2(dest, &dest_len, data, size, Z_DEFAULT_COMPRESSION);
    if (result != Z_OK) {
        free(dest);
        return NULL;
    }
    
    *compressed_size = dest_len;
    return dest;
}

// Decompress data using zlib
static void *decompress_data(const void *data, size_t compressed_size, size_t expected_size) {
    uLongf dest_len = expected_size;
    Bytef *dest = malloc(dest_len);
    if (!dest) return NULL;
    
    int result = uncompress(dest, &dest_len, data, compressed_size);
    if (result != Z_OK) {
        free(dest);
        return NULL;
    }
    
    return dest;
}

// Write an object to storage
int object_write(const void *data, size_t size, object_type_t type, gyatt_hash_t *hash) {
    if (!data || size == 0) return -1;
    
    // Create object header: "type size\0"
    buffer_t *header_buf = buffer_create(64);
    const char *type_str = (type == OBJ_BLOB) ? "blob" :
                          (type == OBJ_TREE) ? "tree" :
                          (type == OBJ_COMMIT) ? "commit" : "unknown";
    
    buffer_append_str(header_buf, type_str);
    buffer_append_char(header_buf, ' ');
    buffer_append_uint(header_buf, size);
    buffer_append_char(header_buf, '\0');
    
    // Combine header and data for hashing
    size_t total_size = header_buf->len + size;
    void *combined = malloc(total_size);
    if (!combined) {
        buffer_free(header_buf);
        return -1;
    }
    
    memcpy(combined, header_buf->data, header_buf->len);
    memcpy((char *)combined + header_buf->len, data, size);
    
    // Calculate hash
    sha1_hash(combined, total_size, hash);
    
    // Check if object already exists
    if (object_exists(hash)) {
        buffer_free(header_buf);
        free(combined);
        return 0;  // Already exists, success
    }
    
    // Compress the combined data
    size_t compressed_size;
    void *compressed = compress_data(combined, total_size, &compressed_size);
    free(combined);
    buffer_free(header_buf);
    
    if (!compressed) return -1;
    
    // Get object path and create directory
    char *obj_path = object_path(hash);
    if (!obj_path) {
        free(compressed);
        return -1;
    }
    
    // Create parent directory
    char *parent_dir = str_duplicate(obj_path);
    char *last_sep = strrchr(parent_dir, '/');
    if (last_sep) {
        *last_sep = '\0';
        mkdir_recursive(parent_dir);
    }
    free(parent_dir);
    
    // Write compressed data
    int result = write_file(obj_path, compressed, compressed_size);
    
    free(obj_path);
    free(compressed);
    
    return result;
}

// Read an object from storage
void *object_read(const gyatt_hash_t *hash, object_type_t *type, size_t *size) {
    char *obj_path = object_path(hash);
    if (!obj_path) return NULL;
    
    // Read compressed data
    size_t compressed_size;
    void *compressed = read_file(obj_path, &compressed_size);
    free(obj_path);
    
    if (!compressed) return NULL;
    
    // Decompress (we need to find the uncompressed size first)
    // Try progressively larger sizes
    void *decompressed = NULL;
    size_t try_size = compressed_size * 2;
    
    for (int i = 0; i < 5; i++) {
        uLongf dest_len = try_size;
        decompressed = malloc(dest_len);
        if (!decompressed) break;
        
        int result = uncompress(decompressed, &dest_len, compressed, compressed_size);
        if (result == Z_OK) {
            // Success! Parse header
            char *data = (char *)decompressed;
            char *space = strchr(data, ' ');
            if (!space) {
                free(decompressed);
                free(compressed);
                return NULL;
            }
            
            *space = '\0';
            const char *type_str = data;
            char *null_term = strchr(space + 1, '\0');
            if (!null_term) {
                free(decompressed);
                free(compressed);
                return NULL;
            }
            
            // Parse type
            if (type) {
                if (strcmp(type_str, "blob") == 0) *type = OBJ_BLOB;
                else if (strcmp(type_str, "tree") == 0) *type = OBJ_TREE;
                else if (strcmp(type_str, "commit") == 0) *type = OBJ_COMMIT;
            }
            
            // Parse size
            size_t obj_size = strtoull(space + 1, NULL, 10);
            if (size) *size = obj_size;
            
            // Extract actual data
            size_t header_len = (null_term - data) + 1;
            void *obj_data = malloc(obj_size);
            if (obj_data) {
                memcpy(obj_data, data + header_len, obj_size);
            }
            
            free(decompressed);
            free(compressed);
            return obj_data;
        }
        
        free(decompressed);
        try_size *= 2;
    }
    
    free(compressed);
    return NULL;
}

// ==================== Blob Operations ====================

int blob_write(blob_object_t *blob) {
    if (!blob) return -1;
    return object_write(blob->data, blob->header.size, OBJ_BLOB, &blob->header.hash);
}

blob_object_t *blob_read(const gyatt_hash_t *hash) {
    object_type_t type;
    size_t size;
    void *data = object_read(hash, &type, &size);
    
    if (!data || type != OBJ_BLOB) {
        free(data);
        return NULL;
    }
    
    blob_object_t *blob = calloc(1, sizeof(blob_object_t));
    if (!blob) {
        free(data);
        return NULL;
    }
    
    blob->header.type = OBJ_BLOB;
    blob->header.size = size;
    hash_copy(&blob->header.hash, hash);
    blob->data = data;
    
    return blob;
}

blob_object_t *blob_from_file(const char *path) {
    size_t size;
    void *data = read_file(path, &size);
    if (!data) return NULL;
    
    blob_object_t *blob = blob_create(data, size);
    free(data);
    
    return blob;
}

// ==================== Tree Operations ====================

int tree_write(tree_object_t *tree) {
    if (!tree) return -1;
    
    // Serialize tree: each entry is "mode name\0hash"
    buffer_t *buf = buffer_create(4096);
    
    for (size_t i = 0; i < tree->entry_count; i++) {
        tree_entry_t *entry = &tree->entries[i];
        
        buffer_append_uint(buf, entry->mode);
        buffer_append_char(buf, ' ');
        buffer_append_str(buf, entry->name);
        buffer_append_char(buf, '\0');
        buffer_append(buf, entry->hash.hash, HASH_SIZE);
    }
    
    int result = object_write(buf->data, buf->len, OBJ_TREE, &tree->header.hash);
    tree->header.size = buf->len;
    
    buffer_free(buf);
    return result;
}

tree_object_t *tree_read(const gyatt_hash_t *hash) {
    object_type_t type;
    size_t size;
    void *data = object_read(hash, &type, &size);
    
    if (!data || type != OBJ_TREE) {
        free(data);
        return NULL;
    }
    
    tree_object_t *tree = tree_create();
    if (!tree) {
        free(data);
        return NULL;
    }
    
    hash_copy(&tree->header.hash, hash);
    tree->header.size = size;
    
    // Parse tree entries
    char *ptr = (char *)data;
    char *end = ptr + size;
    
    while (ptr < end) {
        // Parse mode
        uint32_t mode = strtoul(ptr, &ptr, 10);
        if (*ptr != ' ') break;
        ptr++;
        
        // Parse name
        char *name = ptr;
        while (ptr < end && *ptr != '\0') ptr++;
        if (ptr >= end) break;
        ptr++;  // Skip null terminator
        
        // Parse hash
        if (ptr + HASH_SIZE > end) break;
        gyatt_hash_t entry_hash;
        memcpy(entry_hash.hash, ptr, HASH_SIZE);
        ptr += HASH_SIZE;
        
        // Add entry (assume blob for now, could be tree)
        tree_add_entry(tree, name, mode, &entry_hash, OBJ_BLOB);
    }
    
    free(data);
    return tree;
}

// ==================== Commit Operations ====================

int commit_write(commit_object_t *commit) {
    if (!commit) return -1;
    
    buffer_t *buf = buffer_create(4096);
    
    // Tree
    char tree_hex[HASH_HEX_SIZE];
    hash_to_hex(&commit->tree, tree_hex);
    buffer_append_str(buf, "tree ");
    buffer_append_str(buf, tree_hex);
    buffer_append_char(buf, '\n');
    
    // Parent (if not zero hash)
    int has_parent = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (commit->parent.hash[i] != 0) {
            has_parent = 1;
            break;
        }
    }
    
    if (has_parent) {
        char parent_hex[HASH_HEX_SIZE];
        hash_to_hex(&commit->parent, parent_hex);
        buffer_append_str(buf, "parent ");
        buffer_append_str(buf, parent_hex);
        buffer_append_char(buf, '\n');
    }
    
    // Author
    buffer_append_str(buf, "author ");
    buffer_append_str(buf, commit->author.name);
    buffer_append_str(buf, " <");
    buffer_append_str(buf, commit->author.email);
    buffer_append_str(buf, "> ");
    buffer_append_int(buf, commit->author.timestamp);
    buffer_append_str(buf, " +0000\n");
    
    // Committer
    buffer_append_str(buf, "committer ");
    buffer_append_str(buf, commit->committer.name);
    buffer_append_str(buf, " <");
    buffer_append_str(buf, commit->committer.email);
    buffer_append_str(buf, "> ");
    buffer_append_int(buf, commit->committer.timestamp);
    buffer_append_str(buf, " +0000\n");
    
    // Empty line before message
    buffer_append_char(buf, '\n');
    
    // Message
    buffer_append_str(buf, commit->message);
    
    int result = object_write(buf->data, buf->len, OBJ_COMMIT, &commit->header.hash);
    commit->header.size = buf->len;
    
    buffer_free(buf);
    return result;
}

commit_object_t *commit_read(const gyatt_hash_t *hash) {
    object_type_t type;
    size_t size;
    void *data = object_read(hash, &type, &size);
    
    if (!data || type != OBJ_COMMIT) {
        free(data);
        return NULL;
    }
    
    commit_object_t *commit = commit_create();
    if (!commit) {
        free(data);
        return NULL;
    }
    
    hash_copy(&commit->header.hash, hash);
    commit->header.size = size;
    
    // Parse commit data line by line
    char *content = (char *)data;
    char *line_start = content;
    char *ptr = content;
    int in_message = 0;
    char *message_start = NULL;
    
    while (ptr < content + size) {
        // Find end of line
        char *line_end = ptr;
        while (line_end < content + size && *line_end != '\n') {
            line_end++;
        }
        
        size_t line_len = line_end - line_start;
        
        if (!in_message) {
            if (line_len == 0) {
                // Empty line marks start of commit message
                in_message = 1;
                message_start = line_end + 1;
            } else if (strncmp(line_start, "tree ", 5) == 0 && line_len > 5) {
                char tree_hex[HASH_HEX_SIZE];
                size_t hex_len = (line_len - 5) < (HASH_HEX_SIZE - 1) ? (line_len - 5) : (HASH_HEX_SIZE - 1);
                memcpy(tree_hex, line_start + 5, hex_len);
                tree_hex[hex_len] = '\0';
                hex_to_hash(tree_hex, &commit->tree);
            } else if (strncmp(line_start, "parent ", 7) == 0 && line_len > 7) {
                char parent_hex[HASH_HEX_SIZE];
                size_t hex_len = (line_len - 7) < (HASH_HEX_SIZE - 1) ? (line_len - 7) : (HASH_HEX_SIZE - 1);
                memcpy(parent_hex, line_start + 7, hex_len);
                parent_hex[hex_len] = '\0';
                hex_to_hash(parent_hex, &commit->parent);
            } else if (strncmp(line_start, "author ", 7) == 0) {
                // Parse: "author Name <email> timestamp +0000"
                char *name_start = line_start + 7;
                char *email_start = memchr(name_start, '<', line_len - 7);
                if (email_start) {
                    size_t name_len = email_start - name_start - 1;
                    if (name_len > 0 && name_len < sizeof(commit->author.name)) {
                        memcpy(commit->author.name, name_start, name_len);
                        commit->author.name[name_len] = '\0';
                    }
                    
                    char *email_end = memchr(email_start, '>', line_end - email_start);
                    if (email_end) {
                        size_t email_len = email_end - email_start - 1;
                        if (email_len > 0 && email_len < sizeof(commit->author.email)) {
                            memcpy(commit->author.email, email_start + 1, email_len);
                            commit->author.email[email_len] = '\0';
                        }
                        
                        commit->author.timestamp = atol(email_end + 2);
                    }
                }
            } else if (strncmp(line_start, "committer ", 10) == 0) {
                // Parse committer (same format as author)
                char *name_start = line_start + 10;
                char *email_start = memchr(name_start, '<', line_len - 10);
                if (email_start) {
                    size_t name_len = email_start - name_start - 1;
                    if (name_len > 0 && name_len < sizeof(commit->committer.name)) {
                        memcpy(commit->committer.name, name_start, name_len);
                        commit->committer.name[name_len] = '\0';
                    }
                    
                    char *email_end = memchr(email_start, '>', line_end - email_start);
                    if (email_end) {
                        size_t email_len = email_end - email_start - 1;
                        if (email_len > 0 && email_len < sizeof(commit->committer.email)) {
                            memcpy(commit->committer.email, email_start + 1, email_len);
                            commit->committer.email[email_len] = '\0';
                        }
                        
                        commit->committer.timestamp = atol(email_end + 2);
                    }
                }
            }
        }
        
        // Move to next line
        if (line_end < content + size) {
            line_start = line_end + 1;
            ptr = line_start;
        } else {
            break;
        }
    }
    
    // Copy commit message
    if (message_start && message_start < content + size) {
        size_t message_len = (content + size) - message_start;
        if (message_len > 0 && message_len < sizeof(commit->message)) {
            memcpy(commit->message, message_start, message_len);
            commit->message[message_len] = '\0';
        }
    }
    
    free(data);
    return commit;
}
