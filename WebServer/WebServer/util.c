#include "util.h"
//#define _BSD_
bool 
is_valid_ipv4(const char *ipv4)
{
	struct in_addr addr;
	if (ipv4 == NULL)
		return false;
	if (inet_pton(AF_INET, ipv4, (void *)&addr) == 1)
		return true;
	return false;
}

bool 
is_valid_ipv6(const char *ipv6)
{
	struct in6_addr addr6;
	if (ipv6 == NULL)
		return false;
	if (inet_pton(AF_INET6, ipv6, (void *)&addr6) == 1)
		return true;
	return false;
}

bool
is_valid_portnumber(const char *portnumber)
{	
	if (portnumber == NULL)
		return false;
	
	if (strspn(portnumber, "0123456789") != strlen(portnumber))
		return false;
//	int port = atoi(portnumber);
//	if (port <= 1024)
//		return false;
	return true;
}

/*TODO:Everytime, read a new maxsock,
 *what if others shrink the maxsock when socketnumber is
 *already that big. What should the system do?
 */
bool
get_max_socketnumber(uintmax_t *maxsocketnumber)
{
	if (maxsocketnumber == NULL)
		return false;
	
	struct rlimit limit;
	if (getrlimit(RLIMIT_NOFILE, &limit) != 0)
		return false;
	*maxsocketnumber = limit.rlim_cur;
	return true;
}

int
set_nonblocking(int fd)
{
	if (fd == -1)
		return -1;
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

bool contain_fd(int fd, int *fdlist, int length)
{
	if (fd == -1)
		return false;
	if (fdlist == NULL || length == 0)
		return false;
	for (int n = 0; n != length; ++n) {
		if (fd == fdlist[n])
			return true;
	}
	return false;
}

/*
 *Omit the null lines ahead of request-line.
 *Trim all LWS and change them into SP.
 */
int trim_request(char *newbuffer, int newbuffersize, char *oldbuffer, int oldbuffersize)
{
	trimstate state;
	int replacestartindex, replaceendindex, newbufferindex;
	int n;
	bool omitflag;
	state = OSTATE;
	replacestartindex = 0;
	replaceendindex = 0;
	newbufferindex = 0;
	n = 0;
	omitflag = false;
	for (int checkindex = 0; checkindex != oldbuffersize; ++checkindex) {
		if (!omitflag) {
			if (oldbuffer[checkindex] != ' ' && oldbuffer[checkindex] != '\t' && oldbuffer[checkindex] != '\r' && oldbuffer[checkindex] != '\n')
				omitflag = true;
		}
		if (omitflag) {
			switch (oldbuffer[checkindex]) {
			case '\r':
				if (state == LWSSTATE) {
					if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
						newbuffer[newbufferindex] = ' ';
						++newbufferindex;
					}
				}
				else {
					for (n = 0; n != replaceendindex - replacestartindex; ++n)
						newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
					newbufferindex += n;
				}
				replacestartindex = checkindex;
				replaceendindex = checkindex + 1;
				state = RSTATE;
				break;
			case '\n':
				if (state == RSTATE) {
					++replaceendindex;
					state = NSTATE;
					break;
				}
				else {
					if (state == LWSSTATE) {
						if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
							newbuffer[newbufferindex] = ' ';
							++newbufferindex;
						}
					}
					else {
						for (n = 0; n != replaceendindex - replacestartindex; ++n)
							newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
						newbufferindex += n;
					}
					newbuffer[newbufferindex] = oldbuffer[checkindex];
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = OSTATE;
					break;
				}
			case '\t':
				if (state == OSTATE) {
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = HTSTATE;
					break;
				}
				if (state == RSTATE) {
					newbuffer[newbufferindex] = '\r';
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = HTSTATE;
					break;
				}
				if (state == SPSTATE || state == HTSTATE || state == NSTATE || state == LWSSTATE) {
					++replaceendindex;
					state = LWSSTATE;
					break;
				}
				break;
			case ' ':
				if (state == OSTATE) {
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = SPSTATE;
					break;
				}
				if (state == RSTATE) {
					newbuffer[newbufferindex] = '\r';
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = SPSTATE;
					break;
				}
				if (state == SPSTATE || state == HTSTATE || state == NSTATE || state == LWSSTATE) {
					++replaceendindex;
					state = LWSSTATE;
					break;
				}
				break;
			default:
				if (state == LWSSTATE) {
					if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
						newbuffer[newbufferindex] = ' ';
						++newbufferindex;
					}
				}
				else {
					for (n = 0; n != replaceendindex - replacestartindex; ++n)
						newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
					newbufferindex += n;
				}
				newbuffer[newbufferindex] = oldbuffer[checkindex];
				++newbufferindex;
				replacestartindex = 0;
				replaceendindex = 0;
				state = OSTATE;
				break;
			}
			if (newbufferindex == newbuffersize)
				return newbufferindex;
		}
	}
	return newbufferindex;
}

#ifdef _BSD_

void 
addfd(int kqueuefd, int fd, bool one_shot)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (one_shot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(kqueuefd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void 
removefd(int kqueuefd, int fd)
{
	epoll_ctl(kqueuefd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

void 
modfd(int kqueuefd, int fd, int ev)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(kqueuefd, EPOLL_CTL_MOD, fd, &event);
}

int
safe_realloc(void *ptr, size_t number, size_t size)
{
	return reallocarr(ptr, number, size);
}

#else

void 
add_fd(int epollfd, int fd, bool one_shot)
{
	if (epollfd == -1 || fd == -1)
		return;
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if (one_shot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	set_nonblocking(fd);
}

void 
remove_fd(int epollfd, int fd)
{
	if (epollfd == -1 || fd == -1)
		return;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
}

void 
mod_fd(int epollfd, int fd, int ev)
{
	if (epollfd == -1 || fd == -1)
		return;
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

/*
 *NetBSD: reallocarr.c
 */
int
safe_realloc(void *ptr, size_t number, size_t size)
{
	int saved_errno, result;
	void *optr;
	void *nptr;
	saved_errno = errno;
	memcpy(&optr, ptr, sizeof(ptr));
	if (number == 0 || size == 0) {
		free(optr);
		nptr = NULL;
		memcpy(ptr, &nptr, sizeof(ptr));
		errno = saved_errno;
		return 0;
	}
	if (number > SIZE_MAX / size) {
		errno = saved_errno;
		return EOVERFLOW;
	}
	nptr = realloc(optr, number * size);
	if (nptr == NULL) {
		result = errno;
	}
	else {
		result = 0;
		memcpy(ptr, &nptr, sizeof(ptr));
	}
	errno = saved_errno;
	return result;
}

#endif // _BSD_