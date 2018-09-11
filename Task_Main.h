#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"

// ����ṹ�壺
typedef struct
{
    uint8 Run;                 // �������б�ǣ�0-�����У�1����
    uint16 Timer;              // ��ʱ��
    uint16 ItvTime;              // �������м��ʱ��
    void (*TaskHook)(void);    // Ҫ���е�������
} TASK_COMPONENTS;              // ������










extern void TaskProcess(void);
extern void TaskRemarks(void);
extern float get_temperature(uint8 ad_channel);
extern float get_voltage(uint8 ad_channel);




#endif

