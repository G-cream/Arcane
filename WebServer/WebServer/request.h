#ifndef _REQ_
#define _REQ_

#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "config.h"
#include "util.h"

typedef enum trimstate
{
	OSTATE = 0,
	SPSTATE,
	HTSTATE,
	RSTATE,
	NSTATE,
	LWSSTATE
}trimstate;

typedef enum httpmethod { 
	GET  = 0,	
	HEAD
}httpmethod;

typedef enum httpver { 
	HTTP1d0  = 0,	
	HTTP1d1
}httpver;

typedef enum linestate { 
	LINE_OK   = 0, 
	LINE_BAD, 
	LINE_OPEN,
	LINE_INS 
}linestate;

typedef enum checkstate { 
	CHECK_REQUESTLINE = 0, 
	CHECK_HEADER, 
	CHECK_CONTENT 
}checkstate;

typedef enum httpcode {
	NO_REQUEST = 0, 
	IN_REQUEST,
	GET_REQUEST,
	HEAD_REQUEST,
	BAD_REQUEST, 
	NO_RESOURCE,
	NOT_CHANGED,
	FORBIDDEN_REQUEST, 
	FILE_REQUEST, 
	INTERNAL_ERROR, 
	CLOSED_CONNECTION 
}httpcode;

struct httprequest {
	char msgbuffer[MAX_MESSAGE_SIZE];
	int msgbufferlength;
	int linestartindex;
	int linecheckindex;
	char *entitybody;
	int entitybodylength;
	bool isentitygen;
	char dirhtmlbuffer[MAX_GENFILE_SIZE];
	char cgifilebuffer[MAX_GENFILE_SIZE];
	httpcode requeststate;
	httpmethod method;
	char uri[MAX_URI_SIZE];
	char abspath[MAX_PATH_SIZE];
	httpver version;
	char imsdate[MAX_DATE_SIZE];
	bool iscgi;
};

bool init_httprequest(struct httprequest *);
linestate readline(struct httprequest *, char *, int);
bool parse_requestline(struct httprequest *, char *);
bool parse_generalheader(struct httprequest *, char *);
bool parse_requestheader(struct httprequest *, char *);
bool parse_entityheader(struct httprequest *, char *);
bool parse_nullline(struct httprequest *, char *);
bool parse_messageheader(struct httprequest *, char *);
bool parse_requestpath(struct httprequest *);
bool check_modified(struct httprequest *);
bool get_entitybody(struct httprequest *);
bool do_getmethod(struct httprequest *);
bool do_headmethod(struct httprequest *);
bool do_request(struct httprequest *);
bool process_request(struct httprequest *, char *, int);
bool is_valid_requestpath(char *);
int parse_requestpath_username(char *, char *, int);
int trim_request(char *, int, char *, int);
void *mapfile(char *);

#endif // !_REQ_
