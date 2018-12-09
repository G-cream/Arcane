#ifndef _RESPONSE_
#define _RESPONSE_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "request.h"

struct httpresponse {
	char responsebuffer[MAX_RESPONSE_SIZE];
	int responselength;
	int statuscode;
};

bool init_httpresponse(struct httpresponse *);
void process_response(struct httpresponse *, struct httprequest *);
bool add_response(struct httpresponse *, const char * , ...);
bool add_status_line(struct httpresponse *, int , const char *);
bool add_response_date(struct httpresponse *);
bool add_server(struct httpresponse *);
bool add_last_modified(struct httpresponse *, struct httprequest *);
bool add_content_type(struct httpresponse *, struct httprequest *);
bool add_content_length(struct httpresponse *, struct httprequest *);
bool add_null_line(struct httpresponse *);
bool add_general_headers(struct httpresponse *, struct httprequest *);
bool add_response_headers(struct httpresponse *, struct httprequest *);
bool add_entity_headers(struct httpresponse *, struct httprequest *);
bool add_content(struct httpresponse *, const char *);
const char * get_reason_phrase(int);
#endif // !_RESPONSE_
