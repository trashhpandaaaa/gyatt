#include "ipfs_storage.h"
#include "../utils.h"
#include "../hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

// Because storing everything on your own server is so 2010 🌐

ipfs_storage_t* ipfs_storage_init(const char *repo_path) {
    ipfs_storage_t *storage = malloc(sizeof(ipfs_storage_t));
    if (!storage) return NULL;

    // Initialize IPFS client
    storage->client = ipfs_client_init(NULL, 0);
    if (!storage->client) {
        free(storage);
        return NULL;
    }

    // Set up refs path (.gyatt/ipfs-refs)
    if (repo_path) {
        snprintf(storage->refs_path, sizeof(storage->refs_path), 
                 "%s/.gyatt/ipfs-refs", repo_path);
    } else {
        snprintf(storage->refs_path, sizeof(storage->refs_path), 
                 ".gyatt/ipfs-refs");
    }

    // Create ipfs-refs directory if it doesn't exist
    mkdir(storage->refs_path, 0755);

    storage->auto_pin = true; // Pin by default

    return storage;
}

void ipfs_storage_free(ipfs_storage_t *storage) {
    if (storage) {
        ipfs_client_free(storage->client);
        free(storage);
    }
}

// Helper: Get the path for a CID mapping file
static void get_mapping_path(ipfs_storage_t *storage, 
                             const gyatt_hash_t *hash,
                             char *out_path, size_t path_size) {
    char hash_hex[HASH_HEX_SIZE];
    hash_to_hex(hash, hash_hex);
    
    // Store as .gyatt/ipfs-refs/ab/cdef1234... (2-level directory)
    snprintf(out_path, path_size, "%s/%.2s/%s", 
             storage->refs_path, hash_hex, hash_hex + 2);
}

char* ipfs_storage_get_cid(ipfs_storage_t *storage,
                            const gyatt_hash_t *hash) {
    char path[4096];
    get_mapping_path(storage, hash, path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    char *cid = malloc(IPFS_CID_MAX_LEN);
    if (!cid) {
        fclose(f);
        return NULL;
    }

    if (fgets(cid, IPFS_CID_MAX_LEN, f) == NULL) {
        free(cid);
        fclose(f);
        return NULL;
    }

    // Remove trailing newline
    size_t len = strlen(cid);
    if (len > 0 && cid[len - 1] == '\n') {
        cid[len - 1] = '\0';
    }

    fclose(f);
    return cid;
}

int ipfs_storage_save_mapping(ipfs_storage_t *storage,
                               const gyatt_hash_t *hash,
                               const char *cid) {
    char path[4096];
    get_mapping_path(storage, hash, path, sizeof(path));

    // Create parent directory
    char dir_path[4096];
    char hash_hex[HASH_HEX_SIZE];
    hash_to_hex(hash, hash_hex);
    snprintf(dir_path, sizeof(dir_path), "%s/%.2s", storage->refs_path, hash_hex);
    mkdir(dir_path, 0755);

    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Failed to save CID mapping: %s\n", path);
        return -1;
    }

    fprintf(f, "%s\n", cid);
    fclose(f);
    return 0;
}

bool ipfs_storage_has_object(ipfs_storage_t *storage,
                              const gyatt_hash_t *hash) {
    char path[4096];
    get_mapping_path(storage, hash, path, sizeof(path));
    
    struct stat st;
    return stat(path, &st) == 0;
}

char* ipfs_storage_put_object(ipfs_storage_t *storage,
                               const gyatt_hash_t *hash,
                               const void *data,
                               size_t size) {
    // Check if already uploaded
    if (ipfs_storage_has_object(storage, hash)) {
        char hash_hex[HASH_HEX_SIZE];
        hash_to_hex(hash, hash_hex);
        printf("Object %s already in IPFS\n", hash_hex);
        return ipfs_storage_get_cid(storage, hash);
    }

    // Upload to IPFS
    char *cid = ipfs_add(storage->client, data, size);
    if (!cid) {
        char hash_hex[HASH_HEX_SIZE];
        hash_to_hex(hash, hash_hex);
        fprintf(stderr, "Failed to upload object %s to IPFS\n", hash_hex);
        return NULL;
    }

    // Save mapping
    if (ipfs_storage_save_mapping(storage, hash, cid) < 0) {
        free(cid);
        return NULL;
    }

    // Pin if auto-pin is enabled
    if (storage->auto_pin) {
        if (!ipfs_pin_add(storage->client, cid)) {
            fprintf(stderr, "Warning: Failed to pin CID %s\n", cid);
        }
    }

    char hash_hex[HASH_HEX_SIZE];
    hash_to_hex(hash, hash_hex);
    printf("✓ Uploaded %s -> %s\n", hash_hex, cid);

    return cid;
}

void* ipfs_storage_get_object(ipfs_storage_t *storage,
                               const gyatt_hash_t *hash,
                               size_t *out_size) {
    // Get CID from mapping
    char *cid = ipfs_storage_get_cid(storage, hash);
    if (!cid) {
        char hash_hex[HASH_HEX_SIZE];
        hash_to_hex(hash, hash_hex);
        fprintf(stderr, "No IPFS mapping found for %s\n", hash_hex);
        return NULL;
    }

    // Download from IPFS
    ipfs_response_t *response = ipfs_cat(storage->client, cid);
    free(cid);

    if (!response || response->status_code != 200) {
        if (response) ipfs_response_free(response);
        return NULL;
    }

    // Verify SHA-1 hash
    gyatt_hash_t computed_hash;
    sha1_hash(response->data, response->size, &computed_hash);

    if (memcmp(&computed_hash, hash, sizeof(gyatt_hash_t)) != 0) {
        char hash_hex[HASH_HEX_SIZE];
        hash_to_hex(hash, hash_hex);
        fprintf(stderr, "Hash mismatch for %s! Data corrupted!\n", hash_hex);
        ipfs_response_free(response);
        return NULL;
    }

    // Extract data and size
    void *data = response->data;
    *out_size = response->size;

    // Free response struct but keep data
    free(response->error);
    free(response);

    return data;
}

int ipfs_storage_push_branch(ipfs_storage_t *storage, const char *branch_name) {
    char branch_path[4096];
    snprintf(branch_path, sizeof(branch_path), ".gyatt/refs/heads/%s", branch_name);

    // Read branch commit hash
    FILE *f = fopen(branch_path, "r");
    if (!f) {
        fprintf(stderr, "Branch not found: %s\n", branch_name);
        return -1;
    }

    char commit_hex[HASH_HEX_SIZE];
    if (fgets(commit_hex, sizeof(commit_hex), f) == NULL) {
        fclose(f);
        return -1;
    }
    fclose(f);

    // Remove trailing newline
    size_t len = strlen(commit_hex);
    if (len > 0 && commit_hex[len - 1] == '\n') {
        commit_hex[len - 1] = '\0';
    }

    gyatt_hash_t commit_hash;
    hex_to_hash(commit_hex, &commit_hash);

    printf("Pushing branch '%s' (commit %s) to IPFS...\n", branch_name, commit_hex);

    // Upload commit and walk the tree
    // This is a simplified version - we'll recursively upload all objects
    object_type_t type;
    size_t size;
    void *data = object_read(&commit_hash, &type, &size);
    if (!data) {
        fprintf(stderr, "Failed to read commit object\n");
        return -1;
    }

    // Upload the commit
    char *cid = ipfs_storage_put_object(storage, &commit_hash, data, size);
    free(data);

    if (!cid) {
        return -1;
    }

    printf("✓ Branch '%s' pushed to IPFS (root CID: %s)\n", branch_name, cid);
    free(cid);

    return 0;
}

int ipfs_storage_push_all(ipfs_storage_t *storage) {
    char objects_path[4096];
    snprintf(objects_path, sizeof(objects_path), ".gyatt/objects");

    printf("Scanning local objects...\n");

    // Walk through all objects in .gyatt/objects
    DIR *dir = opendir(objects_path);
    if (!dir) {
        fprintf(stderr, "Failed to open objects directory\n");
        return -1;
    }

    int uploaded = 0;
    int skipped = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (strlen(entry->d_name) != 2) continue;

        // Scan subdirectory
        char subdir_path[4096];
        snprintf(subdir_path, sizeof(subdir_path), "%s/%s", objects_path, entry->d_name);

        DIR *subdir = opendir(subdir_path);
        if (!subdir) continue;

        struct dirent *obj_entry;
        while ((obj_entry = readdir(subdir)) != NULL) {
            if (obj_entry->d_name[0] == '.') continue;

            // Reconstruct full hash
            char hash_hex[HASH_HEX_SIZE];
            snprintf(hash_hex, sizeof(hash_hex), "%s%s", entry->d_name, obj_entry->d_name);

            gyatt_hash_t hash;
            hex_to_hash(hash_hex, &hash);

            // Skip if already uploaded
            if (ipfs_storage_has_object(storage, &hash)) {
                skipped++;
                continue;
            }

            // Read object
            object_type_t type;
            size_t size;
            void *data = object_read(&hash, &type, &size);
            if (!data) {
                fprintf(stderr, "Failed to read object %s\n", hash_hex);
                continue;
            }

            // Upload to IPFS
            char *cid = ipfs_storage_put_object(storage, &hash, data, size);
            free(data);

            if (cid) {
                uploaded++;
                free(cid);
            }
        }

        closedir(subdir);
    }

    closedir(dir);

    printf("\n✓ Push complete: %d uploaded, %d skipped\n", uploaded, skipped);
    return 0;
}

char* ipfs_storage_publish_manifest(ipfs_storage_t *storage) {
    printf("Creating repository manifest...\n");

    // Build manifest JSON
    char manifest[65536] = {0}; // 64KB should be enough
    strcat(manifest, "{\n");
    strcat(manifest, "  \"version\": \"1.0\",\n");
    strcat(manifest, "  \"type\": \"gyatt-repository\",\n");
    strcat(manifest, "  \"branches\": {\n");

    // Read all branches
    DIR *dir = opendir(".gyatt/refs/heads");
    if (!dir) {
        fprintf(stderr, "Failed to open refs/heads directory\n");
        return NULL;
    }

    int branch_count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        // Read branch commit
        char branch_path[4096];
        snprintf(branch_path, sizeof(branch_path), ".gyatt/refs/heads/%s", entry->d_name);

        FILE *f = fopen(branch_path, "r");
        if (!f) continue;

        char commit_hex[HASH_HEX_SIZE];
        if (fgets(commit_hex, sizeof(commit_hex), f) == NULL) {
            fclose(f);
            continue;
        }
        fclose(f);

        // Remove trailing newline
        size_t len = strlen(commit_hex);
        if (len > 0 && commit_hex[len - 1] == '\n') {
            commit_hex[len - 1] = '\0';
        }

        // Get CID for commit
        gyatt_hash_t commit_hash;
        hex_to_hash(commit_hex, &commit_hash);
        char *cid = ipfs_storage_get_cid(storage, &commit_hash);

        if (cid) {
            if (branch_count > 0) strcat(manifest, ",\n");
            char entry_json[512];
            snprintf(entry_json, sizeof(entry_json),
                     "    \"%s\": {\n"
                     "      \"commit\": \"%s\",\n"
                     "      \"cid\": \"%s\"\n"
                     "    }",
                     entry->d_name, commit_hex, cid);
            strcat(manifest, entry_json);
            free(cid);
            branch_count++;
        }
    }

    closedir(dir);

    strcat(manifest, "\n  }\n");
    strcat(manifest, "}\n");

    printf("Manifest:\n%s\n", manifest);

    // Upload manifest to IPFS
    char *manifest_cid = ipfs_add(storage->client, manifest, strlen(manifest));
    if (!manifest_cid) {
        fprintf(stderr, "Failed to upload manifest to IPFS\n");
        return NULL;
    }

    // Pin the manifest
    if (storage->auto_pin) {
        ipfs_pin_add(storage->client, manifest_cid);
    }

    printf("\n✓ Manifest published to IPFS: %s\n", manifest_cid);
    printf("  View at: https://ipfs.io/ipfs/%s\n", manifest_cid);

    return manifest_cid;
}
