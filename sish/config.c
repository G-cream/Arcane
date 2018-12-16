#include "config.h"

struct config CONFIG;

bool 
set_command(char *command)
{
	if (command == NULL)
		return false;
	if (strlen(command) + 1 > MAX_COMMAND_SIZE)
		return false;
	(void)strcpy(CONFIG.command, command);
	return true;
}