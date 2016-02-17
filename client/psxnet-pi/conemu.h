/*
 *	conio emulation module
 */

#ifndef _CONEMU_H
#define _CONEMU_H

void reset_terminal_mode();
void set_conio_terminal_mode();

int kbhit();
int getch();

#endif
