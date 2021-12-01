/**
  ******************************************************************************
  * @file    main.c
  * @author  yichip
  * @version V1.0.0
  * @date    7-9-2018
  * @brief   This file provides all the Application firmware functions.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/ 
#include <string.h>
#include <stdio.h>
#include "usb_main.h"
#include "usb_dcd_int.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "private.h"
#include "usbd_hid_keyboard_core.h"
//#include "hid_keyboard.h"
//#include "app_config.h"

#define USB_OTG_WRITE_REG8(reg,value)   (*(volatile uint8_t *)reg = value)
	


#ifndef USB_USER_MRAM 
//volatile uint8_t   usb_rxbuf[200]       __attribute__ ((at(0x10002600)));
//volatile uint8_t   usb_txbuf0[65]      __attribute__ ((at(0x10002200)));
//volatile uint8_t   usb_txbuf1[65]      __attribute__ ((at(0x10002300)));
//volatile uint8_t   usb_txbuf2[65]      __attribute__ ((at(0x10002400)));
//volatile uint8_t   usb_txbuf3[65]      __attribute__ ((at(0x10002500)));
volatile uint8_t   usb_rxbuf[70]      __attribute__ ((at(0x10003efc)));
volatile uint8_t   usb_txbuf0[65]      __attribute__  ((at(0x10003e98)));
volatile uint8_t   usb_txbuf1[65]      __attribute__  ((at(0x10003dd0)));
volatile uint8_t   usb_txbuf2[65]      __attribute__  ((at(0x10003d6c)));
volatile uint8_t   usb_txbuf3[65]      __attribute__  ((at(0x10003d08)));

#else
 uint8_t   usb_rxbuf[70] ={0};
 uint8_t   usb_txbuf0[65] ={0};
 uint8_t   usb_txbuf1[65] ={0};
 uint8_t   usb_txbuf2[65] ={0};
 uint8_t   usb_txbuf3[65]={0};
#endif

USB_OTG_CORE_HANDLE  USB_OTG_dev;


enum
{
	keyboard_idle = 0,
	keyboard_busy = 1,
	keyboard_release = 2,
};

typedef struct
{
	uint8_t state;
	uint8_t keyboard_state;
} theUSB_t;

static void hal_usb_hid_tx_done(void);
static void hal_usb_hid_rx_cb(uint8_t *buf, uint32_t len);
static void usb_cdc_tx_done(void);
static void hal_usb_cdc_rx_cb(uint8_t *buf, uint32_t len);

USBD_FN_CB_T hid_fn =
{
	hal_usb_hid_tx_done,
	hal_usb_hid_rx_cb,
};

USBD_FN_CB_T cdc_fn = 
{
	usb_cdc_tx_done,
	hal_usb_cdc_rx_cb,
};

static theUSB_t theUSB;
static uint8_t  keycode_pointer[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t  cdc_sned_buf[65] 		={0};

void usb_send_report(void)
{

//	uint16_t len = 0;

//	if(BT_FEATURE(usb_mode) == 1)
//	{
//		if(!theBT.FIFO_Inst->len(FID_USB) || theUSB.keyboard_state != keyboard_idle)	return;
//		memset((uint8_t*)keycode_pointer,0,8);
//		if(BT_FEATURE(hid_mode) == 0)
//		{
//			
//			if(theBT.FIFO_Inst->len(FID_USB)<2)
//				return;
//			theUSB.keyboard_state = keyboard_busy;
//			theBT.FIFO_Inst->get(FID_USB,(uint8_t*)&keycode_pointer[0],1);
//			theBT.FIFO_Inst->get(FID_USB,(uint8_t*)&keycode_pointer[2],1);
//			USBD_HID_SendReport(&USB_OTG_dev, (uint8_t*)keycode_pointer, 8);	
//			
//		}
//		else
//		{
//			uint8_t character;
//			theBT.FIFO_Inst->get(FID_USB,&character,1);
//			if(keycode_and_modifer_us_for_character(character,(uint8_t*)&keycode_pointer[2],(uint8_t*)&keycode_pointer[0]))
//			{
//				theUSB.keyboard_state = keyboard_busy;
//				USBD_HID_SendReport(&USB_OTG_dev, (uint8_t*)keycode_pointer, sizeof(keycode_pointer));	

//			}
//		}	
//	}
//	else if(BT_FEATURE(usb_mode) == 2)
//	{
//		if(!theBT.FIFO_Inst->len(FID_USB)) 
//		{
//			return;
//		}	
//		
//		len = theBT.FIFO_Inst->len(FID_USB);
//		len = len > 64 ? 64 : len;
//		if ((USBHREAD(core_usb_txbusy) & 0x04) == 0x00)
//		{
//			theBT.FIFO_Inst->get(FID_USB,cdc_sned_buf,len);
//			DCD_EP_Tx (&USB_OTG_dev, CDC_IN_EP, cdc_sned_buf, len);
//		}

//	}

}

static void hid_release_keyboard(void)
{
	memset(keycode_pointer,0,sizeof(keycode_pointer));
	USBD_HID_SendReport(&USB_OTG_dev, keycode_pointer, sizeof(keycode_pointer));
}

uint8_t hal_usb_send(uint8_t *data,uint16_t len)
{
//	if(!len)	return 0;
//	if(theBT.FIFO_Inst->left(FID_USB) < len)	return 0;
//	theBT.FIFO_Inst->put(FID_USB,data,len);
	return 1;
}

static void hal_usb_hid_rx_cb(uint8_t *buf, uint32_t len)
{  

}

static void hal_usb_hid_tx_done(void)
{
//	BT_PRINT(("hal_usb_hid_tx_done\r\n"));
	if(theUSB.keyboard_state == keyboard_busy)
	{
		
		theUSB.keyboard_state = keyboard_release;
		hid_release_keyboard();	
	}
	else if(theUSB.keyboard_state == keyboard_release)
	{
		theUSB.keyboard_state = keyboard_idle;
		usb_send_report();
	}	
}

void usb_cdc_tx_done(void)
{
//	if(theUSB.keyboard_state == keyboard_busy)
//	{
//		theUSB.keyboard_state = keyboard_idle;
//		usb_send_report();
//	}
		
	
	return;
}

void hal_usb_cdc_rx_cb(uint8_t *buf, uint32_t len)
{
//	btprintf("hal_usb_cdc_rx_cb\r\n");
//	theBT.FIFO_Inst->put(FID_BT_TRANSFER,buf,len);
//	theBT.hUartRx_Active = 2;	
}

void usb_init()
{
	USBHWRITE(core_usb_config, 0x00);
	USBHWRITE(core_usb_drv,0x00);
	USBHWRITE(core_usb_addr,0x00);
	USBHWRITEW(core_usb_rx_saddr, (uint32_t*)usb_rxbuf);
	USBHWRITEW(core_usb_rxptr, (uint32_t*)usb_rxbuf);
	USBHWRITEW(core_usb_rx_eaddr, (uint32_t*)((int)usb_rxbuf + sizeof(usb_rxbuf)));
	USBHWRITEW(core_usb_tx_saddr0, (uint32_t*)usb_txbuf0);
	USBHWRITEW(core_usb_tx_saddr1, (uint32_t*)usb_txbuf1);
	USBHWRITE(core_usb_status,0xff);
	USBHWRITEW(core_usb_tx_saddr2, (uint32_t*)usb_txbuf2);
	USBHWRITEW(core_usb_tx_saddr3, (uint32_t*)usb_txbuf3);	
	
	
	USBHWRITE(core_usb_config, 0x3c);
	#ifndef USB_USER_MRAM
	USBHWRITE(core_usb_hmode, 0x04);
	#else
	USBHWRITE(core_usb_hmode, 0x0c);
	#endif
	USBHWRITE(core_usb_int_mask,0x16);
	
}

void usb_main(void)
{
//	if(BT_FEATURE(usb_mode) == 0)
//		return; 	
	USBHWCOR(CORE_CLKOFF+1,0x04);
	HWRITE(CORE_CLKSEL,0x01);




	usb_init();

	memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));
	memset(&theUSB,0,sizeof(theUSB_t));



//	if(BT_FEATURE(usb_mode) == 1)
	{

		USBD_Init(&USB_OTG_dev,
		#ifdef USE_USB_OTG_HS
		USB_OTG_HS_CORE_ID,
		#else
		USB_OTG_FS_CORE_ID, 
		#endif
		&USR_desc, 
		&USBD_HID_cb, 
		&USRD_cb);
		
		USBD_HID_RegisterCB(&hid_fn);
		enable_intr(USB_INTID); 
	}	
//	else if(BT_FEATURE(usb_mode) == 2)	//USB-UART
//	{

//		USBD_Init(&USB_OTG_dev,
//		#ifdef USE_USB_OTG_HS
//			  USB_OTG_HS_CORE_ID,
//		#else
//			  USB_OTG_FS_CORE_ID, 
//		#endif
//			  &USR_desc,
//			  &USBD_CDC_cb, 
//			  &USRD_cb);
//		USBD_CDC_RegisterCB(&cdc_fn);
//		enable_intr(USB_INTID);
//	}
//	while(!theBT.usb_init_ok)
//	{
//#ifdef FUNCTION_WATCH_DOG
//		WDT_Kick();
//#endif
//	}

}  



void USB_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&USB_OTG_dev);
}


void usb_reset(void)
{
	disable_intr(USB_INTID);
	USBHWRITE(core_usb_addr, 0x00);
	USBHWRITE(core_usb_status, 0x7F);
	USBHWRITE(core_usb_hmode, 0x00);
	USBHWRITE(core_usb_clear, 0x80);
	//MyPrintf("YICHIP SCPU USB CCID demo V1.0.  reset \n");
	usb_init();
	memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));

	USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
			  USB_OTG_HS_CORE_ID,
#else
			  USB_OTG_FS_CORE_ID,
#endif
			  &USR_desc,
			  &USBD_HID_cb,
			  &USRD_cb);
		enable_intr(USB_INTID); 
	 return ;
}







 


