#ifndef _JOB_
#define _JOB_

#include "config.h"
#include "process.h"

struct job {
	struct process *subprocess;
	size_t subprocessnum;
	bool isbg;
};

bool add_process(struct process);
//bool cat_processes();
bool exe_processes();
void start_pipeline();
void run_pipeline();

#endif // !_JOB_