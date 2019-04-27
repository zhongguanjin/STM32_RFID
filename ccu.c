#include "ccu.h"

#include "rfid.h"


ccubuf_t ccubuf;
uint8 rxbuf[32];
void  ccu_cmdDeal(void);
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
void com3_tx_rsp(ccubuf_t ptx,uint8 sta,uint8 len)
{
    ptx.cmdOrSta = sta; //�ɹ�
    ptx.frameLen = len;
    ptx.rxBuf[ptx.frameLen-2] = rf_bccCalc(ptx.rxBuf, ptx.frameLen-2);
    ptx.rxBuf[ptx.frameLen-1] = 0x03;
    com_send(ccu_com,ptx.rxBuf,ptx.frameLen); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : com3_rxDeal
 ��������  : ccu��������֡
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��11��7��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
                    index -=2;// ָ��bcc
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
 �� �� ��  : ccu_cmdDeal
 ��������  : ccu�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��11��7��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void ccu_cmdDeal(void)
{
    switch(ccubuf.cmdType)
    {
       case 0x01: //��Ƭ��ز�������
       {
            switch (ccubuf.cmdOrSta)
            {
               case 0xA1: //���������������֤��Կ
               {
                    //RFID_STATE = STATE_RFID_READCARD;
                    rf_state_set(STATE_RFID_READCARD);
					break;
               }
               case 0xA2: //ԭ��Ϊע�Ῠ����°����ȡ���˴�����
               {

                    break;
               }
               case 0xA3: //��ָ�����ݿ������֤KEYA
               {
                    //RFID_STATE = STATE_RFID_READDAT;
                    rf_state_set(STATE_RFID_READDAT);
                    break;
               }
               case 0xA4: //дָ�����ݿ������֤KEYA
               {
                   //RFID_STATE = STATE_RFID_WRITEDAT;
                   rf_state_set(STATE_RFID_WRITEDAT);
                    break;
               }
               case 0xA5: //����IC����Կ����,��֤KEYA
               {
                   //RFID_STATE = STATE_RFID_READDAT;
                    //rf_state_set(STATE_RFID_WRITEDAT);
                    break;
               }
               case 0xA6: // //Ǯ����ʼ�������֤KEYA
               {
                    //RFID_STATE = STATE_WALLET_INIT;
                     rf_state_set(STATE_WALLET_INIT);
                    break;
               }
               case 0xA7:  //Ǯ���ۿ������֤KEYA
               {
                    //RFID_STATE = STATE_WALLET_DEC;
                     rf_state_set(STATE_WALLET_DEC);
                    break;
               }
               case 0xA8:  //Ǯ����ֵ�����֤KEYA
               {
                   //RFID_STATE = STATE_WALLET_INC;
                    rf_state_set(STATE_WALLET_INC);
                    break;
               }
               case 0xA9: //Ǯ������ѯ�����֤KEYA
               {
                    //RFID_STATE = STATE_WALLET_BALANCE;
                     rf_state_set(STATE_WALLET_BALANCE);
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


