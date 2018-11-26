#include "connection.h"

void 
init_httpconnection(struct httpconnection *connection, int connfd, struct sockaddr *address)
{
	if (connection == NULL)
		return;
	connection->socktfd = connfd;
	switch (address->sa_family) {
	case AF_INET:
		memcpy(&connection->address,
			(struct sockaddr_in *)address,
			sizeof(struct sockaddr_in));
	case AF_INET6:
		memcpy(&connection->address,
			(struct sockaddr_in6 *)address,
			sizeof(struct sockaddr_in6));
	case AF_UNSPEC:
	default:
		return;
	}
}

bool 
read_httpconnection(struct httpconnection *connection)
{
	char *buffer = connection->readbuffer;
	int index = connection->readindex;
	if (index >= READ_BUFFER_SIZE)
		return false;
	int bytesreceived;
	bytesreceived = 0;
	for (;;)
	{
		bytesreceived = recv(connection->socktfd, &buffer[index], READ_BUFFER_SIZE - index, 0);
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

//void unmap()
//{
//	if (m_file_address)
//	{
//		munmap(m_file_address, m_file_stat.st_size);
//		m_file_address = 0;
//	}
//}

bool 
write_httpconnection(struct httpconnection *connection)
{
	int number = 0;
	int sendedbytes = 0;
	int reservedbytes = connection->writeindex;
	int socktfd = connection->socktfd;
	int pollfd = connection->pollfd;
	bool linger = connection->linger;
	struct iovec *iv = connection->iv;
	int ivcount = connection->ivcount;
	if (reservedbytes == 0) {
		mod_fd(pollfd, socktfd, EPOLLIN);
//		init();
		return true;
	}
	for (;;) {
		number = writev(socktfd, iv, ivcount);
		if (number <= -1) {
			if (errno == EAGAIN) {
				mod_fd(pollfd, socktfd, EPOLLOUT);
				return true;
			}
//			unmap();
			return false;
		}
		reservedbytes -= number;
		sendedbytes += number;
		if (reservedbytes <= sendedbytes) {
//			unmap();
			if (linger) {
//				init();
				mod_fd(pollfd, socktfd, EPOLLIN);
				return true;
			}
			else {
				mod_fd(pollfd, socktfd, EPOLLIN);
				return false;
			} 
		}
	}
}

void
close_httpconnection(struct httpconnection *connection)
{
	close(connection->socktfd);
	connection->socktfd = 0;
}