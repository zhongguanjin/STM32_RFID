#include "Syn6658.h"
#include "string.h"
#include  "dbg.h"
#include "com.h"
#define SYN_BUF_MAX  128

char code_text[] = {"欢迎使用RFID读写器"};
uint8 length;

uint8 rot_val;//回转数据

#define Init_Ok_val   0x4A   //初始化成功回传
#define Frame_Ok_val  0x41   //收到正确的命令帧回传
#define Frame_Err_val  0x45  //收到不能识别命令帧回传
#define Play_St_val  0x4E //芯片播音状态回传
#define Idle_St_val  0x4F //芯片空闲状态回传





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
 函 数 名  : Syn6558_Play
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
void Syn6558_Play(char *buf)
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
 函 数 名  : Syn6558_Stop
 功能描述  : 停止合成命令
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6558_Stop(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x02;
    com_send(COM2,SynMux.Buf, SynMux.lenL+3); //中断发送
}
/*****************************************************************************
 函 数 名  : Syn6558_Pause
 功能描述  : 暂停合成命令
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6558_Pause(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x03;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //中断发送
}

/*****************************************************************************
 函 数 名  : Syn6558_Query
 功能描述  : 芯片状态查询命令
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6558_Query()
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x21;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //中断发送
}

/*****************************************************************************
 函 数 名  : Syn6558_Standby
 功能描述  : 芯片进入Standby模式命令
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6558_Standby(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x22;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //中断发送
}
/*****************************************************************************
 函 数 名  : Syn6558_Awaken
 功能描述  : 芯片唤醒命令
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6558_Awaken(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0xFF;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //中断发送
}

/*****************************************************************************
 函 数 名  : Syn6558_Recover
 功能描述  : 恢复合成命令
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年9月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Syn6558_Recover(void)
{
      SynMux.lenL =0x01;
    SynMux.cmdType = 0x04;
    com_send(COM2,SynMux.Buf,  SynMux.lenL+3); //中断发送
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

