#ifndef _UTIL_
#define _UTIL_

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

void sig_handler(int);
void addsig(int);

#endif // !_UTIL_