/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_dcd.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface layer.
 *****************************************************************************/
 
/* Include ------------------------------------------------------------------*/
#include "usb_dcd.h"
#include "usb_bsp.h"




void DCD_Init(USB_OTG_CORE_HANDLE *pdev , 
              USB_OTG_CORE_ID_TypeDef coreID)
{
	 uint32_t i;
    USB_OTG_EP *ep;
    /* Set Register Address */
    USB_OTG_SelectCore (pdev , coreID);
    
//    /* Force_FS */
//    pdev->regs.COMMREGS->TESTMODE |= BIT5;
    
    pdev->dev.device_status = USB_OTG_DEFAULT;
    pdev->dev.device_address = 0;

       /* Init ep structure */
    for (i = 0; i < pdev->cfg.dev_endpoints; i++)
    {
        ep = &pdev->dev.in_ep[i];
        /* Init ep structure */
        ep->is_in = 1;
        ep->num = i;
        ep->tx_fifo_num = i;
        /* Control until ep is actvated */
        ep->type = EP_TYPE_CTRL;
        ep->maxpacket =  USB_OTG_MAX_EP0_SIZE;
        ep->xfer_buff = 0;
        ep->xfer_len = 0;
    }

    for (i = 0; i < pdev->cfg.dev_endpoints; i++)
    {
        ep = &pdev->dev.out_ep[i];
        /* Init ep structure */
        ep->is_in = 0;
        ep->num = i;
        ep->tx_fifo_num = i;
        /* Control until ep is activated */
        ep->type = EP_TYPE_CTRL;
        ep->maxpacket = USB_OTG_MAX_EP0_SIZE;
        ep->xfer_buff = 0;
        ep->xfer_len = 0;
     }


	/* Init the Core (common init.) */
         USB_OTG_CoreInit(pdev);

//	 /* Force Device Mode*/
         USB_OTG_SetCurrentMode(pdev, DEVICE_MODE);

	
//    /* Init Device */
//    USB_OTG_CoreInitDev(pdev);

}



/**
* @brief  Configure an EP
* @param pdev : Device instance
* @param epdesc : Endpoint Descriptor
* @retval : status
*/
uint32_t DCD_EP_Open(USB_OTG_CORE_HANDLE *pdev , 
                     uint8_t ep_addr,
                     uint16_t ep_mps,
                     uint8_t ep_type)
{
    USB_OTG_EP *ep;

    if ((ep_addr & 0x80) == 0x80)
    {
        ep = &pdev->dev.in_ep[ep_addr & 0x7F];
    }
    else
    {
        ep = &pdev->dev.out_ep[ep_addr & 0x7F];
    }
    ep->num   = ep_addr & 0x7F;

    ep->is_in = (0x80 & ep_addr) != 0;
    ep->maxpacket = ep_mps;
    ep->type = ep_type;
    if (ep->is_in)
    {
        /* Assign a Tx FIFO */
        ep->tx_fifo_num = ep->num;
    }
    /* Set initial data PID. */
    if (ep_type == USB_OTG_EP_BULK )
    {
        ep->data_pid_start = 0;
    }
   // USB_OTG_EPActivate(pdev , ep );
    return 0;
}
/**
* @brief  called when an EP is disabled
* @param pdev: device instance
* @param ep_addr: endpoint address
* @retval : status
*/
uint32_t DCD_EP_Close(USB_OTG_CORE_HANDLE *pdev , uint8_t  ep_addr)
{
    USB_OTG_EP *ep;

    if ((ep_addr&0x80) == 0x80)
    {
        ep = &pdev->dev.in_ep[ep_addr & 0x7F];
    }
    else
    {
        ep = &pdev->dev.out_ep[ep_addr & 0x7F];
    }
    ep->num   = ep_addr & 0x7F;
    ep->is_in = (0x80 & ep_addr) != 0;
    USB_OTG_EPDeactivate(pdev , ep );
    return 0;
}



/**
* @brief  DCD_EP_PrepareRx
* @param pdev: device instance
* @param ep_addr: endpoint address
* @param pbuf: pointer to Rx buffer
* @param buf_len: data length
* @retval : status
*/
uint32_t   DCD_EP_PrepareRx( USB_OTG_CORE_HANDLE *pdev,
                            uint8_t   ep_addr,
                            uint8_t *pbuf,                        
                            uint16_t  buf_len)
{
    USB_OTG_EP *ep;

    ep = &pdev->dev.out_ep[ep_addr & 0x7F];

    /*setup and start the Xfer */
    ep->xfer_buff = pbuf;  
    ep->xfer_len = buf_len;
    ep->xfer_count = 0;
    ep->is_in = 0;
    ep->num = ep_addr & 0x7F;
		
    if (pdev->cfg.dma_enable == 1)
    {
        ep->dma_addr = (uint32_t)pbuf;  
    }

		
    if ( ep->num == 0 )
    {
        USB_OTG_EP0StartXfer(pdev , ep);
    }
    else
    {
		
        USB_OTG_EPStartXfer(pdev, ep );
    }
    return 0;
}


/**
* @brief  Transmit data over USB
* @param pdev: device instance
* @param ep_addr: endpoint address
* @param pbuf: pointer to Tx buffer
* @param buf_len: data length
* @retval : status
*/
uint32_t  DCD_EP_Tx ( USB_OTG_CORE_HANDLE *pdev,
                     uint8_t   ep_addr,
                     uint8_t   *pbuf,
                     uint32_t   buf_len)
{

    USB_OTG_EP *ep;

    ep = &pdev->dev.in_ep[ep_addr & 0x7F];

    /* Setup and start the Transfer */
    ep->is_in = 1;
    ep->num = ep_addr & 0x7F;  
    ep->xfer_buff = pbuf;
//    ep->dma_addr = (uint32_t)pbuf;  
    ep->xfer_count = 0;
    ep->xfer_len  = buf_len;

    if ( ep->num == 0 )
    {
        USB_OTG_EP0StartXfer(pdev , ep);
    }
    else
    {
        USB_OTG_EPStartXfer(pdev, ep );
    }
    return USB_OTG_OK;
}






/**
* @brief  Stall an endpoint.
* @param pdev: device instance
* @param epnum: endpoint address
* @retval : status
*/

uint32_t  DCD_EP_Stall (USB_OTG_CORE_HANDLE *pdev, uint8_t   epnum)
{
    USB_OTG_EP *ep;
    if ((0x80 & epnum) == 0x80)
    {
        ep = &pdev->dev.in_ep[epnum & 0x7F];
    }
    else
    {
        ep = &pdev->dev.out_ep[epnum];
    }

    ep->is_stall = 1;
    ep->num   = epnum & 0x7F;
    ep->is_in = ((epnum & 0x80) == 0x80);

    USB_OTG_EPSetStall(pdev , ep);
    return (0);
}










