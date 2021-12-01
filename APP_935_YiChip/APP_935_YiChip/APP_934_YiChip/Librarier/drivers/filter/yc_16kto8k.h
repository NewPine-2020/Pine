#ifndef DRIVERS_YC_16KTO8K_H_
#define DRIVERS_YC_16KTO8K_H_
#include <stdint.h>
#include "type.h"

void Filter16KTo8KInit(short * dataStartAddr, uint16_t size);
void Filter16KTo8KWork(short ps16InData0, short ps16InData1);
uint16_t Filter16KTo8KGetWptr(void);
#endif //DRIVERS_YC_16KTO8K_H_

