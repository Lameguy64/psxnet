#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#include "psxnet.h"
#include "bs_load.h"

#define BAUD	115200

#define URL		"http://www.psxdev.net/"
#define HOST	"www.psxdev.net"
#define FILE	"/forum/download/file.php?id=851"

char buff[65536];

int download();


int main() {

	DISPENV disp;
	DRAWENV draw;

	// Reset GPU for stability
	ResetGraph(0);

	// Initialize network library
	printf("\nInitializing TCP client library... ");

	if (net_Init(BAUD) == NET_ERR_OK) {
		printf("ERROR: Cannot communicate with TCP client.\n");
		return(0);
	}

	printf("Ok.\n");

	// Attempt to download a BS file from the forum post attachment
	if (download() > 0) {

		printf("Displaying... ");

		SetDefDispEnv(&disp, 0, 0, 640, 480);
		SetDefDrawEnv(&draw, 0, 0, 640, 480);

		disp.isrgb24 = 1;

		PutDrawEnv(&draw);
		PutDispEnv(&disp);
		SetDispMask(1);

		LoadBS(640, 480, 0, 0, 1, (u_long*)buff);

		printf("Ok.\n");

	}

	printf("End of program.\n");

	return(0);

}

int download() {

	char	tbuff[4096];
	char	ip[18];
	char	*c=buff;
	int		socket = 0;
	int		downloaded = 0;
	int		result,i;

	memset(ip, 0x00, 18);

    printf("Connecting... ");
    socket = net_ConnectHost(URL, 80, NET_TYPE_TCP, ip);

    if (socket <= 0)
		return(0);

    printf("%d - %s\n", socket, ip);


	{

		char data[] = { "GET " FILE " HTTP/1.1\r\nHost: " HOST "\r\n\r\n" };

		printf("Sending GET... ");
		net_Send(socket, (void*)data, strlen(data));
		printf("Ok.\n");

	}

	printf("Downloading...");

	do {

		result = net_Receive(socket, tbuff, 4096);

		if (result <= 0)
			break;

		if (result > 0) {

			if (downloaded == 0) {

				char *b = strstr(tbuff, "\r\n\r\n");

				if (b != NULL) {

					b += 4;

					result -= (int)(b-tbuff);

					memcpy(c, b, result);

				}


			} else {

                memcpy(c, tbuff, result);

			}

			downloaded += result;
			c += result;

			printf(".");

		}

	} while(1);

	printf("\nReceived: %d\n", downloaded);

    net_Disconnect(socket);

	return(downloaded);

}
