#ifndef _REQ_
#define _REQ_

#include <stdbool.h>
#include "pattern.h"

#define MAX_LINE_SIZE 1024

typedef enum trimstate
{
	OSTATE   = 0,
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

typedef enum checkstate { 
	CHECK_STATE_REQUESTLINE = 0, 
	CHECK_STATE_HEADER, 
	CHECK_STATE_CONTENT 
}checkstate;

typedef enum httpcode {
	NO_REQUEST = 0, 
	GET_REQUEST, 
	BAD_REQUEST, 
	NO_RESOURCE, 
	FORBIDDEN_REQUEST, 
	FILE_REQUEST, 
	INTERNAL_ERROR, 
	CLOSED_CONNECTION 
}httpcode;

typedef enum linestatus { 
	LINE_OK   = 0, 
	LINE_BAD, 
	LINE_OPEN 
}linestatus;

struct httprequest {
	char *linebuffer[1024];
	int bufferindex;
};

linestatus readline(struct httprequest *, char *, int);
void parse_requestline();
void parse_requestheader();

#endif // !_REQ_
