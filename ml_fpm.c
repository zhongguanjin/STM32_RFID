
#include "ml_fpm.h"
#include "TiMbase.h"
#include "dbg.h"
#include "Syn6658.h"


#define FG_RES_CMD      0x0111       //指纹注册

#define FG_RES_REQ      0x0112       //查询注册结果

#define FG_SAVE_CMD     0x0113       //保存指纹

#define FG_SAVE_REQ     0x0114       //查询指纹保存结果

#define FG_CHECK_CMD    0x0121      //匹配指纹
#define FG_CHECK_REQ    0x0122      //查询指纹匹配结果

#define FG_CLEAR_CMD    0x0131      //指纹特征清除
#define FG_CLEAR_REQ    0x0132      //查询指纹特征清除结果

#define FG_EXIST_REQ    0x0133      //查询指纹ID 是否存在

//#define FG_PART_REQ     0x0134      //查询指纹特征存储分布

#define FG_GET_REQ      0x0135      //查询手指在位状态


#define FG_PWD_CMD      0x0201      //密码设置命令

#define FG_REST_CMD     0x0202      //复位指纹模块

#define ML_PULSE_CMD     0x0303     //心跳命令

//#define FG_ID_REQ      0x0301 //获取指纹模块ID

//0xF11FE22EB66BA88A
const uint8 FrameHead[] = {0xf1,0x1f,0xe2,0x2e,0xb6,0x6b,0xa8,0x8a};  //帧头

uint8  databuf[16]; //命令帧数据区
uint8 MlState;


mlMux_t   mlMux;

  char str_sign_ok[]={"注册成功"};
  char str_sign_err[]={"注册失败"};
  char str_verify_ok[]= {"谢谢"};
  char str_verify_err[]={"请重刷"};
  char str_overtime[]={"超时退出"};



enum
{
    SIGN_ST_IDLE=0,
    SIGN_ST_IN,     //进入注册流程
    SIGN_ST_CHK,    //手指在位检测
    SIGN_ST_QUE,    //注册结果查询
    SIGN_ST_SAVE    //注册结果保存
};

enum
{
  VERIFY_ST_IDLE=0,
  VERIFY_ST_PAIR,
  VERIFY_ST_QUE
};

enum
{
  CLEAR_ST_IDLE=0,
  CLEAR_ST_CMD,
  CLEAR_ST_REQ
};



void ml_reInitRx(void);


uint8 MlFpm_bcc(uint8 * buf,uint8 len);
int16 ml_sendCmd(uint16 cmd,uint8 datalen,uint8 *buf);
uint8 ml_wait(void);
uint8 ml_check(void);
uint8 ml_verify_flow(void);             //验证流程
uint8 fg_exist_check(void);             //查询手指在位
uint8 ml_sign_flow(void);               //注册流程

uint8 ml_fg_clear(void);


#define MLUSER_MAX 	5           /*限制5个用户*/
typedef struct
{
	uint8 cnt;             /*用户数	指示后面启用用户个数：０～５*/
	uint8 crc;             /*CRC	后面启用用户数据的CRC校验值*/
	uint8 mlid[MLUSER_MAX][2];     /*用户n	存储用户mlid*/
}T_MLUSER;

T_MLUSER gmlUser;               /*用户信息*/





#if 0
#define ADDR_MLUSER 0x0000

/*判断卡是否有效，返回1有效，0无效*/
int mlUsr_isRfok(u8 *buf)
{
	int loop=gmlUser.cnt;
	while(loop--)
	{
		if(memcmp(buf, (u8 *)&gmlUser.serial[loop][0], 4) == NULL)
		{/*找到相同数据*/
			return 1;
		}
	}
	return 0;
}
/*返回1，满*/
int mlUsr_isMlFull(void)
{
	if(gmlUser.cnt>=MLUSER_MAX)
	{
	    dbg("user full");
		return 1;
	}
	return 0;
}

/*增加一个到后面*/
int mlUsr_append(u8 *buf)
{
    if(rfUsr_isRfFull()==OK)
    {
        return 0;
    }
	memcpy((u8 *)(&gmlUser.serial[gmlUser.cnt][0]), buf, 4);
	gmlUser.cnt++;
	gmlUser.crc =0xff;
	EEPROM_Write(ADDR_MLUSER, (u8 *)(&gmlUser),sizeof(T_RFUSER));
	dbg("save ml serial[%d]",gmlUser.cnt);
	return 1;
}

void mlUsr_showMlSerial(void)
{
	int loop=0;
	dbg("crc::%04X",gmlUser.crc);
	while(loop < gmlUser.cnt)
	{
		dbg("id:%d, serial:%02X %02X %02X %02X",loop,
		gmlUser.serial[loop][0],gmlUser.serial[loop][1],gmlUser.serial[loop][2],gmlUser.serial[loop][3]);
		loop++;
	}
	return;
}
void mlUsr_setDefault(void)
{
	memset((u8 *)(&gmlUser),0, sizeof(T_RFUSER));
	gmlUser.crc = 0xff;
	EEPROM_Write(ADDR_MLUSER, (u8 *)(&gmlUser),sizeof(T_RFUSER));
	return;
}

void mlUsr_init(void)
{
	EEPROM_Read(ADDR_MLUSER,(u8 *)(&gmlUser),sizeof(T_RFUSER));
	dbg("--ml user cnt::%d---",gmlUser.cnt);
	if(gmlUser.cnt > MLUSER_MAX)
	{
		dbg("cnt fail");
		rfUsr_setDefault();
		return;
	}
	{
		u16 crc = 0xff;
		if(gmlUser.crc != crc)
		{
			dbg("crc fail:0x%04X != 0x%04X",crc,gmlUser.crc);
		//	rfUsr_setDefault();
		}
		else
		{
		    rfUsr_showRfSerial();
		}
	}
	return;
}

#endif
/*****************************************************************************
 函 数 名  : MlFpm_Init
 功能描述  : ml模块初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void MlFpm_Init(void)
{
    Delay_ms(500); //上电至少延时300ms
    ml_reInitRx();
    if(ml_check() == OK)
    {
       //有ml模块
       dbg("ml on-line");
       ml_st_set(ML_ST_IDLE);
    }
    else
    {
        //无ml模块
        ml_st_set(ML_ST_ERR);
        dbg("ml off-line");
    }
}
/*****************************************************************************
 函 数 名  : ml_check
 功能描述  : ml模块检测函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : ok - 模块在线，err - 模块离线
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 ml_check(void)
{
  	uint8 times = 2;
	while(times-- != 0)
	{
		if(ml_sendCmd(ML_PULSE_CMD,0,NULL) == OK)
		{
		     return OK;
		}
		else
		{
		     return ERR;
		}
	}
	return ERR;
}
/*****************************************************************************
 函 数 名  : ml_reInitRx
 功能描述  : ml接收标志初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void ml_reInitRx(void)
{
	mlMux.rIdx = 0;
	mlMux.frameOK = 0;
}
/*****************************************************************************
 函 数 名  : ml_wait
 功能描述  : ml等待接收完成函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 ml_wait( void )
{
    uint32 mlRxTicks = sys_ticks() + 1000;
	while(1)
	{
		Delay_ms(5);
        if(mlMux.frameOK == 1)
        {
            return OK;
        }
        if(tick_timeout(mlRxTicks))
        {
           return ERR;
        }
	}
}
/*****************************************************************************
 函 数 名  : ml_sendCmd
 功能描述  : 发送命令帧函数
 输入参数  :
         cmd :ml模块命令
         datalen:命令后面数据长度
         *buf:   数据->databuf[]
 输出参数  :
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年7月11日 星期二
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
// msb ->lsb
int16 ml_sendCmd(uint16 cmd,uint8 datalen,uint8 *buf)
{
	mlPkt_t mlSend;
	UN16 cmdbak;UN16 len;
	cmdbak.ush = cmd;
	len.ush=7+datalen;
	//head
	memcpy(mlSend.Head,FrameHead,8);        //8帧头
    //applen
	mlSend.Info[0] = len.uch[1];              //应用层长度
	mlSend.Info[1] = len.uch[0];
    //head crc
    mlSend.Info[2] = MlFpm_bcc((uint8 *)&mlSend, 10);//帧头crc
    //psw
    mlSend.Info[3] = 0;
    mlSend.Info[4] = 0;
    mlSend.Info[5] = 0;
    mlSend.Info[6] = 0;
    //cmd
    mlSend.Info[7] = cmdbak.uch[1];
    mlSend.Info[8] = cmdbak.uch[0];
    if(datalen !=0)
    {
        // cmd dat
        memcpy(&mlSend.Info[9],buf,datalen); //数据
    }
    //appinfo crc
    mlSend.Info[len.ush+2] = MlFpm_bcc(&mlSend.Info[3],len.ush-1); //app crc
    ml_reInitRx();
	com_send(ml_com,(uint8 *)&mlSend,len.ush+11); //发送命令帧数据
	if((MlState != ML_ST_IDLE)&&(MlState != ML_ST_WAIT))
	{
        dbg_hex((uint8 *)&mlSend,len.ush+11);
    }
    //for recv
    if(ml_wait() == OK)
    {
        UN32 errnum;
        errnum.uch[3] = mlMux.mPkt.Info[9];
        errnum.uch[2] = mlMux.mPkt.Info[10];
        errnum.uch[1] = mlMux.mPkt.Info[11];
        errnum.uch[0] = mlMux.mPkt.Info[12];
        if(errnum.u == 0x00000000)  //状态成功
        {   // 成功
            return OK;
        }
        else
        {
            dbg("err num->%x",errnum.u);
            return ERR;
        }
    }
    else
    {
        dbg("overtime\r\n");
        ml_st_set(ML_ST_ERR);
        return ERR;
    }
}

/*****************************************************************************
 函 数 名  : mlcom_Deal
 功能描述  : ml串口接收处理
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void mlcom_Deal(uint8 dat)
{
	UN16 len;
    mlMux.rxBuf[mlMux.rIdx++] =  dat;
    len.uch[1] = mlMux.rxBuf[8];
    len.uch[0] = mlMux.rxBuf[9];
    if(mlMux.rIdx == (len.ush+11))// 帧长=AppLen+11,
    {
        if((mlMux.rxBuf[10]== MlFpm_bcc(mlMux.rxBuf,10))
            &&(mlMux.rxBuf[mlMux.rIdx-1] == MlFpm_bcc(&mlMux.rxBuf[11],len.ush-1))) //crc ok
        {
            mlMux.frameOK = 1;
        }
    }

}
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
uint8 MlFpm_bcc(uint8 * buf,uint8 len)
{
	uint8 bcc = 0;
	while(len--)
	{
		bcc += *buf++;
	}
	bcc = (~bcc)+1;
	return bcc;
}
/*****************************************************************************
 函 数 名  : ml_st_set
 功能描述  : ml状态设置函数
 输入参数  : uint8 st
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年12月20日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void ml_st_set(uint8 st)
{
    if(st > ML_ST_MAX)
    {
        dbg("err");
        return;
    }
    MlState = st;
    dbg("st->%d",MlState);
}
/*****************************************************************************
 函 数 名  : mlst_task
 功能描述  : 魔力模块主任务
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年12月20日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void mlst_task(void)    //200ms
{
    switch ( MlState)
    {
        case ML_ST_IDLE:
            {
                if(fg_exist_check() == OK) //手指在位
                {
                    ml_st_set(ML_ST_VERIFY);
                }
            }
            break;
        case ML_ST_SIGN://注册
            {
                if(ml_sign_flow() == GoOn)
                {
                    ml_st_set(ML_ST_SIGN);
                }
                else
                {
                    ml_st_set(ML_ST_WAIT);
                }
            }
            break;
        case ML_ST_VERIFY://验证
            {
               if(ml_verify_flow() == GoOn)
               {
                   ml_st_set(ML_ST_VERIFY);
               }
               else
               {
                   ml_st_set(ML_ST_WAIT);
               }
            }
            break;
        case ML_ST_WAIT://等待手指放开
            {
                if(fg_exist_check() == OK) //手指在位
                {
                    ml_st_set(ML_ST_WAIT);
                }
                else
                {
                    ml_st_set(ML_ST_IDLE);
                }
            }
            break;
        case ML_ST_CLEAR:
            {
                if(ml_fg_clear() == GoOn)
                {
                    ml_st_set(ML_ST_CLEAR);
                }
                else
                {
                    ml_st_set(ML_ST_IDLE);
                }
            }
            break;
        case ML_ST_RST:
            {
                if(ml_sendCmd(FG_REST_CMD,0,NULL) == OK)
                {
                     ml_st_set(ML_ST_IDLE);
                }
                else
                {
                     ml_st_set(ML_ST_RST);
                }
            }
            break;
        case ML_ST_ERR:
            {
                static uint16 check_time =50;
                if((check_time--)==0)
                {
                    if(ml_check() == OK)
                    {
                       //有ml模块
                       dbg("ml on-line");
                       ml_st_set(ML_ST_IDLE);
                    }
                    else
                    {
                        //无ml模块
                        ml_st_set(ML_ST_ERR);
                        dbg("ml off-line");
                    }
                    check_time = 50;
                }
            }
            break;
        default:
            ml_st_set(ML_ST_IDLE);
            break;
    }
}

/*****************************************************************************
 函 数 名  : ml_fg_clear
 功能描述  : 指纹清除流程
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年12月20日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 ml_fg_clear(void)
{
    static uint8 ClearSt = CLEAR_ST_CMD;
    if(ClearSt == CLEAR_ST_CMD)
    {
        databuf[0] = 0x01;//删除全部指纹
        databuf[1]= 0;  //指纹id号
        databuf[2]= 0;
        if(ml_sendCmd(FG_CLEAR_CMD,3,databuf)==OK)
        {
            ClearSt =CLEAR_ST_REQ;
            return GoOn;
        }
        else
        {
            ClearSt =CLEAR_ST_CMD;
            return ERR;
        }
    }
    if(ClearSt == CLEAR_ST_REQ)
    {
       if(ml_sendCmd(FG_CLEAR_REQ,0,NULL) == OK)
       {
           ClearSt =CLEAR_ST_CMD;
           Syn6658_Play("清除成功");
           return OK;
       }
       else
       {
            if(mlMux.mPkt.Info[12] == 0x04)//errnum ->04
            {
                ClearSt =CLEAR_ST_REQ;//
                return GoOn;
            }
           ClearSt =CLEAR_ST_CMD;
           Syn6658_Play("清除失败");
           return ERR;
       }
    }
    return GoOn;
}
/*****************************************************************************
 函 数 名  : fg_exist_check
 功能描述  : 手指在位查询
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年12月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 fg_exist_check(void)
{
    if(ml_sendCmd(FG_GET_REQ, 0, NULL) == OK)//手指在位检测
    {
        if(mlMux.mPkt.Info[13] != 0)//在位
        {
            return OK;
        }
        else
        {
            return ERR;
        }
    }
    else
    {
        return ERR;
    }
}
/*****************************************************************************
 函 数 名  : ml_verify_flow
 功能描述  : 指纹验证流程函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月20日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 ml_verify_flow(void)
{
    static uint32 mlverifyTicks=0;
    static uint8 VerifySt = VERIFY_ST_PAIR;
    if(VerifySt == VERIFY_ST_PAIR )
    {
        mlverifyTicks = sys_ticks() + 10000;//10s
        if(ml_sendCmd(FG_CHECK_CMD, 0, NULL) == OK)//发送匹配命令
        {
            VerifySt=VERIFY_ST_QUE;
            return GoOn;
        }
        else
        {
            VerifySt=VERIFY_ST_PAIR;
            dbg("VERIFY_ST_PAIR err");
            return ERR;
        }
    }
    if(VerifySt == VERIFY_ST_QUE)
    {
        if(tick_timeout(mlverifyTicks))
        {
            VerifySt=VERIFY_ST_PAIR;
            dbg("VerifySt overtime");
            Syn6658_Play(str_overtime);
            return ERR;
        }
        if(ml_sendCmd(FG_CHECK_REQ, 0, NULL) == OK)//查询匹配结果
        {
            if(mlMux.mPkt.Info[14] != 0x01)//result ->01
            {
                VerifySt=VERIFY_ST_PAIR;//
                dbg("it is no sign,please sign it");
                Syn6658_Play("指纹不存在");
                return ERR;
            }
            VerifySt=VERIFY_ST_PAIR;
            dbg("VERIFY_ST_PAIR ok");
            dbg("id->%x,%x",mlMux.mPkt.Info[17],mlMux.mPkt.Info[18]);
            BELL(ON);
            Delay_ms(100);
            BELL(OFF);
            Syn6658_Play(str_verify_ok);
            return OK;
        }
        else
        {
            if(mlMux.mPkt.Info[12] == 0x04)//errnum ->04
            {
                VerifySt=VERIFY_ST_QUE;//
                return GoOn;
            }
            if(mlMux.mPkt.Info[12] == 0x05)//errnum ->05
            {
                if(ml_sendCmd(FG_CHECK_CMD, 0, NULL) == OK)//发送匹配命令
                {
                    VerifySt=VERIFY_ST_QUE;
                    return GoOn;
                }
                else
                {
                    VerifySt=VERIFY_ST_PAIR;
                    dbg("VERIFY_ST_PAIR err");
                    return ERR;
                }
            }
            VerifySt=VERIFY_ST_PAIR;//
            Syn6658_Play(str_verify_err);
            return ERR;
        }
    }
		return GoOn;
}
/*****************************************************************************
 函 数 名  : ml_sign_flow
 功能描述  : 指纹注册流程函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年11月20日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 ml_sign_flow(void)
{
    static uint32 mlsignTicks=0;
    static uint8 index = 1;
    static uint8 SignSt = SIGN_ST_IN;
    if(SignSt == SIGN_ST_IN)
    {
        mlsignTicks = sys_ticks() + 10000;//10s
        databuf[0] = index;
        if(ml_sendCmd(FG_RES_CMD, 1, databuf) == OK)//发送注册命令
        {
            SignSt=SIGN_ST_QUE;
            dbg("please push your fg ->");
            return GoOn;
        }
        else
        {
            SignSt=SIGN_ST_IN;
            dbg("SIGN_ST_IN err");
            return ERR;
        }
    }
    if(SignSt == SIGN_ST_QUE)
    {
        if(tick_timeout(mlsignTicks))
        {
            SignSt=SIGN_ST_IN;
            dbg("sign overtime");
            Syn6658_Play(str_overtime);
            return ERR;
        }
        if(ml_sendCmd(FG_RES_REQ, 0, NULL) == OK)//查询注册结果
        {
            dbg("proc->%d,index:%d",mlMux.mPkt.Info[15],index);
            if(mlMux.mPkt.Info[15] >= 100)//注册进度PROC 值为100
            {
                index=1;
                SignSt=SIGN_ST_SAVE;
                return GoOn;
            }
            else
            {
                index++;
                if(index > 6 )
                {
                    index=1;
                    SignSt=SIGN_ST_SAVE;
                    return GoOn;
                }
                databuf[0] = index;
                if(ml_sendCmd(FG_RES_CMD, 1, databuf) == OK)//发送注册命令
                {
                    SignSt=SIGN_ST_QUE;
                    dbg("please push your fg ->");
                    return GoOn;
                }
                else
                {
                    SignSt=SIGN_ST_IN;
                    index=1;
                    dbg("SIGN_ST_IN err");
                    return ERR;
                }
            }
        }
        else
        {
            if(mlMux.mPkt.Info[12] == 0x04)//errnum ->04
            {
                SignSt=SIGN_ST_QUE;//
                return GoOn;
            }
            if(mlMux.mPkt.Info[12] == 0x09)//errnum ->09
            {
                databuf[0] = index;
                if(ml_sendCmd(FG_RES_CMD, 1, databuf) == OK)//发送注册命令
                {
                    SignSt=SIGN_ST_QUE;
                    dbg("please push your fg ->");
                    return GoOn;
                }
                else
                {
                    SignSt=SIGN_ST_IN;
                    index=1;
                    dbg("SIGN_ST_IN err");
                    return ERR;
                }
            }
            SignSt=SIGN_ST_IN;
            dbg("SIGN_ST_QUE err");
            Syn6658_Play(str_sign_err);
            return ERR;
        }
    }
    if(SignSt == SIGN_ST_SAVE)
    {
        databuf[0]= mlMux.mPkt.Info[13];
        databuf[1]= mlMux.mPkt.Info[14];
        if(ml_sendCmd(FG_SAVE_CMD, 2, databuf) == OK)//保存指纹
        {
            gmlUser.mlid[gmlUser.cnt][0] = databuf[0];
            gmlUser.mlid[gmlUser.cnt][1] = databuf[1];
            gmlUser.cnt++;
            gmlUser.crc = 0xff;
            SignSt=SIGN_ST_IN;
            dbg("SIGN_ST_SAVE ok,id->%x,%x",databuf[0],databuf[1]);
            Syn6658_Play(str_sign_ok);
            return OK;
        }
        else
        {
            SignSt=SIGN_ST_IN;
            dbg("SIGN_ST_SAVE err");
            return ERR;
        }
    }
    return GoOn;
}
