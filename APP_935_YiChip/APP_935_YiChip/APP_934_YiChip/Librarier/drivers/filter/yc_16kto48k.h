#ifndef DRIVERS_YC_16KTO48K_H_
#define DRIVERS_YC_16KTO48K_H_
#include <stdint.h>
#include "type.h"

void Filter16KTo48KInit(short * dataStartAddr, uint16_t size);
void Filter16KTo48KWork(short data);
uint16_t Filter16KTo48KGetWptr(void);
#endif //DRIVERS_VP_YC_VP_H_

