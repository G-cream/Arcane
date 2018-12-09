#ifndef _SWS_
#define _SWS_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "config.h"
#include "server.h"
#include "util.h"

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
	char *rootdir;
};

bool set_sws_ipaddress(struct swsstate *, char *);
bool set_sws_portnumber(struct swsstate *, char *);
bool set_sws_cgidir(struct swsstate *, char *);
bool set_sws_rootdir(struct swsstate *, char *);
bool set_sws_logdir(struct swsstate *, char *);
bool init_sws(struct swsstate *);
int process_sws(const struct swsstate *);
void usage(int fd);
void free_sws(struct swsstate *);

#endif /* !_SWS */