#include "config.h"
#include "com.h"
#include "rfid.h"
#include "dbg.h"

void rf_reInitRx(uint8 mode);
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );
void com3_tx_rsp(uint8 sta,uint8 len);


rfMux_t rfMux;
Uartrx_t UartRx;
uint8 rxbuf[32];

uint8 RFID_STATE;

uint8 Wallet_init_flg;  //Ǯ����ʼ����־



const uint8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1
const uint8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};	// def. key1 for blk1
const uint8 C2M_info[]={0x00,0x26};
const uint8  C2O_info[] ={0x00};
/*****************************************************************************
 �� �� ��  : rf_bccCalc
 ��������  : У���뺯��
 �������  : uint8 * buf
             uint8 len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��7��11�� ���ڶ�
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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

void rf_reInitRx(uint8 mode)
{
	rfMux.rIdx = 0;
	rfMux.frameOK = 0;
	rfMux.mode = mode;
}


uint8 rf_wait( void )
{
    uint32 rfRxTicks = sys_ticks() + 2000;
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
            dbg("rfMux.frame_err\r\n");
           return ERR;
        }

	}
}

/*****************************************************************************
 �� �� ��  : rf_sendCmd
 ��������  : RF��������֡����
 �������  : u32 cmdHead
             const u8 * info
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��7��11�� ���ڶ�
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info)
{
	rfPkt_t rfSend;
	int16 len;
	rfSend.cmdHead = cmdHead;
	len = (uint32)(rfSend.infoLen);
	if((len != 0) && (info != NULL))
	{
		memcpy(rfSend.info,info,len);
	}
	//֡�����벻С��6 �ֽڣ�����ܳ���70 �ֽڣ���֡�����������Ϣ���ȼ�6��
	rfSend.frameLen = rfSend.infoLen + 6;
	//У��ͣ���FrameLen ��ʼ��Info �����һ�ֽ����ȡ��
	rfSend.info[len++] = rf_bccCalc((uint8 *)&rfSend,(rfSend.infoLen+4));
	rfSend.info[len++] = 0x03;
	len += 4;
	rf_reInitRx(0);  //��������ģʽ
	Uart4_Send_Data((uint8 *)&rfSend,len); //��������֡����
	//for recv
    if(rf_wait() == OK)
    {
        if(rfMux.rxBuf[2] == 0)  //״̬�ɹ�
        {	// �ɹ�
        	return OK;
        }
        else
        {
        	//dbg("rsp status: %x\r\n",rfMux.rxBuf[2]);
        	return ERR;
        }
    }
    else
    {
        dbg("overtime\r\n");
        return ERR;
    }

}
/*****************************************************************************
 �� �� ��  : rf_init
 ��������  : RFID��ʼ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��7��13�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8 rf_init(void)
{
    uint8 times = 1;
	memset(&rfMux,0,sizeof(rfMux));
	while(times-- != 0)
	{	// �������ո񣬳�ʼ������ͨ��
		Uart4_Send_Data(" ",1);   //0x20
		Delay_us(40); //40us
		Uart4_Send_Data(" ",1);
		Delay_ms(200);//20ms
		if(rfMux.rxBuf[0] == 0x06)
		{
		    dbg("rfMux.rxBuf[0]: %d\r\n",rfMux.rxBuf[0]);
			break;
		}
		Delay_ms(1000);
	}
	times = 2;
	while(times-- != 0)
	{
		if(rf_sendCmd(RF_HEAD_C1A,NULL) == OK)
		{
		     //dbg("RF_HEAD_C1A ok r\n");
			if(rf_sendCmd(RF_HEAD_C1E1,C1E_info) == OK)
			{
			    //dbg("RF_HEAD_C1E1 ok\r\n");
				return OK;
			}
			else
			{
			     dbg("RF_HEAD_C1E1 err\r\n");
			     return ERR;
			}
		}
		else
		{
		     dbg("RF_HEAD_C1A err\r\n");
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
    {   /* ��⵽ģ�� �����Զ����ģʽ */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
        //��ȡ����Ƭ������
        {
            //dbg("RF_HEAD_C2N ok\r\n");
            rf_reInitRx(1);
        }
        else
        {   /* ���RFģ����� */
           //dbg("RF_HEAD_C2N err\r\n");
        }
    }
    else
    {
        dbg("rf_init err\r\n");
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
                   uint32 rfRxtime = sys_ticks() + 200;
                    if(tick_timeout(rfRxtime))
                    {
                        RFID_STATE = STATE_RFID_TIME;
                    }
                }
                else
                {
                    rfMux.status = 0;
                    dbg("card leave");
                    memset(&rfMux.devInfo,0,sizeof(rfMux.devInfo));
                    // �����ߣ��ٴν����Զ�ģʽ
                    if(OK == rf_sendCmd(RF_HEAD_C2N,C2N_info))
                    {
                        rf_reInitRx(1);
                        RFID_STATE = STATE_RFID_IDLE;
                        dbg("STATE IDLE");
                    }
                }
                break;
            }
        case STATE_RFID_CHKCARD:
            {
                if(rfMux.rPkt.infoLen == 0x19)
                {
                    memcpy(&(rfMux.devInfo),rfMux.rxBuf+9,20);
                    dbg("uid: %X",rfMux.devInfo.uid.u);
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
                     dbg("uid: %X",rfMux.devInfo.uid.u);
                     com3_tx_rsp(0,UartRx.frameLen+4);
                }
                else
                {
                    dbg("read uid err!");//
                    com3_tx_rsp(1,UartRx.frameLen);
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
                    if(OK == rf_sendCmd(RF_HEAD_C2E,(uint8 *)&info)) //��Կ��֤
                    {
                        if(OK == rf_sendCmd(RF_HEAD_C2H,(uint8 *)&UartRx.info))
                        {
                            dbg("write OK!");
                            com3_tx_rsp(0,UartRx.frameLen-17);
                        }
                        else
                        {
                            dbg("write err!");
                            com3_tx_rsp(1,UartRx.frameLen-17);
                        }
                    }
                    else
                    {
                       dbg("password err in write");
                    }
                }
                else
                {
                    com3_tx_rsp(1,UartRx.frameLen-17);
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
                    info.mode = 0x60;                           //0x60������ԿA
                    memcpy(info.uid,rfMux.devInfo.uid.uch,4);   //�����кţ�4 �ֽڣ�
                    info.keyid = 1;                             //��Կ���ţ�1 �ֽڣ��� ȡֵ��Χ0��7
                    info.blkid = UartRx.info[0];
                    /*����ţ�1 �ֽڣ���
                        S50��0��63��
                        S70��0��255��
                        PLUS CPU 2K��0��127��
                        PLUS CPU 4K��0��255��
                    */
                    if(OK == rf_sendCmd(RF_HEAD_C2E,(uint8 *)&info)) //��Կ��֤
                    {
                        info_w.blkid = UartRx.info[0];
                        if(OK == rf_sendCmd(RF_HEAD_C2G,(uint8 *)&info_w.blkid))
                        {
                            memcpy(&(UartRx.info),rfMux.rPkt.info,16);
                            dbg("info:");
                            dbg_hex(rfMux.rPkt.info,16);
                            com3_tx_rsp(0,UartRx.frameLen+15);
                        }
                        else
                        {
                            dbg("read err!");
                            com3_tx_rsp(1,UartRx.frameLen-1);
                        }

                    }
                    else
                    {
                        dbg("password err in read");
                    }
                }
                else
                {
                    com3_tx_rsp(1,UartRx.frameLen-1);
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
       case 0x01: //��Ƭ��ز�������
       {
            switch (UartRx.cmdOrSta)
            {
               case 0xA1: //���������������֤��Կ
               {
                   RFID_STATE = STATE_RFID_READCARD;
					break;
               }
               case 0xA2: //ԭ��Ϊע�Ῠ����°����ȡ���˴�����
               {

                    break;
               }
               case 0xA3: //��ָ�����ݿ������֤KEYA
               {
                   RFID_STATE = STATE_RFID_READDAT;
                    break;
               }
               case 0xA4: //дָ�����ݿ������֤KEYA
               {
                   RFID_STATE = STATE_RFID_WRITEDAT;
                    break;
               }
               case 0xA5: //����IC����Կ����,��֤KEYA
               {
                   //RFID_STATE = STATE_RFID_READDAT;
                    break;
               }
               case 0xA6: // //Ǯ����ʼ�������֤KEYA
               {
                    RFID_STATE = STATE_WALLET_INIT;
                    break;
               }
               case 0xA7:  //Ǯ���ۿ������֤KEYA
               {
                    RFID_STATE = STATE_WALLET_DEC;
                    break;
               }
               case 0xA8:  //Ǯ����ֵ�����֤KEYA
               {
                   RFID_STATE = STATE_WALLET_INC;
                    break;
               }
               case 0xA9: //Ǯ������ѯ�����֤KEYA
               {
                    RFID_STATE = STATE_WALLET_BALANCE;
                    break;
               }
               default: //����
               {

                    break;
               }
            }
            break;
       }
       case 0x02: //��д��������ѯ
       {

            break;
       }
       case 0x03: //��д����������
       {

            break;
       }
       default: //����
       {

            break;
       }

    }

}

/*****************************************************************************
 �� �� ��  : com3_tx_rsp
 ��������  : com3Ӧ����
 �������  : uint8 sta  0-ok��1-err
             uint8 len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��8��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void com3_tx_rsp(uint8 sta,uint8 len)
{
    UartRx.cmdOrSta = sta; //�ɹ�
    UartRx.frameLen = len;
    UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
    UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
    com_send(COM3,UartRx.rxBuf,UartRx.frameLen); //�жϷ���
}

void com3_rxDeal(void)
{
	char ch;
	static uint8 index = 0;
	if(com_rxLeft(COM3) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM3,&ch))
			{
                rxbuf[index++]= ch;
                if((rxbuf[index-1] == 0x03)&&(index == rxbuf[0]))
                {
                    index -=2;// ָ��bcc
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
			else
			{
				break;
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
            rfMux.rIdx -= 2;    // ָ��bcc
            if(rf_bccCalc(rfMux.rxBuf,rfMux.rIdx) == rfMux.rxBuf[rfMux.rIdx])
            {
                 rfMux.frameOK =1; //������һ֡����
                if(rfMux.mode == 1)
                {
                    RFID_STATE =STATE_RFID_CHKCARD;
                    //�����ϱ�����⵽ˢ��
                }
            }
        }
    }
}


