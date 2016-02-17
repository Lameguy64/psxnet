#ifndef _SERIAL_H
#define _SERIAL_H

#define SER_TIMEOUT	5000

void ser_Init(int baud);

int ser_SendBytes(void* buff, int bytes);
int ser_ReadBytes(void* buff, int bytes);

#endif
