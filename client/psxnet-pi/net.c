#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "net.h"

char*	net_ErrorText = NULL;

int net_Connect(const char* ip, int port, int type) {

	struct	sockaddr_in	destAddr;
	int		socketh;

	destAddr.sin_family			= AF_INET;
	destAddr.sin_port			= htons(port);
	destAddr.sin_addr.s_addr	= inet_addr(ip);

	if (type == NET_TYPE_TCP)
		socketh = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		socketh = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);

	if (socketh < 0)
		return(NET_ERR_CONNECT);

	if (connect(socketh, (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0)
		return(NET_ERR_CONNECT);

	return(socketh);

}

int net_ConnectByHostName(const char* hostname, const char* protocol, int port, int type, char *ip) {

	int sockResult;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in* h;

	memset(&hints, 0x00, sizeof(hints));
	hints.ai_family		= AF_UNSPEC;		// Use AF_INET6 to force IPv6
	hints.ai_socktype	= SOCK_STREAM;

	if (getaddrinfo(hostname, protocol, &hints, &servinfo) != 0)
		return(NET_ERR_DNS);

	// Loop through the IP addresses until a usable address is found
	for(p=servinfo; p != NULL; p = p->ai_next) {

		h = (struct sockaddr_in*)p->ai_addr;

		sockResult = net_Connect(inet_ntoa(h->sin_addr), port, type);

		if (sockResult >= 0) {

			if (ip != NULL)
				strcpy(ip, inet_ntoa(h->sin_addr));

			freeaddrinfo(servinfo);

			return(sockResult);

		}

	}

	freeaddrinfo(servinfo);

	return(NET_ERR_DNS);

}

void net_Disconnect(int socket) {

	close(socket);

}
