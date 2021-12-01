#ifndef __NVIC_H__
#define __NVIC_H__

#include "yc11xx.h"
#include "type.h"

#define NVIC_IP_BASE_ADR			(volatile int*)(0xE000E400) 
#define NVIC_IP(x)						*((volatile int*)(0xE000E400 + x*4))

/**
  * @brief  Enable External Interrupt
  * @param  IRQnx  IRQn  External interrupt number. Value cannot be negative.
  * @retval none
  */
void NVIC_EnableIRQ(IRQn_Type IRQnx);


/**
  * @brief  Disable External Interrupt
  * @param  IRQnx  IRQn  External interrupt number. Value cannot be negative.
  * @retval  none
  */
void NVIC_DisableIRQ(IRQn_Type IRQnx);


/**
 * @brief  Set External  Interrupt Priority
 * @param  IRQn  Interrupt number.
 *@retval  priority  PreemptPriority to set.the value can be 0,1,2,3
 */
void NVIC_SetPriority(IRQn_Type IRQn, uint8_t priority);

#endif /* __MISC_H */

