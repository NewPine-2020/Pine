#ifndef DRIVERS_YC_MEDIAFARROW_H_
#define DRIVERS_YC_MEDIAFARROW_H_
#include <stdint.h>
#include "type.h"

void FilterMediaFarrowInit(uint8_t dac_clk, short * dataStartAddr, uint16_t size);
void FilterMediaFarrowWork(uint8_t dac_clk, int16_t din_point);
uint16_t FilterMediaFarrowGetWptr(void);
void FilterMediaFarrowValue(int32_t interp_out);

void FilterMediaFarrowUpdateSampleDiff(int32_t sampleDiff);
void FilterMediaFarrowUpdateDeltaUk(int32_t uk);
#endif //DRIVERS_VP_YC_VP_H_

