/*
File Name    : system.c
Author       : Yichip
Version      : V1.0
Date         : 2019/11/13
Description  : none.
*/

#include <stdarg.h>
#include "system.h"

//*****************************************************************************
//
//! A simple  MyPrintf function supporting \%c, \%d, \%p, \%s, \%u,\%x, and \%X.
//!
//! \param format is the format string.
//! \param ... are the optional arguments, which depend on the contents of the
//! \return None.
//
//*****************************************************************************
#define PRINTPORT  UARTA
#define PRINTRXIO  25
#define PRINTTXIO  28  

static printport_CB printportcb =
{
    .print_port  = UARTA,
    .print_rxio  = PRINTRXIO,
    .print_txio  = PRINTTXIO
};

void printport_init(void)
{
    USART_InitTypeDef USART_InitStruct ;
	
	USART_InitStruct.USART_BaudRate             = UARTE_BAUDRATE_BAUDRATE_Baud921600;
	USART_InitStruct.USART_HardwareFlowControl  = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_WordLength           = USART_WordLength_8b;	
	USART_InitStruct.USART_StopBits             = USART_StopBits_1;
	USART_InitStruct.USART_Mode                 = USART_Mode_duplex;
	USART_InitStruct.USART_Parity               = USART_Parity_Even ;
	USART_InitStruct.USART_TXLen                = 512;
	USART_InitStruct.USART_RXLen                = 1;

	if(UARTA == printportcb.print_port){
	GPIO_SetGpioMultFunction((GPIO_NUM)printportcb.print_txio,GPCFG_UART_TXD);
	GPIO_SetGpioMultFunction((GPIO_NUM)printportcb.print_rxio,GPCFG_UART_RXD);
	}else if (UARTB == printportcb.print_port){
	GPIO_SetGpioMultFunction((GPIO_NUM)printportcb.print_txio,GPCFG_UARTB_TXD);
	GPIO_SetGpioMultFunction((GPIO_NUM)printportcb.print_rxio,GPCFG_UARTB_RXD);
	}
	USART_Init(printportcb.print_port,&USART_InitStruct);
}



void setprintport(USART_TypeDef UARTx)
{
    printportcb.print_port = UARTx;
}

void setprintportcb(printport_CB *printportx)
{
	 printportcb.print_port  = printportx->print_port;
	 printportcb.print_txio  = printportx->print_txio;
	 printportcb.print_rxio  = printportx->print_rxio;
}


void printfsend(uint8_t* buf, int len)
{
    USART_SendDataFromBuff(printportcb.print_port,buf,len);
}
void MyPrintf(char *format, ...)
{ 
        uint32_t ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg;
        char *pcStr, pcBuf[16], cFill;
        char HexFormat;
    	va_list vaArgP;
        static const int8_t* const g_pcHex1 = "0123456789abcdef";
        static const int8_t* const g_pcHex2 = "0123456789ABCDEF";
    	va_start(vaArgP, format);

    	while(*format)
    	{
      	    // Find the first non-% character, or the end of the string.
            for(ulIdx = 0; (format[ulIdx] != '%') && (format[ulIdx] != '\0');ulIdx++)
            {}

        	// Write this portion of the string.
            if(ulIdx>0)	 
            {
                printfsend((uint8_t*)format, ulIdx);
            }
            
            format += ulIdx;

        if(*format == '%')
        {
            format++;

            // Set the digit count to zero, and the fill character to space
            // (i.e. to the defaults).
            ulCount = 0;
            cFill = ' ';

again:
           switch(*format++)
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    if((format[-1] == '0') && (ulCount == 0))
                    {
                        cFill = '0';
                    }

                    ulCount *= 10;
                    ulCount += format[-1] - '0';

                    goto again;
                }

                case 'c':
                {
                    ulValue = va_arg(vaArgP, unsigned long);
                    printfsend((uint8_t*)&ulValue, 1);
                    break;
                }

                case 'd':
                {
                    ulValue = va_arg(vaArgP, unsigned long);
                    ulPos = 0;

                    if((long)ulValue < 0)
                    {
                        ulValue = -(long)ulValue;
                        ulNeg = 1;
                    }
                    else
                    {
                        ulNeg = 0;
                    }

                    ulBase = 10;
                    goto convert;
                }

                case 's':
                {
                    pcStr = va_arg(vaArgP, char *);

                    for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
                    {}

                    printfsend((uint8_t*)pcStr, ulIdx);

                    if(ulCount > ulIdx)
                    {
                        ulCount -= ulIdx;
                        while(ulCount--)
                        {
                            printfsend(" ", 1);
                        }
                    }
                    break;
                }

                case 'u':
                {
                    ulValue = va_arg(vaArgP, unsigned long);
                    ulPos = 0;
                    ulBase = 10;
                    ulNeg = 0;
                    goto convert;
                }


                case 'X':
                {
                    ulValue = va_arg(vaArgP, unsigned long);
                    ulPos = 0;
                    ulBase = 16;
                    ulNeg = 0;
                    HexFormat='X';
                    goto convert;
                }

                case 'x':
                case 'p':
                {
                    ulValue = va_arg(vaArgP, unsigned long);
                    ulPos = 0;
                    ulBase = 16;
                    ulNeg = 0;
                     HexFormat='x';

convert:
                    for(ulIdx = 1;
                        (((ulIdx * ulBase) <= ulValue) &&
                         (((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, ulCount--)
                    {
                    }

                    if(ulNeg)
                    {
                        ulCount--;
                    }

                    if(ulNeg && (cFill == '0'))
                    {
                        pcBuf[ulPos++] = '-';
                        ulNeg = 0;
                    }

                    if((ulCount > 1) && (ulCount < 16))
                    {
                        for(ulCount--; ulCount; ulCount--)
                        {
                            pcBuf[ulPos++] = cFill;
                        }
                    }

                    if(ulNeg)
                    {
                        pcBuf[ulPos++] = '-';
                    }

                    for(; ulIdx; ulIdx /= ulBase)
                    {
                        if(HexFormat=='x')	pcBuf[ulPos++] = g_pcHex1[(ulValue / ulIdx) % ulBase];//x
                        else	pcBuf[ulPos++] = g_pcHex2[(ulValue / ulIdx) % ulBase];//X
                    }

                    printfsend((uint8_t*)pcBuf, ulPos);
                    break;
                }

                case '%':
                {
                    printfsend((uint8_t*)format - 1, 1);
                    break;
                }

                default:
                {
                    printfsend("ERROR", 5);
                    break;
                }
            }//switch
        }//if
    }//while

    va_end(vaArgP);
}

//void _assert_handler(const char* file, int line,const char* func)
//{
//	MyPrintf("Assert trigger at file: %s line:%d func: %s\n ",file,line,func);
//	while(1);
//}

static uint8_t is_open_log_print = 0;

void open_log_print(uint8_t isopen)
{
	is_open_log_print = isopen;
}


void log_print(char *format, ...)
{
	if(is_open_log_print)
    {
        MyPrintf(format);
    }
}

void log2_print(char *format, ...)
{
    if(is_open_log_print>1)
    {
       MyPrintf(format); 
    }
}

