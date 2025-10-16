#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_INITIAL_CAPACITY 256

buffer_t *buffer_create(size_t initial_capacity) {
    buffer_t *buf = malloc(sizeof(buffer_t));
    if (!buf) return NULL;

    if (initial_capacity == 0) {
        initial_capacity = BUFFER_INITIAL_CAPACITY;
    }

    buf->data = malloc(initial_capacity);
    if (!buf->data) {
        free(buf);
        return NULL;
    }

    buf->len = 0;
    buf->capacity = initial_capacity;
    buf->data[0] = '\0';

    return buf;
}

void buffer_free(buffer_t *buf) {
    if (!buf) return;
    free(buf->data);
    free(buf);
}

void buffer_clear(buffer_t *buf) {
    if (!buf) return;
    buf->len = 0;
    if (buf->data) {
        buf->data[0] = '\0';
    }
}

static void buffer_grow(buffer_t *buf, size_t min_capacity) {
    if (buf->capacity >= min_capacity) return;

    size_t new_capacity = buf->capacity;
    while (new_capacity < min_capacity) {
        new_capacity *= 2;
    }

    char *new_data = realloc(buf->data, new_capacity);
    if (!new_data) return;  // Keep old buffer on failure

    buf->data = new_data;
    buf->capacity = new_capacity;
}

void buffer_append(buffer_t *buf, const void *data, size_t len) {
    if (!buf || !data || len == 0) return;

    buffer_grow(buf, buf->len + len + 1);
    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
    buf->data[buf->len] = '\0';
}

void buffer_append_str(buffer_t *buf, const char *str) {
    if (!str) return;
    buffer_append(buf, str, strlen(str));
}

void buffer_append_char(buffer_t *buf, char c) {
    buffer_append(buf, &c, 1);
}

void buffer_append_uint(buffer_t *buf, uint64_t num) {
    char temp[32];
    snprintf(temp, sizeof(temp), "%llu", (unsigned long long)num);
    buffer_append_str(buf, temp);
}

void buffer_append_int(buffer_t *buf, int64_t num) {
    char temp[32];
    snprintf(temp, sizeof(temp), "%lld", (long long)num);
    buffer_append_str(buf, temp);
}

const char *buffer_cstr(buffer_t *buf) {
    if (!buf || !buf->data) return "";
    return buf->data;
}

void *buffer_detach(buffer_t *buf, size_t *len) {
    if (!buf) return NULL;
    
    void *data = buf->data;
    if (len) *len = buf->len;
    
    buf->data = NULL;
    buf->len = 0;
    buf->capacity = 0;
    
    return data;
}
