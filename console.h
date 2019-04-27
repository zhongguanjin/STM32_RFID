#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "config.h"
#include "com.h"

#define console_com  COM1
#define console_rxDeal()  com1_rxDeal()

typedef int (*consoleCallback)(char * buf, int len);
extern consoleCallback console_cb;




extern int uart1_getch(char * p);



extern void dbg_Init(void);
extern void Uart1_Receive_Process(void);

#endif
