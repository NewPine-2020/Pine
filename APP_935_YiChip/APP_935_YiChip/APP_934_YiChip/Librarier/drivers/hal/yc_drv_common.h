/*
 * Copyright 2016, yichip Semiconductor(shenzhen office)
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Yichip Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Yichip Semiconductor.
 */
  
/** 
  *@file common.h
  *@brief common support for application.
  */
#ifndef YC_DRV_COMMON_H
#define YC_DRV_COMMON_H

#include <string.h>
#include "yc11xx.h"
#include "ycdef.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

void HW_REG_24BIT(uint32_t reg, uint32_t data);
	
	
/**
  *@brief Writing hardware register.
  *@param reg register.
  *@param word value.
  *@return None.
  */
void HW_REG_16BIT(uint32_t reg, uint16_t word);

/**
  *@brief Reading hardware register.
  *@param reg register.
  *@return The register 16-bit value.
  */
uint16_t HR_REG_16BIT(uint32_t reg);

/**
  *@brief Reading hardware register.
  *@param reg register.
  *@return The register 24_bit value.
  */
uint32_t HR_REG_24BIT(uint32_t reg);

#define M0_LPM_REG mem_m0_lpm_flag

#define NONE_LPM_FLAG 0x00
#define KEY_LPM_FLAG 0x01
#define VP_LPM_FLAG 0x02
#define LED_LPM_FLAG 0x04
#define LINK_LPM_FLAG 0x10
#define CHARGER_LPM_FLAG 0x20
#define M0_LPM_FLAG 0x40
#define HIBER_LPM_FLAG 0x80
#define IGNORE_LPM_FLAG (BT_POWERON_LINK_PERFORMANCE_LPM_FLAG|LINK_LPM_FLAG|HIBER_LPM_FLAG)
#define PWM_LPM_FLAG 0x100
#define TWS_SYNC_PWR_OFF_LPM_FLAG 0x200
#define IPHONE_INSTORAGE_LPM_FLAG 0x400
#define OAL_LPM_FLAG	0x800
#define BT_POWERON_LINK_PERFORMANCE_LPM_FLAG	0x1000
#define APP_FIRST_POWERON_LPM_FLAG	0x2000


void error_handle(void);
void xmemcpy(uint8_t* dest,const uint8_t* src, uint16_t len);

void Lpm_LockLpm(uint16_t lpmNum);
void Lpm_unLockLpm(uint16_t lpmNum);
BOOL Lpm_CheckLpmFlag(void);
void whileDelay(int delayValue);
void whileDelayshort(int delayValue);

BOOL xramcmp(volatile uint8_t *op1,volatile uint8_t *op2, int len);
void xramcpy(volatile uint8_t *des,volatile uint8_t *src,int len);
uint32_t math_abs(int32_t value);
#ifdef __cplusplus
}
#endif

#endif

