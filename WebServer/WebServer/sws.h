#ifndef _SWS_
#define _SWS_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"
#include "util.h"

#define DEFAULT_PORT "8080"

struct swsstate {
	bool cgiflag;
	bool debuggingflag;	
	bool ipflag;
	bool logflag;
	bool portflag;
	bool usageflag;
	char *cgidir;
	char *ipaddress;
	char *logdir;
	char *portnumber;
	char *docroot;
};

bool set_sws_ipaddress(struct swsstate *, char *);
bool set_sws_portnumber(struct swsstate *, char *);
bool init_sws(struct swsstate *);
int process_sws(const struct swsstate *);
void usage(int fd);

#endif /* !_SWS */