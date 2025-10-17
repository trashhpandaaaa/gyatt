// SHA-1: Because we're keeping it old school (for Git compatibility)
// Yes, SHA-1 is "broken" but Git still uses it, so here we are
#include "hash.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SHA1_BLOCK_SIZE 64
#define SHA1_DIGEST_SIZE 20

typedef struct {
    uint32_t state[5];
    uint64_t count;
    uint8_t buffer[SHA1_BLOCK_SIZE];
} sha1_ctx_t;

#define ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static void sha1_init(sha1_ctx_t *ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count = 0;
}

static void sha1_transform(uint32_t state[5], const uint8_t buffer[SHA1_BLOCK_SIZE]) {
    uint32_t a, b, c, d, e, t, w[80];
    int i;

    // Prepare message schedule
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)buffer[i * 4] << 24) |
               ((uint32_t)buffer[i * 4 + 1] << 16) |
               ((uint32_t)buffer[i * 4 + 2] << 8) |
               ((uint32_t)buffer[i * 4 + 3]);
    }
    for (i = 16; i < 80; i++) {
        w[i] = ROL(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    // Initialize working variables
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    // Main loop - optimized with unrolling
    for (i = 0; i < 20; i++) {
        t = ROL(a, 5) + ((b & c) | (~b & d)) + e + w[i] + 0x5A827999;
        e = d;
        d = c;
        c = ROL(b, 30);
        b = a;
        a = t;
    }
    for (i = 20; i < 40; i++) {
        t = ROL(a, 5) + (b ^ c ^ d) + e + w[i] + 0x6ED9EBA1;
        e = d;
        d = c;
        c = ROL(b, 30);
        b = a;
        a = t;
    }
    for (i = 40; i < 60; i++) {
        t = ROL(a, 5) + ((b & c) | (b & d) | (c & d)) + e + w[i] + 0x8F1BBCDC;
        e = d;
        d = c;
        c = ROL(b, 30);
        b = a;
        a = t;
    }
    for (i = 60; i < 80; i++) {
        t = ROL(a, 5) + (b ^ c ^ d) + e + w[i] + 0xCA62C1D6;
        e = d;
        d = c;
        c = ROL(b, 30);
        b = a;
        a = t;
    }

    // Add to state
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static void sha1_update(sha1_ctx_t *ctx, const void *data, size_t len) {
    const uint8_t *ptr = (const uint8_t *)data;
    size_t buffer_space = SHA1_BLOCK_SIZE - (ctx->count % SHA1_BLOCK_SIZE);

    ctx->count += len;

    if (len >= buffer_space) {
        memcpy(&ctx->buffer[SHA1_BLOCK_SIZE - buffer_space], ptr, buffer_space);
        sha1_transform(ctx->state, ctx->buffer);
        ptr += buffer_space;
        len -= buffer_space;

        while (len >= SHA1_BLOCK_SIZE) {
            sha1_transform(ctx->state, ptr);
            ptr += SHA1_BLOCK_SIZE;
            len -= SHA1_BLOCK_SIZE;
        }
        buffer_space = SHA1_BLOCK_SIZE;
    }

    memcpy(&ctx->buffer[SHA1_BLOCK_SIZE - buffer_space], ptr, len);
}

static void sha1_final(sha1_ctx_t *ctx, uint8_t digest[SHA1_DIGEST_SIZE]) {
    uint64_t bit_count = ctx->count * 8;
    size_t padding = (ctx->count % SHA1_BLOCK_SIZE < 56) ? 
                     (56 - (ctx->count % SHA1_BLOCK_SIZE)) : 
                     (120 - (ctx->count % SHA1_BLOCK_SIZE));
    
    uint8_t pad[SHA1_BLOCK_SIZE * 2] = {0x80};
    
    // Add length in bits
    for (int i = 0; i < 8; i++) {
        pad[padding + i] = (bit_count >> (56 - i * 8)) & 0xFF;
    }
    
    sha1_update(ctx, pad, padding + 8);

    // Output digest
    for (int i = 0; i < 5; i++) {
        digest[i * 4] = (ctx->state[i] >> 24) & 0xFF;
        digest[i * 4 + 1] = (ctx->state[i] >> 16) & 0xFF;
        digest[i * 4 + 2] = (ctx->state[i] >> 8) & 0xFF;
        digest[i * 4 + 3] = ctx->state[i] & 0xFF;
    }
}

// Public API
void sha1_hash(const void *data, size_t len, gyatt_hash_t *hash) {
    sha1_ctx_t ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, data, len);
    sha1_final(&ctx, hash->hash);
}

void sha1_hash_file(const char *path, gyatt_hash_t *hash) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        memset(hash->hash, 0, HASH_SIZE);
        return;
    }

    sha1_ctx_t ctx;
    sha1_init(&ctx);

    uint8_t buffer[8192];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha1_update(&ctx, buffer, bytes_read);
    }

    fclose(f);
    sha1_final(&ctx, hash->hash);
}

void hash_to_hex(const gyatt_hash_t *hash, char *hex) {
    static const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < HASH_SIZE; i++) {
        hex[i * 2] = hex_chars[hash->hash[i] >> 4];
        hex[i * 2 + 1] = hex_chars[hash->hash[i] & 0x0F];
    }
    hex[HASH_HEX_SIZE - 1] = '\0';
}

void hex_to_hash(const char *hex, gyatt_hash_t *hash) {
    for (int i = 0; i < HASH_SIZE; i++) {
        char c1 = hex[i * 2];
        char c2 = hex[i * 2 + 1];
        
        uint8_t n1 = (c1 >= '0' && c1 <= '9') ? (c1 - '0') : (c1 - 'a' + 10);
        uint8_t n2 = (c2 >= '0' && c2 <= '9') ? (c2 - '0') : (c2 - 'a' + 10);
        
        hash->hash[i] = (n1 << 4) | n2;
    }
}

int hash_compare(const gyatt_hash_t *h1, const gyatt_hash_t *h2) {
    return memcmp(h1->hash, h2->hash, HASH_SIZE);
}

void hash_copy(gyatt_hash_t *dest, const gyatt_hash_t *src) {
    memcpy(dest->hash, src->hash, HASH_SIZE);
}
