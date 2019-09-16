#ifndef _BBCPM_
#define _BBCPM_

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "util.h"

#define ARG_SUPPORTED 2
#define MAX_CP_SIZE (1024 * 1024)

void cat_path_name(char *, char *, char *);
void usage(void);

#endif
