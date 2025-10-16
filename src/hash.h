#ifndef HASH_H
#define HASH_H

#include "gyatt.h"
#include <stddef.h>

// SHA-1 hashing functions
void sha1_hash(const void *data, size_t len, gyatt_hash_t *hash);
void sha1_hash_file(const char *path, gyatt_hash_t *hash);

// Hash utility functions
void hash_to_hex(const gyatt_hash_t *hash, char *hex);
void hex_to_hash(const char *hex, gyatt_hash_t *hash);
int hash_compare(const gyatt_hash_t *h1, const gyatt_hash_t *h2);
void hash_copy(gyatt_hash_t *dest, const gyatt_hash_t *src);

#endif // HASH_H
