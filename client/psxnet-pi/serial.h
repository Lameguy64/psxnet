/*
 *	Very simple serial communications module.
 */

#ifndef _SERIAL_H
#define _SERIAL_H

#define	SER_ERR_OK		0
#define SER_ERR_OPEN	-1
#define SER_ERR_INVALID	-2

int ser_Init(const char* device, int baud);
void ser_Close();
int ser_SendBytes(void* buff, int bytes);
int ser_ReadBytes(void* buff, int bytes);

#endif
