#ifndef DRIVERS_YC_8KTO16K_H_
#define DRIVERS_YC_8KTO16K_H_
#include <stdint.h>
#include "type.h"


void Filter8KTo16KInit(short * dataStartAddr, uint16_t size);
void Filter8KTo16KWork(short data);
uint16_t Filter8KTo16KGetWptr(void);
#endif //DRIVERS_YC_8KTO16K_H_

