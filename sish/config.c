#include "config.h"

struct config CONFIG;

bool 
set_command(char *command)
{
	if (command == NULL)
		return false;
	if (strlen(command) + 1 > MAX_COMMANDTEXT_SIZE)
		return false;
	(void)strcpy(CONFIG.commandtext, command);
	return true;
}