
#include "drv_msbc.h"
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
				mSBC_DecodeStart();
				if(HREAD(mem_msbc_adc_start_send))
					mSBC_EncodeStart();
			}
		}
}
		
void mSBC_EncDecStop(void)
{
	HWRITE(CORE_SBC_CTRL2 ,0 );
	HWRITE(CORE_SBC_CTRL ,0 );
	HWRITEW(CORE_SBC_SWP ,0 );
	//Audio_mSbcStop();	
}


void mSBC_EncodeStart(void)
{
	uint16_t temp;
	HWRITEW(CORE_SBC_SADDR ,HREADW(mem_msbc_pcm_in_rptr) );	
	HWRITEW(CORE_SBC_DADDR ,HREADW(mem_msbc_out_wptr) );
	HWRITE(CORE_SBC_CTRL2,0x80);
	HWRITEW(CORE_SBC_SLEN ,1+HREADW(mem_msbc_pcm_packet_len) );	
	HWRITEW(CORE_SBC_DLEN ,HREADW(mem_msbc_packet_len) );	
	HWRITE(CORE_SBC_CLR,0X06);	
	HWRITE(CORE_SBC_CLR,0);	

	HWRITE(CORE_SBC_CTRL2,0X81);	
	
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_ENCODE_BUF_ADDRESS );
	HWRITE(CORE_SBC_CACHE_CFG2,0x02);	
	HWRITE(CORE_SBC_CACHE_CFG2,0);		
	while(mSBC_UpdateCacheWait());
	HWRITEW( CORE_SBC_SWP,HREADW(mem_msbc_pcm_packet_len) );
	while(mSBC_LoadCacheWait());
	
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_ENCODE_BUF_ADDRESS );
	HWRITE(CORE_SBC_CACHE_CFG2,4);	
	HWRITE(CORE_SBC_CACHE_CFG2,0);	
	while(mSBC_UpdateCacheWait());
	
	mSBC_EncDecStop();
	temp = mSBC_OutlenUpdate(HREADW(mem_msbc_out_wptr));
	HWRITEW(mem_msbc_out_wptr ,temp);
	temp = mSBC_PcmInlenUpdate(HREADW(mem_msbc_pcm_in_rptr));
	HWRITEW(mem_msbc_pcm_in_rptr, temp);
	
}


void mSBC_DecodeStart(void)
{
	uint16_t temp;
	
	HWRITEW(CORE_SBC_SADDR ,HREADW(mem_msbc_in_rptr) );	
	HWRITEW(CORE_SBC_DADDR ,HREADW(mem_msbc_pcm_out_wptr) );
	HWRITE(CORE_SBC_CTRL2,0);
	HWRITEW(CORE_SBC_SLEN ,1+HREADW(mem_msbc_packet_len) );	
	HWRITEW(CORE_SBC_DLEN ,HREADW(mem_msbc_pcm_packet_len) );	
	HWRITE(CORE_SBC_CLR,0X06);	
	HWRITE(CORE_SBC_CLR,0);	

	HWRITE(CORE_SBC_CTRL,0x20);	
	HWRITE(CORE_SBC_CTRL2,0x01);	
	
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_DECODE_BUF_ADDRESS );
	HWRITE(CORE_SBC_CACHE_CFG2,0x02);	
	HWRITE(CORE_SBC_CACHE_CFG2,0);		
	while(mSBC_UpdateCacheWait());

	HWRITEW( CORE_SBC_SWP,HREADW(mem_msbc_packet_len) );
	while(mSBC_LoadCacheWait())
		if((HREAD(CORE_SBC_STATUS)) & 0xc0)
				break ;
	
	HWRITEW(CORE_SBC_CACHE_CFG0 ,MSBC_DECODE_BUF_ADDRESS );
	HWRITE(CORE_SBC_CACHE_CFG2,4);	
	HWRITE(CORE_SBC_CACHE_CFG2,0);	
	while(mSBC_UpdateCacheWait());
	
	mSBC_EncDecStop();
	temp = mSBC_PcmOutlenUpdate(HREADW(mem_msbc_pcm_out_wptr));
	HWRITEW(mem_msbc_pcm_out_wptr ,temp);
	HWRITEW(CORE_DAC_WPTR ,temp);

	temp = mSBC_InlenUpdate(HREADW(mem_msbc_in_rptr));	
	HWRITEW(mem_msbc_in_rptr,temp);	
	
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

