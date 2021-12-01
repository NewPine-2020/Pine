/**
 * COPYRIGHT NOTICE
 *Copyright(c) 2014,YICHIP
 *
 * @file yc11xx_timer.c
 * @brief ...
 *
 * @version 1.0
 * @author  WangJingfan
 * @data    7-Nov-2019
**/

#include "yc11xx_timer.h"

void TIM_Init(TIM_InitTypeDef* TIM_init_struct)
{
	_ASSERT(TIM_NumTypeDef(TIM_init_struct->TIMx));
	_ASSERT(IS_TIM_MODE(TIM_init_struct->mode));
	
	uint8_t pwm_ctrl = 0;
	
	if (TIM_init_struct->mode == TIM_Mode_TIMER)
	{
		_ASSERT(IS_PERIOD_VALUE(TIM_init_struct->period));
		_ASSERT(IS_FREQUENCY_DIVISION(TIM_init_struct->frequency));
		
		pwm_ctrl |= TIM_init_struct->mode;
		pwm_ctrl |= TIM_init_struct->frequency;
		
		HW_REG_16BIT(reg_map(CORE_PWM_PCNT(TIM_init_struct->TIMx)), TIM_init_struct->period);
		HW_REG_8BIT(reg_map(CORE_PWM_CTRL(TIM_init_struct->TIMx)), pwm_ctrl);
	}
	else 
	{
		_ASSERT(IS_PERIOD_VALUE(TIM_init_struct->pwm.HighLevelPeriod));
		_ASSERT(IS_PERIOD_VALUE(TIM_init_struct->pwm.LowLevelPeriod));
		_ASSERT(IS_PWM_START(TIM_init_struct->pwm.StartLevel));
		_ASSERT(IS_PWM_SYNC(TIM_init_struct->pwm.Sync));
		
		pwm_ctrl |= TIM_init_struct->mode;
		pwm_ctrl |= TIM_init_struct->pwm.StartLevel;
		pwm_ctrl |= TIM_init_struct->pwm.Sync;
		
		HW_REG_16BIT(reg_map(CORE_PWM_PCNT(TIM_init_struct->TIMx)), TIM_init_struct->pwm.HighLevelPeriod);
		HW_REG_16BIT(reg_map(CORE_PWM_NCNT(TIM_init_struct->TIMx)), TIM_init_struct->pwm.LowLevelPeriod);
		HW_REG_8BIT(reg_map(CORE_PWM_CTRL(TIM_init_struct->TIMx)), pwm_ctrl);
	}
}

void TIM_Cmd(TIM_NumTypeDef TIMx, FunctionalState NewState)
{
	_ASSERT(TIM_NumTypeDef(TIMx));
	_ASSERT(IS_FUNCTIONAL_STATE(NewState));
	
	uint8_t pwm_clken;
	
	pwm_clken = HR_REG_8BIT(reg_map(CORE_PWM_CLKEN));
	if (NewState == ENABLE)
	{
		pwm_clken |= (0x01 << TIMx);
	}
	else if (NewState == DISABLE)
	{
		pwm_clken &= (~(0x01 << TIMx));
	}
	HW_REG_8BIT(reg_map(CORE_PWM_CLKEN), pwm_clken);
}
