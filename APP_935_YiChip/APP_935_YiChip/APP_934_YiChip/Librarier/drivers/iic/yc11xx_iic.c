#include "yc11xx_iic.h"

//#define IS_IICD_CLOCKSPEED(CLOCKSPEED)  ((CLOCKSPEED>=IICD_CLOCKSPEED1MHZ) && (CLOCKSPEED<=IICD_CLOCKSPEED200KHZ))

typedef struct
{
	uint8_t Ctrl;
	uint8_t Scll;
	uint8_t Sclh;
	uint8_t Stsu;
	uint8_t Sthd;
	uint8_t Sosu;
	uint8_t Dtsu;
	uint8_t Dthd;
	uint16_t TxLen;
	uint16_t TxAddr;
	uint16_t RxAddr;
	uint16_t RxLen;
}I2C_RegDef;

void I2C_Init(IIC_InitTypeDef* I2C_InitStruct)
{
#define AUTO_DMA_AFTER_RW_BIT 		0
#define RESTART_BETWEEN_WR_BIT 	1
#define SELECT_RAM_BIT 				2
#define IICD_CTRL_VAL  ((1<<AUTO_DMA_AFTER_RW_BIT)|\
						(1<<RESTART_BETWEEN_WR_BIT)|\
						(1<<SELECT_RAM_BIT))

	I2C_RegDef* I2cAdr = NULL;
	uint16_t* Ptr = NULL;
	
	/*check parameter*/
	_ASSERT(I2C_InitStruct != NULL);
	//_ASSERT(IS_IICD_CLOCKSPEED(I2C_InitStruct->I2C_ClockSpeed));

	I2cAdr = (I2C_RegDef*)(reg_map(CORE_IICD_CTRL));
	HW_REG_8BIT((uint32_t)&I2cAdr->Ctrl, IICD_CTRL_VAL);
	
	/*set iic clock speed*/
	I2cAdr->Scll = I2C_InitStruct->scll;
	I2cAdr->Sclh = I2C_InitStruct->sclh;
	I2cAdr->Stsu = I2C_InitStruct->stsu;
	I2cAdr->Sthd = I2C_InitStruct->sthd;
	I2cAdr->Sosu = I2C_InitStruct->sosu;
	I2cAdr->Dtsu = I2C_InitStruct->dtsu;
	I2cAdr->Dthd = I2C_InitStruct->dthd;
}

void I2C_DeInit(void)
{
	I2C_RegDef* I2cAdr = NULL;
	I2cAdr = (I2C_RegDef *)(reg_map(CORE_IICD_CTRL));
}

#define START_IIC_DMA 			0x04
#define CHECK_IIC_DMA_DONE		0x20

#define IIC_TXLEN_4BYTES		 4
#define IIC_TXLEN_3BYTES		 3
#define IIC_TXLEN_2BYTES		 2

void IIC_SendData(uint8_t *Src, uint16_t len)
{
	I2C_RegDef * I2cAdr = NULL;
	uint8_t * I2cTxPtr = NULL;

	I2cAdr = (I2C_RegDef *)(reg_map(CORE_IICD_CTRL));
	I2cTxPtr = (uint8_t *)reg_map_m0(HR_REG_16BIT((uint32_t)&I2cAdr->TxAddr));

	memcpy(I2cTxPtr, Src, len);
	HW_REG_16BIT((uint32_t )&I2cAdr->TxLen, len);
	HW_REG_16BIT((uint32_t)&I2cAdr->RxLen, 0);
	
	HWRITE(CORE_DMA_START, START_IIC_DMA);
	while(!(HREAD(CORE_DMA_STATUS) & (CHECK_IIC_DMA_DONE)));
}

void IIC_ReceiveData(uint8_t *Src, uint16_t Srclen, uint8_t *Dest, uint16_t Destlen)
{
	I2C_RegDef * I2cAdr = NULL;
	uint8_t * I2cTxPtr = NULL;
	uint8_t * I2cRxPtr = NULL;

	I2cAdr = (I2C_RegDef *)(reg_map(CORE_IICD_CTRL));
	I2cTxPtr = (uint8_t *)reg_map_m0(HR_REG_16BIT((uint32_t)&I2cAdr->TxAddr));
	I2cRxPtr = (uint8_t *)reg_map_m0(HR_REG_16BIT((uint32_t)&I2cAdr->RxAddr));

	memcpy(I2cTxPtr, Src, Srclen);
	HW_REG_16BIT((uint32_t)&I2cAdr->TxLen, Srclen);
	HW_REG_16BIT((uint32_t )&I2cAdr->RxLen, Destlen);
	HWRITE(CORE_DMA_START, START_IIC_DMA);	
	while(!(HREAD(CORE_DMA_STATUS) & (CHECK_IIC_DMA_DONE)));
	memcpy(Dest, I2cRxPtr, Destlen);	
}
