#ifndef IPFS_STORAGE_H
#define IPFS_STORAGE_H

#include "../gyatt.h"
#include "../object.h"
#include "ipfs.h"
#include <stdbool.h>

// IPFS storage configuration
typedef struct {
    ipfs_client_t *client;
    char refs_path[4096];  // Path to .gyatt/ipfs-refs
    bool auto_pin;         // Automatically pin uploaded objects
} ipfs_storage_t;

// Initialize IPFS storage for a repository
ipfs_storage_t* ipfs_storage_init(const char *repo_path);

// Free IPFS storage
void ipfs_storage_free(ipfs_storage_t *storage);

// Upload a Gyatt object to IPFS and store SHA-1 -> CID mapping
// Returns the CID on success, NULL on failure
char* ipfs_storage_put_object(ipfs_storage_t *storage, 
                               const gyatt_hash_t *hash,
                               const void *data, 
                               size_t size);

// Download a Gyatt object from IPFS by SHA-1 hash
// Looks up the CID from the mapping and retrieves the data
void* ipfs_storage_get_object(ipfs_storage_t *storage,
                               const gyatt_hash_t *hash,
                               size_t *out_size);

// Check if an object exists in IPFS (has a CID mapping)
bool ipfs_storage_has_object(ipfs_storage_t *storage,
                              const gyatt_hash_t *hash);

// Get the CID for a given SHA-1 hash
char* ipfs_storage_get_cid(ipfs_storage_t *storage,
                            const gyatt_hash_t *hash);

// Store a SHA-1 -> CID mapping
int ipfs_storage_save_mapping(ipfs_storage_t *storage,
                               const gyatt_hash_t *hash,
                               const char *cid);

// Upload all objects in the repository to IPFS
int ipfs_storage_push_all(ipfs_storage_t *storage);

// Upload a specific branch and its history to IPFS
int ipfs_storage_push_branch(ipfs_storage_t *storage, const char *branch_name);

// Create and publish a repository manifest to IPFS
// Returns the manifest CID
char* ipfs_storage_publish_manifest(ipfs_storage_t *storage);

#endif // IPFS_STORAGE_H
