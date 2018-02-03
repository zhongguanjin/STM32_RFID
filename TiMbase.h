#ifndef _TiMbase_H
#define _TiMbase_H

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_it.h"
#include "config.h"

extern unsigned long SystemTicksCount(void);


void TIM2_Configuration(void);



#define sys_ticks()			SystemTicksCount()
#define tick_timeout(x)		((int)(sys_ticks() - x) > 0)




#endif
