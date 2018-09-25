#include "Syn6658.h"
#include "string.h"
#include  "dbg.h"
#include "com.h"
#define SYN_BUF_MAX  128

char code_text[] = {"��ӭʹ�ö�д��"};
uint8 length;

uint8 rot_val;//��ת����

#define Init_Ok_val   0x4A   //��ʼ���ɹ��ش�
#define Frame_Ok_val  0x41   //�յ���ȷ������֡�ش�
#define Frame_Err_val  0x45  //�յ�����ʶ������֡�ش�
#define Play_St_val  0x4E //оƬ����״̬�ش�
#define Idle_St_val  0x4F //оƬ����״̬�ش�

#define stopcmd         0x02    //ֹͣ�ϳ�����
#define pausecmd        0x03    //��ͣ�ϳ�����
#define querycmd        0x21   //״̬��ѯ
#define standbycmd      0x22   //��������
#define awakencmd       0xFF   //��������
#define recovercmd      0x04   //�ָ��ϳ�����

uint8 Syn6658_Init(void);


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
 �� �� ��  : Syn6658_Play
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
void Syn6658_Play(char *buf)
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
 �� �� ��  : Syn6658_Cmd
 ��������  : ��������
 �������  : uint8 cmd
             stopcmd         0x02    //ֹͣ�ϳ�����
             pausecmd        0x03    //��ͣ�ϳ�����
             querycmd        0x21   //״̬��ѯ
             standbycmd      0x22   //��������
             awakencmd       0xFF   //��������
             recovercmd      0x04   //�ָ��ϳ�����
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��9��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Syn6658_Cmd(uint8 cmd)
{
     SynMux.lenL =0x01;
     SynMux.cmdType = cmd;
    com_send(COM2,SynMux.Buf, SynMux.lenL+3); //�жϷ���
}



uint8 Syn6658_Init(void)
{
  uint8 times = 5;
  SynMux.frameHead = 0xfd;
  Syn6658_Cmd(awakencmd);
  Delay_ms(1000);
  while(times-- != 0)
  {
    if(rot_val==Frame_Ok_val)
    {
        return OK;
    }
  }
  return ERR;
}

void syn6658_check(void)
{
    if(Syn6658_Init() == OK)
    {
        Syn6658_Play(code_text);
        dbg("syn6658 init ok");
    }
    else
    {
        dbg("syn6658 init err");
    }
}

uint8 get_rspcmd(uint8 dat)
{
    rot_val=dat;
    return rot_val;
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
                dbg("rot_val:%x",rot_val);
			}
			else
			{
				break;
			}
		}
	}
}

