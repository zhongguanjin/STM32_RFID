#ifndef UART_H
#define UART_H

#include <stdio.h>

#define Debug printf

void COM3_4_Init( void);



extern void Uart3_Send_Data(unsigned char *buf,unsigned char num);

extern void Uart4_Send_Data(unsigned char *buf,unsigned char num);

#endif
