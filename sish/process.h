#ifndef _PROCESS_
#define _PROCESS_

#include "config.h"

struct process {
	//int pipefd[2];
	pid_t pid;
	//argvs
	char *command;
	struct process *next;
};

bool execute();

#endif // !_PROCESS_