#include "yc11xx_gpio.h"
#include "yc11xx.h"



uint8_t mSBC_UpdateCacheWait(void);
uint8_t mSBC_LoadCacheWait(void);

uint16_t mSBC_OutlenUpdate(uint16_t ptr);
uint16_t mSBC_InlenUpdate(uint16_t ptr);
uint16_t mSBC_PcmOutlenUpdate(uint16_t ptr);
uint16_t mSBC_PcmInlenUpdate(uint16_t ptr);
void mSBC_EncodeStart(void);
void mSBC_DecodeStart(void);
uint8_t mSBC_CheckDecPtr(void);
void mSBC_WaitLock(void);
void mSBC_Lock(void);
void mSBC_ReleaseLock(void);
void mSBC_CheckLoop(void);


