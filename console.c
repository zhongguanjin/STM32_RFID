#include "console.h"
#include "dbg.h"
#include "SysTick.h"
#include "com.h"

int console_main(char * buf, int len);
void console_mainMenu(void);
int cs_drc8837Test(char * buf, int len);
uint8 val_getPara(int16 *cp,char *string);

consoleCallback console_cb = NULL;


#define	CONSOLE_RX_BUF_LEN		32	// must 2**n
#define	CONSOLE_RX_BUF_MASK	(CONSOLE_RX_BUF_LEN-1)

typedef struct _UART_BUF_TAG
{
	unsigned char	in;
	unsigned char	out;
	unsigned char	buf[CONSOLE_RX_BUF_LEN];
} uart_buf_t;

 uart_buf_t uart1rx;

/*****************************************************************************
 �� �� ��  : console_mainMenu
 ��������  : ���˵�
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void console_mainMenu(void)
{
	printf("\r\n\t cs menu:\r\n");
	printf("a: get bat val\r\n");
	printf("b: get tempsensor val\r\n");
	printf("c: motor test\r\n");
	printf("d: drv8837 test\r\n");
}
/*****************************************************************************
 �� �� ��  : console_main
 ��������  : ���˵�����
 �������  : char * buf
             int len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
int console_main(char * buf, int len)
{
	if(memcmp(buf,"reboot",6) == 0)
	{
		dbg("reboot");
		Delay_ms(1000);
		NVIC_SystemReset();
	}
   	switch(buf[0])
	{
		case 'a':
		{
			break;
		}
		case 'b':
		{
			break;
		}
		case 'c':
		{
			break;
		}
		case 'd':
		{
			break;
		}
		default:
		{
            return 1;
		}
    }
	return 0;
}

/*****************************************************************************
 �� �� ��  : cs_drc8837Test
 ��������  : ���巧��������
 �������  : char * buf
             int len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
int cs_drc8837Test(char * buf, int len)
{
	union {
		int16 u[2];
		struct {
			int16		FUNC;
			int16		MID;
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    switch(para.FUNC)
    {
        case 1:
        {
            break;
        }
        case 2:
        {
            break;
        }
        case 3:
        {
            break;
        }
		default:
		{
            return 1;
		}
    }
    memset(para.u,0,sizeof(para));
    //cs_drv8837Menu();
    return 0;

}


void uart_bufInit(uart_buf_t * pBuf)
{
    pBuf->in = 0;
    pBuf->out = 0;
}

/*****************************************************************************
 �� �� ��  : uart2_getch
 ��������  : ���ڻ�ȡ�ַ�����
 �������  : char * p
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��9��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
int uart1_getch(char * p)
{
    if (uart1rx.in != uart1rx.out)
    {
        *p = uart1rx.buf[uart1rx.out & CONSOLE_RX_BUF_MASK];
        uart1rx.out++;
        return 0;
    }
    else
    {
        return -1;
    }
}
/*****************************************************************************
 �� �� ��  : val_getPara
 ��������  : �ַ���ת�������麯��
 �������  : char *string  Ҫת�����ַ���
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��2��6��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8 val_getPara(int16 *cp,char *string)
{
    uint8 i=0,j=0;
    while(*string)
    {
       if(*string >= '0'&&*string <= '9')
       {
           if(*(string+1)==','||*(string+1)==' ')//���ݺ����Ƕ��ţ��ո�
           {
               cp[i]= *string - '0';
               i++;
               j++;
           }
           else
           {
                cp[i]= cp[i]*10+(*string-'0');
           }
       }
       else if(*string == 0x0D) //�س���
       {
          j++;
          break;
       }
       string++;
    }
    return j;
}


 void com1_rxDeal(void)
{
    static char buf[256];
    static int len=0;
    char ch;
	if(com_rxLeft(COM1) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM1,&ch))
			{
                buf[len++] = ch;
                if(ch < 0x20)
                {
                    if(len != 0)
                    {   // ����0D
                        buf[len] = 0;
                        if(console_cb == NULL)
                        {
                            console_cb = console_main;
                        }
                        if(console_cb(buf,len) != 0)
                        {
                            console_cb = NULL;
                            console_mainMenu();
                        }
                    }
                    len = 0;
                }
                else
                {
                    //printf("%c\r\n",ch);
                }

			}
			else
			{
				break;
			}
		}
	}
}

#if 0
void console_process(void)
{
    static char buf[256];
    static int len=0;
    char ch;
    if(0 == uart1_getch(&ch))
    {
        buf[len++] = ch;
        if(ch < 0x20)
        {
            if(len != 0)
            {   // ����0D
                buf[len] = 0;
                if(console_cb == NULL)
                {
                    console_cb = console_main;
                }
                if(console_cb(buf,len) != 0)
                {
                    console_cb = NULL;
                    console_mainMenu();
                }
            }
            len = 0;
        }
        else
        {
            //printf("%c\r\n",ch);
        }
    }
}


void dbg_Init(void)
{
    uart_bufInit(&uart1rx);
}


void Uart1_Receive_Process(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
        USART_ClearFlag(USART1, USART_FLAG_RXNE | USART_FLAG_ORE);
        uart1rx.buf[uart1rx.in++&CONSOLE_RX_BUF_MASK]= USART_ReceiveData(USART1);
	}
}

#endif















