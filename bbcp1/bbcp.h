#ifndef _BBCP_
#define _BBCP_

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define ARG_SUPPORTED 2

void cat_path_name(char *, char *, char *);
void usage(void);

#endif // !_BBCP_