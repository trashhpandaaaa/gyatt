#ifndef IPFS_H
#define IPFS_H

#include <stddef.h>
#include <stdbool.h>

// IPFS daemon configuration
#define IPFS_DEFAULT_HOST "127.0.0.1"
#define IPFS_DEFAULT_PORT 5001
#define IPFS_API_PATH "/api/v0"

// Maximum CID length (CIDv1 can be up to 100+ chars, but we'll be safe)
#define IPFS_CID_MAX_LEN 256

// IPFS connection handle
typedef struct {
    char host[256];
    int port;
    int timeout_ms;
} ipfs_client_t;

// IPFS response structure
typedef struct {
    char *data;
    size_t size;
    int status_code;
    char *error;
} ipfs_response_t;

// Initialize IPFS client
ipfs_client_t* ipfs_client_init(const char *host, int port);

// Free IPFS client
void ipfs_client_free(ipfs_client_t *client);

// Check if IPFS daemon is running
bool ipfs_is_online(ipfs_client_t *client);

// Get IPFS version
char* ipfs_version(ipfs_client_t *client);

// Add data to IPFS (returns CID)
char* ipfs_add(ipfs_client_t *client, const void *data, size_t size);

// Get data from IPFS by CID
ipfs_response_t* ipfs_cat(ipfs_client_t *client, const char *cid);

// Pin a CID (keep it in local storage)
bool ipfs_pin_add(ipfs_client_t *client, const char *cid);

// Unpin a CID
bool ipfs_pin_rm(ipfs_client_t *client, const char *cid);

// List pinned CIDs
char** ipfs_pin_ls(ipfs_client_t *client, size_t *count);

// Free response
void ipfs_response_free(ipfs_response_t *response);

// Free string array
void ipfs_free_string_array(char **array, size_t count);

#endif // IPFS_H
