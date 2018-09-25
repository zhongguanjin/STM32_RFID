#include "Syn6658.h"
#include "string.h"
#include  "dbg.h"
#include "com.h"
#define SYN_BUF_MAX  128

char code_text[] = {"欢迎使用读写器"};
uint8 length;

uint8 rot_val;//回转数据

#define Init_Ok_val   0x4A   //初始化成功回传
#define Frame_Ok_val  0x41   //收到正确的命令帧回传
#define Frame_Err_val  0x45  //收到不能识别命令帧回传
#define Play_St_val  0x4E //芯片播音状态回传
#define Idle_St_val  0x4F //芯片空闲状态回传

#define stopcmd         0x02    //停止合成请求
#define pausecmd        0x03    //暂停合成请求
#define querycmd        0x21   //状态查询
#define standbycmd      0x22   //待机请求
#define awakencmd       0xFF   //唤醒请求
#define recovercmd      0x04   //恢复合成请求

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
 函 数 名  : Syn6658_Play
 功能描述  : 语音合成播放命令
 输入参数  : char *buf --待合成的文本
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6658_Play(char *buf)
{
	    uint8 i=0;
      SynMux.cmdType = 0x01;
      SynMux.cmdval = 0x00;
      length = strlen(code_text); //需要发送文本的长度
       SynMux.lenL= length+2;
      for(i=0;i<length;i++)
      {
          SynMux.info[i] = buf[i];
      }
      com_send(COM2,SynMux.Buf, SynMux.lenL+3); //中断发送
}

/*****************************************************************************
 函 数 名  : Syn6658_Cmd
 功能描述  : 命令请求
 输入参数  : uint8 cmd
             stopcmd         0x02    //停止合成请求
             pausecmd        0x03    //暂停合成请求
             querycmd        0x21   //状态查询
             standbycmd      0x22   //待机请求
             awakencmd       0xFF   //唤醒请求
             recovercmd      0x04   //恢复合成请求
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6658_Cmd(uint8 cmd)
{
     SynMux.lenL =0x01;
     SynMux.cmdType = cmd;
    com_send(COM2,SynMux.Buf, SynMux.lenL+3); //中断发送
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

