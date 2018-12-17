#ifndef _SISH_
#define _SISH_

#include <stdio.h>
#include <unistd.h>
#include "config.h"
#include "parser.h"
#include "util.h"

struct sish {
};

void set_env(char *);
bool expand(struct commandlist *);
bool execute_command(struct commandlist *);
int run_command();

#endif // !_SISH_