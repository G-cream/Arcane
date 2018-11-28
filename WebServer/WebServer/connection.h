#ifndef _CONN_
#define _CONN_

#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include "util.h"
#ifdef _BSD_
#include <sys/event.h>
#include <sys/types.h>
#else
#include <sys/epoll.h>
#endif

#define READ_BUFFER_SIZE 2048
#define WRITE_BUFFER_SIZE 1024

struct httpconnection {
	int pollfd;
	int socktfd;
	struct sockaddr_storage address;
	char readbuffer[READ_BUFFER_SIZE];
	int readindex;
	char writebuffer[WRITE_BUFFER_SIZE];
	int writeindex;
	bool linger;
	struct iovec iv[2];
	int ivcount;
};

bool init_httpconnection(struct httpconnection *, int, struct sockaddr *);
bool read_httpconnection(struct httpconnection *);
bool write_httpconnection(struct httpconnection *);
bool process(struct httpconnection *);
void close_httpconnection(struct httpconnection *);

#endif // !_CONN_