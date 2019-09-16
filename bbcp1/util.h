#ifndef _UTIL_
#define _UTIL_

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool get_file_ioblksize(const char *, blksize_t *);
bool get_file_mode(const char *, mode_t *);
bool is_valid_dir(const char *);
bool is_valid_reg(const char *);
bool file_exists(const char *);
bool file_same(const char *, const char *);

#endif // !_UTIL_