#include "config.h"
#include "rfid.h"
#include "console.h"
#include "com.h"
#include "bsp_i2c.h"
#include "dbg.h"
#include "Syn6658.h"
#include "Task_Main.h"
#include "ccu.h"
#include "SoftTimer.h"
#include "ml_fpm.h"





void GPIO_Config(void);
void NVIC_Configuration(void);
void HSI_SetSysClock(uint32_t pllmul);

void GPIO_Config(void)
{
		/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*����GPIOA��GPIOC������ʱ��*/
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);

        //LED IO ��ʼ��
		/*ѡ��Ҫ���Ƶ�GPIOA����*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		/*��������ģʽΪͨ���������*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		/*������������Ϊ50MHz */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		//BELL IO ��ʼ��
        /*ѡ��Ҫ���Ƶ�GPIOA����*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		/*��������ģʽΪͨ���������*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		/*������������Ϊ50MHz */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
        //BELL(ON);
        Delay_ms(1000);
        BELL(OFF);
}








int main(void)
{
  	RCC_ClocksTypeDef RCC_Clocks;
    SystemInit();					//��ʼ��ϵͳ
    /* ����SysTick Ϊ1us�ж�һ�� */
    SysTick_Init();
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);//���ж�ʸ���ŵ�Flash��0��ַ
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//�������ȼ����õ�ģʽ���������Ķ�ԭ�����е�����
    com_init(console_com, 115200);//����̨com
    RCC_GetClocksFreq(&RCC_Clocks);
    dbg("SYSCLK:%d,HCLK:%d,PCLK1:%d,PCLK2:%d",
            RCC_Clocks.SYSCLK_Frequency,
            RCC_Clocks.HCLK_Frequency,
            RCC_Clocks.PCLK1_Frequency,
            RCC_Clocks.PCLK2_Frequency
        );
	com_init(ccu_com, 9600);     //��λ��com
	com_init(console_com, 115200);//����̨com
	com_init(rfid_com, 9600);
	//com_init(ml_com, 115200);
	com_init(syn6658_com, 9600);
    /* TIM2 ��ʱ���� */
    TIM2_Configuration();
    GPIO_Config();
    I2C_EE_Config();
    syn6658_check();
    rf_state_set(STATE_RFID_INIT);
    TimersInit(SystemTicksCount);

	while(1)
	{
	    ccu_rxDeal();
	    syn6658_rxDeal();
        console_rxDeal();
        ProcessTimer();
        TaskProcess();
    }
}




/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
