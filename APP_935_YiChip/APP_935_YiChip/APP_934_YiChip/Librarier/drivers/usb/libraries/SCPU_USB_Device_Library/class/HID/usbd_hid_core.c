/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick)
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);
uint8_t USBD_HID_DataOut(void *pdev,uint8_t epnum);
/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif  
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */        
__ALIGN_BEGIN static uint32_t  USBD_HID_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */      
__ALIGN_BEGIN static uint32_t  USBD_HID_Protocol  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint32_t  USBD_HID_IdleState __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */ 
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
	#if 0
  0x09, /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x01,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_IN_PACKET, /*wMaxPacketSize: 4 Byte max */
  0x00,
  0x0A,          /*bInterval: Polling Interval (10 ms)*/
  /* 34 */
	#else 
	#if 0
	  0x09, /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/     //???a¨¢??????¨²¨ºy¡ê?2?¡ã¨¹¨¤¡§0???¨²
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/   //¨¦¨¨??3¨¦0
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/    //¨¦¨¨??3¨¦0
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_IN_PACKET, /*wMaxPacketSize: 4 Byte max */
  0x00,
  0x01,          /*bInterval: Polling Interval (10 ms)*/
  /* 34 */

	/*************************¨¬¨ª?¨®¨º?3???¦Ì?*******************************/
	0x07, 
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	HID_OUT_EP,
	0x03,
	HID_OUT_PACKET,
	0x00,
	0x01,
	#endif
	
 /* 41 */
	#if 1
	0x09, 					//Length = 9
	USB_CONFIGURATION_DESCRIPTOR_TYPE, 		//DescriptorType = Configuration
	USB_HID_CONFIG_DESC_SIZ, 0x00, 				//TotalLength
	0x01, 					//NumInterfaces = 1
	0x01, 					//ConfigurationValue = 1
	0x00, 					//iConfiguration string index(Non)
	0x80, 					//SelfPower = 0; RemoteWakeup = 0
	0x32,					//MaxPower = 100mA

    //interface desc
    0x09, 					//Length = 9
	USB_INTERFACE_DESCRIPTOR_TYPE, 			//DescriptorType = Interface
	0x00, 					//InerfaceNumber = 0
	0x00, 					//AlternateSetting = 0
	0x02, 					//NumEndpoint = 2(bulk-IN, bulk-OUT)
	0x03,					//Class = Human Interface Device
	0x00,					//InterfaceSubClass = 0x00(No subclass) 		
	0x00,					//InterfaceProtocol = 0x00(None) 
	0x00,					//iInterface string index(Non)

	//HID descriptor
	0x09,0x21,			//Length, Type
	0x11,0x01,		    //HID Class Specification compliance ?0x10 0x01
	0x00,				//Country localization (=none)
	0x01,				//number of descriptors to follow
	0x22,				//And it's a Report descriptor
	HID_MOUSE_REPORT_DESC_SIZE,0x00,			//Report descriptor length 

    // Endpoint desc
    0x07, 					//Length = 7
	USB_ENDPOINT_DESCRIPTOR_TYPE, 			//DescriptorType = Endpoint		
	HID_IN_EP, 					//In; Ep1
	0x03, 					//Endpoint type = interrupt
	HID_IN_PACKET, 0x00, 			//MaxPacketSize = 64->16
	0x01, //0x0a,					//Poll
    
    0x07, 					//Length = 7
	USB_ENDPOINT_DESCRIPTOR_TYPE, 			//DescriptorType = Endpoint		
	HID_OUT_EP, 					//Out; Ep2
	0x03, 					//Endpoint type = interrupt
	HID_OUT_PACKET, 0x00, 			//MaxPacketSize = 64->16
	0x01, //0x0a,	 DT??USB¦Ì¨¨¡äy¨º¡À??¡ê?¨¬¨¢¨¦yusb?????¨´?¨¨ @ 2018.08.28
	#endif
	#endif
} ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ] __ALIGN_END=
{
	#if 0
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  HID_MOUSE_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  #endif
  #if 1
  	0x09,0x21,			//Length, Type
	0x11,0x01,		    //HID Class Specification compliance ?0x10 0x01
	0x00,				//Country localization (=none)
	0x01,				//number of descriptors to follow
	0x22,				//And it's a Report descriptor
	HID_MOUSE_REPORT_DESC_SIZE,0x00,			//Report descriptor length 
	#endif
};
#endif


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
//__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
//{
//    0x05, 0x01, 		// Usage Page,(Generic Desktop),
//	0x09, 0x02, 		// Usage(Mouse),
//	0xa1, 0x01, 		// Collectiton(Application),
//	0x09, 0x01, 		// Usage(Pointer),
//	0xa1, 0x00, 		// Collection(Physical),
//	0x05, 0x09,			// Usage Page(Buttons), 
//	0x19, 0x01, 		// Usage Minimum(01),
//	0x29, 0x03,			// Usage Maximum(03),
//	0x15, 0x00, 		// Logical Minimum(0),
//	0x25, 0x01, 		// Logical Maximum(1),
//	0x95, 0x03, 		// Report Count(3),
//	0x75, 0x01, 		// Report Size(1),
//	0x81, 0x02, 		// Input(Data, Variable, Absolute),	;3 button bits
//	0x95, 0x01, 		// Report Count(1),
//	0x75, 0x05, 		// Report Size(5),
//	0x81, 0x03, 		// Input(Cnst, Var, Abs),
//	0x05, 0x01, 		// Usage Page(Generic Desktop),
//	0x09, 0x30, 		// Usage(X),
//	0x09, 0x31, 		// Usage(Y),
//	0x09, 0x38,			// Usage(Wheel),
//	0x15, 0x81, 		// Logical Minimum(-127),
//	0x25, 0x7f, 		// Logical Maximum(127),
//	0x75, 0x08, 		// Report Size(8),
//	0x95, 0x03, 		// Report Count(3),
//	0x81, 0x06, 		// Input(Data, Variable, Relative),	;2 position bytes(X & Y)
//	0xc0, 				// End Collection,
//	0xc0				// End Collection
//};
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
{
	#if 0
  0x05,   0x01,
  0x09,   0x02,
  0xA1,   0x01,
  0x09,   0x01,
  
  0xA1,   0x00,
  0x05,   0x09,
  0x19,   0x01,
  0x29,   0x03,
  
  0x15,   0x00,
  0x25,   0x01,
  0x95,   0x03,
  0x75,   0x01,
  
  0x81,   0x02,
  0x95,   0x01,
  0x75,   0x05,
  0x81,   0x01,
  
  0x05,   0x01,
  0x09,   0x30,
  0x09,   0x31,
  0x09,   0x38,
  
  0x15,   0x81,
  0x25,   0x7F,
  0x75,   0x08,
  0x95,   0x03,
  
  0x81,   0x06,
  0xC0,   0x09,
  0x3c,   0x05,
  0xff,   0x09,
  
  0x01,   0x15,
  0x00,   0x25,
  0x01,   0x75,
  0x01,   0x95,
  
  0x02,   0xb1,
  0x22,   0x75,
  0x06,   0x95,
  0x01,   0xb1,
  
  0x01,   0xc0
  #else
	#if 0
   0x05U, 0x01U, /* Usage Page (Vendor defined)*/
    0x09U, 0x00U, /* Usage (Vendor defined) */
    0xA1U, 0x01U, /* Collection (Application) */

    0x15U, 0x00U, /* logical Minimum (-128) */
    0x25U, 0xffU, /* logical Maximum (127) */
0x19U, 0x01U,
    0x29U, 0x08U, /* Report Size (8U) */
    0x95U, 0x08U, /* Report Count (8U) */
    0x75U, 0x08U, /* Input(Data, Variable, Absolute) */

    0x81U, 0x02U, /* Usage (Vendor defined) */
    0x19U, 0x01U, /* logical Minimum (-128) */
    0x29U, 0x08U, /* logical Maximum (127) */
    0x91U, 0x02U, /* Report Size (8U) */
   
    0xC0U,        /* end collection */
    #endif
	#if 0
	0x05, 0x01, //Usage Page: 0x06, 0x00, 0xFF->05 01
	0x09, 0x00, //Usage: Undefined 0x01->0x00
	0xa1, 0x01, //Collection 0x00->0x01
	0x15, 0x00, //Logical Minimum
	0x25, 0xFF,//Logical Maximum 0x26, 0xFF, 0x00->0x25 0xff
	0x19, 0x01, //Usage Minimum 
	0x29, 0x08, //Usage Maximum 0x01->0x08
	0x95, 0x08, //Report Count
	0x75, 0x08, //Report Size 
	0x81, 0x02, //Input (Data, Variable, Absolute,Buffered Bytes) 0x00->0x02
	0x19, 0x01, //Usage Minimum
	0x29, 0x08, //Usage Maximum 0x01->0x08
	0x91, 0x02, //Feature (Data, Variable, Absolute,Buffered Bytes)0xb1, 0x00->0x91, 0x02
	0xc0		//End Collection

    #endif
	#if 1
    0x05U, 0x81U, /* Usage Page (Vendor defined)*/
    0x09U, 0x82U, /* Usage (Vendor defined) */
    0xA1U, 0x01U, /* Collection (Application) */
    0x09U, 0x83U, /* Usage (Vendor defined) */

    0x09U, 0x84U, /* Usage (Vendor defined) */
    0x15U, 0x80U, /* logical Minimum (-128) */
    0x25U, 0x7FU, /* logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, HID_IN_PACKET, /* Report Count (8U) */
    0x81U, 0x02U, /* Input(Data, Variable, Absolute) */

    0x09U, 0x84U, /* Usage (Vendor defined) */
    0x15U, 0x80U, /* logical Minimum (-128) */
    0x25U, 0x7FU, /* logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, HID_OUT_PACKET, /* Report Count (8U) */
    0x91U, 0x02U, /* Input(Data, Variable, Absolute) */
    0xC0U,        /* end collection */
    #endif
  
  #endif
}; 
/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_IN_EP,
              HID_IN_PACKET,
              USB_OTG_EP_INT);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_OUT_EP,
              HID_OUT_PACKET,
              USB_OTG_EP_INT);
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_IN_EP);
  DCD_EP_Close (pdev , HID_OUT_EP);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */

 extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
       USB_OTG_dev.dev.device_status_new = USB_OTG_END;
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
        pbuf = HID_MOUSE_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = USBD_HID_Desc;   
#else
        pbuf = USBD_HID_CfgDesc + 0x12;
#endif 
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_HID_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_HID_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport     (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, HID_IN_EP, report, len);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  
 // DCD_EP_Flush(pdev, HID_IN_EP);
  return USBD_OK;
}

/**
  * @}
  */ 

uint8_t receive_data_flag;
uint8_t HidOut_Data_Buff[HID_MAX_PACKET_SIZE];

uint8_t USBD_HID_DataOut(void *pdev,
                                 uint8_t epnum)
{

			//MyPrintf("333333\n");
	
    DCD_EP_PrepareRx(pdev,
                                     HID_OUT_EP,
                                     (uint8_t *)&HidOut_Data_Buff[0],
                                     HID_EPOUT_SIZE);
     receive_data_flag =1;
    return USBD_OK;
}


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
