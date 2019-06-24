#include "console.h"
#include "dbg.h"
#include "SysTick.h"
#include "com.h"
#include "rfid.h"

#include "bsp_i2c.h"

#include "ml_fpm.h"
#include "SoftTimer.h"



int console_main(char * buf, int len);
void console_mainMenu(void);

int cs_eepromtest(char * buf, int len);
void eeprom_Menu(void);

int cs_rftest(char * buf, int len);
void rf_cmdMenu(void);
uint8 val_getPara(uint8 *cp,char *string);

int cs_softtimetest(char * buf, int len);
void soft_timeMenu(void);
    int pfTimerCallback(void *arg);


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
	printf("0:init rfid user\r\n");
	printf("1,softtime test\r\n");
	printf("a:read rf uid\r\n");
	printf("b:rf cmd\r\n");
    printf("c:add rfid user\r\n");
    printf("d:del rfid user\r\n");
    printf("e:query rfid user\r\n");
    printf("f:eeprom test\r\n");
    printf("g:ml sign\r\n");
    printf("h:ml clear\r\n");


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
		dbg("reboot");
		Delay_ms(1000);
		NVIC_SystemReset();
	}
   	switch(buf[0])
	{
	    case '0':
		{
	    #if USER_TEST
	        rfUsr_setDefault();
	    #else
            if(clear_rfid_user()!=ERR)
            {
                dbg("clsar ok");
            }
            else
            {
                dbg("err");
            }
        #endif
			break;
		}
		case '1':
		{
		    soft_timeMenu();
		    break;
		}
		case 'a':
		{
            uint32 uid = get_rf_uid();
            if(uid!=0)
            {
                dbg("uid:0x%X",uid);
            }
            else
            {
                dbg("err no card");
            }
			break;
		}
		case 'b':
		{
            rf_cmdMenu();
			break;
		}
		case 'c':
		{
		#if USER_TEST
		    UN32 bak_user;
		    bak_user.u = get_rf_uid();
		    if(rfUsr_append(bak_user.uch)==OK)
		#else
            if(add_rfid_user()==OK)
        #endif
            {
                dbg("add user ok");
            }
            else
            {
                dbg("add user err");
            }
			break;
		}
		case 'd':
		{
		 #if USER_TEST
		  #else
            if(del_rfid_user()==OK)
            {
                dbg("del user ok");
            }
            else
            {
                dbg("del user err");
            }
        #endif

			break;
		}
		case 'e':
		{
	    #if USER_TEST
		    rfUsr_showRfSerial();
	    #else
		    uint32 user_dat[5];
		    uint8 i;
		    uint8 num=query_rfid_user(user_dat);
            if(num !=0)
            {
                for(i=0;i<num;i++)
                {
                    dbg("user%d:0x%X",i,user_dat[i]);
                }
            }
            else
            {
                dbg("query user err");
            }
        #endif
			break;
		}
		case 'f':
		{
		    eeprom_Menu();
			break;
		}
		case 'g':
		{
		    ml_st_set(ML_ST_SIGN);
		    break;
		}
		case 'h':
		{
		    ml_st_set(ML_ST_CLEAR);
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
 函 数 名  : eeprom_Menu
 功能描述  : eeprom菜单函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void eeprom_Menu(void)
{
    console_cb = cs_eepromtest;
	printf("\r\n\t eeprom menu:\r\n");
	printf("read:1,addr,len \r\n");
	printf("write:2,addr,len \r\n");
	printf("addr:eeprom address,len:byte len\r\n");
}

/*****************************************************************************
 函 数 名  : cs_eepromtest
 功能描述  : eeprom测试驱动
 输入参数  : char * buf
             int len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int cs_eepromtest(char * buf, int len)
{
	union {
		uint8 u[3];
		struct {
		    uint8       fun;
			uint8		addr;
			uint8		len;
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    dbg("%d,%d,%d",para.fun,para.addr,para.len);
    if((i > 3 )&&( para.len > 127)&& (para.len!=0))
    {
        dbg("para err");
        return 1;
    }
    switch(para.fun)
    {
        case 1: //读
        {
			uint8 i;
            uint8 eeprom_buf[128];
            EEPROM_Read(para.addr,eeprom_buf, para.len);
            for (i=0; i<128; i++)
            {
                printf("0x%02X ", eeprom_buf[i]);
                if((i+1)%16 == 0)
                {
                    printf("\r\n");
                }
            }
            break;
        }
        case 2://写
        {
						uint8 i;
            uint8 eeprom_buf[128];
            for(i=0;i<128;i++)
            {
               eeprom_buf[i]=0xFF;
            }
            EEPROM_Write(para.addr,eeprom_buf, para.len);
            break;
        }
		default:
		{
            return 1;
		}
    }
    memset(para.u,0,sizeof(para));
    eeprom_Menu();
    return 0;

}

/*****************************************************************************
 函 数 名  : rf_cmdMenu
 功能描述  : rfid菜单函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年7月26日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void rf_cmdMenu(void)
{
    console_cb = cs_rftest;
	printf("\r\n\t rf menu:\r\n");
	printf("read dat:1,mid \r\n");
	//dbg("write dat:2,mid\r\n");
}
/*****************************************************************************
 函 数 名  : cs_rftest
 功能描述  : rf测试驱动
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
int cs_rftest(char * buf, int len)
{
	union {
		uint8 u[2];
		struct {
			uint8		FUNC;
			uint8		MID;
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    dbg("%d,%d",para.FUNC,para.MID);
    if(i>3)
    {
        dbg("para err");
        return 1;
    }
    switch(para.FUNC)
    {
        case 1:
        {
            read_rf_dat(para.MID);//读块号的值
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
    rf_cmdMenu();
    return 0;
}


/*****************************************************************************
 函 数 名  : soft_timeMenu
 功能描述  : soft_time菜单函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年7月26日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void soft_timeMenu(void)
{
    console_cb = cs_softtimetest;
	printf("\r\n\t softtime menu:\r\n");
	printf("creat timer:1,arg\r\n");
	printf("kill timer: 2,arg\r\n");
	printf("reset timer:3,arg\r\n");
	//dbg("write dat:2,mid\r\n");
}
/*****************************************************************************
 函 数 名  : cs_softtimetest
 功能描述  : softtime测试驱动
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
    TIMER_TABLE *main_task;

int cs_softtimetest(char * buf, int len)
{
	union {
		uint8 u[2];
		struct {
			uint8		FUNC;
			uint8		MID;
		};
	} para;
    uint8 i=0;
    i=val_getPara(para.u,buf);
    dbg("%d,%d",para.FUNC,para.MID);
    if(i>3)
    {
        dbg("para err");
        return 1;
    }
    switch(para.FUNC)
    {
        case 1:
        {
            main_task= CreatTimer(1000, 1, pfTimerCallback, NULL);
            break;
        }
        case 2:
        {
            if(KillTimer(main_task)==SW_OK)
            {
                dbg("kill ok");
            }
            else
            {
                dbg("kill err");
            }
            break;
        }
        case 3:
        {
            if(ResetTimer(main_task)== SW_OK)
            {
                dbg("reset ok");
            }
            else
            {
                dbg("reset err");
            }
            break;
        }
		default:
		{
            return 1;
		}
    }
    memset(para.u,0,sizeof(para));
    soft_timeMenu();
    return 0;
}

int pfTimerCallback(void *arg)
{
    dbg("pfTimerCallback");
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
uint8 val_getPara(uint8 *cp,char *string)
{
    uint8 i=0,j=0,k=0;
    while(*string)
    {
       if(*string >= '0'&&*string <= '9')
       {
           k++;
           if(k==1)
           {
             cp[i]= *string-'0';
           }
           else if(k>=2)
           {
              cp[i]= cp[i]*10+(*string-'0');
           }
       }
       else if(*string==','||*string==' ')
       {
           k=0;
           i++;
           j++;
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

















