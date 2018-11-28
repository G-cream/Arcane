#include "sws.h"

void
init_sws(struct swsstate *stat)
{
	if (stat == NULL)
		return;
	(void)memset(stat, 0, sizeof(struct swsstate));
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

int
process_sws(const struct swsstate *stat)
{
	/*TODO:The return value of this function need
	 *to be re-evaluated.
	 */
	if (stat == NULL)
		return -1;
	
	bool debugmode;
	char *ipaddress, *portnumber;	
	ipaddress = NULL;
	portnumber = DEFAULT_PORT;
	
	if (stat->debuggingflag) {
		debugmode = stat->debuggingflag;
	}
	if (stat->ipflag) {
		ipaddress = stat->ipaddress;
	}
	if (stat->portflag) {
		portnumber = stat->portnumber;
	}
	if (stat->usageflag) {
		usage(STDOUT_FILENO);
		return 0;
	}
	
	struct simpleserver server;
	init_simpleserver(&server, ipaddress, portnumber, debugmode);
	if (setup_server(&server) != 0) {
		return -1;
	}
	return 0;
}

void 
usage(int fd)
{
	(void)dprintf(fd, "usage: sws [ -dh ] [ -c dir ] [ -i address ] [ -l file ] [ -p port ] dir\n");
}