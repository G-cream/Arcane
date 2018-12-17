#ifndef _CONFIG_
#define _CONFIG_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMANDTEXT_SIZE 255
#define MAX_TOKEN_NUM 50
#define MAX_COMMAND_SIZE 15
#define MAX_PID_SIZE 15
#define MAX_ERRORNO_SIZE 15
#define PROMPT "sish$"


struct config {
	bool cflag;
	bool xflag;
	char commandtext[MAX_COMMANDTEXT_SIZE];
	int exitcode;
};

extern struct config CONFIG;

bool set_command(char *);

#endif // !_CONFIG_