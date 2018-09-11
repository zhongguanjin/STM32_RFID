#include "Syn6658.h"
#include "string.h"
#include  "dbg.h"
#include "com.h"
#define SYN_BUF_MAX  128

char code_text[] = {"��ӭʹ��RFID��д��"};
uint8 length;

uint8 rot_val;//��ת����

#define Init_Ok_val   0x4A   //��ʼ���ɹ��ش�
#define Frame_Ok_val  0x41   //�յ���ȷ������֡�ش�
#define Frame_Err_val  0x45  //�յ�����ʶ������֡�ش�
#define Play_St_val  0x4E //оƬ����״̬�ش�
#define Idle_St_val  0x4F //оƬ����״̬�ش�





typedef  union
{
      struct
      {
        uint8  frameHead;
        uint8  lenH;
        uint8  lenL;
        uint8  cmdType;
        uint8  cmdval;
        uint8  info[SYN_BUF_MAX-5];   //123 byte
      };
      uint8 Buf[SYN_BUF_MAX];
} SynMux_t;


SynMux_t  SynMux;


/*****************************************************************************
 �� �� ��  : Syn6558_Play
 ��������  : �����ϳɲ�������
 �������  : char *buf --���ϳɵ��ı�
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Play(char *buf)
{
	    uint8 i=0;
      SynMux.cmdType = 0x01;
      SynMux.cmdval = 0x00;
      length = strlen(code_text); //��Ҫ�����ı��ĳ���
       SynMux.lenL= length+2;
      for(i=0;i<length;i++)
      {
          SynMux.info[i] = buf[i];
      }
      com_send(COM2,SynMux.Buf, SynMux.lenL+3); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : Syn6558_Stop
 ��������  : ֹͣ�ϳ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Stop(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x02;
    com_send(COM2,SynMux.Buf, SynMux.lenL+3); //�жϷ���
}
/*****************************************************************************
 �� �� ��  : Syn6558_Pause
 ��������  : ��ͣ�ϳ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Pause(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x03;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : Syn6558_Query
 ��������  : оƬ״̬��ѯ����
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Query()
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x21;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : Syn6558_Standby
 ��������  : оƬ����Standbyģʽ����
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Standby(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x22;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //�жϷ���
}
/*****************************************************************************
 �� �� ��  : Syn6558_Awaken
 ��������  : оƬ��������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Awaken(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0xFF;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //�жϷ���
}

/*****************************************************************************
 �� �� ��  : Syn6558_Recover
 ��������  : �ָ��ϳ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6558_Recover(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x04;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //�жϷ���
}

void Syn6558_Init(void)
{
  SynMux.frameHead = 0xfd;

  dbg("rot_val:%d",rot_val);
  Syn6558_Awaken();

}

void com2_rxDeal(void)
{
	char ch;
	if(com_rxLeft(COM2) != 0)
	{
		while(1)
		{
			if(OK == com_getch(COM2,&ch))
			{
                rot_val=ch;
                dbg("rot_val:%d",rot_val);
			}
			else
			{
				break;
			}
		}
	}
}

