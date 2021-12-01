#ifndef _PRIVATE_H_
#define _PRIVATE_H_

//#include "bw_config.h"
//#include "bw_utils.h"

#include <stdbool.h>
#include <string.h>
//#include "fifo.h"
//#include "common.h"
//#include "mini-printf.h"
//#include "blueware.h"
//#include "bw_debug.h"

#include "yc11xx.h"
#include "yc11xx_wdt.h"
#include "yc_drv_common.h"
#include "yc11xx_gpio.h"
#include "yc11xx_uart.h"
#include "yc11xx_qspi.h"
#include "yc_timer.h"
#include "yc11xx_systick.h"

#define UART_PARSER_TICKS							(10/TICK_PERIOD_IN_MS)

#define LED_ON_TIMEOUT								(200)
#define LED_OFF_TIMEOUT								(1000)
#define LED_ON_TIMEOUT_UNAUTHED						(200)
#define LED_OFF_TIMEOUT_UNAUTHED					(200)

#define BT_TRANSFER_BUF_SIZE						(4096)
#define HUART_TX_BUF_SIZE							(4096)
#define BUART_RX_BUF_SIZE							(2048)

#define HOST_WAKE_TIMEOUT 	    					(10000/TICK_PERIOD_IN_MS)
#define BT_WAKE_TIMEOUT 	    					(6000/TICK_PERIOD_IN_MS)

#define EDR_SNIFF_INTERVAL_MIN						(996)//(800)// 800*0.625 = 500ms
#define EDR_SNIFF_INTERVAL_MAX						(996)//(800)// 800*0.625 = 500ms
#define EDR_SNIFF_ATTEMPS							(10)//(1)
#define EDR_SNIFF_TIMEOUT							(10)//(8)

//data temp buffer 
#define DATA_TRANSFER_BUFF_T_SIZE					(520)
#define DATA_HUART_TX_DMA_BUFF_SIZE					64//(32)

#define SPP_DEFAULT_MTU_SIZE						(512)

#define PROFILE_SPP								(uint16)(BIT0)
#define PROFILE_GATT_SERVER						(uint16)(BIT1)
#define PROFILE_GATT_CLIENT						(uint16)(BIT2)
#define PROFILE_HFP_HF							(uint16)(BIT3)
#define PROFILE_HFP_AG							(uint16)(BIT4)
#define PROFILE_A2DP_SINK						(uint16)(BIT5)
#define PROFILE_A2DP_SOURCE						(uint16)(BIT6)
#define PROFILE_AVRCP_CT						(uint16)(BIT7)
#define PROFILE_AVRCP_TG						(uint16)(BIT8)
#define PROFILE_HID_KBD							(uint16)(BIT9)
#define PROFILE_PBAPC							(uint16)(BIT10)
#define PROFILE_A2DP_PEER						(uint16)(BIT11)
#define PROFILE_BIT_MAST						(uint16)(0x0FFF)

#define DEVICE_MASK_PWR_BIT							BIT0
#define DEVICE_MASK_PAIRING_BIT						BIT1				
#define DEVICE_MASK_ADVERTISING_BIT					BIT2				
#define DEVICE_MASK_INQUIRING_BIT					BIT3				
#define DEVICE_MASK_SCANNING_BIT					BIT4				
#define DEVICE_MASK_LPM_BIT							BIT5		
// this bit not need to send to the user
#define DEVICE_MASK_CONNECTABLE_BIT					BIT15				


//typedef struct
//{
//	le_addr_t	bd_addr;
//	//uint8_t		remote_rssi;
//	uint8_t 	remote_name_size;
//}inquiry_scan_result_t;

//typedef enum
//{
//	transport_mode_tbd, // to be determined
//	transport_mode_air_cmd,
//	transport_mode_throughput,
//	transport_mode_dfu
//}transport_mode_t;

//typedef enum
//{
//	device_auth_failed,
//	device_auth_success,
//	device_auth_production
//}device_auth_t;

//typedef enum
//{
//  CONN_LPM_OFF,
//  CONN_LPM_INTERIM,
//  CONN_LPM_ON,
//}conn_lpm_t;

//typedef struct
//{
//	uint8_t 					inquiry_scan_timeout;
//	uint8_t 					inquiry_scan_count;
//	uint8_t 					inquiry_scan_print;
//	inquiry_scan_result_t		inquiry_scan_result[MAX_NO_BD_INQUIRY_RESULT];
//}theScanResult_t;

//typedef struct
//{
//	const theFIFO_Inst_t 		*FIFO_Inst;	
//	bd_addr_t					localBdaddr;
//	transport_mode_t 			transport_mode;	
//	device_auth_t				Authed;
//	uint16_t					rssi_tickout;
//	uint16_t 					connected_profile;
//	uint16_t 					state_mask; 
//	volatile uint32_t			system_ticker;
//	uint32_t					OTA_flash_start_addr;
//	uint8_t						hUartRx_Active;
//	uint16_t                     usb_active;
//	uint8_t						reconnect_attempts;
//	uint16_t					hci_version;
//	uint8_t						blueware_version[3];
//	uint8_t						extended_inquiry_response_data[240];
//	uint32_t 					suicide; // use hci version(non-connected)  ind as bt watchdog
//	
//	le_addr_t					remote_address;
//	uint8_t						software_reset;
//	unsigned 					nvmLock:1;
//	unsigned					production_mode:1;
//	unsigned					throughput_mode:1;
//	unsigned					bt_disabled:1;
//	unsigned					bt_disconnect_gpio_status:1;
//	unsigned					rssi_send_state:1;
//	unsigned					reset_status:1;
//	unsigned					usb_init_ok:1;
//	unsigned					reserved:8;

//}theBT_t;

//void app_set_state(uint16_t mask);
//void app_reset_state(uint16_t mask);
//uint16_t app_get_state(uint16_t bits);

//void app_update_profile_state(uint16_t profile_id,uint8_t old_state,uint8_t new_state,uint8_t idx);
//uint16_t app_is_powered_on(void);
//uint8_t app_bt_is_connected(void);
//void app_reset_bt_scan_param(uint8 period);
//uint8_t app_is_profile_enabled(uint8_t profile);
//void app_update_extended_inquiry_response(void);

//void bt_blueware_init(void);
//void bt_main_thread(void);

//extern theBT_t 	theBT;
//extern theDB_t 	theDB;
//extern theExtendedDB_t theExtendedDB;
//extern theScanResult_t theScanResult;
//extern uint8 hci_auto_reset;

//#define isDevAuthed()         (theBT.Authed == device_auth_success)
//#define isDevProdAuthed()     (theBT.Authed >= device_auth_success) 

//#define BT_FEATURE(x) (theDB.settings.feature.x)

#endif

