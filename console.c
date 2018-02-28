#include "console.h"

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

 uart_buf_t uart3rx;

/*****************************************************************************
 函 数 名  : console_mainMenu
 功能描述  : 主菜单
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

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
 函 数 名  : console_main
 功能描述  : 主菜单驱动
 输入参数  : char * buf
             int len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int console_main(char * buf, int len)
{
	if(memcmp(buf,"reboot",6) == 0)
	{
        printf("reboot\r\n");
        Delay_ms(600);
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
 函 数 名  : cs_drc8837Test
 功能描述  : 脉冲阀测试驱动
 输入参数  : char * buf
             int len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

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
 函 数 名  : uart2_getch
 功能描述  : 串口获取字符函数
 输入参数  : char * p
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int uart2_getch(char * p)
{
    if (uart3rx.in != uart3rx.out)
    {
        *p = uart3rx.buf[uart3rx.out & CONSOLE_RX_BUF_MASK];
        uart3rx.out++;
        return 0;
    }
    else
    {
        return -1;
    }
}
/*****************************************************************************
 函 数 名  : val_getPara
 功能描述  : 字符串转整形数组函数
 输入参数  : char *string  要转化的字符串
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月6日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 val_getPara(int16 *cp,char *string)
{
    uint8 i=0,j=0;
    while(*string)
    {
       if(*string >= '0'&&*string <= '9')
       {
           if(*(string+1)==','||*(string+1)==' ')//数据后面是逗号，空格
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
       else if(*string == 0x0D) //回车符
       {
          j++;
          break;
       }
       string++;
    }
    return j;
}

void console_process(void)
{
    static char buf[256];
    static int len=0;
    char ch;
    if(0 == uart2_getch(&ch))
    {
        buf[len++] = ch;
        if(ch < 0x20)
        {
            if(len != 0)
            {   // 包含0D
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
            printf("%c\r\n",ch);
        }
    }
}

void dbg_Init(void)
{
    uart_bufInit(&uart3rx);
}

void Uart3_Receive_Process(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
        USART_ClearFlag(USART3, USART_FLAG_RXNE | USART_FLAG_ORE);
        uart3rx.buf[uart3rx.in++&CONSOLE_RX_BUF_MASK]= USART_ReceiveData(USART3);
	}
}
















