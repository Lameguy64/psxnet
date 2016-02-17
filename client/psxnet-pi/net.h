/*
 *	Very simple BSD socket helper module (simplifies connects)
 */

#ifndef _NET_H
#define _NET_H

#define NET_TYPE_TCP	0
#define NET_TYPE_UDP	1

#define NET_ERR_OK		0
#define NET_ERR_CONNECT	-1
#define NET_ERR_DNS		-2

extern char* net_ErrorText;

int net_Connect(const char* ipAddress, int port, int type);
int net_ConnectByHostName(const char* hostname, const char* protocol, int port, int type, char *ip);

void net_Disconnect(int socket);

#endif
