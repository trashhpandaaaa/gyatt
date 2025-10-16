#ifndef GYATT_H
#define GYATT_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

// Version
#define GYATT_VERSION "0.1.0"

// Configuration
#define GYATT_DIR ".gyatt"
#define GYATT_OBJECTS_DIR ".gyatt/objects"
#define GYATT_REFS_DIR ".gyatt/refs"
#define GYATT_REFS_HEADS_DIR ".gyatt/refs/heads"
#define GYATT_HEAD_FILE ".gyatt/HEAD"
#define GYATT_INDEX_FILE ".gyatt/index"
#define GYATT_CONFIG_FILE ".gyatt/config"

// Hash size (SHA-1)
#define HASH_SIZE 20
#define HASH_HEX_SIZE 41

// Object types
typedef enum {
    OBJ_BLOB = 1,
    OBJ_TREE = 2,
    OBJ_COMMIT = 3
} object_type_t;

// Hash structure
typedef struct {
    unsigned char hash[HASH_SIZE];
} gyatt_hash_t;

// Command functions
int cmd_init(int argc, char *argv[]);
int cmd_add(int argc, char *argv[]);
int cmd_commit(int argc, char *argv[]);
int cmd_status(int argc, char *argv[]);
int cmd_log(int argc, char *argv[]);
int cmd_branch(int argc, char *argv[]);
int cmd_checkout(int argc, char *argv[]);
int cmd_push(int argc, char *argv[]);
int cmd_pull(int argc, char *argv[]);
int cmd_server(int argc, char *argv[]);

// Utility functions
void hash_to_hex(const gyatt_hash_t *hash, char *hex);
void hex_to_hash(const char *hex, gyatt_hash_t *hash);
int is_gyatt_repo(void);
char *get_gyatt_dir(void);

#endif // GYATT_H
