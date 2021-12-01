#include "yc11xx_gpio.h"
#include "yc11xx.h"
#include "yc_drv_common.h"
#include "yc_timer.h"

void GPIO_SetOut(GPIO_NUM gpio, BOOL BitVal)
{
	hw_delay();
	if (gpio == GPIO_MAX_NUM)
		return;
	
	if (gpio &GPIO_ACTIVE_BIT)
	{
		if (BitVal)	
			HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_OUTPUT_HIGH);
		else
			HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_OUTPUT_LOW);
	}
	else
	{
		if (BitVal)
			HWRITE(CORE_GPIO_CONF + gpio, GPCFG_OUTPUT_LOW);
		else
			HWRITE(CORE_GPIO_CONF + gpio, GPCFG_OUTPUT_HIGH);
	}
	hw_delay();
}

void GPIO_SetBits(GPIO_NUM gpio)
{
	if (gpio == GPIO_MAX_NUM)
		return;
	
	HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_OUTPUT_HIGH);
}

void GPIO_ResetBits(GPIO_NUM gpio)
{
	if (gpio == GPIO_MAX_NUM)
		return;
	
	HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_OUTPUT_LOW);
}

void GPIO_Config(GPIO_NUM gpio,uint8_t dir,uint8_t pull)
{
	if(dir == GPIO_Mode_OUT)
	{
		if(pull == GPIO_PuPd_UP)	
			HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_OUTPUT_HIGH|GPCFG_PULLUP);
		else
			HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_OUTPUT_LOW|GPCFG_PULLDOWN);
	}
	else
	{
		if(pull == GPIO_PuPd_UP)	
			HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_PULLUP);
		else
			HWRITE(CORE_GPIO_CONF + (gpio & GPIO_NUM_MASK), GPCFG_PULLDOWN);	
	}
}

void GPIO_SetInput(GPIO_NUM gpio)
{
	hw_delay();
	if (gpio == GPIO_MAX_NUM)
		return;
	
	if (gpio & GPIO_ACTIVE_BIT)
		HWRITE(CORE_GPIO_CONF +(gpio & GPIO_NUM_MASK) , GPCFG_PULLDOWN);
	else
		HWRITE(CORE_GPIO_CONF + gpio, GPCFG_PULLUP);
	hw_delay();
}

void GPIO_SetOutput(GPIO_NUM gpio)
{
	if (gpio == GPIO_MAX_NUM)
		return;

	GPIO_SetOut(gpio, FALSE);
}

void GPIO_SetWakeup(GPIO_NUM gpio)
{
	uint8_t row;
	uint8_t col;

	if (gpio == GPIO_MAX_NUM)
		return;

	row = (gpio>>3) & 0x03;
	col = (gpio & 0x07);

	if (gpio &GPIO_ACTIVE_BIT) {
		//high active,high bat will wakeup
		HWRITE(mem_gpio_wakeup_low+row,HREAD(mem_gpio_wakeup_low+row)& (~(1<<col)));
		HWRITE(mem_gpio_wakeup_high+row,HREAD(mem_gpio_wakeup_high+row) |(1<<col));
	}
	else {
		//low active, low bat will wakeup
		HWRITE(mem_gpio_wakeup_high+row,HREAD(mem_gpio_wakeup_high+row)& (~(1<<col)));
		HWRITE(mem_gpio_wakeup_low+row,HREAD(mem_gpio_wakeup_low+row) |(1<<col));
	}
}

void GPIO_SetWakeupByCurrentState(GPIO_NUM gpio)
{
	//get real status
	BOOL St = GPIO_GetInputStatus((GPIO_NUM)(gpio|GPIO_ACTIVE_BIT));
	if (St){
		GPIO_SetWakeup((GPIO_NUM)(gpio&GPIO_NUM_MASK)); //low active
	}
	else {
		GPIO_SetWakeup((GPIO_NUM)(gpio|GPIO_ACTIVE_BIT)); //high active
	}
}

void GPIO_ClearWakeup(GPIO_NUM gpio)
{
	uint8_t row;
	uint8_t col;

	if (gpio == GPIO_MAX_NUM)
		return;

	row = (gpio>>3) & 0x03;
	col = (gpio & 0x07);
	HWRITE(mem_gpio_wakeup_low+row,HREAD(mem_gpio_wakeup_low+row)& (~(1<<col)));
	HWRITE(mem_gpio_wakeup_high+row,HREAD(mem_gpio_wakeup_high+row)& (~(1<<col)));
}

void GPIO_SetGpioMultFunction(GPIO_NUM gpio, uint8_t Func)
{
	HWRITE(CORE_GPIO_CONF + (gpio & 0x3f), Func);
}


BOOL gpioGetBit(uint8_t Num, uint32_t regBase)
{
	uint8_t queue = 0;
	uint8_t group = 0;
	uint8_t st = 0;
	
	queue = (Num & 7);
	group = Num >> 3 & 3;
	st = HREAD(regBase+group);
	return (st & (1 << queue));
}

BOOL GPIO_ReadInputStatus(GPIO_NUM gpio)
{	
	if (gpio == GPIO_MAX_NUM)
	{
		error_handle();
	}
	
	return gpioGetBit(gpio&0x1f, CORE_GPIO_IN);
}


BOOL GPIO_GetInputStatus(GPIO_NUM gpio)
{	
	if (gpio == GPIO_MAX_NUM)
	{
		error_handle();
	}
	
	//return gpioGetBit(gpio&GPIO_NUM_MASK, CORE_GPIO_IN);
	if (gpio & GPIO_ACTIVE_BIT)
	{
		return gpioGetBit(gpio&0x1f, CORE_GPIO_IN);
	}
	else
	{
		return !(gpioGetBit(gpio&0x1f, CORE_GPIO_IN));
	}
}

BOOL GPIO_GetInputStatusWithJitter(GPIO_NUM gpio){
	BOOL res1,res2,res3;
	
	do{
		res1 = GPIO_GetInputStatus(gpio);
		SYS_delay_ms(10);
		res2 = GPIO_GetInputStatus(gpio);
		if (res1 != res2)
			continue;
		SYS_delay_ms(10);
		res3 = GPIO_GetInputStatus(gpio);
	}while(res2 != res3);
	
	return TRUE;
}


BOOL GPIO_CheckGpioIsInputFunction(GPIO_NUM gpio)
{
	return ((HREAD(CORE_GPIO_CONF + (gpio & 0x3f)) & 0x3f) == GPCFG_INPUT);
}

