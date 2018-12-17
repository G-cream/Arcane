#include "sish.h"

bool
expand(struct commandlist *commands)
{
	size_t commandnum, tokennum, characternum;
	struct token *currenttoken;
	char pid[MAX_PID_SIZE], exitcode[MAX_ERRORNO_SIZE];
	commandnum = commands->commandnum;
	for (size_t i = 0; i != commandnum; ++i) {
		tokennum = commands->comlist[i].tokennum;
		for (size_t n = 0; n != tokennum; ++n) {
			currenttoken = &commands->comlist[i].tokens[n];			
			if (snprintf(pid, MAX_PID_SIZE, "%d", getpid()) <= 0)
				return false;
			if (snprintf(exitcode, MAX_ERRORNO_SIZE, "%d", CONFIG.exitcode) <= 0)
				return false;
			(void)strrpc(currenttoken->content, "$$", pid);
			(void)strrpc(currenttoken->content, "$?", exitcode);
		}
	}
	return true;
}

bool
execute_command(struct commandlist *commands)
{
	
}

int
run_command()
{
	struct parser commandparser;
	struct commandlist commands;
	(void)memset(&commands, 0, sizeof(struct commandlist));
	int retcode;
	retcode = parse_command(&commandparser, CONFIG.commandtext, &commands);	
	if (retcode != 0)
		return retcode;
	if (!expand(&commands))
		return -4;
	if (!execute_command(&commands))
		return -5;
	return 0;
}