#include "sws.h"

bool
init_sws(struct swsstate *stat)
{
	if (stat == NULL)
		return false;
	(void)memset(stat, 0, sizeof(struct swsstate));
	return true;
}

bool
set_sws_ipaddress(struct swsstate *stat, char *ipaddress)
{
	if (stat == NULL)
		return false;
	if (ipaddress == NULL)
		return false;
	if (is_valid_ipv4(ipaddress) || is_valid_ipv6(ipaddress)) {
		if (safe_realloc(&stat->ipaddress, 1, strlen(ipaddress) + 1) != 0)
			return false;
		(void)strcpy(stat->ipaddress, ipaddress);
		return true;
	}
	return false;
		
}

bool
set_sws_portnumber(struct swsstate *stat, char *portnumber)
{
	if (stat == NULL)
		return false;
	if (portnumber == NULL)
		return false;
	if (is_valid_portnumber(portnumber)) {
		if (safe_realloc(&stat->portnumber, 1, strlen(portnumber) + 1) != 0)
			return false;
		(void)strcpy(stat->portnumber, portnumber);
		return true;
	}
	return false;
}

bool
set_sws_cgidir(struct swsstate *stat, char *dir)
{
	if (stat == NULL)
		return false;
	if (dir == NULL)
		return false;
	if (is_valid_dir(dir)) {
		if (safe_realloc(&stat->cgidir, 1, strlen(dir) + 1) != 0)
			return false;
		(void)strcpy(stat->cgidir, dir);
		return true;
	}
	return false;
}

bool
set_sws_rootdir(struct swsstate *stat, char *dir)
{
	if (stat == NULL)
		return false;
	if (dir == NULL)
		return false;
	if (is_valid_dir(dir)) {
		if (safe_realloc(&stat->rootdir, 1, strlen(dir) + 1) != 0)
			return false;
		(void)strcpy(stat->rootdir, dir);
		return true;
	}
	return false;
}

bool
set_sws_logdir(struct swsstate *stat, char *dir)
{
	if (stat == NULL)
		return false;
	if (dir == NULL)
		return false;
	if (is_valid_dir(dir)) {
		if (safe_realloc(&stat->logdir, 1, strlen(dir) + 1) != 0)
			return false;
		(void)strcpy(stat->logdir, dir);
		return true;
	}
	return false;
}

int
process_sws(const struct swsstate *stat)
{
	if (stat == NULL)
		return -1;	
	bool cgiflag, debugmode;
	char *ipaddress, *portnumber, *cgidir, *rootdir, *logdir;	
	ipaddress = NULL;
	portnumber = NULL;
	cgidir = NULL;
	rootdir = NULL;
	logdir = NULL;
	if (stat->cgiflag) {
		cgiflag = stat->cgiflag;
		cgidir = stat->cgidir;
	 }
	if (stat->debuggingflag)
		debugmode = stat->debuggingflag;
	if (stat->ipflag)
		ipaddress = stat->ipaddress;
	if (stat->portflag)
		portnumber = stat->portnumber;
	if (stat->usageflag) {
		usage(STDOUT_FILENO);
		return 0;
	}
	if (stat->logflag)
		logdir = stat->logdir;
	rootdir = stat->rootdir;
	struct simpleserver server;
	if (!init_simpleserver(&server, ipaddress, portnumber, debugmode, cgiflag, cgidir, rootdir, logdir)) {
		close_server(&server);
		return -1;
	}
	if (setup_server(&server) != 0) {
		close_server(&server);
		return -1;
	}
	return 0;
}

void 
usage(int fd)
{
	(void)dprintf(fd, "usage: sws [ -dh ] [ -c dir ] [ -i address ] [ -l file ] [ -p port ] dir\n");
}

void 
free_sws(struct swsstate *stat)
{
	free(stat->cgidir);
	free(stat->rootdir);
	free(stat->ipaddress);
	free(stat->portnumber);
	free(stat->logdir);
}