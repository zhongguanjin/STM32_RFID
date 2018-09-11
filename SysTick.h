#ifndef _SYSTICK_H
#define _SYSTICK_H

#include "stm32f10x.h"
void    SysTick_Init(void);
void    Delay_us(__IO u32 nTime);

#define Delay_ms(x) Delay_us(100*x)	 //µ¥Î»ms

extern void TimingDelay_Decrement(void);
extern void SysTick_Handler(void);

#endif /* __SYSTICK_H */
