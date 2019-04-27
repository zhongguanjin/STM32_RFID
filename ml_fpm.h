#ifndef _ML_FPM_H
#define _ML_FPM_H

#include "config.h"
#include "com.h"
#define ML_BUF_MAX  32

#define ml_com   COM4





typedef struct
{
     uint8  Head[8];//8byte ֡ͷ
     uint8  Info[24];
} mlPkt_t;


typedef struct
{
	union
	{
 		uint8 rxBuf[ML_BUF_MAX];
 		mlPkt_t mPkt;
	};
	uint8	rIdx;		// ����λ������
	uint8	frameOK;	// ֡�����������: 1--������0--û��
	uint8	status;		// 0--û��ƥ�䵽ָ�ƣ�1--ƥ�䵽ָ��
	UN16    ID;         //ָ��id��
} mlMux_t;








enum
{
    ML_ST_IDLE=0,
    ML_ST_SIGN,     //ע��״̬
    ML_ST_VERIFY,
    ML_ST_WAIT,
    ML_ST_CLEAR,    //ָ�����
    ML_ST_RST,     //��λ
    ML_ST_ERR,
    ML_ST_MAX
};





extern void ml_st_set(uint8 st);
extern void MlFpm_Init(void);
extern void mlcom_Deal(uint8 dat);
extern void mlst_task(void);    //200ms
#endif /* ML_FPM_H */


