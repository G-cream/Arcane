#include "server.h"

void
set_server_ipaddress(struct simpleserver *server, char *ipaddress)
{
	if (server == NULL)
		return;
	if (ipaddress == NULL)
		return;
	if (is_valid_ipv4(ipaddress) || is_valid_ipv6(ipaddress)) {
		server->ipaddress = realloc(server->ipaddress, strlen(ipaddress) + 1);
		if (server->ipaddress == NULL)
			return;
		(void)strcpy(server->ipaddress, ipaddress);
	}
}

void
set_server_portnumber(struct simpleserver *server, char *portnumber)
{
	if (server == NULL)
		return;
	if (portnumber == NULL)
		return;
	if (is_valid_portnumber(portnumber)) {
		server->portnumber = realloc(server->portnumber, strlen(portnumber) + 1);
		if (server->portnumber == NULL)
			return;
		(void)strcpy(server->portnumber, portnumber);
	}
}

bool
insert_server_listentable(struct simpleserver *server, int listenfd)
{
	if (server == NULL)
		return false;
	uintmax_t maxsocketnum;
	if (!get_max_socketnumber(&maxsocketnum))
		return false;
	assert(listenfd <= maxsocketnum);
	if (server->lcount < maxsocketnum) {
		server->ltable[server->lcount] = listenfd;
		++server->lcount;		
	}
	return true;
}

bool 
remove_server_listentable(struct simpleserver *server, int listenfd)
{
	if (server == NULL)
		return false;
	close(server->ltable[listenfd]);
	server->ltable[listenfd] = 0;
	--server->lcount;
	return true;
}

bool
insert_server_connectiontable(struct simpleserver *server, int connfd, struct sockaddr *address)
{
	if (server == NULL)
		return false;
	uintmax_t maxsocketnum;
	if (!get_max_socketnumber(&maxsocketnum))
		return false;
	assert(connfd <= maxsocketnum);
	if (server->ccount < maxsocketnum) {
		++server->ccount;
		init_httpconnection(&server->ctable[connfd], connfd, address);
	}
	return true;
}

bool 
remove_server_connectiontable(struct simpleserver *server, int connfd)
{
	if (server == NULL)
		return false;
	close_httpconnection(&server->ctable[connfd]);
	--server->ccount;
	return true;
}

void
init_simpleserver(struct simpleserver * server, char *ipaddress, char *portnumber, bool debugmode)
{
	if (server == NULL)
		return;	
	(void)memset(server, 0, sizeof(struct simpleserver));
	set_server_ipaddress(server, ipaddress);
	set_server_portnumber(server, portnumber);
	server->debugmode = debugmode;
	uintmax_t maxsocketnum;
	if (!get_max_socketnumber(&maxsocketnum))
		return;
	assert(maxsocketnum * sizeof(int) <= INT_MAX);
	server->ltable = (int *)malloc(maxsocketnum * sizeof(int));
	if (server->ltable == NULL)
		return;
	(void)memset(server->ltable, 0, maxsocketnum * sizeof(int));
	assert(maxsocketnum * sizeof(struct httpconnection) <= INT_MAX);
	server->ctable = (struct httpconnection *)malloc(maxsocketnum * sizeof(struct httpconnection));
	if (server->ctable == NULL)
		return;
	(void)memset(server->ctable, 0, maxsocketnum * sizeof(struct httpconnection));
}

int
get_tcpaddrs(struct simpleserver *server, struct addrinfo **reslist) {
	if (server == NULL)
		return -1;
	struct addrinfo hints;	
	(void)memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	return getaddrinfo(server->ipaddress, server->portnumber, &hints, reslist);
}

int
listen_connections(struct simpleserver *server, struct addrinfo *reslist) {
	if (server == NULL)
		return -1;
	struct addrinfo *p;
	for (p = reslist; p != NULL; p = p->ai_next) {
#ifdef DEBUG
		char addrdst[20];
		if (p->ai_family == AF_INET) {
			inet_ntop(p->ai_family, &((struct sockaddr_in *)p->ai_addr)->sin_addr, addrdst, 20);
			printf("IP: %s\n", addrdst);
			printf("Port: %u\n", ntohs(((struct sockaddr_in *)p->ai_addr)->sin_port));
		}
		if (p->ai_family == AF_INET6) {
			inet_ntop(p->ai_family, &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr, addrdst, 20);
			printf("IP: %s\n", addrdst);
			printf("Port: %u\n", ntohs(((struct sockaddr_in6 *)p->ai_addr)->sin6_port));
		}	  
#endif // DEBUG
		int listenfd, optval, backlog;
		optval = 1;
		backlog = server->debugmode ? 1 : MAX_LISTEN;
		
		listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listenfd < 0)
			continue;
		/*TODO:need to be tested*/
		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
			continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) != 0) {
			close(listenfd);
			continue;
		}
		if (listen(listenfd, backlog) < 0) {
			close(listenfd);
			continue;
		}
		insert_server_listentable(server, listenfd);
	}
	return 0;
}

int
accept_connections(struct simpleserver *server)
{
	if (server == NULL)
		return -1;
	uintmax_t maxsocketnum;
	if (!get_max_socketnumber(&maxsocketnum))
		return -2;
#ifdef _BSD_
	int kq = kqueue();
#else
	struct epoll_event events[maxsocketnum];
	int epollfd = epoll_create(5);
	for (int n = 0; n != server->lcount; ++n)
		add_fd(epollfd, server->ltable[n], false);
	for (;;) {
		int number = epoll_wait(epollfd, events, maxsocketnum, -1);
		if ((number < 0) && (errno != EINTR))
			//TODO: the errno should be logged
			break;
		for (int n = 0; n != number; ++n) {
			int sockfd = events[n].data.fd;
			if (contain_fd(sockfd, server->ltable, server->lcount)) {
				struct sockaddr_storage client_address;
				socklen_t client_addrlength = sizeof(struct sockaddr_storage);
				int connfd = accept(sockfd, (struct sockaddr*)&client_address, &client_addrlength);
				if (connfd < 0)
					continue;
				if (server->ccount == maxsocketnum)
					continue;
				insert_server_connectiontable(server, connfd, (struct sockaddr*)&client_address);
				add_fd(epollfd, connfd, true);
			}
			else if (events[n].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				remove_server_connectiontable(server, sockfd);
				remove_fd(epollfd, sockfd);
			}
			else if (events[n].events & EPOLLIN) {
				if (read_httpconnection(&server->ctable[sockfd]))
					//					pool->append(users + sockfd);
										continue;
				else {
					remove_server_connectiontable(server, sockfd);
					remove_fd(epollfd, sockfd);
				}
			}
			else if (events[n].events & EPOLLOUT) {
				if (!write_httpconnection(&server->ctable[sockfd])) {
					remove_server_connectiontable(server, sockfd);
					remove_fd(epollfd, sockfd);
				}
			}
			else
				continue;
		}
	}
#endif // _BSD_
	
		return 0;
}

int
setup_server(struct simpleserver *server) {
	if (server == NULL)
		return -1;
		
	struct addrinfo *reslist;
	if (get_tcpaddrs(server, &reslist) != 0) 
		return -2;
	if (listen_connections(server, reslist) != 0) 
		return -3;
	if (accept_connections(server) != 0) 
		return -4;	
	if (!server->debugmode) {
		if (daemon(1, 1) == -1)
			return -5;
	}
	freeaddrinfo(reslist);
	return 0;
}

int 
processrequest(struct simpleserver *server, int fd)
{
	char ch;
	int nread;
	ioctl(fd, FIONREAD, &nread);
	if (nread != 0)
		read(fd, &ch, 1); 
	return 0;
}

int
close_server(struct simpleserver *server)
{
	free(server->ltable);
	free(server->ctable);
	return 0;
}