
Stack_Size      EQU     0x00B640

                AREA    STACK, NOINIT, READWRITE, ALIGN=4
Stack_Mem       SPACE   Stack_Size
__initial_sp    EQU		0x1001B640


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00001000
                AREA    HEAP, NOINIT, READWRITE, ALIGN=4
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB
; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY	
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size
__Vectors       DCD     __initial_sp				; Top of Stack
                DCD     Reset_Handler				; Reset Handler
                DCD     NMI_Handler
                DCD     HardFault_Handler			; Hard Fault Handler					
				DCD		0							;	
				DCD		0							;
				DCD		0							;
				DCD		0							;
				DCD		0							;
				DCD		0							;
				DCD		0							;
				DCD		SVC_Handler					;
				DCD		0							;
				DCD		0							;
				DCD		PendSV_Handler				;
				DCD		SysTick_Handler				;	
					
				DCD		USB_IRQHandler              ;
				DCD		IIC_IRQHandler              ;
				DCD		QSPI_IRQHandler             ;
				DCD		SPI_IRQHandler              ;
				DCD		UART_IRQHandler             ;
				DCD		UARTB_IRQHandler            ;
				DCD		ADC_IRQHandler              ;
				DCD		IIS_IRQHandler              ;
				DCD		BT_IRQHandler               ;
				DCD     GPIO_IRQHandler				;
				DCD     OTP_IRQHandler				;
				DCD     SBC_ERR0_IRQHandler			;
				DCD     SBC_ERR1_IRQHandler			;
				DCD     FFT_IRQHandler				;
				DCD     DMA_IRQHandler				;
				DCD     SD_IRQHandler				;
				DCD     TIMER0_IRQHandler			;
				DCD     TIMER1_IRQHandler			;
				DCD     TIMER2_IRQHandler			;
				DCD     TIMER3_IRQHandler			;
				DCD     TIMER4_IRQHandler			;
				DCD     TIMER5_IRQHandler			;
				DCD     TIMER6_IRQHandler			;
				DCD     TIMER7_IRQHandler			;
				DCD     WDT2_IRQHandler				;
				DCD     WDT_IRQHandler				;
				DCD     0							;
				DCD     0							;
				DCD     0							;
				DCD     0							;
				DCD     0							;
				DCD     0							;

__Vectors_End					
					
__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY	,ALIGN=4				
					
					
					
; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  __main
                LDR     R0,= LOADRAMFLAG
                LDR     R0,[R0]
                LDRB    R0,[R0]
                CMP     R0,#1
                BEQ     RUN_TO_MAIN		
				LDR 	R0, =__main
				BX 		R0
                ENDP	

RUN_TO_MAIN		PROC
                IMPORT  main
                LDR 	R0, = main
                BX 		R0
                ENDP
				
                align 4
; mem_wake_flag
LOADRAMFLAG     DCD    0x10004429		
Default_Handler PROC
; ToDo:  Add here the export definition for the device specific external interrupts handler
				EXPORT      HardFault_Handler   [WEAK]
				EXPORT      NMI_Handler		    [WEAK]
				EXPORT      SVC_Handler		    [WEAK]
				EXPORT      PendSV_Handler		[WEAK]
                EXPORT 		SysTick_Handler		[WEAK]
				EXPORT		USB_IRQHandler      [WEAK]
				EXPORT		IIC_IRQHandler      [WEAK]
				EXPORT		QSPI_IRQHandler     [WEAK]
				EXPORT		SPI_IRQHandler      [WEAK]
				EXPORT		UART_IRQHandler     [WEAK]
				EXPORT		UARTB_IRQHandler    [WEAK]
				EXPORT 		ADC_IRQHandler      [WEAK]				
				EXPORT		IIS_IRQHandler      [WEAK]
				EXPORT		BT_IRQHandler       [WEAK]
				EXPORT		GPIO_IRQHandler		[WEAK]
				EXPORT		OTP_IRQHandler		[WEAK]
				EXPORT		SBC_ERR0_IRQHandler	[WEAK]
				EXPORT		SBC_ERR1_IRQHandler	[WEAK]
				EXPORT		FFT_IRQHandler		[WEAK]
				EXPORT		DMA_IRQHandler		[WEAK]
				EXPORT		SD_IRQHandler		[WEAK]
				EXPORT		TIMER0_IRQHandler	[WEAK]
				EXPORT		TIMER1_IRQHandler	[WEAK]
				EXPORT		TIMER2_IRQHandler	[WEAK]
				EXPORT		TIMER3_IRQHandler	[WEAK]	
				EXPORT		TIMER4_IRQHandler	[WEAK]	
				EXPORT		TIMER5_IRQHandler	[WEAK]	
				EXPORT		TIMER6_IRQHandler	[WEAK]	
				EXPORT 		TIMER7_IRQHandler	[WEAK]

				EXPORT 		WDT2_IRQHandler		[WEAK]
				EXPORT 		WDT_IRQHandler		[WEAK]

; ToDo:  Add here the names for the device specific external interrupts handler
HardFault_Handler
NMI_Handler
SVC_Handler
PendSV_Handler
SysTick_Handler
USB_IRQHandler          	
IIC_IRQHandler          	
QSPI_IRQHandler         	
SPI_IRQHandler          	 	
UART_IRQHandler         	
UARTB_IRQHandler        	
ADC_IRQHandler          
IIS_IRQHandler          
BT_IRQHandler           			
GPIO_IRQHandler			
OTP_IRQHandler			
SBC_ERR0_IRQHandler		
SBC_ERR1_IRQHandler		
FFT_IRQHandler			
DMA_IRQHandler			
SD_IRQHandler			
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
TIMER4_IRQHandler
TIMER5_IRQHandler
TIMER6_IRQHandler
TIMER7_IRQHandler

WDT2_IRQHandler
WDT_IRQHandler
                B       .
                ENDP												
		
				 ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
					
                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF
					
				END		