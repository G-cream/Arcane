#include "request.h"

bool
init_httprequest(struct httprequest *req)
{
	if (req == NULL)
		return false;
	(void)memset(req->msgbuffer, 0, sizeof(req->msgbuffer));
	(void)memset(req->uri, 0, sizeof(req->uri));
	(void)memset(req->dirhtmlbuffer, 0, sizeof(req->dirhtmlbuffer));
	(void)memset(req->cgifilebuffer, 0, sizeof(req->cgifilebuffer));
	(void)memset(req->abspath, 0, sizeof(req->abspath));
	(void)memset(req->imsdate, 0, sizeof(req->imsdate));
	req->msgbufferlength = 0;
	req->linestartindex = 0;
	req->linecheckindex = 0;
	req->entitybodylength = 0;
	req->isentitygen = false;
	req->requeststate = NO_REQUEST;
	req->method = GET;
	req->version = HTTP1d0;
	req->iscgi = false;
	return true;
}

/*
 *Omit the null lines ahead of request-line.
 *Trim all LWS and change them into SP.
 *Merge multiple SP and HT into one SP including changed SPs.
 */
int 
trim_request(char *newbuffer, int newbuffersize, char *oldbuffer, int oldbuffersize)
{
	if (newbuffer == NULL || newbuffersize < 0 || oldbuffer == NULL || oldbuffersize < 0)
		return -1;
	trimstate state;
	int replacestartindex, replaceendindex, newbufferindex;
	int n;
	bool omitflag;
	state = OSTATE;
	replacestartindex = 0;
	replaceendindex = 0;
	newbufferindex = 0;
	n = 0;
	omitflag = false;
	for (int checkindex = 0; checkindex != oldbuffersize; ++checkindex) {
		if (!omitflag) {
			if (oldbuffer[checkindex] != ' ' && oldbuffer[checkindex] != '\t' && 
				oldbuffer[checkindex] != '\r' && oldbuffer[checkindex] != '\n')
				omitflag = true;
		}
		if (omitflag) {
			switch (oldbuffer[checkindex]) {
			case '\r':
				if (state == LWSSTATE) {
					if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
						newbuffer[newbufferindex] = ' ';
						++newbufferindex;
					}
				}
				else {
					for (n = 0; n != replaceendindex - replacestartindex; ++n)
						newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
					newbufferindex += n;
				}
				replacestartindex = checkindex;
				replaceendindex = checkindex + 1;
				state = RSTATE;
				break;
			case '\n':
				if (state == RSTATE) {
					++replaceendindex;
					state = NSTATE;
					break;
				}
				else {
					if (state == LWSSTATE) {
						if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
							newbuffer[newbufferindex] = ' ';
							++newbufferindex;
						}
					}
					else {
						for (n = 0; n != replaceendindex - replacestartindex; ++n)
							newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
						newbufferindex += n;
					}
					newbuffer[newbufferindex] = oldbuffer[checkindex];
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = OSTATE;
					break;
				}
			case '\t':
				if (state == OSTATE) {
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = HTSTATE;
					break;
				}
				if (state == RSTATE) {
					newbuffer[newbufferindex] = '\r';
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = HTSTATE;
					break;
				}
				if (state == SPSTATE || state == HTSTATE || state == NSTATE || state == LWSSTATE) {
					++replaceendindex;
					state = LWSSTATE;
					break;
				}
				break;
			case ' ':
				if (state == OSTATE) {
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = SPSTATE;
					break;
				}
				if (state == RSTATE) {
					newbuffer[newbufferindex] = '\r';
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = SPSTATE;
					break;
				}
				if (state == SPSTATE || state == HTSTATE || state == NSTATE || state == LWSSTATE) {
					++replaceendindex;
					state = LWSSTATE;
					break;
				}
				break;
			default:
				if (state == LWSSTATE) {
					if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
						newbuffer[newbufferindex] = ' ';
						++newbufferindex;
					}
				}
				else {
					for (n = 0; n != replaceendindex - replacestartindex; ++n)
						newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
					newbufferindex += n;
				}
				newbuffer[newbufferindex] = oldbuffer[checkindex];
				++newbufferindex;
				replacestartindex = 0;
				replaceendindex = 0;
				state = OSTATE;
				break;
			}
			if (newbufferindex == newbuffersize)
				return newbufferindex;
		}
	}
	//flush the last buffer.
	if(state == LWSSTATE) {
		if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
			newbuffer[newbufferindex] = ' ';
			++newbufferindex;
		}
	}
	else {
		for (n = 0; n != replaceendindex - replacestartindex; ++n)
			newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
		newbufferindex += n;
	}
	return newbufferindex;
}

linestate 
readline(struct httprequest *req, char *linebuffer, int length)
{
	if (req == NULL || linebuffer == NULL || length < 0)
		return LINE_BAD;
	if (req->linestartindex == req->msgbufferlength)
		return LINE_OPEN;
	
	int linebufferindex;
	char appendedchar;
	bool findflag;
	linebufferindex = 0;
	findflag = false;
	while (req->linecheckindex < req->msgbufferlength) {
		if (linebufferindex == length)
			return LINE_INS;
		linebuffer[linebufferindex] = req->msgbuffer[req->linecheckindex];
		++linebufferindex;
		if (req->msgbuffer[req->linecheckindex] == '\n' &&
		req->msgbuffer[req->linecheckindex - 1] == '\r') {			
			linebuffer[linebufferindex] = '\0';
			findflag = true;
		}
		++req->linecheckindex;
		if (findflag) {
			req->linestartindex = req->linecheckindex;
			return LINE_OK;
		}
	}
	return LINE_BAD;
}

bool 
parse_requestline(struct httprequest *req, char *line)
{
	if (req == NULL || line == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	char *currentstring;
	char *saveptr;
	saveptr = NULL;
	currentstring = strtok_r(line, " \t\r\n", &saveptr);
	if (currentstring == NULL) {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	if (strcmp(currentstring, "GET") == 0)
		req->method = GET;
	else if (strcmp(currentstring, "HEAD") == 0)
		req->method = HEAD;
	else {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	
	currentstring = strtok_r(NULL, " \t\r\n", &saveptr);
	if (currentstring == NULL) {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	(void)strcpy(req->uri, currentstring);
	if (strlen(currentstring) == 8) {
		if (CONFIG.cgienabled && strncasecmp(currentstring, "/cgi-bin", 8) == 0)
			req->iscgi = true;
	}
	else {
		if (CONFIG.cgienabled && strncasecmp(currentstring, "/cgi-bin/", 9) == 0)
			req->iscgi = true;
	}
	
	currentstring = strtok_r(NULL, " \t\r\n", &saveptr);
	if (currentstring == NULL) {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	if (strcasecmp(currentstring, "HTTP/1.0") == 0)
		req->version = HTTP1d0;
	else {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	
	currentstring = strtok_r(NULL, " \t\r\n", &saveptr);
	if (currentstring != NULL) {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	req->requeststate = IN_REQUEST;
	return true;
}

bool
parse_generalheader(struct httprequest *req, char *line)
{
	if (req == NULL || line == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	/*
	 *No general header need to be processed in this
	 *project.
	 */
	return false;
}

bool
parse_requestheader(struct httprequest *req, char *line)
{
	if (req == NULL || line == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	char *currentstring;
	char *saveptr;
	if (strncasecmp(line, "If-Modified-Since: ", 19) == 0) {
		currentstring = line + 19;
		currentstring = strtok_r(currentstring, "\r\n", &saveptr);
		(void)strcpy(req->imsdate, currentstring);
		req->requeststate = IN_REQUEST;
		return true;
	}
	return false;
}

bool
parse_entityheader(struct httprequest *req, char *line)
{
	if (req == NULL || line == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	/*
	 *The header that cannot be recognized are treated as
	 *entity header.
	 */
	return true;
}

bool
parse_nullline(struct httprequest *req, char *line)
{
	if (req == NULL || line == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	if (strcmp(line, "\r\n") == 0) {
		switch (req->method) {
		case GET:
			req->requeststate = GET_REQUEST;
			break;
		case HEAD:
			req->requeststate = HEAD_REQUEST;
			break;
		default:
			req->requeststate = BAD_REQUEST;
			break;
		}
		return true;
	}
	return false;
}

bool 
parse_messageheader(struct httprequest *req, char *line)
{
	if (req == NULL || line == NULL)
		return false;
	if (parse_nullline(req, line))
		return true;
	if (parse_generalheader(req, line))
		return true;
	if (parse_requestheader(req, line))
		return true;
	if (parse_entityheader(req, line))
		return true;
	return false;
	/*NOTREACHED*/
}

bool 
is_valid_requestpath(char *path)
{
	if (path == NULL)
		return false;	
	char newpath[MAX_PATH_SIZE];
	int fstat, checkindex;
	char *dirpath;
	(void)strcpy(newpath, path);
	fstat = get_file_stat(newpath);
	if (fstat <= 0)
		return false;
	if (fstat == 2)
		for (checkindex = strlen(newpath); checkindex != -1; --checkindex)
			if (newpath[checkindex] == '/') {
				newpath[checkindex] = '\0';
				break;
			}
	dirpath = newpath;
	if (strstr(dirpath, "..") == NULL)
		return true;
	return false;
}

int 
parse_requestpath_username(char *path, char *username, int length)
{
	if (path == NULL || username == NULL)
		return -1;
	int checkindex;	
	if (strncmp(path, "~", 1) == 0 && strncmp(path, "~/", 2) != 0) {
		path = path + 1;
		for (checkindex = 0; checkindex != strlen(path); ++checkindex) {
			if (path[checkindex] == '/' || checkindex == length)
				break;
			username[checkindex] = path[checkindex];
		}
		if (checkindex == length)
			return -1;
		username[checkindex]  = '\0';
		return checkindex;
	}
	return 0;
}

bool
parse_requestpath(struct httprequest *req)
{
	if (req == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	char decodedpath[MAX_PATH_SIZE];
	char temppath[MAX_PATH_SIZE];
	char username[MAX_PATH_SIZE];
	int checkindex, predictedpathlength;
	checkindex = 0;
	decode(req->uri, decodedpath);
	if (req->iscgi) {
		checkindex += strlen("/cgi-bin");
		predictedpathlength = strlen(CONFIG.cgidir) + strlen(decodedpath) + 1 - checkindex;
		if (predictedpathlength > MAX_PATH_SIZE) {
			//status code: 414
			req->requeststate = BAD_REQUEST;
			return false;
		}
		(void)strcpy(temppath, CONFIG.cgidir);
		(void)strcat(temppath, "/");
		(void)strcat(temppath, &decodedpath[checkindex]);
	}
	else {
		int usernamelength;
		usernamelength = parse_requestpath_username(decodedpath, username, MAX_PATH_SIZE);
		if (usernamelength > 0) {
			checkindex += usernamelength + 1;
			predictedpathlength = strlen("/home/") + strlen("/sws/") + strlen(decodedpath);
			if (predictedpathlength > MAX_PATH_SIZE) {
				//status code: 414
				req->requeststate = BAD_REQUEST;
				return false;
			}
			(void)strcpy(temppath, "/home/");
			(void)strcat(temppath, username);
			(void)strcat(temppath, "/sws/");
			(void)strcat(temppath, &decodedpath[checkindex]);
		}
		else {
			predictedpathlength = strlen(CONFIG.rootdir) + strlen(decodedpath) + 2;
			if (predictedpathlength > MAX_PATH_SIZE) {
				//status code: 414
				req->requeststate = BAD_REQUEST;
				return false;
			}
			(void)strcpy(temppath, CONFIG.rootdir);
			(void)strcat(temppath, "/");
			(void)strcat(temppath, decodedpath);
		}
	}
	if (get_file_stat(temppath) <= 0) {
		req->requeststate = NO_RESOURCE;
		return false;
	}
	if (!is_file_accessible(temppath)) {
		req->requeststate = FORBIDDEN_REQUEST;
		return false;
	}
	if (!is_valid_requestpath(temppath)) {
		req->requeststate = FORBIDDEN_REQUEST;
		return false;
	}
	if (realpath(temppath, req->abspath) == NULL) {
		req->requeststate = BAD_REQUEST;
		return false;
	}
	return true;
}

bool
check_modified(struct httprequest *req)
{
	if (req == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	struct stat st;
	(void)stat(req->abspath, &st);
	char filedate[MAX_DATE_SIZE];
	(void)get_date_rfc822(filedate, gmtime(&st.st_mtime));
	if (strcmp(req->imsdate, filedate) == 0) {
		req->requeststate = NOT_CHANGED;
		return true;
	}
	(void)get_date_rfc850(filedate, gmtime(&st.st_mtime));
	if (strcmp(req->imsdate, filedate) == 0) {
		req->requeststate = NOT_CHANGED;
		return true;
	}
	(void)get_date_asctime(filedate, gmtime(&st.st_mtime));
	if (strcmp(req->imsdate, filedate) == 0) {
		req->requeststate = NOT_CHANGED;
		return true;
	}
	return true;
}

bool 
generate_cigbody(struct httprequest *req)
{
	if (req == NULL)
		return false;
	pid_t pid;
	int status;
	int pipefd[2];
	if (pipe(pipefd) == -1)
		return false;
	pid = fork();
	if (pid == -1)
		return false;
	if (pid == 0) {
		printf("!");
		if(dup2(pipefd[1], STDOUT_FILENO) == -1 && errno != EINTR)
			return false;
		close(pipefd[0]);
		if (execl(req->abspath, "", NULL) == -1)
			return false;
	}
	else {
		close(pipefd[1]);
		int readlength;
		int index;
		readlength = 0;
		index = 0;
		for (;;) {
			readlength = read(pipefd[0], req->cgifilebuffer + index, sizeof(req->cgifilebuffer));
			index += readlength;
			if (readlength == -1 && errno != EINTR)
				return false;
			if (readlength == 0)
				break;			
		}
		close(pipefd[0]);
		req->cgifilebuffer[index] = '\0';
		if (wait(&status) == -1 || WIFEXITED(status) == 0) 
			return false;
	}
	return true;
}

bool 
get_entitybody(struct httprequest *req)
{
	if (req == NULL) {
		req->requeststate = INTERNAL_ERROR;
		return false;
	}
	if (req->iscgi) {
		if (!generate_cigbody(req)) {
			req->requeststate = INTERNAL_ERROR;
			return false;
		}
		req->entitybodylength = strlen(req->cgifilebuffer);
		req->requeststate = FILE_REQUEST;
		return true;
	}
	//dir
	if(get_file_stat(req->abspath) == 1) {
		char localindexpath[MAX_PATH_SIZE];
		if (strlen(req->abspath) + strlen("/index.html") + 1 >= MAX_PATH_SIZE) {
			req->requeststate = INTERNAL_ERROR;
			return false;
		}
		strcpy(localindexpath, req->abspath);
		strcat(localindexpath, "/index.html");
		if (is_file_accessible(localindexpath)) {
			void *mapaddr;
			int filesize;
			filesize = map_file(localindexpath, &mapaddr);
			if (mapaddr == MAP_FAILED) { 
				req->requeststate = INTERNAL_ERROR;
				return false;
			}
			req->entitybody = (char *)mapaddr;
			req->entitybodylength = filesize;	
			req->requeststate = FILE_REQUEST;
		}
		else {
			if (!generate_dirhtml(req->abspath, req->dirhtmlbuffer)) {
				req->requeststate = INTERNAL_ERROR;
				return false;
			}
			req->entitybody = req->dirhtmlbuffer;
			req->entitybodylength = strlen(req->dirhtmlbuffer);
			req->requeststate = FILE_REQUEST;
			req->isentitygen = true;
			return true;
		}
	}
	//regular file
	if(get_file_stat(req->abspath) == 2) {
		void *mapaddr;
		int filesize;
		filesize = map_file(req->abspath, &mapaddr);
		if (mapaddr == MAP_FAILED) { 
			req->requeststate = INTERNAL_ERROR;
			return false;
		}
		req->entitybody = (char *)mapaddr;
		req->entitybodylength = filesize;	
		req->requeststate = FILE_REQUEST;
		return true;
	}
	req->requeststate = INTERNAL_ERROR;
	return false;
}

bool
do_getmethod(struct httprequest *req)
{
	if (req == NULL)
		return false;
	if (!parse_requestpath(req))
		return false;
	if (check_modified(req) && req->requeststate != NOT_CHANGED)
		if (!get_entitybody(req))
			return false;
	return true;
}

bool
do_headmethod(struct httprequest *req)
{
	if (req == NULL)
		return false;
	if (!parse_requestpath(req))
		return false;
	if (!check_modified(req))
		return false;
	return true;
}

bool
do_request(struct httprequest *req)
{
	if (req == NULL)
		return false;
	switch (req->method) {
	case GET:
		do_getmethod(req);
		break;
	case HEAD:
		do_headmethod(req);
		break;
	default:
		break;
	}
	return true;
}

bool
process_request(struct httprequest *req, char *message,  int length)
{
	if (req == NULL || message == NULL || length < 0)
		return false;
	assert(length <= MAX_MESSAGE_SIZE);
	req->msgbufferlength = trim_request(req->msgbuffer, MAX_MESSAGE_SIZE, message, length);
	if (req->msgbufferlength == -1) {
		req->requeststate = BAD_REQUEST;
		return true;
	}
	char linebuffer[MAX_LINE_SIZE];
	linestate ls = LINE_OK;
	checkstate cs = CHECK_REQUESTLINE;
	for(;;) {
		(void)memset(linebuffer, 0, MAX_LINE_SIZE);
		ls = readline(req, linebuffer, MAX_LINE_SIZE);
		if (ls == LINE_OK) {
			switch (cs) {
			case CHECK_REQUESTLINE: 
				(void)parse_requestline(req, linebuffer);
				if (req->requeststate == BAD_REQUEST)
					return true;
				if (req->requeststate == IN_REQUEST)
					cs = CHECK_HEADER;
				break;
			case CHECK_HEADER:
				(void)parse_messageheader(req, linebuffer);
				if (req->requeststate == BAD_REQUEST)
					return true;
				if (req->requeststate != IN_REQUEST) {
					(void)do_request(req);
					return true;
				}
				break;
			default:
				req->requeststate = INTERNAL_ERROR;
				return true;
			}
		}
		if (ls == LINE_OPEN) {
			req->requeststate = NO_REQUEST;
			return true;
		}
		if (ls == LINE_BAD) {
			req->requeststate = BAD_REQUEST;
			return true;
		}
	}
	return false;
}