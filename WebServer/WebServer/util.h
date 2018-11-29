#ifndef _UTIL_
#define _UTIL_

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/resource.h>
#ifdef _BSD_
#include <sys/event.h>
#include <sys/types.h>
#else
#include <stdlib.h>
#include <sys/epoll.h>
#endif

bool is_valid_ipv4(const char *);
bool is_valid_ipv6(const char *);
bool is_valid_portnumber(const char *);
/*TODO:cast rlim_t to uintmax_t might have some promblem here
 *maybe or maybe not
 */
bool get_max_socketnumber(uintmax_t *);
int set_nonblocking(int);
int trim_request(char *, int, char *, int);
bool contain_fd(int, int *, int);
void add_fd(int, int , bool);
void remove_fd(int, int);
void mod_fd(int, int, int);
int safe_realloc(void *, size_t, size_t);
//void log(int fd, );
#endif // !_UTIL_

