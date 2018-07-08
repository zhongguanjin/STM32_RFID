#include "config.h"
#include "rfid.h"
#include "console.h"
#include "com.h"
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

}


int main(void)
{
    SystemInit();					//初始化系统
    /* 配置SysTick 为1us中断一次 */
    SysTick_Init();
    /* TIM2 定时配置 */
    TIM2_Configuration();
	//COM3_4_Init();  //初始化UART1,UART4
	com_init(COM3, 9600);
	com_init(COM1, 115200);
	com_init(COM4, 9600);
    NVIC_Configuration(); //初始化相关中断
    /* TIM2 重新开时钟，开始计时 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    GPIO_Config();
	rf_check();  //RFID检测初始化。
	//dbg_Init();
	while(1)
	{
	    com3_rxDeal();
        console_process();
        Rfid_Task_Process();
    }
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

