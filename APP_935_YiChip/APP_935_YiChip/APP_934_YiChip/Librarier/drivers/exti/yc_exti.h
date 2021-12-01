#ifndef _EXTI_H_
#define _EXTI_H_
#include "ycdef.h"
#include "yc11xx.h"
//#include "core_cm0.h"
typedef enum{DISABLE =0, ENABLE =1} FunctionalState;

void NVIC_Config(IRQn_Type IRQn , FunctionalState enable );
void GPIO_Interrupt_Config();

#endif