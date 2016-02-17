#include <sys/types.h>
#include <libsio.h>
#include "serial.h"

void ser_Init(int baud) {

	_sio_control(1, 2, MR_SB_01 | MR_CHLEN_8 | 0x02); /* 8bit, no-parity, 1 stop-bit */
	_sio_control(1, 3, baud);
	_sio_control(1, 1, CR_RXEN | CR_TXEN);	/* RTS:off DTR:off */

	// Flush UART clean
	while(1) {

		int tc = 0;

		while(!(_sio_control(0, 0, 0) & SR_RXRDY)) {

			if (tc > SER_TIMEOUT)
				return;

			   tc++;

		}

		_sio_control(0, 4, 0);

	}

}

int ser_SendBytes(void* buff, int bytes) {

	int i;

	for(i=0; i<bytes; i++) {

		while((_sio_control(0, 0, 0) & (SR_TXU|SR_TXRDY)) != (SR_TXU|SR_TXRDY));
		_sio_control(1, 4, ((u_char*)buff)[i]);

	}

	return(bytes);

}

int ser_ReadBytes(void* buff, int bytes) {

	int		tc;
	int		bytesRead = 0;
	char*	c = (char*)buff;

	while(bytesRead < bytes) {

		tc = 0;

		while(!(_sio_control(0, 0, 0) & SR_RXRDY)) {

			if (tc > SER_TIMEOUT)
				return(bytesRead);

            tc++;

		}

		*c = _sio_control(0, 4, 0); c++;
		bytesRead++;

	}

	return(bytesRead);

}
