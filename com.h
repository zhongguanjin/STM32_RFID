#ifndef _COM_H_
#define _COM_H_


#include "config.h"

typedef struct {
	__IO u8	in;
	__IO u8	out;
	char	buf[256];
} comCycle_t;

typedef struct {
	comCycle_t	tx;
	comCycle_t	rx;
} tComBuf_t;

typedef enum {
	COM1 = 0,
	COM2,
	COM3,
	COM4
} COM_DEF;

extern tComBuf_t comBuf[4];


void com_cycleReset(comCycle_t * pBuf);
u32 com_txLeft(COM_DEF COMx);
u32 com_rxLeft(COM_DEF COMx);
int com_getch(COM_DEF COMx, char * p);
void com_send(COM_DEF COMx, uint8 * buf, u32 len);

void com2_rxDeal(void);
void com3_rxDeal(void);
void com4_rxDeal(void);
void com1_rxDeal(void);
void com_init(COM_DEF COMx,u32 baud);


#endif

