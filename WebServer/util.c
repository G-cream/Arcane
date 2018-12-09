#include "util.h"

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
	int port = atoi(portnumber);
	if (port > 65535)
		return false;
	return true;
}

bool
is_valid_dir(const char *path)
{	
	if (path == NULL)
		return false;
	char resolvedpath[MAX_PATH_SIZE];
	if (realpath(path, resolvedpath) == NULL)
		return false;
	struct stat st;
	return stat(resolvedpath, &st) == 0 && S_ISDIR(st.st_mode);
}

bool
is_file_accessible(const char *path)
{
	if (path == NULL)
		return false;
	char resolvedpath[MAX_PATH_SIZE];
	if (realpath(path, resolvedpath) == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) < 0)
		return false;
	if (st.st_mode & S_IROTH && 
		access(resolvedpath, R_OK) == 0)
		return true;
	return false;
}

int
get_mime(const char* path, char *type)
{
	const char *mime;
	magic_t magic;		
	magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, NULL);
	mime = magic_file(magic, path);
	if (mime == NULL)
		mime = "";
	(void)strcpy(type, mime);
	magic_close(magic);	
	return strlen(type);
}

int
get_file_stat(const char *path)
{
	if (path == NULL)
		return -1;
	char resolvedpath[MAX_PATH_SIZE];
	if (realpath(path, resolvedpath) == NULL)
		return false;
	struct stat st;
	if (stat(resolvedpath, &st) < 0)
		return 0;	
	if (S_ISDIR(st.st_mode))
		return 1;
	if (S_ISREG(st.st_mode))
		return 2;
	return -1;
}

bool 
get_date_rfc822(char *date, const struct tm *time)
{
	if (date == NULL || time == NULL)
		return false;
	if (strftime(date, MAX_DATE_SIZE, "%a, %d %b %Y %T GMT", time) == 0)
		return false;
	return true;
}
	
bool 
get_date_rfc850(char *date, const struct tm *time)
{
	if (date == NULL || time == NULL)
		return false;
	if (strftime(date, MAX_DATE_SIZE, "%A, %d-%b-%y %T GMT", time) == 0)
		return false;
	return true;
}

bool 
get_date_asctime(char *date, const struct tm *time)
{
	if (date == NULL || time == NULL)
		return false;
	if (strftime(date, MAX_DATE_SIZE, "%c", time) == 0)
		return false;
	return true;	
}

void 
get_server_date(char *date)
{
	time_t timep;
	(void)time(&timep);
	(void)get_date_rfc822(date, gmtime(&timep));
}

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

bool 
contain_fd(int fd, const int *fdlist, int length)
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

bool
generate_dirhtml(const char *path, char *dirhtml)
{ 
	if (path == NULL || !is_valid_dir(path))
		return false;
	int dircount;
	char lmdate[MAX_DATE_SIZE], resolvedpath[MAX_PATH_SIZE];
	struct dirent **dirlist;
	struct stat st;	
	if (realpath(path, resolvedpath) == NULL)
		return false;
	dircount = scandir(resolvedpath, &dirlist, 0, alphasort);
	BEGIN_DIRHTML
		for(int n = 0 ; n != dircount ; ++n) {
		if (dirlist[n]->d_name[0] != '.') {
			(void)lstat(resolvedpath, &st);
			(void)strftime(lmdate, MAX_DATE_SIZE, "%F", localtime(&st.st_mtime));
			INSERT_LINE(dirlist[n]->d_name, lmdate);
		}
	}
	END_DIRHTML
	(void)strcpy(dirhtml, DIRHTML);
	return true;
}

int
map_file(char *abspath, void **mapaddr)
{
	if (abspath == NULL)
		return -1;
	struct stat st;
	(void)stat(abspath, &st);
	int fd = open(abspath, O_RDONLY);
	*mapaddr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	return st.st_size;
}

bool 
unmap_file(void *address, int length, bool needfree)
{
	if (address == NULL)
		return false;
	if (length <= 0)
		return false;
	if (needfree)
		free(address);
	else
		(void)munmap(address, length);
		return true;
}

int 
ishex(int x)
{
	return (x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}
 
//refers wiki rosettacode
int 
decode(const char *s, char *dec)
{
	char *o;
	const char *end = s + strlen(s);
	int c;
 
	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+') 
			c = ' ';
		if (c == '?') {
			setenv("QUERY_STRING", s, 1);
			c = '\0';
		}
		else if (c == '%' && (!ishex(*s++)	||
					!ishex(*s++)	||
					!sscanf(s - 2, "%2x", &c)))
			return -1;
 
		if (dec) *o = c;
	}
 
	return o - dec;
}

struct sem LOGLOCK;

bool 
init_log()
{
	if (!init_sem(&LOGLOCK))
		return false;
	if (!post_sem(&LOGLOCK))
		return false;
	return true;
}

void 
logfile(const char* remoteip, const char* requesttime, 
	const char *statusline, int statuscode, int contentlength)
{
	if (CONFIG.logdir == NULL)
		return;
	(void)wait_sem(&LOGLOCK);
	if (CONFIG.debugmode) {
		(void)fprintf(stdout,
			"%s %s %s %d %d\n", 
			remoteip,
			requesttime,
			statusline,
			statuscode,
			contentlength);
		fflush(stdout);
	}
	else {
		FILE *f;
		if ((f = fopen(CONFIG.logdir, "a+")) == NULL)
			return;
		(void)fprintf(f,
			"%s %s %s %d %d\n", 
			remoteip,
			requesttime,
			statusline,
			statuscode,
			contentlength);
		(void)fclose(f);
	}
	(void)post_sem(&LOGLOCK);
}

void
destroy_log()
{
	destroy_sem(&LOGLOCK);
}

void
add_sig(int sig, void(handler)(int), bool restart)
{
	struct sigaction sa;
	(void)memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart)
		sa.sa_flags |= SA_RESTART;
	(void)sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}

#ifdef _BSD_

void 
add_fd(int kqueuefd, int fd, bool oneshot)
{
	struct kevent event;
	int flag = EV_ADD | EV_CLEAR | EV_EOF;
	if (oneshot)
		flag |= EV_ONESHOT;
	EV_SET(&event, fd, EVFILT_READ, flag, 0, 0, NULL);
	kevent(kqueuefd, &event, 1, NULL, 0, NULL);
	set_nonblocking(fd);
}

void 
remove_fd(int kqueuefd, int fd)
{
	struct kevent event;
	EV_SET(&event, fd, EV_DELETE, 0, 0, 0, NULL);
	kevent(kqueuefd, &event, 1, NULL, 0, NULL);
	close(fd);
}

void 
mod_fd(int kqueuefd, int fd, int ev)
{
	struct kevent event;
	int flag = EV_ADD | EV_CLEAR | EV_EOF | EV_ONESHOT;
	EV_SET(&event, fd, ev, flag, 0, 0, NULL);
	kevent(kqueuefd, &event, 1, NULL, 0, NULL);
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
	if (one_shot) {
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
