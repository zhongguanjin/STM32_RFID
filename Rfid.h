
#ifndef _RFID_H_
#define _RFID_H_

#include "config.h"

#include "com.h"

#define rfid_com  COM4

//#define rfid_rxDeal()  com4_rxDeal()

#define RFID_BUF_MAX	32

#define  USER_TEST     1



typedef struct
{
	u8 mode;
	u8 uid[4];
	u8 keyid;
	u8 blkid;
} rfC2E_t;    // E2密钥验证信息结构

typedef struct
{
	u8 blkid;
	u8 dat[16];
} rfC2H_t;    // 写数据信息结构

typedef struct
{
   	u8      blkid;
    u8	Wallet_val[4];
}rfC2P_t;



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
	uint8	rIdx;		// 接收位置索引
	uint8	frameOK;	// 帧接收完整标记: 1--完整，0--没有
	uint8	mode;		// 0--主动检测  1--自动检测
	uint8	status;		// 0--没有检测到卡，1--检测到卡
	rfInfo_t	devInfo;	// 卡信息
} rfMux_t;



#define RF_HEAD_C1A		0x00410106		// 读取设备信息
#define RF_HEAD_C1B		0x00420106		// 配置IC卡接口
#define RF_HEAD_C1C		0x00430106		// 关闭IC卡接口
#define RF_HEAD_C1D		0x01440107		// 设置IC卡接口协议
#define RF_HEAD_C1E1	0x0845010E		// 装载IC卡密钥，6位密钥
#define RF_HEAD_C1E2	0x12450118		// 装载IC卡密钥，16位密钥
#define RF_HEAD_C1F		0x02460108		// 设置IC卡接口的寄存器值
#define RF_HEAD_C1G		0x01470107		// 获取IC卡接口的寄存器值
#define RF_HEAD_C1H		0x01480107		// 设置波特率
#define RF_HEAD_C1I		0x01490107		// 设置天线驱动方式
#define RF_HEAD_C1K		0x014B0107		// 设置新旧帧格式
#define RF_HEAD_C1U		0x02550108		// 设置设备工作模式
#define RF_HEAD_C1V		0x00560106		// 获取设备工作模式
#define RF_HEAD_C1a		0x11610117		// 装载用户密钥, info: 扇区+密钥
#define RF_HEAD_C1b		0x02620108		// 读eeprom, info: addr+len
#define RF_HEAD_C1c		0x00630106		// 写eeprom，info: addr+len+data

#define RF_HEAD_C2A		0x01410207		// 请求
#define RF_HEAD_C2B1	0x02420208		// 防碰撞1
#define RF_HEAD_C2B2	0x0642020C		// 防碰撞2
#define RF_HEAD_C2C		0x0543020B		// 卡选择
#define RF_HEAD_C2D		0x00440206		// 卡挂起
#define RF_HEAD_C2E		0x0745020D		// 密钥验证
#define RF_HEAD_C2FA	0x0C460212		// 直接密钥A验证
#define RF_HEAD_C2FB	0x0C460212		// 直接密钥B验证
#define RF_HEAD_C2G		0x01470207		// Mifare卡读
#define RF_HEAD_C2H		0x11480217		// Mifare卡写
#define RF_HEAD_C2I		0x0549020B		// UltraLight卡读
#define RF_HEAD_C2J		0x074A020D		// Mifare值操作
#define RF_HEAD_C2L		0x014C0207		// 卡复位
#define RF_HEAD_C2M		0x024D0208		// 卡激活
#define RF_HEAD_C2N		0x074E020D		// 自动检测，使用密钥E2或不验证
#define RF_HEAD_C2O		0x014F0207		// 读自动检测数据
#define RF_HEAD_C2P		0x0550020B		// 设置值块的值
#define RF_HEAD_C2Q		0x01510207		// 获取值块的值
#define RF_HEAD_C2X		0x00580206		// 数据交互命令, info: data+2B

enum
{
    STATE_RFID_IDLE =0,
    STATE_RFID_INIT,
    STATE_RFID_TIME,  //定时查询卡
    STATE_RFID_CHKCARD ,  //检测到刷卡
    STATE_RFID_READCARD,
    STATE_RFID_WRITEDAT,
    STATE_RFID_READDAT,
    STATE_WALLET_INIT,  //钱包初始化
    STATE_WALLET_DEC,   //钱包扣款
    STATE_WALLET_INC,   //钱包充值
    STATE_WALLET_BALANCE, //钱包余额查询
    STATE_RFID_MAX
};



extern void rf_init_check(void);

extern void Rfid_Task_Process(void);

extern void com4_rxDeal(void);

extern uint32 get_rf_uid(void);
extern void read_rf_dat(uint8 blank);

#if USER_TEST
extern int rfUsr_isRfok(u8 *buf);
extern int rfUsr_isRfFull(void);
extern int rfUsr_append(u8 *buf);
extern void rfUsr_showRfSerial(void);
extern void rfUsr_setDefault(void);
extern void rfUsr_init(void);
#else
extern uint8 check_rfid_user(uint32 uid);
extern uint8 add_rfid_user(void);
extern uint8 del_rfid_user(void);
extern uint8 clear_rfid_user(void);
extern uint8 query_rfid_user(uint32* buf);
extern void rfid_user_init(void);

#endif
extern void rf_state_set(uint8 st);

extern uint8 rf_bccCalc(uint8 * buf,uint8 len);
#endif
