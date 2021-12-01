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
  *@file iic.h
  *@brief IIC support for application.
  */
#ifndef _IIC_H_
#define _IIC_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yc11xx.h"
#include "yc_drv_common.h"

/**
  *@brief I2C_ClockSpeed.
  */ 
#define IICD_CLOCKSPEED200KHZ 26
#define IICD_CLOCKSPEED400KHZ 11
#define IICD_CLOCKSPEED800KHZ 4
#define IICD_CLOCKSPEED1MHZ 2

/** 
  *@brief I2C initialized structure definition.  
  */
/** 
  *@brief IIC Core parameters struct
  */
typedef struct{
	uint8_t scll;	/*!< Specifies the Clock Pulse Width Low. */
	uint8_t sclh;   /*!< Specifies the Clock Pulse Width High. */
	uint8_t stsu;   /*!< Specifies the Start Setup Time. */
	uint8_t sthd;   /*!< Specifies the Start Hold Time. */
	uint8_t sosu;   /*!< Specifies the Stop Setup Time. */
	uint8_t dtsu;   /*!< Specifies the Data Setup Time. */
	uint8_t dthd;   /*!< Specifies the Data Hold Time. */
}IIC_InitTypeDef;

/** 
  *@brief I2C_RegAddr.
  */
#define I2C_REGADDR8BITS(x) (0x01000000|x)
#define I2C_REGADDR16BITS(x) (0x10000000|x)

/** 
  *@brief I2C_RegAddr.
  */
#define IS_I2C_REGADDR(x) (0x11000000&x)
#define IS_I2C_REGADDR8BITS(x) (0x01000000&x)
#define IS_I2C_REGADDR16BITS(x) (0x10000000&x)

/**
  *@brief cancel IIC initialization.
  *@param None.
  *@return None.
  */
void I2C_DeInit(void);

/**
  *@brief IIC initialization.
  *@param I2C_InitStruct IIC initializing structure.@ref I2C_InitTypeDef
  *@return None.
  */
void I2C_Init(IIC_InitTypeDef* I2C_InitStruct);

/**
  * @brief	Write slave devices.  	 
  * @param  Src: pointer to the buffer that contains the data you want to send.
  * @param  len: the length of send data
  * @note	  After writting slave device, must delay a period of time before reading slave device.
  * @retval None
  */
void IIC_SendData(uint8_t *Src, uint16_t len);

/**
  * @brief	Read slave devices.    	 
  * @param  Src: pointer to the buffer that contains the data you want to send.
  * @param  len: the length of send data
  * @param  Dest: pointer to the buffer that contains the data received from slaver.
  * @param	Destlen: the length of received data
  * @retval None
  */
void IIC_ReceiveData(uint8_t *Src, uint16_t Srclen, uint8_t *Dest, uint16_t Destlen);

#endif
