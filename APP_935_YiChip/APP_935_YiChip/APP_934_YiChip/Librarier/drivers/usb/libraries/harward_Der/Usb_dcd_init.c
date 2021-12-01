/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_dcd_int.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Peripheral Device interrupt subroutines.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usb_dcd_int.h"
#include "usb_defines.h"
#include "usbd_desc.h"
#include "usbd_usr.h"
#include "usb_main.h"
#include "private.h"
#include "yc_debug.h"
//#include "systick.h"
//#include "usb_std.h"
/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_DCD_INT
* @brief This file contains the interrupt subroutines for the Device mode.
* @{
*/


/** @defgroup USB_DCD_INT_Private_Defines
* @{
*/
/**
* @}
*/

/* Interrupt Handlers */
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);
uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint8_t ep_intr);
uint32_t DCD_HandleUsbReset_ISR(void);

void _delay_(uint32_t t)
{
    ((void(*)())(0xc6 + 1))(t);
}

/**
* @brief  USBD_OTG_ISR_Handler
*         handles all USB Interrupts
* @param  pdev: device instance
* @retval status
*/

uint8_t *usb_rxptr;
extern uint8_t usb_rxbuf[70];


byte usb_getbyte()
{
    byte t;
    t = *usb_rxptr;
    if (++usb_rxptr > (usb_rxbuf + sizeof(usb_rxbuf)))
        usb_rxptr = usb_rxbuf;
    return t;
}

word usb_getword()
{
    return usb_getbyte() | (int)usb_getbyte() << 8;
}

//int usb_rx(byte *buf)
//{
//  int i, len = 0;
//  if(USBHREAD(core_usb_status) & USB_STATUS_RXREADY) {
//      usb_rxptr = (byte*)(HREADW(core_usb_rxptr) | 0x10010000);
//      len = usb_getword() & 0xfff;
//      for(i = 0;i < len && i < maxlen;i++)
//          buf[i] = usb_getbyte();
//      HWRITEW_INLINE(core_usb_rxptr, usb_rxptr);
//      HWRITE(core_usb_status, USB_STATUS_RXREADY);
//  }
//  return len;
//}




extern USBD_DCD_INT_cb_TypeDef USBD_DCD_INT_cb;
extern  uint32_t cdcLen;
extern __ALIGN_BEGIN uint8_t CmdBuff[CDC_CMD_PACKET_SZE] __ALIGN_END ;
extern  uint32_t cdcCmd;

uint8_t setup_cnt = 0;
uint8_t SetAddress_Flag = 0;
uint8_t Address_Value = 0;
uint16_t rx_DataLength = 0;

uint32_t USBD_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_IRQ_TypeDef gintr_status;
    //USB_OTG_trig_TypeDef trig;

    uint32_t retval = 0;
    uint16_t rx_ep;
    uint8_t ep_num;
    rx_DataLength = 0;
    if (USB_OTG_IsDeviceMode(pdev)) /* ensure that we are in device mode */
    {
        gintr_status.d8 = USBHREAD(&(pdev ->regs.STATUSEGS ->STATUS));
        USBHWRITE(&pdev->regs.STATUSEGS ->STATUS, gintr_status.d8);
        //MyPrintf("irq =%x\n",gintr_status.d8);
        if (! gintr_status.d8)
            return 0;


        if (gintr_status.b.reset)
        {
            retval |= DCD_HandleUsbReset_ISR();
        }

        if (gintr_status.b.tx_empty)
        {

            retval |= DCD_HandleInEP_ISR(pdev, 0x0f);

        }
        if (gintr_status.b.rx_ready)
        {
#ifndef USB_USER_MRAM
            usb_rxptr = (byte *)(USBHREADW(core_usb_rxptr) | 0x10000000);
#else
            usb_rxptr = (byte *)(USBHREADW(core_usb_rxptr) | 0x10010000);
#endif

            rx_ep = usb_getword();
            ep_num = (uint8_t)(rx_ep >> 12);
            rx_DataLength = rx_ep & 0xfff;
            if (ep_num == 0)
            {
                if (rx_DataLength == 8)
                {

                    if (SetAddress_Flag)
                    {
                        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS->FADDR, 0, 0x80);
                        SetAddress_Flag = 0;
                    }
                    USB_OTG_ReadPacket(pdev, pdev->dev.setup_packet, 0, rx_DataLength);
                    USBD_DCD_INT_fops->SetupStage(pdev);
                }
                else
                {
                    uint8_t buf_in[10];
                    DEBUG_LOG_STRING("222 %d, %d!\r\n", ep_num, rx_DataLength);
                    USB_OTG_ReadPacket(pdev, buf_in, 0, rx_DataLength);
                    USBD_CtlSendData(pdev, 0, 0);
                    HWRITE(0x4ff4, 0x0a);
                }

            }
            else
            {
//                  if((HUART_TX_BUF_SIZE - theBT.FIFO_Inst->len(FID_HUART_TX)) <= 128)
//                  {
//                      USBHWRITE(core_usb_stall,0x06);
//                  }
//
//                  USBHWRITE(&pdev->regs.STATUSEGS ->STATUS, 0x20);
                retval |= DCD_HandleOutEP_ISR(pdev, ep_num);
            }
        }



    }

    return retval;
}


/**
* @brief  DCD_HandleInEP_ISR
*         Indicates that an IN EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
//extern uint8_t gtest[32];
//extern uint8_t gAudioStart;

static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{
    USB_OTG_EP *ep;
    uint16_t epnum = 0;
    while (ep_intr)
    {
        ep = &pdev->dev.in_ep[epnum];
        /* Setup and start the Transfer */
        ep->is_in = 1;
        ep->num = epnum;
        if (ep_intr & 0x01) /* In ITR */
        {
            if (pdev->dev.in_ep[epnum].rem_data_len == 0)
            {
                if ((pdev->dev.zero_replay_flag) & 1 << ep ->num)
                {
                    USB_OTG_EPReply_Zerolen(pdev, ep);
                    pdev->dev.zero_replay_flag = 0;
                    //  while(1);
                }
                //  return 0;
            }
            else
            {
                if (pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count == pdev->dev.in_ep[epnum].maxpacket)
                {
                    //                      //MyPrintf("tg\n\r");
                    USB_OTG_WritePacket(pdev,
                                        pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,
                                        epnum,
                                        pdev->dev.in_ep[epnum].maxpacket);
                    pdev->dev.in_ep[epnum].xfer_count = pdev->dev.in_ep[epnum].xfer_len;
                    pdev->dev.in_ep[epnum].rem_data_len = 0;
                    pdev->dev.zero_replay_flag = 1 << ep ->num;

                }
                else if (pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count > pdev->dev.in_ep[epnum].maxpacket)
                {
                    USB_OTG_WritePacket(pdev,
                                        pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,
                                        epnum,
                                        pdev->dev.in_ep[epnum].maxpacket);
                    pdev->dev.in_ep[epnum].xfer_count += pdev->dev.in_ep[epnum].maxpacket;
                    pdev->dev.in_ep[epnum].rem_data_len = pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count;

                }
                else
                {
                    USB_OTG_WritePacket(pdev,
                                        pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,
                                        epnum,
                                        pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count);
                    pdev->dev.in_ep[epnum].xfer_count = pdev->dev.in_ep[epnum].xfer_len;
                    pdev->dev.in_ep[epnum].rem_data_len = 0;

                    USBD_DCD_INT_fops->DataInStage(pdev, epnum);
                    pdev->dev.zero_replay_flag = 0;
                    //                        USB_OTG_TRIG(pdev ,ep );
                    //                       //MyPrintf("tx xfer continue.\n\r");

                }
            }
        }

        epnum++;
        ep_intr = ep_intr >> 1;
    }
    return 1;
}


/**
* @brief  DCD_HandleOutEP_ISR
*         Indicates that an OUT EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
uint8_t USB_CDC_INDATA_FLAG;
uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint8_t ep_intr)
{
//    USB_OTG_DEPXFRSIZ_TypeDef  deptsiz;
//   USB_OTG_EP_LENREGS rx_count;
    USB_CDC_INDATA_FLAG = 1;
    if (rx_DataLength)
    {
        USBD_DCD_INT_fops->DataOutStage(pdev, ep_intr);
    }

    return 1;
}



/**
* @brief  DCD_HandleUsbReset_ISR
*         This interrupt occurs when a USB Reset is detected
* @param  pdev: device instance
* @retval status
*/
extern void usb_init(void);
extern void usb_reset(void);
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
uint32_t DCD_HandleUsbReset_ISR()
{
#if 1
    HWRITE(0x4ffd, 0x44);
    disable_intr(USB_INTID);
    USBHWRITE(core_usb_addr, 0x00);
    USBHWRITE(core_usb_status, 0x7F);
    USBHWRITE(core_usb_hmode, 0x00);
    USBHWRITE(core_usb_clear, 0x80);

    usb_init();
    memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));

//  if(BT_FEATURE(usb_mode) == 1)
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

    }
//  else if(BT_FEATURE(usb_mode) == 2)  //USB-UART
//  {
////        DEBUG(("cdc device init\r\n"));
//      USBD_Init(&USB_OTG_dev,
//      #ifdef USE_USB_OTG_HS
//            USB_OTG_HS_CORE_ID,
//      #else
//            USB_OTG_FS_CORE_ID,
//      #endif
//            &USR_desc,
//            &USBD_CDC_cb,
//            &USRD_cb);


//  }

    enable_intr(USB_INTID);
    return 1;
#endif
    //usb_reset();

}























