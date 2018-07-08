#ifndef _DBG_H
#define _DBG_H

#include "Uart.h"
#include "config.h"


#define dbg(fmt,args...)	printf("[%s]-[%d]:" fmt "\r\n",__func__,__LINE__,## args)


extern void dbg_hex(uint8 *buf,uint8 len);



#endif

