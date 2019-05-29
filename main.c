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
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启GPIOA和GPIOC的外设时钟*/
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);

        //LED IO 初始化
		/*选择要控制的GPIOA引脚*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		/*设置引脚速率为50MHz */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		//BELL IO 初始化
        /*选择要控制的GPIOA引脚*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		/*设置引脚速率为50MHz */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
        //BELL(ON);
        Delay_ms(1000);
        BELL(OFF);
}

//配置矢量中断，矢量的意思就是有顺序，有先后的意思。
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure1;	//定义数据结构体
  NVIC_InitTypeDef NVIC_InitStructure2;	//定义数据结构体
  NVIC_InitTypeDef NVIC_InitStructure3;
  NVIC_InitTypeDef NVIC_InitStructure4;
  NVIC_InitTypeDef NVIC_InitStructure5;
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);//将中断矢量放到Flash的0地址
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//设置优先级配置的模式，详情请阅读原材料中的文章

  //使能串口1中断，并设置优先级
  NVIC_InitStructure1.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure1.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure1);	//将结构体丢到配置函数，即写入到对应寄存器中

    //使能串口4中断，并设置优先级
  NVIC_InitStructure2.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure2);	//将结构体丢到配置函数，即写入到对应寄存器中

  NVIC_InitStructure3.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure3.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure3.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure3.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure3);

    //使能串口3中断，并设置优先级
  NVIC_InitStructure4.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure4.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure4.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure4.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure4);	//将结构体丢到配置函数，即写入到对应寄存器中
    //使能串口3中断，并设置优先级
  NVIC_InitStructure5.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure5.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure5.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure5.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure5);	//将结构体丢到配置函数，即写入到对应寄存器中


}



int main(void)
{
    SystemInit();					//初始化系统
    /* 配置SysTick 为1us中断一次 */
    SysTick_Init();
	com_init(ccu_com, 9600);     //上位机com
	com_init(console_com, 115200);//控制台com
	com_init(rfid_com, 9600);
	//com_init(ml_com, 115200);
	com_init(syn6658_com, 9600);
    NVIC_Configuration(); //初始化相关中断
    /* TIM2 定时配置 */
    TIM2_Configuration();
    GPIO_Config();
    I2C_EE_Config();
    syn6658_check();
    rf_init_check();
    //MlFpm_Init();
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

