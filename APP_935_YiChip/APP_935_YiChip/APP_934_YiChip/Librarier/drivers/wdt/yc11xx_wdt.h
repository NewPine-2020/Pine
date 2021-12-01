#ifndef __YC11XX_WDT_H__
#define __YC11XX_WDT_H__

#include "yc11xx.h"
#include "type.h"

/** 
  * @brief  watch dog number definition  
  */ 
typedef enum
{
  WDT2 = 2,
	WDT,
}WDT_NumTypeDef;

/** 
  * @brief  watch dog mode definition  
  */ 
typedef enum
{
	RESET_MODE = 0x02,
	INTR_MODE = 0xfd
} WDT_ModeTypedef;

#define WDT_timer_100ms  0x7F
#define WDT_timer_500ms  0x7b
#define WDT_timer_1_5s  0x75
#define WDT_timer_Max  0x00

/** 
  * @brief  watch dog Init Structure definition  
  */ 
typedef struct 
{
	WDT_NumTypeDef WDTx;
	WDT_ModeTypedef mode;
	uint8_t setload;
}WDT_InitTypeDef;

/**
 * @brief Initialize WDTx ,not start timer,use WDT_Enable start wdt
 *
 * @param WDT_init_struct : the WDT_InitTypeDef Structure
 *
 * @retval none
 */
void WDT_Init(WDT_InitTypeDef* WDT_init_struct);

/*
 * @brief:Enable WDT
 * @param:WDT_NumTypeDef : the WDT Num
 * @return: none
 */
void WDT_Enable(void);
void WDT_Disable(void);

/*
 * @brief:Reload WDT
 * @param:WDT_NumTypeDef : the WDT Num, reload(0~255)
 * @return: none
 */
void WDT_Reload(WDT_NumTypeDef WDTx, uint8_t reload);

/**
 * @brief Initialize WDTx and start timer,use WDT_Enable start wdt
 *
 * @param WDT_init_struct : the WDT_InitTypeDef Structure
 *
 * @retval none
 */
void WDT_Start(WDT_InitTypeDef* WDT_init_struct);

/**
 * @brief:kick wDT
 *
 * @param WDT_init_struct : the WDT_InitTypeDef Structure
 *
 * @retval none
 */
void WDT_Kick(void);

#endif 
