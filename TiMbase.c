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
#include "Task_Main.h"

#include "dbg.h"

/*
 * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71
 * �ж�����Ϊ = 1/(36MHZ /36) * 1000 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> �ж� ��TIMxCNT����Ϊ0���¼���
 */
void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

		/* ����TIM2CLK Ϊ 18MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    //TIM_DeInit(TIM2);

	/* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
    TIM_TimeBaseStructure.TIM_Period=999;

    /* �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж� */
	  /* ʱ��Ԥ��Ƶ��Ϊ72 */
    TIM_TimeBaseStructure.TIM_Prescaler= 35;

		/* ���ⲿʱ�ӽ��в�����ʱ�ӷ�Ƶ,����û���õ� */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);

    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

    TIM_Cmd(TIM2, ENABLE);
       /* TIM2 ���¿�ʱ�ӣ���ʼ��ʱ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


}


u32 tick_time1;



u32 SystemTicksCount(void)
{
    return tick_time1;
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET )
	{
	    tick_time1++;
	    TaskRemarks();
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
	}

}





/*********************************************END OF FILE**********************/
