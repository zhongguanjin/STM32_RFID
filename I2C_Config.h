#ifndef __I2C_CONFIG_H__
#define __I2C_CONFIG_H__


#include "config.h"

#define  IIC_SDA             LATC3
#define  IIC_SCL             LATC4

#define 	SDA_IN()         {TRISC3 = 1;} // ‰»Î
#define 	SDA_OUT()        {TRISC3 = 0;} // ‰≥ˆ

#define     READ_SDA         RC3

#define		I2CInit()			{	TRISC3 = 0; TRISC4 = 0; ANSC3 = 0;\
                                    LATC3 = 1; LATC4 = 1;\
                                }



extern void IIC_Start(void);
extern void IIC_Stop(void);
extern uint8 IIC_Wait_Ack(void);
extern void IIC_Ack(void);
extern void IIC_NAck(void);

extern void IIC_Send_Byte(uint8 txd);
extern uint8 IIC_Read_Byte(uint8 ack);
#endif

