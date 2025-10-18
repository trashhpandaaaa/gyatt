#include "../gyatt.h"
#include "../ipfs/ipfs_storage.h"
#include "../hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// gyatt ipfs - IPFS integration commands
// Because decentralization is the future 🚀

static void print_ipfs_help(void) {
    printf("Usage: gyatt ipfs <command> [options]\n\n");
    printf("IPFS Integration Commands:\n");
    printf("  init       Check IPFS daemon status and initialize IPFS storage\n");
    printf("  push       Upload repository objects to IPFS\n");
    printf("  pull       Download repository objects from IPFS manifest\n");
    printf("  publish    Create and publish repository manifest to IPFS\n");
    printf("  status     Show IPFS storage status and statistics\n");
    printf("\nExamples:\n");
    printf("  gyatt ipfs init           # Check IPFS daemon\n");
    printf("  gyatt ipfs push           # Upload all objects to IPFS\n");
    printf("  gyatt ipfs push main      # Upload specific branch\n");
    printf("  gyatt ipfs publish        # Publish manifest and get shareable CID\n");
    printf("  gyatt ipfs status         # Show what's uploaded\n");
}

static int cmd_ipfs_init(void) {
    printf("Initializing IPFS storage...\n\n");

    if (!is_gyatt_repo()) {
        fprintf(stderr, "Not a Gyatt repository\n");
        return 1;
    }

    ipfs_storage_t *storage = ipfs_storage_init(".");
    if (!storage) {
        fprintf(stderr, "Failed to initialize IPFS storage\n");
        return 1;
    }

    printf("Checking IPFS daemon...\n");
    if (!ipfs_is_online(storage->client)) {
        fprintf(stderr, "✗ IPFS daemon is not running\n\n");
        fprintf(stderr, "Please start the IPFS daemon:\n");
        fprintf(stderr, "  ipfs daemon\n\n");
        fprintf(stderr, "If IPFS is not installed, get it from:\n");
        fprintf(stderr, "  https://docs.ipfs.tech/install/\n");
        ipfs_storage_free(storage);
        return 1;
    }

    printf("✓ IPFS daemon is online\n");

    // Get version info
    char *version = ipfs_version(storage->client);
    if (version) {
        printf("  IPFS version: %s\n", version);
        free(version);
    }

    printf("\n✓ IPFS storage initialized successfully\n");
    printf("  Storage path: .gyatt/ipfs-refs\n");
    
    ipfs_storage_free(storage);
    return 0;
}

static int cmd_ipfs_push(int argc, char *argv[]) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Not a Gyatt repository\n");
        return 1;
    }

    ipfs_storage_t *storage = ipfs_storage_init(".");
    if (!storage) {
        fprintf(stderr, "Failed to initialize IPFS storage\n");
        return 1;
    }

    if (!ipfs_is_online(storage->client)) {
        fprintf(stderr, "✗ IPFS daemon is not running. Run: gyatt ipfs init\n");
        ipfs_storage_free(storage);
        return 1;
    }

    int result;
    if (argc > 0) {
        // Push specific branch
        printf("Pushing branch '%s' to IPFS...\n\n", argv[0]);
        result = ipfs_storage_push_branch(storage, argv[0]);
    } else {
        // Push all objects
        printf("Pushing all objects to IPFS...\n\n");
        result = ipfs_storage_push_all(storage);
    }

    ipfs_storage_free(storage);
    return result;
}

static int cmd_ipfs_publish(void) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Not a Gyatt repository\n");
        return 1;
    }

    ipfs_storage_t *storage = ipfs_storage_init(".");
    if (!storage) {
        fprintf(stderr, "Failed to initialize IPFS storage\n");
        return 1;
    }

    if (!ipfs_is_online(storage->client)) {
        fprintf(stderr, "✗ IPFS daemon is not running. Run: gyatt ipfs init\n");
        ipfs_storage_free(storage);
        return 1;
    }

    char *manifest_cid = ipfs_storage_publish_manifest(storage);
    if (!manifest_cid) {
        fprintf(stderr, "Failed to publish manifest\n");
        ipfs_storage_free(storage);
        return 1;
    }

    printf("\n🎉 Repository published to IPFS!\n");
    printf("\nShare this CID to clone your repository:\n");
    printf("  %s\n\n", manifest_cid);
    printf("Anyone can view your repository at:\n");
    printf("  https://ipfs.io/ipfs/%s\n", manifest_cid);
    printf("  https://gateway.pinata.cloud/ipfs/%s\n", manifest_cid);
    printf("  https://cloudflare-ipfs.com/ipfs/%s\n", manifest_cid);

    free(manifest_cid);
    ipfs_storage_free(storage);
    return 0;
}

static int cmd_ipfs_status(void) {
    if (!is_gyatt_repo()) {
        fprintf(stderr, "Not a Gyatt repository\n");
        return 1;
    }

    ipfs_storage_t *storage = ipfs_storage_init(".");
    if (!storage) {
        fprintf(stderr, "Failed to initialize IPFS storage\n");
        return 1;
    }

    printf("=== IPFS Storage Status ===\n\n");

    // Check daemon status
    printf("IPFS Daemon: ");
    if (ipfs_is_online(storage->client)) {
        printf("✓ Online\n");
    } else {
        printf("✗ Offline\n");
        ipfs_storage_free(storage);
        return 1;
    }

    // Count local objects
    printf("\nLocal Objects:\n");
    int total_objects = 0;
    int uploaded_objects = 0;

    char objects_path[4096];
    snprintf(objects_path, sizeof(objects_path), ".gyatt/objects");

    DIR *dir = opendir(objects_path);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strlen(entry->d_name) != 2) continue;

            char subdir_path[4096];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", objects_path, entry->d_name);

            DIR *subdir = opendir(subdir_path);
            if (!subdir) continue;

            struct dirent *obj_entry;
            while ((obj_entry = readdir(subdir)) != NULL) {
                if (obj_entry->d_name[0] == '.') continue;

                total_objects++;

                // Check if uploaded to IPFS
                char hash_hex[HASH_HEX_SIZE];
                snprintf(hash_hex, sizeof(hash_hex), "%s%s", entry->d_name, obj_entry->d_name);

                gyatt_hash_t hash;
                hex_to_hash(hash_hex, &hash);

                if (ipfs_storage_has_object(storage, &hash)) {
                    uploaded_objects++;
                }
            }
            closedir(subdir);
        }
        closedir(dir);
    }

    printf("  Total: %d objects\n", total_objects);
    printf("  Uploaded to IPFS: %d objects\n", uploaded_objects);
    printf("  Not uploaded: %d objects\n", total_objects - uploaded_objects);

    if (total_objects > 0) {
        int percent = (uploaded_objects * 100) / total_objects;
        printf("  Upload progress: %d%%\n", percent);
    }

    // Show branches
    printf("\nBranches:\n");
    DIR *refs_dir = opendir(".gyatt/refs/heads");
    if (refs_dir) {
        struct dirent *entry;
        while ((entry = readdir(refs_dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;

            printf("  %s", entry->d_name);

            // Check if branch is uploaded
            char branch_path[4096];
            snprintf(branch_path, sizeof(branch_path), ".gyatt/refs/heads/%s", entry->d_name);

            FILE *f = fopen(branch_path, "r");
            if (f) {
                char commit_hex[HASH_HEX_SIZE];
                if (fgets(commit_hex, sizeof(commit_hex), f) != NULL) {
                    // Remove newline
                    size_t len = strlen(commit_hex);
                    if (len > 0 && commit_hex[len - 1] == '\n') {
                        commit_hex[len - 1] = '\0';
                    }

                    gyatt_hash_t commit_hash;
                    hex_to_hash(commit_hex, &commit_hash);

                    if (ipfs_storage_has_object(storage, &commit_hash)) {
                        char *cid = ipfs_storage_get_cid(storage, &commit_hash);
                        if (cid) {
                            printf(" (✓ in IPFS: %.16s...)", cid);
                            free(cid);
                        }
                    } else {
                        printf(" (not uploaded)");
                    }
                }
                fclose(f);
            }
            printf("\n");
        }
        closedir(refs_dir);
    }

    ipfs_storage_free(storage);
    return 0;
}

int cmd_ipfs(int argc, char *argv[]) {
    if (argc < 1) {
        print_ipfs_help();
        return 0;
    }

    const char *subcommand = argv[0];

    if (strcmp(subcommand, "init") == 0) {
        return cmd_ipfs_init();
    } else if (strcmp(subcommand, "push") == 0) {
        return cmd_ipfs_push(argc - 1, argv + 1);
    } else if (strcmp(subcommand, "publish") == 0) {
        return cmd_ipfs_publish();
    } else if (strcmp(subcommand, "status") == 0) {
        return cmd_ipfs_status();
    } else if (strcmp(subcommand, "help") == 0 || strcmp(subcommand, "--help") == 0) {
        print_ipfs_help();
        return 0;
    } else {
        fprintf(stderr, "Unknown IPFS command: %s\n\n", subcommand);
        print_ipfs_help();
        return 1;
    }
}
