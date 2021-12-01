
#ifndef _DRV_MSBC11_H_

#define _DRV_MSBC11_H_

#include "yc11xx_gpio.h"
#include "yc11xx.h"
#include "yc11xx_audio.h"
#include "yc_debug.h"



uint8_t mSBC_UpdateCacheWait(void);
uint8_t mSBC_LoadCacheWait(void);

uint16_t mSBC_OutlenUpdate(uint16_t ptr);
uint16_t mSBC_InlenUpdate(uint16_t ptr);
uint16_t mSBC_PcmOutlenUpdate(uint16_t ptr);
uint16_t mSBC_PcmInlenUpdate(uint16_t ptr);
void mSBC_EncodeStart(uint16_t sbcOutAddr, BOOL isOutMRAM, uint16_t pcmInPtr, BOOL isPCMInMRAM);
void mSBC_DecodeStart(uint16_t sbcInAddr, BOOL isSourceMRAM, uint16_t pcmOutPtr, BOOL isPCMOutMRAM);
uint8_t mSBC_CheckDecPtr(void);
void mSBC_WaitLock(void);
void mSBC_Lock(void);
void mSBC_ReleaseLock(void);
void mSBC_CheckLoop(void);

void mSBC_ClearBuf(void);
void mSBC_ClearOutbuf(void);
void mSBC_ClearTxbuf(void);
void mSBC_EncodeInit(void);
void mSBC_DecodeInit(void);
void mSBC_Init(void);
void mSBC_Stop(void);

#endif

