#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <errno.h> 
#include <sys/types.h>
#include <winsock2.h>
#include <winsock.h>

using namespace std;

// simple static class for socket;
class Socket
{
public:
    Socket(){}
    ~Socket(){}
    static const SOCKET open (SOCKET&fd, struct sockaddr_in &address);
    static void connect (SOCKET&fd, const char *ip, const uint16_t portNo);
    static const int write(SOCKET socket, const void *buf, const uint32_t size);
    static const int read (SOCKET socket, void *buf, const uint32_t size);
    static int close(SOCKET fd);
	static void shutdown(SOCKET fd, int how = SD_BOTH );
};