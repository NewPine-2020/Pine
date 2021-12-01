/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file n32g020xx_usbd_hid_keyboard_core.c
 * @author Nations Solution Team
 * @version v1.0.0
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */




/* Includes ------------------------------------------------------------------*/
#include "usbd_desc.h"

#include "usbd_hid_keyboard_core.h"
#include "usb_dcd.h"
//#include "blueware.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "private.h"
#include "usb_dcd_int.h"
#ifndef LOG_ENABLE
    #include <stdio.h>
#endif


/* Private typedef -----------------------------------------------------------*/

typedef struct
{
    USBD_FN_CB_T fn_cb;
} theCustomHID_t;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
uint8_t  USBD_HID_Init(void  *pdev,
                       uint8_t cfgidx);

uint8_t  USBD_HID_DeInit(void  *pdev,
                         uint8_t cfgidx);

uint8_t  USBD_HID_Setup(void  *pdev,
                        USB_SETUP_REQ *req);

uint8_t  *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length);


uint8_t  USBD_HID_DataIn(void  *pdev, uint8_t epnum);


uint8_t  USBD_HID_DataOut(void  *pdev, uint8_t epnum);


uint8_t  USBD_HID_EP0_RxReady(void  *pdev);


USBD_Class_cb_TypeDef  USBD_HID_cb =
{
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Setup,
    NULL, /*EP0_TxSent*/
    USBD_HID_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
    USBD_HID_DataIn, /*DataIn*/
    USBD_HID_DataOut, /*DataOut*/
    NULL, /*SOF */
    NULL, /*SOF */
    NULL,
    USBD_HID_GetCfgDesc,
};



uint8_t Report_buf[HID_KEYBOARD_OUT_PACKET];
uint8_t USBD_HID_Report_ID = 0;
uint8_t flag = 0;

static uint32_t  USBD_HID_AltSet = 0;

static uint32_t  USBD_HID_Protocol = 0;

static uint32_t  USBD_HID_IdleState = 0;

static theCustomHID_t theCustomHID =
{
    .fn_cb.usb_txdone = NULL,
    .fn_cb.usb_rx_cb = NULL,
};

/* USB HID device Configuration Descriptor */
/*const*/ uint8_t USBD_HID_CfgDesc[CUSTOMHID_SIZ_CONFIG_DESC] =
{
    0x09, /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    CUSTOMHID_SIZ_CONFIG_DESC,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01,         /*bNumInterfaces: 1 interface*/
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
    0x80,         /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Custom HID interface ****************/
    /* 09 */
    0x09,         /*bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
    0x00,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x01,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x01,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,            /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Custom HID ********************/
    /* 18 */
    0x09,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x00,         /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,         /*bDescriptorType*/
    CUSTOMHID_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Custom HID endpoints ***********/
    /* 27 */
    0x07,          /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */

    HID_IN_EP,     /* bEndpointAddress: Endpoint Address (IN) */
    0x03,          /* bmAttributes: Interrupt endpoint */
    HID_KEYBOARD_IN_PACKET, /* wMaxPacketSize: 64 Bytes max */
    0x00,
    0x02,          /* bInterval: Polling Interval (1 ms) */
    /* 34 */

//  0x07,            /* bLength: Endpoint Descriptor size */
//  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: */
//  /*  Endpoint descriptor type */
//  HID_OUT_EP, /* bEndpointAddress: */
//  /*  Endpoint Address (OUT) */
//  0x03,   /* bmAttributes: Interrupt endpoint */
//  HID_KEYBOARD_OUT_PACKET,    /* wMaxPacketSize: 64 Bytes max  */
//  0x00,
//  0x02,   /* bInterval: Polling Interval (1 ms) */
    /* 41 */
} ;


//63  keyboard
const uint8_t CustomHID_ReportDescriptor[CUSTOMHID_SIZ_REPORT_DESC] =
{
    0x05,0x01,				// Usage Page (Generic Desktop),   
	0x09,0x06,				// Usage (Keyboard),                               
	0xA1,0x01,				// Collection (Application),      
	0x85,0x01,			//Report ID (01)	
	0x05,0x07,				// Usage Page (Key Codes);                         
    0x19,0xe0,              // Usage Minimum (224),                            
	0x29,0xe7,              // Usage Maximum (231),                            
	0x15,0x00,              // Logical Minimum (0),                            
	0x25,0x01,              // Logical Maximum (1),                            
    0x75,0x01,              // Report Size (1),                                
	0x95,0x08,              // Report Count (8),                               
	0x81,0x02,              // Input (Data, Variable, Absolute), ;Modifier byte
	0x95,0x01,              // Report Count (1),                               
    0x75,0x08,              // Report Size (8),                                
	0x81,0x01,              // Input (Constant), ;Reserved byte                
	0x95,0x03,              // Report Count (3),                               
	0x75,0x01,              // Report Size (1),                                
    0x05,0x08,              // Usage Page (Page# for LEDs),                    
	0x19,0x01,              // Usage Minimum (1),                              
	0x29,0x03,              // Usage Maximum (3),                              
	0x91,0x02,              // Output (Data, Variable, Absolute), ;Led report  
    0x95,0x05,              // Report Count (5),                               
	0x75,0x01,              // Report Size (1),                                
	0x91,0x01,              // Output (Constant), ;Led report padding          
	0x95,0x06,              // Report Count (6),                               
    0x75,0x08,              // Report Size (8),                                
	0x26,0xff,0x00,         //0x25, 0xff,        // Logical Maximum(101),                        
	0x05,0x07,              // Usage Page (Key Codes),                          
	0x19,0x00,              // Usage Minimum (0),                             
	0x29,0x91,              // Usage Maximum (101),                           
	0x81,0x00,              // Input (Data, Array), ;Key arrays (6 bytes) 
	0xC0 ,                   // End Collection 
	
	0x06,0x66,0xff,		// Usage Page (Generic Desktop), 
	0x75,0x08,			//Report Size (8)
	0x26,0xff,0x00,		//Logical Maximum (255)
	0x09,0x03,			//Usage (Vendor-Defined 3)
	0xa1,0x02,			//Collection (Logical)
	0x85,0xfd,			//Report ID (253)
	0x95,0x0c,			//Report Count (12)
	0x09,0x03,			//Usage (Vendor-Defined 3) 
	0xb2,0x82,0x01,
	0xC0                    // End Collection  

}; /* CustomHID_ReportDescriptor */


/* Private function ----------------------------------------------------------*/
/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
uint8_t  USBD_HID_Init(void  *pdev, uint8_t cfgidx)
{
    //DCD_PMA_Config(pdev , HID_IN_EP,USB_SNG_BUF,HID_IN_TX_ADDRESS);
    //DCD_PMA_Config(pdev , HID_OUT_EP,USB_SNG_BUF,HID_OUT_RX_ADDRESS);

    /* Open EP IN */
    DCD_EP_Open(pdev,
                HID_IN_EP,
                HID_KEYBOARD_IN_PACKET,
                USB_EP_INT);

    /* Open EP OUT */
//  DCD_EP_Open(pdev,
//              HID_OUT_EP,
//              HID_KEYBOARD_OUT_PACKET,
//              USB_EP_INT);

    /*Receive Data*/
//  DCD_EP_PrepareRx(pdev,HID_OUT_EP,Report_buf,HID_KEYBOARD_OUT_PACKET);

    return 0;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
uint8_t  USBD_HID_DeInit(void  *pdev, uint8_t cfgidx)
{
    /* Close HID EPs */
    DCD_EP_Close(pdev, HID_IN_EP);
    DCD_EP_Close(pdev, HID_OUT_EP);

    return 0;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
uint8_t  USBD_HID_Setup(void  *pdev, USB_SETUP_REQ *req)
{
	uint16_t rx_ep;
	uint8_t USBD_HID_Report_LENGTH=0;
    uint16_t len = 0;
    uint8_t  *pbuf = NULL;
    uint8_t  hid_buffer[8] = {0};
//  theBT.usb_init_ok =1;
    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    case USB_REQ_TYPE_CLASS :
        switch (req->bRequest)
        {
        case HID_REQ_SET_PROTOCOL:
            USBD_HID_Protocol = (uint8_t)(req->wValue);
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_Protocol,
                             1);
            break;

        case HID_REQ_GET_PROTOCOL:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_Protocol,
                             1);
            break;

        case HID_REQ_SET_IDLE:
            USBD_CtlSendData(pdev,
                             0,
                             0);
            break;

        case HID_REQ_GET_IDLE:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_IdleState,
                             1);
            break;

        case HID_REQ_SET_REPORT:
            for (int i = 0; i < 30000; i++)
            {}
            USBHWRITE(core_usb_status, 0x20);
            rx_ep = usb_getword();
            //ep_num_cdc = (uint8_t)(rx_ep_cdc >> 12);                
			USBD_HID_Report_LENGTH = rx_ep & 0xfff;
				
            USB_OTG_ReadPacket(pdev, hid_buffer, 0, USBD_HID_Report_LENGTH);
            USBHWRITE(core_usb_trig, 0x10);

            break;
        case HID_REQ_GET_REPORT:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_IdleState,
                             1);

            break;
        default:
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
        }
        break;

    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:

            if (req->wValue >> 8 == HID_REPORT_DESC)
            {
                len = MIN(CUSTOMHID_SIZ_REPORT_DESC, req->wLength);
                pbuf = (uint8_t *)CustomHID_ReportDescriptor;
            }
            else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
            {
//          USBD_HID_CfgDesc[33] = BT_FEATURE(hid_report_tick);
//      USBD_HID_CfgDesc[40] = BT_FEATURE(hid_report_tick);
                pbuf = (uint8_t *)USBD_HID_CfgDesc + 0x12;
                len = MIN(USB_HID_DESC_SIZ, req->wLength);
            }

            USBD_CtlSendData(pdev,
                             pbuf,
                             len);

            break;

        case USB_REQ_GET_INTERFACE :
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_AltSet,
                             1);
            break;

        case USB_REQ_SET_INTERFACE :
            USBD_HID_AltSet = (uint8_t)(req->wValue);
            break;
        }
    }
    return 0;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE  *pdev,
                            uint8_t *report,
                            uint16_t len)
{
    /* Check if USB is configured */
    if (pdev->dev.device_status == USB_OTG_CONFIGURED)
    {
        DCD_EP_Tx(pdev, HID_IN_EP, report, len);
    }
    return 0;
}

/**
  * @brief  USBD_HID_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length)
{
//  USBD_HID_CfgDesc[33] = BT_FEATURE(hid_report_tick);
//  USBD_HID_CfgDesc[40] = BT_FEATURE(hid_report_tick);
    *length = sizeof(USBD_HID_CfgDesc);
    return (uint8_t *)USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_HID_DataIn(void  *pdev, uint8_t epnum)
{
//  DEBUG(("USBD_HID_DataIn\r\n"));
//  BT_PRINT(("USBD_HID_DataIn epnum = %x\r\n",epnum));
//  if (epnum == (HID_IN_EP&0x7F))
//  if(epnum != 0)
    {
        if (theCustomHID.fn_cb.usb_txdone)
            theCustomHID.fn_cb.usb_txdone();
    }

    return 0;
}

/**
  * @brief  USBD_HID_DataOut
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_HID_DataOut(void  *pdev, uint8_t epnum)
{
//  uint16_t USB_Rx_Cnt;
//  if (epnum ==(HID_OUT_EP&0x7F))
//  {
//    if(theCustomHID.fn_cb.usb_rx_cb)
//        theCustomHID.fn_cb.usb_rx_cb(Report_buf, USB_Rx_Cnt);
//  }

    DCD_EP_PrepareRx(pdev, HID_OUT_EP, Report_buf, HID_KEYBOARD_OUT_PACKET);

    return 0;
}

/**
  * @brief  USBD_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

uint8_t USBD_HID_EP0_RxReady(void *pdev)
{
    if (flag == 1)
    {
        flag = 0;

    }
    return 0;
}

extern void USBD_HID_PrepareRx(void  *pdev)
{
    DCD_EP_PrepareRx(pdev, HID_OUT_EP, Report_buf, HID_KEYBOARD_OUT_PACKET);
}

extern void USBD_HID_RegisterCB(USBD_FN_CB_T *fn)
{
    theCustomHID.fn_cb.usb_txdone = fn->usb_txdone;
    theCustomHID.fn_cb.usb_rx_cb = fn->usb_rx_cb;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

