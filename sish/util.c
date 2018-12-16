#include "util.h"

void sig_handler(int sig)
{
	int save_errno = errno;
	errno = save_errno;
}

void addsig(int sig)
{
	struct sigaction sa;
	(void)memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = sig_handler;
	sa.sa_flags |= SA_RESTART;
	(void)sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}