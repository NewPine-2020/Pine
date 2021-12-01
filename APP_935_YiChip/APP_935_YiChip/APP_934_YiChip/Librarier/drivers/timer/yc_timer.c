/**
 * COPYRIGHT NOTICE
 *Copyright(c) 2014,YICHIP
 *
 * @file yc_timer.c
 * @brief ...
 *
 * @version 1.0
 * @author  jingzou
 * @data    Jan 23, 2018
**/


#include "yc_timer.h"
#include "ycdef.h"
//#include "yc_debug.h"
//#include "ipc.h"
//#include "yc11xx_bt_interface.h"

SYS_TIMER_TYPE *pheader;
SYS_TIMER_TYPE header_Timer;
uint32_t sys_tick_count=0;

uint32_t gSystemTimerAdjustClknbt;
uint8_t  sys_Timer_Check_Flag;

void SYS_TimerExpireDefaultHandle(int params)
{
	// YC_LOG_ERROR("default timer expire !\r\n");
	while(0);
}
void SYS_ChangeDPLL(uint8_t systemClk)
{
	uint8_t tmp = HREAD(CORE_CONFIG);
//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_A2DPCallBack: 0x%04X", LOG_POINT_A503, systemClk);
	if(systemClk != CLOCK_48M_multiple)
	{
		systemClk = CLOCK_48M_multiple;
	}
//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_A2DPCallBack: 0x%04X", LOG_POINT_A503, systemClk);
	uint8_t tmpVal = systemClk;
	tmp &= 0xf9;
	tmp |= tmpVal << 1;
	HWRITE(CORE_CONFIG, tmp);

	//SysTick_Config(SYS_TimerGet1usTicks() * 10000); //each  systick interrupt is 10ms

	// To reduce power. becareful, when you want change to 0, must make sure change clk before.
	// becouse bbldo 0 with 192M may error.
	/*
	if(systemClk == CLOCK_48M_multiple)
	{
		Bt_LpmChangeBBLdoLevel(0);
	}
	else
	{
		Bt_LpmChangeBBLdoLevel(7);
	}*/
}
uint8_t  gSystemInFastDpll = 0;
void SYS_ChangeToFastDPLL(void)
{
	gSystemInFastDpll = TRUE;
	uint8_t tmp = HREAD(CORE_CONFIG);
	uint8_t tmpVal = CLOCK_192M_multiple;
	tmp &= 0xf9;
	tmp |= tmpVal << 1;
	HWRITE(CORE_CONFIG, tmp);

	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
}
void SYS_ChangeToNormalDPLL(void)
{
	gSystemInFastDpll = FALSE;
	uint8_t tmp = HREAD(CORE_CONFIG);
	uint8_t tmpVal = CLOCK_48M_multiple;
	tmp &= 0xf9;
	tmp |= tmpVal << 1;
	HWRITE(CORE_CONFIG, tmp);
	
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
	hw_delay();
}
uint32_t SYS_TimerGet1usTicks()
{
	uint8_t tmp = HREAD(CORE_CONFIG);
	uint8_t tmpVal = (tmp >> 1) & 0x03;
	uint8_t rtnVal = 0;
	switch(tmpVal)
	{
		case CLOCK_48M_multiple:{
			rtnVal = SYSTEM_CLOCK_48M;
			break;
		}
		case CLOCK_64M_multiple:{
			rtnVal = SYSTEM_CLOCK_64M;
			break;
		}
		case CLOCK_96M_multiple:{
			rtnVal = SYSTEM_CLOCK_96M;
			break;
		}
		case CLOCK_192M_multiple:{
			rtnVal = SYSTEM_CLOCK_192M;
			break;
		}
		default:
				break;
	}
	return rtnVal;
}
void SYS_TimerInit(uint8_t systemClk)
{
	//initial timer lists
	pheader = &header_Timer;
	pheader->pNextTimer = NULL;
	pheader->pfExpireCb = SYS_TimerExpireDefaultHandle;
	pheader->mTimerStatus = TIMER_START;
	pheader->mTimerValue = 0;

	gSystemTimerAdjustClknbt = 0;

	SYS_TimerStartTickTimer(systemClk);
}

void SYS_TimerStartTickTimer(uint8_t systemClk)
{
	SYS_ClkTicks();
	//SysTick_Config(SYSTEM_CLOCK/100); //each  systick interrupt is 10ms
	SYS_ChangeDPLL(systemClk);
}



//union is 10ms
BOOL SYS_SetTimer(SYS_TIMER_TYPE *pTimer, int tick,TIMER_TYPE isSingle,Timer_Expire_CB pfExpire_CB)
{
	SYS_TIMER_TYPE *pTemp;
	if(pheader == NULL)
	{
		return FALSE;
	}
	if(pTimer == NULL)
	{
//		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_A500, (uint32_t)pTimer);
		return FALSE;
	}
	if (pfExpire_CB == NULL)
	{
//		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_A501, (uint32_t)pTimer);
		pTimer->pfExpireCb = SYS_TimerExpireDefaultHandle;
	}
	else
	{
		pTimer->pfExpireCb = pfExpire_CB;
	}

	OS_ENTER_CRITICAL();
	pTimer->mTimerValue = sys_tick_count+tick;
	
	pTimer->mTick = tick;
	
	pTimer->mTimerStatus = TIMER_START;
	pTimer->mIsCycle = (TIMER_TYPE)(isSingle &0x3f);

	if (SYS_TimerisExist(pTimer))
	{
		OS_EXIT_CRITICAL();
		return TRUE;
	}
	
	//insert to list
	pTemp = pheader;
	pheader = pTimer;
	pheader->pNextTimer = pTemp;
	OS_EXIT_CRITICAL();

	//YC_LOG_INFO("set timer %d \r\n",pTimer->cbParams);

	return TRUE;
}

BOOL SYS_TimerisExist(SYS_TIMER_TYPE *pTimer)
{
	SYS_TIMER_TYPE *pTemp;
	if(pheader == NULL)
	{
		return FALSE;
	}
	if (pTimer == NULL)
	{
		return FALSE;
	}
	for(pTemp = pheader; pTemp->pNextTimer != NULL; pTemp = pTemp->pNextTimer)
	{
		if (pTimer == pTemp)
		{
			return TRUE;
		}
	}
	return FALSE;
}


BOOL SYS_ResetTimer(SYS_TIMER_TYPE *pTimer)
{
	SYS_TIMER_TYPE *pTemp;
	if(pheader == NULL)
	{
		return FALSE;
	}
	if (pTimer == NULL)
	{
		return FALSE;
	}
	for(pTemp = pheader; pTemp->pNextTimer != NULL; pTemp = pTemp->pNextTimer)
	{
		if (pTimer == pTemp)
		{
			pTimer->mTimerValue = sys_tick_count+(pTimer->mTick);
			return TRUE;
		}
	}
	return FALSE;
}


void SYS_TimerTest()
{
	SYS_TIMER_TYPE *pTimer;
	int i = 0;
	for (pTimer = pheader; pTimer->pNextTimer != NULL; pTimer = pTimer->pNextTimer)
	{
		i++;
	}
	//YC_LOG_INFO("timer count %d \r\n",i);
}
/*
void SYS_timerPolling()
{
	SYS_TIMER_TYPE *pTimer;
	if(HREAD(IPC_MCU_STATE) != IPC_MCU_STATE_RUNNING)
	{
		return;
	}
	//Lpm_LockLpm(TIMER_LPM_FLAG);
	if (sys_Timer_Check_Flag)
	{
		sys_Timer_Check_Flag = FALSE;
		SYS_TimerTest();
		for (pTimer = pheader; pTimer->pNextTimer != NULL; pTimer = pTimer->pNextTimer)
		{
			if (pTimer->mTimerStatus == TIMER_START)
			{
				if (pTimer->mTimerValue <= sys_tick_count)
				{
					if (pTimer->mIsCycle == TIMER_SINGLE)
					{
						//YC_LOG_INFO("timer release %d in\r\n",pTimer->cbParams);
						SYS_ReleaseTimer(pTimer);
					}
					else
					{
						//YC_LOG_INFO("timer cycle in\r\n");
						//YC_LOG_INFO("timer release %d in\r\n",pTimer->cbParams);
						pTimer->mTimerValue = sys_tick_count+(pTimer->mTick);
					}
					//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_A502, (uint32_t)pTimer);
					pTimer->pfExpireCb(pTimer->cbParams);
				}
			}
		}
	}
	//Lpm_unLockLpm(TIMER_LPM_FLAG);
}
*/
BOOL SYS_ReleaseTimer(SYS_TIMER_TYPE *pTimer)
{
	SYS_TIMER_TYPE *pTemp, *pPre;
	if(pheader == NULL)
	{
		return FALSE;
	}
	if (pTimer == NULL)
	{
		return FALSE;
	}

	if (pTimer == &header_Timer)
	{
		return FALSE;
	}

	for(pTemp = pPre = pheader; pTemp->pNextTimer != NULL; pPre = pTemp,pTemp = pTemp->pNextTimer)
	{
		if (pTimer == pTemp)
		{
			pTemp->mTimerStatus = TIMER_STOP;
			if (pPre == pTemp) //delete the first one element
			{
				pheader = pTemp->pNextTimer;
			}
			else
			{
				pPre->pNextTimer = pTemp->pNextTimer;
			}

			//YC_LOG_INFO("release timer %d,%d \r\n",pPre->cbParams,pTimer->cbParams);
			return TRUE;
		}
	}
	return FALSE;
}

void SYS_ReleaseAllTimer()
{
	SYS_TIMER_TYPE *pTemp;
	if(pheader == NULL)
	{
		return;
	}
	for(pTemp = pheader; pTemp->pNextTimer != NULL; pTemp = pTemp->pNextTimer)
	{
		pTemp->mTimerStatus = TIMER_STOP;
	}
	pheader = &header_Timer;
}

void SYStick_handle()
{
	SYS_ClkTicks();
	sys_Timer_Check_Flag = TRUE;
}

void Bt_100ms_timer(uint8_t count)
{
	SYS_ClkTicks();
	sys_Timer_Check_Flag = TRUE;
}

void SYS_ClkTicks(void)
{
	if(SystemCheckInFastSpeed())
	{
		return;
	}
	uint32_t clknbt = HREADL(CORE_CLKN);

	uint32_t diffbt = 0;

	// Check initial
	if(gSystemTimerAdjustClknbt == 0)
	{
		gSystemTimerAdjustClknbt = clknbt;
	}
	else
	{
		// TODO: Think the clock overflow
		if(gSystemTimerAdjustClknbt > clknbt)
		{
			gSystemTimerAdjustClknbt = clknbt;
			return;
		}
		// Every time need check the diff, if diff is big
		diffbt = clknbt - gSystemTimerAdjustClknbt;
		
		// Check diff clk
		// 1s = 3200subSlots = 12000000 = 0xB71B00 ticks
		// 1ms = 32subSlots = 12000000 = 0xB71B00 ticks
		while(diffbt >= (SYSTEM_TIMER_UNIT_SUBSLOTS))
		{
			sys_tick_count ++;
			gSystemTimerAdjustClknbt += SYSTEM_TIMER_UNIT_SUBSLOTS;

			diffbt = diffbt - (SYSTEM_TIMER_UNIT_SUBSLOTS);
		}
	}
}

void SYS_delay_ms(uint32_t nms)
{
	SYS_delay_us(nms*1000);
}

void SYS_delay_us(uint32_t nus){
	uint32_t ticks=0;    
	uint32_t told=0,tnow=0,tcnt=0;    
	//temp code
	//ticks=nus*(SYSTEM_CLOCK/1000000); 
	ticks = nus*SYS_TimerGet1usTicks();
	uint32_t reload = *SYSTICK_RVR;
	told = *(SYSTICK_CVR);
	while(1)
	{
		
		tnow = *(SYSTICK_CVR);

		if(tnow != told)
		{
			if(tnow < told)
			{
				tcnt += told - tnow;
			}	
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			
			if(tcnt >= ticks)
			{
				break;
			}
		}
		
	}

	
}

BOOL SystemCheckInFastSpeed(void)
{
	return gSystemInFastDpll;
}

