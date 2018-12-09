#ifndef _CONFIG_
#define _CONFIG_

#include <stdbool.h>

#define MAX_TYPE_SIZE 255
#define MAX_PATH_SIZE 255
#define MAX_DATE_SIZE 100
#define MAX_GENFILE_SIZE 4096
#define MAX_LINE_SIZE 1024
#define MAX_MESSAGE_SIZE 2048
#define MAX_URI_SIZE 255
#define MAX_VER_SIZE 10
#define MAX_RESPONSE_SIZE 1024
#define MAX_LISTEN 5
#define DEFAULT_PORT "8080"
#define DEFAULT_THREAD_NUM 5
#define DEFAULT_CONN_NUM 100
#define DEFAULT_LOG_NAME "swslog.txt"
#define DEFAULT_PROTOCOL_NAME "HTTP/1.0"
#define DEFAULT_SERVER_NAME "SWS 1.1"

struct serverconfig
{
	bool debugmode;
	char *ipaddress;
	char *portnumber;
	bool cgienabled;
	char *cgidir;
	char *rootdir;
	char *logdir;
	char *protocolver;
	char *servername;
};

extern struct serverconfig CONFIG;

#endif // !_CONFIG_
