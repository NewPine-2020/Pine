#include "Drv_cvsd.h"
#include "yc11xx.h"

DRV_CVSD_PARAM gCVSDParam = 
{
	0,	//int mCVSDLen;
	0,	//uint8_t * mCvsdOutputPtr;
	0,  //uint8_t * mCvsdInputPtr;
	0,	//uint8_t * mPcminputdataPtr;
	0,	//uint8_t * mPcmOutputdataPtr;
	CVSD_DECODE_8K_MODE_ENABLE| CVSD_DECODE_48K_MODE_ENABLE|CVSD_SELECT_MRAM|CVSD_64TO48K_ENABLE|CVSD_DMA_ENABLE,	//uint8_t mCVSDCtrl;
};
void Init_filter_ram()
{
	HWRITEW(0x8848,0x4f9);
	HWRITEW(0x884a,0x4f9);
	HWRITEW(0x884c,0x4f9);
	HWRITEW(0x886c,0x4f9);
	HWRITEW(0x888c,0x4f9);
	HWRITEW(0x88ac,0x4f9);
	HWRITEW(0x88ae,0x204);
	
	HWRITEW(0x88b0,0x0506);
	HWRITEW(0x88b2,0x0504);
	HWRITEW(0x88b4,0x0200);
	HWRITEW(0x88b6,0xFEFB);
	HWRITEW(0x88b8,0xFAF9);
	HWRITEW(0x88ba,0xFAFB);
	HWRITEW(0x88bc,0xFD00);
	HWRITEW(0x88be,0x0408);
	
	HWRITEW(0x88c2,0x0b09);
	HWRITEW(0x88c4,0x0500);
	HWRITEW(0x88c6,0xf8f1);
	HWRITEW(0x88c8,0xece8);
	HWRITEW(0x88ca,0xe9eb);
	HWRITEW(0x88cc,0xf500);
	HWRITEW(0x88ce,0x1223);
	
	HWRITEW(0x88d0,0x374b);
	HWRITEW(0x88d2,0x5e71);
	HWRITEW(0x88d4,0x787f);
}
	

uint32_t mem_adc_start_addr;
uint32_t mem_adc_buf_size;
uint32_t mem_cvsd_out_addr_ptr;
uint32_t mem_adc_buf_size;
uint32_t mem_adc_buf_size;
uint32_t mem_adc_buf_size;

void Drv_cvsd_init()
{
	//clock enable
	HWCORW(CORE_CLKOFF,CLOCK_OFF_VOICE_FILTER);
	HWCORW(CORE_CLKOFF,CLOCK_OFF_SBC);
	HWCORW(CORE_CLKOFF,CLOCK_OFF_MRAM);

	//aduio_adc_cvsd_init
	mem_adc_start_addr = PCM_IN_BUFFER_ADDR;
	mem_adc_buf_size = CVSD_IN_MAX_LEN << 1;
	
	HWRITEW(core_cvsdout_saddr, CVSD_OUT_BUFFER_ADDR);
	mem_cvsd_out_addr_ptr = CVSD_OUT_BUFFER_ADDR;

	mem_cvsd_out_addr_end = CVSD_OUT_BUFFER_ADDR + CVSD_IN_MAX_LEN;
	HWRITEW(core_cvsd_buf_len, CVSD_IN_MAX_LEN - 1);

	Drv_cvsd_adc_enable_8K();


	// cvsd filter enable, it will reuse sbc resource. 1: for cvsd; 0: for sbc/msbc
	HWOR(core_sbc_cache_cfg2,0x80);
	
	HWRITEW(core_cvsdin_saddr, CVSD_IN_BUFFER_ADDR);
	mem_cvsd_in_addr_ptr = CVSD_IN_BUFFER_ADDR;
	mem_cvsd_in_addr_end = CVSD_IN_BUFFER_ADDR + CVSD_IN_MAX_LEN;


	// PCM out setting
	HWRITEW(core_pcmout_8k_saddr, PCM_8K_OUT_BUFFER_ADDR);
	HWRITEW(core_pcmout_48k_saddr, PCM_48K_OUT_BUFFER_ADDR);

	HWRITEW(core_cvsd_buf_len, CVSD_IN_MAX_LEN-1);
	
	HWOR(core_cvsd_ctrl,CVSD_FILTER_DELAY_CHAIN_CLEAN_ENABLE);
	//audio_cvsd_coef_init
	//cvsd_wait_cvsd_filter_clr_done

	HWRITEW(core_cvsd_grp_len, HREADW(mem_sco_rx_packet_len));//input,pcm sour

/*
	HWRITEW(CORE_PCMIN_SADDR, gCVSDParam.mPcminputdataPtr);//input,pcm sour
	HWRITEW(CORE_CVSDOUT_SADDR, gCVSDParam.mCvsdOutputPtr); //output,cvsd dest

	HWRITEW(CORE_CVSDIN_SADDR, gCVSDParam.mCvsdOutputPtr); //in,cvsd source
	//HWRITEW(CORE_CVSDIN_SADDR, gCVSDParam.mCvsdInputPtr); //in,cvsd source
	HWRITEW(CORE_PCMOUT_SADDR, gCVSDParam.mPcmOutputdataPtr);//output,pcm dest
	HWRITEW(CORE_CVSD_LEN, gCVSDParam.mCVSDLen-1); //len
	*/

	Drv_cvsd_enable();
}

void Drv_cvsd_adc_enable_8K()
{
	HWRITE(core_adcd_delay, 0x80);
	HWRITE(core_adcd_ctrl, 0);
	// select mram
	HWOR(core_adcd_ctrl,0x40);

	//enable adc
	HWOR(core_adcd_ctrl,0x80);
}

void Drv_cvsd_adc_enable_16K()
{
	HWRITE(core_adcd_delay, 0x80);
	HWRITE(core_adcd_ctrl, 0x10);
	// select mram
	HWOR(core_adcd_ctrl,0x40);

	//enable adc
	HWOR(core_adcd_ctrl,0x80);
}
void Drv_cvsd_enable()
{
	HWOR(CORE_CVSD_CTRL, gCVSDParam.mCVSDCtrl | CVSD_GROUP_ENABLE);
}

void Drv_cvsd_disable()
{
	HWRITE(CORE_CVSD_CTRL, gCVSDParam.mCVSDCtrl);
}

uint8_t * Drv_cvsd_get_pcmdata_ptr()
{
	return 0;
}

uint8_t * Drv_cvsd_get_cvsddata_ptr()
{
	return 0;
}

