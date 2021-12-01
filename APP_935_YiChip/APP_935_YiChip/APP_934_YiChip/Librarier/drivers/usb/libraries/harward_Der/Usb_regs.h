
/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_regs.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : USB OTG IP hardware registers.
 *****************************************************************************/
 
 
#ifndef __USB_OTG_REGS_H__
#define __USB_OTG_REGS_H__

#include "yc11xx.h"
#include "type.h"
#define USB_OTG_FS_BASE_ADDR               core_usb_config 
#define NUM_EP_FIFO                                         4 
#define USB_OTG_EP_FIFO_REG_OFFSET                          0x0001

#define USB_OTG_MAX_TX_FIFOS                 15

#define USB_OTG_HS_MAX_PACKET_SIZE           512
#define USB_OTG_FS_MAX_PACKET_SIZE           64
#define USB_OTG_MAX_EP0_SIZE                 64




typedef struct _USB_OTG_CTRLREGS
{
    volatile uint8_t USB_CTL;
    volatile uint8_t IRQ_MASK;
    volatile uint8_t FADDR;
   
}USB_OTG_CTRLREGS;


typedef struct _USB_OTG_DMA_RXCTL
{
  volatile uint16_t USB_RX_SADDR;
  volatile uint16_t USB_RX_EADDR;
  volatile uint16_t USB_RXptr;
  
}USB_OTG_DMA_RXCTL;


typedef struct _USB_OTG_XFERCTL
{
   volatile uint8_t TRIG;
   volatile uint8_t STALL;
   volatile uint8_t TOGGLE;   
}USB_OTG_XFERCTL;


typedef struct _USB_OTG_STATUSREGS
{
  volatile uint8_t STALL_STATUS;	
   volatile uint8_t STATUS;
   volatile uint8_t USB_TX_BUSY;
   volatile uint8_t SOF_CNTL;
   volatile uint8_t SPF_CNTH;
}USB_OTG_STATUSREGS;


typedef struct _USB_OTG_HMODE
{

   volatile uint8_t Hmode;
   volatile uint8_t DRV;
}USB_OTG_HMODE;
typedef struct
{
  USB_OTG_CTRLREGS 	*CTRLREGS;
  USB_OTG_DMA_RXCTL	*RXCTL;
  uint16_t					*TXADDR[NUM_EP_FIFO];
  USB_OTG_HMODE		  *Hmode;
  USB_OTG_XFERCTL		*xferctl;
  volatile uint16_t 			RX_Wptr;
  USB_OTG_STATUSREGS	*STATUSEGS;	

}USB_OTG_CORE_REGS, *PUSB_OTG_CORE_REGS;




typedef union _USB_OTG_CTL_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t enable_iso_outep2			:1;
	uint8_t enable_iso_inep2			:1;
	uint8_t pad_bias_ctl         			:2;
	uint8_t enable_usb_function			:1;
	uint8_t speed						:1;
	uint8_t resume  					:1;
	uint8_t wakeup_enable				:1;

   }b;


}USB_OTG_CTL_TypeDef;


typedef union _USB_OTG_IRQ_MASK1_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t setup_mask				:1;
	uint8_t suspend_mask				:1;
	uint8_t nak_mask				:1;
	uint8_t reset_mask				:1;
	uint8_t ack_mask					:1;
	uint8_t rx_ready_mask				:1;
	uint8_t tx_empty					:1;
	uint8_t reserved1					:1;
   }		b;

}		USB_OTG_IRQ_MASK1_TypeDef;








typedef union _USB_OTG_trig_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t send_ep0_data										:1;
	uint8_t send_ep1_data										:1;
	uint8_t send_ep2_data										:1;
	uint8_t send_ep3_data										:1;
	uint8_t reply_endpiont0_zero_packet			:1;
	uint8_t reply_endpiont1_zero_packet			:1;
	uint8_t reply_endpiont2_zero_packet			:1;
	uint8_t reply_endpiont3_zero_packet			:1;
   }b;

}USB_OTG_trig_TypeDef;


typedef union _USB_OTG_stall_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t ep0_stall				:1;
	uint8_t inep1_stall				:1;
	uint8_t outep1_stall				:1;
	uint8_t inep2_stall				:1;
	uint8_t outep2_stall				:1;
	uint8_t inep3_stall				:1;
	uint8_t outep3_stall				:1;
	uint8_t host_mode_start				:1;
   }b;

}USB_OTG_stall_TypeDef;

typedef union _USB_OTG_toggle_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t clear_inep1_to_data0		:1;
	uint8_t clear_outep1_to_data0	:1;
	uint8_t clear_inep2_to_data0		:1;
	uint8_t clear_outep2_to_data0	:1;
	uint8_t clear_inep3_to_data0		:1;
	uint8_t clear_outep3_to_data0	:1;
	uint8_t reset						:1;
	uint8_t reserved						:1;
   }b;

}USB_OTG_toggle_TypeDef;

typedef union _USB_OTG_stall_status_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t ep0_is_stall				:1;
	uint8_t inep1_is_stall			:1;
	uint8_t outep1_is_stall			:1;
	uint8_t inep2_is_stall			:1;
	uint8_t outep2_is_stall			:1;
	uint8_t inep3_is_stall			:1;
	uint8_t outep3_is_stall			:1;
	uint8_t reserved				:1;
   }b;

}USB_OTG_stall_status_TypeDef;

typedef union _USB_OTG_IRQ_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t setup				:1;
	uint8_t suspend				:1;
	uint8_t nak				:1;
	uint8_t reset				:1;
	uint8_t ack					:1;
	uint8_t rx_ready				:1;
	uint8_t tx_empty					:1;
	uint8_t reserved1					:1;
   }b;

}USB_OTG_IRQ_TypeDef;

typedef union _USB_OTG_TXBUSY_TypeDef
{
   uint8_t d8;
   struct
   {
	uint8_t endpiont0_busy				:1;
	uint8_t endpoint1_busy				:1;
	uint8_t endpoint2_busy				:1;
	uint8_t endpoint3_busy				:1;
	uint8_t dn_status					:1;
	uint8_t dp_status				:1;
	uint8_t rx_empty_state					:1;
	uint8_t reserved					:1;
   }b;

}USB_OTG_TXBUSY_TypeDef;


#endif	/* __USB_OTG_REGS_H__ */

