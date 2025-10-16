#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdint.h>

// Dynamic buffer for efficient string/data building
typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} buffer_t;

// Buffer operations
buffer_t *buffer_create(size_t initial_capacity);
void buffer_free(buffer_t *buf);
void buffer_clear(buffer_t *buf);

// Append operations
void buffer_append(buffer_t *buf, const void *data, size_t len);
void buffer_append_str(buffer_t *buf, const char *str);
void buffer_append_char(buffer_t *buf, char c);
void buffer_append_uint(buffer_t *buf, uint64_t num);
void buffer_append_int(buffer_t *buf, int64_t num);

// Access
const char *buffer_cstr(buffer_t *buf);
void *buffer_detach(buffer_t *buf, size_t *len);

#endif // BUFFER_H
