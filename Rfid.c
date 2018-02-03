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

uint8 Wallet_init_flg;  //Ǯ����ʼ����־

const u8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1
const u8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};	// def. key1 for blk1
const u8 C2M_info[]={0x00,0x26};
const u8 C2O_info[] ={0x00};
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
	//֡�����벻С��6 �ֽڣ�����ܳ���70 �ֽڣ���֡�����������Ϣ���ȼ�6��
	rfSend.frameLen = rfSend.infoLen + 6;
	//У��ͣ���FrameLen ��ʼ��Info �����һ�ֽ����ȡ��
	rfSend.info[len++] = rf_bccCalc((u8 *)&rfSend,(rfSend.infoLen+4));
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
    {   /* ��⵽ģ�� �����Զ����ģʽ */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
        //��ȡ����Ƭ������
        {
            //printf("RF_HEAD_C2N ok\r\n");
            rf_reInitRx(1);
        }
        else
        {   /* ���RFģ����� */
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
                    // �����ߣ��ٴν����Զ�ģʽ
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
                     UartRx.cmdOrSta = 0; //�ɹ�
                     UartRx.frameLen += 4;
                     UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                     UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                     Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                     memset(&(UartRx.rxBuf),0,32);
                }
                else
                {
                     printf("card leave!");//���뿪
                     UartRx.cmdOrSta = 1; //ʧ��
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
                    if(OK == rf_sendCmd(RF_HEAD_C2E,(u8 *)&info)) //��Կ��֤
                    {
                        //info_w.blkid = UartRx.info[0];
                       // memcpy(info_w.dat,&UartRx.info[1],16);
                        if(OK == rf_sendCmd(RF_HEAD_C2H,(u8 *)&UartRx.info))
                        {
                            printf("write OK!");
                            UartRx.cmdOrSta = 0; //�ɹ�
                            UartRx.frameLen = UartRx.frameLen-17;
                            UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                            UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                            Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                            memset(&(UartRx.rxBuf),0,32);

                        }
                        else
            {
                            printf("write err!");
                            UartRx.cmdOrSta = 1; //ʧ��
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
                     printf("card leave!");//���뿪
                     UartRx.cmdOrSta = 1; //ʧ��
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
                    if(OK == rf_sendCmd(RF_HEAD_C2E,(u8 *)&info)) //��Կ��֤
                    {
                        info_w.blkid = UartRx.info[0];
                        if(OK == rf_sendCmd(RF_HEAD_C2G,(u8 *)&info_w.blkid))
                        {
                            printf("read OK!");
                            memcpy(&(UartRx.info),rfMux.rPkt.info,16);
                            UartRx.cmdOrSta = 0; //�ɹ�
                            UartRx.frameLen = UartRx.frameLen+15;

                            UartRx.rxBuf[UartRx.frameLen-2] = rf_bccCalc(UartRx.rxBuf, UartRx.frameLen-2);
                            UartRx.rxBuf[UartRx.frameLen-1] = 0x03;
                            Uart3_Send_Data(UartRx.rxBuf, UartRx.frameLen);
                            memset(&(UartRx.rxBuf),0,32);

                        }
                        else
                        {
                            printf("[%s]write err!",__FUNCTION__);
                            UartRx.cmdOrSta = 1; //ʧ��
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
                     printf("card leave!");//���뿪
                     UartRx.cmdOrSta = 1; //ʧ��
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

void Uart_Receive_Process(void)
{
	static uint8 index = 0;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
        USART_ClearFlag(USART3, USART_FLAG_RXNE | USART_FLAG_ORE);
        rxbuf[index++]= USART_ReceiveData(USART3);
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


