#include "yc_exti.h"
#include "yc11xx_gpio.h"
#include "core_cm0.h"
void NVIC_Config(IRQn_Type IRQn, FunctionalState enable )
{
	if(enable)
	{
		NVIC_ISER |= 1 << IRQn;
	}
	else
		NVIC_ICER |= 1 << IRQn;
}
void GPIO_Interrupt_Config()
{
	uint8_t i;
	NVIC_SetPriority(gpio0_handler_IRQn, 0);
	NVIC_SetPriority(gpio8_handler_IRQn, 0);
	NVIC_SetPriority(gpio9_handler_IRQn, 0);
	NVIC_SetPriority(gpio12_handler_IRQn, 1);
	NVIC_SetPriority(gpio13_handler_IRQn, 1);
	NVIC_SetPriority(gpio14_handler_IRQn, 1);
	NVIC_SetPriority(gpio15_handler_IRQn, 1);
	NVIC_SetPriority(gpio16_handler_IRQn, 1);
	NVIC_SetPriority(gpio17_handler_IRQn, 1);
	NVIC_SetPriority(gpio18_handler_IRQn, 1);
	NVIC_SetPriority(gpio19_handler_IRQn, 1);
	NVIC_SetPriority(gpio20_handler_IRQn, 1);
	NVIC_SetPriority(gpio21_handler_IRQn, 1);
	NVIC_SetPriority(gpio22_handler_IRQn, 1);
	
	for(i=0;i<23;i++)GPIO_SetInput(i,1);
	
	NVIC_Config(gpio0_handler_IRQn,ENABLE);
	NVIC_Config(gpio8_handler_IRQn,ENABLE);
	NVIC_Config(gpio9_handler_IRQn,ENABLE);
	NVIC_Config(gpio12_handler_IRQn,ENABLE);
	NVIC_Config(gpio13_handler_IRQn,ENABLE);
	NVIC_Config(gpio14_handler_IRQn,ENABLE);
	NVIC_Config(gpio15_handler_IRQn,ENABLE);
	NVIC_Config(gpio16_handler_IRQn,ENABLE);
	NVIC_Config(gpio17_handler_IRQn,ENABLE);
	NVIC_Config(gpio18_handler_IRQn,ENABLE);
	NVIC_Config(gpio19_handler_IRQn,ENABLE);
	NVIC_Config(gpio20_handler_IRQn,ENABLE);
	NVIC_Config(gpio21_handler_IRQn,ENABLE);
	NVIC_Config(gpio22_handler_IRQn,ENABLE);
}


