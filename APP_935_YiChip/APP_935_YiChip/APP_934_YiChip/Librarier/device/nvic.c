
#include "nvic.h"

/**
  * @brief  Enable External Interrupt
  * @param  IRQnx  IRQn  External interrupt number. Value cannot be negative.
  * @retval none
  */
void NVIC_EnableIRQ(IRQn_Type IRQnx)
{
	enable_intr((int) IRQnx);
}

/**
  * @brief  Disable External Interrupt
  * @param  IRQnx  IRQn  External interrupt number. Value cannot be negative.
  * @retval  none
  */
void NVIC_DisableIRQ(IRQn_Type IRQnx)
{
	disable_intr((int)IRQnx); 
}  

/**
 * @brief  Set External  Interrupt Priority
 * @param  IRQn  Interrupt number.
 *@retval  priority  PreemptPriority to set.the value can be 0,1,2,3
 */
void NVIC_SetPriority(IRQn_Type IRQn, uint8_t priority)
{	
	uint8_t temp = IRQn/4;
	NVIC_IP(temp)=((priority << 6) & 0xff)<<((IRQn%4)*8); 
}



