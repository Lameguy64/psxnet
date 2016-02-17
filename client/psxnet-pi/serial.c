#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "serial.h"


static int serialStream = -1;


int ser_Init(const char* device, int baud) {


	// Close if already open
	if (serialStream >= 0)
		ser_Close();

	// Open serial device
	serialStream = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (serialStream == -1)
		return(SER_ERR_OPEN);

	// Configure the serial device
	struct termios options;

	tcgetattr(serialStream, &options);

	switch(baud) {
	case 1200:
		options.c_cflag = B1200;
		break;
	case 2400:
		options.c_cflag = B2400;
		break;
	case 4800:
		options.c_cflag = B4800;
		break;
	case 9600:
		options.c_cflag = B9600;
		break;
	case 19200:
		options.c_cflag = B19200;
		break;
	case 38400:
		options.c_cflag = B38400;
		break;
	case 57600:
		options.c_cflag = B57600;
		break;
	case 115200:
		options.c_cflag = B115200;
		break;
	default:
		close(serialStream);
		return(SER_ERR_INVALID);
	}
	
	options.c_cflag |= CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	tcflush(serialStream, TCIFLUSH);
	tcsetattr(serialStream, TCSANOW, &options);

	return(SER_ERR_OK);

}


void ser_Close() {

	if (serialStream == -1)
		return;

	close(serialStream);

	serialStream = -1;

}

int ser_SendBytes(void* buff, int bytes) {

	if (serialStream < 0)
		return(-1);

	int ret = write(serialStream, buff, bytes);

	return(ret);

}

int ser_ReadBytes(void* buff, int bytes) {

	if (serialStream < 0)
		return(-1);

	int ret = read(serialStream, buff, bytes);

	return(ret);

}

/*int ser_SendBytes(void* buff, int bytes) {

	return(0);

}

int ser_ReadBytes(void* buff, int bytes) {

	u_short	len = 0;
	u_short msg;
	u_short offs = 0;
	u_short result;
	u_short to = 0;

	if (ser_ReadBytesRaw(&len, 2) <= 0)
		return(-1);

	msg = 0;
	ser_SendBytesRaw(&msg, 1);

	do {

		result = ser_ReadBytesRaw(((char*)buff)[offs], len);

		if (result <= 0) {

			to++;

			if (to > 1000) {
				msg = 1;
				ser_SendBytesRaw(&msg, 1);
				to=0;
				offs = 0;
			}

			continue;

		}

		offs += result;
		to = 0;

	} while(offs < len);

	return(len);

}*/
