#ifndef _YC11XX_BT_SCO_H_
#define _YC11XX_BT_SCO_H_
#include "yc11xx.h"
#include "app_common.h"
#include "yc11xx_bt_interface.h"
#include "yc_debug.h"
#include "yc11xx_timer.h"
#include "yc11xx_audio.h"
#include "app_config.h"
#include "noise_suppression_x.h"
#include "yc11xx_dev_voiceprompt.h"
#include "aecm.h"


#define M0_SCO_BUF_START_ADDR    (reg_map(HREADADDR3(mem_param_m0_respin_sco_buffer_start_addr)))

#define M0_SCO_RX_BUFFER_SIZE 0x03
#define M0_SCO_TX_BUFFER_SIZE 0x03

#define M0_CVSD_START_FLAG_ENABLE 0x01
#define M0_CVSD_START_FLAG_DISABLE 0x00

typedef struct{
	uint8_t bufferValid;
	uint8_t bufferData[60];
}__attribute__((packed)) BT_SCO_RX_DATA;
typedef struct{
	uint8_t rptr;
	uint8_t wptr;
	BT_SCO_RX_DATA scoRxData[M0_SCO_RX_BUFFER_SIZE];
}__attribute__((packed)) BT_SCO_RX_DATA_FIFO;
typedef struct{
	uint8_t bufferData[60];
}__attribute__((packed)) BT_SCO_TX_DATA;
typedef struct{
	uint8_t rptr;
	uint8_t wptr;
	BT_SCO_TX_DATA scoTxData[M0_SCO_TX_BUFFER_SIZE];
}__attribute__((packed)) BT_SCO_TX_DATA_FIFO;




typedef struct{
	BT_SCO_RX_DATA_FIFO rxDataFifo;
	BT_SCO_TX_DATA_FIFO txDataFifo;
}__attribute__((packed)) BT_SCO_DATA_FIFO;

typedef struct{
	BT_CLK_INFO rxTimer[M0_SCO_RX_BUFFER_SIZE];
}__attribute__((packed)) BT_SCO_DATA_RX_TIMER;



typedef struct{
	uint8_t bufferValid;
	BT_CLK_INFO rxClk;
	uint8_t bufferData[60];
}__attribute__((packed)) OAL_BT_SCO_RX_DATA;
#define OAL_M0_SCO_BUFFER_SIZE 6 
typedef struct{
	uint8_t rptr;
	uint8_t wptr;
	OAL_BT_SCO_RX_DATA scoRxData[OAL_M0_SCO_BUFFER_SIZE];
}__attribute__((packed)) OAL_BT_SCO_RX_DATA_FIFO;
typedef struct{
	uint8_t rptr;
	uint8_t wptr;
	BT_SCO_TX_DATA scoTxData[OAL_M0_SCO_BUFFER_SIZE];
}__attribute__((packed)) OAL_BT_SCO_TX_DATA_FIFO;
typedef struct{
	OAL_BT_SCO_RX_DATA_FIFO rxDataFifo;
	OAL_BT_SCO_TX_DATA_FIFO txDataFifo;
}__attribute__((packed)) OAL_BT_SCO_DATA_FIFO;


typedef struct{
	uint32_t dac_rptr;
	uint32_t adc_wptr;
	uint8_t nr_flag;
}__attribute__((packed)) BT_NR_AEC_WORK_INFO;


typedef struct {
	uint32_t dac_wptr;
	uint32_t dac_rptr;

}__attribute__((packed)) BT_SBC_DAC_INFO;

void Bt_HfpEnable(void);
void Bt_HfpDisable(void);
void Bt_HfpRestart(void);
void Bt_SCOTimerConfig(uint32_t periodUs);
void Bt_SCODataProcess(void);
void Bt_SCOPushToLocalRxBuffer(uint8_t isRxPacket, uint8_t isRxBasebandPacket
	, volatile uint8_t* dataPtr, volatile BT_CLK_INFO* rxTimerPtr);
void Bt_SCOPopFromLocalTxBuffer(uint8_t* dataPtr);
void Bt_SCOInit(void);
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
void Bt_NRAECWorkStart(void);
void Bt_NRAECAdcReinit(uint8_t calltype);
void Bt_NRAECAdcInitWithTwsSync(void);
uint16_t Bt_NRAECGetAdcCurrentBufferSize(void);
uint16_t Bt_GetAdcBufferSizeBeforeNRAEC(void);
void Bt_NRAECDataProcess(void);
void BT_NRProcess(void);
void BT_AECProcess(void);
uint16_t  BT_AECDacPtrJudge(void);
#endif
uint16_t Bt_SCOGetAdcCurrentBufferSize(void);

void Bt_HFPStartDataPolling(void);
void Bt_CVSDDataPolling(void);
void Bt_SCOPushToLocalTxBuffer(uint8_t isTxPacket, volatile uint8_t* dataPtr);
uint8_t Bt_CvsdStartPacketCache(void);
uint8_t Bt_CheckAllowCVSD(void);
void Bt_CvsdStartWork(void);
uint16_t Bt_GetSeqByClkInSco(BT_CLK_INFO clke);
void Bt_CvsdWorkEnd(void);


#ifdef FUNCTION_CM0_BEFORECVSD_ADC_DEBUG
void Bt_CVSDbeforedataprint(void);
#endif
// extern function
extern void SCO_NVIC_Config(void);
extern void SCO_NVIC_DeConfig(void);
extern void TIMER6_IRQHandler(void);
void Bt_MsbcPtrUpdata(void);

void Bt_mSBCUpdateTxSeq(void);
void Bt_mSBCDataPolling(void);
uint8_t Bt_CheckAllowmSBC(void);
void Bt_SCODataPolling(void);
#endif //_YC11XX_BT_INTERFACE_H_

 
