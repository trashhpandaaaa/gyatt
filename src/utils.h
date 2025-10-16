#ifndef UTILS_H
#define UTILS_H

#include "gyatt.h"
#include <stdbool.h>

// File system utilities
int mkdir_recursive(const char *path);
int file_exists(const char *path);
int dir_exists(const char *path);
char *read_file(const char *path, size_t *size);
int write_file(const char *path, const void *data, size_t size);

// String utilities
char *str_concat(const char *s1, const char *s2);
char *str_duplicate(const char *str);
void str_trim(char *str);

// Path utilities
char *path_join(const char *p1, const char *p2);
char *get_current_dir(void);
char *find_repo_root(void);
int is_gyatt_repo(void);
char *get_gyatt_dir(void);

#endif // UTILS_H
