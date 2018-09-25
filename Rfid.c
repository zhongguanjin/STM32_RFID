#include "config.h"
#include "com.h"
#include "rfid.h"
#include "dbg.h"
#include "bsp_i2c.h"
#include<stdlib.h>
#include<string.h>
#include <stdio.h>

#include "Syn6658.h"

const uint8 C1E_info[]={0x60,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	// for key1
const uint8 C2N_info[]={0x03,0x03,0x26,0x45,0x60,0x01,0x08};	// def. key1 for blk1
const uint8 C2M_info[]={0x00,0x26};
const uint8  C2O_info[] ={0x00};

uint32 overticks;
rfMux_t rfMux;
uint8 rxbuf[32];
Uartrx_t UartRx;

uint8 RFID_STATE;


void rf_reInitRx(uint8 mode);
int16 rf_sendCmd(uint32 cmdHead, const uint8 * info);
uint8 rf_init(void);
uint8 rf_wait( void );
void com3_tx_rsp(uint8 sta,uint8 len);
void Rf_SetOverTicks(u32 time);
void rf_over_check(void);


#if USER_TEST

typedef struct
{
	uint8 cnt;             /*�û���	ָʾ���������û�������������*/
	uint8 crc;             /*CRC	���������û����ݵ�CRCУ��ֵ*/
	uint8 serial[5][4];     /*�û�n	�洢�û�RFID��*/
}T_RFUSER;

T_RFUSER grfUser; /*�û���Ϣ*/
#define RFUSER_MAX 	5 /*����5���û�*/
#define ADDR_RFUSER 0x0000

/*�жϿ��Ƿ���Ч������1��Ч��0��Ч*/
int rfUsr_isRfok(u8 *buf)
{
	int loop=grfUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&grfUser.serial[loop][0], 4) == NULL)
		{/*�ҵ���ͬ����*/
			return 1;
		}
	}
	return 0;
}
/*����1����*/
int rfUsr_isRfFull(void)
{
	if(grfUser.cnt>=RFUSER_MAX)
	{
	    dbg("user full");
		return 1;
	}
	return 0;
}

/*����һ��������*/
int rfUsr_append(u8 *buf)
{
    if(rfUsr_isRfFull()==OK)
    {
        return 0;
    }
	memcpy((u8 *)(&grfUser.serial[grfUser.cnt][0]), buf, 4);
	grfUser.cnt++;
	grfUser.crc =0xff;
	EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
	dbg("save rf serial[%d]",grfUser.cnt);
	return 1;
}

void rfUsr_showRfSerial(void)
{
	int loop=0;
	dbg("crc::%04X",grfUser.crc);
	while(loop < grfUser.cnt)
	{
		dbg("id:%d, serial:%02X %02X %02X %02X",loop,
		grfUser.serial[loop][0],grfUser.serial[loop][1],grfUser.serial[loop][2],grfUser.serial[loop][3]);
		loop++;
	}
	return;
}
void rfUsr_setDefault(void)
{
	memset((u8 *)(&grfUser),0, sizeof(T_RFUSER));
	grfUser.crc = 0xff;
	EEPROM_Write(ADDR_RFUSER, (u8 *)(&grfUser),sizeof(T_RFUSER));
	return;
}

void rfUsr_init(void)
{
	EEPROM_Read(ADDR_RFUSER,(u8 *)(&grfUser),sizeof(T_RFUSER));
	dbg("--rf user cnt::%d---",grfUser.cnt);
	if(grfUser.cnt > RFUSER_MAX)
	{
		dbg("cnt fail");
		rfUsr_setDefault();
		return;
	}
	{
		u16 crc = 0xff;
		if(grfUser.crc != crc)
		{
			dbg("crc fail:0x%04X != 0x%04X",crc,grfUser.crc);
		//	rfUsr_setDefault();
		}
		else
		{
		    rfUsr_showRfSerial();
		}
	}
	return;
}

#else


#define user_len 5
#define user_addr   0x0000   //0x0000-0x0014

static  uint32 user_buf[user_len]; //������ջ����
int top = -1;
/*

user_buf[0] =0x01020304  ������0x0000 - 0x0003  �ȵͺ�� 04 03 02 01
....
user_buf[4] =0x01020304  ������0x0010 - 0x0013  �ȵͺ�� 04 03 02 01

*/

//���
uint8 add_rfid_user(void)
{
    uint32 uid =rfMux.devInfo.uid.u;
    UN32  bak_user;
    if((top == user_len-1)||(uid == 0)||(check_rfid_user(uid) == OK)) //��,�޿����ظ���
    {
        //user_buf[top] = uid;
        return ERR;
    }
    top += 1;
    user_buf[top] = uid;
    bak_user.u =  user_buf[top];
    EEPROM_Write(user_addr+top*4, bak_user.uch, 4);
    return OK;
}

//���
uint8 clear_rfid_user(void)
{
    UN32  bak_user;
    if(top == -1)  //��
    {
        return ERR;
    }
    else
    {
		uint8 i;
        for( i=0;i<=top;i++)
        {
           user_buf[i]=0;
           bak_user.u =  user_buf[i];
           EEPROM_Write(user_addr+i*4, bak_user.uch, 4);
        }
        top = -1;
	   return OK;
    }
}

/* ɾ�� */
uint8 del_rfid_user(void)
{
    uint32 uid = rfMux.devInfo.uid.u;
    UN32  bak_user ;
    if((top == -1)||(uid == 0)||(check_rfid_user(uid) == ERR)) //��,�޿�
    {
        return ERR;
    }
    /*
    if(user_buf[top] == uid) //��ջ��Ԫ��
    {
        user_buf[top] = 0; //����
        bak_user.u =  user_buf[top];
        EEPROM_Write(user_addr+top*4, bak_user.uch, 4);
        top -= 1;
	  return OK;
    }
    */
    else
    {
		uint8 i;
        for( i=0;i<=top;i++)
        {
            if(user_buf[i] == uid)//
            {
				uint8 j;
                for(j=i; j<top;j++) //��i��������Ԫ����ǰ��
                {
                    user_buf[j]=user_buf[j+1];
                    bak_user.u =  user_buf[j];
                    EEPROM_Write(user_addr+j*4, bak_user.uch, 4);

                }
                user_buf[top] = 0; //����
                bak_user.u =  user_buf[top];
                EEPROM_Write(user_addr+top*4, bak_user.uch, 4);
                top -= 1;  //��������ջ����ż�1
            }
        }
        return OK;
    }
}

//��ѯ
uint8 query_rfid_user(uint32* buf)
{
    uint8 i;
    for( i=0;i<=top;i++)
    {
       buf[i]=user_buf[i];
    }
    return i;
}
//����
uint8 check_rfid_user(uint32 uid)
{
    uint8 i;
    for( i=0;i<=top;i++)
    {
        if((uid == user_buf[i])&&(uid != 0))
        {
           return OK;
        }
    }
    return ERR;
}
void rfid_user_init(void)
{
    UN32 bak_user;
    uint8 i;
    for(i=0;i<user_len;i++) //��ʼ����ȡ�ڴ�user ֵ��
    {
        EEPROM_Read(user_addr+i*4, bak_user.uch,4);
        if(bak_user.u != 0)
        {
            user_buf[i]=bak_user.u;
            top++;
            if(top>4)  //��ֹ�������
            {
                top=4;
            }
            dbg("user%d:0x%X",i,user_buf[i]);
        }
    }
}



#endif






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
            dbg("rfMux.frame_err");
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
		    dbg("rfMux.rxBuf[0]: %d",rfMux.rxBuf[0]);
			break;
		}
		Delay_ms(1000);
	}
	times = 2;
	while(times-- != 0)
	{
		if(rf_sendCmd(RF_HEAD_C1A,NULL) == OK)
		{
			if(rf_sendCmd(RF_HEAD_C1E1,C1E_info) == OK)
			{
			    dbg("RF_HEAD_C1E1 ok");
				return OK;
			}
			else
			{
			     dbg("RF_HEAD_C1E1 err");
			     return ERR;
			}
		}
		else
		{
		     dbg("RF_HEAD_C1A err");
		     return ERR;
		}
	}
	return ERR;
}

void rf_over_check(void)
{
	rf_reInitRx(0);
    if( rf_init() == OK)
    {   /* ��⵽ģ�� �����Զ����ģʽ */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
        //��ȡ����Ƭ������
        {
            dbg("RF_HEAD_C2N ok");
            rf_reInitRx(1);
        }
        else
        {    /* ���RFģ����� */
            dbg("RF_HEAD_C2N err");
        }
    }
    else
    {
        dbg("rf_init err");
    }
}

void rf_init_check(void)
{
    RFID_STATE =STATE_RFID_IDLE;
	rf_reInitRx(0);
    if( rf_init() == OK)
    {   /* ��⵽ģ�� �����Զ����ģʽ */
        if(rf_sendCmd(RF_HEAD_C2N,C2N_info) == OK)
        //���������ڿ�Ƭ���Զ���⣬ִ�и�����ɹ�����UART ģʽ�£�ģ�齫��������
        //��ȡ����Ƭ������
        {
            dbg("RF_HEAD_C2N ok");
        #if USER_TEST
            rfUsr_init();
        #else
            rfid_user_init();
        #endif
            rf_reInitRx(1);
        }
        else
        {    /* ���RFģ����� */
            dbg("RF_HEAD_C2N err");
        }
    }
    else
    {
        dbg("rf_init err");
    }
}
/*****************************************************************************
 �� �� ��  : get_rf_uid
 ��������  : ������uid
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��26��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32 get_rf_uid(void)
{
    return rfMux.devInfo.uid.u;
}

/*****************************************************************************
 �� �� ��  : read_rf_dat
 ��������  : ��rfid��ŵ�ֵ����
 �������  : uint8 blank
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��26��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void read_rf_dat(uint8 blank)
{
    rfC2H_t info_w;
    rfC2E_t info;
    if(OK == rf_sendCmd(RF_HEAD_C2M,C2M_info))
    {
        info.mode = 0x60;                           //0x60������ԿA
        memcpy(info.uid,rfMux.devInfo.uid.uch,4);   //�����кţ�4 �ֽڣ�
        info.keyid = 1;                             //��Կ���ţ�1 �ֽڣ��� ȡֵ��Χ0��7
        info.blkid = blank;
        /*����ţ�1 �ֽڣ���
            S50��0��63��
            S70��0��255��
            PLUS CPU 2K��0��127��
            PLUS CPU 4K��0��255��
        */
        if(OK == rf_sendCmd(RF_HEAD_C2E,(uint8 *)&info)) //��Կ��֤
        {
            info_w.blkid = blank;
            if(OK == rf_sendCmd(RF_HEAD_C2G,(uint8 *)&info_w.blkid))
            {
                dbg("info:");
                dbg_hex(rfMux.rPkt.info,16);
            }
            else
            {
                dbg("read err!");
            }
        }
        else
        {
            dbg("password err ");
        }
    }
    else
    {
         dbg("no card !");
    }
}










/*****************************************************************************
 �� �� ��  : Rfid_Task_Process
 ��������  : rfid������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��7��26��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Rfid_Task_Process()//200ms
{
    switch(RFID_STATE)
    {
        case STATE_RFID_IDLE:
            {
                if((overticks++)>=300)
                {
                   overticks=0;
                   rf_over_check();
                }
                break;
            }
        case STATE_RFID_TIME:
            {
                 static uint8 scan_time=0;
							   overticks=0;
                 if((scan_time++)>=2)//400ms
                 {
                     scan_time = 0;
                     if(ERR== rf_sendCmd(RF_HEAD_C2M,C2M_info))
                     {
                         rfMux.status = 0;
                         //dbg("card leave");
                         memset(&rfMux.devInfo,0,sizeof(rfMux.devInfo));
                         // �����ߣ��ٴν����Զ�ģʽ
                         if(OK == rf_sendCmd(RF_HEAD_C2N,C2N_info))
                         {
                             rf_reInitRx(1);
                             RFID_STATE = STATE_RFID_IDLE;
                         }
                     }
                 }
                break;
            }
        case STATE_RFID_CHKCARD:
            {
                overticks=0;
                if(rfMux.rPkt.infoLen == 0x19)
                {
                    memcpy(&(rfMux.devInfo),rfMux.rxBuf+9,20);
                    dbg("uid: 0x%X",rfMux.devInfo.uid.u);
                    rfMux.status = 1;
                 #if USER_TEST
                    if(rfUsr_isRfok(rfMux.devInfo.uid.uch)==OK)
                 #else
                    if(check_rfid_user(rfMux.devInfo.uid.u)==OK)
                 #endif
                    {
    					 char str[]={"��Ч��"};
                        BELL(ON);
                        Delay_ms(100);
                        BELL(OFF);
                        Delay_ms(100);
                        BELL(ON);
                        Delay_ms(100);
                        BELL(OFF);
                        Syn6658_Play(str);

                    }
                    else
                    {
                        char str[]={"��Ч������ˢ"};
                        Syn6658_Play(str);
                    }
                }
                RFID_STATE = STATE_RFID_TIME;
                break;
            }
        case STATE_RFID_READCARD:
            {
                overticks=0;
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
                rfC2E_t info;
                overticks=0;
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
                 overticks=0;
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
        default:
            {
                RFID_STATE = STATE_RFID_IDLE;
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


