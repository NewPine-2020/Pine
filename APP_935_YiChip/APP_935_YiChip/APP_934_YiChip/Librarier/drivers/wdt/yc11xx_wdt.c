#include "yc11xx_wdt.h"
#include "ycdef.h"

WDT_InitTypeDef* gpWdtInit;
void WDT_Kick(void)
{
	WDT_InitTypeDef* WDT_init_struct = gpWdtInit;
	if(WDT_init_struct == 0)
	{
		return;
	}
	WDT_Enable();
	if (WDT == WDT_init_struct->WDTx)
			HW_REG_8BIT(reg_map(CORE_WDT), WDT_init_struct->setload);
	else if (WDT2 == WDT_init_struct->WDTx)
			HW_REG_8BIT(reg_map(CORE_WDT2), WDT_init_struct->setload);
}
void WDT_Start(WDT_InitTypeDef* WDT_init_struct)
{
	WDT_Init(WDT_init_struct);
	WDT_Enable();
}

void WDT_Init(WDT_InitTypeDef* WDT_init_struct)
{
	uint8_t en_rst;

	gpWdtInit = WDT_init_struct;
	
	en_rst = HR_REG_8BIT(reg_map(CORE_ENCRYPT));
	
	if (RESET_MODE == WDT_init_struct->mode)
	{
		en_rst &= ~(BIT_1);
		if (WDT == WDT_init_struct->WDTx)
			HW_REG_8BIT(reg_map(CORE_WDT), WDT_init_struct->setload);
		else if (WDT2 == WDT_init_struct->WDTx)
			HW_REG_8BIT(reg_map(CORE_WDT2), WDT_init_struct->setload);
	}
	else if (INTR_MODE == WDT_init_struct->mode)
	{
		en_rst |= BIT_1;
		if (WDT == WDT_init_struct->WDTx)
			HW_REG_8BIT(reg_map(CORE_WDT), WDT_init_struct->setload);
		else if (WDT2 == WDT_init_struct->WDTx)
			HW_REG_8BIT(reg_map(CORE_WDT2), WDT_init_struct->setload);
	}
	HW_REG_8BIT(reg_map(CORE_ENCRYPT), en_rst);
}

void WDT_Enable(void)
{
	uint8_t wdt_en;

	WDT_NumTypeDef WDTx = gpWdtInit->WDTx;

	wdt_en = HR_REG_8BIT(reg_map(CORE_CONFIG));
	
	if (WDT2 == WDTx)
	{
		wdt_en|= 0x08;
	}
	else if (WDT == WDTx)
	{
		wdt_en|= 0x10;
	}
	HW_REG_8BIT(reg_map(CORE_CONFIG), wdt_en);
}
void WDT_Disable(void)
{
	uint8_t wdt_en;
	
	WDT_NumTypeDef WDTx = gpWdtInit->WDTx;

	wdt_en = HR_REG_8BIT(reg_map(CORE_CONFIG));
	
	if (WDT2 == WDTx)
	{
		wdt_en &= ~(0x08);
	}
	else if (WDT == WDTx)
	{
		wdt_en &= ~(0x10);
	}
	HW_REG_8BIT(reg_map(CORE_CONFIG), wdt_en);
}

void WDT_Reload(WDT_NumTypeDef WDTx, uint8_t reload)
{
	uint8_t wdt_re;
	
	if (WDTx == WDT)
	{
		wdt_re = reload;
		HW_REG_8BIT(reg_map(CORE_WDT), wdt_re);
	}
	else if (WDTx == WDT2)
	{
		wdt_re = reload;
		HW_REG_8BIT(reg_map(CORE_WDT2), wdt_re);
	}
}
	
