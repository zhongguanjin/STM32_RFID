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

//����ʸ���жϣ�ʸ������˼������˳�����Ⱥ����˼��
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure1;	//�������ݽṹ��
  NVIC_InitTypeDef NVIC_InitStructure2;	//�������ݽṹ��
  NVIC_InitTypeDef NVIC_InitStructure3;
  NVIC_InitTypeDef NVIC_InitStructure4;
  NVIC_InitTypeDef NVIC_InitStructure5;
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);//���ж�ʸ���ŵ�Flash��0��ַ
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//�������ȼ����õ�ģʽ���������Ķ�ԭ�����е�����

  //ʹ�ܴ���1�жϣ����������ȼ�
  NVIC_InitStructure1.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure1.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure1);	//���ṹ�嶪�����ú�������д�뵽��Ӧ�Ĵ�����

    //ʹ�ܴ���4�жϣ����������ȼ�
  NVIC_InitStructure2.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure2);	//���ṹ�嶪�����ú�������д�뵽��Ӧ�Ĵ�����

  NVIC_InitStructure3.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure3.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure3.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure3.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure3);

    //ʹ�ܴ���3�жϣ����������ȼ�
  NVIC_InitStructure4.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure4.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure4.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure4.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure4);	//���ṹ�嶪�����ú�������д�뵽��Ӧ�Ĵ�����
    //ʹ�ܴ���3�жϣ����������ȼ�
  NVIC_InitStructure5.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure5.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure5.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure5.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure5);	//���ṹ�嶪�����ú�������д�뵽��Ӧ�Ĵ�����


}



int main(void)
{
    SystemInit();					//��ʼ��ϵͳ
    /* ����SysTick Ϊ1us�ж�һ�� */
    SysTick_Init();
	com_init(ccu_com, 9600);     //��λ��com
	com_init(console_com, 115200);//����̨com
	//com_init(rfid_com, 9600);
	com_init(ml_com, 115200);
	com_init(syn6658_com, 9600);
    NVIC_Configuration(); //��ʼ������ж�
    /* TIM2 ��ʱ���� */
    TIM2_Configuration();
    GPIO_Config();
    I2C_EE_Config();
    syn6658_check();
    MlFpm_Init();
    rf_state_set(STATE_RFID_INIT);
	while(1)
	{
	    ccu_rxDeal();
	    syn6658_rxDeal();
        console_rxDeal();
        TaskProcess();
    }
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

