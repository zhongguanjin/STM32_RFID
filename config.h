#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stm32f10x.h"
#include "Uart.h"
#include "stdio.h"
#include "string.h"
#include "SysTick.h"
#include "TiMbase.h"
#include "stm32f10x_it.h"

typedef char	int8;
typedef int		int16;
typedef long	int32;
typedef unsigned char		uint8;
typedef unsigned int		uint16;
typedef unsigned long		uint32;
typedef unsigned long		ulong;
typedef unsigned long long	ullong;

#pragma anon_unions

#define OK		(1)
#define ERR		(0)

#define ON 1
#define OFF 0
/* 带参宏，可以像内联函数一样使用 */
#define LED(a)	if (a)	\
					GPIO_ResetBits(GPIOA,GPIO_Pin_1);\
					else		\
					GPIO_SetBits(GPIOA,GPIO_Pin_1)

#define BELL(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_7);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_7)

#endif

