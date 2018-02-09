
#ifndef _RFID_H_
#define _RFID_H_

#include "config.h"

#define RFID_BUF_MAX	32
typedef union
{
	void *	ptr;
	uint32	u;
	int32	i;
	uint16	ush[2];
	int16	sh[2];
	uint8	uch[4];
	int8	ch[4];
} UN32;

typedef struct
{
	u8 mode;
	u8 uid[4];
	u8 keyid;
	u8 blkid;
} rfC2E_t;    // E2��Կ��֤��Ϣ�ṹ

typedef struct
{
	u8 blkid;
	u8 dat[16];
} rfC2H_t;    // д������Ϣ�ṹ

typedef struct
{
   	u8      blkid;
    u8	Wallet_val[4];
}rfC2P_t;

typedef union
{
	uint16	ush;
	int16	sh;
	uint8	uch[2];
	int8	ch[2];
} UN16;

typedef struct
{
union {
        struct
        {
            uint8  frameLen;
            uint8  cmdType;
            uint8  cmdOrSta;
            uint8  infoLen;
        };
        uint32  cmdHead;
      };
    uint8  info[RFID_BUF_MAX-4];   //28 byte
} rfPkt_t;


typedef struct {
	UN32	uid;
	uint8	dat[16];
} rfInfo_t;


typedef struct
{
	union
	{
		uint8 rxBuf[RFID_BUF_MAX];
		rfPkt_t rPkt;
	};
	uint8	rIdx;		// ����λ������
	uint8	frameOK;	// ֡�����������: 1--������0--û��
	uint8	mode;		// 0--�������  1--�Զ����
	uint8	status;		// 0--û�м�⵽����1--��⵽��
	rfInfo_t	devInfo;	// ����Ϣ
} rfMux_t;

typedef  union
{
      struct
      {
        uint8  frameLen;
        uint8  cmdType;
        uint8  cmdOrSta;
        uint8  info[RFID_BUF_MAX-3];   //28 byte
      };
      uint8 rxBuf[RFID_BUF_MAX];
} Uartrx_t;

#define RF_HEAD_C1A		0x00410106		// ��ȡ�豸��Ϣ
#define RF_HEAD_C1B		0x00420106		// ����IC���ӿ�
#define RF_HEAD_C1C		0x00430106		// �ر�IC���ӿ�
#define RF_HEAD_C1D		0x01440107		// ����IC���ӿ�Э��
#define RF_HEAD_C1E1	0x0845010E		// װ��IC����Կ��6λ��Կ
#define RF_HEAD_C1E2	0x12450118		// װ��IC����Կ��16λ��Կ
#define RF_HEAD_C1F		0x02460108		// ����IC���ӿڵļĴ���ֵ
#define RF_HEAD_C1G		0x01470107		// ��ȡIC���ӿڵļĴ���ֵ
#define RF_HEAD_C1H		0x01480107		// ���ò�����
#define RF_HEAD_C1I		0x01490107		// ��������������ʽ
#define RF_HEAD_C1K		0x014B0107		// �����¾�֡��ʽ
#define RF_HEAD_C1U		0x02550108		// �����豸����ģʽ
#define RF_HEAD_C1V		0x00560106		// ��ȡ�豸����ģʽ
#define RF_HEAD_C1a		0x11610117		// װ���û���Կ, info: ����+��Կ
#define RF_HEAD_C1b		0x02620108		// ��eeprom, info: addr+len
#define RF_HEAD_C1c		0x00630106		// дeeprom��info: addr+len+data

#define RF_HEAD_C2A		0x01410207		// ����
#define RF_HEAD_C2B1	0x02420208		// ����ײ1
#define RF_HEAD_C2B2	0x0642020C		// ����ײ2
#define RF_HEAD_C2C		0x0543020B		// ��ѡ��
#define RF_HEAD_C2D		0x00440206		// ������
#define RF_HEAD_C2E		0x0745020D		// ��Կ��֤
#define RF_HEAD_C2FA	0x0C460212		// ֱ����ԿA��֤
#define RF_HEAD_C2FB	0x0C460212		// ֱ����ԿB��֤
#define RF_HEAD_C2G		0x01470207		// Mifare����
#define RF_HEAD_C2H		0x11480217		// Mifare��д
#define RF_HEAD_C2I		0x0549020B		// UltraLight����
#define RF_HEAD_C2J		0x074A020D		// Mifareֵ����
#define RF_HEAD_C2L		0x014C0207		// ����λ
#define RF_HEAD_C2M		0x024D0208		// ������
#define RF_HEAD_C2N		0x074E020D		// �Զ���⣬ʹ����ԿE2����֤
#define RF_HEAD_C2O		0x014F0207		// ���Զ��������
#define RF_HEAD_C2P		0x0550020B		// ����ֵ���ֵ
#define RF_HEAD_C2Q		0x01510207		// ��ȡֵ���ֵ
#define RF_HEAD_C2X		0x00580206		// ���ݽ�������, info: data+2B

enum
{
    STATE_RFID_IDLE =0,
    STATE_RFID_TIME,  //��ʱ��ѯ��
    STATE_RFID_CHKCARD ,  //��⵽ˢ��
    STATE_RFID_READCARD,
    STATE_RFID_WRITEDAT,
    STATE_RFID_READDAT,
    STATE_WALLET_INIT,  //Ǯ����ʼ��
    STATE_WALLET_DEC,   //Ǯ���ۿ�
    STATE_WALLET_INC,   //Ǯ����ֵ
    STATE_WALLET_BALANCE, //Ǯ������ѯ
    STATE_RFID_MAX
};



extern void rf_check(void);

extern void Rfid_Task_Process(void);

extern void Rfid_Receive_Process(void);
extern void Uart_Receive_Process(void);

#endif