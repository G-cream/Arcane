#include "connection.h"

bool 
init_httpconnection(struct httpconnection *connection, int pollfd, int connfd, struct sockaddr *address)
{
	if (connection == NULL)
		return false;
	(void)memset(connection, 0, sizeof(struct httpconnection));
	connection->pollfd = pollfd;
	connection->socktfd = connfd;
	switch (address->sa_family) {
	case AF_INET:
		memcpy(&connection->address,
			(struct sockaddr_in *)address,
			sizeof(struct sockaddr_in));
		break;
	case AF_INET6:
		memcpy(&connection->address,
			(struct sockaddr_in6 *)address,
			sizeof(struct sockaddr_in6));
		break;
	case AF_UNSPEC:
	default:
		return false;
	}
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
		printf("%d, %d\n", bytesreceived, errno);
		if (bytesreceived == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			return false;
		}
		if (bytesreceived == 0)
			return false;
		connection->readindex += bytesreceived;
	}
#ifdef DEBUG
	printf("receive: %s", buffer);
#endif // DEBUG

	return true;
}

void 
unmap(struct httpconnection *connection)
{
	if (connection->iv[1].iov_base == NULL)
		return;
	(void)munmap(connection->iv[1].iov_base, connection->iv[1].iov_len);
	connection->iv[1].iov_base = NULL;
	connection->iv[1].iov_len = 0;
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
				mod_fd(connection->pollfd, connection->socktfd, EPOLLOUT);
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
		mod_fd(connection->pollfd, connection->socktfd, EPOLLIN);
		return true;
	}	
	struct httpresponse response;
	if (!init_httpresponse(&response))
		return false;
	process_response(&response, &request);
	connection->iv[0].iov_base = response.responsebuffer;
	connection->iv[0].iov_len = response.responselength;
	connection->ivcount = 1;
	if (request.method == GET) {
		if (strlen(request.cgifilebuffer) != 0)
			connection->iv[1].iov_base = request.cgifilebuffer;
		else if (strlen(request.dirhtmlbuffer) != 0)
			connection->iv[1].iov_base = request.cgifilebuffer;
		else
			connection->iv[1].iov_base = request.entitybody;
		connection->iv[1].iov_len = request.entitybodylength;
		++connection->ivcount;
	}
	mod_fd(connection->pollfd, connection->socktfd, EPOLLOUT);
	char currentdate[MAX_DATE_SIZE];
	get_server_date(currentdate);
	char firstline[MAX_LINE_SIZE];
	for (int n = 0; n != request.msgbufferlength; ++n) {
		if (request.msgbuffer[n] == '\n' &&
		request.msgbuffer[n - 1] == '\r') {
			firstline[n - 1] = '\0';
			break;
		}
		firstline[n] = request.msgbuffer[n];			
	}
	char ipaddress[MAX_URI_SIZE];
	if (connection->address.ss_family == AF_INET) {
		struct sockaddr_in *tempaddrv4;
		tempaddrv4 = (struct sockaddr_in *)(&connection->address);
		inet_ntop(tempaddrv4->sin_family, &tempaddrv4->sin_addr, ipaddress, MAX_URI_SIZE);
	}
	if (connection->address.ss_family == AF_INET6) {
		struct sockaddr_in6 *tempaddrv6;
		tempaddrv6 = (struct sockaddr_in6 *)(&connection->address);
		inet_ntop(tempaddrv6->sin6_family, &tempaddrv6->sin6_addr, ipaddress, MAX_URI_SIZE);
	}
	logfile(ipaddress,
		currentdate,
		firstline,
		response.statuscode,
		request.entitybodylength);
	return true;
}

void
close_httpconnection(struct httpconnection *connection)
{
	close(connection->socktfd);
	connection->socktfd = 0;
}