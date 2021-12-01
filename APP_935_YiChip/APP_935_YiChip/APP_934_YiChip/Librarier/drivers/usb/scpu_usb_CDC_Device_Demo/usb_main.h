#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include <string.h>
#include <stdio.h>
#include "usb_dcd_int.h"

//#define USB_USER_MRAM
void usb_main(void);
//uint8_t usb_cdc_send(uint8_t* buf,uint8_t length);
//void hal_usb_cdc_rx_cb(uint8_t *buf, uint32_t len);
uint8_t hal_usb_send(uint8_t *data,uint16_t len);
void usb_send_report(void);



