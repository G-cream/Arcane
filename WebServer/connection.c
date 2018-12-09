<<<<<<< HEAD:WebServer/WebServer/connection.c
#include "connection.h"

bool 
init_httpconnection(struct httpconnection *connection, int pollfd, int connfd, struct sockaddr *address)
{
	if (connection == NULL)
		return false;
	(void)memset(connection, 0, sizeof(struct httpconnection));	
	struct timeval ti; 
	ti.tv_sec = 5;
	ti.tv_usec = 0;
	setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, &ti, sizeof(ti));
	setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &ti, sizeof(ti)); 
	connection->pollfd = pollfd;
	connection->socktfd = connfd;
	if (address->sa_family == AF_INET)
		(void)memcpy(&connection->address,
			(struct sockaddr_in *)address,
			sizeof(struct sockaddr_in));
	else if (address->sa_family == AF_INET6)
		(void)memcpy(&connection->address,
			(struct sockaddr_in6 *)address,
			sizeof(struct sockaddr_in6));
	else
		return false;
	return true;
}

bool 
read_httpconnection(struct httpconnection *connection)
{
	char *buffer = connection->readbuffer;
	int index = connection->readindex;
	if (index >= MAX_MESSAGE_SIZE)
		return false;
	int bytesreceived;
	for (;;)
	{
		bytesreceived = recv(connection->socktfd, &buffer[index], MAX_MESSAGE_SIZE - index, 0);
		if (bytesreceived == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			return false;
		}
		if (bytesreceived == 0)
			return false;
		connection->readindex += bytesreceived;
	}
	return true;
}

bool 
write_httpconnection(struct httpconnection *connection)
{
	int byteswrited;
	int bytesreserved;
	bytesreserved = connection->iv[0].iov_len + connection->iv[1].iov_len;
	for (;;) {
		byteswrited = writev(connection->socktfd, connection->iv, connection->ivcount);
		if (byteswrited <= -1) {
			if (errno == EAGAIN) {				
#ifdef _BSD_
				mod_fd(connection->pollfd, connection->socktfd, EVFILT_WRITE);
#else				mod_fd(connection->pollfd, connection->socktfd, EPOLLOUT);
#endif // _BSD_
				return true;
			}
			return false;
		}
		bytesreserved -= byteswrited;
		if (bytesreserved == 0)
			break;
	}
	return true;
}

bool 
process(struct httpconnection *connection)
{
	struct httprequest request;
	if (!init_httprequest(&request))
		return false;
	process_request(&request, connection->readbuffer, connection->readindex);
	//The request has the right format but not ends with a null line.
	if (request.requeststate == NO_REQUEST) {
#ifdef _BSD_
		mod_fd(connection->pollfd, connection->socktfd, EVFILT_READ);
#else		mod_fd(connection->pollfd, connection->socktfd, EPOLLIN);
#endif // _BSD_
		return true;
	}	
	struct httpresponse response;
	if (!init_httpresponse(&response))
		return false;
	process_response(&response, &request);
	(void)strcpy(connection->writebuffer, response.responsebuffer);
	connection->writeindex = response.responselength;	
	connection->iv[0].iov_base = connection->writebuffer;
	connection->iv[0].iov_len = connection->writeindex;
	connection->ivcount = 1;
	if (request.requeststate == FILE_REQUEST) {
		connection->iv[1].iov_base = request.entitybody;
		connection->iv[1].iov_len = request.entitybodylength;
		++connection->ivcount;
		connection->isentitygen = request.isentitygen;
	}
	log_message(connection, &request, &response);
#ifdef _BSD_
	mod_fd(connection->pollfd, connection->socktfd, EVFILT_WRITE);
#else	mod_fd(connection->pollfd, connection->socktfd, EPOLLOUT);
#endif // _BSD_	

	return true;
}

void
close_httpconnection(struct httpconnection *connection)
{
	if (connection->ivcount == 2)
		unmap_file(connection->iv[1].iov_base, connection->iv[1].iov_len, connection->isentitygen);
	close(connection->socktfd);
}

void 
log_message(struct httpconnection *connection, 
	struct httprequest *request, 
	struct httpresponse *response)
{
	char currentdate[MAX_DATE_SIZE];
	get_server_date(currentdate);
	char ipaddress[MAX_URI_SIZE];
	if (connection->address.ss_family == AF_INET) {
		struct sockaddr_in *tempaddrv4;
		tempaddrv4 = (struct sockaddr_in *)(&connection->address);
		inet_ntop(tempaddrv4->sin_family, &tempaddrv4->sin_addr, ipaddress, MAX_URI_SIZE);
	}
	else if (connection->address.ss_family == AF_INET6) {
		struct sockaddr_in6 *tempaddrv6;
		tempaddrv6 = (struct sockaddr_in6 *)(&connection->address);
		inet_ntop(tempaddrv6->sin6_family, &tempaddrv6->sin6_addr, ipaddress, MAX_URI_SIZE);
	}
	else
		return;
	char requestline[MAX_LINE_SIZE];
	(void)strcpy(requestline, request->requestline);
	requestline[strlen(requestline) - 2] = '\0';
	logfile(ipaddress,
		currentdate,
		requestline,
		response->statuscode,
		request->entitybodylength);
}
=======
#include "connection.h"

bool 
init_httpconnection(struct httpconnection *connection, int pollfd, int connfd, struct sockaddr *address)
{
	if (connection == NULL)
		return false;
	(void)memset(connection, 0, sizeof(struct httpconnection));	
	struct timeval ti; 
	ti.tv_sec = 5;
	ti.tv_usec = 0;
	setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, &ti, sizeof(ti));
	setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &ti, sizeof(ti)); 
	connection->pollfd = pollfd;
	connection->socktfd = connfd;
	if (address->sa_family == AF_INET)
		(void)memcpy(&connection->address,
			(struct sockaddr_in *)address,
			sizeof(struct sockaddr_in));
	else if (address->sa_family == AF_INET6)
		(void)memcpy(&connection->address,
			(struct sockaddr_in6 *)address,
			sizeof(struct sockaddr_in6));
	else
		return false;
	return true;
}

bool 
read_httpconnection(struct httpconnection *connection)
{
	char *buffer = connection->readbuffer;
	int index = connection->readindex;
	if (index >= MAX_MESSAGE_SIZE)
		return false;
	int bytesreceived;
	for (;;)
	{
		bytesreceived = recv(connection->socktfd, &buffer[index], MAX_MESSAGE_SIZE - index, 0);
		if (bytesreceived == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			return false;
		}
		if (bytesreceived == 0)
			return false;
		connection->readindex += bytesreceived;
	}
	return true;
}

bool 
write_httpconnection(struct httpconnection *connection)
{
	int byteswrited;
	int bytesreserved;
	bytesreserved = connection->iv[0].iov_len + connection->iv[1].iov_len;
	for (;;) {
		byteswrited = writev(connection->socktfd, connection->iv, connection->ivcount);
		if (byteswrited <= -1) {
			if (errno == EAGAIN) {				
#ifdef _BSD_
				mod_fd(connection->pollfd, connection->socktfd, EVFILT_WRITE);
#else
				mod_fd(connection->pollfd, connection->socktfd, EPOLLOUT);
#endif // _BSD_
				return true;
			}
			return false;
		}
		bytesreserved -= byteswrited;
		if (bytesreserved == 0)
			break;
	}
	return true;
}

bool 
process(struct httpconnection *connection)
{
	struct httprequest request;
	if (!init_httprequest(&request))
		return false;
	process_request(&request, connection->readbuffer, connection->readindex);
	//The request has the right format but not ends with a null line.
	if (request.requeststate == NO_REQUEST) {
#ifdef _BSD_
		mod_fd(connection->pollfd, connection->socktfd, EVFILT_READ);
#else
		mod_fd(connection->pollfd, connection->socktfd, EPOLLIN);
#endif // _BSD_
		return true;
	}	
	struct httpresponse response;
	if (!init_httpresponse(&response))
		return false;
	process_response(&response, &request);
	(void)strcpy(connection->writebuffer, response.responsebuffer);
	connection->writeindex = response.responselength;	
	connection->iv[0].iov_base = connection->writebuffer;
	connection->iv[0].iov_len = connection->writeindex;
	connection->ivcount = 1;
	if (request.requeststate == FILE_REQUEST) {
		connection->iv[1].iov_base = request.entitybody;
		connection->iv[1].iov_len = request.entitybodylength;
		++connection->ivcount;
		connection->isentitygen = request.isentitygen;
	}
	log_message(connection, &request, &response);
#ifdef _BSD_
	mod_fd(connection->pollfd, connection->socktfd, EVFILT_WRITE);
#else
	mod_fd(connection->pollfd, connection->socktfd, EPOLLOUT);
#endif // _BSD_	

	return true;
}

void
close_httpconnection(struct httpconnection *connection)
{
	if (connection->ivcount == 2)
		unmap_file(connection->iv[1].iov_base, connection->iv[1].iov_len, connection->isentitygen);
	close(connection->socktfd);
}

void 
log_message(struct httpconnection *connection, 
	struct httprequest *request, 
	struct httpresponse *response)
{
	char currentdate[MAX_DATE_SIZE];
	get_server_date(currentdate);
	char ipaddress[MAX_URI_SIZE];
	if (connection->address.ss_family == AF_INET) {
		struct sockaddr_in *tempaddrv4;
		tempaddrv4 = (struct sockaddr_in *)(&connection->address);
		inet_ntop(tempaddrv4->sin_family, &tempaddrv4->sin_addr, ipaddress, MAX_URI_SIZE);
	}
	else if (connection->address.ss_family == AF_INET6) {
		struct sockaddr_in6 *tempaddrv6;
		tempaddrv6 = (struct sockaddr_in6 *)(&connection->address);
		inet_ntop(tempaddrv6->sin6_family, &tempaddrv6->sin6_addr, ipaddress, MAX_URI_SIZE);
	}
	else
		return;
	char requestline[MAX_LINE_SIZE];
	(void)strcpy(requestline, request->requestline);
	requestline[strlen(requestline) - 2] = '\0';
	logfile(ipaddress,
		currentdate,
		requestline,
		response->statuscode,
		request->entitybodylength);
}
>>>>>>> 0c9f42d21608910dd0e87c8fc7c8356a3c6eb91c:WebServer/connection.c
