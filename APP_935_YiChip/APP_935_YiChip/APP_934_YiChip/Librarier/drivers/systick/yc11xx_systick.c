/*
File Name    : yc11xx_systick.c
Author       : Yichip
Version      : V1.0
Date         : 2019/11/06
Description  : systick encapsulation.
*/

#include "yc11xx_systick.h"

tick SystickCount;

uint32_t SysTick_Config(uint32_t ReloadValue)
{
	ReloadValue-=1;
	if(!IS_RELOAD_VALUE(ReloadValue))	return 1;

	*SYSTICK_CSR &= ~(((uint32_t)1)<<SYSTICK_CSR_ENABLE);
	
  	 *SYSTICK_RVR =ReloadValue;//Set the reload value 
  	 *SYSTICK_CVR = 0; //clear the current value
 	 SystickCount = 0;// Reset the overflow counter 
	*SYSTICK_CSR |= \
	((SYSTICK_SYSCLOCK<<SYSTICK_CSR_CLKSOURCE) | \
	(1<< SYSTICK_CSR_ENABLE )| \
	(1<<SYSTICK_CSR_TICKINT)); 
	 return 0;
}


tick SysTick_GetTick()
{
	return SystickCount;
}


Boolean SysTick_IsTimeOut(tick start_tick,int interval)
{
	return FALSE;
}

uint32_t SysTick_GetRelativeTime(tick start_tick)
{
	return 0;
}
