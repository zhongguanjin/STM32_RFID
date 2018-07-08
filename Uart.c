
#include "stm32f10x.h"
#include "Uart.h"



//发送新封装的包
void Uart3_Send_Data(unsigned char *buf,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{
	 	USART_SendData(USART3, buf[i]);
	 	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}
}

void Uart4_Send_Data(unsigned char *buf,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{
	 	USART_SendData(UART4, buf[i]);
	 	while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
	}
}

void Uart1_Send_Data(unsigned char *buf,unsigned char num)
{
	unsigned char i;
	for(i=0;i<num;i++)
	{
	 	USART_SendData(USART1, buf[i]);
	 	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
}


//不使用半主机模式
#if 1 //如果没有这段，则需要在target选项中选择使用USE microLIB
#pragma import(__use_no_semihosting)
struct __FILE
{
	int handle;
};
FILE __stdout;

_sys_exit(int x)
{
	x = x;
}
#endif

int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (unsigned char) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{}

	return ch;
}



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

