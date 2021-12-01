/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Generic media access Layer.
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

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED 
#pragma     data_alignment = 4 
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_vcp.h"
#include "usb_conf.h"
#include "usbd_cdc_core.h"
#include "usb_main.h"
#include <string.h>
//#include "Misc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LINE_CODING linecoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };


//UART_InitTypeDef USART_InitStructure;

/* These are external variables imported from CDC core to be used for IN 
   transfer management. */

//extern volatile uint32_t APP_Rx_ptr_in;    
                                  /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */

/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init     (void);
static uint16_t VCP_DeInit   (void);
static uint16_t VCP_Ctrl     (uint32_t Cmd, uint8_t* Buf, uint32_t Len);
//static uint16_t VCP_DataTx   (uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx   (uint8_t* Buf, uint32_t Len);

static uint16_t VCP_COMConfig(uint8_t Conf);

CDC_IF_Prop_TypeDef VCP_fops = 
{
  VCP_Init,
  VCP_DeInit,
  VCP_Ctrl,
  VCP_DataTx,
  VCP_DataRx
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
//    NVIC_InitTypeDef NVIC_InitStructure;

    /**/
//#ifdef USER_SPECIFIED_DATA_SOURCE
//    APP_Rx_Buffer = COM_Rx_data_buf;
//#else
//    APP_Gdata_param.COM_config_cmp = 0;
//#endif
//    memset(&APP_Gdata_param, 0, sizeof(APP_Gdata_param));
    return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{

  return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */

static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t *Buf, uint32_t Len)
{
    switch (Cmd)
    {
    case SEND_ENCAPSULATED_COMMAND:
        /* Not  needed for this driver */
        break;

    case GET_ENCAPSULATED_RESPONSE:
        /* Not  needed for this driver */
        break;

    case SET_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

    case GET_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

    case CLEAR_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

    case SET_LINE_CODING:
				
        linecoding.bitrate = (uint32_t)(Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
        linecoding.format = Buf[4];
        linecoding.paritytype = Buf[5];
        linecoding.datatype = Buf[6];
//        APP_Gdata_param.COM_config_cmp = 1;
        /* Set the new configuration */
        VCP_COMConfig(OTHER_CONFIG);
	
//		set_line_code+=1;
//		HWRITE(0x4ff0,set_line_code);

        break;

    case GET_LINE_CODING:
        Buf[0] = (uint8_t)(linecoding.bitrate);
        Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
        Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
        Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
        Buf[4] = linecoding.format;
        Buf[5] = linecoding.paritytype;
        Buf[6] = linecoding.datatype;
		    
        break;

    case SET_CONTROL_LINE_STATE:
        /* Not  needed for this driver */
        break;

    case SEND_BREAK:
        /* Not  needed for this driver */
        break;

    default:
        break;
    }

    return USBD_OK;
}

int isUsbBufFull()
{
	int ret = 0;
//	NVIC_DisableIRQ(USB_IRQn);
//	ret = APP_Gdata_param.rx_structure.Rx_counter >= CDC_APP_RX_DATA_SIZE;
//	NVIC_EnableIRQ(USB_IRQn);
	return ret;
}
/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in 
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
uint16_t VCP_DataTx(uint8_t *Buf, uint32_t Len)
{

    return USBD_OK;
}

/**
  * @brief  Get_TxBuf_length
  *         Get the length of the remaining data to be transmitted    
  * @param  NONE
  * @retval Result receive data length
  */
uint32_t VCP_GetTxBuflen(void)
{
	uint32_t ret = 0x00;

    return ret;
}

/**
  * @brief  Get_RxBuf_rsaddr
  *         Get reading receive data starting position.    
  * @param  NONE
  * @retval Result received data is read starting position
  */
uint32_t VCP_GetTxBufrsaddr(void)
{
    return 0;
}

/**
  * @brief  Get_RxData
  *         Get receive data by byte     
  * @param  NONE
  * @retval Result receive data 
  */
int32_t VCP_GetRxChar(void)
{
    /* 没有接收到新数据 */
	int ret = -1;
	return ret;
}

/**
  * @brief  Get_RxBuf_length
  *         Get receive data length     
  * @param  NONE
  * @retval Result receive data length
  */
uint32_t VCP_GetRxBuflen(void)
{
    return 0;
}

/**
  * @brief  Get_RxBuf_rsaddr
  *         Get reading receive data starting position.    
  * @param  NONE
  * @retval Result received data is read starting position
  */
int8_t * VCP_GetRxBufrsaddr(void)
{
    return NULL;
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataRx (uint8_t* Buf, uint32_t Len)
{
	
    return USBD_OK;
}



/**
  * @brief  VCP_COMConfig
  *         Configure the COM Port with default values or values received from host.
  * @param  Conf: can be DEFAULT_CONFIG to set the default configuration or OTHER_CONFIG
  *         to set a configuration received from the host.
  * @retval None.
  */
static uint16_t VCP_COMConfig(uint8_t Conf)
{

  return USBD_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
