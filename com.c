#include "com.h"
#include "console.h"
#include "rfid.h"
#include "SysTick.h"

tComBuf_t comBuf[4];
USART_TypeDef * Tab_uartbase[5] = {USART1,USART2,USART3,UART4,UART5};

void com_cycleReset(comCycle_t * pBuf)
{
	pBuf->in = 0;
	pBuf->out = 0;
}

void com_init(COM_DEF COMx,u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	com_cycleReset(&(comBuf[COMx].tx));
	com_cycleReset(&(comBuf[COMx].rx));
	// 配置串口
    switch(COMx)
    {
        case COM1:
            {
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
                /* Configure USART Tx1 as alternate function push-pull */
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(GPIOA, &GPIO_InitStructure);
                /* Configure USART Rx1 as floating */
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
                GPIO_Init(GPIOA, &GPIO_InitStructure);
                USART_InitStructure.USART_BaudRate = baud;  //波特率
                USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //数据位
                USART_InitStructure.USART_StopBits = USART_StopBits_1;      //停止位
                USART_InitStructure.USART_Parity = USART_Parity_No;     //奇偶校验
                USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //数据流控制
                USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;     //模式
                USART_Init(USART1, &USART_InitStructure); // 配置串口
                USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
                USART_Cmd(USART1, ENABLE);
                break;
            }
        case COM2:
            {
                break;
            }
        case COM3:
            {
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
                /* Configure USART Tx3 as alternate function push-pull */
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(GPIOB, &GPIO_InitStructure);
                /* Configure USART Rx3 as floating */
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
                GPIO_Init(GPIOB, &GPIO_InitStructure);
                USART_InitStructure.USART_BaudRate = baud;  //波特率
                USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //数据位
                USART_InitStructure.USART_StopBits = USART_StopBits_1;      //停止位
                USART_InitStructure.USART_Parity = USART_Parity_No;     //奇偶校验
                USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //数据流控制
                USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;     //模式
                USART_Init(USART3, &USART_InitStructure); // 配置串口
                USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
                USART_Cmd(USART3, ENABLE);
                break;
            }
        case COM4:
            {
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
                 /* Configure USART Tx4 as alternate function push-pull */
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(GPIOC, &GPIO_InitStructure);
                /* Configure USART Rx4 asfloating */
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
                GPIO_Init(GPIOC, &GPIO_InitStructure);
                USART_InitStructure.USART_BaudRate = baud;  //波特率
                USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //数据位
                USART_InitStructure.USART_StopBits = USART_StopBits_1;      //停止位
                USART_InitStructure.USART_Parity = USART_Parity_No;     //奇偶校验
                USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //数据流控制
                USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;     //模式
                USART_Init(UART4, &USART_InitStructure); // 配置串口
                USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
                USART_Cmd(UART4, ENABLE);
                break;
            }
    }

}



u32 com_txLeft(COM_DEF COMx)
{
    if(((Tab_uartbase[COMx])->SR & USART_SR_TC) != 0) //0：发送还未完成；1：发送完成。
	{
		return 255;
	}
	else
	{
		u32 left = (u32)(comBuf[COMx].tx.out - comBuf[COMx].tx.in);
		return left;
	}
}

u32 com_rxLeft(COM_DEF COMx)
{
	return (u32)(comBuf[COMx].rx.in - comBuf[COMx].rx.out);
}

int com_getch(COM_DEF COMx, char * p)
{
	comCycle_t * pcbuf = &(comBuf[COMx].rx);

	if (pcbuf->in != pcbuf->out)
	{
		*p = pcbuf->buf[pcbuf->out++];
		return OK;
	}
	else
	{
		return ERR;
	}
}

void com_send(COM_DEF COMx, char * buf, u32 len)
{
	comCycle_t * pcbuf = &(comBuf[COMx].tx);

	while(1)
	{
		u32 left = com_txLeft(COMx);

		if(left > len)
		{
			while(len)
			{
				len--;
				pcbuf->buf[pcbuf->in++] = *buf++;
			}
		}
		else
		{
			len -= left;
			while(left--)
			{
				pcbuf->buf[pcbuf->in++] = *buf++;
			}
		}
		USART_ITConfig(Tab_uartbase[COMx], USART_IT_TXE, ENABLE);
		if(len == 0)
		{
			break;
		}
		else
		{
		  //break;
		  Delay_ms(5);
			//task_sleep(15);	// 按照波特率115200和缓冲区256设置，一般以发送缓冲区2/3做延时。
		}
	}
}

__weak void com1_rxDeal(void)
{
	char ch;
	if(com_rxLeft(COM1) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM1,&ch))
			{

			}
			else
			{
				break;
			}
		}
	}
}

__weak void com2_rxDeal(void)
{
	char ch;
	if(com_rxLeft(COM2) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM2,&ch))
			{

			}
			else
			{
				break;
			}
		}
	}
}

__weak void com3_rxDeal(void)
{
	char ch;
	if(com_rxLeft(COM3) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM3,&ch))
			{

			}
			else
			{
				break;
			}
		}
	}
}

__weak void com4_rxDeal(void)
{
	char ch;
	if(com_rxLeft(COM4) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM4,&ch))
			{

			}
			else
			{
				break;
			}
		}
	}
}

void USART1_IRQHandler(void)
{
	if((USART1->SR & USART_SR_RXNE) != 0)
	{
		// Receive data & clear flag
		comBuf[COM1].rx.buf[comBuf[COM1].rx.in++] = (u8)(USART1->DR);
	}

	if((USART1->SR & USART_SR_TXE) != 0)
	{
		if(comBuf[COM1].tx.in != comBuf[COM1].tx.out)
		{
			USART1->DR = (u16)(comBuf[COM1].tx.buf[comBuf[COM1].tx.out++]);
		}
		else
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);

		}
	}

}

void USART2_IRQHandler(void)
{
	if((USART2->SR & USART_SR_RXNE) != 0) //0：数据没有收到；1：收到数据，可以读出。
	{
		/* Receive data & clear flag */
		comBuf[COM2].rx.buf[comBuf[COM2].rx.in++] = (u8)(USART2->DR);
	}
	if((USART2->SR & USART_SR_TXE) != 0)//0：数据还没有被转移到移位寄存器；1：数据已经被转移到移位寄存器。
	{
		if(comBuf[COM2].tx.in != comBuf[COM2].tx.out)
		{
			USART2->DR = (u16)(comBuf[COM2].tx.buf[comBuf[COM2].tx.out++]);
		}
		else
		{
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		}
	}

}

void USART3_IRQHandler(void)
{
	if((USART3->SR & USART_SR_RXNE) != 0)
	{
		/* Receive data & clear flag */
		comBuf[COM3].rx.buf[comBuf[COM3].rx.in++] = (u8)(USART3->DR);
	}
	if((USART3->SR & USART_SR_TXE) != 0)
	{
		if(comBuf[COM3].tx.in != comBuf[COM3].tx.out)
		{
			USART3->DR = (u16)(comBuf[COM3].tx.buf[comBuf[COM3].tx.out++]);
		}
		else
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		}
	}

}

void UART4_IRQHandler(void)
{
/*
	if((UART4->SR & USART_SR_RXNE) != 0)
	{
		// Receive data & clear flag
		comBuf[COM4].rx.buf[comBuf[COM4].rx.in++] = (u8)(UART4->DR);
	}
	if((UART4->SR & USART_SR_TXE) != 0)
	{
		if(comBuf[COM4].tx.in != comBuf[COM4].tx.out)
		{
			UART4->DR = (u16)(comBuf[COM4].tx.buf[comBuf[COM4].tx.out++]);
		}
		else
		{
			USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
		}
	}
*/
    Rfid_Receive_Process();

}


