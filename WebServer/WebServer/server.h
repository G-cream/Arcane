#ifndef _SIMPLE_SERVER_
#define _SIMPLE_SERVER_

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>
#include "connection.h"
#include "util.h"
#ifdef _BSD_
#include <sys/event.h>
#include <sys/types.h>
#else
#include <sys/epoll.h>
#endif

#define MAX_LISTEN 5

struct simpleserver {
	bool debugmode;
	char *ipaddress;
	char *portnumber;
	int *ltable;
	int lcount;
	struct httpconnection *ctable;
	int ccount;
};

bool set_server_ipaddress(struct simpleserver *, char *);
bool set_server_portnumber(struct simpleserver *, char *);
bool insert_server_listentable(struct simpleserver *, int);
bool remove_server_listentable(struct simpleserver *, int);
bool insert_server_connectiontable(struct simpleserver *, int, struct sockaddr *);
bool remove_server_connectiontable(struct simpleserver *, int);
bool init_simpleserver(struct simpleserver *, char *, char *, bool);
int get_tcpaddrs(struct simpleserver *, struct addrinfo **);
int listen_connections(struct simpleserver *, struct addrinfo *);
int accept_connections(struct simpleserver *);
int setup_server(struct simpleserver *);
int processrequest(struct simpleserver *, int);
int close_server(struct simpleserver *);


#endif // !_SIMPLE_SERVER_
