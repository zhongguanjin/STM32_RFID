#ifndef _CCU_H_
#define _CCU_H_

#include "config.h"
#include "com.h"


#define ccu_com   COM3
#define ccu_rxDeal()  com3_rxDeal()
#define CCU_BUF_MAX  32

typedef  union
{
      struct
      {
        uint8  frameLen;
        uint8  cmdType;
        uint8  cmdOrSta;
        uint8  info[CCU_BUF_MAX-3];   //28 byte
      };
      uint8 rxBuf[CCU_BUF_MAX];
} ccubuf_t;

extern ccubuf_t ccubuf;

extern void com3_tx_rsp(ccubuf_t ptx,uint8 sta,uint8 len);
#endif /* _CCU_H_ */
