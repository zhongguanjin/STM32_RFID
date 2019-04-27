#ifndef _ML_FPM_H
#define _ML_FPM_H

#include "config.h"
#include "com.h"
#define ML_BUF_MAX  32

#define ml_com   COM4





typedef struct
{
     uint8  Head[8];//8byte 帧头
     uint8  Info[24];
} mlPkt_t;


typedef struct
{
	union
	{
 		uint8 rxBuf[ML_BUF_MAX];
 		mlPkt_t mPkt;
	};
	uint8	rIdx;		// 接收位置索引
	uint8	frameOK;	// 帧接收完整标记: 1--完整，0--没有
	uint8	status;		// 0--没有匹配到指纹，1--匹配到指纹
	UN16    ID;         //指纹id号
} mlMux_t;








enum
{
    ML_ST_IDLE=0,
    ML_ST_SIGN,     //注册状态
    ML_ST_VERIFY,
    ML_ST_WAIT,
    ML_ST_CLEAR,    //指纹清除
    ML_ST_RST,     //复位
    ML_ST_ERR,
    ML_ST_MAX
};





extern void ml_st_set(uint8 st);
extern void MlFpm_Init(void);
extern void mlcom_Deal(uint8 dat);
extern void mlst_task(void);    //200ms
#endif /* ML_FPM_H */


