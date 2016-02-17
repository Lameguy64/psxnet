#ifndef _PSXNET_H
#define _PSXNET_H


#define NET_ERR_OK		0	// No errors
#define NET_ERR_INIT	-1	// Init error
#define NET_ERR_CMD		-2	// Unknown command
#define NET_ERR_CONNECT	-3	// Connect fail


#define NET_TYPE_TCP	0
#define NET_TYPE_UDP	1

#define NET_CRC32_REMAINDER		0xFFFFFFFF

// Initializes serial and tests communication with the client device
int net_Init(int baud);

// Get client information
int net_GetClientInfo(char* infobuff, int infolen);

int net_Connect(char* ipaddr, int port, int type);

int net_ConnectHost(char* hostName, int port, int connectType, char* ip);

int net_Disconnect(int socket);

int net_Send(int socket, void* buff, int bytes);

int net_Receive(int socket, void* buff, int bytes);

unsigned int net_crc32(void* buff, int bytes, unsigned int crc);

#endif
