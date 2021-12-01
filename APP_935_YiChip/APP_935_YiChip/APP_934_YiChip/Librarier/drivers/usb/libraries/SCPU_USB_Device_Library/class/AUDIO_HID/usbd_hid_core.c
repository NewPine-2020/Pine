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
#include "usbd_audio_core.h"
#include "usbd_audio_out_if.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usb_defines.h"
#include "yc11xx.h"

uint8_t  Audiovolmute[2];
uint8_t AudioVolumeMax[2]={0xff,0x80};
uint8_t AudioVolumeMin[2]={0x00,0x80};
uint8_t AudioVolumeRes[2]={0x01,0x00};
uint8_t AudioCtl[64] = {0xFF,0x80,00};
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

static void  AUDIO_Req_GetVolumeMax(void *pdev, USB_SETUP_REQ *req)
{
	USBD_CtlSendData (pdev, 
                    AudioVolumeMax,
                    req->wLength);

}
static void  AUDIO_Req_GetVolumeMin(void *pdev, USB_SETUP_REQ *req)
{uint8_t audiomin[2]= {0x00,0x80};
//	USBD_CtlSendData (pdev, 
  //                  AudioVolumeMin,
  //                  req->wLength);
                    USBD_CtlSendData (pdev, 
                    audiomin,
                    2);

}

static void  AUDIO_Req_GetVolumeRes(void *pdev, USB_SETUP_REQ *req)
{
	//DEBUG_LOG_STRING("AUDIO_Req_GetVolumeRes          :   %d   %d \r\n",AudioVolumeRes[0],AudioVolumeRes[1]);
	uint8_t audiores[2]= {0x01,0x00};
	//USBD_CtlSendData (pdev, 
         //AudioVolumeRes,
          //          req->wLength);
                    USBD_CtlSendData (pdev, 
                    audiores,
                    2);

}

uint8_t gAudioStart=0;

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
static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE];

static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE] =
{
  /* Configuration 1 */
//  0x09,                                 /* bLength */
//  USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
//  LOBYTE(AUDIO_CONFIG_DESC_SIZE),       /* wTotalLength  141 bytes*/
//  HIBYTE(AUDIO_CONFIG_DESC_SIZE),      
//  0x04,                                 /* bNumInterfaces */
//  0x01,                                 /* bConfigurationValue */
//  0x00,                                 /* iConfiguration */
//  0xC0,                                 /* bmAttributes  BUS Powred*/
//  0x32,                                 /* bMaxPower = 100 mA*/
//  /* 09 byte*/
//  0x09,
//  0x04,
//  0x00,
//  0x00,
//  0x01,
//  0xff,
//  0x00,
//  0x00,
//  0x04,

//  0x07,
//  0x05,
//  0x01,
//  0x02,
//  0x40,
//  0x00,
//  0x00,

//  0x09,
//  0x04,
//  0x01,
//  0x00,
//  0x01,
//  0x03,
//  0x00,
//  0x00,
//  0x00,

//  0x09,
//  0x21,
//  0x10,
//  0x01,
//  0x00,
//  0x01,
//  0x22,
//  0xB0,
//  0x00,

//  0x07,
//  0x05,
//  0x81,
//  0x03,
//  0x40,
//  0x00,
//  0x09,
//    
//  /* USB Speaker Standard interface descriptor */
//  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
//  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  0x02,                                 /* bInterfaceNumber */
//  0x00,                                 /* bAlternateSetting */
//  0x00,                                 /* bNumEndpoints */
//  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
//  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
//  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
//  0x00,                                 /* iInterface */
//  /* 09 byte*/
//  
//  /* USB Speaker Class-specific AC Interface Descriptor */
//  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
//  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
//  0x00,          /* 1.00 */             /* bcdADC */
//  0x01,
//  0x1E,                                 /* wTotalLength = 39*/
//  0x00,
//  0x01,                                 /* bInCollection */
//  0x03,                                 /* baInterfaceNr */
//  /* 09 byte*/
//  
//  /* USB Speaker Input Terminal Descriptor */
//  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
//  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
//  0x01,                                 /* bTerminalID */
//  0x01,                                 /* wTerminalType Microphone   0x0201 */
//  0x02,
//  0x00,                                 /* bAssocTerminal */
//  0x01,                                 /* bNrChannels */
//  0x00,                                 /* wChannelConfig 0x0000  Mono */
//  0x00,
//  0x00,                                 /* iChannelNames */
//  0x00,                                 /* iTerminal */
//  /* 12 byte*/


// /*USB Speaker Output Terminal Descriptor */
//  0x09,      /* bLength */
//  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
//  0x03,                                 /* bTerminalID */
//  0x01,                                 /* wTerminalType  0x0101*/
//  0x01,
//  0x00,                                 /* bAssocTerminal */
//  0x01,                                 /* bSourceID */
//  0x00,                                 /* iTerminal */

//  /* 09 byte*/
//  
//  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
//  /* Interface 1, Alternate Setting 0                                             */
//  AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
//  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  0x03,                                 /* bInterfaceNumber */
//  0x00,                                 /* bAlternateSetting */
//  0x00,                                 /* bNumEndpoints */
//  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
//  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
//  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
//  0x00,                                 /* iInterface */
//  /* 09 byte*/
//  
//  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
//  /* Interface 1, Alternate Setting 1                                           */
//  AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
//  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
//  0x03,                                 /* bInterfaceNumber */
//  0x01,                                 /* bAlternateSetting */
//  0x01,                                 /* bNumEndpoints */
//  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
//  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
//  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
//  0x00,                                 /* iInterface */
//  /* 09 byte*/
//  
//  /* USB Speaker Audio Streaming Interface Descriptor */
//  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
//  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
//  0x03,                                 /* bTerminalLink */
//  0x01,                                 /* bDelay */
//  0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
//  0x00,
//  /* 07 byte*/
//  
//  /* USB Speaker Audio Type III Format Interface Descriptor */
//  0x0B,                                 /* bLength */
//  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
//  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
//  AUDIO_FORMAT_TYPE_I,                /* bFormatType */ 
//  0x01,                                 /* bNrChannels */
//  0x02,                                 /* bSubFrameSize :  2 Bytes per frame (16bits) */
//  16,                                   /* bBitResolution (16-bits per sample) */ 
//  0x01,                                 /* bSamFreqType only one frequency supported */ 
//  SAMPLE_FREQ(USBD_AUDIO_FREQ),         /* Audio sampling frequency coded on 3 bytes */
//  /* 11 byte*/
//  
//  /* Endpoint 2 Descriptor */
//  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
//  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
//  AUDIO_IN_EP,                         /* bEndpointAddress 1 out endpoint*/
//  USB_ENDPOINT_TYPE_ISOCHRONOUS,        /* bmAttributes */
//  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
//  0x01,                                 /* bInterval */
//  0x00,                                 /* bRefresh */
//  0x00,                                 /* bSynchAddress */
//  /* 09 byte*/
//  
//  /* Endpoint - Audio Streaming Descriptor*/
//  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
//  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
//  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
//  0x01,                                 /* bmAttributes */
//  0x01,                                 /* bLockDelayUnits */
//  0x01,                                 /* wLockDelay */
//  0x00,
/////////////////////////////
 0x09,0x02,0xDA,0x00,0x04,0x01,0x00,0x80,0xC8,0x09,0x04,0x00,0x00,0x00,
0x01,0x01,0x00,0x00,0x0A,0x24,0x01,0x00,0x01,0x47,0x00,0x02,0x01,0x02,
0x0C,0x24,0x02,0x01,0x01,0x01,0x00,0x02,0x03,0x00,0x00,0x00,0x0A,0x24,
0x06,0x09,0x01,0x01,0x01,0x02,0x02,0x00,0x09,0x24,0x03,0x03,0x01,0x03,
0x00,0x09,0x00,0x0C,0x24,0x02,0x02,0x01,0x02,0x00,0x01,0x01,0x00,0x00,
0x00,0x09,0x24,0x06,0x0A,0x02,0x01,0x03,0x00,0x00,0x09,0x24,0x03,0x04,
0x01,0x01,0x00,0x0A,0x00,0x09,0x04,AUDIO_OUT_INTERFACENUM,0x00,0x00,0x01,0x02,0x00,0x00,
0x09,0x04,AUDIO_OUT_INTERFACENUM,0x01,0x01,0x01,0x02,0x00,0x00,0x07,0x24,0x01,0x01,0x01,
0x01,0x00,0x0B,0x24,0x02,0x01,0x02,0x02,0x10,0x01,0x80,0xBB,0x00,0x09,
0x05,0x02,0x09,0xC0,0x00,0x01,0x00,0x00,0x07,0x25,0x01,0x00,0x00,0x00,
0x00,0x09,0x04,AUDIO_IN_INTERFACENUM,0x00,0x00,0x01,0x02,0x00,0x00,0x09,0x04,AUDIO_IN_INTERFACENUM,0x01,
0x01,0x01,0x02,0x00,0x00,0x07,0x24,0x01,0x04,0x01,0x01,0x00,0x0B,0x24,
0x02,0x01,0x01,0x02,0x10,0x01,0x80,0xBB,0x00,0x09,0x05,0x82,0x09,0x60,
0x00,0x01,0x00,0x00,0x07,0x25,0x01,0x00,0x00,0x00,0x00,0x09,0x04,HID_IN_INTERFACENUM,
0x00,0x01,0x03,0x00,0x00,0x00,0x09,0x21,0x01,0x02,0x00,0x01,0x22,HID_MOUSE_REPORT_DESC_SIZE,
0x00,0x07,0x05,0x81,0x03,0x08,0x00,0x01

} ;

//__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
//{
//0x05,0x01,0x09,0x80,0xA1,0x01,0x85,0x01,0x19,0x01,0x29,0xB7,0x15,0x01,
//0x26,0xB7,0x00,0x95,0x01,0x75,0x08,0x81,0x00,0xC0,0x05,0x0C,0x09,0x01,
//0xA1,0x01,0x85,0x02,0x19,0x00,0x2A,0x14,0x05,0x15,0x00,0x26,0x14,0x05,
//0x95,0x01,0x75,0x10,0x81,0x00,0xC0,0x05,0x01,0x09,0x02,0xA1,0x01,0x85,
//0x03,0x09,0x01,0xA1,0x00,0x05,0x09,0x19,0x01,0x29,0x08,0x15,0x00,0x25,
//0x01,0x95,0x08,0x75,0x01,0x81,0x02,0x05,0x01,0x09,0x30,0x09,0x31,0x15,
//0x81,0x25,0x7F,0x75,0x08,0x95,0x02,0x81,0x06,0x09,0x38,0x15,0x81,0x25,
//0x7F,0x75,0x08,0x95,0x01,0x81,0x06,0xC0,0xC0,0x05,0x01,0x09,0x06,0xA1,
//0x01,0x85,0x04,0x05,0x07,0x19,0xE0,0x29,0xE7,0x15,0x00,0x25,0x01,0x75,
//0x01,0x95,0x08,0x81,0x02,0x95,0x03,0x75,0x01,0x05,0x08,0x19,0x01,0x29,
//0x03,0x91,0x02,0x95,0x05,0x75,0x01,0x91,0x01,0x95,0x06,0x75,0x08,0x15,
//0x00,0x26,0xFF,0x00,0x05,0x07,0x19,0x00,0x2A,0xFF,0x00,0x81,0x00,0x09,
//0x00,0x75,0x08,0x95,0x14,0xB1,0x00,0xC0,
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
    0xc0        //End Collection

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
//__ALIGN_BEGIN static uint8_t HID_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
//{
//0x05,0x0C,0x09,0x01,0xA1,0x01,0x85,0x01,0x15,0x00,0x25,0x01,0x75,0x01,
//0x95,0x01,0x09,0xE9,0x81,0x02,0x09,0xEA,0x81,0x02,0x09,0xCD,0x81,0x02,
//0x09,0xB5,0x81,0x02,0x09,0xB6,0x81,0x02,0x09,0xB7,0x81,0x02,0x09,0xB3,
//0x81,0x02,0x09,0xB4,0x81,0x02,0x05,0x0B,0x09,0x24,0x81,0x02,0x09,0x20,
//0x81,0x02,0x09,0x2F,0x81,0x06,0x95,0x05,0x81,0x01,0xC0,                                                 
//};
__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZ]  __ALIGN_END =
{
    /*Configuration Descriptor*/
    0x09,   /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
    USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered */
    0x32,   /* MaxPower 0 mA */

    /*---------------------------------------------------------------------------*/

    /*Interface Descriptor */
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */

    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,

    /*Call Management Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x00,   /* bDataInterface: 0 */
//  0x01,   /* bDataInterface: 1 */

    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */

    /*Endpoint 2 Descriptor*/
    0x07,                           /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                     /* bEndpointAddress */
    0x03,                           /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SZE),
#ifdef USE_USB_OTG_HS
    0x10,                           /* bInterval: */
#else
    0xFF,                           /* bInterval: */
#endif /* USE_USB_OTG_HS */

    /*---------------------------------------------------------------------------*/

    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00,                              /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_IN_EP,                         /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00                               /* bInterval: ignore for Bulk transfer */
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


/******************************************************************************
     AUDIO Class requests management
******************************************************************************/


//uint8_t  AudioCtl[64];
//uint8_t  AudioCtl[64] = {0x80,0x3E,00};
uint8_t  AudioCtlCmd = 0;
uint32_t AudioCtlLen = 0;
uint8_t  AudioCtlUnit = 0;

/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *rrq)
{  
  /* Send the current mute state */
	
	
  USBD_CtlSendData (pdev, 
                    AudioCtl,
                    rrq->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
 
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req)
{ 
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
     USBHWRITE(core_usb_trig,0x10);
    /* Set the global variables indicating current request and its length 
    to the function usbd_audio_EP0_RxReady() which will process the request */
    AudioCtlCmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    AudioCtlLen = req->wLength;          /* Set the request data length */
    AudioCtlUnit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}


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

extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
static uint8_t  USBD_HID_Setup(void  *pdev,
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
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_Protocol,
                             1);
            break;

        case HID_REQ_SET_IDLE:
            USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
            USB_OTG_dev.dev.device_status_new = USB_OTG_END;
            break;

        case HID_REQ_GET_IDLE:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_IdleState,
                             1);
            break;
        case HID_REQ_SET_REPORT:
            USBD_HID_Protocol = (uint8_t)(req->wValue);

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
                len = MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
                pbuf = HID_MOUSE_ReportDesc;
            }
            else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
            {

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
                pbuf = USBD_HID_Desc;
#else
                pbuf = USBD_HID_CfgDesc + 0x12;
#endif
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
		*length = sizeof(USBD_HID_CfgDesc);
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

			//printf("333333\n");
	
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
