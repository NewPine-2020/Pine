/*
File Name    : yc11xx_systick.h
Author       : Yichip
Version      : V1.0
Date         : 2019/11/06
Description  : systick encapsulation.
*/

#ifndef __YC_SYSTICK_H__
#define __YC_SYSTICK_H__
#include "yc11xx.h"
#include "type.h"

/* SysTick registers */
/* SysTick control & status */
#define SYSTICK_CSR     ((volatile unsigned int *)0xE000E010)
/* SysTick Reload value */ 
#define SYSTICK_RVR     ((volatile unsigned int *)0xE000E014)
/* SysTick Current value */ 
#define SYSTICK_CVR     ((volatile unsigned int *)0xE000E018)
/* SysTick CSR register bits */ 
#define SYSTICK_CSR_COUNTFLAG 		16
#define SYSTICK_CSR_CLKSOURCE 		2
#define SYSTICK_CSR_TICKINT   		1
#define SYSTICK_CSR_ENABLE   		0
#define SYSTICK_SYSCLOCK			1
#define SYSTICK_HALF_SYSCLOCK		0

#define IS_RELOAD_VALUE(x) (x>0&&x<=0xffffff)

typedef int32_t tick;
#define TICK_MAX_VALUE	(int32_t)0x7FFFFFFF

/**
 * @brief Initialize systick and start systick
 *
 * @param ReloadValue : the systick reload value
 *
 * @retval 0:succeed  1:error
 */
uint32_t SysTick_Config(uint32_t ReloadValue);

#endif   /* __YC_SYSTICK_H__ */
