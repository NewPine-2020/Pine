/*
 * Copyright 2016, yichip Semiconductor(shenzhen office)
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Yichip Semiconductor;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Yichip Semiconductor.
 */
  
/** 
  *@file dac.h
  *@brief DAC support for application.
  */
#ifndef CM0_COMPONENTS_YC11XX_AUDIO_H_
#define CM0_COMPONENTS_YC11XX_AUDIO_H_

#include <stdint.h>
#include "yc11xx.h"
#include "type.h"
#include "yc_timer.h"
#include "yc11xx_qspi.h"
#include "yc_debug.h"
#include "yc11xx_bt_interface.h"
#include "yc_8kto48k.h"
#include "yc_mediafarrow.h"

#include "yc_8kto16k.h"
#include "yc_16kto8k.h"
#include "yc_16kto48k.h"

#define DIN_SEL_L_LEFT_CHANNEL 0x00
#define DIN_SEL_L_RIGHT_CHANNEL 0x01
#define DIN_SEL_L_LEFT_RIGHT_MIX_CHANNEL 0x02
#define DIN_SEL_L_FIX_ZERO 0x03
#define DIN_SEL_L_CLOSE 0xFF

#define DIN_SEL_R_LEFT_CHANNEL 0x00
#define DIN_SEL_R_RIGHT_CHANNEL 0x10
#define DIN_SEL_R_LEFT_RIGHT_MIX_CHANNEL 0x20
#define DIN_SEL_R_FIX_ZERO 0x30
#define DIN_SEL_R_CLOSE 0xFF

#define COEF_DAC_TABLE_LENGTH (192)
#define COEF_VOICE_TABLE_LENGTH (82)
#define COEF_ADC_TABLE_LENGTH (336)
#define COEF_EQ_TABLE_LENGTH (153)

#define COEF_DAC_TABLE_48K_OFFSET (0)
#define COEF_DAC_TABLE_44K_OFFSET (192)
#define COEF_DAC_TABLE_8K_OFFSET (384)
#define COEF_VOICE_TABLE_OFFSET (576)
#define COEF_ADC_TABLE_OFFSET (658)
#define COEF_EQ_TABLE_OFFSET (994)
#define COEF_TABLE_ALL_LENGTH (1147)

#define CVSD_IN_MAX_LEN (0x2d0)


	/*Audio DAC output full-scale control
		1100: 1.2V (VDD>=1.7V)
		1101: 1.6V (VDD>=2.1V) 
		1110: 2.0V (VDD>=2.5V)
		1111: 2.4V (VDD>=2.9V)	*/
#define AUDIO_DAC_OUT_FULL_SCAL_CONTROL_1_2V (0x0C)
#define AUDIO_DAC_OUT_FULL_SCAL_CONTROL_1_6V (0x0d)
#define AUDIO_DAC_OUT_FULL_SCAL_CONTROL_2_0V (0x0e)
#define AUDIO_DAC_OUT_FULL_SCAL_CONTROL_2_4V (0x0f)


///////CORE_AADC_0///////
#define  OPACMP_VDD200mV_RG_ADAC_OPABIAS_SEL1 		0x01
#define  OPACMP_VDD260mV_RG_ADAC_OPABIAS_SEL1_2	0x02
#define  OPACMP_VDD360mV_RG_ADAC_OPABIAS_SEL2_3	0x04
#define  OPACMP_VDD460mV_RG_ADAC_OPABIAS_SEL3		0x08
typedef enum
{
	DAC_BIAS_2_0uA=0,
	DAC_BIAS_2_5uA,
	DAC_BIAS_3_0uA,
	DAC_BIAS_5_5uA=7,
	DAC_BIAS_4_0uA,
	DAC_BIAS_4_5uA,
	DAC_BIAS_7_5uA=15,
}DevAudioDacBiasControl;
///////CORE_AADC_1///////
typedef enum
{
	HPPA_BIAS_CURRENT_PROHIBITED=0, 
	HPPA_BIAS_CURRENT_4uA, 
	HPPA_BIAS_CURRENT_8uA,
	HPPA_BIAS_CURRENT_12uA,
}DevAudioHPPAbiasCurrentControl;
typedef enum
{
	HPPA_COMPENSATION_CAP_260fF=0, 
	HPPA_COMPENSATION_CAP_520fF,
	HPPA_COMPENSATION_CAP_780fF,
	HPPA_COMPENSATION_CAP_1040fF,
}DevAudioHPPACompensationCapControl;
///////CORE_AADC_2///////
#define HPPA_NMOS_CASCODE_VOLTAGE_200mV_OPABIAS1 		0x01
#define HPPA_NMOS_CASCODE_VOLTAGE_280mV_OPABIAS1_2 	0x02
#define HPPA_NMOS_CASCODE_VOLTAGE_280mV_OPABIAS2_3 	0x04
#define HPPA_NMOS_CASCODE_VOLTAGE_280mV_OPABIAS3 		0x08
typedef enum
{	
	//440uA/(1+ RG_ADAC_OPA3STA_SEL)
	RG_ADAC_OPA3STA_SEL0=0,
	RG_ADAC_OPA3STA_SEL1,
	RG_ADAC_OPA3STA_SEL2,
	RG_ADAC_OPA3STA_SEL3,
}DevAudioHPPAOutputStageQuiescentCurrent;
///////CORE_AADC_3///////
typedef enum
{
	OUTPUT_STAGE_CUR_80uA=0, 
	OUTPUT_STAGE_CUR0_120uA, 
	OUTPUT_STAGE_CUR_120uA, 
 	OUTPUT_STAGE_CUR_160uA,
}DevDACComModeVolBufOutputStageCur;
typedef enum
{
	RG_ADAC_OPABIAS_SEL1=6, //b110
	RG_ADAC_OPABIAS_SEL2=5, //b101
	RG_ADAC_OPABIAS_SEL3=3, //b011
}DevAudioHPPACascodeVoltage;
typedef enum
{
	USE_DEDICATED_BUFFER=0,
	USE_GOLBAL_COMMON_MODE_VOL_BUFFER,
}DevAudioVoltageBufferBypassMode;
typedef enum
{
	Feedback_Cap_8_8pF,
	Feedback_Cap_2x8_8pF,
}DevAudioDACFeedbackCapControl;
///////CORE_AADC_5///////
#define ADAC_LDO_HPVDD_VCTRL_VALUE 0
typedef enum
{
	DAC_COM_MODE_VOL1_25VDD2_5=0,//0.8*(40+22.5)/40=1.25V (VDD=2.5V)
	DAC_COM_MODE_VOL1VDD2,//1: 0.8*(40+10)/40=1.0V   (VDD=2V)
	DAC_COM_MODE_VOL0_9VDD1_8,//2/3: 0.8*(40+5)/40=0.9V (VDD=1.8V)
}DevAudioDACCommonModeVoltageControl;



void Audio_EqDisable(void);
void Audio_DACClkOff(void);
void Audio_DacStop(void);
void Audio_AdcStop(void);
void Audio_CvsdClkEnable(void);
void Audio_CvsdClkDisable(void);
void Audio_CvsdWaitCvsdFilterClrDone(void);
void Audio_CvsdCoefInit(void);
void Audio_CvsdStart(void);
void Audio_CvsdStop(void);
void Audio_CvsdInit(void);
void Audio_CvsdInitProcess(void);
void Audio_AdcCvsdInit(void);
void Audio_AdcInit(uint16_t adcStartAddr, uint16_t adcBufferSize, BOOL adcMramFlag, BOOL adcIs16K);
void Audio_AdcDmaDisable(void);
void Audio_AdcDmaEnable(void);
BOOL Audio_CheckAdcDmaEnable(void);
void Audio_AdcStart8K(BOOL adcMramFlag);
void Audio_AdcFilterInit(void);
void Audio_SbcBusyWait(void);
void Audio_SbcClkOff(void);
void Audio_SbcClkOn(void);
void Audio_SbcStop(void);
void Audio_mSbcStop(void);
void Audio_DacAnaClkEnable(void);
void Audio_DacAnaClkDisable(void);
void Audio_DacCloseL(void);
void Audio_DacSetL(uint8_t din_sel);
void Audio_DacCloseR(void);
void Audio_DacSetR(uint8_t din_sel);
void Audio_DacStero(void);
void Audio_DacMono(void);
void Audio_DacMonoStero(uint8_t mono_flag);
void Audio_DacFilter(void);
void Audio_DacFilterDisable(void);
void Audio_DacSdm(void);
void Audio_DacSdmDisable(void);
void Audio_DlychainDacInit(void);
uint32_t Audio_GetCoefStartAddr(uint16_t offset);
void Audio_LoadDacCoefCommon(uint32_t startAddr);
void Audio_LoadDac8kCoef(void);
void Audio_LoadDac16kCoef(void);
void Audio_LoadDac48kCoef(void);
void Audio_LoadDac44kCoef(void);
void Audio_DacAnalogSel48k(void);
void Audio_DacAnalogSel44k(void);
void Audio_DacSamplefreqSel8k(void);
void Audio_DacSamplefreqSel16k(void);
void Audio_DacSamplefreqSel48k(void);
void Audio_DacSamplefreqSel44k(void);
uint8_t Audio_DacClkCheck(uint8_t dac_clk);
void Audio_DacCoefInit(void);
void Audio_DacSetSampleRate(uint8_t dac_clk);
void Audio_DacVolAdjust(void);
void Audio_DacInit(uint8_t mono_flag, uint8_t l_din_sel, uint8_t r_din_sel, uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
uint8_t Audio_CheckDacMRAMSelect(void);
void Audio_DacInitMono(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
void Audio_DacInitStero(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
void Audio_DacInitTwsLeftMonoStero(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
void Audio_DacInitTwsRightMonoStero(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
void Audio_DacInitMedia(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
void Audio_DacInitVpCall(uint8_t dac_clk, uint8_t dac_mram_flag
	, uint16_t dac_start_addr, uint16_t dac_buf_size);
BOOL Audio_CheckDacEmpty(void);
void Audio_TishiAddressInit(uint16_t saddr, uint16_t len);
void Audio_TishiEnable(void);
void Audio_TishiDisable(void);
uint8_t Audio_CheckTishiMRAMSelect(void);


void Audio_AdacRefCtrl(uint8_t val);
void Audio_AdacPorDefaultInit(void);
void Audio_DacSetRampDownEnL(void);
void Audio_DacSetRampDownEnR(void);
void Audio_DacSetRampDownEn(void);
void Audio_DacSetDaAdacIntL(void);
void Audio_DacSetDaAdacIntR(void);
void Audio_DacSetDaAdacInt(void);
void Audio_DacDisableDaAdacInt(void);
void Audio_DacSetDaAdacOpaL(void);
void Audio_DacSetDaAdacOpaR(void);
void Audio_DacSetDaAdacOpa(void);
uint8_t Audio_DacCheckDaAdacOpaEnable(void);
uint8_t Audio_DacCheckDaAdacOpaEnableLeft(void);
uint8_t Audio_DacCheckDaAdacOpaEnableRight(void);
void Audio_DacDisableDaAdacOpa(void);
void Audio_DacSetDaAdacPathL(void);
void Audio_DacSetDaAdacPathR(void);
void Audio_DacSetDaAdacPath(void);
void Audio_DacDisableDaAdacPath(void);
void Audio_DacSetDaAdacDepopSwL(void);
void Audio_DacSetDaAdacDepopSwR(void);
void Audio_DacSetDaAdacDepopSw(void);
void Audio_DacDisableDaAdacDepopSw(void);
void Audio_DacSetDaAdacOpaDummyToMainL(void);
void Audio_DacSetDaAdacOpaDummyToMainR(void);
void Audio_DacSetDaAdacOpaDummyToMain(void);
void Audio_DacSetDaAdacRstIntL(void);
void Audio_DacSetDaAdacRstIntR(void);
void Audio_DacSetDaAdacRstInt(void);
void Audio_DacSetDaAdacRstIntDisable(void);
void Audio_DacSetDaEnAdacOpaDummyL(void);
void Audio_DacSetDaEnAdacOpaDummyR(void);
void Audio_DacSetDaEnAdacOpaDummy(void);
void Audio_DacDisableDaAdacOpaDummy(void);
void Audio_LpmWrite2AdcLow(uint32_t val);
void Audio_DaAaddaBgEn(void);
void Audio_DaAaddaBgDisable(void);
void Audio_DacSetVolBeforeCalibration(void);
void Audio_DacInitRampDown(void);
void Audio_DacInitForPop(uint8_t dac_clk);
void Audio_AdacStartPAWithPop(uint8_t dac_clk);
void Audio_DacWaitRampDown(void);
void Audio_DacClearRampDown(void);
void Audio_AdacStopPAWithPop(void);
void Audio_AdacSetAnaVol(uint8_t val);
void Audio_AdacSetAnaVolLeft(uint8_t val);
uint8_t Audio_AdacGetAnaVolLeft(void);
void Audio_AdacSetAnaVolRight(uint8_t val);
uint8_t Audio_AdacGetAnaVolRight(void);
void Audio_AdacAdjustAnaVolToNormal(void);
uint16_t Audio_AdacToolGetAdjustInterval(int16_t rang);
int16_t Audio_AdacToolGetSrcToDstVal(int16_t src, int16_t dst, uint16_t interval);
void Audio_AdacAdjustDCCaliToNormal(void);
void Audio_AdacAdjustDCCaliToZero(void);
void Audio_AdacAdjustDigitalVolToCali(void);
void Audio_AdacAdjustDigitalVolToZero(void);
void Audio_AdacPrepareReadyWork(void);
void Audio_AdacPreparePAClose(void);
void Audio_AdacPrepareSoftMute(void);
uint8_t Audio_AdacCheckLastDacClkSame(uint8_t dac_clk);
void Audio_CBClosePaDelay(int paras);
void Audio_ClosePaDelay(void);
void Audio_RelasePaDelay(void);
void Audio_AvdStopSbcDac(void);
void Audio_AnaAdcLowControlDisable(void);
void Audio_AnaAdcLowControlEn(void);
void Audio_AnaAdcHighControlDisable(void);
void Audio_AnaAdcHighControlEn(void);
void Audio_AnaStartAdcWork(void);
void Audio_AnaStopAdcWork(void);
BOOL Audio_CheckDacEnable(void);
uint16_t Audio_GetDacBufferSize(void);

void Audio_AdacOpaCmnSel(uint8_t vol);
void Audio_AdacHPPAOutputStageQuiescentCurrentControl(DevAudioHPPAOutputStageQuiescentCurrent cur);
void Audio_HPPACascodeVoltageControl(DevAudioHPPACascodeVoltage cas);
void Audio_AdacBiasSel(DevAudioDacBiasControl bias);
void Audio_AdacOpacmpSel(uint8_t opacmp);
void  Audio_AdacOpaBiasSel(DevAudioHPPAbiasCurrentControl hpcur);
void  Audio_AdacOpamcmpSel(DevAudioHPPACompensationCapControl hpcap);
void Audio_AdacVCMIOPA_SEL(DevDACComModeVolBufOutputStageCur cur);
void Audio_AdacComVoltageBufferModeSet(DevAudioVoltageBufferBypassMode mode);
void Audio_AdacFeedbackCapControl(DevAudioDACFeedbackCapControl feed);
void Audio_DacLDOOutputControl(uint8_t value);
void Audio_AdacDacCommonModeVoltageControl(DevAudioDACCommonModeVoltageControl val);
void Audio_AdacLdoAvddByPassEnable(void);
void  Audio_AdacLdoHpvddByPassEnable(void) ;
void NrAec_Init(uint8_t calltype);

void Audio_SbcClear(void);
void Audio_AdcStart16K(BOOL adcMramFlag);
void Audio_mSBCInitAdc(void);
void Audio_mSBCInitDac(void);
#endif
