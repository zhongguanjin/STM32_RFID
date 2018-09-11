#ifndef _TiMbase_H
#define _TiMbase_H

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "config.h"

extern unsigned long SystemTicksCount(void);


void TIM2_Configuration(void);



#define sys_ticks()			SystemTicksCount()
#define tick_timeout(x)		((int)(sys_ticks() - x) > 0)

extern void TIM2_IRQHandler(void);



#endif
