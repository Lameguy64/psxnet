#include <sys/types.h>
#include "psxnet.h"
#include "psxnet_cmd.h"
#include "serial.h"


static void _net_SendCommand(u_char code) {

	u_char	cmd[] = { 0x8a, code };

	ser_SendBytes(cmd, 2);

}

static int _net_GetResult() {

	u_char	result[4];
	int		r,c=0;

    while(c<4) {

        r = ser_ReadBytes(&result[c], 4);

        if (r <= 0)
			continue;

		c += r;

    }

	return(*((int*)result));
}

static void _net_crc32_init(unsigned int* table) {

	int i,j;
	unsigned int crcVal;

	for(i=0; i<256; i++) {

		crcVal = i;

		for(j=0; j<8; j++) {

			if (crcVal&0x00000001L)
				crcVal = (crcVal>>1)^0xEDB88320L;
			else
				crcVal = crcVal>>1;

		}

		table[i] = crcVal;

	}

}


unsigned int net_crc32(void* buff, int bytes, unsigned int crc) {

	int	i;
	unsigned char*	byteBuff = (unsigned char*)buff;
	unsigned int	byte;
	unsigned int	crcTable[256];

    _net_crc32_init(crcTable);

	for(i=0; i<bytes; i++) {

		byte = 0x000000ffL&(unsigned int)byteBuff[i];
		crc = (crc>>8)^crcTable[(crc^byte)&0xff];

	}

	return(crc^0xFFFFFFFF);

}

int net_Init(int baud) {

	int result=0;

	// Initialize serial
    ser_Init(baud);

	// Send a test command to the client
    _net_SendCommand(NET_CMD_TEST);

	// Get result
	result = _net_GetResult();

	// Check if result is correct
	if (result == 0xdeadbeef)
		return(NET_ERR_INIT);

	// Return
	return(NET_ERR_OK);

}

int net_GetClientInfo(char* infobuff, int infolen) {

	int	len=0;
	int	result=0xff;

	_net_SendCommand(NET_CMD_GETINFO);
    result = _net_GetResult();

	if (result != 0)
		return(NET_ERR_CMD);

	ser_ReadBytes(&len, 2);

	if (len > infolen) {

		short i;
		char dummy;

		ser_ReadBytes(infobuff, infolen);

		for(i=0; i<(len-infolen); i++)
			ser_ReadBytes(&dummy, 1);

	} else {

		ser_ReadBytes(infobuff, len);

	}

	return(NET_ERR_OK);

}

int net_Connect(char* ipaddr, int port, int type) {

	int		result=-1;
	short	len;

    _net_SendCommand(NET_CMD_CONNECT);
	result = _net_GetResult();

	if (result != 0)
		return(NET_ERR_CMD);

	ser_SendBytes(&port, sizeof(short));
	ser_SendBytes(&type, sizeof(short));

	len = strlen(ipaddr);
	ser_SendBytes(&len, 2);
	ser_SendBytes(ipaddr, len);

	while(ser_ReadBytes(&result, sizeof(int)) == 0);

	if (result < 0)
		return(NET_ERR_CONNECT);

	return(result);

}

int net_ConnectHost(char* hostName, int port, int connectType, char* ip) {

    int hostNameLen = strlen(hostName)+1;
	int socket,result;

	_net_SendCommand(NET_CMD_CONNECTHOST);
	result = _net_GetResult();

	ser_SendBytes(&hostNameLen, sizeof(short));
	ser_SendBytes(&connectType, sizeof(short));
	ser_SendBytes(&port, sizeof(short));

	ser_SendBytes(hostName, hostNameLen);

	// Wait until result is relayed back
	while(ser_ReadBytes(&result, sizeof(int)) == 0);

	if (result < 0)
		return(NET_ERR_CONNECT);

	socket = result;

	ser_SendBytes(&result, sizeof(int));
	ser_ReadBytes(ip, 18);

	return(socket);

}

int net_Disconnect(int socket) {

	int result=0xff;

	_net_SendCommand(NET_CMD_DISCONNECT);
	result = _net_GetResult();
	ser_SendBytes(&socket, 2);

	return(NET_ERR_OK);

}

int net_Send(int socket, void* buff, int bytes) {

	int result,crc32;

	_net_SendCommand(NET_CMD_SEND);
	result = _net_GetResult();

	crc32 = net_crc32(buff, bytes, NET_CRC32_REMAINDER);

	// Send socket, data length and CRC32
	ser_SendBytes(&socket, sizeof(short));
	ser_SendBytes(&bytes, sizeof(u_short));
	ser_SendBytes(&crc32, sizeof(int));

	// Wait for acknowledgment
	result = _net_GetResult();

	do {

		// Send data
		ser_SendBytes(buff, bytes);

		// Get result
		result = _net_GetResult();

		// Retry if CRC check failed
	} while (result != 0);

	// Get result from sending packet
	result = _net_GetResult();

	return(result);

}

int net_Receive(int socket, void* buff, int bytes) {

	int result;
	int rec=0,dlen;
	unsigned int crc32;

	_net_SendCommand(NET_CMD_RECEIVE);
	result = _net_GetResult();

	// Send socket and data length
	ser_SendBytes(&socket, sizeof(short));
	ser_SendBytes(&bytes, sizeof(u_short));

	// Get downloaded length
	dlen = _net_GetResult();

	// If failed
	if (dlen <= 0)
		return(-1);

	// Get CRC
	while(ser_ReadBytes(&crc32, sizeof(int)) <= 0);

	do {

		// Begin receiving stream
		while(rec < dlen) {

			rec += ser_ReadBytes(&buff[rec], dlen);

		}

		if (net_crc32(buff, dlen, NET_CRC32_REMAINDER) != crc32)
			result = 1;
		else
			result = 0;

		ser_SendBytes(&result, sizeof(int));

	} while(result != 0);

	return(dlen);

}
