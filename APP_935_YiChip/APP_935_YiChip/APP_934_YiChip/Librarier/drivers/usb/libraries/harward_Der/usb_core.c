/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_core.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/
 
/* Include ------------------------------------------------------------------*/
//#include "mhscpu.h"
#include "usb_core.h"
#include "usb_bsp.h"
#include "usb_dcd_int.h"
#include "usbd_usr.h"
#include "private.h"
//#include "hal_uart.h"
#include <string.h>
//#include "yc_debug.h"
//#include "systick.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/	
/* Private macro ------------------------------------------------------------*/	
/* Private variables --------------------------------------------------------*/	
/* Ptivate function prototypes ----------------------------------------------*/	

/******************************************************************************
* Function Name  :
* Description    :
* Input          :
* Output         :
* Return         :
*******************************************************************



*******************************************************************************
* @brief  USB_OTG_WritePacket : Writes a packet into the Tx FIFO associated 
*         with the EP
* @param  pdev : Selected device
* @param  src : source pointer
* @param  ch_ep_num : end point number
* @param  bytes : No. of bytes
* @retval USB_OTG_STS : status
*/
#define USB_STATUS_NAK 4


extern  volatile uint8_t usb_txbuf0[65];
extern  volatile uint8_t usb_txbuf1[65];
extern  volatile uint8_t usb_txbuf2[65];
extern  volatile uint8_t usb_txbuf3[65];

USB_OTG_STS USB_OTG_WritePacket(USB_OTG_CORE_HANDLE *pdev, 
                                uint8_t             *src, 
                                uint8_t             ch_ep_num, 
                                uint8_t            len)
{
    USB_OTG_STS status = USB_OTG_OK;
	uint16_t i = 0;

//	my_delay_ms(1);
   // #ifdef USE_DEVICE_MODE
	uint8_t *ptr=src;
    if(ch_ep_num == 0)
    {
       if(USBHREAD(core_usb_txbusy)&0x01)			 
       		return USB_OTG_OK;

       		//MyPrintf("txtxtx");
       		//usb_txbuf0[0] = len-1;
       		USBHWRITE((uint8_t *)&usb_txbuf0[0],len-1);
       		memcpy((void *)(((uint8_t *)usb_txbuf0) + 1),ptr,len);
			USBHWRITEW(core_usb_tx_saddr0, (void*)usb_txbuf0);	  
    }
    else if (ch_ep_num ==1)
    {
//    	while(USBHREAD(core_usb_txbusy)&0x02);
//		if(USBHREAD(core_usb_txbusy)&0x02)
//			return USB_OTG_OK;
//    		usb_txbuf1[0] =len-1;
		USBHWRITE((uint8_t *)&usb_txbuf1[0],len-1);
		memcpy(( uint8_t *)(((uint8_t *)usb_txbuf1) + 1),( uint8_t *)ptr,len);
		USBHWRITEW(core_usb_tx_saddr1, (void*)usb_txbuf1);

    }
     else if (ch_ep_num ==2)
    {	
		USBHWRITE((uint8_t *)&usb_txbuf2[0],len-1);
		memcpy((uint8_t *)(((uint8_t *)usb_txbuf2) + 1),(uint8_t*)ptr,len);		
		USBHWRITEW(core_usb_tx_saddr2, (void*)usb_txbuf2);	
		
    }
    else
    {
        if (USBHREAD(core_usb_txbusy) & 0x08)
        {
            return USB_OTG_OK;
        }
        USBHWRITE((uint8_t *)&usb_txbuf3[0], len - 1);
        memcpy((void *)(((uint8_t *)usb_txbuf3) + 1), ptr, len);
        USBHWRITEW(core_usb_tx_saddr3, (void*)&usb_txbuf3);
        USBHWRITE(core_usb_trig, 8);

    }

	USBHWRITE(core_usb_status,USB_STATUS_NAK);
	
    for(i = 0;i < 10000;i++)
    {
          if(USBHREAD(core_usb_status)&USB_STATUS_NAK)
          {
                break;
          }
    }
    USBHWRITE(core_usb_trig,1 << ch_ep_num);

    return status;
}

//    return status;

extern byte *usb_rxptr;
extern uint16_t rx_DataLength;
extern byte usb_rxbuf[70];

/**
* @brief  USB_OTG_ReadPacket : Reads a packet from the Rx FIFO
* @param  pdev : Selected device
* @param  dest : Destination Pointer
* @param  bytes : No. of bytes
* @retval None
*/
void USB_OTG_ReadPacket(USB_OTG_CORE_HANDLE *pdev, 
                         uint8_t *dest, 
                         uint8_t ch_ep_num, 
                         uint16_t len)
{
    uint16_t i=0;
    uint16_t count8b = len ;
    uint8_t *data_buff = (uint8_t *)dest;
//	if(BT_FEATURE(usb_mode) == 2)
	{
		do
		{
			*data_buff++ = usb_getbyte();
			 i++;
		 } while (i < count8b);

		 USBHWRITEW(core_usb_status, USB_STATUS_NAK);
	 
		 for(i =0;i<10000;i++)
		 {
			if(USBHREAD(core_usb_status)&USB_STATUS_NAK)
			{
				break;
			}
		 }	
	}
//	else
//	{
//		for (i = 0; i < count8b; i++, data_buff++)
//		{
//			*data_buff = usb_getbyte();
//		}

//	}

	 
	USBHWRITEW(core_usb_rxptr, usb_rxptr);
	rx_DataLength =0;
//	if(!(USBHREAD(core_usb_txbusy)&0x40))
//	{
//		USB_RX_CONTINUE(pdev);	
//	}
//	USBHWRITE(core_usb_status, USB_STATUS_RXREADY);	
    /* Return the buffer pointer because if the transfer is composed of several
     packets, the data of the next packet must be stored following the 
     previous packet's data */
//    return ;
}
extern  uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint8_t ep_intr);
//extern USBD_DCD_INT_cb_TypeDef *USBD_DCD_INT_fops;

void USBD_Get_Packet(USB_OTG_CORE_HANDLE *pdev,uint8_t *Ptr)
{
	uint16_t RxHead,glen;
	uint8_t epnum;
	RxHead =usb_getword();
	glen = RxHead &0xfff;          	
        epnum= (uint8_t)(RxHead>>12);
        
        USB_OTG_ReadPacket(pdev , 
                                       Ptr,
                                       epnum, 
                                       glen);
}

void USB_RX_CONTINUE(USB_OTG_CORE_HANDLE *pdev)
{
		uint16_t rx_ep;
//		uint8_t j[10];
		volatile uint16_t cnt,i;
		rx_DataLength =0;
		//MyPrintf("rx continue\n ");
		for(i=0;i<1000;i++)
		{
		   if(USBHREAD(core_usb_status) & USB_STATUS_RXREADY) 
		   {

		       USBHWRITE(core_usb_status, USB_STATUS_RXREADY);	
			//MyPrintf("rx_ptr =%2x\n",usb_rxptr);
		   }
		 }  	
          	rx_ep = usb_getword() ;
          	//MyPrintf("rx =%02x\n",rx_ep);
          	rx_DataLength = rx_ep &0xfff;
         // 	MyPrintf("len =%x\n",rx_DataLength);
          	if(rx_DataLength ==0)
			{
				return ;
			}
          	if(rx_ep>>12 == 0)
          	{
			if(rx_DataLength != 8 )
			{
			  for(uint16_t i=0;i<rx_DataLength;i++)
				{
//					j[i]= usb_getbyte();
				//	HWRITEW_INLINE(core_usb_rxptr, usb_rxptr);
				}
			  
			}
			else
			{
			//	MyPrintf("rx1 continue\n");

				USB_OTG_ReadPacket(pdev , 
									   pdev->dev.out_ep[0].xfer_buff + pdev->dev.out_ep[0].xfer_count,
									   0, 
									   rx_DataLength);
							   USBD_DCD_INT_fops->SetupStage(pdev);				



			}

          	}
          	else
          	{
          		//MyPrintf("rx continue1\n ");
			DCD_HandleOutEP_ISR(pdev,rx_ep>>12);
			//HWRITEW_INLINE(core_usb_rxptr, usb_rxptr);
          	}
   //       	}

}

/**
* @brief  USB_OTG_SelectCore 
*         Initialize core registers address.
* @param  pdev : Selected device
* @param  coreID : USB OTG Core ID
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SelectCore(USB_OTG_CORE_HANDLE *pdev, 
                               USB_OTG_CORE_ID_TypeDef coreID)
{

       USB_OTG_STS status = USB_OTG_OK;
        #ifdef  USB_OTG_DMA_ENABLE
        pdev->cfg.dma_enable       = 1; 
        #else
        pdev->cfg.dma_enable       = 0;
        #endif

	 /* at startup the core is in FS mode */
       pdev->cfg.speed            = USB_OTG_SPEED_FULL;
       pdev->cfg.mps              = USB_OTG_FS_MAX_PACKET_SIZE ;    

       
       /* initialize device cfg following its address */
    if (coreID == USB_OTG_FS_CORE_ID)
    {

        pdev->cfg.coreID           = USB_OTG_FS_CORE_ID;
//        pdev->cfg.host_channels    = 8 ;
        pdev->cfg.dev_endpoints    = 4 ;
        pdev->cfg.TotalFifoSize    = 64; /* in 8-bits */
        pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;     

        #ifdef USB_OTG_FS_SOF_OUTPUT_ENABLED    
        pdev->cfg.Sof_output       = 1;    
        #endif 

        #ifdef USB_OTG_FS_LOW_PWR_MGMT_SUPPORT    
        pdev->cfg.low_power        = 1;    
        #endif     
    }
    else if (coreID == USB_OTG_HS_CORE_ID)
    {
//        baseAddress                = USB_OTG_FS_BASE_ADDR;
        pdev->cfg.coreID           = USB_OTG_HS_CORE_ID;    
        pdev->cfg.host_channels    = 8 ;
        pdev->cfg.dev_endpoints    = 4 ;
        pdev->cfg.TotalFifoSize    = 512;/* in 8-bits */

        #ifdef USB_OTG_ULPI_PHY_ENABLED
            pdev->cfg.phy_itface       = USB_OTG_ULPI_PHY;
        #else    
        #ifdef USB_OTG_EMBEDDED_PHY_ENABLED
            pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;
        #endif  
        #endif      

        #ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED    
            pdev->cfg.dma_enable       = 1;    
        #endif

        #ifdef USB_OTG_HS_SOF_OUTPUT_ENABLED    
            pdev->cfg.Sof_output       = 1;    
        #endif 

        #ifdef USB_OTG_HS_LOW_PWR_MGMT_SUPPORT    
            pdev->cfg.low_power        = 1;    
        #endif 
    }

	/* Common USB Registers */
	pdev ->regs.CTRLREGS = (USB_OTG_CTRLREGS *)core_usb_config;
//	pdev ->regs.RXCTL =(USB_OTG_DMA_RXCTL *)core_usb_rx_saddr;
	pdev ->regs. Hmode=(USB_OTG_HMODE*)core_usb_hmode;
	pdev ->regs.xferctl = (USB_OTG_XFERCTL*)core_usb_trig;
	  /* FIFOS */
 //   for (i = 0; i < NUM_EP_FIFO; i++)
  //  {
  //      pdev->regs.TXADDR[i] = (uint16_t *)(core_usb_tx_saddr0+i*2);
   // }
  
	pdev ->regs.STATUSEGS= (USB_OTG_STATUSREGS *)core_usb_status_stall;
    return status;
}


/**
* @brief  USB_OTG_CoreInit
*         Initializes the USB_OTG controller registers and prepares the core
*         device mode or host mode operation.
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_CoreInit(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS status = USB_OTG_OK;
//    USB_OTG_GUSBCFG_TypeDef  usbcfg;
//    USB_OTG_GCCFG_TypeDef    gccfg;
//    USB_OTG_GAHBCFG_TypeDef  ahbcfg;

//    usbcfg.d32 = 0;
//    gccfg.d32 = 0;
//    ahbcfg.d32 = 0;
//  
//    /* FS interface*/ 
//    /* Reset after a PHY select and set Host mode */
//    USB_OTG_CoreReset(pdev);
//    /* Deactivate the power down*/
//    gccfg.d32 = 0;
//    gccfg.b.pwdn = 1;
//    
//    gccfg.b.vbussensingA = 1 ;
//    gccfg.b.vbussensingB = 1 ;     
//#ifndef VBUS_SENSING_ENABLED
//    gccfg.b.disablevbussensing = 1; 
//#endif    
//    
//    if(pdev->cfg.Sof_output)
//    {
//        gccfg.b.sofouten = 1;  
//    }
//    
//    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GCCFG, gccfg.d32);
//    USB_OTG_BSP_mDelay(20);
//    /* case the HS core is working in FS mode */
//    if(pdev->cfg.dma_enable == 1)
//    {
//        ahbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GAHBCFG);
//        ahbcfg.b.hburstlen = 5; /* 64 x 32-bits*/
//        ahbcfg.b.dmaenable = 1;
//        USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GAHBCFG, ahbcfg.d32);
//    }
//    /* initialize OTG features */
//#ifdef  USE_OTG_MODE
//    usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);
//    usbcfg.b.hnpcap = 1;
//    usbcfg.b.srpcap = 1;
//    USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);
//    USB_OTG_EnableCommonInt(pdev);
//#endif
    return status;
}



/**
* @brief  USB_OTG_SetCurrentMode : Set ID line
* @param  pdev : Selected device
* @param  mode :  (Host/device)only device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SetCurrentMode(USB_OTG_CORE_HANDLE *pdev , uint8_t mode)
{
    USB_OTG_STS status = USB_OTG_OK;
//    USB_OTG_POWER_TypeDef  power;
//    USB_OTG_DEVCTL_TypeDef devctl;
    
//    power.d8 = 0;
//    devctl.d8 = USBHREAD(&pdev->regs.DYNFIFOREGS->DEVCTL);
//    
    if ( mode == HOST_MODE)
    {
//        power.b.en_suspendM = 1;
//        USBHWRITE(&pdev->regs.COMMREGS->POWER, power.d8);
//        devctl.b.host_mode = 1;
//        devctl.b.session = 1;
//        USBHWRITE(&pdev->regs.DYNFIFOREGS->DEVCTL, devctl.d8);
    }
    else if ( mode == DEVICE_MODE)
    {
//        devctl.b.host_mode = 0;
//        power.b.en_suspendM = 1;
//        devctl.b.session = 0;
//        power.b.soft_conn = 1;
//        USBHWRITE(&pdev->regs.DYNFIFOREGS->DEVCTL, devctl.d8);
//        USBHWRITE(&pdev->regs.COMMREGS->POWER, power.d8);
        #ifdef USE_DEVICE_MODE
        pdev->dev.out_ep[0].xfer_buff = pdev->dev.setup_packet;
        pdev->dev.out_ep[0].xfer_len = 8;
        #endif
    }

//	delay_us(50);
   // USB_OTG_BSP_mDelay(50);
	
    return status;
}

/**
* @brief  USB_OTG_EPActivate : Activates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPActivate(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_IRQ_MASK1_TypeDef intr_rxtxe;
//    volatile uint16_t *addr;
    /* Read DEPCTLn register */
    if (ep->is_in == 1)
    {
//        addr = &pdev->regs.COMMREGS->INTRTXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS->IRQ_MASK, intr_rxtxe.d8, 0);
    }
    else
    {
//        addr = &pdev->regs.COMMREGS->INTRRXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK, intr_rxtxe.d8, 0);
    }  
    return status;
}


/**
* @brief  USB_OTG_EPDeactivate : Deactivates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPDeactivate(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_IRQ_MASK1_TypeDef intr_rxtxe;
//    volatile uint16_t *addr;
    /* Read DEPCTLn register */
    if (ep->is_in == 1)
    {
//        addr = &pdev->regs.COMMREGS->INTRTXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK, 0, intr_rxtxe.d8);
    }
    else
    {
//        addr = &pdev->regs.COMMREGS->INTRRXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK, 0, intr_rxtxe.d8);
     }  
    return status;
}



/**
* @brief  USB_OTG_GetMode : Get current mode
* @param  pdev : Selected device
* @retval current mode
*/
uint8_t USB_OTG_GetMode(USB_OTG_CORE_HANDLE *pdev)
{
    return DEVICE_MODE;
}


/**
* @brief  USB_OTG_IsDeviceMode : Check if it is device mode
* @param  pdev : Selected device
* @retval num_in_ep
*/
uint8_t USB_OTG_IsDeviceMode(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_GetMode(pdev) != HOST_MODE);
}

/**
* @brief  USB_OTG_EPStartXfer : Handle the setup for data xfer for an EP and 
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
extern uint16_t rx_DataLength;
USB_OTG_STS USB_OTG_EPStartXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint8_t  rx_count;
//	uint16_t i;
	

    /* IN endpoint */
	
    if (ep->is_in == 1)
	{
		 if ((pdev->cfg.dma_enable == 0) || ((USB_OTG_DEV_DMA_EP_NUM & 0x07) != ep->num))
		 {
			ep->rem_data_len = ep->xfer_len - ep->xfer_count;
			if(ep->rem_data_len == ep->maxpacket)
			{
				USB_OTG_WritePacket(pdev, 
									ep->xfer_buff + ep->xfer_count, 
									ep->num, 
									ep->maxpacket); 
//				ep->xfer_count += ep->maxpacket;
				ep->xfer_count = ep->xfer_len; 
				ep->rem_data_len = 0;
				pdev->dev.zero_replay_flag =1 << ep ->num;
			
			}
			/* Zero Length Packet? */
			else if (ep->rem_data_len == 0)
			{

					USB_OTG_WritePacket(pdev, 
										ep->xfer_buff + ep->xfer_count, 
										ep->num, 
										0);
					ep->xfer_count = ep->xfer_len;
					ep->rem_data_len = 0; 
			}
			 else
			{
					if (ep->rem_data_len >ep->maxpacket)
					{

							USB_OTG_WritePacket(pdev, 
												ep->xfer_buff + ep->xfer_count, 
												ep->num, 
												ep->maxpacket); 
							USBHWRITE(core_usb_trig,1 << ep->num);
							ep->xfer_count += ep->maxpacket;

							if (ep->xfer_len >= ep->xfer_count)
							{
									ep->rem_data_len = ep->xfer_len - ep->xfer_count;
							}		 
							else		 
							{
									ep->rem_data_len = 0;
									ep->xfer_count = ep->xfer_len;
							}										

					}
					else
					{
							//MyPrintf("tx_len0 %x\n",ep->rem_data_len);
//							DEBUG(("tx_len0 %x\n",ep->rem_data_len));
							USB_OTG_WritePacket(pdev, 
												ep->xfer_buff + ep->xfer_count, 
												ep->num, 
												ep->rem_data_len); 
//													USB_OTG_TRIG(pdev,ep);
							ep->xfer_count = ep->xfer_len; 
							ep->rem_data_len = 0;	

					}
				}
			}
		}
		else
		{
				/* OUT endpoint */	

				rx_count = rx_DataLength; 

				USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count,ep->num, rx_count);																 
				ep->xfer_count += rx_count;
				if (ep->xfer_len <= ep->xfer_count)
				{
						ep->rem_data_len = ep->xfer_count - ep->xfer_len;
				}
				else
				{
						ep->rem_data_len = 0;

						ep->xfer_len = ep->xfer_count;
				}			

		}
		return status;
}


/**
* @brief  USB_OTG_EP0StartXfer : Handle the setup for a data xfer for EP0 and 
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EP0StartXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS  status = USB_OTG_OK;
 
//    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;
//    uint32_t fifoemptymsk = 0;
//    csr0l.d8 = USBHREAD(&pdev->regs.INDEXREGS->CSRL.CSR0L);
    /* IN endpoint */
    if (ep->is_in == 1)
    {
        ep->rem_data_len = ep->xfer_len - ep->xfer_count;
        /* Zero Length Packet? */
        if (ep->rem_data_len == 0)
        {
//        	//	send zero packet
           USB_OTG_EPReply_Zerolen(pdev,ep);
            ep->xfer_count = ep->xfer_len;
            ep->rem_data_len = 0;
//            csr0l.b.tx_pkt_rdy = 1;
//            csr0l.b.data_end = 1;
//            USBHWRITE(&pdev->regs.INDEXREGS->CSRL.CSR0L,csr0l.d8);
        }
        else
        {
            if (ep->rem_data_len > ep->maxpacket)
            {
                USB_OTG_WritePacket(pdev, 
                                    ep->xfer_buff + ep->xfer_count, 
                                    0, 
                                    ep->maxpacket); 
                ep->xfer_count += ep->maxpacket;
                ep->rem_data_len = ep->xfer_len - ep->xfer_count;
   //             USB_OTG_TRIG(pdev , ep);
//                csr0l.b.tx_pkt_rdy = 1;
//                USBHWRITE(&pdev->regs.INDEXREGS->CSRL.CSR0L,csr0l.d8);
            }
            else
            {
                USB_OTG_WritePacket(pdev, 
                                    ep->xfer_buff + ep->xfer_count, 
                                    0, 
                                    ep->rem_data_len);  
                ep->xfer_count = ep->xfer_len;    
                ep->rem_data_len = 0;   
 //               USB_OTG_TRIG(pdev , ep);
//                //MyPrintf("usb0 tx\n");
//                csr0l.b.tx_pkt_rdy = 1;
//                csr0l.b.data_end = 1;
//                USBHWRITE(&pdev->regs.INDEXREGS->CSRL.CSR0L,csr0l.d8);
            }
        }
//        if (pdev->cfg.dma_enable == 1)
//        {

//        }

        if (pdev->cfg.dma_enable == 0)
        {
            /* Enable the Tx FIFO Empty Interrupt for this EP */
            if (ep->xfer_len > 0)
            {
            }
        }
    }
//    else
//    {
//        /* Program the transfer size and packet count as follows:
//        * xfersize = N * (maxpacket + 4 - (maxpacket % 4))
//        * pktcnt = N           */
//        if (ep->xfer_len == 0)
//        {

//        }
//        else
//        {

//        }
//        if (pdev->cfg.dma_enable == 1)
//        {
//        }
//    }
    return status;
}

/**
* @brief  USB_OTG_TRIG : Handle start xfer and set tx trig
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/

USB_OTG_STS  USB_OTG_TRIG(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
//   int i;
   USB_OTG_STS status = USB_OTG_OK;

//	USBHWRITE(core_usb_trig,1 << ep->num);
 return status;
}
/**
* @brief  USB_OTG_EPSetStall : Set the EP STALL
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPSetStall(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
//   USB_OTG_stall_TypeDef stcsrl;
			if(ep->num ==0)
				USBHWRITE(&pdev->regs.xferctl ->STALL, 0x01);
//    if (ep->is_in == 1)
//    {
//        if (ep->num)
//        {
//            stcsrl.d8 = 1 <<  2 * (ep ->num) -1;
//            /* set the stall bit */
//            USB_OTG_MODIFY_REG8(&pdev->regs.xferctl ->STALL, 0,stcsrl.d8);
//        }
//        else
//        {
//            
//            USBHWRITE(&pdev->regs.xferctl ->STALL, 0x01);
//        }
//    }
//    else    /* OUT Endpoint */
//    {
//        if (ep->num)
//        {
//             stcsrl.d8 =1 << 2 * (ep ->num) ;
//            USB_OTG_MODIFY_REG8(&pdev->regs.xferctl ->STALL, 0,stcsrl.d8);
//        }
//        else
//        {
//            USBHWRITE(&pdev->regs.xferctl ->STALL, 0x01);
//        }
//    }
    USB_OTG_EPReply_Zerolen(pdev,ep);
    return status;
}

/**
* @brief  USB_OTG_EPSetStall : ack zero  length packet
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/

USB_OTG_STS USB_OTG_EPReply_Zerolen(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
	USB_OTG_STS status = USB_OTG_OK;
           USBHWRITE(&pdev->regs.xferctl ->TRIG, 0x10 <<(ep ->num) );
    return status;
}



/**
* @brief  USB_OTG_RemoteWakeup : active remote wakeup signalling
* @param  None
* @retval : None
*/
void USB_OTG_ActiveRemoteWakeup(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_CTL_TypeDef power;
    /* Note: If CLK has been stopped,it will need be restarted before 
     * this write can occur. 
     */
    power.d8 = USBHREAD(&pdev->regs.CTRLREGS ->USB_CTL); 
    power.b.resume = 1;
    power.b.wakeup_enable = 1;
    USBHWRITE(&pdev->regs.CTRLREGS ->USB_CTL, power.d8);
    /* The software should leave then this bit set for approximately 10ms
     * (minimum of 2ms, a maximum of 15ms) before resetting it to 0.    
     */
   my_delay_ms(2);
    power.b.resume = 0;
    power.b.wakeup_enable = 0;
    USBHWRITE(&pdev->regs.CTRLREGS ->USB_CTL, power.d8);
}




