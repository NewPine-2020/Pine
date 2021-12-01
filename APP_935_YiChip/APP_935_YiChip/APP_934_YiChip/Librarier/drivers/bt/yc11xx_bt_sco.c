#include "yc11xx_bt_sco.h"
#include "yc11xx_dev_oal.h"
#include "yc11xx_mSBC.h"
uint8_t HFPstartflag=0;
uint32_t scocnt=0;
uint32_t cvsdcnt=0;
uint8_t gBtScoAllowAdcWork = 0;
volatile BT_SCO_DATA_FIFO *gpM0RespinSCOFifo;
//volatile BT_SCO_DATA_FIFO gM0SCOFifoProcess;
volatile OAL_BT_SCO_DATA_FIFO gM0SCOFifoProcess BIG_MEMORY_LPM_NOT_PROTECT_ATTRIBUTES;
volatile BT_SCO_DATA_RX_TIMER *gpM0RespinSCORxClk;

BT_NR_AEC_WORK_INFO gNRAECWorkInfo;

#define MAX_TEST_SCO_LEN 80
uint8_t gTestModeInd = 0;
uint8_t gRxADCBuffer[MAX_TEST_SCO_LEN][60] BIG_MEMORY_LPM_NOT_PROTECT_ATTRIBUTES=
{
0x55,
};
uint16_t gSCORxErrorCnt;
uint16_t gSCORxNoneCnt;
uint16_t gSCORxTotalCnt;
uint8_t gCvsdStartFlag=0;
uint16_t gSCOTxPacketLen = 0;
uint16_t gSCORxPacketLen = 0;
uint16_t gSCORxPacketType = 0;
#ifdef FUNCTION_CM0_BEFORECVSD_ADC_DEBUG
uint8_t gcvsdtestbuf[200];
#endif

uint8_t gHfpAllowDacWork = 0;
uint8_t gScoTwsFirstPacketProtect = 0;

void Bt_HfpStart(void)
{
	DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D00E, HREAD(mem_audio_allow), HREAD(mem_vp_enable));
	gSCOTxPacketLen = HREAD(mem_sco_tx_packet_len);
	gSCORxPacketLen = HREAD(mem_sco_rx_packet_len);
	gSCORxPacketType = HREAD(mem_sco_rx_packet_type);
	// Check muisc play or not
	if(HREAD(mem_audio_allow) != AVD_AUDIO_FORBID)
	{
		Audio_AvdStopSbcDac();
	}

	if(HREAD(mem_vp_enable) != VP_STATE_DISENABLE)
	{
		return;
	}

	switch(HREAD(mem_codec_type))
	{
		case CODEC_TYPE_CVSD:
			gCvsdStartFlag =M0_CVSD_START_FLAG_ENABLE;
			Audio_CvsdInit();
			break;
		case CODEC_TYPE_mSBC:
			mSBC_Init();
			break;
	}
}

void Bt_HfpStop(void)
{
	uint8_t tmp = HREAD(mem_vp_enable);
	DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D00F, HREAD(mem_enable_hfp_start_sync_new), HREAD(mem_vp_enable));

	HWRITE(mem_enable_hfp_start_sync_new, 0);
	if(((tmp & (1<<VP_MODE_BIT)) == 0)
		&& (tmp != VP_STATE_DISENABLE))
	{
		return;
	}

	
	Audio_DacStop();
	Audio_AdcStop();

#ifdef FUNCTION_OAL_MODE
	Oal_Restart();
#endif
	
	switch(HREAD(mem_codec_type))
	{
		case CODEC_TYPE_CVSD:
			Audio_CvsdStop();
			break;
		case CODEC_TYPE_mSBC:
			mSBC_Stop();
			break;
	}
}
void Bt_HfpEnable(void)
{
	uint32_t periodUs = 7500;
	DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D00A, HREAD(mem_sco_rx_packet_type), HREAD(mem_sco_rx_packet_len));

	if(HREAD(mem_param_sco_use_cm0) != 0)
	{
		HWRITE(mem_media_in_fast_mode, 0);
		// Start sync work
		Bt_SndCmdHFPStartSync();

		SYS_ChangeDPLL(CLOCK_192M_multiple);  //start
		Bt_SCOInit();
		Bt_HfpStart();
		// stop cvsd encode and decode work.
		Bt_CVSDWorkDisable();

		if(HREAD(mem_enable_hfp_start_sync_new))
		{
			gHfpAllowDacWork = 0;
			gScoTwsFirstPacketProtect = 0;
			// dac stop
			HWCOR(CORE_DAC_CTRL, BIT_0);//DAC_ENABLE
			// NR/AEC init
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
			Bt_NRAECAdcInitWithTwsSync();
#endif
		}
		
		// Check if the HV3/EV3 packet
		if(Bt_CheckSCOInHV3EV3Transaction())
		{
			periodUs = 3750;
		}
		Bt_SCOTimerConfig(periodUs);
		SCO_NVIC_Config();
		HFPstartflag=1;
	}
}
void Bt_HfpDisable(void)
{
	if(HREAD(mem_param_sco_use_cm0) != 0)
	{
		SYS_ChangeDPLL(CLOCK_48M_multiple);  //end
		SCO_NVIC_DeConfig();

		Bt_HfpStop();
		Bt_SCOInit();
		// stop cvsd encode and decode work.
		Bt_CVSDWorkDisable();
		gNRAECWorkInfo.nr_flag = 0;
		HFPstartflag=0;
	}
}


void Bt_HfpRestart(void)
{
	if(HREAD(mem_param_sco_use_cm0) != 0)
	{
		Bt_HfpDisable();
		Bt_HfpEnable();
	}
}


void Bt_SCOTimerConfig(uint32_t periodUs)
{
	TIM_InitTypeDef TIM_InitStruct;

	// Origin clock is 48M
	TIM_InitStruct.TIMx = SCO_PWM_DEFINE;
	TIM_InitStruct.mode = TIM_Mode_TIMER;
	TIM_InitStruct.period = periodUs*3 - 1;// user FREQUENCY_DIVISION_4, unit is 3
	TIM_InitStruct.frequency = FREQUENCY_DIVISION_4;
	
	TIM_Init(&TIM_InitStruct);
	TIM_Cmd(SCO_PWM_DEFINE, ENABLE);
}


void Bt_SCODataProcess(void)
{
	uint32_t tempPersent = 0;
	// avoid warning
	tempPersent = tempPersent;
	BOOL isRxPacket = TRUE;
	BOOL isRxBasebandPacket = TRUE;
	volatile BT_SCO_RX_DATA * realRxDataPtr;
	volatile BT_SCO_RX_DATA_FIFO * prxM0RespinDataFifo = &(gpM0RespinSCOFifo->rxDataFifo);
	volatile BT_SCO_TX_DATA_FIFO * ptxM0RespinDataFifo = &(gpM0RespinSCOFifo->txDataFifo);
	volatile BT_CLK_INFO* rxTimerPtr = NULL;
	//DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D000, prxM0RespinDataFifo->rptr, prxM0RespinDataFifo->wptr);
	//DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D001, ptxM0RespinDataFifo->rptr, ptxM0RespinDataFifo->wptr);

	// TEST_MODE_SCO 2
	if ((HREAD(mem_test_mode) == 1) && (HREAD(mem_test_mode_status) == 2))
    {    
		prxM0RespinDataFifo->wptr += 1;
		if(prxM0RespinDataFifo->wptr == M0_SCO_RX_BUFFER_SIZE)
		{
			prxM0RespinDataFifo->wptr = 0;
		}
    }

	// Check rptr and wptr, to judge receive valid packet
	if(prxM0RespinDataFifo->rptr
		== prxM0RespinDataFifo->wptr)
	{
		isRxPacket = FALSE;
		isRxBasebandPacket = FALSE;
		gSCORxNoneCnt ++;
	}
	else
	{
		scocnt++;
		realRxDataPtr = &(prxM0RespinDataFifo->scoRxData[prxM0RespinDataFifo->rptr]);
		rxTimerPtr = &(gpM0RespinSCORxClk->rxTimer[prxM0RespinDataFifo->rptr]);
		// check packet valid or not
		//if(Bt_CheckSCOInHV3Transaction())
		if(gSCORxPacketType  == 0x02)
		{
			if((realRxDataPtr->bufferValid & 0x08) == 0)
			{
				isRxPacket = FALSE;
				gSCORxErrorCnt ++;
			}
		}
		else
		{
			if((realRxDataPtr->bufferValid & 0x01) == 0)
			{
				isRxPacket = FALSE;
				gSCORxErrorCnt ++;
			}
		}
		
		prxM0RespinDataFifo->rptr += 1;
		if(prxM0RespinDataFifo->rptr == M0_SCO_RX_BUFFER_SIZE)
		{
			prxM0RespinDataFifo->rptr = 0;
		}
	}

	if(gScoTwsFirstPacketProtect < TWS_SCO_CACHE_BUFFER_CNT+4)
	{
		gScoTwsFirstPacketProtect++;
	}
	gSCORxTotalCnt ++;
	if(gSCORxTotalCnt >= 1000)
	{
		if(!SystemCheckInFastSpeed())
		{
			DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D00B, HREAD(mem_sco_rx_packet_type), HREAD(mem_sco_rx_packet_len));
			// For debug, need print the error rate
			tempPersent = gSCORxErrorCnt * 100 *100/gSCORxTotalCnt;
			DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D00C, tempPersent/100, tempPersent%100);
			tempPersent = gSCORxNoneCnt * 100 *100/gSCORxTotalCnt;
			DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D00D, tempPersent/100, tempPersent%100);
			//avoid warning
			tempPersent = tempPersent;
		}

		gSCORxErrorCnt = 0;
		gSCORxNoneCnt = 0;
		gSCORxTotalCnt = 0;
	}
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D002, isRxPacket);
	if(HREAD(mem_test_mode) != 1) 
	{
		Bt_SCOPushToLocalRxBuffer(isRxPacket,isRxBasebandPacket, realRxDataPtr->bufferData, rxTimerPtr);
	}	// Check Tx
	Bt_SCOPopFromLocalTxBuffer((uint8_t *)&(ptxM0RespinDataFifo->scoTxData[ptxM0RespinDataFifo->wptr]));
	if(HREAD(mem_test_mode) == 1)
	{
			//if (gTestModeInd == 0)
		Bt_SCOPushToLocalRxBuffer(TRUE,TRUE,(uint8_t *)gRxADCBuffer[gTestModeInd], NULL);
		for(int k=0;k<60;k++){
			gRxADCBuffer[gTestModeInd][k] = 
				ptxM0RespinDataFifo->scoTxData[ptxM0RespinDataFifo->wptr].bufferData[k];
		}
		gTestModeInd++;
		if (gTestModeInd>=MAX_TEST_SCO_LEN)
		gTestModeInd = 0; 
	}
	ptxM0RespinDataFifo->wptr += 1;
	if(ptxM0RespinDataFifo->wptr == M0_SCO_TX_BUFFER_SIZE)
	{
		ptxM0RespinDataFifo->wptr = 0;
	}

	// TEST_MODE_SCO 2
	if ((HREAD(mem_test_mode) == 1) && (HREAD(mem_test_mode_status) == 2))
    {
		ptxM0RespinDataFifo->rptr += 1;
		if(ptxM0RespinDataFifo->rptr == M0_SCO_RX_BUFFER_SIZE)
		{
			ptxM0RespinDataFifo->rptr = 0;
		}
    }

	
}

void Bt_SCOPushToLocalRxBuffer(uint8_t isRxPacket, uint8_t isRxBasebandPacket
	, volatile uint8_t* dataPtr, volatile BT_CLK_INFO* rxTimerPtr)
{
	//volatile BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	volatile OAL_BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	// drop packets
	if(!gHfpAllowDacWork)
	{
		if(HREAD(mem_enable_hfp_start_sync_new) 
			&& !HREAD(mem_hfp_start_dac_work))
		{
		}
		else
		{
			gHfpAllowDacWork = 1;
		}
	}

	//avoid limit area.
	if(!gHfpAllowDacWork)
	{
		return;
	}
	if(!isRxBasebandPacket
		&& (gScoTwsFirstPacketProtect < TWS_SCO_CACHE_BUFFER_CNT+4))
	{
		//Skip.
		return;
	}
	// Becouse current no ellisys, so is no baseband packet, avoid master slave mismatch, just drop the packet.
	if(!isRxBasebandPacket)
	{
		//Skip.
		return;
	}
	if(rxTimerPtr == NULL)
	{
		prxDataFifo->scoRxData[prxDataFifo->wptr].rxClk.clke_rt = 0;
		prxDataFifo->scoRxData[prxDataFifo->wptr].rxClk.clke_bt = 0;
	}
	else
	{
		prxDataFifo->scoRxData[prxDataFifo->wptr].rxClk.clke_rt = rxTimerPtr->clke_rt;
		prxDataFifo->scoRxData[prxDataFifo->wptr].rxClk.clke_bt = rxTimerPtr->clke_bt;
	}
	if(isRxPacket)
	{
		prxDataFifo->scoRxData[prxDataFifo->wptr].bufferValid = 1;
		memcpy((uint8_t*)prxDataFifo->scoRxData[prxDataFifo->wptr].bufferData
			, (uint8_t*)dataPtr, 60);
	}
	else
	{
		prxDataFifo->scoRxData[prxDataFifo->wptr].bufferValid = 0;
	}

	prxDataFifo->wptr += 1;
	//if(prxDataFifo->wptr == M0_SCO_RX_BUFFER_SIZE)
	if(prxDataFifo->wptr == OAL_M0_SCO_BUFFER_SIZE)
	{
		prxDataFifo->wptr = 0;
	}
	if(prxDataFifo->wptr==prxDataFifo->rptr)
	{
		if(!SystemCheckInFastSpeed())
		{
			DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_C502, prxDataFifo->wptr);
		}
	}
}
/*const uint8_t errval[]={0x01,0x08,0xad,0x00,0x00,0x71,0xec,0xbe,0xeb,0xcd 
,0xbb,0x5d,0xd6,0xee,0xd7,0x75,0xbb,0xb5,0xdd,0x6e,0xd2,0x77,0x14,
0x85,0x16,0x63,0xaf,0x53,0xb4,0xbd 
,0x31,0xa2,0x6b,0x0d,0x77,0x5b,0xc3,0x5d,0xd6,0xee,0xd7,0x75,0xbb,0xb5
,0xdd,0x6e,0xed,0x77,0x5b,0xbb,0x5d,0xd6,0xee,0xd7,0x75,0xbb,0xb5,0xdd,
0x6c,0x00};*/
void Bt_SCOPopFromLocalTxBuffer(uint8_t* dataPtr)
{
	//volatile BT_SCO_TX_DATA_FIFO* ptxDataFifo = &gM0SCOFifoProcess.txDataFifo;
	volatile OAL_BT_SCO_TX_DATA_FIFO* ptxDataFifo = &gM0SCOFifoProcess.txDataFifo;
	//DEBUG_LOG_2(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe000, ptxDataFifo->rptr, ptxDataFifo->wptr);

	uint16_t scoTxPacketLength = gSCOTxPacketLen;
	uint8_t i;
	if(ptxDataFifo->rptr
		== ptxDataFifo->wptr)
	{
		for(i = 0; i < scoTxPacketLength; i ++)
		{
			*((uint8_t *)(dataPtr + i)) = 0x55;//errval[i];
		}
	}
	else
	{
		memcpy(dataPtr
			, (uint8_t *)ptxDataFifo->scoTxData[ptxDataFifo->rptr].bufferData, 60);
			
		ptxDataFifo->rptr += 1;
		//if(ptxDataFifo->rptr == M0_SCO_TX_BUFFER_SIZE)
		if(ptxDataFifo->rptr == OAL_M0_SCO_BUFFER_SIZE)
		{
			ptxDataFifo->rptr = 0;
		}
	}
}

void Bt_SCOInit(void)
{
	gSCORxErrorCnt = 0;
	gSCORxNoneCnt = 0;
	gSCORxTotalCnt = 0;

	gpM0RespinSCOFifo = (BT_SCO_DATA_FIFO *)M0_SCO_BUF_START_ADDR;
	gpM0RespinSCORxClk = (BT_SCO_DATA_RX_TIMER *)(M0_SCO_BUF_START_ADDR + 0x170);//M0_SCO_RX_CLKE_TIMER_OFFSET

	memset((void *)gpM0RespinSCOFifo, 0, sizeof(BT_SCO_DATA_FIFO));
	//memset((void *)&gM0SCOFifoProcess, 0, sizeof(BT_SCO_DATA_FIFO));
	memset((void *)&gM0SCOFifoProcess, 0, sizeof(OAL_BT_SCO_DATA_FIFO));
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
	memset((void *)&gNRAECWorkInfo, 0, sizeof(BT_NR_AEC_WORK_INFO));
#endif
	 if (HREAD(mem_test_mode) == 1)
    {
        gTestModeInd = 0;
        for(int i = 0;i<MAX_TEST_SCO_LEN;i++)
        {
            for(int j=0;j<60;j++)
            {
                gRxADCBuffer[i][j] = 0x55;
            }
        }
    }
}
void Bt_MsbcPtrUpdata(void)
{
	gNRAECWorkInfo.dac_rptr=HREADW(CORE_DAC_WPTR);
	gNRAECWorkInfo.adc_wptr = HREADW(CORE_ADCD_SADDR);
}
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
void Bt_NRAECWorkStart(void)
{
	gNRAECWorkInfo.nr_flag = 1;
}
BOOL Bt_NRAECCheckWorkStart(void)
{
	if(gNRAECWorkInfo.nr_flag == 0)
	{
		return FALSE;
	}

	return TRUE;
}
void Bt_NRAECAdcReinit(uint8_t calltype)
{
	uint16_t frameSize = gSCORxPacketLen * 2;//CVSD is double
	Audio_AdcStop();
	if(calltype)
	{
		Audio_mSBCInitAdc();
		Bt_MsbcPtrUpdata();
	}
	else
	{
		Audio_AdcCvsdInit();

		// dac rptr point to the 8K Dac out current wptr
		gNRAECWorkInfo.dac_rptr = HREADW(CORE_PCMOUT_PTR) - frameSize;
		// Here need be think buffer overflow
		if(gNRAECWorkInfo.dac_rptr < HREADW(CORE_PCMOUT_SADDR))
		{
			gNRAECWorkInfo.dac_rptr += (HREADW(CORE_CVSD_BUF_LEN) + 1)*2;
		}
	
		gNRAECWorkInfo.adc_wptr = HREADW(CORE_ADCD_SADDR);
	}	
	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe020, 0x20);
	gNRAECWorkInfo.nr_flag = 1;
}
void Bt_NRAECAdcInitWithTwsSync(void)
{
	uint16_t frameSize = gSCORxPacketLen * 2;//CVSD is double
	Audio_AdcStop();
	if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	{
		Audio_mSBCInitAdc();
		Audio_AdcDmaDisable();
		Bt_MsbcPtrUpdata();
	}
	else
	{
		Audio_AdcCvsdInit();

		Audio_AdcDmaDisable();

		// dac rptr point to the 8K Dac out current wptr
		gNRAECWorkInfo.dac_rptr = HREADW(CORE_PCMOUT_PTR) - frameSize;
		// Here need be think buffer overflow
		if(gNRAECWorkInfo.dac_rptr < HREADW(CORE_PCMOUT_SADDR))
		{
			gNRAECWorkInfo.dac_rptr += (HREADW(CORE_CVSD_BUF_LEN) + 1)*2;
		}
	
		gNRAECWorkInfo.adc_wptr = HREADW(CORE_ADCD_SADDR);
	}
}
uint16_t Bt_NRAECGetAdcCurrentBufferSize(void)
{
	uint16_t adcWptr, pcmInSrcPtr;
	
	// Check adc buffer already for cvsd work
	adcWptr = gNRAECWorkInfo.adc_wptr;
	if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	{
		pcmInSrcPtr = HREADW(mem_msbc_pcm_in_rptr);	
	}
	else
	{
		pcmInSrcPtr = HREADW(CORE_PCMIN_SADDR);
	}
	if(!Bt_NRAECCheckWorkStart())
	{
		return 0;
	}
	if(adcWptr >= pcmInSrcPtr)
	{
		return (adcWptr - pcmInSrcPtr);
	}
	else
	{
		return (HREADW(mem_adc_buf_size) - (pcmInSrcPtr  - adcWptr));
	}
}

uint16_t Bt_GetAdcBufferSizeBeforeNRAEC(void)
{
	uint16_t adcProcessRptr, adcRealWptr;
	
	// Check adc buffer already for cvsd work
	if(!Bt_NRAECCheckWorkStart())
	{
		return 0;
	}
	adcProcessRptr = gNRAECWorkInfo.adc_wptr;
	adcRealWptr = HREADW(CORE_ADCD_ADDR);
	
	if(adcRealWptr >= adcProcessRptr)
	{
		return (adcRealWptr - adcProcessRptr);
	}
	else
	{
		return (HREADW(mem_adc_buf_size) - (adcProcessRptr  - adcRealWptr));
	}
}



uint16_t  BT_AECDacPtrJudge(void)
{
	uint16_t frameNrAecFifoSize;
	uint16_t len,temp1;
	if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	{
		frameNrAecFifoSize=256;
		temp1 = HREADW(CORE_DAC_SADDR) + MSBC_NEW_DAC_48K_MAX_BUFFER_LEN ;	
	}
	else
	{
		frameNrAecFifoSize=128;
		temp1 = HREADW(CORE_PCMOUT_SADDR) + (HREADW(CORE_CVSD_BUF_LEN) + 1)*2 ;
	}
	
	if(gNRAECWorkInfo.dac_rptr+frameNrAecFifoSize > temp1)
	{
		len= temp1 -gNRAECWorkInfo.dac_rptr;
	}
	else
	{
		len=0;
	}
	return len;
}


void BT_AECProcess(void)
{
	uint16_t frameNrAecFifoSize = 128;
	int16_t temp,i;
	uint16_t j=0;
	int16_t AECADC_Nearbuf[64],AECDAC_Farbuf[64],AEC_Outbuf[64];
	int16_t pcminptr= HREADW(CORE_ADCD_SADDR);
	int16_t pcmoutptr =HREADW(CORE_PCMOUT_SADDR);
	uint32_t AEC_Nearptr = gNRAECWorkInfo.adc_wptr;
	uint32_t AEC_Farptr =gNRAECWorkInfo.dac_rptr;
	temp = HREADW(CORE_ADCD_EADDR) + 1;
	if(gNRAECWorkInfo.adc_wptr+frameNrAecFifoSize > temp)
	{
		i = temp-gNRAECWorkInfo.adc_wptr;
		xmemcpy((uint8_t *)AECADC_Nearbuf,(void *)(reg_map(AEC_Nearptr)),i);
		xmemcpy((void *)(((uint8_t *)AECADC_Nearbuf) + i),(void *)(reg_map(pcminptr)),frameNrAecFifoSize-i);
		//USART_SendDataFromBuff(UARTA,AECADC_Nearbuf,128);
		if((j =BT_AECDacPtrJudge()) != FALSE)
		{
		 xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),j);
		 xmemcpy((void *)(((uint8_t *)AECDAC_Farbuf) + j),(void *)(reg_map(pcmoutptr)),frameNrAecFifoSize-j);	
		}
		else
		{
		 xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),frameNrAecFifoSize);
		}
		
		AECM(AECADC_Nearbuf,AECDAC_Farbuf,AEC_Outbuf,0);
		xmemcpy((void *)(reg_map(AEC_Nearptr)),(void *)AEC_Outbuf,i);
		xmemcpy((void*)(reg_map(pcminptr)),(void *)(((uint8_t *)AEC_Outbuf) + i),frameNrAecFifoSize-i);
		
	}
	else
	{
		xmemcpy((uint8_t *)AECADC_Nearbuf,(void *)(reg_map(AEC_Nearptr)), frameNrAecFifoSize);
	//	USART_SendDataFromBuff(UARTA,AECADC_Nearbuf,128);
		if((j =BT_AECDacPtrJudge()) != FALSE)
		{
		 xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),j);
		 xmemcpy((void *)(((uint8_t *)AECDAC_Farbuf) + j),(void *)(reg_map(pcmoutptr)),frameNrAecFifoSize-j);	
		}
		else
		{
		 xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),frameNrAecFifoSize);
		}
		AECM(AECADC_Nearbuf,AECDAC_Farbuf,AEC_Outbuf,0);
		xmemcpy((void *)(reg_map(AEC_Nearptr)),(uint8_t *)AEC_Outbuf, frameNrAecFifoSize);

	}

	//USART_SendDataFromBuff(UARTA,AEC_Outbuf,128);

}
void BT_AECProcessMsbc(void)
{
	uint16_t frameNrAecFifoSize = 256;
	int32_t temp,i;
	uint16_t j=0;
	int16_t AECADC_Nearbuf[128],AECDAC_Farbuf[128],AEC_Outbuf[128];
	int16_t pcminptr= HREADW(CORE_ADCD_SADDR);
	int16_t pcmoutptr =HREADW(CORE_DAC_SADDR);
	uint32_t AEC_Nearptr = gNRAECWorkInfo.adc_wptr;// HREADW(CORE_ADCD_SADDR);
	uint32_t AEC_Farptr =gNRAECWorkInfo.dac_rptr;// HREADW(CORE_PCMOUT_PTR) - frameSize;//pcm out data read pointer
	if(AEC_Farptr==(MSBC_NEW_DAC_48K_START_ADDR+MSBC_NEW_DAC_48K_MAX_BUFFER_LEN))
	{
		AEC_Farptr=MSBC_NEW_DAC_48K_START_ADDR;
	}
	if(AEC_Nearptr==MSBC_NEW_ADC_8K_START_ADDR+MSBC_NEW_PCM_MAX_BUFFER_LEN)
	{
		AEC_Nearptr=MSBC_NEW_ADC_8K_START_ADDR;
	}
	temp = HREADW(CORE_ADCD_EADDR) + 1;
	if(AEC_Nearptr+frameNrAecFifoSize > temp)
	{
		//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd503, AEC_Nearptr);
		i = temp-gNRAECWorkInfo.adc_wptr;
		memcpy((uint8_t *)AECADC_Nearbuf,(void *)(reg_map_m0(AEC_Nearptr)),i);
		memcpy((void *)(((uint8_t *)AECADC_Nearbuf) + i),(void *)(reg_map_m0(pcminptr)),frameNrAecFifoSize-i);
		//USART_SendDataFromBuff(UARTA,(uint8_t*)AECADC_Nearbuf,256);
		if((j =BT_AECDacPtrJudge()) != FALSE)
		{
		 	xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),j);
			xmemcpy((void *)(((uint8_t *)AECDAC_Farbuf) + j),(void *)(reg_map(pcmoutptr)),frameNrAecFifoSize-j);	
		}
		else
		{
		 	xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),frameNrAecFifoSize);
		}
	
		//USART_SendDataFromBuff(UARTA,AECDAC_Farbuf,256);
		//USART_SendDataFromBuff(UARTA,(uint8_t *)reg_map_m0(AEC_Farptr),256);
		AECM(AECADC_Nearbuf,AECDAC_Farbuf,AEC_Outbuf,1);
		memcpy((void *)(reg_map_m0(AEC_Nearptr)),(void *)AEC_Outbuf,i);
		memcpy((void*)(reg_map_m0(pcminptr)),(void *)(((uint8_t *)AEC_Outbuf) + i),frameNrAecFifoSize-i);
		//USART_SendDataFromBuff(UARTA,AEC_Outbuf,256);
	}
	else
	{
		//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd504, AEC_Nearptr);
		memcpy((uint8_t *)AECADC_Nearbuf,(void *)(reg_map_m0(AEC_Nearptr)), frameNrAecFifoSize);
		//HWORW(CORE_CLKOFF, REG_CLOCK_OFF_VOICE_FILTER);
		//USART_SendDataFromBuff(UARTA,(uint8_t*)AECADC_Nearbuf,256);
		if((j =BT_AECDacPtrJudge()) != FALSE)
		{
		 	xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),j);
		 	xmemcpy((void *)(((uint8_t *)AECDAC_Farbuf) + j),(void *)(reg_map(pcmoutptr)),frameNrAecFifoSize-j);	
		}
		else
		{
		 	xmemcpy((uint8_t *)AECDAC_Farbuf,(void *)(reg_map(AEC_Farptr)),frameNrAecFifoSize);
		}
		AECM(AECADC_Nearbuf,AECDAC_Farbuf,AEC_Outbuf,1);
		memcpy((void *)(reg_map_m0(AEC_Nearptr)),(uint8_t *)AEC_Outbuf, frameNrAecFifoSize);
	}
	//USART_SendDataFromBuff(UARTA,AEC_Outbuf,256);
}
void BT_NRProcess(void)
{
	uint16_t frameNrAecFifoSize = 128;
	int16_t NRAEC_Inbuf[64];
	int16_t temp,i;
	int16_t pcminptr= HREADW(CORE_ADCD_SADDR);
	uint32_t NRinput_ptr = gNRAECWorkInfo.adc_wptr;
	temp = HREADW(CORE_ADCD_EADDR) + 1;
	if(gNRAECWorkInfo.adc_wptr+frameNrAecFifoSize > temp)
	{
		i =temp-gNRAECWorkInfo.adc_wptr;
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd333, i);
		xmemcpy((uint8_t *)NRAEC_Inbuf,(void *)(reg_map(NRinput_ptr)),i);
		xmemcpy((void *)(((uint8_t *)NRAEC_Inbuf) + i),(void *)(reg_map(pcminptr)),frameNrAecFifoSize-i);
		noise_suppression(NRAEC_Inbuf,NRAEC_Inbuf);
		xmemcpy((void *)(reg_map(NRinput_ptr)),(void *)NRAEC_Inbuf,i);
		xmemcpy((void*)(reg_map(pcminptr)),(void *)(((uint8_t *)NRAEC_Inbuf) + i),frameNrAecFifoSize-i);
	}
	else
	{
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd444, (NRAEC_Inbuf + 10));
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd555, (((uint8_t *)NRAEC_Inbuf) + 10));
		xmemcpy((uint8_t *)NRAEC_Inbuf,(void *)(reg_map(NRinput_ptr)), frameNrAecFifoSize);
		noise_suppression(NRAEC_Inbuf,NRAEC_Inbuf);
		xmemcpy((void *)(reg_map(NRinput_ptr)),(uint8_t *)NRAEC_Inbuf, frameNrAecFifoSize);
	}

}
void BT_NRProcessMsbc(void)
{
	uint16_t frameNrAecFifoSize = 256;
	int16_t NRAEC_Inbuf[128];
	int32_t temp,i;
	int32_t pcminptr= HREADW(CORE_ADCD_SADDR);
	uint32_t NRinput_ptr = gNRAECWorkInfo.adc_wptr;// HREADW(CORE_ADCD_SADDR);
	temp = HREADW(CORE_ADCD_EADDR) + 1;
	if(NRinput_ptr==MSBC_NEW_ADC_8K_START_ADDR+MSBC_NEW_PCM_MAX_BUFFER_LEN)
	{
		NRinput_ptr=MSBC_NEW_ADC_8K_START_ADDR;
	}
	if(NRinput_ptr+frameNrAecFifoSize > temp)
	{
		//DEBUG_LOG_STRING("NRProcess addr:%04x\r\n",reg_map_m0(NRinput_ptr));
		i =temp-gNRAECWorkInfo.adc_wptr;
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd333, i);
		xmemcpy((uint8_t *)NRAEC_Inbuf,(void *)(reg_map_m0(NRinput_ptr)),i);
		xmemcpy((void *)(((uint8_t *)NRAEC_Inbuf) + i),(void *)(reg_map_m0(pcminptr)),frameNrAecFifoSize-i);
		noise_suppression(NRAEC_Inbuf,NRAEC_Inbuf);
		xmemcpy((void *)(reg_map_m0(NRinput_ptr)),(void *)NRAEC_Inbuf,i);
		xmemcpy((void*)(reg_map_m0(pcminptr)),(void *)(((uint8_t *)NRAEC_Inbuf) + i),frameNrAecFifoSize-i);
	}
	else
	{
		//DEBUG_LOG_STRING("NRProcess1 addr:%04x\r\n",reg_map_m0(NRinput_ptr));
		xmemcpy((uint8_t *)NRAEC_Inbuf,(void *)(reg_map_m0(NRinput_ptr)), frameNrAecFifoSize);
		noise_suppression(NRAEC_Inbuf,NRAEC_Inbuf);
		xmemcpy((void *)(reg_map_m0(NRinput_ptr)),(uint8_t *)NRAEC_Inbuf, frameNrAecFifoSize);
	}

}

void Bt_NRAECDataProcess(void)
{
	if(VP_ModeJudge())
	{
		return;
	}
	//if(HREAD(mem_hf_call_state)<=HFP_OUTGOING)
	//{
	//	return;
	//}
#ifdef FUNCTION_OAL_MODE
	if(Oal_CheckIsWorkOn())
	{
		return;
	}
#endif
	uint16_t adcWaitProcessBufferSize = Bt_GetAdcBufferSizeBeforeNRAEC();
	uint16_t frameNrAecFifoSize = 128;
	uint16_t temp;
	if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	{
		frameNrAecFifoSize = 256;
	}

	if(!Bt_NRAECCheckWorkStart())
	{
		return;
	}
	
	if(adcWaitProcessBufferSize > frameNrAecFifoSize)
	{
		// Cal dac buffer size
	//	tempDac = Bt_GetDacBufferSize();

		// Cvsd need 4ms, nr need 7ms
	//	if(tempDac < (5*96))
	//		{
		//	return;
	//		}
		// NR Process
//		uint32_t i=HREADL(CORE_CLKN);
		if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
		{
			//BT_NRProcessMsbc();
			BT_AECProcessMsbc();
		}
		else
		{
			BT_NRProcess();
			BT_AECProcess();
		}
//		uint32_t j=HREADL(CORE_CLKN);
//		uint32_t clk=(j-i);
//		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"App_EvtCallBack: 0x%04X", LOG_POINT_D105, clk);
		// AEC Process

		// After process need update info
		adcWaitProcessBufferSize -= frameNrAecFifoSize;
		gNRAECWorkInfo.dac_rptr += frameNrAecFifoSize;
		
		if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
		{
			temp = HREADW(CORE_DAC_SADDR) +MSBC_NEW_DAC_48K_MAX_BUFFER_LEN ;
		}
		else
		{
			temp = HREADW(CORE_PCMOUT_SADDR) + (HREADW(CORE_CVSD_BUF_LEN) + 1)*2 ;
		}
		if(gNRAECWorkInfo.dac_rptr > temp)
		{
			if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
			{
				gNRAECWorkInfo.dac_rptr -=MSBC_NEW_DAC_48K_MAX_BUFFER_LEN;
			}
			else
			{
				gNRAECWorkInfo.dac_rptr -= (HREADW(CORE_CVSD_BUF_LEN) + 1)*2;
			}
		}
		
		gNRAECWorkInfo.adc_wptr += frameNrAecFifoSize;
		temp = HREADW(CORE_ADCD_EADDR) + 1;
		if(gNRAECWorkInfo.adc_wptr > temp)
		{
			if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
			{
				gNRAECWorkInfo.adc_wptr -=MSBC_NEW_PCM_MAX_BUFFER_LEN;
			}
			else
			{
				gNRAECWorkInfo.adc_wptr -= (HREADW(CORE_CVSD_BUF_LEN) + 1)*2;
			}
		}
	//	GPIO_SetGpioMultFunction(27, GPCFG_OUTPUT_LOW);
	}

}
#endif


uint16_t Bt_SCOGetAdcCurrentBufferSize(void)
{
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
	return Bt_NRAECGetAdcCurrentBufferSize();
#else
	return Bt_GetAdcCurrentBufferSize();
#endif
}
uint8_t Bt_CheckAllowCVSD(void)
{
	
	if(HREAD(mem_param_sco_use_cm0) == 0)
	{
		return 1;
	}
	if(!Bt_CheckHfpStart())
	{
		return 1;
	}
	if(Bt_CheckVpInprocess()
		&& !Bt_CheckVpInHardwareWork())
	{
		return 1;
	}
	if(Bt_CheckHfpLocalInprocess())
	{
		return 1;
	}
	if(!Bt_CheckCVSDWorkStart())
	{
		return 1;
	}

	return 0;

}
void Bt_CVSDDataPolling(void)
{
	//volatile BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	volatile OAL_BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	if(!SystemCheckInFastSpeed())
	{
		if(Bt_CheckAllowCVSD())
		{
			return ;
		}
	}
#ifdef TEST_FUNCTION_USE_SPECIAL_VOICE_ENABLE
	return;
#endif
	if(Bt_CheckCVSDEnable())
	{	
		// Check cvsd work success
		if(Bt_CheckCVSDWorkInprocess())
		{
			return;
		}
		// AEC will call Bt_CVSDDataPolling
		BOOL isNeedReturnToFastSpeed = FALSE;
		if(SystemCheckInFastSpeed())
		{
			SYS_ChangeToNormalDPLL();
			isNeedReturnToFastSpeed = TRUE;
		}
		Bt_CvsdWorkEnd();
		if(isNeedReturnToFastSpeed)
		{
			SYS_ChangeToFastDPLL();
		}
	}
	if(!Bt_CheckCVSDEnable())
	{
		// Check rptr and wptr, to judge receive valid packet
		if(prxDataFifo->rptr
			== prxDataFifo->wptr)
		{
			return;
		}
		// AEC will call Bt_CVSDDataPolling
		BOOL isNeedReturnToFastSpeed = FALSE;
		if(SystemCheckInFastSpeed())
		{
			SYS_ChangeToNormalDPLL();
			isNeedReturnToFastSpeed = TRUE;
		}
		// Check Dac enough
		uint32_t remain = HREADW(CORE_DAC_LEN) + 1 - Audio_GetDacBufferSize();
		if(remain < 120)
		{
			if(isNeedReturnToFastSpeed)
			{
				SYS_ChangeToFastDPLL();
			}
			return;
		}
		
		Bt_CvsdStartWork();
		if(isNeedReturnToFastSpeed)
		{
			SYS_ChangeToFastDPLL();
		}
	}
}
uint16_t gCurrentScoSeqNum;
void Bt_CvsdStartWork(void)
{
	BOOL isRxPacket = TRUE;
	volatile OAL_BT_SCO_RX_DATA * realRxDataPtr;
	//volatile BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	volatile OAL_BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	uint16_t scoTxPacketLength = gSCOTxPacketLen;
	uint16_t scoRxPacketLength = gSCORxPacketLen;
	uint32_t cvsdInDataPtr = 0;
	uint16_t i;
		// Check dac buffer size enough
		// Check rx
		{
			realRxDataPtr = &(prxDataFifo->scoRxData[prxDataFifo->rptr]);
			// check packet valid or not
			if((realRxDataPtr->bufferValid & 0x01) == 0)
			{
				isRxPacket = FALSE;
			}
			//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D010, isRxPacket);

			// put data to cvsd
			// update ptr
			/*
			if(((HREADW(CORE_CVSDIN_SADDR) + scoRxPacketLength) 
				>= (HREADW(mem_cvsd_in_addr_end))))
			{
				HWRITEW(CORE_CVSDIN_SADDR, (HREADW(mem_cvsd_in_addr_ptr)));
			}
			else
			{
				HWRITEW(CORE_CVSDIN_SADDR, (HREADW(CORE_CVSDIN_SADDR) + scoRxPacketLength));
			}
			*/
			// get cvsd in wptr
			cvsdInDataPtr = HREADW(CORE_CVSDIN_SADDR);
			if(Bt_CheckCVSDInUseMRam())
			{
				cvsdInDataPtr = reg_map_m0(cvsdInDataPtr);
			}
			else
			{
				cvsdInDataPtr = reg_map(cvsdInDataPtr);
			}

			for(i = 0; i < scoRxPacketLength; i ++)
			{
				if(isRxPacket)
				{
					*((uint8_t *)(cvsdInDataPtr + i)) = realRxDataPtr->bufferData[i];
				}
				else
				{
					*((uint8_t *)(cvsdInDataPtr + i)) = 0x55;
				}
			}
	#ifdef 	FUNCTION_CM0_CVSD_DAC_DEBUG	
			if(HREAD(0x4ff0) == 1)
			{
				USART_SendDataFromBuff(UARTA,(uint8_t *)cvsdInDataPtr,scoRxPacketLength);	
			}
				
	#endif		
			
			uint16_t seq = Bt_GetSeqByClkInSco(realRxDataPtr->rxClk);
			gCurrentScoSeqNum = seq;
			// update read ptr
			prxDataFifo->rptr += 1;
			//if(prxDataFifo->rptr == M0_SCO_RX_BUFFER_SIZE)
			if(prxDataFifo->rptr == OAL_M0_SCO_BUFFER_SIZE)
			{
				prxDataFifo->rptr = 0;
			}
		}
		
		// Check tx
		{
			gBtScoAllowAdcWork = 0;
			// Check adc buffer already for cvsd work
			if(Bt_SCOGetAdcCurrentBufferSize() >= (scoTxPacketLength*2))
			{
				gBtScoAllowAdcWork = 1;
			}
			
			//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe050, Bt_SCOGetAdcCurrentBufferSize());
			//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe051, gBtScoAllowAdcWork);
		}
		// start cvsd encode and decode work.
		Bt_CVSDWorkEnable();	

}
#ifdef FUNCTION_CM0_BEFORECVSD_ADC_DEBUG
void Bt_CVSDbeforedataprint(void)
{
	uint16_t pcmInSrcPtr;
	uint16_t scoTxPacketLength = gSCOTxPacketLen;
	pcmInSrcPtr = HREADW(CORE_PCMIN_SADDR);	
	xmemcpy(gcvsdtestbuf,(void *)(reg_map(pcmInSrcPtr)),(scoTxPacketLength*2));
	USART_SendDataFromBuff(UARTA,(uint8_t *)gcvsdtestbuf,(scoTxPacketLength*2));

}
#endif

uint16_t Bt_GetSeqByClkInSco(BT_CLK_INFO clke)
{
	// Cal the start sync clock.
	uint32_t lastClke = clke.clke_bt;
	uint16_t scoParmD = HREADW(mem_sco_dsniff);
	uint16_t scoParmT = HREADW(mem_sco_tsniff);

	lastClke = (lastClke & (~0x01));

	uint32_t scoAnchorClke = (lastClke/scoParmT) * scoParmT + scoParmD;

	scoAnchorClke -= HREADL(mem_hfp_start_dac_clk);

	return scoAnchorClke/scoParmT;
}

void Bt_CvsdWorkEnd(void)
{
	uint16_t pcmInSrcPtr;
	uint32_t cvsdOutDataPtr = 0;
	//uint16_t scoRxPacketLength = gSCORxPacketLen;
	uint16_t scoTxPacketLength = gSCOTxPacketLen;
		cvsdcnt++;
	
		// stop cvsd encode and decode work.
		Bt_CVSDWorkDisable();
		// tx work
		{
			if(gBtScoAllowAdcWork)
			{
				// update pcm in ptr
				cvsdOutDataPtr = HREADW(CORE_CVSDOUT_PTR);
				//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe020, cvsdOutDataPtr);
				//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe021, HREADW(CORE_CVSDOUT_SADDR));
				// if reach bottom, back to last
				if(cvsdOutDataPtr == (HREADW(CORE_CVSDOUT_SADDR)))
				{
					cvsdOutDataPtr = (HREADW(CORE_CVSDOUT_SADDR)) + (HREADW(CORE_CVSD_BUF_LEN)) + 1;
				}
				// get cvsd start address
				cvsdOutDataPtr -= scoTxPacketLength;

				if(Bt_CheckCVSDOutUseMRam())
				{
					cvsdOutDataPtr = reg_map_m0(cvsdOutDataPtr);
				}
				else
				{
					cvsdOutDataPtr = reg_map(cvsdOutDataPtr);
				}
				//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe023, cvsdOutDataPtr);

				Bt_SCOPushToLocalTxBuffer(gBtScoAllowAdcWork, (uint8_t *)cvsdOutDataPtr);

	
				// update pcm in addr
				// Check adc buffer already for cvsd work
				#ifdef FUNCTION_CM0_BEFORECVSD_ADC_DEBUG
					Bt_CVSDbeforedataprint();
				#endif
				pcmInSrcPtr = HREADW(CORE_PCMIN_SADDR);
				//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe024, pcmInSrcPtr);

				pcmInSrcPtr = pcmInSrcPtr + (scoTxPacketLength * 2);
				if(pcmInSrcPtr >= ((HREADW(CORE_ADCD_EADDR)) + 1))
				{
					pcmInSrcPtr = HREADW(CORE_ADCD_SADDR);
				}

				HWRITEW(CORE_PCMIN_SADDR, pcmInSrcPtr);
			}
			else
			{
				Bt_SCOPushToLocalTxBuffer(gBtScoAllowAdcWork, NULL);
			}
		}

		{
			// Check Buffer empty
			if(Bt_CvsdStartPacketCache())
			{
				return ;
			}
			uint16_t dacPtr = 0;
#ifdef FUNCTION_CVSD_OVER_SOFT_8KTO48K
			{
				extern uint16_t gDacBufOffset;
				uint32_t lastDac = gDacBufOffset;
				uint16_t dacStartAddr;
				//lastDac = lastDac - dacStartAddr;
				uint32_t newDac = HREADW(CORE_PCMOUT_PTR);
				dacStartAddr = HREADW(CORE_PCMOUT_SADDR);
				newDac = newDac - dacStartAddr;
				gDacBufOffset = newDac;
				uint16_t dacBufSize = (HREADW(CORE_CVSD_BUF_LEN) + 1) * 2;
				if(newDac == 0)
				{
					newDac += dacBufSize;
				}

				uint16_t tmpOffset = gSCOTxPacketLen*2;
				uint16_t tmpStartDac = lastDac;
				uint16_t tmpEndDac = lastDac + tmpOffset;
				uint16_t tmpBuf[60];

				while(tmpEndDac <= newDac)
				{
					uint16_t i,j = 0;
					for(i = tmpStartDac; i < tmpEndDac; i = i + 2)
					{
						uint16_t tmpIndex = (dacStartAddr + i);
						uint16_t tmpValue = HREADW(tmpIndex);
						tmpBuf[j] = tmpValue;
						j++;
					}

					SYS_ChangeToFastDPLL();
					for(j = 0; j < (tmpOffset>>1); j++)
					{
						Filter8KTo48KWork(tmpBuf[j]);
					}
					SYS_ChangeToNormalDPLL();

					tmpStartDac += tmpOffset;
					tmpEndDac += tmpOffset;
				}
				dacPtr = (Filter8KTo48KGetWptr() * 2) + HREADW(CORE_DAC_SADDR);
			}
#else
			dacPtr = HREADW(CORE_48K_PCMOUT_PTR);
#endif
			if(Audio_CheckDacEmpty())
			{
				DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D011, 0);
				if(cvsdcnt !=scocnt)
				{
					DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D10a, cvsdcnt);
					DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D10b, scocnt);
				}
				// for nr/aec, need restart work, becouse of tws function.
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
				if(!HREAD(mem_enable_hfp_start_sync_new))
				{
					// Reinit ADC work
					Bt_NRAECAdcReinit(0);//TODO
				}
#endif
			}
			// update dac wptr
			HWRITEW(CORE_DAC_WPTR, dacPtr);

			HWRITEW(mem_a2dp_seq_num, gCurrentScoSeqNum);
			HWRITE(mem_sco_media_release, 1);
			
			Audio_DacVolAdjust();
		}

	
}


uint8_t Bt_CvsdStartPacketCache(void)
{
	if(gCvsdStartFlag == M0_CVSD_START_FLAG_ENABLE)
	{
		// Cache 3 packet
		if((HREADW(CORE_PCMOUT_PTR) -HREADW(CORE_PCMOUT_SADDR)) < 3*120)
		{
			return 1;	
		}
		else
		{
			gCvsdStartFlag=M0_CVSD_START_FLAG_DISABLE;
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

void Bt_SCOPushToLocalTxBuffer(uint8_t isTxPacket, volatile uint8_t* dataPtr)
{
	//volatile BT_SCO_TX_DATA_FIFO* ptxDataFifo = &gM0SCOFifoProcess.txDataFifo;
	volatile OAL_BT_SCO_TX_DATA_FIFO* ptxDataFifo = &gM0SCOFifoProcess.txDataFifo;
	uint16_t scoTxPacketLength = gSCOTxPacketLen;
	uint8_t i;
	if(isTxPacket)
	{
		memcpy((uint8_t *)ptxDataFifo->scoTxData[ptxDataFifo->wptr].bufferData
			, (uint8_t *)dataPtr, 60);
	}
	else
	{
		
		for(i = 0; i < scoTxPacketLength; i ++)
		{
			(ptxDataFifo->scoTxData[ptxDataFifo->wptr].bufferData[i]) = 0x55;
		}
	}
	#ifdef FUNCTION_CM0_AFTERECVSD_ADC_DEBUG
		USART_SendDataFromBuff(UARTA,(uint8_t *)dataPtr,60);
	#endif
	ptxDataFifo->wptr += 1;
	//if(ptxDataFifo->wptr == M0_SCO_TX_BUFFER_SIZE)
	if(ptxDataFifo->wptr == OAL_M0_SCO_BUFFER_SIZE)
	{
		ptxDataFifo->wptr = 0;
	}
}

void Bt_mSBCUpdateTxSeq(void)
{
	uint8_t seq = HREAD(mem_sequence_number_tx);
	switch(seq)
	{
		case MSBC_SEQUENCE_NUMBER_0:
			seq = MSBC_SEQUENCE_NUMBER_1;
			break;
		case MSBC_SEQUENCE_NUMBER_1:
			seq = MSBC_SEQUENCE_NUMBER_2;
			break;
		case MSBC_SEQUENCE_NUMBER_2:
			seq = MSBC_SEQUENCE_NUMBER_3;
			break;
		case MSBC_SEQUENCE_NUMBER_3:
			seq = MSBC_SEQUENCE_NUMBER_0;
			break;
		default:
			seq = MSBC_SEQUENCE_NUMBER_0;
			break;
	}

	HWRITE(mem_sequence_number_tx, seq);
}

const char mutevoice[]={0x01,0x38,0xAD,0x00,0x00,0xC5,0x00,0x00,0x00,0x00,
0x77,0x6D,0xB6,0xDD,0xDB,0x6D,0xB7,0x76,0xDB,0x6D,0xDD,0xB6,0xDB,0x77,
0x6D,0xB6,0xDD,0xDB,0x6D,0xB7,0x76,0xDB,0x6D,0xDD,0xB6,0xDB,0x77,0x6D,
0xB6,0xDD,0xDB,0x6D,0xB7,0x76,0xDB,0x6D,0xDD,0xB6,0xDB,0x77,0x6D,0xB6,
0xDD,0xDB,0x6D,0xB7,0x76,0xDB,0x6C,0x00};
int32_t gHfpMuteDacSum;
uint16_t gHfpMuteDacWorkCnt;
uint16_t gHfpForceMuteCnt;
uint16_t mSbcDacOutIndex;
#define DAC_PLAY_CHECK_CNT (128)
#define DAC_PLAY_CHECK_PWR (7)
#define DAC_PLAY_CHECK_THRESHOLD (400)
BOOL Filter8KTo22p05KCheckNeedMute(void)
{
	return gHfpForceMuteCnt != 0;
}
void Bt_mSBCDataPolling(void)
{
	volatile OAL_BT_SCO_RX_DATA_FIFO* prxDataFifo = &gM0SCOFifoProcess.rxDataFifo;
	uint16_t i;
	if(Bt_CheckAllowmSBC())
	{
		return ;
	}
	if(HFPstartflag==0)
	{
		return;
	}
#ifdef TEST_FUNCTION_USE_SPECIAL_VOICE_ENABLE
	return;
#endif

	BOOL isRxPacket = TRUE;
	volatile OAL_BT_SCO_RX_DATA * realRxDataPtr;
	volatile BT_SCO_TX_DATA realTxData;
	//uint16_t scoTxPacketLength = gSCOTxPacketLen;
	
	// Check rptr and wptr, to judge receive valid packet
	if(prxDataFifo->rptr== prxDataFifo->wptr)
	{
		return;
	}
	// Check dac buffer size enough
	// Check rx
//	GPIO_SetOut((GPIO_28|GPIO_ACTIVE_BIT),1);
	{
		realRxDataPtr = &(prxDataFifo->scoRxData[prxDataFifo->rptr]);
		// check packet valid or not
		if((realRxDataPtr->bufferValid & 0x01) == 0)
		{
			isRxPacket = FALSE;
		}
		//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D010, isRxPacket);
		if(!isRxPacket)
		{
			for(uint8_t te=0;te<60;te++)
			{
				realRxDataPtr->bufferData[te]=mutevoice[te];
			}
		}
		// update ptr
		//if(FALSE)
		{
			uint16_t sync = realRxDataPtr->bufferData[0] |(realRxDataPtr->bufferData[1] << 8);
			uint8_t decodeOutBuffer[MSBC_PCM_PACKET_LEN];
			uint16_t dacWptr = HREADW(CORE_DAC_WPTR);
			uint16_t seq = Bt_GetSeqByClkInSco(realRxDataPtr->rxClk);
			gCurrentScoSeqNum = seq;
			//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd500, dacWptr);
			mSBC_DecodeStart((uint16_t)(uint32_t)(realRxDataPtr->bufferData + 2), TRUE, (uint16_t)(uint32_t)decodeOutBuffer, TRUE);	
			// data update to 48k for DAC play
			//uint16_t dacWptr = HREADW(CORE_DAC_WPTR);
			uint16_t dacStartAddr = HREADW(CORE_DAC_SADDR);
			uint16_t dacEndAddr = dacStartAddr + HREADW(CORE_DAC_LEN) + 1;
	#ifdef MSBC_ONLY_PASS
			uint32_t preIndex=dacWptr-DAC_PLAY_CHECK_CNT;
			if(preIndex<dacStartAddr)
			{
				preIndex+=MSBC_NEW_DAC_48K_MAX_BUFFER_LEN;
			}
			for(i = 0; i < (MSBC_PCM_PACKET_LEN/2); i ++)
			{
				gHfpMuteDacSum += abs(*(int16_t*)(decodeOutBuffer + i * 2));
				gHfpMuteDacWorkCnt ++;
			
				int16_t preValue =HREADW(reg_map(preIndex));// gpFilter8kTo22p05KOut[preIndex]
				//int16_t preValue =*(uint16_t*)(reg_map_m0(preIndex));
				preIndex+=2;
				if(preIndex>=dacEndAddr)
					preIndex=dacStartAddr;
	
				gHfpMuteDacSum -= abs(preValue);
				if(gHfpMuteDacWorkCnt > DAC_PLAY_CHECK_CNT)
				{
					int32_t val = (gHfpMuteDacSum >> DAC_PLAY_CHECK_PWR);
					if(val > DAC_PLAY_CHECK_THRESHOLD)
					{
						gHfpForceMuteCnt = 60;
					}
					if(gHfpForceMuteCnt != 0)
					{
						gHfpForceMuteCnt--;
					}
		
					//DEBUG_LOG_STRING("farrow mute: %d, Sum: %d, Ave: %d\r\n" 
					//	, gHfpForceMuteCnt, gHfpMuteDacSum, val);
			
					gHfpMuteDacWorkCnt = 0;
					gHfpMuteDacSum = 0;
				}
			}
			#endif
			//0.4ms
			for(i = 0; i < (MSBC_PCM_PACKET_LEN); i ++)
			{
				HWRITE(dacWptr, decodeOutBuffer[i]);
				//(HREADDEF(dacWptr) = (byte)(decodeOutBuffer[i]));
			//	*(uint8_t*)(reg_map_m0(dacWptr))=decodeOutBuffer[i];
				dacWptr = dacWptr + 1;
				if(dacWptr >= dacEndAddr)
				{
					dacWptr = dacStartAddr;
				}
			}
		////
			if(Audio_CheckDacEmpty())
			{
				DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", LOG_POINT_D011, 0);
#ifdef FUNCTION_CONTROL_AECNR_ENABLE
				// Reinit ADC work
				Bt_NRAECAdcReinit(1);
#endif
			}

			HWRITEW(CORE_DAC_WPTR, dacWptr);
			HWRITEW(mem_a2dp_seq_num, gCurrentScoSeqNum);
			HWRITE(mem_sco_media_release, 1);
			DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd500, dacWptr);
		}
		
		// update read ptr
		prxDataFifo->rptr += 1;
		if(prxDataFifo->rptr == OAL_M0_SCO_BUFFER_SIZE)
		{
			prxDataFifo->rptr = 0;
		}
		Audio_DacVolAdjust();
		//HWRITE(CORE_DAC_LVOL, 0x40);
		//HWRITE(CORE_DAC_RVOL, 0x40);
	}
	
	#if 1
	// Check tx
	{
		// Check adc buffer already for cvsd work
		if(Bt_SCOGetAdcCurrentBufferSize() >= (MSBC_PCM_PACKET_LEN))
		{
			uint8_t seq = HREAD(mem_sequence_number_tx);
			Bt_mSBCUpdateTxSeq();
			uint16_t sync = MSBC_H2_SYNCWORD |(seq << 8);
			realTxData.bufferData[0] = sync & 0xff;
			realTxData.bufferData[1] = (sync >> 8) & 0xff;
			
			uint16_t pcmInSrcPtr= HREADW(mem_msbc_pcm_in_rptr);
			uint8_t *encodeOutBuffer16K = (uint8_t *)(reg_map_m0(pcmInSrcPtr));	

#ifdef MSBC_ONLY_PASS
			int16_t *adcBuf = (int16_t *)reg_map_m0(HREADW(mem_msbc_pcm_in_rptr));
			uint8_t gain = 6;
			if(Filter8KTo22p05KCheckNeedMute())
			{
				gain = 0;
			}
			for(int i = 0; i < (MSBC_PCM_PACKET_LEN/2); i ++)
			{
				*(int16_t *)(adcBuf + i) = (int16_t)(((int32_t)(*(int16_t *)(adcBuf + i)) * gain) >> 3);
			}
#endif
						
//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", 0xd501, encodeOutBuffer16K);
			mSBC_EncodeStart((uint16_t)(uint32_t)(realTxData.bufferData + 2), TRUE
				,(uint16_t)(uint32_t)encodeOutBuffer16K, TRUE);
			// Update pcm in ptr.
			pcmInSrcPtr = pcmInSrcPtr + MSBC_PCM_PACKET_LEN;//0xf0 240
			if(pcmInSrcPtr >= ((HREADW(CORE_ADCD_EADDR)) + 1))
			{
				pcmInSrcPtr = HREADW(CORE_ADCD_SADDR);
			}

			HWRITEW(mem_msbc_pcm_in_rptr, pcmInSrcPtr);
			//DEBUG_LOG_STRING("Bt_mSBCDataPolling, tx rptr update: 0x%02X\r\n", pcmInSrcPtr);
			Bt_SCOPushToLocalTxBuffer(TRUE, (uint8_t *)realTxData.bufferData);
		}
		else
		{
//			GPIO_SetOut((GPIO_31|GPIO_ACTIVE_BIT),1);
		}
		
		//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe050, Bt_SCOGetAdcCurrentBufferSize());
		//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"xxx: 0x%04X", 0xe051, gBtScoAllowAdcWork);
	}
	#endif
//	GPIO_SetOut((GPIO_31|GPIO_ACTIVE_BIT),0);
//	GPIO_SetOut((GPIO_28|GPIO_ACTIVE_BIT),0);
}
uint8_t Bt_CheckAllowmSBC(void)
{
	
	if(HREAD(mem_param_sco_use_cm0) == 0)
	{
		return 1;
	}
	if(!Bt_CheckHfpStart())
	{
		return 1;
	}
	if(Bt_CheckVpInprocess()
		&& !Bt_CheckVpInHardwareWork())
	{
		return 1;
	}
	if(Bt_CheckHfpLocalInprocess())
	{
		return 1;
	}
	//if(HREAD(mem_enable_hfp_start_sync) != 0)
	if(HREAD(mem_enable_hfp_start_sync_new) != 0)
	{
		return 1;
	}
	/*
	if(!Bt_CheckCVSDWorkStart())
	{
		return 1;
	}
	*/

	return 0;

}

void Bt_SCODataPolling(void)
{
	switch(HREAD(mem_codec_type))
	{
		case CODEC_TYPE_CVSD:
			Bt_CVSDDataPolling();
			break;
		case CODEC_TYPE_mSBC:
			Bt_mSBCDataPolling();
			break;
	}
}
