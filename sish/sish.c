#include "sish.h"

int run_command(char *command)
{
	struct parser commandparser;
	if (!parse_command(&commandparser, command))
		return -1;
}