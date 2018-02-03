#include "config.h"

#include "rfid.h"

void rf_reInitRx(u8 mode);
int16 rf_sendCmd(u32 cmdHead, const u8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );
void Rfid_Task_Process(void);

rfMux_t rfMux;
Uartrx_t UartRx;
uint8 rxbuf[32];

uint8 RFID_STATE;

uint8 Wallet_init_flg;  //钱包初始化标志

const u8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1
const u8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};	// def. key1 for blk1
const u8 C2M_info[]={0x00,0x26};
const u8 C2O_info[] ={0x00};
/*****************************************************************************
 函 数 名  : rf_bccCalc
 功能描述  : 校验码函数
 输入参数  : uint8 * buf
             uint8 len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月11日 星期二
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 rf_bccCalc(uint8 * buf,uint8 len)
{
	uint8 bcc = 0;
	while(len--)
	{
		bcc ^= *buf++;
	}
	return ~bcc;
}

void rf_reInitRx(u8 mode)
{
	rfMux.rIdx = 0;
	rfMux.frameOK = 0;
	rfMux.mode = mode;
}


uint8 rf_wait( void )
{
    u32 rfRxTicks = sys_ticks() + 2000;
	while(1)
	{
		Delay_ms(5);
        if(rfMux.frameOK == 1)
        {
        	//printf("rfMux.frameOK:%d\r\n",rfMux.frameOK);
            return OK;
        }
        if(tick_timeout(rfRxTicks))
        {
            printf("rfMux.frame_err\r\n");
           return ERR;
        }

	}
}

/*****************************************************************************
 函 数 名  : rf_sendCmd
 功能描述  : RF发送命令帧函数
 输入参数  : u32 cmdHead
             const u8 * info
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月11日 星期二
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int16 rf_sendCmd(u32 cmdHead, const u8 * info)
{
	rfPkt_t rfSend;
	int16 len;
	rfSend.cmdHead = cmdHead;
	len = (u32)(rfSend.infoLen);
	if((len != 0) && (info != NULL))
	{
		memcpy(rfSend.info,info,len);
	}
	//帧长必须不小于6 字节，最大不能超过70 字节，且帧长必须等于信息长度加6；
	rfSend.frameLen = rfSend.infoLen + 6;
	//校验和，从FrameLen 开始到Info 的最后一字节异或取反
	rfSend.info[len++] = rf_bccCalc((u8 *)&rfSend,(rfSend.infoLen+4));
	rfSend.info[len++] = 0x03;
	len += 4;
	rf_reInitRx(0);  //主动交互模式
	Uart4_Send_Data((uint8 *)&rfSend,len); //发送命令帧数据
	//for recv
    if(rf_wait() == OK)
    {
        if(rfMux.rxBuf[2] == 0)  //状态成功
        {	// 成功
        	return OK;
        }
        else
        {
        	printf("rsp status: %x\r\n",rfMux.rxBuf[2]);
        	return ERR;
        }
    }
    else
    {
        //printf("overtime\r\n");
        return ERR;
    }

}
/*****************************************************************************
 函 数 名  : rf_init
 功能描述  : RFID初始化函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月13日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 rf_init(void)
{
    uint8 times = 1;
	memset(&rfMux,0,sizeof(rfMux));
	while(times-- != 0)
	{	// 发送两空格，初始化串口通信
		Uart4_Send_Data(" ",1);   //0x20
		Delay_us(40); //40us
		Uart4_Send_Data(" ",1);
		Delay_ms(200);//20ms
		if(rfMux.rxBuf[0] == 0x06)
		{
		    printf("rfMux.rxBuf[0]: %d\r\n",rfMux.rxBuf[0]);
			break;
		}
		Delay_ms(1000);
	}
	times = 2;
	while(times-- != 0)
	{
		if(rf_sendCmd(RF_HEAD_C1A,NULL) == OK)
		{
		     //printf("RF_HEAD_C1A ok r\n");
			if(rf_sendCmd(RF_HEAD_C1E1,C1E_info) == OK)
			{
			    //printf("RF_HEAD_C1E1 ok\r\n");
				return OK;
			}
			else
			{
			     printf("RF_HEAD_C1E1 err\r\n");
			     return ERR;
			}
		}
		else
		{
		     printf("RF_HEAD_C1A err\r\n");
		     return ERR;
		}
	}
	return ERR;
}


void rf_check(void)
{
    RFID_STATE =STATE_RFID_IDLE;
	rf_reInitRx(0);
    if( rf_init() == OK)
    {   /* 检测到模块 进入自动检测模式 */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //该命令用于卡片的自动检测，执行该命令成功后，在UART 模式下，模块将主动发送
        //读取到卡片的数据
        {
            //printf("RF_HEAD_C2N ok\r\n");
            rf_reInitRx(1);
        }
        else
        {   /* 检测RF模块错误 */
           //printf("RF_HEAD_C2N err\r\n");
        }
    }
    else
    {
        printf("rf_init err\r\n");
    }
}

void Rfid_Task_Process()
{
    switch(RFID_STATE)
    {
        case STATE_RFID_IDLE:
            {
                break;
            }
        case STATE_RFID_TIME:
            {
                if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                {
                   u32 rfRxtime = sys_ticks() + 200;
                    if(tick_timeout(rfRxtime))
                    {
                        RFID_STATE = STATE_RFID_TIME;
                    }
                }
                else
                {
                    rfMux.status = 0;
                    memset(&rfMux.devInfo,0,sizeof(rfMux.devInfo));
                    // 卡移走，再次进入自动模式
                    if(OK == rf_sendCmd(RF_HEAD_C2N,C2N_info))
                    {
                        rf_reInitRx(1);
                        RFID_STATE = STATE_RFID_IDLE;
                    }
                }
                break;
            }
        case STATE_RFID_CHKCARD:
            {
                if(rfMux.rPkt.infoLen == 0x19)
                {
                    memcpy(&(rfMux.devInfo),rfMux.rxBuf+9,20);
                    //Uart3_Send_Data(rfMux.devInfo.uid.uch, 4);
                    printf("uid: %X\r\n",rfMux.devInfo.uid.u);
                    rfMux.status = 1;
                    BELL(ON);
                    Delay_ms(100);
                    BELL(OFF);
                    Delay_ms(100);
                    BELL(ON);
                    Delay_ms(100);
                    BELL(OFF);
                }
                RFID_STATE = STATE_RFID_TIME;
                break;
            }
        case STATE_RFID_READCARD:
            {
                if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                {
                     memcpy(&(UartRx.info),rfMux.devInfo.uid.uch,4);
                     UartRx.cmdOrSta = 0; //成功
                     UartRx.frameLen += 4;
                     UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                     UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                     Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                     memset(&(UartRx.rxBuf),0,32);
                }
                else
                {
                     printf("card leave!");//卡离开
                     UartRx.cmdOrSta = 1; //失败
                     UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                     UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                     Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                     memset(&(UartRx.rxBuf),0,32);

                }
                RFID_STATE = STATE_RFID_TIME;
                break;
            }
        case STATE_RFID_WRITEDAT:
            {
               // rfC2H_t info_w;
                rfC2E_t info;
                if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                {
                    info.mode = 0x60;
                    memcpy(info.uid,rfMux.devInfo.uid.uch,4);
                    info.keyid = 1;
                    info.blkid = UartRx.info[0];
                    if(OK == rf_sendCmd(RF_HEAD_C2E,(u8 *)&info)) //密钥验证
                    {
                        //info_w.blkid = UartRx.info[0];
                       // memcpy(info_w.dat,&UartRx.info[1],16);
                        if(OK == rf_sendCmd(RF_HEAD_C2H,(u8 *)&UartRx.info))
                        {
                            printf("write OK!");
                            UartRx.cmdOrSta = 0; //成功
                            UartRx.frameLen = UartRx.frameLen-17;
                            UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                            UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                            Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                            memset(&(UartRx.rxBuf),0,32);

                        }
                        else
            {
                            printf("write err!");
                            UartRx.cmdOrSta = 1; //失败
                            UartRx.frameLen = UartRx.frameLen-17;
                            UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                            UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                            Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                            memset(&(UartRx.rxBuf),0,32);

                        }

                    }
                }
                else
                {
                     printf("card leave!");//卡离开
                     UartRx.cmdOrSta = 1; //失败
                     UartRx.frameLen = UartRx.frameLen-17;
                     UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                     UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                     Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                     memset(&(UartRx.rxBuf),0,32);
                }
                RFID_STATE = STATE_RFID_TIME;
                break;
            }
        case STATE_RFID_READDAT:
            {
                rfC2H_t info_w;
                rfC2E_t info;
                if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
                {
                    info.mode = 0x60;
                    memcpy(info.uid,rfMux.devInfo.uid.uch,4);
                    info.keyid = 1;
                    info.blkid = UartRx.info[0];
                    if(OK == rf_sendCmd(RF_HEAD_C2E,(u8 *)&info)) //密钥验证
                    {
                        info_w.blkid = UartRx.info[0];
                        if(OK == rf_sendCmd(RF_HEAD_C2G,(u8 *)&info_w.blkid))
                        {
                            printf("read OK!");
                            memcpy(&(UartRx.info),rfMux.rPkt.info,16);
                            UartRx.cmdOrSta = 0; //成功
                            UartRx.frameLen = UartRx.frameLen+15;

                            UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                            UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                            Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                            memset(&(UartRx.rxBuf),0,32);

                        }
                        else
                        {
                            printf("[%s]write err!",__FUNCTION__);
                            UartRx.cmdOrSta = 1; //失败
                            UartRx.frameLen = UartRx.frameLen-1;
                            UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                            UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                            Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                            memset(&(UartRx.rxBuf),0,32);

                        }

                    }
                }
                else
                {
                     printf("card leave!");//卡离开
                     UartRx.cmdOrSta = 1; //失败
                     UartRx.frameLen = UartRx.frameLen-1;
                     UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                     UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                     Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                     memset(&(UartRx.rxBuf),0,32);
                }
                RFID_STATE = STATE_RFID_TIME;

                break;
            }
    }
}

void Rx_Task_Process(void)
{
    switch(UartRx.cmdType)
    {
       case 0x01: //卡片相关操作命令
       {
            switch (UartRx.cmdOrSta)
            {
               case 0xA1: //读卡号命令，无需验证密钥
               {
                   RFID_STATE = STATE_RFID_READCARD;
					break;
               }
               case 0xA2: //原来为注册卡命令，新版软件取消了此命令
               {

                    break;
               }
               case 0xA3: //读指定数据块命令，验证KEYA
               {
                   RFID_STATE = STATE_RFID_READDAT;
                    break;
               }
               case 0xA4: //写指定数据块命令，验证KEYA
               {
                   RFID_STATE = STATE_RFID_WRITEDAT;
                    break;
               }
               case 0xA5: //设置IC卡密钥命令,验证KEYA
               {
                   //RFID_STATE = STATE_RFID_READDAT;
                    break;
               }
               case 0xA6: // //钱包初始化命令，验证KEYA
               {
                    RFID_STATE = STATE_WALLET_INIT;
                    break;
               }
               case 0xA7:  //钱包扣款命令，验证KEYA
               {
                    RFID_STATE = STATE_WALLET_DEC;
                    break;
               }
               case 0xA8:  //钱包充值命令，验证KEYA
               {
                   RFID_STATE = STATE_WALLET_INC;
                    break;
               }
               case 0xA9: //钱包余额查询命令，验证KEYA
               {
                    RFID_STATE = STATE_WALLET_BALANCE;
                    break;
               }
               default: //其它
               {

                    break;
               }
            }
            break;
       }
       case 0x02: //读写器参数查询
       {

            break;
       }
       case 0x03: //读写器参数设置
       {

            break;
       }
       default: //其它
       {

            break;
       }

    }

}

void Uart_Receive_Process(void)
{
	static uint8 index = 0;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
        USART_ClearFlag(USART3, USART_FLAG_RXNE | USART_FLAG_ORE);
        rxbuf[index++]= USART_ReceiveData(USART3);
        if((rxbuf[index-1] == 0x03)&&(index == rxbuf[0]))
        {
            index -=2;// 指向bcc
            if(rf_bccCalc(rxbuf, index) == rxbuf[index]) //bcc ok?
            {
                memcpy(&(UartRx.rxBuf),rxbuf,index);
                index = 0;
                Rx_Task_Process();
            }
            else
            {
                memset(&rxbuf,0,32);
                index = 0;
            }
        }
	}
}

void Rfid_Receive_Process(void)
{
    if(USART_GetFlagStatus(UART4,USART_IT_RXNE)!= RESET)
    {
        USART_ClearFlag(UART4, USART_FLAG_RXNE | USART_FLAG_ORE);
        rfMux.rxBuf[rfMux.rIdx++] = USART_ReceiveData(UART4);
        if ((rfMux.rxBuf[rfMux.rIdx-1] == 0x03) && (rfMux.rIdx == rfMux.rPkt.frameLen))
        {
            rfMux.rIdx -= 2;    // 指向bcc
            if(rf_bccCalc(rfMux.rxBuf,rfMux.rIdx) == rfMux.rxBuf[rfMux.rIdx])
            {
                 rfMux.frameOK =1; //接收完一帧数据
                if(rfMux.mode == 1)
                {
                    RFID_STATE =STATE_RFID_CHKCARD;
                    //主动上报，检测到刷卡
                }
            }
        }
    }
}


