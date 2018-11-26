#ifndef _UTIL_
#define _UTIL_

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#ifdef _BSD_
#include <sys/event.h>
#include <sys/types.h>
#else
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
bool contain_fd(int, int *, int);
void add_fd(int, int , bool);
void remove_fd(int, int);
void mod_fd(int, int, int);
//void log(int fd, );
#endif // !_UTIL_

