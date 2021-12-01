/**
  ******************************************************************************
  * @file    usbd_cdc_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                CDC Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class

  *           @note
  *             For the Abstract Control Model, this core allows only transmitting the requests to
  *             lower layer dispatcher (ie. usbd_cdc_vcp.c/.h) which should manage each request and
  *             perform relative actions.
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
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
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usb_dcd_int.h"
#include "private.h"

/*********************************************
   CDC Device library callbacks
 *********************************************/
static uint8_t  usbd_cdc_Init        (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_cdc_DeInit      (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_cdc_Setup       (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_cdc_EP0_RxReady  (void *pdev);
static uint8_t  usbd_cdc_DataIn      (void *pdev, uint8_t epnum);
static uint8_t  usbd_cdc_DataOut     (void *pdev, uint8_t epnum);
//static uint8_t  usbd_cdc_SOF         (void *pdev);

/*********************************************
   CDC specific management functions
 *********************************************/
//static void Handle_USBAsynchXfer  (void *pdev);
static uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length);
#ifdef USE_USB_OTG_HS  
static uint8_t  *USBD_cdc_GetOtherCfgDesc (uint8_t speed, uint16_t *length);
#endif
/**
  * @}
  */ 

/** @defgroup usbd_cdc_Private_Variables
  * @{
  */ 
extern uint8_t USBD_DeviceDesc   [USB_SIZ_DEVICE_DESC];


static uint32_t  usbd_cdc_AltSet  = 0;

uint8_t USB_Rx_Buffer[CDC_DATA_MAX_PACKET_SIZE];




uint8_t CmdBuff[CDC_CMD_PACKET_SZE] ;

//volatile uint32_t APP_Rx_ptr_in  = 0;
//volatile uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;

uint8_t  USB_Tx_State = 0;

 uint32_t cdcCmd = 0xFF;
 uint32_t cdcLen = 0;
 
static USBD_FN_CB_T theCustomCDC = {
    .usb_txdone = NULL,
    .usb_rx_cb = NULL,
};

/* CDC interface class callbacks structure */
 USBD_Class_cb_TypeDef    USBD_CDC_cb = 
{
  usbd_cdc_Init,
  usbd_cdc_DeInit,
  usbd_cdc_Setup,
  NULL,                 /* EP0_TxSent, */
  usbd_cdc_EP0_RxReady,
  usbd_cdc_DataIn,
  usbd_cdc_DataOut,
  //usbd_cdc_SOF,
	NULL,
  NULL,
  NULL,     
  USBD_cdc_GetCfgDesc,
#ifdef USE_USB_OTG_HS   
  USBD_cdc_GetOtherCfgDesc, /* use same cobfig as per FS */
#endif /* USE_USB_OTG_HS  */
};

uint8_t usbd_cdc_CfgDesc[] ={
    0x09,0x02,0x43,0x00,0x02,0x01,0x00,0xa0,0x32,             //配置描述符（两个接口）
	//以下为接口0（CDC接口）描述符	
    0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,0x00,             //CDC接口描述符(一个端点)
	//以下为功能描述符
    0x05,0x24,0x00,0x10,0x01,                                 //功能描述符(头)
	0x05,0x24,0x01,0x00,0x00,                                 //管理描述符(没有数据类接口) 03 01
	0x04,0x24,0x02,0x02,                                      //支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding、Serial_State	
	0x05,0x24,0x06,0x00,0x01,                                 //编号为0的CDC接口;编号1的数据类接口
	0x07,0x05,0x81,0x03,0x08,0x00,0xFF,                       //中断上传端点描述符
	//以下为接口1（数据接口）描述符
	0x09,0x04,0x01,0x00,0x02,0x0a,0x00,0x00,0x00,             //数据接口描述符
    0x07,0x05,0x02,0x02,0x40,0x00,0x00,                       //端点描述符	
	0x07,0x05,0x82,0x02,0x40,0x00,0x00,                       //端点描述符
};

/**
  * @}
  */ 

/** @defgroup usbd_cdc_Private_Functions
  * @{
  */ 

/**
  * @brief  usbd_cdc_Init
  *         Initilaize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */

static uint8_t  usbd_cdc_Init (void  *pdev, 
                               uint8_t cfgidx)
{

	return USBD_OK;
}

/**
  * @brief  usbd_cdc_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_cdc_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{

  return USBD_OK;
}
uint8_t cdc_linecode[8] = {0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x08};
//extern word usb_getword();
static uint8_t  usbd_cdc_Setup(void  *pdev,
                               USB_SETUP_REQ *req)
{
    uint16_t len = 0;
    uint8_t  *pbuf = NULL;
    uint8_t cdc_buffer[8] = {0};
//	theBT.usb_init_ok = 1;
//    uint16_t rx_ep_cdc;
//    uint8_t  ep_num_cdc;
//    uint8_t rx_DataLength_cdc;
//	DEBUG(("usbd_cdc_Setup req->bmRequest & USB_REQ_TYPE_MASK = %x req->wLength = %x req->bRequest = %x\r\n",req->bmRequest & USB_REQ_TYPE_MASK, req->wLength,req->bRequest));

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    /* CDC Class Requests -------------------------------*/
    case USB_REQ_TYPE_CLASS :
        /* Check if the request is a data setup packet */
//        if (req->wLength)
//        {
//            /* Check if the request is Device-to-Host */
//            if (req->bmRequest & 0x80)
//            {
//                /* Get the data to be sent to Host from interface layer */
//                APP_FOPS.pIf_Ctrl(req->bRequest, CmdBuff, req->wLength);

//                /* Send the data to the host */
//                USBD_CtlSendData(pdev,
//                                 CmdBuff,
//                                 req->wLength);
//            }
//            else /* Host-to-Device requeset */
//            {
//                /* Set the value of the current command to be processed */
//                cdcCmd = req->bRequest;
//                cdcLen = req->wLength;
//				APP_FOPS.pIf_Ctrl(req->bRequest, CmdBuff, req->wLength);
//				USBD_CtlPrepareRx (pdev,
//									CmdBuff,
//									req->wLength);    
////								USB_OTG_EPReply_Zerolen(pdev, 0);
//                /* Prepare the reception of the buffer over EP0
//                Next step: the received data will be managed in usbd_cdc_EP0_TxSent()
//                function. */
//							if(req->wLength!=0)
//							{
//								USBD_CtlSendStatus(pdev);
//							}
//            }
//        }
//        else /* No Data request */
        {
            /* Transfer the command to the interface layer */
//            APP_FOPS.pIf_Ctrl(req->bRequest, NULL, 0);
			switch(req->bRequest)
			{
				case SET_LINE_CODING:
					for(int i=0;i<30000;i++)
					{}
				  
					USBHWRITE(core_usb_status, 0x20); 
					usb_getword();
//					ep_num_cdc = (uint8_t)(rx_ep_cdc >> 12);
//					rx_DataLength_cdc = rx_ep_cdc & 0xfff;
					USB_OTG_ReadPacket(pdev, cdc_buffer, 0, 7);
					USBHWRITE(core_usb_trig, 0x10);
//					OS_EXIT_CRITICAL();
					break;
				case GET_LINE_CODING:
					USBD_CtlSendData(pdev, cdc_linecode, 7);
					break;
				case SET_CONTROL_LINE_STATE:
//					USBD_CtlSendData(pdev,0,0);
					break;
				default :
//					USBD_CtlSendData(pdev,0,0);
					break;
				
			}
        }

        return USBD_OK;

    /* Standard Requests -------------------------------*/
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:
            if ((req->wValue >> 8) == CDC_DESCRIPTOR_TYPE)
            {

//                pbuf = usbd_cdc_CfgDesc + 9 + (9 * USBD_ITF_MAX_NUM);

//                len = MIN((sizeof(usbd_cdc_CfgDesc) - 9), req->wLength);
				
                pbuf = usbd_cdc_CfgDesc ;

                len = sizeof(usbd_cdc_CfgDesc);
            }

            USBD_CtlSendData(pdev,
                             pbuf,
                             len);
            break;

        case USB_REQ_GET_INTERFACE :
            USBD_CtlSendData(pdev,
                             (uint8_t *)&usbd_cdc_AltSet,
                             1);
            break;

        case USB_REQ_SET_INTERFACE :
            if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
            {
                usbd_cdc_AltSet = (uint8_t)(req->wValue);
            }
            else
            {
                /* Call the error management function (command will be nacked */
                USBD_CtlError(pdev, req);
            }
            break;
        }
		default:
			USBD_CtlError(pdev, req);
        return USBD_FAIL;
    }
}

/**
  * @brief  usbd_cdc_EP0_RxReady
  *         Data received on control endpoint
  * @param  pdev: device device instance
  * @retval status
  */
static uint8_t  usbd_cdc_EP0_RxReady (void  *pdev)
{ 
  if (cdcCmd != NO_CMD)
  {

    
    /* Reset the command variable to default value */
    cdcCmd = NO_CMD;
  }
  
  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_DataIn (void *pdev, uint8_t epnum)
{
	theCustomCDC.usb_txdone();
    return USBD_OK;
}

/**
  * @brief  usbd_cdc_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
#define HID_MAX_PACKET_SIZE 64
uint8_t CDC_Recv_Flag;
uint8_t receive_data_flag;
uint8_t HidOut_Data_Buff[HID_MAX_PACKET_SIZE];
static uint8_t  usbd_cdc_DataOut (void *pdev, uint8_t epnum)
{      
    uint16_t USB_Rx_Cnt;

    /* Get the received data buffer and update the counter */

    /* USB data will be immediately processed, this allow next USB traffic being
       NAKed till the end of the application Xfer */
    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev,
                     CDC_OUT_EP,
                     (uint8_t *)(USB_Rx_Buffer),
                     CDC_DATA_OUT_PACKET_SIZE);
    
    USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE *)pdev)->dev.out_ep[epnum].xfer_count;
//	DEBUG(("usbd_cdc_DataOut USB_Rx_Cnt = %d\r\n",USB_Rx_Cnt));
	theCustomCDC.usb_rx_cb(USB_Rx_Buffer,USB_Rx_Cnt);

    return USBD_OK;
}

/**
  * @brief  usbd_audio_SOF
  *         Start Of Frame event management
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
//static uint8_t  usbd_cdc_SOF (void *pdev)
//{      
//  static uint32_t FrameCount = 0;
//  
//  if (FrameCount++ == CDC_IN_FRAME_INTERVAL)
//  {
//    /* Reset the frame counter */
//    FrameCount = 0;
//    
//    /* Check the data to be sent through IN pipe */
//    Handle_USBAsynchXfer(pdev);
//  }
//  
//  return USBD_OK;
//}

/**
  * @brief  Handle_USBAsynchXfer
  *         Send data to USB
  * @param  pdev: instance
  * @retval None
  */
//static void Handle_USBAsynchXfer (void *pdev)
//{
//  uint16_t USB_Tx_ptr;
//  uint16_t USB_Tx_length;
//  
//  if(USB_Tx_State != 1)
//  {
//    if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out == CDC_APP_RX_DATA_SIZE)
//    {
//      APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;
//    }
//	#if 1
//	if (!APP_Gdata_param.rx_structure.Rx_counter)
//	{
//		USB_Tx_State = 0;
//        return;
//	}
//	APP_Rx_length = APP_Gdata_param.rx_structure.Rx_counter;
//	#else
//    /* */
////    if (!APP_Gdata_param.rx_structure.Rx_counter)
//	if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out == \
//		APP_Gdata_param.rx_structure.APP_Rx_ptr_in)
//    {
//        USB_Tx_State = 0;
//        return;
//    }
////    APP_Rx_length = APP_Gdata_param.rx_structure.Rx_counter;
////	APP_Rx_length = APP_Gdata_param.rx_structure.APP_Rx_ptr_in - \
////					APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
////    if(APP_Gdata_param.rx_structure.APP_Rx_ptr_out == APP_Gdata_param.rx_structure.APP_Rx_ptr_in) 
////    {
////      USB_Tx_State = 0;
////      return;
////    }
//    
//    if(APP_Gdata_param.rx_structure.APP_Rx_ptr_out > APP_Gdata_param.rx_structure.APP_Rx_ptr_in) /* rollback */
//    { 
//      APP_Rx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
//    }
//    else 
//    {
//      APP_Rx_length = APP_Gdata_param.rx_structure.APP_Rx_ptr_in - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
//    }
//	#endif
//#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
//     APP_Rx_length &= ~0x03;
//#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
//    
//    if (APP_Rx_length > CDC_DATA_IN_PACKET_SIZE)
//    {
//        USB_Tx_ptr = APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
//        if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out + CDC_DATA_IN_PACKET_SIZE >= CDC_APP_RX_DATA_SIZE)
//        {
//            USB_Tx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
//            APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;	
//            APP_Rx_length -= USB_Tx_length;
//            APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
//        }else{
//            USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
//            APP_Gdata_param.rx_structure.APP_Rx_ptr_out += CDC_DATA_IN_PACKET_SIZE;	
//            APP_Rx_length -= CDC_DATA_IN_PACKET_SIZE;
//            APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
//        }
//    }
//    else
//    {
//        USB_Tx_ptr = APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
//        if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out + APP_Rx_length >= CDC_APP_RX_DATA_SIZE)
//        {
//            USB_Tx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
//            APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;	
//            APP_Rx_length -= USB_Tx_length;
//            APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
//        }else{
//            USB_Tx_length = APP_Rx_length;
//            APP_Gdata_param.rx_structure.APP_Rx_ptr_out += APP_Rx_length;
//            APP_Rx_length = 0;
//            APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
//        }
//    }
//    USB_Tx_State = 1; 

//    DCD_EP_Tx (pdev,
//               CDC_IN_EP,
//               (uint8_t*)&APP_Gdata_param.rx_structure.APP_Rx_Buffer[USB_Tx_ptr],
//               USB_Tx_length);
//  }  
//  
//}

void USBD_CDC_RegisterCB( USBD_FN_CB_T *fn )
{
	theCustomCDC.usb_txdone = fn->usb_txdone;
	theCustomCDC.usb_rx_cb = fn->usb_rx_cb;
}

/**
  * @brief  USBD_cdc_GetCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_cdc_CfgDesc);
  return usbd_cdc_CfgDesc;
}

/**
  * @brief  USBD_cdc_GetCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
#ifdef USE_USB_OTG_HS 
static uint8_t  *USBD_cdc_GetOtherCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_cdc_OtherCfgDesc);
  return usbd_cdc_OtherCfgDesc;
}
#endif
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
