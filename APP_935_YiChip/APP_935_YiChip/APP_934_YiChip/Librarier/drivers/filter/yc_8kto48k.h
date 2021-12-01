#ifndef DRIVERS_YC_8KTO48K_H_
#define DRIVERS_YC_8KTO48K_H_
#include <stdint.h>
#include "type.h"

void Filter8KTo48KInit(short * dataStartAddr, uint16_t size);
void Filter8KTo48KWork(short data);
uint16_t Filter8KTo48KGetWptr(void);
void Filter8KTo48KUpdateSampleDiff(int32_t sampleDiff);
#endif //DRIVERS_VP_YC_VP_H_

