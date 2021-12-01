/*
File Name    : system.h
Author       : Yichip
Version      : V1.0
Date         : 2018/05/22
Description  : none.
*/

#ifndef __SYSTEM_H__
#define __SYSTEM_H__


#include <string.h>
#include "yc11xx_uart.h"
#include "yc11xx_gpio.h"

#define SDK_DEBUG	//Debug switch

#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))				//bit Negation
#define BIT_GET(a,b) (((a) & (1<<(b)))>>(b))

typedef struct printportStruct
{
    USART_TypeDef print_port;
    uint8_t       print_rxio;
    uint8_t       print_txio;
}printport_CB;
/**
 * @brief Print format string to serial port 0.You need to initialize the serial port 0 before you use MyPrintf.
 *
 * @param format : format string
 * @param ...: format parameter
 */
void printport_init(void);

void setprintport(USART_TypeDef UARTx);

void getprintportcb(printport_CB *printportx);

void MyPrintf(char *format, ...);

void open_log_print(uint8_t isopen);

void log_print(char *format, ...);
void log2_print(char *format, ...);

#endif /*__SYSTEM_H__*/

