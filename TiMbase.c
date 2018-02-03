/**
  ******************************************************************************
  * @file    TimBase.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   TIM2 1ms ��ʱӦ��bsp
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� iSO STM32 ������
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */

#include "TiMbase.h"


/*
 * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71
 * �ж�����Ϊ = 1/(72MHZ /72) * 1000 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼���
 */
void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

		/* ����TIM2CLK Ϊ 72MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    //TIM_DeInit(TIM2);

	/* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    TIM_TimeBaseStructure.TIM_Period=1000;

    /* �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж� */
	  /* ʱ��Ԥ��Ƶ��Ϊ72 */
    TIM_TimeBaseStructure.TIM_Prescaler= 71;

		/* ���ⲿʱ�ӽ��в�����ʱ�ӷ�Ƶ,����û���õ� */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);

    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

    TIM_Cmd(TIM2, ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);		/*�ȹرյȴ�ʹ��*/
}


 unsigned long led_time;

 u32 tick_time1;
 uint32 SystemTicksCount(void)
 {
    led_time++;
	if(led_time%150 == 60)
	{
		LED(ON);
	}
	if(led_time%150== 120)
	{
		LED(OFF);
	}
	if(led_time%150 == 0)
	{
		led_time =0;
	}
    return tick_time1++;
 }







/*********************************************END OF FILE**********************/
