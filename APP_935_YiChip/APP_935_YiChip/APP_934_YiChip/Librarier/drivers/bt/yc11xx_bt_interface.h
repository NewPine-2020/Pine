#ifndef _YC11XX_BT_INTERFACE_H_
#define _YC11XX_BT_INTERFACE_H_
#include "yc11xx.h"
#include "type.h"
#include "ipc.h"
#include "yc_debug.h"
#include "yc_timer.h"

#define VP_MODE   (0)
#define VP_MODE_BIT   (7)
typedef enum
{
	VP_STATE_DISENABLE=0,
	VP_STATE_ENABLE,
	VP_STATE_WAIT_SNIFF,
	VP_MODE_TISHI = 0x80,
}VP_PLAY_STATE;
typedef enum
{
	AVD_AUDIO_FORBID=0,
	AVD_AUDIO_ALLOW,
}AVD_AUDIO_ALLOW_CTRL;
typedef enum
{
	VP_LANGUAGE_ENGLISH=0,
	VP_LANGUAGE_CHINESE,
}VP_PLAY_LANGUAGE;


typedef struct{
	uint16_t clke_rt;
	uint32_t clke_bt;
	uint16_t empty;
}__attribute__((packed)) BT_CLK_INFO;

typedef enum{
	MUSIC_PLAY,
	MUSIC_PAUSE,
	MUSIC_SWITCH_PLAY_PAUSE,
}MUSIC_CMD;

void Bt_SndCmdBLE( uint8_t *data,uint8_t len);
void Bt_SndCmdStopReconTws(void);
void Bt_SndCmdChargePowerOff(void);
void Bt_SndCmdChargeStopPowerOff(void);
void Bt_SndCmdChargeChargeInscEnb(void);
void Bt_SndCmdChargeChargeInscEnbClear(void);
void Bt_SndCmdOtaReset(void);
BOOL Bt_CheckVpInProcess(void);
void Bt_SndCmdTwsUser1(void);
void Bt_SndCmdTwsUser2(void);
void Bt_SndCmdTwsUser3(void);
void Bt_SndCmdTwsUser4(void);
void Bt_SndCmdTwsUser5(void);
void Bt_SndCmdTwsUser6(void);
void Bt_SndCmdTwsUser7(void);
void Bt_SndCmdTwsFixVoicePlay(void);
void Bt_SndCmdPlayPause(MUSIC_CMD cmd);
void Bt_SndCmdBackward(void);
void Bt_SndCmdForward(void);
void Bt_SndCmdVolUp(void);
void Bt_SndCmdVolDown(void);
void Bt_SndCmdAcceptCall(void);
void Bt_SndCmdHangup(void);
void Bt_SndCmdRedial(void);
void Bt_SndCmdThreeWayReleaseAll(void);
void Bt_SndCmdThreeWayReleaseNAcp(void);
void Bt_SndCmdThreeWay_HoldNAcp(void);
void Bt_SndCmdRecognition(void);
void Bt_SndCmdRecognitionStop(void);
void Bt_SndCmdTwsSyncPwroff(void);
void Bt_SndCmdTwsSlaveInStorage(void);
void Bt_SndCmdTwsMasterInStorage(void);
void Bt_TwsSyncPowerOff(void);
void Bt_TwsInStorage(void);
void Bt_SndCmdReconnect(void);
void Bt_SndCmdStopReconnect(void);
void Bt_SndCmdHFPStartSync(void);
BOOL Bt_CheckAllowTishiWork(void);
void Bt_SndCmdWaitMaster(void);
void Bt_SndCmdWaitSlave(void);
void Bt_SndCmdExitTWS(void);
void Bt_SndCmdTwsNoConnectTransCmdPowerOff(void);
void Bt_SndCmdTwsNoConnectTransCmdInStore(void);
void Bt_SndCmdTwsNoConnectTransCmdChangeLanguageEn(void);
void Bt_SndCmdTwsNoConnectTransCmdChangeLanguageCh(void);
void Bt_SndCmdTwsNoConnectTransCmdSwitchOALOFF(void);
void Bt_SndCmdTwsNoConnectTransCmdSwitchOALON(void);
BOOL Bt_CheckAVRCPConnected(void);
BOOL Bt_CheckA2DPConnected(void);
BOOL Bt_CheckA2DPStart(void);
BOOL Bt_CheckHfpStart(void);
BOOL Bt_CheckHFPIsIncomming(void);
BOOL Bt_CheckTwsMasterConnected(void);
BOOL Bt_CheckTwsInMasterWorkProcess(void);
BOOL Bt_CheckTwsSlaveConnected(void);
BOOL Bt_CheckTwsInSlaveWorkProcess(void);
BOOL Bt_CheckTwsConnected(void);
BOOL Bt_CheckTwsHSPRoleLeft(void);
BOOL Bt_CheckHfpLocalInprocess(void);
BOOL Bt_CheckVpInprocess(void);
BOOL Bt_CheckVpInHardwareWork(void);
BOOL Bt_CheckMediaPlayInprocess(void);
void Bt_SndCmdEnterFastSyncMode(void);
BOOL Bt_CheckInFastSyncMode(void);
void Bt_CVSDWorkEnable(void);
void Bt_CVSDWorkDisable(void);
BOOL Bt_CheckTwsRoleMaster(void);
BOOL Bt_CheckTwsRoleSlave(void);
BOOL Bt_CheckCVSDEnable(void);
BOOL Bt_CheckCVSDWorkStart(void);
BOOL Bt_CheckCVSDWorkInprocess(void);
BOOL Bt_CheckCVSDInUseMRam(void);
BOOL Bt_CheckCVSDOutUseMRam(void);
BOOL Bt_CheckSCOInHV3EV3Transaction(void);
BOOL Bt_CheckSCOInHV3Transaction(void);
uint16_t Bt_GetAdcCurrentBufferSize(void);
void Bt_GetMac(uint8_t *adr);
void Bt_AdjustVol(BOOL isVolUp);
void Bt_SndCmdDisconnect(void);
void Bt_SndCmdStartDiscovery(void);
void Bt_SndCmdStopDiscovery(void);
void Bt_SndCmdStartReconTws(void);
void Bt_SndCmdPwroff(void);
void Bt_SndCmdAudioProcessInfo(void);
void Bt_SndCmdExitFastSyncMode(void);
void Bt_SndCmdEnterFastSyncMode(void);
void Bt_SndCmdRecoverAudio(void);
void Bt_SndCmdLeUpdateConn(void);
void Bt_LpmUpdateChargeSetting(uint32_t tmp);
void Bt_ChargeInitSettingInPowerOn(void);
void Bt_ChargeShutdownSw(void);
void Bt_ChargeShutdownSwClear(void);
void Bt_ChargeShutdownBias(void);
void Bt_ChargeShutdownBiasClear(void);
void Bt_ChargeShutdownCore(void);
void Bt_ChargeShutdownCoreClear(void);
void Bt_ChargeShutdownProcess(void);
void Bt_ChargePowerUpProcess(void);
void Bt_ChargeInscEnb(void);
void Bt_ChargeInscEnbClear(void);
void Bt_LpmUpdateChargeWakeupSetting(uint16_t lowActive, uint16_t highActive);
void Bt_ChargeSetRchc(uint8_t val);
uint16_t MedianFilter(uint16_t* pData,int nSize);
void Bt_SndCmdShutdownQspi(void);
void Bt_RemoveSCOLink(void);
void Bt_CreateSCOLink(void);
void Bt_SwitchSCOLinkState(void);
void Bt_LpmChangeBBLdoLevel(uint8_t val);

void Bt_SndCmdTwsOalFunc(uint8_t wordnum);
void Bt_SndCmdEnterSniff(void);
void Bt_SndCmdExitSniff(void);
uint8_t Bt_CheckLinkInSniff(void);

void Bt_SndCmdEnterTestPlay(void);

void Bt_SndCmdEnterTestPause(void);
void Bt_SndCmdExitTestSCO(void);

#endif //_YC11XX_BT_INTERFACE_H_

 
