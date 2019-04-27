#include "ccu.h"

#include "rfid.h"


ccubuf_t ccubuf;
uint8 rxbuf[32];
void  ccu_cmdDeal(void);
/*****************************************************************************
 函 数 名  : com3_tx_rsp
 功能描述  : com3应答函数
 输入参数  : uint8 sta  0-ok，1-err
             uint8 len
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年7月8日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void com3_tx_rsp(ccubuf_t ptx,uint8 sta,uint8 len)
{
    ptx.cmdOrSta = sta; //成功
    ptx.frameLen = len;
    ptx.rxBuf[ptx.frameLen-2] = rf_bccCalc(ptx.rxBuf, ptx.frameLen-2);
    ptx.rxBuf[ptx.frameLen-1] = 0x03;
    com_send(ccu_com,ptx.rxBuf,ptx.frameLen); //中断发送
}

/*****************************************************************************
 函 数 名  : com3_rxDeal
 功能描述  : ccu接收数据帧
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月7日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void com3_rxDeal(void)
{
	char ch;
	static uint8 index = 0;
	if(com_rxLeft(ccu_com) != 0)
	{
		while(1)
		{
			if(OK == com_getch(ccu_com,&ch))
			{
                rxbuf[index++]= ch;
                if((rxbuf[index-1] == 0x03)&&(index == rxbuf[0]))
                {
                    index -=2;// 指向bcc
                    if(rf_bccCalc(rxbuf, index) == rxbuf[index]) //bcc ok?
                    {
                        memcpy(&(ccubuf.rxBuf),rxbuf,index);
                        index = 0;
                        ccu_cmdDeal();
                    }
                    else
                    {
                        memset(&rxbuf,0,32);
                        index = 0;
                    }
                }

			}
			else
			{
				break;
			}
		}
	}
}

/*****************************************************************************
 函 数 名  : ccu_cmdDeal
 功能描述  : ccu命令处理
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月7日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void ccu_cmdDeal(void)
{
    switch(ccubuf.cmdType)
    {
       case 0x01: //卡片相关操作命令
       {
            switch (ccubuf.cmdOrSta)
            {
               case 0xA1: //读卡号命令，无需验证密钥
               {
                    //RFID_STATE = STATE_RFID_READCARD;
                    rf_state_set(STATE_RFID_READCARD);
					break;
               }
               case 0xA2: //原来为注册卡命令，新版软件取消了此命令
               {

                    break;
               }
               case 0xA3: //读指定数据块命令，验证KEYA
               {
                    //RFID_STATE = STATE_RFID_READDAT;
                    rf_state_set(STATE_RFID_READDAT);
                    break;
               }
               case 0xA4: //写指定数据块命令，验证KEYA
               {
                   //RFID_STATE = STATE_RFID_WRITEDAT;
                   rf_state_set(STATE_RFID_WRITEDAT);
                    break;
               }
               case 0xA5: //设置IC卡密钥命令,验证KEYA
               {
                   //RFID_STATE = STATE_RFID_READDAT;
                    //rf_state_set(STATE_RFID_WRITEDAT);
                    break;
               }
               case 0xA6: // //钱包初始化命令，验证KEYA
               {
                    //RFID_STATE = STATE_WALLET_INIT;
                     rf_state_set(STATE_WALLET_INIT);
                    break;
               }
               case 0xA7:  //钱包扣款命令，验证KEYA
               {
                    //RFID_STATE = STATE_WALLET_DEC;
                     rf_state_set(STATE_WALLET_DEC);
                    break;
               }
               case 0xA8:  //钱包充值命令，验证KEYA
               {
                   //RFID_STATE = STATE_WALLET_INC;
                    rf_state_set(STATE_WALLET_INC);
                    break;
               }
               case 0xA9: //钱包余额查询命令，验证KEYA
               {
                    //RFID_STATE = STATE_WALLET_BALANCE;
                     rf_state_set(STATE_WALLET_BALANCE);
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


