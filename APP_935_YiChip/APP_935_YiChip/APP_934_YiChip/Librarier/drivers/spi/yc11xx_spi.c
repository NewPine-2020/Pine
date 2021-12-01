/*
 * Copyright 2016, yichip Semiconductor(shenzhen office)
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Yichip Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Yichip Semiconductor.
 */
#include "yc11xx_spi.h"
#include <stdlib.h>
#include <string.h>

void SPI_Init(SPI_InitTypeDef* SPI_InitStruct)
{
#define SPI_AUTO_INCR_ADDR		((uint8_t)1<<6)
	register uint16_t regspictrl = 0;
	register uint16_t regspidelay = 0;

	_ASSERT(IS_SPI_BAUDRATE_PRESCALER(SPI_InitStruct->BaudRatePrescaler));
	_ASSERT(IS_SPI_CPOL(SPI_InitStruct->CPOL));
	_ASSERT(IS_SPI_CPHA(SPI_InitStruct->CPHA));
	_ASSERT(IS_SPI_RW_Delay(SPI_InitStruct->RW_Delay));
	_ASSERT(IS_SPI_MEMORY(SPI_InitStruct->RAM_SELECT));
	
	regspictrl = SPI_InitStruct->BaudRatePrescaler| \
               SPI_InitStruct->CPOL | \
               SPI_InitStruct->CPHA | \
               SPI_AUTO_INCR_ADDR;
	
	regspidelay = SPI_InitStruct->RW_Delay| \
                SPI_InitStruct->RAM_SELECT;
	
	HWRITE((uint32_t)CORE_SPID_CTRL, regspictrl);
	HWRITE((uint32_t)CORE_SPID_DELAY, regspidelay);
}

void SPI_SendAndReceiveData(uint8_t *TxBuff, uint16_t TxLen, uint8_t *RxBuff, uint16_t RxLen)
{
	volatile int  j;
	
	HWRITEW(CORE_SPID_TXADDR ,(int)TxBuff );
	HWRITEW(CORE_SPID_TXLEN , TxLen);
	HWRITEW(CORE_SPID_RXADDR , (int)RxBuff);
	HWRITEW(CORE_SPID_RXLEN , RxLen);
	HWRITE(CORE_DMA_START , 2);
	for(j = 0;j < 20;j++);
	
	while(!(HREAD(CORE_DMA_STATUS) & 0x40) );
}
