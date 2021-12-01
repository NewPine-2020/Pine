#include <stdarg.h>
#include "private.h"
#include "type.h"
#include "btreg.h"
#include "yc_debug.h"
//#include "ipc.h"
//#include "Drv_bt.h"
//#include "nvic.h"

//#include "hal.h"
//#include "hal_uart.h"
//#include "spp.h"
//#include "le_peripheral.h"
//#include "hid_keyboard.h"
//#include "xmodem.h"
#include "usb_main.h"


//tIPCHandleCb gTIPCHandleCb[IPC_TYPE_NUM]=
//{
//	0,
//	IpcDefaultCallBack,//cmd
//	Bt_EvtCallBack,//evt
//	IpcDefaultCallBack,//hid
//	//Bt_SPPCallBack,//spp
//	IpcDefaultCallBack,
//	IpcDefaultCallBack,//ble
//	IpcDefaultCallBack,//24g
//	IpcDefaultCallBack,//mesh
//	IpcDefaultCallBack,//mesh
//	IpcDefaultCallBack,//mesh
//	Bt_DataBufferCallBack,//a2dp
//	IpcDefaultCallBack,//hfp
//	IpcDefaultCallBack,//tws
//};
//WDT_InitTypeDef  gWdtInit =
//{
//	.WDTx =WDT2,
//	.mode =RESET_MODE,
//	.setload =WDT_timer_1_5s,

//};

int main(void)
{
//	Bt_HciFifoInit();
//	*(volatile byte*)(reg_map_m0(0x1f000)) = 0x1c;//Enable cm0 debug
	SYS_TimerInit(CLOCK_48M_multiple);
//	BT_FEATURE(usb_mode) = 1;
		
	
//	IPC_init(&gTIPCHandleCb);
//	
//	
//	DEBUG_INIT();
//	DEBUG_LOG_STRING("init sussess!\r\n");
	usb_main();
//#ifdef FUNCTION_WATCH_DOG
//	WDT_Start(&gWdtInit);
//#endif

//	hal_init();

//	bt_blueware_init();
	
	

	while (1)
	{
//		switch (HREAD(IPC_MCU_STATE))
//		{
//			case IPC_MCU_STATE_RUNNING:
//				//always first init bt
//				IPC_HandleRxPacket();

//				bt_main_thread();
//					
//				xmodem_thread();	
//					
//				//Bt_watchdog();
//				if (gBRState.topState != BR_POWER_OFF)
//				{
//					DEBUG_POLLING_PRINT();

//					SYS_timerPolling();
//		
//					//h4_polling_rx();				
//				}
//				Lpm_unLockLpm(M0_LPM_FLAG);
//				break;
//			case IPC_MCU_STATE_LMP:
//				if (IPC_IsTxBuffEmpty()
//					&& DEBUG_LOG_BUFFER_IS_EMPTY())
//				{
//					OS_ENTER_CRITICAL();
//					Bt_ActionBeforeLpm();
//					HWRITE(IPC_MCU_STATE,IPC_MCU_STATE_STOP);
//				}
//				else
//				{
//					HWRITE(IPC_MCU_STATE,IPC_MCU_STATE_RUNNING);
//				}
//				break;
//			case IPC_MCU_STATE_HIBERNATE:
//				OS_ENTER_CRITICAL();
//				Bt_ActionBeforeHibernate();
//				HWRITE(IPC_MCU_STATE,IPC_MCU_STATE_STOP);
//				break;
//			case IPC_MCU_STATE_STOP:
//				break;
//		}
	};
}

//void HardFault_Handler(void)
//{
//	HWRITE(CORE_RESET,0x03);
//	while(1);
//}

//void PendSV_Handler(void)
//{
//	HWRITE(CORE_RESET,0x03);
//	while(1);
//}

/**
  * @brief  Systick interrupt service function.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//	SYStick_handle();
//	blueware_tick_handler();	
//	theBT.system_ticker++;
//}

void _assert_handler(const char* file, int line,const char* func)
{
//	HWRITE(CORE_RESET,0x03);
	while(1);
}

