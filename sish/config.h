#ifndef _CONFIG_
#define _CONFIG_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_SIZE 255
#define MAX_PROCESS_NUM 16


struct config {
	bool cflag;
	bool xflag;
	char command[MAX_COMMAND_SIZE];
	int exitcode;
};

extern struct config CONFIG;

bool set_command(char *);

#endif // !_CONFIG_