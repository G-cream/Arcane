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