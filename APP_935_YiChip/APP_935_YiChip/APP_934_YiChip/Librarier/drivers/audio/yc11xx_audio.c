#include "yc11xx_audio.h"
#include "noise_suppression_x.h"
#include "aecm.h"
#include "yc11xx_dev_oal.h"
#include "yc11xx_wdt.h"
#include "yc11xx_dev_flash.h"

void Audio_EqDisable(void)
{
	HWCOR(CORE_SBC_CTRL, BIT_6);
}
void Audio_EqEnable(void)
{
	HWOR(CORE_SBC_CTRL, BIT_6);
}
void Audio_DACClkOff(void)
{
	HWORW(CORE_CLKOFF, REG_CLOCK_OFF_AUDIO_DAC);
	HWORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
}

void Audio_DacStop(void)
{
	uint8_t tmp = HREAD(CORE_DAC_CTRL);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60A, tmp);

	//1. Disable DMA to pause audio data streaming
	HWCOR(CORE_DAC_CTRL, BIT_0);//DAC_ENABLE
	HWRITE(mem_dac_clk, 0);
	// sleep a wait//500us
	SYS_delay_us(500);

	//2. Wait Tramp for ramp down to finish
	//Tramp= (last_data_value/(left/right_down_step +1)*(1/48)) mS, if sample rate is 48K. The maximum time is 40ms if
	// down_step is '0xF' and sample rate is 48K. Use 8K sample rate the maximum time is 6*40=240ms.
	if((tmp & (BIT_0)) != 0)//DAC_ENABLE
	{
		Audio_DacWaitRampDown();
	}

	//Mute voice
	Audio_AdacPrepareSoftMute();

	//HWRITE(CORE_DAC_LVOL, 0);
	//HWRITE(CORE_DAC_RVOL, 0);
	Audio_EqDisable();

	Audio_ClosePaDelay();
}
void Audio_AdcStop(void)
{
	uint8_t temp;

	Audio_AnaStopAdcWork();
	
	//clear adc_fir3_scale
	temp = HREAD(CORE_TONE_CTRL);
	temp &= 0xf0;
	HWRITE(CORE_TONE_CTRL, temp);

	HWRITE(mem_cvsd_start_send, 0);
	HWRITE(mem_msbc_adc_start_send, 0);
	
	HWRITE(CORE_ADCD_DELAY, 0);
	HWRITE(CORE_ADCD_CTRL, 0);
}

void Audio_CvsdClkEnable(void)
{
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_SBC);// Only need for 64K to 48K farrow filter init(8K -> 64K -> 48K)
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_MRAM);
}
void Audio_CvsdClkDisable(void)
{
}
void Audio_CvsdWaitCvsdFilterClrDone(void)
{
	while((HREAD(CORE_PERF_STATUS) & BIT_7) == 0)
	{

	}
}
void Audio_CvsdCoefInit(void)
{
	uint16_t temp_16, i;
	uint32_t startAddr = Audio_GetCoefStartAddr(COEF_VOICE_TABLE_OFFSET);
	
	HWOR(CORE_CVSD_CTRL, BIT_1);//CVSD_FILTER_DELAY_CHAIN_CLEAN_ENABLE
	
	QSPI_ReadFlashDataToXRAM(startAddr, COEF_VOICE_TABLE_LENGTH, (uint8_t *)reg_map(mem_coef_table_temp));
	HWRITE(CORE_COEF_CTRL, 0x40);
	for(i = 0; i < 41; i ++)
	{
		temp_16 = HREADW((mem_coef_table_temp + (i * 2)));
		HWRITE24BIT(CORE_COEF_WDATA, temp_16);
		
		HWRITE(CORE_COEF_CTRL, 0x40);
		HWRITE(CORE_COEF_CTRL, 0xC0);
	}

	HWRITE(CORE_COEF_CTRL, 0x00);

	Audio_CvsdWaitCvsdFilterClrDone();
}

void Audio_CvsdStart(void)
{
	uint8_t tmp = HREAD(CORE_CVSD_CTRL);
	tmp |= BIT_4;//CVSD_DECODE_8K_MODE_ENABLE
#ifdef FUNCTION_CVSD_OVER_SOFT_8KTO48K
	tmp &= ~(BIT_5);//CVSD_DECODE_48K_MODE_ENABLE
	tmp &= ~(BIT_2);//CVSD_64TO48K_ENABLE
#else
	tmp |= BIT_5;//CVSD_DECODE_48K_MODE_ENABLE
	tmp |= BIT_2;//CVSD_64TO48K_ENABLE
#endif
	tmp |= BIT_7;//CVSD_DMA_ENABLE
	HWRITE(CORE_CVSD_CTRL, tmp);
}
void Audio_CvsdStop(void)
{
	HWRITE(CORE_CVSD_CTRL, 0);
}
uint16_t gDacBufOffset = 0;
void Audio_CvsdInitDac(void)
{
	uint32_t dacStartAddr = HREADW(CORE_PCMOUT_48K_SADDR);
	uint16_t dacClk = 48;
	uint16_t dacBufSize = (HREADW(CORE_CVSD_BUF_LEN) + 1) * 12;//6*2=12  6 means 8K to 48K
	BOOL isMram = (HREAD(CORE_CVSD_CTRL) & BIT_6) != 0;//CVSD_SELECT_MRAM
#ifdef FUNCTION_CVSD_OVER_SOFT_8KTO48K
	dacStartAddr = reg_map_m0(dacStartAddr);
	//dacStartAddr = reg_map_m0(0x7800);
	dacBufSize = 0x2000;
	gDacBufOffset = 0;
	Filter8KTo48KInit((short *)dacStartAddr, dacBufSize/2);//uint8_t to short
#endif
	
#ifdef TEST_FUNCTION_USE_SPECIAL_VOICE_ENABLE
	extern void App_TestForceVoice(void);
	App_TestForceVoice();
#else
	Audio_DacInitVpCall(dacClk, isMram
		, dacStartAddr, dacBufSize);
#endif
}
void Audio_CvsdInit(void)
{
	Audio_CvsdInitProcess();
	Audio_CvsdInitDac();
}
void Audio_CvsdInitProcess(void)
{
	uint16_t cvsdInBufferAddr = HREADW(mem_param_cvsd_in_buffer_addr);
	uint16_t cvsdInBufferEndAddr = cvsdInBufferAddr + CVSD_IN_MAX_LEN;
	
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A630, HREAD(CORE_CVSD_CTRL));

	// Enable clock
	Audio_CvsdClkEnable();

	// Init cvsd out, pcm in buffer, and init adc
	Audio_AdcCvsdInit();

	//cvsd filter enable, it will reuse sbc resource. 1: for cvsd; 0: for sbc/msbc
	HWOR(CORE_SBC_CACHE_CFG2, BIT_7);
	
	// Init cvsd in buffer addr
	HWRITEW(CORE_CVSDIN_SADDR, cvsdInBufferAddr);
	HWRITEW(mem_cvsd_in_addr_ptr, cvsdInBufferAddr);
	
	HWRITEW(mem_cvsd_in_addr_end, cvsdInBufferEndAddr);

	if(HREAD(mem_param_cvsd_in_buffer_addr_mram_flag) != 0)
	{
		HWOR(CORE_CVSD_CTRL1, BIT_2);//DECODE_CVSD_IN_MRAM_SEL
	}
	else
	{
		HWCOR(CORE_CVSD_CTRL1, BIT_2);//DECODE_CVSD_IN_MRAM_SEL
	}

	// In cvsd, it have 8K and 48K out
	HWRITEW(CORE_PCMOUT_SADDR, HREADW(mem_param_pcm_8k_out_buffer_addr));
	
	// mram setting
	if(HREAD(mem_param_pcm_8k_out_buffer_addr_mram_flag) != 0)
	{
		HWOR(CORE_CVSD_CTRL1, BIT_1);//DECODE_8KPCM_OUT_MRAM_SEL
	}
	else
	{
		HWCOR(CORE_CVSD_CTRL1, BIT_1);//DECODE_8KPCM_OUT_MRAM_SEL
	}

	HWRITEW(CORE_PCMOUT_48K_SADDR, HREADW(mem_param_pcm_48k_out_buffer_addr));
	
	// mram setting
	if(HREAD(mem_param_pcm_48k_out_buffer_addr_mram_flag) != 0)
	{
		HWOR(CORE_CVSD_CTRL, BIT_6);//CVSD_SELECT_MRAM
	}
	else
	{
		HWCOR(CORE_CVSD_CTRL, BIT_6);//CVSD_SELECT_MRAM
	}

	HWRITEW(CORE_CVSD_BUF_LEN, CVSD_IN_MAX_LEN - 1);

	// clear and init cvsd filter
	Audio_CvsdCoefInit();

 	// cvsd encode and decode will work with the length
 	HWRITEW(CORE_CVSD_GRP_LEN, HREADW(mem_sco_rx_packet_len));

	// Only need for 64K to 48K farrow filter init(8K -> 64K -> 48K), after init, not need any more
	HWORW(CORE_CLKOFF, REG_CLOCK_OFF_SBC);

	Audio_CvsdStart();
}
void Audio_AdcCvsdInit(void)
{
	uint16_t adcStartAddr, adcBufferSize, cvsdOutBufferAddr;
	BOOL adcMramFlag, cvsdOutMramFlag;
	adcStartAddr = HREADW(mem_param_pcm_in_buffer_addr);
	adcBufferSize = CVSD_IN_MAX_LEN * 2;
	adcMramFlag = HREAD(mem_param_pcm_in_buffer_addr_mram_flag);
	
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A631, adcStartAddr);

	//
#ifdef FUNCTION_OAL_MODE
	if(Oal_CheckIsFunctionEnable())
	{
		Oal_Start(OALADCBUFFBYCALL);
	}
	else
#endif
	{
		Audio_AdcInit(adcStartAddr, adcBufferSize, adcMramFlag,0);
	}

	// PCM In Setting
	HWRITEW(CORE_PCMIN_SADDR, adcStartAddr);
	HWRITEW(mem_pcm_in_addr_end, (adcStartAddr + adcBufferSize - 1));
	
	if(adcMramFlag)
	{
		HWOR(CORE_CVSD_CTRL1, BIT_0);//ENCODE_8KPCM_IN_MRAM_SEL
	}
	else
	{
		HWCOR(CORE_CVSD_CTRL1, BIT_0);//ENCODE_8KPCM_IN_MRAM_SEL
	}

	cvsdOutBufferAddr = HREADW(mem_param_cvsd_out_buffer_addr);

	HWRITEW(CORE_CVSDOUT_SADDR, cvsdOutBufferAddr);
	HWRITEW(mem_cvsd_out_addr_ptr, cvsdOutBufferAddr);
	HWRITEW(mem_cvsd_out_addr_end, cvsdOutBufferAddr + CVSD_IN_MAX_LEN);
	cvsdOutMramFlag = HREAD(mem_param_cvsd_out_buffer_addr_mram_flag);
	
	if(cvsdOutMramFlag)
	{
		HWOR(CORE_CVSD_CTRL1, BIT_3);//ENCODE_CVSD_OUT_MRAM_SEL
	}
	else
	{
		HWCOR(CORE_CVSD_CTRL1, BIT_3);//ENCODE_CVSD_OUT_MRAM_SEL
	}

	HWRITEW(CORE_CVSD_BUF_LEN, CVSD_IN_MAX_LEN  - 1);
#ifndef FUNCTION_OAL_MODE
	NrAec_Init(0);
#endif
}
void NrAec_Init(uint8_t calltype)
{
#ifdef FUNCTION_CONTROL_AECNR_ENABLE

	if(calltype)//msbc
	{
		//nsProcess_init();       //ns init
		gpAECStructVariable->gechoMode =3;
		gpAECStructVariable->gAecMobilePtr = AECProcess_init(16000,128,gpAECStructVariable->gechoMode);    //AEC init
	}
	else//cvsd
	{
		nsProcess_init();       //ns init
		if (HREAD(mem_test_mode) == 1)
			gpAECStructVariable->gechoMode =3;
		else
			gpAECStructVariable->gechoMode =4;
		gpAECStructVariable->gAecMobilePtr = AECProcess_init(8000,64,gpAECStructVariable->gechoMode);    //AEC init
	}
#endif
}
void Audio_AdcInit(uint16_t adcStartAddr, uint16_t adcBufferSize, BOOL adcMramFlag,BOOL adcIs16K)
{
	uint8_t temp;
	uint16_t adcEndAddr = (adcStartAddr + adcBufferSize - 1);

	Audio_AnaStartAdcWork();

	//Step1: init adc
	Audio_AdcFilterInit();

	HWRITEW(mem_adc_start_addr, adcStartAddr);
	HWRITEW(mem_adc_buf_size, adcBufferSize);

	//Step2: init adc buffer info
	HWRITEW(CORE_ADCD_SADDR, adcStartAddr);
	
	HWRITEW(CORE_ADCD_EADDR, adcEndAddr);

	// ADC SCAL setting
	//HWRITE(CORE_ADC_FILTER_CTRL, 0x28);
	HWRITE(CORE_ADC_FILTER_CTRL, 0x38);
	temp = HREAD(CORE_TONE_CTRL);
	temp &= 0xf0;
	//temp |= 0x0e;
	temp |= 0x0f;
	HWRITE(CORE_TONE_CTRL, temp);

	if(adcIs16K)
	{
		Audio_AdcStart16K(adcMramFlag);
	}
	else
	{
		Audio_AdcStart8K(adcMramFlag);
	}
}

void Audio_AdcDmaDisable(void)
{
	HWCOR(CORE_ADCD_CTRL, BIT_7);
}
void Audio_AdcDmaEnable(void)
{
	HWOR(CORE_ADCD_CTRL, BIT_7);
}
BOOL Audio_CheckAdcDmaEnable(void)
{
	return (HREAD(CORE_ADCD_CTRL) & BIT_7) != 0;
}
void Audio_AdcStart8K(BOOL adcMramFlag)
{
	HWRITE(CORE_ADCD_DELAY, 0x80);
	HWRITE(CORE_ADCD_CTRL, 0);

	if(adcMramFlag)
	{
		HWOR(CORE_ADCD_CTRL, BIT_6);
	}
	else
	{
		HWCOR(CORE_ADCD_CTRL, BIT_6);
	}
	// Start voice filter clock
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
	Audio_AdcDmaEnable();
}
void Audio_AdcFilterInit(void)
{
	uint8_t i;
	uint32_t temp_32;
	uint32_t startAddr = Audio_GetCoefStartAddr(COEF_ADC_TABLE_OFFSET);
	QSPI_ReadFlashDataToXRAM(startAddr, COEF_ADC_TABLE_LENGTH, (uint8_t *)reg_map(mem_coef_table_temp));
	HWRITE(CORE_COEF_CTRL, 0x10);
	for(i = 0; i < 112; i ++)
	{
		temp_32 = HREAD24BIT((mem_coef_table_temp + (i * 3)));
		HWRITE24BIT(CORE_COEF_WDATA, temp_32);
		
		HWRITE(CORE_COEF_CTRL, 0x30);
		HWRITE(CORE_COEF_CTRL, 0x10);
	}
	for(i = 0; i < 224; i ++)
	{
		HWRITE24BIT(CORE_COEF_WDATA, 0);
		
		HWRITE(CORE_COEF_CTRL, 0x30);
		HWRITE(CORE_COEF_CTRL, 0x10);
	}
	HWRITE(CORE_COEF_CTRL, 0x00);
}
void Audio_SbcBusyWait(void)
{
	while(((HREAD(CORE_MISC_STATUS)) & BIT_4) != 0); 
}
void Audio_SbcClkOff(void)
{
	HWORW(CORE_CLKOFF, REG_CLOCK_OFF_SBC);
}
void Audio_SbcClkOn(void)
{
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_SBC);
}
void Audio_SbcStop(void)
{
	uint8_t temp;
	HWRITE(CORE_SBC_CTRL2, 0);
	HWRITEW(CORE_SBC_SWP, 0);
	HWRITE(CORE_SBC_CLR, 0xff);
	SYS_delay_us(1);
	HWRITE(CORE_SBC_CLR, 0);
	Audio_SbcBusyWait();

	temp = HREAD(CORE_SBC_CTRL);
	temp &= 0x80;
	HWRITE(CORE_SBC_CTRL, temp);
	
	temp = HREAD(CORE_TONE_CTRL);
	temp &= 0x8f;
	HWRITE(CORE_TONE_CTRL, temp);
	
	Audio_SbcClkOff();
}
void Audio_mSbcStop(void)
{
	Audio_SbcStop();
}
void Audio_DacAnaClkEnable(void)
{
	HWORW(CORE_DAC_SEL2, BIT_10);//DAC_ANA_CLK_EN
}
void Audio_DacAnaClkDisable(void)
{
	HWCORW(CORE_DAC_SEL2, BIT_10);//DAC_ANA_CLK_EN
}
void Audio_DacCloseL(void)
{
	HWCOR(CORE_DAC_SEL2, BIT_2);//core_dac_sel2_clk_en_l
}
void Audio_DacSetL(uint8_t din_sel)
{
	uint8_t temp;
	if(din_sel == DIN_SEL_L_CLOSE)
	{
		Audio_DacCloseL();
	}
	else
	{
		temp = HREAD(CORE_DAC_SEL2);
		temp |= BIT_2;//core_dac_sel2_clk_en_l
		temp &= 0xfc;
		temp |= din_sel;
		HWRITE(CORE_DAC_SEL2, temp);
	}
}
void Audio_DacCloseR(void)
{
	HWCOR(CORE_DAC_SEL2, BIT_6);//core_dac_sel2_clk_en_r
}
void Audio_DacSetR(uint8_t din_sel)
{
	uint8_t temp;
	if(din_sel == DIN_SEL_R_CLOSE)
	{
		Audio_DacCloseR();
	}
	else
	{
		temp = HREAD(CORE_DAC_SEL2);
		temp |= BIT_6;//core_dac_sel2_clk_en_r
		temp &= 0xcf;
		temp |= din_sel;
		HWRITE(CORE_DAC_SEL2, temp);
	}
}
void Audio_DacStero(void)
{
	HWCOR(CORE_DAC_SEL, BIT_0);
}
void Audio_DacMono(void)
{
	HWOR(CORE_DAC_SEL, BIT_0);
}
void Audio_DacMonoStero(uint8_t mono_flag)
{
	if(mono_flag == 0)
	{
		Audio_DacStero();
	}
	else
	{
		Audio_DacMono();
	}
}
void Audio_DacFilter(void)
{
	HWOR(CORE_DAC_CTRL, BIT_4);//DAC_INIT_FILTER
}
void Audio_DacFilterDisable(void)
{
	HWCOR(CORE_DAC_CTRL, BIT_4);//DAC_INIT_FILTER
}
void Audio_DacSdm(void)
{
	HWOR(CORE_DAC_CTRL, BIT_5);//DAC_SDM
}
void Audio_DacSdmDisable(void)
{
	HWCOR(CORE_DAC_CTRL, BIT_5);//DAC_SDM
}
void Audio_DlychainDacInit(void)
{
	uint8_t i;
	HWRITE24BIT(CORE_COEF_WDATA, 0);
	HWCOR(CORE_COEF_CTRL, BIT_0);
	HWOR(CORE_COEF_CTRL, BIT_0);
	for(i = 0; i < 32; i ++)
	{
		HWOR(CORE_COEF_CTRL, BIT_1);
		SYS_delay_us(5);//5US
		HWCOR(CORE_COEF_CTRL, BIT_1);
		SYS_delay_us(5);//5US
	}
	HWCOR(CORE_COEF_CTRL, BIT_0);
}
uint32_t Audio_GetCoefStartAddr(uint16_t offset)
{
	return HREAD24BIT(mem_coef_base_addr) + HREAD24BIT(mem_storage_start_addr) + offset;
}
void Audio_LoadDacCoefCommon(uint32_t startAddr)
{
	QSPI_ReadFlashDataToXRAM(startAddr, COEF_DAC_TABLE_LENGTH, (uint8_t *)reg_map(mem_coef_table_temp));
}
void Audio_LoadDac8kCoef(void)
{
	uint32_t startAddr = Audio_GetCoefStartAddr(COEF_DAC_TABLE_8K_OFFSET);
	Audio_LoadDacCoefCommon(startAddr);
}
void Audio_LoadDac16kCoef(void)
{
	Audio_LoadDac8kCoef();
}
void Audio_LoadDac48kCoef(void)
{
	uint32_t startAddr = Audio_GetCoefStartAddr(COEF_DAC_TABLE_48K_OFFSET);
	Audio_LoadDacCoefCommon(startAddr);
}
void Audio_LoadDac44kCoef(void)
{
	uint32_t startAddr = Audio_GetCoefStartAddr(COEF_DAC_TABLE_44K_OFFSET);
	Audio_LoadDacCoefCommon(startAddr);
}
void Audio_DacAnalogSel48k(void)
{
	HWOR(RF_RX_IB_LNA, BIT_0);
}
void Audio_DacAnalogSel44k(void)
{
	HWCOR(RF_RX_IB_LNA, BIT_0);
}
void Audio_DacSamplefreqSel8k(void)
{
	Audio_LoadDac8kCoef();
	Audio_DacAnalogSel48k();
}
void Audio_DacSamplefreqSel16k(void)
{
	Audio_LoadDac16kCoef();
	Audio_DacAnalogSel48k();
}
void Audio_DacSamplefreqSel48k(void)
{
	Audio_LoadDac48kCoef();
	Audio_DacAnalogSel48k();
}
void Audio_DacSamplefreqSel44k(void)
{
	Audio_LoadDac44kCoef();
	Audio_DacAnalogSel44k();
}
uint8_t Audio_DacClkCheck(uint8_t dac_clk)
{
	uint8_t temp;
	switch(dac_clk)
	{
		case 8:
			Audio_DacSamplefreqSel8k();
			temp = 8;
			break;
		case 16:
			Audio_DacSamplefreqSel16k();
			temp = 4;
			break;
		case 48:
			Audio_DacSamplefreqSel48k();
			temp = 0;
			break;
		default:
			Audio_DacSamplefreqSel44k();
			temp = 0;
			break;
	}

	return temp;
}

void Audio_DacCoefInit(void)
{
	uint32_t temp;
	uint8_t i;
	HWCOR(CORE_COEF_CTRL, BIT_2);
	HWOR(CORE_COEF_CTRL, BIT_2);
	for(i = 0; i < 64; i ++)
	{
		temp = HREAD24BIT(mem_coef_table_temp + (i * 3));
		temp = temp >> 4;
		HWRITE24BIT(CORE_COEF_WDATA, temp);
		
		HWOR(CORE_COEF_CTRL, BIT_3);
		SYS_delay_us(5);
		HWCOR(CORE_COEF_CTRL, BIT_3);
		SYS_delay_us(5);
	}
	HWCOR(CORE_COEF_CTRL, BIT_2);
}

void Audio_DacSetSampleRate(uint8_t dac_clk)
{
	uint8_t temp_0, temp_1;
	Audio_DacAnaClkDisable();
	Audio_DacFilterDisable();
	Audio_DacSdmDisable();

	// init coef
	Audio_DlychainDacInit();

	// clock select and coef init
	temp_0 = Audio_DacClkCheck(dac_clk);

	// set clk
	temp_1 = HREAD(CORE_DAC_SEL);
	temp_1 &= 0xf3;
	temp_1 |= temp_0;
	HWRITE(CORE_DAC_SEL, temp_1);

	//init coef
	SYS_delay_us(20);
	Audio_DacCoefInit();
	
	Audio_DacFilter();
	Audio_DacSdm();
	SYS_delay_us(100);
	
	Audio_DacAnaClkEnable();
}

void Audio_DacVolAdjust(void)
{
	uint16_t vol = 0;
	uint16_t volNow = HREAD(CORE_DAC_LVOL);
	if(!Bt_CheckHfpStart())
	{
		vol = HREAD(mem_music_vlm_speaker)*4;
	}
	else
	{
		vol = (HREAD(mem_hf_vlm_speaker) + 2)*4;
	}
	if(volNow != vol)
	{
		if(volNow < vol)
		{
			volNow++;
			HWRITE(CORE_DAC_LVOL, volNow);
			HWRITE(CORE_DAC_RVOL, volNow);
		}
		else
		{
			volNow--;
			HWRITE(CORE_DAC_LVOL, volNow);
			HWRITE(CORE_DAC_RVOL, volNow);
		}
	}
}
void Audio_OnlineDebugEqInit(void)
{
	uint32_t temp_32;
	uint16_t i;
	// Use spec eq info.
	if(Flash_CheckEqInfoValid()) 
	{		
		HWCORW(CORE_CLKOFF, BIT_6);//CLOCK_OFF_VOICE_FILTER
		
		HWRITE(CORE_COEF_CTRL, 0x10);
		for(i = 0; i < 51; i ++)
		{
			temp_32 = HREAD24BIT((int)(Flash_GetEqTable() + (i * 3)));
			HWRITE24BIT(CORE_COEF_WDATA, temp_32);
			
			HWRITE(CORE_COEF_CTRL, 0x30);
			HWRITE(CORE_COEF_CTRL, 0x10);
		}

		for(i = 0; i < 66; i ++)
		{
			HWRITE24BIT(CORE_COEF_WDATA, 0);
			
			HWRITE(CORE_COEF_CTRL, 0x30);
			HWRITE(CORE_COEF_CTRL, 0x10);
		}
		HWRITE(CORE_COEF_CTRL, 0x00);


		Audio_AdacSetAnaVol(gpRecinfo->sEqInfo.sEqGainValue);
	
		Audio_EqEnable();

		HWRITE(mem_eq_flag, 1);

		
		uint16_t vol = 0;
		if(!Bt_CheckHfpStart())
		{
			vol = HREAD(mem_music_vlm_speaker)*4;
		}
		else
		{
			vol = (HREAD(mem_hf_vlm_speaker) + 2)*4;
		}
		HWRITE(CORE_DAC_LVOL, vol);
		HWRITE(CORE_DAC_RVOL, vol);
	}
}
void Audio_DacInit(uint8_t mono_flag, uint8_t l_din_sel, uint8_t r_din_sel, uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	BOOL isNeedStartPa = FALSE;
	uint8_t tmp = HREAD(CORE_DAC_CTRL);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A623, tmp);

	//Check Dac open or not.
	if((tmp & (BIT_0)) != 0)//DAC_ENABLE
	{
		DEBUG_LOG_2(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A624, mono_flag, dac_clk);
		return;
	}
	DEBUG_LOG_2(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A600, mono_flag, dac_clk);
	HWRITE(mem_dac_clk,dac_clk);
	//TODO: Check need enable or not
	if(Audio_DacCheckDaAdacOpaEnable())
	{
		if(!Audio_AdacCheckLastDacClkSame(dac_clk))
		{
			Audio_AdacStopPAWithPop();
			// Need delay a while
			SYS_delay_ms(5);
			isNeedStartPa = TRUE;
		}
	}
	else
	{
		isNeedStartPa = TRUE;
	}
	if(isNeedStartPa)
	{
		Audio_AdacStartPAWithPop(dac_clk);
		// Need delay a while
		SYS_delay_ms(5);
	}
	//When open pa the ana vol is limit
	Audio_AdacPrepareReadyWork();
	
	// For online eq test, here may occur pop.
	Audio_OnlineDebugEqInit();

	//Avoid last finish flag
	Audio_DacClearRampDown();
	
	Audio_DacSetL(l_din_sel);
	Audio_DacSetR(r_din_sel);
	Audio_DacMonoStero(mono_flag);

	// Should not change immediate, it will make pop
	//Audio_DacVolAdjust();
	if(dac_mram_flag)
	{
		HWOR(CORE_DAC_CTRL, BIT_1);//DAC_MRAM_SEL
	}
	else
	{
		HWCOR(CORE_DAC_CTRL, BIT_1);//DAC_MRAM_SEL
	}
	
	HWCOR(CORE_DAC_CTRL, BIT_2);//DAC_CONTINUOUS_MODE
	HWCOR(CORE_DAC_CTRL, BIT_3);//DAC_MUTU

	HWRITEW(CORE_DAC_SADDR, dac_start_addr);
	HWRITEW(CORE_DAC_WPTR, dac_start_addr);
	HWRITEW(CORE_DAC_RPTR, dac_start_addr);
	
	HWRITEW(CORE_DAC_LEN, dac_buf_size - 1);

	// dac start
	HWOR(CORE_DAC_CTRL, BIT_0);//DAC_ENABLE

	Audio_RelasePaDelay();

	// For reduce power
	if(Bt_CheckA2DPStart())
	{
		HWRITE(0x8a01, 0xe2);
	}
	else
	{
		HWRITE(0x8a01, 0xf2);
	}

	// Check adc and eq enable or not
	if((HREAD(mem_eq_flag) == 0)
		&& !Bt_CheckHfpStart()
		&& !Audio_CheckAdcDmaEnable())
	{
#ifdef FUNCTION_OAL_MODE
		if(!Oal_CheckIsFunctionEnable())
#endif
		{
			HWORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
		}
	}
#ifdef FUNCTION_USE_SOFT_FARROW_FOR_ANDROID
	FilterMediaFarrowInit(dac_clk, (short *)(dac_mram_flag?reg_map_m0(dac_start_addr):reg_map(dac_start_addr))
		, dac_buf_size >> 1);
#endif
}

uint8_t Audio_CheckDacMRAMSelect(void)
{
	return ((HREAD(CORE_DAC_CTRL) & BIT_1) != 0);//DAC_MRAM_SEL
}
void Audio_DacInitMono(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	Audio_DacInit(1, DIN_SEL_L_LEFT_CHANNEL, DIN_SEL_R_LEFT_CHANNEL, dac_clk, dac_mram_flag
		, dac_start_addr, dac_buf_size);
}
void Audio_DacInitStero(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	Audio_DacInit(0, DIN_SEL_L_LEFT_CHANNEL, DIN_SEL_R_RIGHT_CHANNEL, dac_clk, dac_mram_flag
		, dac_start_addr, dac_buf_size);
}
void Audio_DacInitTwsLeftMonoStero(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	Audio_DacInit(1, DIN_SEL_L_LEFT_CHANNEL, DIN_SEL_R_CLOSE, dac_clk, dac_mram_flag
		, dac_start_addr, dac_buf_size);
}
void Audio_DacInitTwsRightMonoStero(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	Audio_DacInit(1, DIN_SEL_L_CLOSE, DIN_SEL_R_LEFT_CHANNEL, dac_clk, dac_mram_flag
		, dac_start_addr, dac_buf_size);
}
void Audio_DacInitMedia(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacInitTwsLeftMonoStero(dac_clk, dac_mram_flag
				, dac_start_addr, dac_buf_size);
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacInitTwsRightMonoStero(dac_clk, dac_mram_flag
				, dac_start_addr, dac_buf_size);
			break;
		default:
			Audio_DacInitStero(dac_clk, dac_mram_flag
				, dac_start_addr, dac_buf_size);
			break;
	}
}
void Audio_DacInitVpCall(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacInitTwsLeftMonoStero(dac_clk, dac_mram_flag
				, dac_start_addr, dac_buf_size);
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacInitTwsRightMonoStero(dac_clk, dac_mram_flag
				, dac_start_addr, dac_buf_size);
			break;
		default:
			Audio_DacInitMono(dac_clk, dac_mram_flag
				, dac_start_addr, dac_buf_size);
			break;
	}
}
BOOL Audio_CheckDacEmpty(void)
{
	return (HREADW(CORE_DAC_WPTR) == HREADW(CORE_DAC_RPTR));
}

void Audio_TishiAddressInit(uint16_t saddr, uint16_t len)
{
	HWRITEW(CORE_TISHI_SADDR, saddr);
	HWRITEW(CORE_TISHI_LEN, len - 1);
	HWRITEW(CORE_TISHI_WPTR, saddr);
	HWRITEW(CORE_TISHI_RPTR, saddr);
}

void Audio_TishiEnable(void)
{
	// use mram
	// M = 2, N = 2
	// play_sound = main_sound/M + tishi_sound/N
	// tishi_l_en, tishi_r_en
	HWRITEW(CORE_TISHI_CTRL, 0xe7);
}

void Audio_TishiDisable(void)
{
	HWRITEW(CORE_TISHI_SADDR, 0);
	HWRITEW(CORE_TISHI_LEN, 0);
	HWRITEW(CORE_TISHI_CTRL, 0);
}

uint8_t Audio_CheckTishiMRAMSelect(void)
{
	return ((HREAD(CORE_TISHI_CTRL) & BIT_5) != 0);//tishi_mram_sel, 1: mram, 0: not mram
}










void Audio_AdacOpaCmnSel(uint8_t vol)//RG_ADAC_OPACMN_SEL        
{
	uint8_t tmp = HREADW(CORE_AADC_2);
	tmp &= 0xF0;
	tmp |=vol;
	HWRITEW(CORE_AADC_2, tmp);
}
void Audio_AdacHPPAOutputStageQuiescentCurrentControl(DevAudioHPPAOutputStageQuiescentCurrent cur)
{
	uint8_t tmp = HREADW(CORE_AADC_2);
	tmp &= 0x8f;
	tmp |=(cur<<4);
	HWRITEW(CORE_AADC_2, tmp);
}
void Audio_HPPACascodeVoltageControl(DevAudioHPPACascodeVoltage cas)
{
	uint8_t tmp = HREADW(CORE_AADC_2);
	tmp &= 0x7f;
	tmp |=((cas&0x01)<<7);
	HWRITEW(CORE_AADC_2, tmp);
	tmp=HREADW(CORE_AADC_3);
	tmp &= 0xfc;
	tmp |=((cas&0x06)>>1);
	HWRITEW(CORE_AADC_3, tmp);
}

void Audio_AdacRefCtrl(uint8_t val)
{
	uint8_t tmp = HREAD(CORE_AADC_7);
	tmp &= 0x0F;
	HWRITE(CORE_AADC_7, tmp | (val << 4));
}

//"Audio DAC feedback cap control0: 8.8pF; 1: 2x8.8pF"
/*void Audio_AdacFeedbackCapControl(uint8_t val)
{
	uint8_t tmp = HREAD(CORE_AADC_3);
	tmp |= ((val & 0x01) << 5);
	HWRITE(CORE_AADC_7, tmp | (val << 4));
}*/

void Audio_AdacBiasSel(DevAudioDacBiasControl bias)//RG_ADAC_BIAS_SEL          
{
	uint8_t tmp = HREAD(CORE_AADC_0);
	tmp &= 0xF0;
	tmp |= bias ;
	HWRITE(CORE_AADC_0, tmp );
}
void Audio_AdacOpacmpSel(uint8_t opacmp)//RG_ADAC_OPACMP_SEL        
{
	uint8_t tmp = HREAD(CORE_AADC_0);
	tmp &= 0x0F;
	tmp |= (opacmp << 4);
	HWRITE(CORE_AADC_0, tmp );
}

void  Audio_AdacOpaBiasSel(DevAudioHPPAbiasCurrentControl hpcur)//RG_ADAC_OPABIAS_SEL       
{
	uint8_t tmp = HREAD(CORE_AADC_1);
	tmp &= 0xCF;
	tmp |= (hpcur<<4);
	HWRITE(CORE_AADC_1, tmp );
}
void  Audio_AdacOpamcmpSel(DevAudioHPPACompensationCapControl hpcap)//RG_ADAC_OPAMCAP_SEL       
{
	uint8_t tmp = HREAD(CORE_AADC_1);
	tmp &= 0x3F;
	tmp |= (hpcap<<6);
	HWRITE(CORE_AADC_1, tmp );
}

void Audio_AdacVCMIOPA_SEL(DevDACComModeVolBufOutputStageCur cur)
{
	uint8_t tmp = HREAD(CORE_AADC_3);
	tmp &=0xf3;
	tmp |=(cur<<2);
	HWRITE(CORE_AADC_3, tmp );
}
void Audio_AdacComVoltageBufferModeSet(DevAudioVoltageBufferBypassMode mode)
{
	uint8_t tmp = HREAD(CORE_AADC_3);
	tmp &=0xef;
	tmp |=(mode<<4);
	HWRITE(CORE_AADC_3, tmp);
}
void Audio_AdacFeedbackCapControl(DevAudioDACFeedbackCapControl feed)
{
	uint8_t tmp = HREAD(CORE_AADC_3);
	tmp &=0xdf;
	tmp |=(feed<<5);
	HWRITE(CORE_AADC_3, tmp );
}
void Audio_DacLDOOutputControl(uint8_t value)
{
	uint8_t tmp = HREAD(CORE_AADC_5);
	tmp &=0xd8;
	tmp |=(value&0x07);
	HWRITE(CORE_AADC_5, tmp );
}
void Audio_AdacDacCommonModeVoltageControl(DevAudioDACCommonModeVoltageControl val)
{
	uint8_t tmp = HREAD(CORE_AADC_5);
	tmp &=0xc7;
	tmp |=((val&0x03)<<3);
	HWRITE(CORE_AADC_5, tmp );
}
void Audio_AdacLdoAvddByPassEnable(void)
{
	uint8_t tmp = HREAD(CORE_AADC_5);
	tmp |=(1<<6);
	HWRITE(CORE_AADC_5, tmp );
}
void  Audio_AdacLdoHpvddByPassEnable(void) 
{
	uint8_t tmp = HREAD(CORE_AADC_6);
	tmp |=(1<<5);
	HWRITE(CORE_AADC_6, tmp );
}
int16_t gDacDcCaliTargetLeft;
int16_t gDacDcCaliTargetRight;

uint8_t gDacAnaVolTargetLeft;
uint8_t gDacAnaVolTargetRight;
void Audio_AdacPorDefaultInit(void)
{
	//CORE_AADC_0
	Audio_AdacBiasSel(DAC_BIAS_3_0uA);
	Audio_AdacOpacmpSel(OPACMP_VDD200mV_RG_ADAC_OPABIAS_SEL1);
	//CORE_AADC_1
	Audio_AdacOpaBiasSel(HPPA_BIAS_CURRENT_8uA);
	Audio_AdacOpamcmpSel(HPPA_COMPENSATION_CAP_1040fF);
	//CORE_AADC_2
	Audio_AdacOpaCmnSel(HPPA_NMOS_CASCODE_VOLTAGE_200mV_OPABIAS1);
	Audio_AdacHPPAOutputStageQuiescentCurrentControl(RG_ADAC_OPA3STA_SEL2);
	Audio_HPPACascodeVoltageControl(RG_ADAC_OPABIAS_SEL1);

	// This will change max output vol
	/*Audio DAC output full-scale control
		1100: 1.2V (VDD>=1.7V)
		1101: 1.6V (VDD>=2.1V) 
		1110: 2.0V (VDD>=2.5V)
		1111: 2.4V (VDD>=2.9V)	*/
	//CORE_AADC_7
	Audio_AdacRefCtrl(AUDIO_DAC_OUT_FULL_SCAL_CONTROL_1_6V);

	// When open, shoud use low ana vol
	//TODO: Current bottom nosie is 10-11uV, no need do this work
	//CORE_AADC_3
	Audio_AdacVCMIOPA_SEL(OUTPUT_STAGE_CUR_80uA);
	Audio_AdacSetAnaVolLeft(ADAC_SETTING_ANA_VOL_TARGET);
	Audio_AdacComVoltageBufferModeSet(USE_GOLBAL_COMMON_MODE_VOL_BUFFER);
	Audio_AdacFeedbackCapControl(Feedback_Cap_2x8_8pF);
	//CORE_AADC_4
	Audio_AdacSetAnaVolRight(ADAC_SETTING_ANA_VOL_TARGET);
	//CORE_AADC_5
	Audio_DacLDOOutputControl(ADAC_LDO_HPVDD_VCTRL_VALUE);
	Audio_AdacDacCommonModeVoltageControl(DAC_COM_MODE_VOL0_9VDD1_8);
	Audio_AdacLdoAvddByPassEnable();
	//CORE_AADC_6
	Audio_AdacLdoHpvddByPassEnable();
	
	HWRITE(CORE_AADC_0, 0x12);
	HWRITE(CORE_AADC_1, 0xf2);
	HWRITE(CORE_AADC_2, 0x21);
	HWRITE(CORE_AADC_3, 0xf3);
	HWRITE(CORE_AADC_4, 0x18);
	HWRITE(CORE_AADC_5, 0x50);
	HWRITE(CORE_AADC_6, 0x20);
	HWRITE(CORE_AADC_7, 0xd0);

        if(HREAD(mem_test_mode) == 1)	
	    	Audio_AdacSetAnaVol(0);
        else
	    	Audio_AdacSetAnaVol(gEqGain);

	//DC cali setting
	if(( (HREAD(CORE_AADC_4)>>3)&0x1f)>=3&&( (HREAD(CORE_AADC_4)>>3)&0x1f)<6)//0db
	{
		gDacDcCaliTargetLeft = HREADW(mem_efuse_dc_offset_l);//ff90 -112 0dBm
		if(abs(gDacDcCaliTargetLeft)>0xf0)
		{
			gDacDcCaliTargetLeft=0;
		}
		gDacDcCaliTargetRight = HREADW(mem_efuse_dc_offset_r);
		if(abs(gDacDcCaliTargetRight)>0xf0)
		{
			gDacDcCaliTargetRight=0;
		}
	}
	else if(( (HREAD(CORE_AADC_4)>>3)&0x1f)>=6)//-6db
	{
		gDacDcCaliTargetLeft = HREADW(mem_efuse_dc_offset_l_f6);
		if(abs(gDacDcCaliTargetLeft)>0xf0)
		{
			gDacDcCaliTargetLeft=0;
		}
		gDacDcCaliTargetRight = HREADW(mem_efuse_dc_offset_r_f6);
		if(abs(gDacDcCaliTargetRight)>0xf0)
		{
			gDacDcCaliTargetRight=0;
		}
	}
	else if(( (HREAD(CORE_AADC_4)>>3)&0x1f)==0)//6db
	{
		gDacDcCaliTargetLeft = HREADW(mem_efuse_dc_offset_l);//ff90 -112 0dBm
		gDacDcCaliTargetLeft*=2;
		if(abs(gDacDcCaliTargetLeft)>0x1e0)
		{
			gDacDcCaliTargetLeft=0;
		}
		gDacDcCaliTargetRight = HREADW(mem_efuse_dc_offset_r);
		gDacDcCaliTargetRight*=2;
		if(abs(gDacDcCaliTargetRight)>0x1e0)
		{
			gDacDcCaliTargetRight=0;
		}
	}
	HWRITEW(CORE_DAC_DC_CALI_L, gDacDcCaliTargetLeft);
	HWRITEW(CORE_DAC_DC_CALI_R, gDacDcCaliTargetRight);
}

void Audio_DacSetRampDownEnL(void)
{
	HWOR(CORE_RAMP_CTRL, 1<< left_down_en);//left_down_en
}
void Audio_DacSetRampDownEnR(void)
{
	HWOR(CORE_RAMP_CTRL, 1<< right_down_en);//right_down_en
}
void Audio_DacSetRampDownEn(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetRampDownEnL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetRampDownEnR();
			break;
		default:
			Audio_DacSetRampDownEnL();
			Audio_DacSetRampDownEnR();
			break;
	}
}

void Audio_DacSetDaAdacIntL(void)
{
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_INT_L);
}
void Audio_DacSetDaAdacIntR(void)
{
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_INT_R);
}
void Audio_DacSetDaAdacInt(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacIntL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacIntR();
			break;
		default:
			Audio_DacSetDaAdacIntL();
			Audio_DacSetDaAdacIntR();
			break;
	}
}
void Audio_DacDisableDaAdacInt(void)
{
	uint8_t tmp = HREAD(CORE_AADC_9);
	tmp &= ~(1<<DA_EN_ADAC_INT_L);
	tmp &= ~(1<<DA_EN_ADAC_INT_R);
	HWRITE(CORE_AADC_9, tmp);
}

void Audio_DacSetDaAdacOpaL(void)
{
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_OPAL);
}
void Audio_DacSetDaAdacOpaR(void)
{
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_OPAR);
}
void Audio_DacSetDaAdacOpa(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacOpaL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacOpaR();
			break;
		default:
			Audio_DacSetDaAdacOpaL();
			Audio_DacSetDaAdacOpaR();
			break;
	}
}
uint8_t Audio_DacCheckDaAdacOpaEnable(void)
{
	uint8_t tmp = HREAD(CORE_AADC_9);
	
	return (tmp & (1<<DA_EN_ADAC_OPAL)) | (tmp & (1<<DA_EN_ADAC_OPAR));
}
uint8_t Audio_DacCheckDaAdacOpaEnableLeft(void)
{
	uint8_t tmp = HREAD(CORE_AADC_9);
	
	return (tmp & (1<<DA_EN_ADAC_OPAL));
}
uint8_t Audio_DacCheckDaAdacOpaEnableRight(void)
{
	uint8_t tmp = HREAD(CORE_AADC_9);
	
	return (tmp & (1<<DA_EN_ADAC_OPAR));
}
void Audio_DacDisableDaAdacOpa(void)
{
	uint8_t tmp = HREAD(CORE_AADC_9);
	tmp &= ~(1<<DA_EN_ADAC_OPAL);
	tmp &= ~(1<<DA_EN_ADAC_OPAR);
	HWRITE(CORE_AADC_9, tmp);
}

void Audio_DacSetDaAdacPathL(void)
{
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_PATHL);
}
void Audio_DacSetDaAdacPathR(void)
{
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_PATHR);
}
void Audio_DacSetDaAdacPath(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacPathL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacPathR();
			break;
		default:
			Audio_DacSetDaAdacPathL();
			Audio_DacSetDaAdacPathR();
			break;
	}
}
void Audio_DacDisableDaAdacPath(void)
{
	uint8_t tmp = HREAD(CORE_AADC_9);
	tmp &= ~(1<<DA_EN_ADAC_PATHL);
	tmp &= ~(1<<DA_EN_ADAC_PATHR);
	HWRITE(CORE_AADC_9, tmp);
}

void Audio_DacSetDaAdacDepopSwL(void)
{
	HWOR(CORE_AADC_A, 1<<DA_EN_ADAC_DEPOP_SW_L);
}
void Audio_DacSetDaAdacDepopSwR(void)
{
	HWOR(CORE_AADC_A, 1<<DA_EN_ADAC_DEPOP_SW_R);
}
void Audio_DacSetDaAdacDepopSw(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacDepopSwL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacDepopSwR();
			break;
		default:
			Audio_DacSetDaAdacDepopSwL();
			Audio_DacSetDaAdacDepopSwR();
			break;
	}
}
void Audio_DacDisableDaAdacDepopSw(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= ~(1<<DA_EN_ADAC_DEPOP_SW_L);
	tmp &= ~(1<<DA_EN_ADAC_DEPOP_SW_R);
	HWRITE(CORE_AADC_A, tmp);
}


void Audio_DacSetDaAdacOpaDummyToMainL(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= ~(1<<DA_EN_ADAC_OPA_DUMMY_L);
	tmp |= (1<<DA_EN_ADAC_OPA_LOOP_L);
	HWRITE(CORE_AADC_A, tmp);
}
void Audio_DacSetDaAdacOpaDummyToMainR(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= ~(1<<DA_EN_ADAC_OPA_DUMMY_R);
	tmp |= (1<<DA_EN_ADAC_OPA_LOOP_R);
	HWRITE(CORE_AADC_A, tmp);
}
void Audio_DacSetDaAdacOpaDummyToMain(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacOpaDummyToMainL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacOpaDummyToMainR();
			break;
		default:
			Audio_DacSetDaAdacOpaDummyToMainL();
			Audio_DacSetDaAdacOpaDummyToMainR();
			break;
	}
}

void Audio_DacSetDaAdacOpaMainToDummyL(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= ~(1<<DA_EN_ADAC_OPA_LOOP_L);
	tmp |= (1<<DA_EN_ADAC_OPA_DUMMY_L);
	HWRITE(CORE_AADC_A, tmp);
}
void Audio_DacSetDaAdacOpaMainToDummyR(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= ~(1<<DA_EN_ADAC_OPA_LOOP_R);
	tmp |= (1<<DA_EN_ADAC_OPA_DUMMY_R);
	HWRITE(CORE_AADC_A, tmp);
}
void Audio_DacSetDaAdacOpaMainToDummy(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacOpaMainToDummyL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacOpaMainToDummyR();
			break;
		default:
			Audio_DacSetDaAdacOpaMainToDummyL();
			Audio_DacSetDaAdacOpaMainToDummyR();
			break;
	}
}
void Audio_DacSetDaAdacRstIntL(void)
{
	HWOR(CORE_AADC_A, 1<<DA_ADAC_RST_INT_L);//DA_ADAC_RST_INT_L
}
void Audio_DacSetDaAdacRstIntR(void)
{
	HWOR(CORE_AADC_A, 1<<DA_ADAC_RST_INT_R);//DA_ADAC_RST_INT_R
}
void Audio_DacSetDaAdacRstInt(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaAdacRstIntL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaAdacRstIntR();
			break;
		default:
			Audio_DacSetDaAdacRstIntL();
			Audio_DacSetDaAdacRstIntR();
			break;
	}
}
void Audio_DacSetDaAdacRstIntDisable(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= (1<<DA_ADAC_RST_INT_L);
	tmp &= (1<<DA_ADAC_RST_INT_R);
	HWRITE(CORE_AADC_A, tmp);
}
void Audio_DacSetDaEnAdacOpaDummyL(void)
{
	HWOR(CORE_AADC_A, 1<<DA_EN_ADAC_OPA_DUMMY_L);//DA_EN_ADAC_OPA_DUMMY_L
}
void Audio_DacSetDaEnAdacOpaDummyR(void)
{
	HWOR(CORE_AADC_A, 1<<DA_EN_ADAC_OPA_DUMMY_R);//DA_EN_ADAC_OPA_DUMMY_L
}
void Audio_DacSetDaEnAdacOpaDummy(void)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_DacSetDaEnAdacOpaDummyL();
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_DacSetDaEnAdacOpaDummyR();
			break;
		default:
			Audio_DacSetDaEnAdacOpaDummyL();
			Audio_DacSetDaEnAdacOpaDummyR();
			break;
	}
}
void Audio_DacDisableDaAdacOpaDummy(void)
{
	uint8_t tmp = HREAD(CORE_AADC_A);
	tmp &= (1<<DA_EN_ADAC_OPA_DUMMY_L);
	tmp &= (1<<DA_EN_ADAC_OPA_DUMMY_R);
	HWRITE(CORE_AADC_A, tmp);
}
void Audio_LpmWrite2AdcLow(uint32_t val)
{
	HWRITEL(mem_lpm_write_temp_adc_low, val);
	HWRITEL(CORE_LPM_REG, val);
	HWRITESINGLE(CORE_LPM_WR2, LPMREG_SEL_ADC_LOW);
	SYS_delay_ms(1);
}
void Audio_LpmWrite2AdcHigh(uint32_t val)
{
	HWRITEL(mem_lpm_write_temp_adc_high, val);
	HWRITEL(CORE_LPM_REG, val);
	HWRITESINGLE(CORE_LPM_WR2, LPMREG_SEL_ADC_HIGH);
	SYS_delay_ms(1);
}
void Audio_DaAaddaBgEn(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_adc_low);
	// If enable, rtn
	if(tmp & (1 << da_aadda_bg_en)) // da_aadda_bg_en
	{
		return;
	}
	tmp |= (1 << da_aadda_bg_en); // da_aadda_bg_en
	Audio_LpmWrite2AdcLow(tmp);

	//DA_AADDA_BG_EN_FC = 1 FOR >=5US
	//DA_AADDA_BG_EN_FC = 0
	tmp |= BIT_31;//da_aadda_bg_en_fc
	Audio_LpmWrite2AdcLow(tmp);
	 // for >=5uS
	SYS_delay_ms(1);
	tmp &= ~(BIT_31);//da_aadda_bg_en_fc
	Audio_LpmWrite2AdcLow(tmp);
}

void Audio_DaAaddaBgDisable(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_adc_low);
	tmp &= ~(1 << da_aadda_bg_en); // da_aadda_bg_en
	Audio_LpmWrite2AdcLow(tmp);
}
void Audio_DacSetVolBeforeCalibration(void)
{
	Audio_AdacAdjustDigitalVolToCali();
}

void Audio_DacInitRampDown(void)
{
	//Set left_down_dest(8048)/right_down_dest(804a) to 0x0000 (ramp down end value)
	HWRITEW(CORE_LEFT_DOWN_DEST, 0);
	HWRITEW(CORE_RIGHT_DOWN_DEST, 0);
	
	//Set left_down_step(806f)/right_down_step(8070) to 0xf // larger value faster ramp down speed.
	// 0xf too fast, will make pop.
	HWRITE(CORE_LEFT_STEP, 0x01);
	HWRITE(CORE_RIGHT_STEP, 0x01);

	Audio_DacSetRampDownEn();
}

uint8_t gLastStartDacClk;
// For mute POP, before PA enable, Must:
// 1. Enable clock
// 2. Fix zero out
// 3. Enable filter clk
// 4. Enable sdm clk
// 5. Enable ana clk
void Audio_DacInitForPop(uint8_t dac_clk)
{
	//1. Turn on DAC digital clock
	//RG_MISC (896E) = 0x94 for 48K/16K/8K or 0xC4 for 44.1K
	if(dac_clk != 44)
	{
		HWRITE(RG_MISC, rg_misc_8_16_48K);
		HWRITE(RF_AFC_CAP, rg_clkpll_8_16_48K);
	}
	else
	{
		HWRITE(RG_MISC, rg_misc_44d1K);
		HWRITE(RF_AFC_CAP, rg_clkpll_44d1K);
	}

	gLastStartDacClk = dac_clk;

	// enable clk
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_AUDIO_DAC);
	Audio_DacAnaClkDisable();

	// Left right fix zero
	HWRITE(CORE_DAC_SEL2, 0x77);

	
	Audio_DacSetSampleRate(dac_clk);
}
void Audio_AdacStartPAWithPop(uint8_t dac_clk)
{
	uint8_t tmp;
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A601, dac_clk);

	//2. DAC init
	// Here need enable ana clock
	Audio_DacInitForPop(dac_clk);

	//3. Set digital volume to the value used when calibration
	// Must set the vol while it set in dc cali
	Audio_DacSetVolBeforeCalibration();
	
	//4. Enable digital ramp down for fading out music needed for pause function
	Audio_DacInitRampDown();

	//5. Turn on audio bandgap (required by audio ADC & DAC)
	Audio_DaAaddaBgEn();

	Audio_DacSetDaAdacRstInt();
	Audio_DacSetDaEnAdacOpaDummy();

	//6. Turn on DAC LDOs
	//DA_EN_ADAC_BIAS = 1
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_BIAS);
	
	//DA_ADAC_LDO_HPVDD_EN = 1
	//DA_ADAC_LDO_AVDD_EN = 1
	tmp = HREAD(CORE_AADC_8);
	tmp |= (1 << DA_ADAC_LDO_AVDD_EN);
	tmp |= (1 << DA_ADAC_LDO_HPVDD_EN);
	HWRITE(CORE_AADC_8, tmp);

	//
	Audio_DacSetDaAdacRstIntDisable();
	SYS_delay_us(10);

	
	//7. Turn on DAC reference generator
	//DA_EN_ADAC_REF = 1
	HWOR(CORE_AADC_9, 1<<DA_EN_ADAC_REF);
	
	//8. Turn on DAC core
	Audio_DacSetDaAdacInt();
	SYS_delay_us(10);

	//9. Turn on HP PA
	Audio_DacSetDaAdacOpa();
	SYS_delay_us(10);

	Audio_DacSetDaAdacPath();
	SYS_delay_ms(5);

	Audio_DacSetDaAdacDepopSw();
	SYS_delay_ms(5);

	Audio_DacSetDaAdacOpaDummyToMain();
}

void Audio_DacWaitRampDown(void)
{
	uint16_t waitLimit = 0;
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60B, HREAD(CORE_DAC_STATUS));
	// Wait ramp down finish
	//Tramp= (last_data_value/(left/right_down_step +1)*(1/48)) mS
	//,  if sample rate is 48K. The maximum time is 40ms if down_step is '0xF' and sample rate is 48K.
	//Use 8K sample rate the maximum time is 6*40=240ms.
	if(HREAD(mem_music_vlm_speaker) <= (HFP_SPEAKER_MAX_VLM_MEDIA / 2))
	{
		//For Iphone
		HWRITE(CORE_LEFT_STEP, 0x02);
		HWRITE(CORE_RIGHT_STEP, 0x02);	
	}
	while((HREAD(CORE_DAC_STATUS) & BIT_4) == 0)
	{
		// Idle no need wait.
		if((HREAD(CORE_DAC_STATUS)) == 0)
		{
			DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60F, HREAD(CORE_DAC_STATUS));
			break;
		}
		SYS_delay_us(1);
		if(waitLimit ++ > 2000)
		{
#ifdef FUNCTION_WATCH_DOG
			WDT_Kick();
#endif
			uint8_t curRampStep = HREAD(CORE_LEFT_STEP);
			if(curRampStep < 0x0f)
			{
				curRampStep += 1;
				
				HWRITE(CORE_LEFT_STEP, curRampStep);
				HWRITE(CORE_RIGHT_STEP, curRampStep);	
			}
			else
			{
				if((HREAD(CORE_DAC_STATUS) & 0x0f) == 0)
				{
					DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60D, HREAD(CORE_DAC_STATUS));
					break;
				}
			}

			waitLimit  = 0;
		}
	}
	// Restore the step value
	HWRITE(CORE_LEFT_STEP, 0x01);
	HWRITE(CORE_RIGHT_STEP, 0x01);	
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60F, HREAD(CORE_DAC_STATUS));

	Audio_DacClearRampDown();
}

void Audio_DacClearRampDown(void)
{
	 // write 8138, bit[4], ¡®10¡¯ to clear down_finish_flag
	HWOR(CORE_RAMP_CTRL, BIT_4);
	SYS_delay_us(1);
	HWCOR(CORE_RAMP_CTRL, BIT_4);
}
void Audio_AdacStopPAWithPop(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60C, HREAD(CORE_AADC_9));

	if(!Audio_DacCheckDaAdacOpaEnable())
	{
		return;
	}
	//3. Set digital volume to the value used when calibration
	// Must set the vol while it set in dc cali
	Audio_AdacPreparePAClose();
	// Need delay a while
	SYS_delay_ms(5);

	Audio_DacSetDaAdacOpaMainToDummy();
	SYS_delay_ms(5);

	Audio_DacDisableDaAdacDepopSw();
	Audio_DacDisableDaAdacPath();

	
	//4. Turn off HP PA
	Audio_DacDisableDaAdacOpa();
	//5. Turn off other DAC-related blocks
	//DA_EN_ADAC_INT_L (R) = 0
	Audio_DacDisableDaAdacInt();
	//DA_EN_ADAC_REF = 0
	HWCOR(CORE_AADC_9, (1<<DA_EN_ADAC_REF));
	//DA_ADAC_LDO_HPVDD_EN = 0
	HWCOR(CORE_AADC_8, (1<<DA_ADAC_LDO_HPVDD_EN));
	//DA_ADAC_LDO_AVDD_EN = 0
	HWCOR(CORE_AADC_8, (1<<DA_ADAC_LDO_AVDD_EN));
	//DA_EN_ADAC_BIAS = 0
	HWCOR(CORE_AADC_9, (1<<DA_EN_ADAC_BIAS));


	Audio_DacSetDaAdacRstInt();
	Audio_DacDisableDaAdacOpaDummy();
	
	//6. Turn off audio bandgap if needed (required by audio ADC & DAC)
	//da_aadda_bg_en = 0
#ifdef FUNCTION_OAL_MODE
	if(!Oal_CheckIsFunctionEnable())
#endif
	{
		Audio_DaAaddaBgDisable();
	}

	HWRITEW(CORE_DAC_SEL2, 0);
	HWRITEW(CORE_DAC_CTRL, 0);
	
	// disable clk
	HWORW(CORE_CLKOFF, REG_CLOCK_OFF_AUDIO_DAC);
#ifdef FUNCTION_OAL_MODE
	if(!Oal_CheckIsFunctionEnable())
#endif
	{
		if(!Bt_CheckHfpStart())
		{
			HWORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
		}
	}
}


void Audio_AdacSetAnaVol(uint8_t value)
{
	switch(HREAD(mem_audio_output_setting))
	{
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_LEFT:
			Audio_AdacSetAnaVolLeft(value);
			break;
		case AUDIO_OUTPUT_TWS_SINGLE_EAR_RIGHT:
			Audio_AdacSetAnaVolRight(value);
			break;
		default:
			Audio_AdacSetAnaVolLeft(value);
			Audio_AdacSetAnaVolRight(value);
			break;
	}
}

//RG_ADAC_VOLL[1:0]	2'b11
//RG_ADAC_VOLL[4:2]	3'b000
//"Audio HP PA left-channel analog volume control6dB-2dB*RG_ADAC_VOLL"
void Audio_AdacSetAnaVolLeft(uint8_t value)
{
	uint8_t tmp = HREAD(CORE_AADC_3);
	tmp &=0x3f;
	tmp |=((value&0x03)<<6);
	HWRITE(CORE_AADC_3, tmp );
	tmp=HREAD(CORE_AADC_4);
	tmp &=0xfc;
	tmp |=((value&0x0c)>>2);
	HWRITE(CORE_AADC_4, tmp );
}/*
void Audio_AdacSetAnaVolLeft(uint8_t val)
{
	uint16_t tmp = HREADW(CORE_AADC_3);
	tmp &= 0xF83F;
	tmp |= ((val & 0x03) << 6);
	tmp |= (((val & 0x1c)  >> 2) << 8);
	HWRITEW(CORE_AADC_3, tmp);
}*/
uint8_t Audio_AdacGetAnaVolLeft(void)
{
	uint16_t tmp = HREADW(CORE_AADC_3);
	tmp = tmp >> 6;
	return (tmp & 0x1f);
}
//RG_ADAC_VOLR              	5'b00011
//"Audio HP PA right-channel analog volume control6dB-2dB*RG_ADAC_VOLL"
void Audio_AdacSetAnaVolRight(uint8_t val)
{
	uint16_t tmp = HREAD(CORE_AADC_4);
	tmp &= 0x07;
	tmp |= ((val & 0x1f) << 3);
	HWRITE(CORE_AADC_4, tmp);
}
uint8_t Audio_AdacGetAnaVolRight(void)
{
	uint16_t tmp = HREAD(CORE_AADC_4);
	tmp = tmp >> 3;
	return (tmp & 0x1f);
}


uint8_t Audio_AdacCheckAnaVolAlreadyNormal(void)
{
	uint8_t tmp = 0;
	tmp = Audio_AdacGetAnaVolLeft();
	return (tmp == ADAC_SETTING_ANA_VOL_TARGET);
}
void Audio_AdacAdjustAnaVolToNormal(void)
{
	uint8_t tmp = 0;
	tmp = Audio_AdacGetAnaVolLeft();
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A609, tmp);
	while(tmp > ADAC_SETTING_ANA_VOL_TARGET)
	{
		tmp--;
		Audio_AdacSetAnaVolLeft(tmp);
		SYS_delay_ms(1);
	}
}
uint8_t Audio_AdacCheckAnaVolAlreadyLimit(void)
{
	uint8_t tmp = 0;
	tmp = Audio_AdacGetAnaVolLeft();
	return (tmp == ADAC_SETTING_ANA_VOL_LIMIT);
}
void Audio_AdacAdjustAnaVolToLimit(void)
{
	uint8_t tmp = 0;
	tmp = Audio_AdacGetAnaVolLeft();
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A608, tmp);
	while(tmp < ADAC_SETTING_ANA_VOL_LIMIT)
	{
		tmp++;
		Audio_AdacSetAnaVolLeft(tmp);
		SYS_delay_ms(1);
	}
}

uint16_t Audio_AdacToolGetAdjustInterval(int16_t rang)
{
	uint16_t tmpNoSignVal = rang;
	uint16_t tmpInterval = 0;
	if(rang < 0)
	{
		tmpNoSignVal = -rang;
	}
	tmpInterval = tmpNoSignVal / 30;
	if(tmpInterval == 0)
	{
		tmpInterval = tmpNoSignVal / 20;
		if(tmpInterval == 0)
		{
			tmpInterval = tmpNoSignVal / 10;
		}
	}

	if(tmpInterval == 0)
	{
		tmpInterval = 1;
	}

	return tmpInterval;
}
int16_t Audio_AdacToolGetSrcToDstVal(int16_t src, int16_t dst, uint16_t interval)
{
	if(src != dst)
	{
		if(src< dst)
		{
			src += interval;
			if(src > dst)
			{
				src = dst;
			}
		}
		else
		{
			src -= interval;
			if(src < dst)
			{
				src = dst;
			}
		}
	}

	return src;
}
void Audio_AdacAdjustDCCaliToNormal(void)
{
	int16_t tmpLeft = HREADW(CORE_DAC_DC_CALI_L);
	int16_t tmpTargetLeft = gDacDcCaliTargetLeft;
	uint16_t tmpIntervalLeft = Audio_AdacToolGetAdjustInterval(tmpTargetLeft);
	
	int16_t tmpRight = HREADW(CORE_DAC_DC_CALI_R);
	int16_t tmpTargetRight = gDacDcCaliTargetRight;
	uint16_t tmpIntervalRight = Audio_AdacToolGetAdjustInterval(tmpTargetRight);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A607, tmpLeft);
	while((tmpLeft != tmpTargetLeft)
		||(tmpRight != tmpTargetRight))
	{
		if(tmpLeft != tmpTargetLeft)
		{
			tmpLeft = Audio_AdacToolGetSrcToDstVal(tmpLeft, tmpTargetLeft, tmpIntervalLeft);
			HWRITEW(CORE_DAC_DC_CALI_L, tmpLeft);
		}
		if(tmpRight != tmpTargetRight)
		{
			tmpRight = Audio_AdacToolGetSrcToDstVal(tmpRight, tmpTargetRight, tmpIntervalRight);
			HWRITEW(CORE_DAC_DC_CALI_R, tmpRight);
		}
		SYS_delay_ms(1);
	}
}

void Audio_AdacAdjustDCCaliToZero(void)
{
	int16_t tmpLeft = HREADW(CORE_DAC_DC_CALI_L);
	int16_t tmpTargetLeft = 0;
	uint16_t tmpIntervalLeft = Audio_AdacToolGetAdjustInterval(tmpLeft);
	
	int16_t tmpRight = HREADW(CORE_DAC_DC_CALI_R);
	int16_t tmpTargetRight = 0;
	uint16_t tmpIntervalRight = Audio_AdacToolGetAdjustInterval(tmpRight);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A606, tmpLeft);
	while((tmpLeft != tmpTargetLeft)
		||(tmpRight != tmpTargetRight))
	{
		if(tmpLeft != tmpTargetLeft)
		{
			tmpLeft = Audio_AdacToolGetSrcToDstVal(tmpLeft, tmpTargetLeft, tmpIntervalLeft);
			HWRITEW(CORE_DAC_DC_CALI_L, tmpLeft);
		}
		if(tmpRight != tmpTargetRight)
		{
			tmpRight = Audio_AdacToolGetSrcToDstVal(tmpRight, tmpTargetRight, tmpIntervalRight);
			HWRITEW(CORE_DAC_DC_CALI_R, tmpRight);
		}
		SYS_delay_ms(1);
	}
}

void Audio_AdacAdjustDigitalVolToCali(void)
{
	uint16_t tmpLeft = HREAD(CORE_DAC_LVOL);
	uint16_t tmpTargetLeft = ADAC_SETTING_DIGITAL_VOL_LIMIT;
	uint16_t tmpIntervalLeft = Audio_AdacToolGetAdjustInterval(tmpTargetLeft);
	
	uint16_t tmpRight = HREAD(CORE_DAC_RVOL);
	uint16_t tmpTargetRight = ADAC_SETTING_DIGITAL_VOL_LIMIT;
	uint16_t tmpIntervalRight = Audio_AdacToolGetAdjustInterval(tmpTargetRight);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A605, tmpLeft);
	while((tmpLeft != tmpTargetLeft)
		||(tmpRight != tmpTargetRight))
	{
		if(tmpLeft != tmpTargetLeft)
		{
			tmpLeft = Audio_AdacToolGetSrcToDstVal(tmpLeft, tmpTargetLeft, tmpIntervalLeft);
			HWRITE(CORE_DAC_LVOL, tmpLeft);
		}
		if(tmpRight != tmpTargetRight)
		{
			tmpRight = Audio_AdacToolGetSrcToDstVal(tmpRight, tmpTargetRight, tmpIntervalRight);
			HWRITE(CORE_DAC_RVOL, tmpRight);
		}
		SYS_delay_ms(1);
	}
}

void Audio_AdacAdjustDigitalVolToZero(void)
{
	uint16_t tmpLeft = HREAD(CORE_DAC_LVOL);
	uint16_t tmpTargetLeft = 0;
	uint16_t tmpIntervalLeft = Audio_AdacToolGetAdjustInterval(tmpLeft);
	
	int16_t tmpRight = HREAD(CORE_DAC_RVOL);
	int16_t tmpTargetRight = 0;
	uint16_t tmpIntervalRight = Audio_AdacToolGetAdjustInterval(tmpRight);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A605, tmpLeft);
	while((tmpLeft != tmpTargetLeft)
		||(tmpRight != tmpTargetRight))
	{
		if(tmpLeft != tmpTargetLeft)
		{
			tmpLeft = Audio_AdacToolGetSrcToDstVal(tmpLeft, tmpTargetLeft, tmpIntervalLeft);
			HWRITE(CORE_DAC_LVOL, tmpLeft);
		}
		if(tmpRight != tmpTargetRight)
		{
			tmpRight = Audio_AdacToolGetSrcToDstVal(tmpRight, tmpTargetRight, tmpIntervalRight);
			HWRITE(CORE_DAC_RVOL, tmpRight);
		}
		SYS_delay_ms(1);
	}
}

// When we need start work, shoul change ana vol to ready state, dc cali need be zero(becouse is will make voice work error)
void Audio_AdacPrepareReadyWork(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A602, HREAD(CORE_AADC_9));
	//TODO: Current bottom nosie is 10-11uV, no need do this work
	/*
	if(!Audio_AdacCheckAnaVolAlreadyNormal())
	{
		// Step 1: prepare dc cali and digital vol
		Audio_AdacAdjustDCCaliToNormal();
		Audio_AdacAdjustDigitalVolToCali();
		
		// Step 2: change ana vol to normal
		Audio_AdacAdjustAnaVolToNormal();
	}
	else
	{
		DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A610, HREAD(CORE_AADC_9));
	}
	*/
	// Step 3: change dc and vol to zero, becouse dc cali will make voice error
	Audio_AdacAdjustDCCaliToZero();
	Audio_AdacAdjustDigitalVolToZero();
}
void Audio_AdacPreparePAClose(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A60E, HREAD(CORE_AADC_9));
	
	// Step 1: prepare dc cali and digital vol
	Audio_AdacAdjustDCCaliToNormal();
	Audio_AdacAdjustDigitalVolToCali();
	
	//TODO: Current bottom nosie is 10-11uV, no need do this work
	// Step 2: change ana vol to normal
	//Audio_AdacAdjustAnaVolToLimit();
}
void Audio_AdacPrepareSoftMute(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A603, HREAD(CORE_AADC_9));
	//TODO: Current bottom nosie is 10-11uV, no need do this work
	/*
	// Step 1: prepare dc cali and digital vol
	Audio_AdacAdjustDCCaliToNormal();
	Audio_AdacAdjustDigitalVolToCali();
	
	// Step 2: change ana vol to normal
	Audio_AdacAdjustAnaVolToLimit();
	*/
}

uint8_t Audio_AdacCheckLastDacClkSame(uint8_t dac_clk)
{
	return gLastStartDacClk == dac_clk;
}



SYS_TIMER_TYPE gClosePaDelayTimer;
void Audio_CBClosePaDelay(int paras)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A622, HREAD(CORE_AADC_9));

	Audio_AdacStopPAWithPop();
}
void Audio_ClosePaDelay(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A620, HREAD(CORE_AADC_9));
	SYS_SetTimer(&gClosePaDelayTimer, CLOSE_PA_DELAY,
		TIMER_SINGLE, Audio_CBClosePaDelay);
}

void Audio_RelasePaDelay(void)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", LOG_POINT_A621, HREAD(CORE_AADC_9));
	SYS_ReleaseTimer(&gClosePaDelayTimer);
}

void Audio_AvdStopSbcDac(void)
{
	HWRITE(mem_audio_allow, AVD_AUDIO_FORBID);
	Audio_SbcStop();
	Audio_DacStop();
}

void Audio_AnaAdcLowControlDisable(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_adc_low);
	tmp &= ~(1<<da_aadc_en);
	tmp &= ~(1<<da_aadc_en_biasgen);
	tmp &= ~(1<<da_aadc_en_constgm);
	tmp &= ~(1<<da_aadc_en_reg);
	
	Audio_LpmWrite2AdcLow(tmp);
}

void Audio_AnaAdcLowControlEn(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_adc_low);
	tmp |= (1<<da_aadc_en);
	tmp |= (1<<da_aadc_en_biasgen);
	tmp |= (1<<da_aadc_en_constgm);
	tmp |= (1<<da_aadc_en_reg);
	//tmp &= ~(1<<rg_mic_diff_en);//Disable mic diff
	tmp |= (1<<rg_mic_diff_en);//Enable mic diff
	
	Audio_LpmWrite2AdcLow(tmp);
}

void Audio_AnaAdcHighControlDisable(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_adc_high);
	tmp &= ~(1<<da_mic_pga_en);
	tmp &= ~(1<<da_mic_bias_en);
	
	Audio_LpmWrite2AdcHigh(tmp);
}
void Audio_AnaAdcHighControlEn(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_adc_high);
	tmp |= (1<<da_mic_pga_en);
	tmp |= (1<<da_mic_bias_en);
	
	Audio_LpmWrite2AdcHigh(tmp);
}


void Audio_AnaStartAdcWork(void)
{
	Audio_DaAaddaBgEn();
	Audio_AnaAdcLowControlEn();
	Audio_AnaAdcHighControlEn();
}


void Audio_AnaStopAdcWork(void)
{
	//Audio_DaAaddaBgDisable();  DAC close
	Audio_AnaAdcLowControlDisable();
	Audio_AnaAdcHighControlDisable();
}


BOOL Audio_CheckDacEnable(void)
{
	return ((HREAD(CORE_DAC_CTRL) & BIT_0) != 0); //DAC_ENABLE
}


uint16_t Audio_GetDacBufferSize(void)
{
	uint16_t adcProcessRptr, adcRealWptr;
	
	// Check adc buffer already for cvsd work
	adcProcessRptr = HREADW(CORE_DAC_RPTR);
	adcRealWptr = HREADW((CORE_DAC_WPTR));
	if(adcRealWptr >= adcProcessRptr)
	{
		return (adcRealWptr - adcProcessRptr);
	}
	else
	{
		return (HREADW(CORE_DAC_LEN) + 1 - (adcProcessRptr  - adcRealWptr));
	}
}

void Audio_SbcClear(void)
{
	Audio_SbcClkOn(); 
	
	// For safe, init work should close sbc block first
	HWRITE(CORE_SBC_CTRL2, 0);
	
	HWRITEW(CORE_SBC_SWP, 0);
	// clear last info, include the write wptr or rptr
	HWRITE(CORE_SBC_CLR, 0xff);
	SYS_delay_us(1);
	HWRITE(CORE_SBC_CLR, 0x0);
	// here need to think should enbale here?
	HWRITE(CORE_SBC_CTRL2, 1);
	
	Audio_SbcBusyWait();
}
void Audio_AdcStart16K(BOOL adcMramFlag)
{
	HWRITE(CORE_ADCD_DELAY, 0x80);
	HWRITE(CORE_ADCD_CTRL, 0x10);
	if(adcMramFlag)
	{
		HWOR(CORE_ADCD_CTRL, BIT_6);
	}
	else
	{
		HWCOR(CORE_ADCD_CTRL, BIT_6);
	}
	// Start voice filter clock
	HWCORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
	Audio_AdcDmaEnable();
}
void Audio_mSBCInitAdc(void)
{
	uint16_t adcStartAddr = MSBC_NEW_ADC_8K_START_ADDR;
	uint16_t adcBufferSize = MSBC_NEW_PCM_MAX_BUFFER_LEN;
	BOOL adcMramFlag = TRUE;

	Audio_AdcInit(adcStartAddr, adcBufferSize, adcMramFlag, 1);
	DEBUG_LOG(LOG_LEVEL_CORE, "UI" ,"Audio_DacInit: 0x%04X", 0xf001, HREAD(CORE_ADCD_CTRL));
}
void Audio_mSBCInitDac(void)
{
	uint32_t dacStartAddr = MSBC_NEW_DAC_48K_START_ADDR;
	uint16_t dacClk = 16;
	uint16_t dacBufSize = MSBC_NEW_DAC_48K_MAX_BUFFER_LEN;//6*2=12  6 means 8K to 48K
	BOOL isMram = FALSE;//CVSD_SELECT_MRAM
	
#ifdef TEST_FUNCTION_USE_SPECIAL_VOICE_ENABLE
	extern void App_TestForceVoice(void);
	App_TestForceVoice();
#else
	Audio_DacInitVpCall(dacClk, isMram
		, dacStartAddr, dacBufSize);
#endif
}






