#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/ipfs/ipfs.h"

int main() {
    printf("=== Gyatt IPFS Client Test ===\n\n");

    // Initialize IPFS client
    ipfs_client_t *client = ipfs_client_init(NULL, 0);
    if (!client) {
        fprintf(stderr, "Failed to initialize IPFS client\n");
        return 1;
    }

    printf("IPFS client initialized (connecting to %s:%d)\n", client->host, client->port);

    // Check if IPFS daemon is online
    printf("\nChecking if IPFS daemon is running...\n");
    if (ipfs_is_online(client)) {
        printf("✓ IPFS daemon is online!\n");
    } else {
        printf("✗ IPFS daemon is offline or unreachable\n");
        printf("  Make sure IPFS is installed and running:\n");
        printf("    ipfs daemon\n");
        ipfs_client_free(client);
        return 1;
    }

    // Get IPFS version
    printf("\nGetting IPFS version...\n");
    char *version = ipfs_version(client);
    if (version) {
        printf("IPFS version response: %s\n", version);
        free(version);
    } else {
        printf("Failed to get IPFS version\n");
    }

    // Test adding data to IPFS
    printf("\n=== Testing IPFS Add ===\n");
    const char *test_data = "Hello from Gyatt! This is a test of IPFS integration.";
    size_t test_size = strlen(test_data);
    
    printf("Adding data to IPFS: \"%s\"\n", test_data);
    char *cid = ipfs_add(client, test_data, test_size);
    if (cid) {
        printf("✓ Data added successfully!\n");
        printf("  CID: %s\n", cid);

        // Test retrieving the data
        printf("\n=== Testing IPFS Cat ===\n");
        printf("Retrieving data from IPFS using CID...\n");
        ipfs_response_t *response = ipfs_cat(client, cid);
        if (response && response->status_code == 200) {
            printf("✓ Data retrieved successfully!\n");
            printf("  Size: %zu bytes\n", response->size);
            printf("  Content: %.*s\n", (int)response->size, response->data);
            
            // Verify data matches
            if (response->size == test_size && memcmp(response->data, test_data, test_size) == 0) {
                printf("  ✓ Data matches original!\n");
            } else {
                printf("  ✗ Data mismatch!\n");
            }
        } else {
            printf("✗ Failed to retrieve data\n");
            if (response && response->error) {
                printf("  Error: %s\n", response->error);
            }
        }
        ipfs_response_free(response);

        // Test pinning
        printf("\n=== Testing IPFS Pin ===\n");
        printf("Pinning CID to keep it in local storage...\n");
        if (ipfs_pin_add(client, cid)) {
            printf("✓ CID pinned successfully!\n");
        } else {
            printf("✗ Failed to pin CID (it may already be pinned)\n");
        }

        free(cid);
    } else {
        printf("✗ Failed to add data to IPFS\n");
    }

    // List pinned CIDs
    printf("\n=== Listing Pinned CIDs ===\n");
    size_t pin_count = 0;
    char **pins = ipfs_pin_ls(client, &pin_count);
    if (pins && pin_count > 0) {
        printf("Found %zu pinned CID(s):\n", pin_count);
        for (size_t i = 0; i < pin_count && i < 5; i++) { // Show first 5
            printf("  - %s\n", pins[i]);
        }
        if (pin_count > 5) {
            printf("  ... and %zu more\n", pin_count - 5);
        }
        ipfs_free_string_array(pins, pin_count);
    } else {
        printf("No pinned CIDs found or failed to list pins\n");
    }

    printf("\n=== Test Complete ===\n");
    ipfs_client_free(client);
    return 0;
}
