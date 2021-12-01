#include "yc11xx_bt_interface.h"
#include "yc11xx_dev_oal.h"

uint8_t *gpHFPCallState =(uint8_t *) reg_map( mem_hf_call_state);


void Bt_SndCmdBLE( uint8_t *data,uint8_t len)
{
	IPC_TxCommon(IPC_TYPE_BLE,data,len);
}

void Bt_SndCmdStopReconTws(void)
{
	IPC_TxControlCmd(BT_CMD_STOP_RECON_TWS);
}
void Bt_SndCmdChargePowerOff(void)
{
	IPC_TxControlCmd(BT_CMD_CHARGE_POWER_OFF);
}
void Bt_SndCmdChargeStopPowerOff(void)
{
	IPC_TxControlCmd(BT_CMD_CHARGE_STOP_POWEROFF);
}
void Bt_SndCmdChargeChargeInscEnb(void)
{
	IPC_TxControlCmd(BT_CMD_CHARGE_INSCENB);
}
void Bt_SndCmdChargeChargeInscEnbClear(void)
{
	IPC_TxControlCmd(BT_CMD_CHARGE_INSCENB_CLEAR);
}
void Bt_SndCmdOtaReset(void)
{
	IPC_TxControlCmd(BT_CMD_OTA_RESET);
}
BOOL Bt_CheckVpInProcess(void)
{
	return (HREAD(mem_vp_enable) != 0); //VP_STATE_DISENABLE
}
void Bt_SndCmdTwsUser1(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER1);
}

void Bt_SndCmdTwsUser2(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER2);
}

void Bt_SndCmdTwsUser3(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER3);
}

void Bt_SndCmdTwsUser4(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER4);
}
void Bt_SndCmdTwsUser5(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER5);
}
void Bt_SndCmdTwsUser6(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER6);
}
void Bt_SndCmdTwsUser7(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_USER7);
}
void Bt_SndCmdTwsFixVoicePlay(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_FIX_VOICE_PLAY);
}


void Bt_SndCmdPlayPause(MUSIC_CMD cmd)
{
	if ((!Bt_CheckAVRCPConnected() )
		||(HREAD(mem_hf_call_state) != HFP_NONO))
	{
		return;
	}
       switch (cmd)
       {
	case MUSIC_PLAY:
	    IPC_TxControlCmd(BT_CMD_PLAY);
	    break;
	case MUSIC_PAUSE:
	    IPC_TxControlCmd(BT_CMD_PAUSE);
	    break;
	case MUSIC_SWITCH_PLAY_PAUSE:
	    IPC_TxControlCmd(BT_CMD_PLAY_PAUSE);
	    break;
       }
	
}
void Bt_SndCmdBackward(void)
{
	IPC_TxControlCmd(BT_CMD_BACKWARD);
}
void Bt_SndCmdForward(void)
{
	IPC_TxControlCmd(BT_CMD_FORWARD);
}
void Bt_SndCmdVolUp(void)
{
	IPC_TxControlCmd(BT_CMD_VOL_UP);
}
void Bt_SndCmdVolDown(void)
{
	IPC_TxControlCmd(BT_CMD_VOL_DOWN);
}
void Bt_SndCmdAcceptCall(void)
{
	IPC_TxControlCmd(BT_CMD_ACCEPT_CALL);
}
void Bt_SndCmdHangup(void)
{
	IPC_TxControlCmd(BT_CMD_HANG_UP);
}
void Bt_SndCmdRedial(void)
{
	IPC_TxControlCmd(BT_CMD_REDIAL);
}
void Bt_SndCmdThreeWayReleaseAll(void)
{
	IPC_TxControlCmd(BT_CMD_ThreeWay_ReleaseAll);
}
void Bt_SndCmdThreeWayReleaseNAcp(void)
{
	IPC_TxControlCmd(BT_CMD_ThreeWay_ReleaseNAcp);
}
void Bt_SndCmdThreeWay_HoldNAcp(void)
{
	IPC_TxControlCmd(BT_CMD_ThreeWay_HoldNAcp);
}
void Bt_SndCmdRecognition(void)
{
	IPC_TxControlCmd(BT_CMD_RECOGNITION);
}
void Bt_SndCmdRecognitionStop(void)
{
	IPC_TxControlCmd(BT_CMD_RECOGNITION_STOP);
}
void Bt_SndCmdTwsSyncPwroff(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_POWR_OFF);
}
void Bt_SndCmdTwsSlaveInStorage(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_SLAVE_IN_STORAGE);
}
void Bt_SndCmdTwsMasterInStorage(void)
{
	// set the reason tell remote.
	HWRITE(mem_tws_buffer_master_switch_reason, 1);
	IPC_TxControlCmd(TWS_SYNC_CMD_MASTER_SWITCH);
}
void Bt_TwsSyncPowerOff(void)
{
	Bt_SndCmdTwsSyncPwroff();
}
void Bt_TwsInStorage(void)
{
	if(Bt_CheckTwsMasterConnected())
	{
		Bt_SndCmdTwsMasterInStorage();
	}
	else
	{
		Bt_SndCmdTwsSlaveInStorage();
	}
}

void Bt_SndCmdReconnect()
{
	HWRITE(mem_connection_options, (HREAD(mem_app_connection_options)));
	HWRITE(mem_link_key_exists, 1);

	IPC_TxControlCmd(BT_CMD_RECONNECT);
}
void Bt_SndCmdStopReconnect()
{
	IPC_TxControlCmd(BT_CMD_BB_RECONN_CANCEL);
}

void Bt_SndCmdHFPStartSync(void)
{
	if (!Bt_CheckHfpStart())
	{
		return;
	}
	if(HREAD(mem_vp_enable) != VP_STATE_DISENABLE)
	{
		return;
	}
	if(Bt_CheckTwsMasterConnected())
	{
		HWRITE(mem_hfp_start_dac_work, 0);

		// Cal the start sync clock.
		uint32_t lastClke = HREADL(mem_tws_sync_clk);
		uint16_t scoParmD = HREADW(mem_sco_dsniff);
		uint16_t scoParmT = HREADW(mem_sco_tsniff);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B100, lastClke >> 16);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B101, lastClke >> 0);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B102, scoParmD >> 0);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B103, scoParmT >> 0);

		lastClke = (lastClke & (~0x01));

		uint32_t scoAnchorClke = (lastClke/scoParmT) * scoParmT + scoParmD;
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B104, scoAnchorClke >> 16);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B105, scoAnchorClke >> 0);
		
		uint32_t scoSyncAnchorClke = scoAnchorClke + scoParmT*40 + (scoParmT + 1);//here must add one, avoid master slave mismatch
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B106, scoSyncAnchorClke >> 16);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"tws_user_hfp_start_sync_action: 0x%04X", LOG_POINT_B107, scoSyncAnchorClke >> 0);

		//Save start work time
		HWRITEL(mem_hfp_start_sync_clk, scoSyncAnchorClke);
		
		//Save start play time
		// Cache four packet
		scoSyncAnchorClke += scoParmT*TWS_SCO_CACHE_BUFFER_CNT;
		HWRITEL(mem_hfp_start_dac_clk, scoSyncAnchorClke);

		// Must disable hfp start sync
		HWRITE(mem_enable_hfp_start_sync_new, 1);
	
		IPC_TxControlCmd(TWS_SYNC_CMD_HFP_START_SYNC);
	}
}

BOOL Bt_CheckAllowTishiWork(void)
{
	if((HREAD(CORE_DAC_CTRL) & BIT_0) != 0)
	{
		if(HREAD(mem_dac_clk) == 48)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void Bt_SndCmdWaitMaster(void)
{
	IPC_TxControlCmd(BT_CMD_START_WAIT_MASTER);
}

void Bt_SndCmdWaitSlave(void)
{
       if (HREAD(mem_test_mode) == 0)
	    IPC_TxControlCmd(BT_CMD_START_WAIT_SLAVE);
}

void Bt_SndCmdExitTWS(void)
{
	// For function good, teminate close.
	//return;
	IPC_TxControlCmd(BT_CMD_TWS_NONE);
}
void Bt_SndCmdTwsNoConnectTransCmdPowerOff(void)
{
	IPC_TxControlCmd(TWS_NO_CONNECT_TRANS_CMD_POWER_OFF);
}
void Bt_SndCmdTwsNoConnectTransCmdInStore(void)
{
	IPC_TxControlCmd(TWS_NO_CONNECT_TRANS_CMD_IN_STORE);
}
void Bt_SndCmdTwsNoConnectTransCmdChangeLanguageEn(void)
{
	IPC_TxControlCmd(TWS_NO_CONNECT_TRANS_CMD_CHANGE_LANGUAGE_EN);
}
void Bt_SndCmdTwsNoConnectTransCmdChangeLanguageCh(void)
{
	IPC_TxControlCmd(TWS_NO_CONNECT_TRANS_CMD_CHANGE_LANGUAGE_CH);
}
void Bt_SndCmdTwsNoConnectTransCmdSwitchOALOFF(void)
{
	IPC_TxControlCmd(TWS_NO_CONNECT_TRANS_CMD_SWITCH_OAL_OFF);
}
void Bt_SndCmdTwsNoConnectTransCmdSwitchOALON(void)
{
	IPC_TxControlCmd(TWS_NO_CONNECT_TRANS_CMD_SWITCH_OAL_ON);
}
BOOL Bt_CheckAVRCPConnected(void)
{
	uint16_t temp;
	uint8_t temp1;
	temp =  HREADW(mem_avr_l2capch_ptr);
	if (temp == 0)
		return FALSE;
	temp+=1;
	temp1 = HREAD(temp);
	if (temp1 != 0x3f)
		return FALSE;
	return TRUE;
}

BOOL Bt_CheckA2DPConnected(void)
{
	uint16_t temp;
	uint8_t temp1;
	temp =  HREADW(mem_avdtpmedia_l2capch_ptr);
	if (temp == 0)
		return FALSE;
	temp+=1;
	temp1 = HREAD(temp);
	if (temp1 != 0x3f)
		return FALSE;
	return TRUE;
}


BOOL Bt_CheckA2DPStart(void)
{
	return ((HREAD(mem_a2dp_state) == A2DP_STATE_START));
}
BOOL Bt_CheckHfpStart(void)
{
	return (HREAD(mem_sco_transaction_enable) == 1);
}
BOOL Bt_CheckHFPIsIncomming(void)
{
	return (*gpHFPCallState == HFP_INCOMMING);
}
BOOL Bt_CheckTwsMasterConnected(void)
{
	return (HREAD(mem_tws_state) == TWS_STATE_MASTER_CONNECTED);
}
BOOL Bt_CheckTwsInMasterWorkProcess(void)
{
	if ((HREAD(mem_tws_state) == TWS_STATE_WAIT_SLAVE)
		|| (HREAD(mem_tws_state) == TWS_STATE_MASTER_LISTEN)
		|| (HREAD(mem_tws_state) == TWS_STATE_MASTER_CONNECTED))
	{
		return TRUE;
	}

	return FALSE;
}
BOOL Bt_CheckTwsSlaveConnected(void)
{
	return (HREAD(mem_tws_state) == TWS_STATE_SLAVE_CONNECTED);
}
BOOL Bt_CheckTwsInSlaveWorkProcess(void)
{
	if ((HREAD(mem_tws_state) == TWS_STATE_WAIT_MASTER)
		|| (HREAD(mem_tws_state) == TWS_STATE_SLAVE_LISTEN)
		|| (HREAD(mem_tws_state) == TWS_STATE_SLAVE_CONNECTED)
		|| (HREAD(mem_tws_state) == TWS_STATE_SLAVE_SWITCH_LISTEN))
	{
		return TRUE;
	}

	return FALSE;
}
BOOL Bt_CheckTwsConnected(void)
{
	return (Bt_CheckTwsMasterConnected()) | (Bt_CheckTwsSlaveConnected());
}
BOOL Bt_CheckTwsHSPRoleLeft(void)
{
	return (HSP_ROLE_L == HREAD(mem_hsp_role));
}
BOOL Bt_CheckHfpLocalInprocess(void)
{
	return (HREAD(mem_local_sco_in_process_flag) != 0);
}
BOOL Bt_CheckVpInprocess(void)
{
	return (HREAD(mem_vp_enable) != 0);
}
BOOL Bt_CheckVpInHardwareWork(void)
{
	return ((HREAD(mem_vp_enable) & VP_MODE_BIT) != 0);
}
BOOL Bt_CheckMediaPlayInprocess(void)
{
	return (HREAD(mem_audio_allow) == AVD_AUDIO_ALLOW);
}
BOOL Bt_CheckInFastSyncMode(void)
{
	return HREAD(mem_media_in_fast_mode);
}



void Bt_CVSDWorkEnable(void)
{
	HWOR(CORE_CVSD_CTRL,0x01);
}

void Bt_CVSDWorkDisable(void)
{
	HWCOR(CORE_CVSD_CTRL,0x01);
}
BOOL Bt_CheckTwsRoleMaster(void)
{
	// Becouse we need think tws role not set.
	return ((TWS_SET_SLAVE != HREAD(mem_tws_role)));
}
BOOL Bt_CheckTwsRoleSlave(void)
{
	return ((TWS_SET_SLAVE == HREAD(mem_tws_role)));
}
BOOL Bt_CheckCVSDEnable(void)
{
	return ((HREAD(CORE_CVSD_CTRL) & 0x01) != 0); //CVSD_GROUP_ENABLE
}
BOOL Bt_CheckCVSDWorkStart(void)
{
	return ((HREAD(CORE_CVSD_CTRL) & 0x80) != 0); //CVSD_DMA_ENABLE
}


BOOL Bt_CheckCVSDWorkInprocess(void)
{
	return ((HREAD(CORE_PERF_STATUS) & 0x04) == 0);
}

BOOL Bt_CheckCVSDInUseMRam(void)
{
	return ((HREAD(CORE_CVSD_CTRL1) & 0x04) != 0); //DECODE_CVSD_IN_MRAM_SEL
}
BOOL Bt_CheckCVSDOutUseMRam(void)
{
	return ((HREAD(CORE_CVSD_CTRL1) & 0x08) != 0); //ENCODE_CVSD_OUT_MRAM_SEL
}

BOOL Bt_CheckSCOInHV3EV3Transaction(void)
{
	return ((HREAD(mem_sco_rx_packet_len)) == 30);
}
BOOL Bt_CheckSCOInHV3Transaction(void)
{
	return ((HREAD(mem_sco_rx_packet_type)) == 0x02);
}

uint16_t Bt_GetAdcCurrentBufferSize(void)
{
	uint16_t adcWptr, pcmInSrcPtr;
	
	// Check adc buffer already for cvsd work
	adcWptr = HREADW(CORE_ADCD_ADDR);
	pcmInSrcPtr = HREADW(CORE_PCMIN_SADDR);
	if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	{
		pcmInSrcPtr = HREADW(mem_msbc_pcm_in_rptr);
	}

	if(adcWptr >= pcmInSrcPtr)
	{
		return (adcWptr - pcmInSrcPtr);
	}
	else
	{
		return (HREADW(mem_adc_buf_size)- (pcmInSrcPtr  - adcWptr));
	}
}

void Bt_GetMac(uint8_t *adr)
{
	for(int i = 0;i < 6; i++)
	{
		adr[i] = HREAD((mem_lap+5-i));
	}
}

void Bt_SndCmdDisconnect(void)
{
	IPC_TxControlCmd(BT_CMD_DISCONNECT);
}

void Bt_SndCmdStartDiscovery(void)
{
	IPC_TxControlCmd(BT_CMD_START_DISCOVERY);
}
void Bt_SndCmdStopDiscovery(void)
{
	IPC_TxControlCmd(BT_CMD_STOP_DISCOVERY);
}

void Bt_SndCmdStartReconTws(void)
{
	IPC_TxControlCmd(BT_CMD_START_RECON_TWS);
}
void Bt_SndCmdPwroff(void)
{
	IPC_TxControlCmd(BT_CMD_ENTER_HIBERNATE);
}


void Bt_SndCmdAudioProcessInfo(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_AUDIO_PROCESS_INFO);
}
void Bt_SndCmdExitFastSyncMode(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_EXIT_FAST_SYNC);
}
void Bt_SndCmdEnterFastSyncMode(void)
{
	IPC_TxControlCmd(TWS_SYNC_CMD_ENTER_FAST_SYNC);
}

void Bt_SndCmdRecoverAudio(void)
{
	IPC_TxControlCmd(BT_CMD_RECOVER_AUDIO);
}

void Bt_SndCmdLeUpdateConn(void)
{
	IPC_TxControlCmd(BT_CMD_LE_UPDATE_CONN);
}

void Bt_LpmUpdateChargeSetting(uint32_t tmp)
{
	HWRITEL(mem_lpm_write_temp_charge_control, tmp);
	HWRITEL(CORE_LPM_REG, tmp);
	HWRITESINGLE(CORE_LPM_WR, LPMREG_SEL_CHARGE_CTRL);
	SYS_delay_ms(10);
}
void Bt_ChargeInitSettingInPowerOn(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownSw(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp |= (1 << CHARGE_CHGR_SHUTDOWN_SW);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownSwClear(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp &=~ (1 << CHARGE_CHGR_SHUTDOWN_SW);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownBias(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp |= (1 << CHARGE_CHGR_SHUTDOWN_BIAS);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownBiasClear(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp &=~ (1 << CHARGE_CHGR_SHUTDOWN_BIAS);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownCore(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp |= (1 << CHARGE_CHGR_SHUTDOWN_CORE);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownCoreClear(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp &=~ (1 << CHARGE_CHGR_SHUTDOWN_CORE);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeShutdownProcess(void)
{
	Bt_ChargeShutdownCore();
	SYS_delay_ms(1);
	Bt_ChargeShutdownSw();
	SYS_delay_ms(1);
	Bt_ChargeShutdownBias();
}
void Bt_ChargePowerUpProcess(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp |= (1 << CHARGE_CHGR_SHUTDOWN_CORE);
	tmp |= (1 << CHARGE_CHGR_SHUTDOWN_BIAS);
	tmp |= (1 << CHARGE_CHGR_SHUTDOWN_SW);
	HWRITEL(mem_lpm_write_temp_charge_control, tmp);
	Bt_ChargeShutdownBiasClear();
	SYS_delay_ms(1);
	Bt_ChargeShutdownSwClear();
	SYS_delay_ms(1);
	Bt_ChargeShutdownCoreClear();
	SYS_delay_ms(1);
}
void Bt_ChargeInscEnb(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp |= (1 << CHARGE_CHGR_INSC_ENB);
	Bt_LpmUpdateChargeSetting(tmp);
}
void Bt_ChargeInscEnbClear(void)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp &=~ (1 << CHARGE_CHGR_INSC_ENB);
	Bt_LpmUpdateChargeSetting(tmp);
}

void Bt_LpmUpdateChargeWakeupSettingHardware(uint16_t lowActive, uint16_t highActive)
{
	uint32_t tmp = lowActive | (highActive << 10);
	HWRITEL(CORE_LPM_REG, tmp);
	HWRITESINGLE(CORE_LPM_WR2, LPMREG_SEL_CHARGER_LOW_HIGH_WAKEUP_REG);
	SYS_delay_ms(10);
}

void Bt_ChargeSetRchc(uint8_t val)
{
	uint32_t tmp = HREADL(mem_lpm_write_temp_charge_control);
	tmp &= 0xffffff0f;
	tmp |= (val << 4);
	Bt_LpmUpdateChargeSetting(tmp);
}


void Bt_LpmUpdateChargeWakeupSetting(uint16_t lowActive, uint16_t highActive)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_A2DPCallBack: 0x%04X", LOG_POINT_C223, lowActive);
	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_A2DPCallBack: 0x%04X", LOG_POINT_C224, highActive);
	HWRITEW(mem_lpm_write_temp_charge_wakeup_low_active, lowActive);
	HWRITEW(mem_lpm_write_temp_charge_wakeup_high_active, highActive);
}


/**
*  Median Average Filtering
* @param pData:Data without filtering
* @param nSize:Data Size
* @return:filter value
*/
uint16_t MedianFilter(uint16_t* pData,int nSize)
{
    uint16_t max,min;
    int sum;
    if(nSize>2)
    {
        max = pData[0];
        min = max;
        sum = 0;
        for(int i=0;i<nSize;i++)
        {
            sum += pData[i];            
            if(pData[i]>max)
            {
                max = pData[i];   //get max
            }

            if(pData[i]<min)
            {
                min = pData[i];   //get min
            }
        }

        sum = sum-max-min;       //sub min and max
        return sum/(nSize-2); //judge ave value   
    }

    return 0;
}


void Bt_SndCmdShutdownQspi(void)
{
	IPC_TxControlCmd(BT_CMD_SHUTDOWN_QSPI);
}

void Bt_RemoveSCOLink(void)
{
	IPC_TxControlCmd(BT_CMD_RMV_ESCO_REQ);
}
void Bt_CreateSCOLink(void)
{
	IPC_TxControlCmd(BT_CMD_EST_ESCO_REQ);
}

void Bt_SwitchSCOLinkState(void)
{
	if(HREAD(mem_sco_transaction_enable))
	{
		Bt_RemoveSCOLink();
	}
	else
	{
		Bt_CreateSCOLink();
	}
}


void Bt_LpmChangeBBLdoLevel(uint8_t val)
{
	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_A2DPCallBack: 0x%04X", LOG_POINT_A504, val);
	//pmu_bbldo_vctrim<3:0>	R	7:4	bb ldo voltage control
	uint32_t tmp = HREADL(CORE_LPM_CTRL);
	uint32_t newTmp = (tmp & 0x0fffffff) | (val << 28);
	//avoid write twice
	if(newTmp == tmp)
	{
		return;
	}
	HWRITEL(CORE_LPM_REG, newTmp);
	HWRITESINGLE(CORE_LPM_WR, LPMREG_SEL_CTRL);
	// delay for clock ready
	SYS_delay_ms(50);
}

void Bt_SndCmdEnterSniff(void)
{
	IPC_TxControlCmd(BT_CMD_ENTER_SNIFF);
}

void Bt_SndCmdExitSniff(void)
{
	IPC_TxControlCmd(BT_CMD_EXIT_SNIFF);
}
void Bt_SndCmdTwsOalFunc(uint8_t wordnum)
{
#ifdef FUNCTION_OAL_MODE
	switch(wordnum)
	{
		case OAL_WAKE_WORD:
			break;
		case OAL_ANSWERTHEPHONE: //2//
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_CALL);
			break;
		case OAL_REJECTCALL:	//3//
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_NOCALL);
			break;
		case OAL_PLAY_MUSIC://4//
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_MUSIC_PLAY);
			break;
		case OAL_STOP_MUSIC://5
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_MUSIC_PAUSE);
			break;
		case OAL_LAST_MUSIC://6
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_LAST_MUSIC);
			break;
		case OAL_NEXT_MUSIC://7
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_NEXT_MUSIC);
			break;
		case OAL_ADD_VOL://8
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_ADD_VOL);
			break;
		case OAL_SUB_VOL://9
			IPC_TxControlCmd(TWS_SYNC_CMD_OAL_SUB_VOL);
			break;
	}
#endif
}

uint8_t Bt_CheckLinkInSniff(void)
{
	uint16_t uiStateMap = HREADW(mem_ui_state_map);

 	if(uiStateMap & (BIT_5)) //UI_STATE_BT_SNIFF
 	{
		return TRUE;
 	}

 	return FALSE;
}

void Bt_SndCmdEnterTestPlay()
{
	IPC_TxControlCmd(BT_CMD_TestPlay);
}

void Bt_SndCmdEnterTestPause()
{
	IPC_TxControlCmd(BT_CMD_TetstPause);
}

void Bt_SndCmdEnterTestSCO()
{
	IPC_TxControlCmd(BT_CMD_TestEnterSCO);
}

void Bt_SndCmdExitTestSCO()
{
	IPC_TxControlCmd(BT_CMD_TestExitSCO);
}


