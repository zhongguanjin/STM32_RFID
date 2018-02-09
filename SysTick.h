#ifndef _SYSTICK_H
#define _SYSTICK_H

#include "stm32f10x.h"
#include "stm32f10x_it.h"
void    SysTick_Init(void);
void    Delay_us(__IO u32 nTime);

#define Delay_ms(x) Delay_us(100*x)	 //��λms

extern void TimingDelay_Decrement(void);

#endif /* __SYSTICK_H */
