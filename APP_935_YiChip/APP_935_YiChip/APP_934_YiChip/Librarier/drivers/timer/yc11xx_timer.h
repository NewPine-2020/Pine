 /**
 * Copyright 2016, yichip Semiconductor(shenzhen office)
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Yichip Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Yichip Semiconductor.
 */

/** 
  *@file yc11xx_timer.h
  *@brief timer support for application.
  */
#ifndef __YC11XX_TIMER_H__
#define __YC11XX_TIMER_H__
	
#include "yc11xx.h"
#include "ycdef.h"
#include "yc_drv_common.h"

/** 
  * @brief  timer number Structure definition  
  */ 
typedef enum
{
  TIM0 = 0,
	TIM1,
	TIM2,
	TIM3,
	TIM4,
	TIM5,
  TIM6,
  TIM7
}TIM_NumTypeDef;
#define TIM_NumTypeDef(TIMx)	(TIMx==TIM0||\
																TIMx==TIM1||\
																TIMx==TIM2||\
																TIMx==TIM3||\
																TIMx==TIM4||\
																TIMx==TIM5||\
																TIMx==TIM6||\
																TIMx==TIM7)

#define FREQUENCY_DIVISION_0		0x0
#define FREQUENCY_DIVISION_1		0x1
#define FREQUENCY_DIVISION_2		0x2
#define FREQUENCY_DIVISION_3		0x3
#define FREQUENCY_DIVISION_4		0x4
#define FREQUENCY_DIVISION_5		0x5
#define FREQUENCY_DIVISION_6		0x6
#define FREQUENCY_DIVISION_7		0x7
#define IS_FREQUENCY_DIVISION(FREQUENCY)		((FREQUENCY == FREQUENCY_DIVISION_0)||\
																								(FREQUENCY == FREQUENCY_DIVISION_1) ||\
																								(FREQUENCY == FREQUENCY_DIVISION_2)	||\
																								(FREQUENCY == FREQUENCY_DIVISION_3) ||\
																								(FREQUENCY == FREQUENCY_DIVISION_4)	||\
																								(FREQUENCY == FREQUENCY_DIVISION_5) ||\
																								(FREQUENCY == FREQUENCY_DIVISION_6)	||\
																								(FREQUENCY == FREQUENCY_DIVISION_7))

/** 
  * @brief gpio output enumeration
  */  
typedef enum 
{
	OutputLow  = 0,
	OutputHigh = 0x10
} START_TypeDef;
#define IS_PWM_START(LEVEL)		(LEVEL == OutputLow || LEVEL == OutputHigh)

/**
  *@brief Sync.
  */
typedef enum
{
	Sync_default = 0,
	Sync_enable  = 0x08
} PWM_SyncTypedef;
#define IS_PWM_SYNC(SYNC)		(SYNC == Sync_default || SYNC == Sync_enable)

/** 
  * @brief  PWM Init Structure definition  
  */  
typedef struct 
{
	uint16_t LowLevelPeriod;
	uint16_t HighLevelPeriod;
	START_TypeDef StartLevel;
	PWM_SyncTypedef Sync;
} PWM_InitTypeDef;

typedef enum
{
	TIM_Mode_PWM	  = 0x20,
	TIM_Mode_TIMER	= 0xe0
}TIM_ModeTypeDef;
#define IS_TIM_MODE(MODE)		(MODE == TIM_Mode_TIMER || MODE == TIM_Mode_PWM)

/** 
  * @brief  timer Init Structure definition  
  */ 
typedef struct 
{
	TIM_NumTypeDef TIMx;
	uint16_t period;
	uint8_t frequency;
	TIM_ModeTypeDef mode;
	PWM_InitTypeDef pwm;
}TIM_InitTypeDef;
#define IS_PERIOD_VALUE(x) (x<=0xffff)



/**
 * @brief Initialize TIMx ,not start timer,use TIM_Cmd start timer
 *
 * @param TIM_init_struct : the TIM_InitTypeDef Structure
 *
 * @retval none
 */
void TIM_Init(TIM_InitTypeDef* TIM_init_struct);

/**
 * @brief enable or disable timer clk
 *
 * @param TIMx : the timer number,TIM0-TIM7
 *
 * @param NewState :DISABLE or ENABLE 
 *
 * @retval none
 */
void TIM_Cmd(TIM_NumTypeDef TIMx, FunctionalState NewState);

#endif
