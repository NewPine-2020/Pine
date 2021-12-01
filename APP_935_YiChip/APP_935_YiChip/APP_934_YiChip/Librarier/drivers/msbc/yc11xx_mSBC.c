
#include "yc11xx_mSBC.h"
#include "btreg.h"


void mSBC_CheckLoop(void)
{
	//	mSBC_WaitLock();
		if(!(HREAD(mem_msbc_enc_dec_bt_lock)))
		{
			//HWRITE(0x4ffa,HREAD(0x4ffa)+1);
			if(mSBC_CheckDecPtr())
			{
				//HWRITE(0x4ffb,HREAD(0x4ffb)+1);
				//mSBC_DecodeStart();
				//if(HREAD(mem_msbc_adc_start_send))
					//mSBC_EncodeStart();
			}
		}
}
		
void mSBC_EncDecStop(void)
{
	uint16_t temp;
	HWRITE(CORE_SBC_CTRL2 ,0 );
	
	temp = HREAD(CORE_SBC_CTRL);
	temp &= 0x80;
	HWRITE(CORE_SBC_CTRL, temp);
	
	HWRITEW(CORE_SBC_SWP ,0 );
}
#if 0
void mSBC_EncodeStart(void)
{
	
}
#endif
void mSBC_EncodeStart(uint16_t sbcOutAddr, BOOL isOutMRAM, uint16_t pcmInPtr, BOOL isPCMInMRAM)
{
	uint16_t temp;
	
	HWRITEW(CORE_SBC_SADDR , pcmInPtr);	
	HWRITEW(CORE_SBC_DADDR , sbcOutAddr);
	
	temp = 0;
	if(isPCMInMRAM)
	{
		temp |= BIT_2;//sbc source buffer bit 16
	}
	else
	{
		temp &= ~(BIT_2);//sbc source buffer bit 16
	}
	if(isOutMRAM)
	{
		temp |= BIT_3;//sbc destination buffer bit 16
	}
	else
	{
		temp &= ~(BIT_3);//sbc destination buffer bit 16
	}
	temp |= BIT_7;//enable msbc encode 
	HWRITE(CORE_SBC_CTRL2, temp);
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe044, temp);

	HWRITEW(CORE_SBC_SLEN ,1+HREADW(mem_msbc_pcm_packet_len) );	
	HWRITEW(CORE_SBC_DLEN ,HREADW(mem_msbc_packet_len) );	
	HWRITE(CORE_SBC_CLR,0X06);	
	HWRITE(CORE_SBC_CLR,0);	

	temp = HREAD(CORE_SBC_CTRL2);
	temp |= BIT_0;//sbc block enable
	HWRITE(CORE_SBC_CTRL2, temp);
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe055, temp);
	
	// Load data
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_NEW_CACHE_START_ADDR );
	HWRITE(CORE_SBC_CACHE_CFG2,0x03);// bit 0£ºmram/schedule ram£¬bit 1£ºload enable
	HWRITE(CORE_SBC_CACHE_CFG2,0x01);// generate signal
	while(mSBC_UpdateCacheWait());
	HWRITEW( CORE_SBC_SWP,HREADW(mem_msbc_pcm_packet_len) );
	while(mSBC_LoadCacheWait());
	
	// Update data
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_NEW_CACHE_START_ADDR );
	HWRITE(CORE_SBC_CACHE_CFG2,0x05); // bit 0£ºmram/schedule ram£¬bit 2£ºupdate enable
	HWRITE(CORE_SBC_CACHE_CFG2,0x01);// generate signal
	while(mSBC_UpdateCacheWait());
	//DEBUG_LOG_STRING("**********mSBC encode end*******************\r\n");
	mSBC_EncDecStop();

	//USART_SendDataFromBuff(UARTA,(uint8_t *)reg_map(pcmInPtr),HREADW(mem_msbc_pcm_packet_len));
	
}


void mSBC_DecodeStart(uint16_t sbcInAddr, BOOL isSourceMRAM, uint16_t pcmOutPtr, BOOL isPCMOutMRAM)
{
	uint16_t temp;
	
	//DEBUG_LOG_STRING("mSBC_DecodeStart, sbcInAddr: 0x%02X, pcmOutPtr: 0x%02X\r\n", sbcInAddr, pcmOutPtr);
	HWRITEW(CORE_SBC_SADDR , sbcInAddr);	
	HWRITEW(CORE_SBC_DADDR , pcmOutPtr);
	HWRITE(CORE_SBC_CTRL2,0);
	temp = 0;
	if(isSourceMRAM)
	{
		temp |= BIT_2;//sbc source buffer bit 16
	}
	else
	{
		temp &= ~(BIT_2);//sbc source buffer bit 16
	}
	if(isPCMOutMRAM)
	{
		temp |= BIT_3;//sbc destination buffer bit 16
	}
	else
	{
		temp &= ~(BIT_3);//sbc destination buffer bit 16
	}
	HWRITE(CORE_SBC_CTRL2, temp);
	HWRITEW(CORE_SBC_SLEN ,1+HREADW(mem_msbc_packet_len) );	
	HWRITEW(CORE_SBC_DLEN ,HREADW(mem_msbc_pcm_packet_len) );	
	HWRITE(CORE_SBC_CLR,0X06);	
	HWRITE(CORE_SBC_CLR,0);	
	
	HWOR(CORE_SBC_CTRL,BIT_5);

	temp = HREAD(CORE_SBC_CTRL2);
	temp |= BIT_0;//sbc block enable
	HWRITE(CORE_SBC_CTRL2, temp);
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe666, sbcInAddr >> 16);
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe777, sbcInAddr);

	// Load data
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_NEW_DECODE_CACHE_START_ADDR );
	HWRITE(CORE_SBC_CACHE_CFG2,0x03);// bit 0£ºmram/schedule ram£¬bit 1£ºload enable
	HWRITE(CORE_SBC_CACHE_CFG2,0x01);// generate signal
	while(mSBC_UpdateCacheWait());

	// Wait sbc ready
	HWRITEW( CORE_SBC_SWP,HREADW(mem_msbc_packet_len) );
	while(mSBC_LoadCacheWait())
	{
		if((HREAD(CORE_SBC_STATUS)) & 0xc0)
		{
				break;
		}
	}
	// Update data
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_NEW_DECODE_CACHE_START_ADDR);//MSBC_NEW_CACHE_START_ADDR );
	HWRITE(CORE_SBC_CACHE_CFG2,0x05); // bit 0£ºmram/schedule ram£¬bit 2£ºupdate enable
	HWRITE(CORE_SBC_CACHE_CFG2,0x01);// generate signal
	while(mSBC_UpdateCacheWait());

	mSBC_EncDecStop();
	
}
uint8_t mSBC_CheckDecPtr(void)
{
	if((HREADW(mem_msbc_in_wptr)) != (HREADW(mem_msbc_in_rptr)))
		return 1; 
	else
		return 0;
}

uint8_t mSBC_UpdateCacheWait(void)
{
	return ((HREAD(CORE_MISC_STATUS)) & 0x10);
}

uint8_t mSBC_LoadCacheWait(void)
{
	if(((HREADW(CORE_SBC_SWP)) == (HREADW(CORE_SBC_SRP))))
		return 0;
	else
		return 1;
}

uint16_t mSBC_OutlenUpdate(uint16_t ptr)
{
	if((ptr+(HREADW(mem_msbc_packet_len))) >= (MSBC_PACKET_MAX_LEN+MSBC_OUT_BUFFER_ADDR))
		return (MSBC_OUT_BUFFER_ADDR);
	else
		return (ptr+(HREADW(mem_msbc_packet_len)));
}

uint16_t mSBC_InlenUpdate(uint16_t ptr)
{
	if((ptr+(HREADW(mem_msbc_packet_len))) >= (MSBC_PACKET_MAX_LEN+MSBC_IN_BUFFER_ADDR))
		return(MSBC_IN_BUFFER_ADDR);
	else
		return(ptr+(HREADW(mem_msbc_packet_len)));
}

uint16_t mSBC_PcmOutlenUpdate(uint16_t ptr)
{
	uint16_t temp ;
	temp = ptr + (HREADW(mem_msbc_pcm_packet_len));
	if(temp >= (MSBC_PCM_MAX_PACKET_LEN+MSBC_PCM_OUT_BUFFER_ADDR))
		return(MSBC_PCM_OUT_BUFFER_ADDR);
	else
	{
		HWRITEW(0x10004fd0,temp);	
//		while(1);
		__asm("nop");
		__asm("nop");
		__asm("nop");
		__asm("nop");
		__asm("nop");
		return temp;
	}	
}

uint16_t mSBC_PcmInlenUpdate(uint16_t ptr)
{
	if((ptr+(HREADW(mem_msbc_pcm_packet_len))) >= (MSBC_PCM_MAX_PACKET_LEN+MSBC_PCM_IN_BUFFER_ADDR))
		return(MSBC_PCM_IN_BUFFER_ADDR);
	else
		return(ptr+(HREADW(mem_msbc_pcm_packet_len)));
}

void mSBC_WaitLock(void)
{
	while(HREAD(mem_msbc_enc_dec_bt_lock))
		;
}

void mSBC_Lock(void)
{
	HWRITE(mem_msbc_enc_dec_bt_lock,1);
}

void mSBC_ReleaseLock(void)
{
	HWRITE(mem_msbc_enc_dec_bt_lock,0);
}

void mSBC_ClearBuf(void)
{
	uint16_t outPtr = MSBC_ENCODE_BUF_ADDRESS;
	for(uint16_t i = 0; i < MSBC_BUF_LENTH; i ++)
	{
		HWRITE(outPtr + i, 0);
	}

	outPtr = MSBC_DECODE_BUF_ADDRESS;
	for(uint16_t i = 0; i < MSBC_BUF_LENTH; i ++)
	{
		HWRITE(outPtr + i, 0);
	}
}
void mSBC_ClearOutbuf(void)
{
	uint16_t outPtr = HREADW(mem_msbc_out_wptr);
	for(uint16_t i = 0; i < MSBC_PACKET_MAX_LEN; i ++)
	{
		HWRITE(outPtr + i, 0);
	}
}
void mSBC_ClearTxbuf(void)
{
	uint16_t outPtr = (mem_sco_outdata);
	uint16_t len = HREADW(mem_sco_tx_packet_len);
	for(uint16_t i = 0; i < len; i ++)
	{
		HWRITE(outPtr + i, 0);
	}
}

uint8_t decodeOutBuffer48K[MSBC_PCM_PACKET_LEN * 3];
//uint8_t encodeOutBuffer16K[MSBC_PCM_PACKET_LEN];
void mSBC_EncodeInit(void)
{
	uint16_t rxLen = HREADW(mem_sco_rx_packet_len);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A801, rxLen);
	rxLen = rxLen - 3;
	if((rxLen) != MSBC_PACKET_LEN)
	{
		_ASSERT_FAULT();
	}
	HWRITEW(mem_msbc_packet_len, rxLen);
	
	HWRITEW(mem_msbc_pcm_packet_len, MSBC_PCM_PACKET_LEN);
	
	HWRITEW(mem_msbc_pcm_in_rptr, MSBC_NEW_ADC_8K_START_ADDR);
	
	Audio_SbcClkOn(); 

	mSBC_ClearTxbuf();

	//Filter8KTo16KInit((short *)encodeOutBuffer16K, (MSBC_PCM_PACKET_LEN >> 1));
}

void mSBC_DecodeInit(void)
{
	//Filter16KTo48KInit((short *)decodeOutBuffer48K, (MSBC_PCM_PACKET_LEN * 3 >> 1));
	//Filter16KTo8KInit((short *)reg_map_m0(MSBC_NEW_DAC_8K_START_ADDR), MSBC_NEW_PCM_MAX_BUFFER_LEN);
}
extern void Bt_MsbcPtrUpdata(void);
//extern void Bt_NRAECWorkStart(void);
#ifdef MSBC_ONLY_PASS
extern int32_t gHfpMuteDacSum;
extern uint16_t gHfpMuteDacWorkCnt;
extern uint16_t gHfpForceMuteCnt;
extern uint16_t mSbcDacOutIndex;
#endif 
void mSBC_Init(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A800, 0);
	Audio_SbcClear();
	
	mSBC_EncodeInit();
	mSBC_DecodeInit();
	
	Audio_mSBCInitDac();
	Audio_mSBCInitAdc();
	
	//DEBUG_LOG_STRING("nr aec init msbc.\r\n");
	NrAec_Init(1);
//	Bt_NRAECWorkStart();
	Bt_MsbcPtrUpdata();
#ifdef MSBC_ONLY_PASS
	gHfpMuteDacSum = 0;
	gHfpMuteDacWorkCnt = 0;
	gHfpForceMuteCnt = 0;
	mSbcDacOutIndex=0;
#endif
}
void mSBC_Stop(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A802, 0);
	Audio_mSbcStop();
}

