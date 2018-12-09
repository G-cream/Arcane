#include "response.h"

const char *REASON_MAP[] = {	"200", "OK",
								"201", "Created",
								"202", "Accepted",
								"204", "No Content",
								"301", "Moved Permanently",
								"302", "Moved Temporarily",
								"304", "Not Modified",
								"400", "Bad Request",
								"401", "Unauthorized",
								"403", "Forbidden",
								"404", "Not Found",
								"500", "Internal Server Error",
								"501", "Not Implemented",
								"502", "Bad Gateway",
								"503", "Service Unavailable"
						   };

const char *
get_reason_phrase(int statuscode)
{
	if (statuscode <= 0)
		return NULL;
	int length = sizeof(REASON_MAP);
	for (int n = 0; n != length; n += 2) {
		if (statuscode == atoi(REASON_MAP[n]))
			return REASON_MAP[n + 1];
	}
	return NULL;
}

bool 
init_httpresponse(struct httpresponse *resp)
{
	(void)memset(resp->responsebuffer, 0, sizeof(resp->responselength));
	resp->responselength = 0;
	resp->statuscode = 0;
	return true;
}

bool 
add_response(struct httpresponse *resp, const char* format, ...)
{
	if (resp == NULL || format == NULL)
		return false;
	if (resp->responselength == MAX_RESPONSE_SIZE)
		return false;
	va_list arglist;
	va_start(arglist, format);
	int len;
	len = vsnprintf(resp->responsebuffer + resp->responselength,
		MAX_RESPONSE_SIZE - resp->responselength - 1,
		format,
		arglist);
	if (len >= (MAX_RESPONSE_SIZE - resp->responselength - 1))
		return false;
	resp->responselength += len;
	va_end(arglist);
	return true;
}

bool 
add_status_line(struct httpresponse *resp, int status, const char* title)
{
	return add_response(resp, "%s %d %s\r\n", CONFIG.protocolver, status, title);
}

bool
add_response_date(struct httpresponse *resp)
{
	char serverdate[MAX_DATE_SIZE];
	get_server_date(serverdate);
	return add_response(resp, "Date: %s\r\n", serverdate);
}

bool
add_server(struct httpresponse *resp)
{
	return add_response(resp, "Server: %s\r\n", CONFIG.servername);
}

bool
add_last_modified(struct httpresponse *resp, struct httprequest *req)
{
	char filelmdate[MAX_DATE_SIZE];
	if (req->isentitygen)
		get_server_date(filelmdate);
	else {
		struct stat st;
		(void)stat(req->abspath, &st);	
		(void)get_date_rfc822(filelmdate, gmtime(&st.st_mtime));
	}
	return add_response(resp, "Last-Modified: %s\r\n", filelmdate);
}

bool
add_content_type(struct httpresponse *resp, struct httprequest *req)
{
	if (req->isentitygen)
		return add_response(resp, "Content-Type: %s\r\n", "text/html");
	else {
		char contenttype[MAX_TYPE_SIZE];
		if(get_mime(req->abspath, contenttype) == 0)
			(void)strcpy(contenttype, "unknown");
		return add_response(resp, "Content-Type: %s\r\n", contenttype);
	}
}

bool
add_content_length(struct httpresponse *resp, struct httprequest *req)
{
	return add_response(resp, "Content-Length: %d\r\n", req->entitybodylength);
}

bool
add_null_line(struct httpresponse *resp)
{
	return add_response(resp, "%s", "\r\n");
}

bool
add_general_headers(struct httpresponse *resp, struct httprequest *req)
{
	(void)add_response_date(resp);	
	return true;
}

bool
add_response_headers(struct httpresponse *resp, struct httprequest *req)
{
	(void)add_server(resp);
	return true;
}

bool 
add_entity_headers(struct httpresponse *resp, struct httprequest *req)
{
	(void)add_last_modified(resp, req);
	(void)add_content_type(resp, req);
	(void)add_content_length(resp, req);
	return true;
}

bool
add_content(struct httpresponse *resp, const char* content)
{
	return add_response(resp, "%s", content);
}

void 
process_response(struct httpresponse *resp, struct httprequest *req)
{
	switch (req->requeststate) {
	case INTERNAL_ERROR:
		resp->statuscode = 500;
		(void)add_status_line(resp, 500, get_reason_phrase(500));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_null_line(resp);
		break;
	case BAD_REQUEST:
		resp->statuscode = 400;
		(void)add_status_line(resp, 400, get_reason_phrase(500));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_null_line(resp);
		break;
	case NO_RESOURCE:
		resp->statuscode = 404;
		(void)add_status_line(resp, 404, get_reason_phrase(404));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_null_line(resp);
		break;
	case FORBIDDEN_REQUEST:
		resp->statuscode = 403;
		(void)add_status_line(resp, 403, get_reason_phrase(403));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_null_line(resp);
		break;
	case NOT_CHANGED:
		resp->statuscode = 304;
		(void)add_status_line(resp, 304, get_reason_phrase(304));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_null_line(resp);
		break;
	case HEAD_REQUEST:
		resp->statuscode = 200;
		(void)add_status_line(resp, 200, get_reason_phrase(200));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_null_line(resp);
	case FILE_REQUEST:
		resp->statuscode = 200;
		(void)add_status_line(resp, 200, get_reason_phrase(200));
		(void)add_general_headers(resp, req);
		(void)add_response_headers(resp, req);
		(void)add_entity_headers(resp, req);
		(void)add_null_line(resp);
	default:
		return;
		/*NOTREACHED*/
	}
}
