#ifndef _DBG_H
#define _DBG_H

#include "Uart.h"
#include "config.h"


#define dbg(fmt,args...)	printf("[%s]:" fmt "\r\n",__func__,## args)


extern void dbg_hex(uint8 *buf,uint16 len);



#endif

