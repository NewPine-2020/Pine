#include "yc11xx_uart.h"

typedef struct 
{
	uint16_t Baudrate;
	uint16_t RxSadr;
	uint16_t RxEadr;
	uint16_t RxRptr;
	uint16_t TxSadr;
	uint16_t TxEadr;
	uint16_t TxWptr;
}UartxRegDef;

typedef struct
{
	UartxRegDef rbu;
	uint8_t cbu;
}UartxRegControlBackup;

#define UART_PER_NUM 2

UartxRegControlBackup regBeck[UART_PER_NUM];




uint8_t uartA_TX_buf[uart_DMA_buf_len]={0};
uint8_t uartA_RX_buf[uart_DMA_buf_len] = {0};
uint8_t uartB_TX_buf[uart_DMA_buf_len] = {0};
uint8_t uartB_RX_buf[uart_DMA_buf_len] = {0};

void USART_Init(USART_TypeDef USARTx, USART_InitTypeDef* USART_InitStruct)
{	
	uint8_t CtrValue = 0;
	void *Ptr = NULL;
	uint16_t UartxCtrlAdr = 0;
	UartxRegDef *UartAdr = NULL;
	HWOR(CORE_UART_CLKSEL, 1);
	HWCOR(CORE_CLKOFF + 1, 0x80);
	
#define UARTC_BIT_ENABLE (1<<0)
#define BAUD_USE_SETTING (1<<7)
	
    /*check parameter*/
	_ASSERT(USART_InitStruct != NULL);
	_ASSERT(IS_USARTAB(USARTx));
//	log_print("\r\n is=%d-%x",IS_UARTE_BAUDRATE(USART_InitStruct->USART_BaudRate),USART_InitStruct->USART_BaudRate);
	//_ASSERT(IS_UARTE_BAUDRATE(USART_InitStruct->USART_BaudRate));
	_ASSERT(IS_USART_WORD_LENGTH(USART_InitStruct->USART_WordLength));
	_ASSERT(IS_USART_STOPBITS(USART_InitStruct->USART_StopBits));
	_ASSERT(IS_USART_PARITY(USART_InitStruct->USART_Parity));
	_ASSERT(IS_USART_MODE(USART_InitStruct->USART_Mode));
	_ASSERT(IS_USART_HARDWARE_FLOW_CONTROL(USART_InitStruct->USART_HardwareFlowControl));
	_ASSERT(IS_USART_TXLen(USART_InitStruct->USART_TXLen));
	_ASSERT(IS_USART_RXLen(USART_InitStruct->USART_RXLen));

    /*init baud backup*/
	regBeck[USARTx].rbu.Baudrate = USART_InitStruct->USART_BaudRate;
	if (USARTx == UARTA) {
	/*init tx ring buffer backup*/
	Ptr = uartA_TX_buf;
	regBeck[USARTx].rbu.TxSadr = (uint32_t)Ptr;
	regBeck[USARTx].rbu.TxEadr = (uint32_t)Ptr + USART_InitStruct->USART_TXLen - 1;

    /*init rx ring buffer backup*/
	Ptr = uartA_RX_buf;
	regBeck[USARTx].rbu.RxSadr = (uint32_t)Ptr;
	regBeck[USARTx].rbu.RxEadr = (uint32_t)Ptr + USART_InitStruct->USART_RXLen - 1;
	}
	else {
		/*init tx ring buffer backup*/
		Ptr = uartB_TX_buf;
		regBeck[USARTx].rbu.TxSadr = (uint32_t)Ptr;
		regBeck[USARTx].rbu.TxEadr = (uint32_t)Ptr + USART_InitStruct->USART_TXLen - 1;

    /*init rx ring buffer backup*/
		Ptr = uartB_RX_buf;
		regBeck[USARTx].rbu.RxSadr = (uint32_t)Ptr;
		regBeck[USARTx].rbu.RxEadr = (uint32_t)Ptr + USART_InitStruct->USART_RXLen - 1;
	}

	CtrValue =  USART_InitStruct->USART_Mode | USART_InitStruct->USART_HardwareFlowControl |\
	USART_InitStruct->USART_Parity |USART_InitStruct->USART_StopBits| \
	USART_InitStruct->USART_WordLength|BAUD_USE_SETTING|UARTC_BIT_ENABLE;
	
	regBeck[USARTx].cbu = CtrValue;

	if(USARTx == UARTA) {
		UartxCtrlAdr = CORE_UART_CTRL;
		UartAdr = (UartxRegDef *)(reg_map(CORE_UART_BAUD));
	}
	else {
		UartxCtrlAdr = CORE_UARTB_CTRL;
		UartAdr = (UartxRegDef *)(reg_map(CORE_UARTB_BAUD));
	}

	HWCOR(UartxCtrlAdr, 1);

    /*init all reg by backup*/
	HW_REG_16BIT(((uint32_t)(&UartAdr->Baudrate)), regBeck[USARTx].rbu.Baudrate);
	HW_REG_16BIT(((uint32_t)(&UartAdr->TxSadr)), (uint32_t)regBeck[USARTx].rbu.TxSadr);
	HW_REG_16BIT(((uint32_t)(&UartAdr->TxEadr)), (uint32_t)regBeck[USARTx].rbu.TxEadr);
	HW_REG_16BIT(((uint32_t)(&UartAdr->TxWptr)), (uint32_t)regBeck[USARTx].rbu.TxSadr);
	HW_REG_16BIT(((uint32_t)(&UartAdr->RxSadr)), (uint32_t)regBeck[USARTx].rbu.RxSadr);
	HW_REG_16BIT(((uint32_t)(&UartAdr->RxEadr)), (uint32_t)regBeck[USARTx].rbu.RxEadr);
	HW_REG_16BIT(((uint32_t)(&UartAdr->RxRptr)), (uint32_t)regBeck[USARTx].rbu.RxSadr);
	HWRITE(UartxCtrlAdr, regBeck[USARTx].cbu);
}

//void USART_Init(USART_TypeDef USARTx, USART_InitTypeDef* USART_InitStruct)
//{	
//	uint8_t CtrValue = 0;
//	void *Ptr = NULL;
//#define UARTC_BIT_ENABLE (1<<0)
//#define BAUD_USE_SETTING (1<<7)
//	HWRITE(CORE_UART_CLKSEL, 1);
//	HWCOR(CORE_CLKOFF + 1, 0x80);


//	/*check parameter*/
//	_ASSERT(USART_InitStruct != NULL);
//	_ASSERT(IS_USARTAB(USARTx));
//	_ASSERT(IS_UARTE_BAUDRATE(USART_InitStruct->USART_BaudRate));
//	_ASSERT(IS_USART_WORD_LENGTH(USART_InitStruct->USART_WordLength));
//	_ASSERT(IS_USART_STOPBITS(USART_InitStruct->USART_StopBits));
//	_ASSERT(IS_USART_PARITY(USART_InitStruct->USART_Parity));
//	_ASSERT(IS_USART_MODE(USART_InitStruct->USART_Mode));
//	_ASSERT(IS_USART_HARDWARE_FLOW_CONTROL(USART_InitStruct->USART_HardwareFlowControl));
//	_ASSERT(IS_USART_TXLen(USART_InitStruct->USART_TXLen));
//	_ASSERT(IS_USART_RXLen(USART_InitStruct->USART_RXLen));

//	/*init baud backup*/
//	regBeck[USARTx].rbu.Baudrate = USART_InitStruct->USART_BaudRate;
//	
//	/*init tx ring buffer backup*/
//	Ptr = malloc(USART_InitStruct->USART_TXLen);
//	regBeck[USARTx].rbu.TxSadr = (uint32_t)Ptr;

//	_ASSERT(Ptr != NULL);	
//	regBeck[USARTx].rbu.TxEadr = ((uint32_t)Ptr) + USART_InitStruct->USART_TXLen;
//	Ptr = NULL;
//	
//	/*init rx ring buffer backup*/
//	Ptr = malloc(USART_InitStruct->USART_RXLen);
//	regBeck[USARTx].rbu.RxSadr = (uint32_t)Ptr;
//	_ASSERT(Ptr != NULL);	
//	regBeck[USARTx].rbu.RxEadr = ((uint32_t)Ptr) + USART_InitStruct->USART_RXLen;

//	CtrValue =  USART_InitStruct->USART_Mode | USART_InitStruct->USART_HardwareFlowControl |\
//			  USART_InitStruct->USART_Parity |USART_InitStruct->USART_StopBits| \
//			  USART_InitStruct->USART_WordLength|BAUD_USE_SETTING|UARTC_BIT_ENABLE;
//	
//	regBeck[USARTx].cbu = CtrValue;

//	/*init rx ring buffer regs*/
//	if(USARTx == UARTA) {
//		usartInitByBackup(UARTA);
//		//DEV_Register(Device_Id_UartA, usartAReinit, NULL);
//	}
//	else {
//		usartInitByBackup(UARTB);
//		//DEV_Register(Device_Id_UartB, usartBReinit, NULL);
//	}
//}


void USART_DeInit(USART_TypeDef USARTx)
{
	_ASSERT(IS_USARTAB(USARTx));
	if(USARTx == UARTA) {
		HWOR(reg_map(CORE_UART_CTRL), (1<<0));
		HWOR(reg_map(CORE_UART_CTRL), (0<<0));
	}else {

		HWOR(reg_map(CORE_UARTB_CTRL), (1<<0));
		HWOR(reg_map(CORE_UARTB_CTRL), (0<<0));
	}
}



void USART_SendData(USART_TypeDef USARTx, uint8_t Data)
{
	UartxRegDef * UartAdr = NULL;
	uint16_t  WPtr = 0; 
	_ASSERT(IS_USARTAB(USARTx));

	if(USARTx == UARTA) 
	{
		 while(HREADW(reg_map(CORE_UART_TX_ITEMS)));
	}
	else
	{
		 while(HREADW(reg_map(CORE_UARTB_TX_ITEMS)));
	}
	if(USARTx == UARTA) {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UART_BAUD));	
	}else {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UARTB_BAUD));
	}
	_ASSERT((&UartAdr->TxSadr  != NULL));
	
	WPtr = HR_REG_16BIT((uint32_t)(&UartAdr->TxWptr));
	
	if(USARTx == UARTA) {
		while(HREAD(CORE_UART_TX_ITEMS) > 0);
	}else {
		while(HREAD(CORE_UARTB_TX_ITEMS) > 0);
	}
	
	HW_REG_8BIT(WPtr|M0_MEMORY_BASE,Data);
	RB_UPDATE_PTR(WPtr, HR_REG_16BIT((uint32_t)(&UartAdr->TxSadr)), HR_REG_16BIT((uint32_t)(&UartAdr->TxEadr)));	
	HW_REG_16BIT((uint32_t)(&UartAdr->TxWptr), (uint32_t)(WPtr)); 
	//HW_REG_16BIT((uint32_t)(&UartAdr->TxWptr), (uint32_t)(HR_REG_16BIT((uint32_t)(&UartAdr->TxSadr)))); 	
	
}



uint8_t USART_ReceiveData(USART_TypeDef USARTx)
{	
	UartxRegDef *UartAdr = NULL;
	uint16_t  RPtr = 0;
	uint8_t  RdData = 0;
	_ASSERT(IS_USARTAB(USARTx)); 

	if(USARTx == UARTA) {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UART_BAUD));	
	}else {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UARTB_BAUD));
	}
	RPtr = HR_REG_16BIT((uint32_t)(&UartAdr->RxRptr));
	RdData = HR_REG_16BIT(RPtr|M0_MEMORY_BASE);
	RB_UPDATE_PTR(RPtr, HR_REG_16BIT((uint32_t)(&UartAdr->RxSadr)), HR_REG_16BIT((uint32_t)(&UartAdr->RxEadr)));
	HW_REG_16BIT((uint32_t)(&UartAdr->RxRptr), RPtr);	
	return RdData;
}


uint16_t USART_GetRxCount(USART_TypeDef USARTx)
{
	_ASSERT(IS_USARTAB(USARTx));
	if(USARTx == UARTA) {
		return HR_REG_16BIT(reg_map(CORE_UART_RX_ITEMS));
	}else {
		return HR_REG_16BIT(reg_map(CORE_UARTB_RX_ITEMS));
	}
}


uint16_t USART_ReadDatatoBuff(USART_TypeDef USARTx, uint8_t* RxBuff, uint16_t RxSize)
{
	uint16_t RxLen = 0;	
    	uint16_t RPtr = 0;
    	uint16_t RdataLen = 0;
   	uint32_t RxITEMS = 0;
	UartxRegDef *UartAdr = NULL;
    	_ASSERT(IS_USARTAB(USARTx));
	_ASSERT(RxBuff != NULL);
	
	if(USARTx == UARTA) {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UART_BAUD));
		RxITEMS = reg_map(CORE_UART_RX_ITEMS);
	}else {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UARTB_BAUD));
		RxITEMS = reg_map(CORE_UARTB_RX_ITEMS);
	}
	RxLen = HR_REG_16BIT(reg_map(RxITEMS)); 
	if (RxSize!=0) {
		if (RxLen < RxSize) return 0; else RxLen = RxSize;
	}
	 if (0 == RxLen) {
        		return 0;
    	} else {
		RPtr = HR_REG_16BIT((uint32_t)(&UartAdr->RxRptr));
		for(RdataLen = 0; RdataLen<RxLen; RdataLen++ )
		{
                			
        				RxBuff[RdataLen] = HR_REG_8BIT(RPtr|M0_MEMORY_BASE);   
        				RB_UPDATE_PTR(RPtr, HR_REG_16BIT((uint32_t)(&UartAdr->RxSadr)), HR_REG_16BIT((uint32_t)(&UartAdr->RxEadr)));
		}
    	}
	HW_REG_16BIT((uint32_t)(&UartAdr->RxRptr), (RPtr));
	return RdataLen;
}


uint16_t USART_SendDataFromBuff(USART_TypeDef USARTx, uint8_t* TxBuff, uint16_t TxLen)
{
	uint16_t  WPtr = 0;
	uint16_t  SDataLen = 0;	
	uint16_t i;
	UartxRegDef *UartAdr = NULL;
	_ASSERT(IS_USARTAB(USARTx));
	_ASSERT(TxBuff != 0);
	_ASSERT(TxLen > 0);
	
	if(USARTx == UARTA) {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UART_BAUD));	
	}else {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UARTB_BAUD));
	}
   	WPtr = HR_REG_16BIT((uint32_t)(&UartAdr->TxWptr));
   	for ( i=0; i<TxLen; i++)  {	
		
		HW_REG_8BIT(WPtr|M0_MEMORY_BASE,TxBuff[i]);	
		RB_UPDATE_PTR(WPtr, HR_REG_16BIT((uint32_t)(&UartAdr->TxSadr)),  HR_REG_16BIT((uint32_t)(&UartAdr->TxEadr)));	   
		SDataLen++;	
	}
	HW_REG_16BIT((uint32_t)(&UartAdr->TxWptr),  WPtr);
		
	if(USARTx == UARTA) 
	{
		 while(HREADW(reg_map(CORE_UART_TX_ITEMS)));
	}
	else
	{
		 while(HREADW(reg_map(CORE_UARTB_TX_ITEMS)));
	}
	return SDataLen;
}



void USART_SetRxITNum(USART_TypeDef USARTx, uint8_t Num)
{
	_ASSERT(IS_USARTAB(USARTx));
	if(USARTx == UARTA) 
	{
		HWRITE(reg_map(CORE_UART_RXINTER),Num);
	}
	else
	{
		HWRITE(reg_map(CORE_UARTB_RXINTER),Num);
	}
}

void USART_SetRxTimeout(USART_TypeDef USARTx, uint16_t time)
{	
	_ASSERT(IS_USARTAB(USARTx));
	if(USARTx == UARTA) 
	{
		HWRITEW(reg_map(CORE_UART_TIMEOUT),time);
	}
	else
	{
		HWRITEW(reg_map(CORE_UARTB_TIMEOUT),time);
	}
}

uint16_t USART_DMASendData(USART_TypeDef USARTx, uint8_t* TxBuff, uint16_t TxLen)
{
	uint16_t  WPtr = 0;
	uint16_t  SDataLen = 0;	
	uint16_t i;
	UartxRegDef *UartAdr = NULL;
	_ASSERT(IS_USARTAB(USARTx));
	_ASSERT(TxBuff != 0);
	_ASSERT(TxLen > 0);

	if(USARTx == UARTA) 
	{
		 if(HREADW(reg_map(CORE_UART_TX_ITEMS)))
		 	return 0;
	}
	else
	{
		 if(HREADW(reg_map(CORE_UARTB_TX_ITEMS)))
		 	return 0;
	}	

	if(USARTx == UARTA) {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UART_BAUD));	
	}else {
		UartAdr = (UartxRegDef *)(reg_map(CORE_UARTB_BAUD));
	}
   	WPtr = HR_REG_16BIT((uint32_t)(&UartAdr->TxWptr));
   	for ( i=0; i<TxLen; i++)  {	
		
		HW_REG_8BIT(WPtr|M0_MEMORY_BASE,TxBuff[i]);	
		RB_UPDATE_PTR(WPtr, HR_REG_16BIT((uint32_t)(&UartAdr->TxSadr)),  HR_REG_16BIT((uint32_t)(&UartAdr->TxEadr)));	   
		SDataLen++;	
	}
	HW_REG_16BIT((uint32_t)(&UartAdr->TxWptr),  WPtr);
		
	return 1;
}


uint8_t USARTA_DMAGetSendStatus(USART_TypeDef USARTx)
{
	 if(HREADW(reg_map(CORE_UART_TX_ITEMS)))
	 	return 1;
	 else
	 	return 0;
}


//int fputc(unsigned char ch, FILE *f)
//{
//	USART_SendData(UARTB, (unsigned char) ch);
//	while (!(HR_REG_16BIT(reg_map(CORE_UARTB_TX_ITEMS))));
//	return (ch);
//}

