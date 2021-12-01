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
 * uart support for application
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "yc_drv_common.h"
#include "yc11xx.h"
#include "ycdef.h"
#ifdef __cplusplus
extern "C" {
#endif

#define UARTE_BAUDRATE_BAUDRATE_Baud2400        (0xce20) /*!< 2400 baud  */
#define UARTE_BAUDRATE_BAUDRATE_Baud4800        (0xa710) /*!< 4800 baud  */
#define UARTE_BAUDRATE_BAUDRATE_Baud9600        (0x9388) /*!< 9600 baud  */
#define UARTE_BAUDRATE_BAUDRATE_Baud14400       (0x8d05) /*!< 14400 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud19200       (0x89c4) /*!< 19200 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud28800       (0x875C) /*!< 28800 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud31250       (0x8800) /*!< 31250 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud38400       (0x84e2) /*!< 38400 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud56000       (0x8359) /*!< 56000 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud57600       (0x8341) /*!< 57600 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud76800       (0x8271) /*!< 76800 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud115200      (0x81A0) /*!< 115200 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud230400      (0x80d0) /*!< 230400 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud256000      (0x80bb) /*!< 250000 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud460800      (0x8068) /*!< 460800 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud921600      (0x8034) /*!< 921600 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud512000      (0x805e) /*!< 512000 baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud1M          (0x8030) /*!< 1Mega baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud1_5M          (0x8020) /*!< 1Mega baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud1_6M          (0x801e) /*!< 1Mega baud */
#define UARTE_BAUDRATE_BAUDRATE_Baud2M          (0x8018) /*!< 2Mega baud */

#define IS_UARTE_BAUDRATE(BAUDRATE) ((BAUDRATE > 0) && (BAUDRATE < UARTE_BAUDRATE_BAUDRATE_Baud2400))

#define UARTN_BAUDRATE_BAUDRATE_Baud2400        (0x0e20) /*!< 2400 baud  */
#define UARTN_BAUDRATE_BAUDRATE_Baud4800        (0x0710) /*!< 4800 baud  */
#define UARTN_BAUDRATE_BAUDRATE_Baud9600        (0x0388) /*!< 9600 baud  */
#define UARTN_BAUDRATE_BAUDRATE_Baud14400       (0x0d05) /*!< 14400 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud19200       (0x09c4) /*!< 19200 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud28800       (0x075C) /*!< 28800 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud31250       (0x0800) /*!< 31250 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud38400       (0x04e2) /*!< 38400 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud56000       (0x0359) /*!< 56000 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud57600       (0x0341) /*!< 57600 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud76800       (0x0271) /*!< 76800 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud115200      (0x01A0) /*!< 115200 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud230400      (0x00d0) /*!< 230400 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud256000      (0x00bb) /*!< 250000 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud460800      (0x0068) /*!< 460800 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud921600      (0x0034) /*!< 921600 baud */
#define UARTN_BAUDRATE_BAUDRATE_Baud1M         	 (0x1000) /*!< 1Mega baud */

#define IS_UARTN_BAUDRATE(BAUDRATE) ((BAUDRATE > 0) && (BAUDRATE< UARTN_BAUDRATE_BAUDRATE_Baud2400))


/** @
  * @defgroup USART_Word_Length 
  */ 
#define USART_WordLength_8b                  (0<<2)
#define USART_WordLength_9b                  (1<<2)

#define IS_USART_WORD_LENGTH(LENGTH) 		(((LENGTH) == USART_WordLength_8b) || \
                                    	((LENGTH) == USART_WordLength_9b))

/** @defgroup USART_Stop_Bits 
  * @{
  */ 
#define USART_StopBits_1                 (0<<3)
#define USART_StopBits_2                 (1<<3)
#define IS_USART_STOPBITS(STOPBITS)		 (((STOPBITS) == USART_StopBits_1) ||  \
							((STOPBITS) == USART_StopBits_2) )
/**
  * @}
  */ 


  /** @defgroup USART_Parity 
  * @{
  */ 
#define USART_Parity_Even                    (0<<1)
#define USART_Parity_Odd                     (1 << 1) 
#define IS_USART_PARITY(PARITY)		 ( ((PARITY) == USART_Parity_Even) || \
                                 ((PARITY) == USART_Parity_Odd))
/**
  * @}
  */

/** @defgroup USART_Mode 
  * @{
  */ 
#define USART_Mode_Single_Line        (1<<6)
#define USART_Mode_duplex                      (0<<6)
#define IS_USART_MODE(MODE) 		(((MODE) == USART_Mode_Single_Line) ||\
				((MODE) == USART_Mode_duplex))
/**
  * @}
  */ 

/** @defgroup USART_Hardware_Flow_Control 
  * @{
  */ 
#define USART_HardwareFlowControl_None       (0<<4)
#define USART_HardwareFlowControl_ENABLE       (1<<4)


#define IS_USART_HARDWARE_FLOW_CONTROL(CONTROL)\
                              (((CONTROL) == USART_HardwareFlowControl_None) || \
                               ((CONTROL) == USART_HardwareFlowControl_ENABLE))
/**
  * @}
  */ 
	
#define uart_DMA_buf_len	512

/** @defgroup  USART_TXLen   
  * @{
  */ 
#define IS_USART_TXLen(TxLen)       ((TxLen > 0) && (TxLen <= uart_DMA_buf_len))
/**
  * @}
  */ 


/** @defgroup  USART_RXLen   
  * @{
  */ 
#define IS_USART_RXLen(RxLen)       ((RxLen > 0) && (RxLen <= uart_DMA_buf_len))
/**
  * @}
  */ 

/** 
  * @brief  USART channel define
  */ 
typedef enum
{
	UARTA = 0,
	UARTB = 1,
}USART_TypeDef;

#define IS_USARTAB(USARTx)         (USARTx == UARTA)||\
						(USARTx == UARTB)

/** 
  * @brief  USART Init Structure definition  
  */ 

/** @defgroup  USART_RXBuffer   
  * @{
  */ 
#define IS_USART_RXBuffer(RxBuffer)         ((RxBuffer > 0x10000000) ||(RxBuffer < 0x10001fff) ) |\
									((RxBuffer < 0x10004000) ||(RxBuffer > 0x10004fff))|\
									((RxBuffer < 0x10010000) ||(RxBuffer > 0x10013fff))
/**
  * @}
  */ 

/** @defgroup  USART_TXBuffer   
  * @{
  */ 
#define IS_USART_TXBuffer(TxBuffer)         ((0x10000000 < TxBuffer ) ||(TxBuffer < 0x10001fff) ) |\
									((TxBuffer < 0x10004000) ||(TxBuffer > 0x10004fff))|\
									((TxBuffer < 0x10010000) ||(TxBuffer > 0x10013fff))
/**
  * @}
  */
  
typedef struct
{
  uint32_t USART_BaudRate;            /*!< This member configures the USART communication baud rate.
                                           The baud rate is computed using the following formula:
                                            - IntegerDivider = ((PCLKx) / (8 * (OVR8+1) * (USART_InitStruct->USART_BaudRate)))
                                            - FractionalDivider = ((IntegerDivider - ((u32) IntegerDivider)) * 8 * (OVR8+1)) + 0.5 
                                           Where OVR8 is the "oversampling by 8 mode" configuration bit in the CR1 register. */

  uint16_t USART_WordLength;          /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref USART_Word_Length */

  uint16_t USART_StopBits;            /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref USART_Stop_Bits */

  uint16_t USART_Parity;              /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref USART_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */
 
  uint16_t USART_Mode;                /*!< Specifies wether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref USART_Mode */

  uint16_t USART_HardwareFlowControl; /*!< Specifies wether the hardware flow control mode is enabled
                                           or disabled.
                                           This parameter can be a value of @ref USART_Hardware_Flow_Control */
                                           
  uint16_t USART_TXLen; /*!< Specifies Tx DMA buff len */
  
  uint16_t USART_RXLen; /*!< Specifies Rx DMA buff len */

//  uint32_t USART_TxBuffer; /*!Tx ring buffer start addr */

//  uint32_t USART_RxBuffer; /*!Rx ring buffer start addr */
} USART_InitTypeDef;


void USART_Init(USART_TypeDef USARTx, USART_InitTypeDef* USART_InitStruct);

void USART_DeInit(USART_TypeDef USARTx);

void USART_SendData(USART_TypeDef USARTx, uint8_t Data);

uint8_t USART_ReceiveData(USART_TypeDef USARTx);

uint16_t USART_GetRxCount(USART_TypeDef USARTx);

uint16_t USART_ReadDatatoBuff(USART_TypeDef USARTx, uint8_t* RxBuff, uint16_t RxSize);

uint16_t USART_SendDataFromBuff(USART_TypeDef USARTx, uint8_t* TxBuff, uint16_t TxLen);

void USART_SetRxITNum(USART_TypeDef USARTx, uint8_t Num);

void USART_SetRxTimeout(USART_TypeDef USARTx, uint16_t time);

uint16_t USART_DMASendData(USART_TypeDef USARTx, uint8_t* TxBuff, uint16_t TxLen);

uint8_t USARTA_DMAGetSendStatus(USART_TypeDef USARTx);


#ifdef __cplusplus
}
#endif

#endif /* _UART_H_ */

