#include "yc_debug.h"
#include <stdarg.h>
#include "yc_timer.h"


#ifdef FUNCTION_CONTROL_DEBUG_ENABLE
unsigned char uarttxbuf[DEBUG_PROTOCL_TOTAL_ALLOW_LENGTH], uartrxbuf[2];
unsigned char *txptr;

void uart_txBuffer(uint8_t* send_buf, uint32_t length)
{
	uint32_t i;
		
	for (i = 0; i < length; i++)
	{
		uart_txChar(*(send_buf+i));
	}
}

void uart_txChar(unsigned char c)
{
	while(HREAD(CORE_UARTB_TX_ITEMS) >= sizeof(uarttxbuf) - 10);
	*txptr = c;
	if(++txptr > uarttxbuf + sizeof(uarttxbuf) - 1) txptr = uarttxbuf;
	HWRITEW(CORE_UARTB_TX_WPTR, txptr);
}

void init_debugUart(void)
{
	HWOR(CORE_CONFIG, BIT_0);
	HWCOR(CORE_CLKOFF + 1, 0x80);

	//gpio config
	HWRITE(CORE_GPIO_CONF + DEBUG_GPIO_DEFINE, GPCFG_UARTB_TXD);
	HWRITE(CORE_GPIO_CONF + DEBUG_GPIO_DEFINER, GPCFG_UARTB_RXD);
	
	HWRITEW(CORE_UARTB_BAUD, 0x8034);// 921600
	//HWRITEW(CORE_UARTB_BAUD, 0x81bf); //115200
	HWRITEW(CORE_UARTB_TX_SADDR, (uint16_t)(uint32_t)&uarttxbuf);//write start addr
	HWRITEW(CORE_UARTB_TX_WPTR, (uint16_t)(uint32_t)&uarttxbuf); //init write pointer
	HWRITEW(CORE_UARTB_TX_EADDR, (uint16_t)((int)&uarttxbuf + sizeof(uarttxbuf) - 1));//write end addr

	HWRITEW(CORE_UARTB_RX_SADDR, (uint16_t)(uint32_t)&uartrxbuf); //read start addr
	HWRITEW(CORE_UARTB_RX_RPTR, (uint16_t)(uint32_t)&uartrxbuf);  //init read pointer
	HWRITEW(CORE_UARTB_RX_EADDR, (uint16_t)((int)&uartrxbuf + sizeof(uartrxbuf) - 1)); //read end addr
	
	HWRITE(CORE_UARTB_CTRL, 0x81);
	txptr = uarttxbuf;

	// Init other function
	init_print_log();
	return;
}

void debugUart_pollTx(void)
{
	// Type_Log print
	print_log_to_uart();
	return;
}




















void write_long_value(uint8_t* addr, uint32_t value)
{
	*(addr) = (value >> 0);
	*(addr + 1) = (value >> 8);
	*(addr + 2) = (value >> 16);
	*(addr + 3) = (value >> 24);
}

void print_round_buffer(uint8_t index, uint32_t start_addr, uint32_t end_addr, uint32_t write_addr, uint32_t read_addr)
{
	uint8_t uart_send_buf[DEBUG_PROTOCL_TOTAL_ALLOW_LENGTH];
	uint8_t uart_send_length = 0;
	uint8_t offset = 0;
	
	*(uart_send_buf + offset + DEBUG_PROTOCL_PAYLOAD_START_OFFSET) = index;
	offset = offset + 1;
	write_long_value((uart_send_buf + offset + DEBUG_PROTOCL_PAYLOAD_START_OFFSET), start_addr);
	offset = offset + 4;
	write_long_value((uart_send_buf + offset + DEBUG_PROTOCL_PAYLOAD_START_OFFSET), end_addr);
	offset = offset + 4;
	write_long_value((uart_send_buf + offset + DEBUG_PROTOCL_PAYLOAD_START_OFFSET), write_addr);
	offset = offset + 4;
	write_long_value((uart_send_buf + offset + DEBUG_PROTOCL_PAYLOAD_START_OFFSET), read_addr);
	offset = offset + 4;
	// Prepare packet
	yichip_debug_uart_protocol(DEBUG_PROTOCL_TYPE_ROUND_BUFFER, offset, uart_send_buf, &uart_send_length);

	uart_txBuffer(uart_send_buf, uart_send_length);
}



/**
  * To avoid uart lost info, we need add protocol to protect it, The input length must limit DEBUG_PROTOCL_PAYLOAD_ALLOW_LENGTH
  * To reduce timing, you should prepare payload into <uart_send_buf> in right point.
  *    1byte      1byte    1byte        nbytes         1byte
  * | 0xAA  | Type  | length  |  Payload   | 0xBB  |
  */
void yichip_debug_uart_protocol(uint8_t type, uint8_t length
	, uint8_t* uart_send_buf, uint8_t* uart_send_length_ptr)
{
	uint16_t offset = 0;
	*(uart_send_buf + offset) = 0xAA;
	offset ++;
	*(uart_send_buf + offset) = type;
	offset ++;
	*(uart_send_buf + offset) = length;
	offset ++;
	length = length > DEBUG_PROTOCL_PAYLOAD_ALLOW_LENGTH?DEBUG_PROTOCL_PAYLOAD_ALLOW_LENGTH:length;
	// DO NOTHING, to reduce timing
	//memcpy(uart_send_buf+offset, payload, length);
	offset += length;
	*(uart_send_buf + offset) = 0xBB;
	offset ++;
	*uart_send_length_ptr = offset;
}







// use share memory
//uint32_t log_bug[LOG_BUFFER_SIZE];
//uint16_t log_read_index;
//uint16_t log_write_index;
//uint8_t log_lock_flag;

uint32_t* log_bug_ptr;
volatile uint8_t* log_initial_flag_ptr;
// use Peterson's Algorithm
volatile uint8_t* log_lock_flag_cm0_ptr;
volatile uint8_t* log_lock_flag_respin_ptr;
volatile uint8_t* log_lock_victim_ptr;
volatile uint16_t* log_read_index_ptr;
volatile uint16_t* log_write_index_ptr;

void init_print_log(void)
{
	// Set ptr
	log_bug_ptr = (uint32_t *)DEBUG_LOG_BUF_START_ADDR;
	log_initial_flag_ptr = (uint8_t *)DEBUG_LOG_BUF_INITIAL_FLAG_ADDR;
	log_lock_flag_cm0_ptr = (uint8_t *)DEBUG_LOG_BUF_LOCK_FLAG_CM0_ADDR;
	log_lock_flag_respin_ptr = (uint8_t *)DEBUG_LOG_BUF_LOCK_FLAG_RESPIN_ADDR;
	log_lock_victim_ptr = (uint8_t *)DEBUG_LOG_BUF_LOCK_VICTIM_ADDR;
	log_read_index_ptr = (uint16_t *)DEBUG_LOG_BUF_READ_INDEX_ADDR;
	log_write_index_ptr = (uint16_t *)DEBUG_LOG_BUF_WRITE_INDEX_ADDR;
	
	if(*(log_initial_flag_ptr) != DEBUG_LOG_INITIAL_SUCCESS_FLAG)
	{
		memset((void *)log_bug_ptr, 0, LOG_BUFFER_SIZE*4);
		*(log_lock_flag_cm0_ptr) = 0;
		*(log_lock_flag_respin_ptr) = 0;
		*(log_lock_victim_ptr) = 0;
		*(log_read_index_ptr) = 0;
		*(log_write_index_ptr) = 0;
		
		*(log_initial_flag_ptr) = DEBUG_LOG_INITIAL_SUCCESS_FLAG;
	}
}

BOOL print_log_buffer_is_empty(void)
{
	volatile uint16_t* read_index_ptr = log_read_index_ptr;
	volatile uint16_t* write_index_ptr = log_write_index_ptr;
	uint16_t write_index;
	uint16_t read_index;
	OS_ENTER_CRITICAL();
	// get write lock
	print_log_get_lock();
	write_index = (*write_index_ptr);
	read_index = (*read_index_ptr);
	print_log_free_lock();
	OS_EXIT_CRITICAL();

	return (write_index == read_index);
}
void print_log_to_uart(void)
{
	volatile uint16_t* read_index_ptr = log_read_index_ptr;
	volatile uint16_t* write_index_ptr = log_write_index_ptr;
	uint16_t write_index;
	uint16_t read_index;
	uint16_t offset = 0;
	uint8_t uart_send_buf[DEBUG_PROTOCL_TOTAL_ALLOW_LENGTH];
	uint8_t uart_send_length = 0;
	OS_ENTER_CRITICAL();
	// get write lock
	print_log_get_lock();
	write_index = (*write_index_ptr);
	read_index = (*read_index_ptr);
	// To avoid protocol header lost timing
	while((write_index != read_index)
		&& ((offset + 4) < DEBUG_PROTOCL_PAYLOAD_ALLOW_LENGTH))// Here offset must add 4
	{
		memcpy((void *)(uart_send_buf + DEBUG_PROTOCL_PAYLOAD_START_OFFSET + offset)
			, log_bug_ptr + read_index, 4);
		read_index = (read_index + 1)%LOG_BUFFER_SIZE;
		*read_index_ptr = read_index;
		offset += 4;
	}
	print_log_free_lock();
	OS_EXIT_CRITICAL();

	// Real Send To Uart
	if(offset != 0)
	{
		// Prepare packet
		yichip_debug_uart_protocol(DEBUG_PROTOCL_TYPE_LOG, offset, uart_send_buf, &uart_send_length);

		uart_txBuffer(uart_send_buf, uart_send_length);
	}
}
uint16_t gHaltLogFlag;
uint16_t gHaltLogValue;
void print_log(uint16_t flag, uint16_t value)
{
	volatile uint16_t* read_index_ptr = log_read_index_ptr;
	volatile uint16_t* write_index_ptr = log_write_index_ptr;
	uint16_t write_index;
	uint16_t read_index;
	// Becareful, it will be error
	if(SystemCheckInFastSpeed())
	{
		gHaltLogFlag = flag;
		gHaltLogValue = value;
		_ASSERT_FAULT();
	}
	
	OS_ENTER_CRITICAL();
	// get lock
	print_log_get_lock();
	write_index = (*write_index_ptr);
	*(log_bug_ptr + write_index) = (flag << 16) | value;
	write_index = (write_index + 1)%LOG_BUFFER_SIZE;
#ifdef LOG_PROTECT_OVERFLOW
	// TODO: Here we should do something to resolve overflow
	read_index = (*read_index_ptr);
	if(write_index == read_index)
	{
		//to drop one log
		if(write_index == 0) 
		{
			write_index = LOG_BUFFER_SIZE - 1;
		}
		else
		{
			write_index --;
		}
	}
	*write_index_ptr = write_index;
#endif
	print_log_free_lock();
	OS_EXIT_CRITICAL();
	// Try to send log right now
	print_log_to_uart();
	return;
}


void print_log_get_lock(void)
{
	*(log_lock_flag_cm0_ptr) = 1;// I'm interested
	*(log_lock_victim_ptr) = DEBUG_LOG_LOCK_VICTIM_CM0;// you go first
	while(((*log_lock_flag_respin_ptr) != 0) 
		&& (*(log_lock_victim_ptr) == DEBUG_LOG_LOCK_VICTIM_CM0));// wait
	return;
}
void print_log_free_lock(void)
{
	*log_lock_flag_cm0_ptr = 0;
	return;
}



//*****************************************************************************
//
//! A simple  MyPrintf function supporting \%c, \%d, \%p, \%s, \%u,\%x, and \%X.
//!
//! \param format is the format string.
//! \param ... are the optional arguments, which depend on the contents of the
//! \return None.
//
//*****************************************************************************
void print_string_to_uart(uint8_t* buf, int len)
{
	uint8_t uart_send_buf[3];
	uint8_t offset = 0;

	if(len == 0)
	{
		return;
	}

	if(len > 255 - DEBUG_PROTOCL_HEADER_SIZE)
	{
		len = 255 - DEBUG_PROTOCL_HEADER_SIZE;
	}

	*(uart_send_buf + offset) = 0xAA;
	offset ++;
	*(uart_send_buf + offset) = DEBUG_PROTOCL_TYPE_STRING;
	offset ++;
	*(uart_send_buf + offset) = len;

	// send header
	uart_txBuffer(uart_send_buf, DEBUG_PROTOCL_PAYLOAD_START_OFFSET);
	
	uart_txBuffer(buf, len);

	// send trail
	*(uart_send_buf + 0) = 0xBB;
	uart_txBuffer(uart_send_buf, 1);
}

void print_string(uint8_t* uartSndBuf, uint8_t* uartSndBufUseLen, uint8_t* buf, int len)
{
	for(uint8_t i = 0; i < len; i ++)
	{
		if(*uartSndBufUseLen >= DEBUG_STRING_LOG_SINGLE_LINE_MAX_SIZE)
		{
			return;
		}
		*(uartSndBuf + *uartSndBufUseLen) = buf[i];

		*uartSndBufUseLen = *uartSndBufUseLen+1;
	}
}
void pringt_string_log(char *format, ...)
{ 
        uint32_t ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg;
        char *pcStr, pcBuf[16], cFill;
        char HexFormat;
    	va_list vaArgP;
        static const int8_t* const g_pcHex1 = "0123456789abcdef";
        static const int8_t* const g_pcHex2 = "0123456789ABCDEF";
    	va_start(vaArgP, format);
    	uint8_t uart_tx_buff[DEBUG_STRING_LOG_SINGLE_LINE_MAX_SIZE];
    	uint8_t uart_tx_local_ptr = 0;

    	while(*format)
    	{
      	    // Find the first non-% character, or the end of the string.
            for(ulIdx = 0; (format[ulIdx] != '%') && (format[ulIdx] != '\0');ulIdx++)
            {}

        	// Write this portion of the string.
            if(ulIdx>0)	 
            {
                print_string(uart_tx_buff, &uart_tx_local_ptr, (uint8_t*)format, ulIdx);
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
                    print_string(uart_tx_buff, &uart_tx_local_ptr, (uint8_t*)&ulValue, 1);
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

                    print_string(uart_tx_buff, &uart_tx_local_ptr, (uint8_t*)pcStr, ulIdx);

                    if(ulCount > ulIdx)
                    {
                        ulCount -= ulIdx;
                        while(ulCount--)
                        {
                            print_string(uart_tx_buff, &uart_tx_local_ptr, " ", 1);
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

                    print_string(uart_tx_buff, &uart_tx_local_ptr, (uint8_t*)pcBuf, ulPos);
                    break;
                }

                case '%':
                {
                    print_string(uart_tx_buff, &uart_tx_local_ptr, (uint8_t*)format - 1, 1);
                    break;
                }

                default:
                {
                    print_string(uart_tx_buff, &uart_tx_local_ptr, "ERROR", 5);
                    break;
                }
            }//switch
        }//if
    }//while

    va_end(vaArgP);

    print_string_to_uart(uart_tx_buff, uart_tx_local_ptr);
}




void print_protocol_to_uart(uint16_t logLevel, uint16_t logType, uint8_t* buf, int len)
{
	uint8_t uart_send_buf[3];
	uint8_t offset = 0;

	if(len == 0)
	{
		return;
	}

	if(len > 255 - DEBUG_PROTOCL_HEADER_SIZE)
	{
		len = 255 - DEBUG_PROTOCL_HEADER_SIZE;
	}

	*(uart_send_buf + offset) = 0xAA;
	offset ++;
	*(uart_send_buf + offset) = DEBUG_PROTOCL_TYPE_PROTOCOL;
	offset ++;
	*(uart_send_buf + offset) = len + 4;

	// send header
	uart_txBuffer(uart_send_buf, DEBUG_PROTOCL_PAYLOAD_START_OFFSET);

	offset = 0;
	*(uart_send_buf + offset) = logLevel& 0xff;
	*(uart_send_buf + offset + 1) = (logLevel >> 8) & 0xff;
	offset += 2;
	*(uart_send_buf + offset) = logType & 0xff;
	*(uart_send_buf + offset + 1) = (logType >> 8) & 0xff;
	offset += 2;
	// send sub header
	uart_txBuffer(uart_send_buf, offset);
	
	uart_txBuffer(buf, len);

	// send trail
	*(uart_send_buf + 0) = 0xBB;
	uart_txBuffer(uart_send_buf, 1);
}

void print_hci_h4_to_uart(uint8_t isRcv, uint8_t* buf, int len)
{
	uint8_t uart_send_buf[3];
	uint8_t offset = 0;

	if(len == 0)
	{
		return;
	}

	*(uart_send_buf + offset) = 0xAA;
	offset ++;
	*(uart_send_buf + offset) = DEBUG_PROTOCL_TYPE_HCI_H4;
	offset ++;
	*(uart_send_buf + offset) = len + 1;
	offset ++;
	*(uart_send_buf + offset) = (len + 1) >> 8;

	// send header
	uart_txBuffer(uart_send_buf, DEBUG_PROTOCL_BIG_PAYLOAD_START_OFFSET);

	offset = 0;
	*(uart_send_buf + offset) = isRcv;
	offset ++;
	// send sub header
	uart_txBuffer(uart_send_buf, offset);
	
	uart_txBuffer(buf, len);

	// send trail
	*(uart_send_buf + 0) = 0xBB;
	uart_txBuffer(uart_send_buf, 1);
}

#endif


void print_string_empty_fuction(char *format, ...)
{

}

