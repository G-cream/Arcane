#ifndef _UTIL_
#define _UTIL_

#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
//#include <magic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "config.h"
#ifdef _BSD_
#include <sys/event.h>
#include <sys/types.h>
#else
#include <sys/epoll.h>
#endif

#define DOCTYPE					"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
#define BEGIN_HTML				"<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>"
#define HEAD_TITLE				"<head><title>G_cream</title></head>"
#define BEGIN_BODY				"<body><div><table border=\"1\"><tr><th>Name</th><th>Last Modified</th></tr>"
#define END_BODY				"</table></div></body>"
#define END_HTML				"</html>"
#define BEGIN_DIRHTML			char _dirhtml[MAX_GENFILE_SIZE];\
								char _buffer[MAX_GENFILE_SIZE];\
								(void)strcpy(_dirhtml, DOCTYPE);\
								(void)strcat(_dirhtml, BEGIN_HTML);\
								(void)strcat(_dirhtml, HEAD_TITLE);\
								(void)strcat(_dirhtml, BEGIN_BODY);
#define INSERT_LINE(name, lm)	(void)sprintf(_buffer, "<tr><td>%s</td><td>%s</td></tr>", name, lm);\
								(void)strcat(_dirhtml, _buffer);
#define END_DIRHTML				(void)strcat(_dirhtml, END_BODY);\
								(void)strcat(_dirhtml, END_HTML);
#define DIRHTML					_dirhtml

bool is_valid_ipv4(const char *);
bool is_valid_ipv6(const char *);
bool is_valid_portnumber(const char *);
bool is_valid_dir(const char *);
bool is_file_accessible(const char *);
char* get_mime(const char*);
int get_file_stat(const char *);
bool get_date_rfc822(char *, const struct tm *);
bool get_date_rfc850(char *, const struct tm *);
bool get_date_asctime(char *, const struct tm *);
void get_server_date(char *);
bool get_max_socketnumber(uintmax_t *);
int set_nonblocking(int);
bool contain_fd(int, const int *, int);
void add_fd(int, int , bool);
void remove_fd(int, int);
void mod_fd(int, int, int);
int safe_realloc(void *, size_t, size_t);
bool generate_dirhtml(const char *, char *);
int map_file(char *, void **);
int ishex(int);
int decode(const char *, char *);
void logfile(const char *, const char *, const char *, int, int);
#endif // !_UTIL_

