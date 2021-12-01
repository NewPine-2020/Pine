/*
 * Copyright 2016, yichip Semiconductor(shenzhen office)
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Yichip Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Yichip Semiconductor.
 */

  /** @file
 *
 * spi support for application
 */
#ifndef _SPI_H_
#define _SPI_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "yc11xx.h"
#include "yc_drv_common.h"

/** @
  * @defgroup SPI_BaudSpeed 
  */ 

#define SPI_BaudRatePrescaler_2MHz       4
#define SPI_BaudRatePrescaler_750KHz     5
#define SPI_BaudRatePrescaler_375KHz     6
#define SPI_BaudRatePrescaler_187KHz     7

#define IS_SPI_BAUDRATE_PRESCALER(PRESCALER) (((PRESCALER) == SPI_BaudRatePrescaler_2MHz) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_750KHz) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_375KHz) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_187KHz))

/** @
  * @defgroup SPI_CPOL 
  */ 
#define  SPI_CPOL_Low         ((uint8_t)0<<4)
#define  SPI_CPOL_High	      ((uint8_t)1<<4)

#define IS_SPI_CPOL(CPOL) 		(((CPOL) == SPI_CPOL_High) || \
                                    	((CPOL) == SPI_CPOL_Low))


/** @
  * @defgroup SPI_CPHA 
  */ 
#define SPI_CPHA_First_Edge	  ((uint8_t)0<<5)
#define SPI_CPHA_Second_Edge	((uint8_t)1<<5)

#define IS_SPI_CPHA(CPHA) 		(((CPHA) == SPI_CPHA_First_Edge) || \
                                    	((CPHA) == SPI_CPHA_Second_Edge))

/** @
  * @defgroup SPI_RW_Delay
  */ 
#define IS_SPI_RW_Delay(RW_Delay)   (RW_Delay>0 && RW_Delay<127)

/** @
  * @defgroup SPI_MEM_SELECT
  */ 
#define SPI_SCHED_RAM	              ((uint8_t)0<<7)
#define SPI_64K_MRAM	              ((uint8_t)1<<7)

#define IS_SPI_MEMORY(RAM_SELECT)   (((RAM_SELECT) == SPI_SCHED_RAM) || \
                                    	           ((RAM_SELECT) == SPI_64K_MRAM))

/** 
  * @brief  SPI Init structure definition  
  */
typedef struct
{
	uint8_t BaudRatePrescaler;     /*!< Specifies the Baud Rate prescaler value which will be
                                      used to configure the transmit and receive SCK clock.
                                      This parameter can be a value of @ref SPI_BaudRate_Prescaler.
                                      @note The communication clock is derived from the master
                                      clock. The slave clock does not need to be set. */
																		
	uint8_t CPOL;                  /*!< Specifies the serial clock steady state.
                                      This parameter can be a value of @ref SPI_Clock_Polarity */

	uint8_t CPHA;                  /*!< Specifies the clock active edge for the bit capture.
                                      This parameter can be a value of @ref SPI_Clock_Phase */

	uint8_t RW_Delay;              /* Specifies the Delay time between send  and receive data,the 
                                    value must be 0 to 127 */
	
	uint8_t RAM_SELECT;            /* 
                                     */
}SPI_InitTypeDef;

/**
  * @brief  Initializes the SPI peripheral according to the specified
  *         parameters in the SPI_InitStruct.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure
  *         that contains the configuration information for the specified SPI 
  *         peripheral.
  * @retval None
  */
void SPI_Init(SPI_InitTypeDef* SPI_InitStruct);

/**
  * @brief    Send data first then  recerive data.
  * @param: TxBuff: pointer to a TxBuff  that contains the data you want send.
  * @param: TxLen: the length of send datas
  * @param: RxBuff: pointer to a TxBuff  that contains the data you want receive.
  * @param: RxLen: the length of receive datas
  * @retval None
  */
void SPI_SendAndReceiveData(uint8_t *TxBuff, uint16_t TxLen, uint8_t *RxBuff, uint16_t RxLen);




#endif /* _SPI_H_ */
 
