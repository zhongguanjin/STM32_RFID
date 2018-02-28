#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "config.h"


typedef int (*consoleCallback)(char * buf, int len);
extern consoleCallback console_cb;




extern int uart2_getch(char * p);
extern uint8 val_getPara(int16 *cp,char *string);
extern void console_process(void);
extern void Uart3_Receive_Process(void);
extern void dbg_Init(void);
#endif
