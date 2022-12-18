#include "Socket.h"

const SOCKET Socket::open (SOCKET&fd, struct sockaddr_in &address)
{
	SOCKET	sock;
    int opt = 1;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {

		cout <<"[Socket::open] socket failed" << endl;
        return INVALID_SOCKET;
	}

	if (setsockopt(sock, SOL_SOCKET,
                   SO_REUSEADDR, (const char*) & opt,
                   sizeof(opt))) {
        cout << "setsockopt Failed" << endl;
        exit(EXIT_FAILURE);
    }

	if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cout << "bind failed" << endl;
        return INVALID_SOCKET;
    }

	fd = sock;

	return (fd);
}
void Socket::connect (SOCKET &fd, const char *ip, const uint16_t portNo)
{
    fd = INVALID_SOCKET;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error \n";
        return;
    }

	// connect to server
	sockaddr_in	hostAddr;

	hostAddr.sin_family = AF_INET;
	hostAddr.sin_port   = htons (portNo);
	hostAddr.sin_addr.s_addr = inet_addr(ip);

	int		retval = INVALID_SOCKET;

	while (((retval = ::connect (fd, (struct sockaddr *)&hostAddr, sizeof(sockaddr_in))) == INVALID_SOCKET) && (errno == EINTR)) ;

	if (retval == INVALID_SOCKET) {
		closesocket (fd);
		fd = INVALID_SOCKET;
		cout<< "[Socket::connect] connect failed" << endl;
	}
}
const int Socket::write(SOCKET socket, const void *buf, const uint32_t nbytes)
{
    char	*ptr = (char *)buf;
	int		nleft, nwritten;

	nleft = nbytes;

	while (nleft > 0) {
#if !defined(WIN32)
		if ((nwritten = ::write (socket, ptr, nleft)) < 0) 
#else
		if ((nwritten = ::send (socket, ptr, nleft, 0)) < 0) 
#endif
		{
			if (EINTR == errno) {
				continue;
			}
			else {
				return (-1);
			}
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	return (nbytes - nleft);
}
const int Socket::read (SOCKET socket, void *buf, const uint32_t nbytes)
{
    char	*ptr = (char *)buf;
	int		nleft, nread;

	nleft = nbytes;

	while (nleft > 0) {
#if !defined(WIN32)
		if ((nread = ::read (socket, ptr, nleft)) < 0) 
#else
		if ((nread = ::recv (socket, ptr, nleft, 0)) < 0) 
#endif
		{
			if (EINTR == errno) {
				continue;
			}
			else {					// Error
				return (-1);
			}
		}
		else if (0 == nread) {		// EOF
			break;
		}

		nleft -= nread;
		ptr   += nread;
	}

	return (nbytes - nleft);
}
int Socket::close(SOCKET fd)
{
	shutdown (fd, SD_BOTH);
	closesocket (fd);

	return (0);
}
void Socket::shutdown(SOCKET fd, int how)
{
    if (::shutdown(fd, how) < 0)
	{
		cout << "[Socket::shutdown] shutdown failed" << endl;
	}
}