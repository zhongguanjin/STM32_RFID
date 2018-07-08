#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "config.h"
#include "com.h"


typedef int (*consoleCallback)(char * buf, int len);
extern consoleCallback console_cb;



#define console_process()  com1_rxDeal()
extern int uart1_getch(char * p);
extern uint8 val_getPara(int16 *cp,char *string);
//extern void console_process(void);

extern void dbg_Init(void);
extern void Uart1_Receive_Process(void);

#endif
