#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "conemu.h"
#include "serial.h"
#include "net.h"


// Default device for serial comms
#define SERIAL_DEV	"/dev/ttyAMA0"

// Client software version
#define CLIENT_VER	"0.70b"

// Client info string (to be sent to the PSX)
#define CLIENT_INFO	"PSXNET TCP Client " CLIENT_VER " by Lameguy64\n" \
					"Client Device: Raspberry Pi Model B\n" \
					"Features: TCP\n"


#define CRC32_REMAINDER		0xFFFFFFFF
#define TRUE 1
#define FALSE 0


// Client commands
void CommsTest();
void SendClientInfo();
void Connect();
void ConnectHost();
void DisconnectSocket();
void SendBytes();
void ReadBytes();
// Command read and result send functions
unsigned int ReadCommand();
void SendResult(int result);
// CRC32 calculation functions
void _crc32_init(unsigned int* table);
unsigned int CalcCrc32(void* buff, int bytes, unsigned int crc);


int main(int argc, const char* argv[]) {

	char*	dev = strdup(SERIAL_DEV);
	int		baud = 115200;

	printf("PSXNET Serial Client " CLIENT_VER " by Lameguy64\n");
	printf("2016 Meido-Tek Productions\n\n");

	{

		int i;
		int showHelp = FALSE;

		for(i=1; i<argc; i++) {

			if (strcmp("-d", argv[i]) == 0) {
				free(dev);
				dev = strdup(argv[i+1]);
				i++;
			} else if (strcmp("-b", argv[i]) == 0) {
				baud = atoi(argv[i+1]);
				i++;
			} else if (strcmp("-h", argv[i]) == 0) {
				showHelp = TRUE;
			}

		}

		if (showHelp) {
			printf("Options:\n");
			printf("   psxnet -p <dev> -b <baud> -h\n\n");
			printf("   -d <dev>  - Specify serial device (default is /dev/ttyAMA0).\n");
			printf("   -b <baud> - Specify baud rate (default is 115200).\n");
			printf("   -h        - Show this help text.\n\n");
			return(0);
		}

	}


	printf("Opening %s at %d baud...", dev, baud);

	if (ser_Init(dev, baud) != SER_ERR_OK) {
		printf("ERROR: Cannot open serial device.\n");
		return(1);
	}

	printf("Ok.\n");

	printf("Client is now active! Press Esc to quit\n----\n");


	// Set conio emulation
	set_conio_terminal_mode();


	// Main client loop
	while(1) {

		// Keep reading the serial until a command was sent
		while(!kbhit()) {

			unsigned char cmd[2];

			memset(cmd, 0x00, 2);
			ser_ReadBytes(cmd, 2);

			// Parse the command
			if (cmd[0] == 0x8a) {

				switch(cmd[1]) {
				case 0:		// Comms test
					CommsTest();
					break;
				case 1:		// Get serial client information
					SendClientInfo();
					break;
				case 2:		// Connect via IP address
					Connect();
					break;
				case 3:		// Connect via host name
					ConnectHost();
					break;
				case 4:		// Close socket
					DisconnectSocket();
					break;
				case 5:		// Send bytes (with CRC check)
					SendBytes();
					break;
				case 6:		// Receive bytes (with CRC check)
					ReadBytes();
					break;
				default:
					printf("Unknown command %02x\r\n", cmd[1]);
					SendResult(1);
					break;
				}

			}

		}

		// Program exit
		if (kbhit()) {

			if (getch() == 27)
				break;

		}

	}

	ser_Close();

	printf("----\r\nClient closed...\r\n");

	return(0);

}


void CommsTest() {

	printf("PSX Comms Test... ");
	SendResult(0xdeadbeef);
	printf("Test packet sent.\r\n");

}

void SendClientInfo() {

	int len = strlen(CLIENT_INFO)+1;

	printf("Sending TCP client info... ");

	SendResult(0);
	ser_SendBytes(&len, 2);
	ser_SendBytes(CLIENT_INFO, strlen(CLIENT_INFO));

	printf("Done.\r\n");

}

void Connect() {

	short	ipaddrLen;
	char	ipaddr[18];
	u_short	port;
	short	connectType;
	int		p=0;
	int		socket;

	// Send response result
	SendResult(0);

	// Get port value
	while(ser_ReadBytes(&port, sizeof(short)) <= 0);
	// Get connection type
	ser_ReadBytes(&connectType, sizeof(short));

	// Get IP address len
	ser_ReadBytes(&ipaddrLen, sizeof(short));
	// Get IP address string
	while(p < ipaddrLen) {

		int result = ser_ReadBytes(&ipaddr[p], ipaddrLen);

		if (result < 0)
			continue;

		p += result;

	}

	printf("Connecting to IP address %s:%d... ", ipaddr, port);

	if (connectType == 0)
		printf("TCP... ");
	else if (connectType == 1)
		printf("UDP... ");
	else
		printf("Unknown... ");

	socket = net_Connect(ipaddr, port, connectType);

	if (socket < 0)
		printf("Cannot connect to IP.\r\n");
	else
		printf("Ok.\r\n");

	SendResult(socket);

}

void ConnectHost() {

	// Receive host name length and connection type
	short	p=0,hostNameLen,connectType;
	u_short	port;
	char	ipaddr[18];
	int		result;

	SendResult(0);

	while(ser_ReadBytes(&hostNameLen, 2) <= 0);
	ser_ReadBytes(&connectType, 2);
	ser_ReadBytes(&port, 2);

	char hostName[64] = { 0 };
	char* hostNamep;

	// Receive host name string
	while(p < hostNameLen) {

		result = ser_ReadBytes(&hostName[p], hostNameLen);

		if (result < 0)
			continue;

		p += result;

	}

	printf("Connecting to host %s port %d... ", hostName, port);

	if (connectType == 0)
		printf("TCP... ");
	else if (connectType == 1)
		printf("UDP... ");
	else
		printf("Unknown... ");

	hostNamep = strstr(hostName, "//")+2;

	if (hostNamep == NULL) {
		SendResult(2);
		return;
	}


	if (strchr(hostName, ':') == NULL) {
		SendResult(2);
		return;
	}

	*strchr(hostName, ':') = 0x00;
	*strchr(hostNamep, '/') = 0x00;


	short socket = net_ConnectByHostName(hostNamep, hostName, port, connectType, ipaddr);

	if (socket > 0)
		printf("Ok.\r\n");
	else
		printf("Could not connect to specified host.\r\n");

	SendResult(socket);

	if (socket < 0) {
		SendResult(1);
		return;
	}

	while(ser_ReadBytes(&result, sizeof(int)) <= 0);
	ser_SendBytes(&ipaddr, strlen(ipaddr)+1);

}

void DisconnectSocket() {

	short socket;

	SendResult(0);
	while(ser_ReadBytes(&socket, 2) <= 0);
	net_Disconnect(socket);

	printf("Socket %d is now closed.\r\n", socket);

}

void SendBytes() {

	short	socket;
	u_short	dataLen;
	u_int	crc32,ccrc32;

	int		dataRead;
	int		result;

	SendResult(0);

	while(ser_ReadBytes(&socket, sizeof(short)) <= 0);
	ser_ReadBytes(&dataLen, sizeof(short));
	ser_ReadBytes(&crc32, sizeof(int));


	printf("Sending %d byte(s) to socket %d... ", dataLen, socket);

	char buff[dataLen+1];
	memset(buff, 0x00, dataLen+1);

	SendResult(0);

	do {

		dataRead = 0;

		while(dataRead < dataLen) {

			result = ser_ReadBytes(&buff[dataRead], dataLen);

			if (result < 0)
				continue;

			dataRead += result;

		}

		ccrc32 = CalcCrc32(buff, dataRead, CRC32_REMAINDER);

		if (ccrc32 != crc32) {
			result = 1;
		} else {
			result = 0;
		}

		SendResult(result);	// 0 - OK, 1 - CRC mismatch, retry

	} while (result != 0);

	result = send(socket, buff, dataLen, 0);

	SendResult(result);

	printf("Ok.\r\n");

}

void ReadBytes() {

	short	socket;
	u_short	dataLen;
	unsigned int crc32;

	SendResult(0);

	while(ser_ReadBytes(&socket, sizeof(short)) <= 0);
	ser_ReadBytes(&dataLen, sizeof(u_short));

	printf("Receiving %d byte(s) from socket %d... ", dataLen, socket);

	char	buff[dataLen];
	int		dlen = recv(socket, buff, dataLen, 0);

	SendResult(dlen);

	if (dlen <= 0) {
		printf("Failed to receive data.\r\n");
		return;
	}

	printf("Got %d byte(s)... ", dlen);

	crc32 = CalcCrc32(buff, dlen, CRC32_REMAINDER);

	SendResult(crc32);

	int result;

	do {

		result = 0;

		// Send downloaded data
		ser_SendBytes(buff, dlen);

		// Get result (0 - OK, 1 - CRC mismatch, retry)
		while(ser_ReadBytes(&result, sizeof(int)) <= 0);

	} while (result != 0);

	printf("Ok.\r\n");

}

unsigned int ReadCommand() {

	unsigned int value = 0;

	if (ser_ReadBytes(&value, 4) > 0)
		return(value);

	return(0);

}

void SendResult(int result) {

	ser_SendBytes(&result, sizeof(int));

}

void _crc32_init(unsigned int* table) {

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

unsigned int CalcCrc32(void* buff, int bytes, unsigned int crc) {

	int	i;
	unsigned char*	byteBuff = (unsigned char*)buff;
	unsigned int	byte;
	unsigned int	crcTable[256];

    _crc32_init(crcTable);

	for(i=0; i<bytes; i++) {

		byte = 0x000000ffL&(unsigned int)byteBuff[i];
		crc = (crc>>8)^crcTable[(crc^byte)&0xff];

	}

	return(crc^0xFFFFFFFF);

}
