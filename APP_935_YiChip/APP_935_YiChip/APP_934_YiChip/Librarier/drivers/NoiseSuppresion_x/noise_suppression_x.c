/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "noise_suppression_x.h"
#include "yc_debug.h"
#include "yc_timer.h"
#include "app_common.h"
//#include <assert.h>

#include <stdlib.h>
#include <stdint.h>
#define m0_reg_base             0x1f000
#define m0_fft_ctrl             m0_reg_base+0x40
#define m0_fft_size_ind         m0_reg_base+0x44
#define m0_fft_src_addrh        m0_reg_base+0x48
#define m0_fft_src_addrl        m0_reg_base+0x4c
#define m0_fft_calc_addrh       m0_reg_base+0x50
#define m0_fft_calc_addrl       m0_reg_base+0x54
#define m0_fft_int_status       m0_reg_base+0x58
#define m0_fft_int_clr          m0_reg_base+0x5c
#define m0_fft_scal_h           m0_reg_base+0x60
#define m0_fft_scal_l           m0_reg_base+0x64

#ifndef nullptr
#define nullptr 0
#endif

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

enum nsLevel {
    kLow,
    kModerate,
    kHigh,
    kVeryHigh
};
NsxInst_t  g_NsxHandles BIG_MEMORY_LPM_NOT_PROTECT_ATTRIBUTES;
int16_t g_frameBuffer[256] BIG_MEMORY_LPM_NOT_PROTECT_ATTRIBUTES;
enum nsLevel NRlevel;
extern void Bt_CVSDDataPolling(void);
int nsProcess_init(void)
{
	NsxInst_t *NsxHandles  = &g_NsxHandles;
	uint32_t rate;
	if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	{
		rate=16000;
		NRlevel=kLow;
	}
	else
	{
		rate=8000;
		NRlevel=kVeryHigh;
	}
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D103, NsxHandles);
	if (NsxHandles != NULL) {
		int status = WebRtcNsx_Init((NsxHandle*)NsxHandles, rate);
		if (status != 0) {
			WebRtcNsx_Free((NsxHandle*)NsxHandles);
			NsxHandles = NULL;
			_ASSERT_FAULT();
		} else {
			status = WebRtcNsx_set_policy((NsxHandle*)NsxHandles, NRlevel);
			if (status != 0) {
				WebRtcNsx_Free((NsxHandle*)NsxHandles);
				NsxHandles = NULL;
				_ASSERT_FAULT();
			}
		}
		if (NsxHandles == NULL) {
			WebRtcNsx_Free((NsxHandle*)NsxHandles);
			_ASSERT_FAULT();
			return -1;
		}
    }
  //  DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D103, NsxHandles);
    return 1;

}


int nsProcess(int16_t *buffer, uint32_t sampleRate, uint64_t samplesCount, uint32_t channels, enum nsLevel level) {
    if (buffer == nullptr) return -1;
    if (samplesCount == 0) return -1;
	size_t samples = 0;
	if (sampleRate == 16000)
	{
		samples = 128;
	//	fir_16to8(buffer,input);
	}
	if (sampleRate == 8000)
	{
		samples = 64;
		// input = buffer;
	}
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D100, sampleRate);
	
    if (samples == 0) return -1;
    int16_t *input = buffer;
    size_t frames = (samplesCount / (samples * channels));
   int16_t *frameBuffer=g_frameBuffer;
   NsxInst_t *NsxHandles = &g_NsxHandles;
//  DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D103, NsxHandles);

// DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D101, 0x20);
    for (int i = 0; i < frames; i++) 
    {
    	    int c=0;
            for (int k = 0; k < samples; k++)
            {
                frameBuffer[k] = input[k * channels + c];
            }
	     int16_t nsIn[128] = { 0 }, nsOut[128] = {0};
	    if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
	    {
		 memcpy(nsIn, frameBuffer, 128 * sizeof(short));
	     	 memcpy(nsOut, frameBuffer+128, 128 * sizeof(short));	
	    }
	    else
	    {
	    	memcpy(nsIn, frameBuffer, 64 * sizeof(short));
	        memcpy(nsOut, frameBuffer+64, 64 * sizeof(short));
	    }
	     
	     int nRet=WebRtcNsx_Process((NsxHandle*)NsxHandles, nsIn,NULL, nsOut, NULL);
	     if (nRet != 0) 
			{
	//			printf("failed in WebRtcNsx_Process\n");
				WebRtcNsx_Free((NsxHandle*)NsxHandles);
				_ASSERT_FAULT();
				return -1;
			}


            for (int k = 0; k < samples; k++)
                input[k * channels + c] = nsOut[k];
        input += samples * channels;
    }
//    free(NsxHandles);
//    free(frameBuffer);
    return 1;
}

void noise_suppression(int16_t *in_file, int16_t *out_file) {

    uint32_t sampleRate; 
    uint64_t inSampleCount; 
    uint32_t buffersize;
    uint32_t channels = 1;
 
    int16_t *inBuffer = in_file;
    if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
    {
		sampleRate=16000;
		inSampleCount=128;
		buffersize=256;
    }
    else
    {
    		sampleRate=8000;
		inSampleCount=64;
		buffersize=128;	
    }
   SYS_ChangeToFastDPLL();
    if (inBuffer != nullptr) {
		nsProcess((int16_t *)inBuffer, sampleRate, inSampleCount, channels,NRlevel);
		memcpy((void*)out_file,(void*)inBuffer,buffersize);
        	//free(inBuffer);
    }
   SYS_ChangeToNormalDPLL();
}

int fft_calc(int16_t* complex_buffer,int order,int fft_mode,int fix_coeff)
{
    int calc_buf[1024];
    int scaler_factor;
    uint8_t addrh,addrl,tmp;
    uint8_t int_status=0;
	HWRITE2(m0_fft_int_clr,1);
	HWRITE2(m0_fft_int_clr,0);
	HWRITE2 (m0_fft_ctrl,0);
//    *(volatile byte*) (m0_fft_int_clr)=1);
//    *(volatile byte*) (m0_fft_int_clr,0);//interrupt clear
//    *(volatile byte*) (m0_fft_ctrl,0);

    addrh = ((int)(&complex_buffer[0]))>>10;
    addrl = ((int)(&complex_buffer[0]))>>2;    
   HWRITE2(m0_fft_src_addrh,addrh); 
   HWRITE2(m0_fft_src_addrl,addrl); 
//    *(volatile byte*) (m0_fft_src_addrh,addrh); 
//    *(volatile byte*) (m0_fft_src_addrl,addrl); 
    addrh = ((int)(&calc_buf[0]))>>10;
    addrl = ((int)(&calc_buf[0]))>>2;
    	HWRITE2 (m0_fft_calc_addrh,addrh); 
    	HWRITE2 (m0_fft_calc_addrl,addrl);
//    *(volatile byte*) (m0_fft_calc_addrh,addrh); 
//    *(volatile byte*) (m0_fft_calc_addrl,addrl); 
    tmp=order-6;
    HWRITE2(m0_fft_size_ind,tmp); 
    HWRITE2(m0_fft_ctrl,4); 
//    *(volatile byte*) (m0_fft_size_ind,tmp); 
//    *(volatile byte*) (m0_fft_ctrl,4); //clock enable
    tmp = (fix_coeff << 3) | (1 << 2) | (fft_mode << 1) | 1;
    HWRITE2(m0_fft_ctrl,tmp);
//    *(volatile byte*) (m0_fft_ctrl,tmp); //clock enable

    while(int_status==0) 
    {
    	int_status = HREAD2 (m0_fft_int_status);
    }
    addrh= HREAD2(m0_fft_scal_h);
    addrl= HREAD2(m0_fft_scal_l);
   /* if(order != 7)
    {
        OS_ENTER_CRITICAL();
        _ASSERT_FAULT();
    }*/
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D105, order);
  if(order ==7 ||order ==8)
  {
	scaler_factor =  ((addrl&0xc0)>>6) + ((addrl&0x30)>>4) + ((addrl&0xc)>>2) + (addrl&0x3);
  }
  else if(order ==9)
  {
    scaler_factor = (addrh&0x3) + ((addrl&0xc0)>>6) + ((addrl&0x30)>>4) + ((addrl&0xc)>>2) + (addrl&0x3);
   }
   else
   {
	scaler_factor =   ((addrl&0x30)>>4) + ((addrl&0xc)>>2) + (addrl&0x3);
   }
     //DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D102, scaler_factor);
  //    DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D105, order);
  //   DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D102, 0x40);
    return scaler_factor;
}


static const int16_t index_7[112] = {
		1, 64, 2, 32, 3, 96, 4, 16, 5, 80, 6, 48, 7, 112, 9, 72, 10, 40, 11, 104,
		12, 24, 13, 88, 14, 56, 15, 120, 17, 68, 18, 36, 19, 100, 21, 84, 22, 52,
		23, 116, 25, 76, 26, 44, 27, 108, 29, 92, 30, 60, 31, 124, 33, 66, 35, 98,
		37, 82, 38, 50, 39, 114, 41, 74, 43, 106, 45, 90, 46, 58, 47, 122, 49, 70,
		51, 102, 53, 86, 55, 118, 57, 78, 59, 110, 61, 94, 63, 126, 67, 97, 69,
		81, 71, 113, 75, 105, 77, 89, 79, 121, 83, 101, 87, 117, 91, 109, 95, 125,
		103, 115, 111, 123
};

/* Indexes for the case of stages == 8. */
static const int16_t index_8[240] = {
		1, 128, 2, 64, 3, 192, 4, 32, 5, 160, 6, 96, 7, 224, 8, 16, 9, 144, 10, 80,
		11, 208, 12, 48, 13, 176, 14, 112, 15, 240, 17, 136, 18, 72, 19, 200, 20,
		40, 21, 168, 22, 104, 23, 232, 25, 152, 26, 88, 27, 216, 28, 56, 29, 184,
		30, 120, 31, 248, 33, 132, 34, 68, 35, 196, 37, 164, 38, 100, 39, 228, 41,
		148, 42, 84, 43, 212, 44, 52, 45, 180, 46, 116, 47, 244, 49, 140, 50, 76,
		51, 204, 53, 172, 54, 108, 55, 236, 57, 156, 58, 92, 59, 220, 61, 188, 62,
		124, 63, 252, 65, 130, 67, 194, 69, 162, 70, 98, 71, 226, 73, 146, 74, 82,
		75, 210, 77, 178, 78, 114, 79, 242, 81, 138, 83, 202, 85, 170, 86, 106, 87,
		234, 89, 154, 91, 218, 93, 186, 94, 122, 95, 250, 97, 134, 99, 198, 101,
		166, 103, 230, 105, 150, 107, 214, 109, 182, 110, 118, 111, 246, 113, 142,
		115, 206, 117, 174, 119, 238, 121, 158, 123, 222, 125, 190, 127, 254, 131,
		193, 133, 161, 135, 225, 137, 145, 139, 209, 141, 177, 143, 241, 147, 201,
		149, 169, 151, 233, 155, 217, 157, 185, 159, 249, 163, 197, 167, 229, 171,
		213, 173, 181, 175, 245, 179, 205, 183, 237, 187, 221, 191, 253, 199, 227,
		203, 211, 207, 243, 215, 235, 223, 251, 239, 247
};

void WebRtcSpl_ComplexBitReverse(int16_t* __restrict complex_data, int stages) {
	/* For any specific value of stages, we know exactly the indexes that are
	 * bit reversed. Currently (Feb. 2012) in WebRTC the only possible values of
	 * stages are 7 and 8, so we use tables to save unnecessary iterations and
	 * calculations for these two cases.
	 */
	if (stages == 7 || stages == 8) {
		int m = 0;
		int length = 112;
		const int16_t* index = index_7;

		if (stages == 8) {
			length = 240;
			index = index_8;
		}

		/* Decimation in time. Swap the elements with bit-reversed indexes. */
		for (m = 0; m < length; m += 2) {
			/* We declare a int32_t* type pointer, to load both the 16-bit real
			 * and imaginary elements from complex_data in one instruction, reducing
			 * complexity.
			 */
			int32_t* complex_data_ptr = (int32_t*)complex_data;
			int32_t temp = 0;

			temp = complex_data_ptr[index[m]];  /* Real and imaginary */
			complex_data_ptr[index[m]] = complex_data_ptr[index[m + 1]];
			complex_data_ptr[index[m + 1]] = temp;
		}
	}
	else {
		int m = 0, mr = 0, l = 0;
		int n = 1 << stages;
		int nn = n - 1;

		/* Decimation in time - re-order data */
		for (m = 1; m <= nn; ++m) {
			int32_t* complex_data_ptr = (int32_t*)complex_data;
			int32_t temp = 0;

			/* Find out indexes that are bit-reversed. */
			l = n;
			do {
				l >>= 1;
			} while (l > nn - mr);
			mr = (mr & (l - 1)) + l;

			if (mr <= m) {
				continue;
			}

			/* Swap the elements with bit-reversed indexes.
			 * This is similar to the loop in the stages == 7 or 8 cases.
			 */
			temp = complex_data_ptr[m];  /* Real and imaginary */
			complex_data_ptr[m] = complex_data_ptr[mr];
			complex_data_ptr[mr] = temp;
		}
	}
}

#define CFFTSFT 14
#define CFFTRND 1
#define CFFTRND2 16384

#define CIFFTSFT 14
#define CIFFTRND 1


static const int16_t kSinTable1024[] = {
		0, 201, 402, 603, 804, 1005, 1206, 1406,
		1607, 1808, 2009, 2209, 2410, 2610, 2811, 3011,
		3211, 3411, 3611, 3811, 4011, 4210, 4409, 4608,
		4807, 5006, 5205, 5403, 5601, 5799, 5997, 6195,
		6392, 6589, 6786, 6982, 7179, 7375, 7571, 7766,
		7961, 8156, 8351, 8545, 8739, 8932, 9126, 9319,
		9511, 9703, 9895, 10087, 10278, 10469, 10659, 10849,
		11038, 11227, 11416, 11604, 11792, 11980, 12166, 12353,
		12539, 12724, 12909, 13094, 13278, 13462, 13645, 13827,
		14009, 14191, 14372, 14552, 14732, 14911, 15090, 15268,
		15446, 15623, 15799, 15975, 16150, 16325, 16499, 16672,
		16845, 17017, 17189, 17360, 17530, 17699, 17868, 18036,
		18204, 18371, 18537, 18702, 18867, 19031, 19194, 19357,
		19519, 19680, 19840, 20000, 20159, 20317, 20474, 20631,
		20787, 20942, 21096, 21249, 21402, 21554, 21705, 21855,
		22004, 22153, 22301, 22448, 22594, 22739, 22883, 23027,
		23169, 23311, 23452, 23592, 23731, 23869, 24006, 24143,
		24278, 24413, 24546, 24679, 24811, 24942, 25072, 25201,
		25329, 25456, 25582, 25707, 25831, 25954, 26077, 26198,
		26318, 26437, 26556, 26673, 26789, 26905, 27019, 27132,
		27244, 27355, 27466, 27575, 27683, 27790, 27896, 28001,
		28105, 28208, 28309, 28410, 28510, 28608, 28706, 28802,
		28897, 28992, 29085, 29177, 29268, 29358, 29446, 29534,
		29621, 29706, 29790, 29873, 29955, 30036, 30116, 30195,
		30272, 30349, 30424, 30498, 30571, 30643, 30713, 30783,
		30851, 30918, 30984, 31049, 31113, 31175, 31236, 31297,
		31356, 31413, 31470, 31525, 31580, 31633, 31684, 31735,
		31785, 31833, 31880, 31926, 31970, 32014, 32056, 32097,
		32137, 32176, 32213, 32249, 32284, 32318, 32350, 32382,
		32412, 32441, 32468, 32495, 32520, 32544, 32567, 32588,
		32609, 32628, 32646, 32662, 32678, 32692, 32705, 32717,
		32727, 32736, 32744, 32751, 32757, 32761, 32764, 32766,
		32767, 32766, 32764, 32761, 32757, 32751, 32744, 32736,
		32727, 32717, 32705, 32692, 32678, 32662, 32646, 32628,
		32609, 32588, 32567, 32544, 32520, 32495, 32468, 32441,
		32412, 32382, 32350, 32318, 32284, 32249, 32213, 32176,
		32137, 32097, 32056, 32014, 31970, 31926, 31880, 31833,
		31785, 31735, 31684, 31633, 31580, 31525, 31470, 31413,
		31356, 31297, 31236, 31175, 31113, 31049, 30984, 30918,
		30851, 30783, 30713, 30643, 30571, 30498, 30424, 30349,
		30272, 30195, 30116, 30036, 29955, 29873, 29790, 29706,
		29621, 29534, 29446, 29358, 29268, 29177, 29085, 28992,
		28897, 28802, 28706, 28608, 28510, 28410, 28309, 28208,
		28105, 28001, 27896, 27790, 27683, 27575, 27466, 27355,
		27244, 27132, 27019, 26905, 26789, 26673, 26556, 26437,
		26318, 26198, 26077, 25954, 25831, 25707, 25582, 25456,
		25329, 25201, 25072, 24942, 24811, 24679, 24546, 24413,
		24278, 24143, 24006, 23869, 23731, 23592, 23452, 23311,
		23169, 23027, 22883, 22739, 22594, 22448, 22301, 22153,
		22004, 21855, 21705, 21554, 21402, 21249, 21096, 20942,
		20787, 20631, 20474, 20317, 20159, 20000, 19840, 19680,
		19519, 19357, 19194, 19031, 18867, 18702, 18537, 18371,
		18204, 18036, 17868, 17699, 17530, 17360, 17189, 17017,
		16845, 16672, 16499, 16325, 16150, 15975, 15799, 15623,
		15446, 15268, 15090, 14911, 14732, 14552, 14372, 14191,
		14009, 13827, 13645, 13462, 13278, 13094, 12909, 12724,
		12539, 12353, 12166, 11980, 11792, 11604, 11416, 11227,
		11038, 10849, 10659, 10469, 10278, 10087, 9895, 9703,
		9511, 9319, 9126, 8932, 8739, 8545, 8351, 8156,
		7961, 7766, 7571, 7375, 7179, 6982, 6786, 6589,
		6392, 6195, 5997, 5799, 5601, 5403, 5205, 5006,
		4807, 4608, 4409, 4210, 4011, 3811, 3611, 3411,
		3211, 3011, 2811, 2610, 2410, 2209, 2009, 1808,
		1607, 1406, 1206, 1005, 804, 603, 402, 201,
		0, -201, -402, -603, -804, -1005, -1206, -1406,
		-1607, -1808, -2009, -2209, -2410, -2610, -2811, -3011,
		-3211, -3411, -3611, -3811, -4011, -4210, -4409, -4608,
		-4807, -5006, -5205, -5403, -5601, -5799, -5997, -6195,
		-6392, -6589, -6786, -6982, -7179, -7375, -7571, -7766,
		-7961, -8156, -8351, -8545, -8739, -8932, -9126, -9319,
		-9511, -9703, -9895, -10087, -10278, -10469, -10659, -10849,
		-11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
		-12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
		-14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
		-15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
		-16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
		-18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
		-19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
		-20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
		-22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
		-23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
		-24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
		-25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
		-26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
		-27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
		-28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
		-28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
		-29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
		-30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
		-30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
		-31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
		-31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
		-32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
		-32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
		-32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
		-32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
		-32767, -32766, -32764, -32761, -32757, -32751, -32744, -32736,
		-32727, -32717, -32705, -32692, -32678, -32662, -32646, -32628,
		-32609, -32588, -32567, -32544, -32520, -32495, -32468, -32441,
		-32412, -32382, -32350, -32318, -32284, -32249, -32213, -32176,
		-32137, -32097, -32056, -32014, -31970, -31926, -31880, -31833,
		-31785, -31735, -31684, -31633, -31580, -31525, -31470, -31413,
		-31356, -31297, -31236, -31175, -31113, -31049, -30984, -30918,
		-30851, -30783, -30713, -30643, -30571, -30498, -30424, -30349,
		-30272, -30195, -30116, -30036, -29955, -29873, -29790, -29706,
		-29621, -29534, -29446, -29358, -29268, -29177, -29085, -28992,
		-28897, -28802, -28706, -28608, -28510, -28410, -28309, -28208,
		-28105, -28001, -27896, -27790, -27683, -27575, -27466, -27355,
		-27244, -27132, -27019, -26905, -26789, -26673, -26556, -26437,
		-26318, -26198, -26077, -25954, -25831, -25707, -25582, -25456,
		-25329, -25201, -25072, -24942, -24811, -24679, -24546, -24413,
		-24278, -24143, -24006, -23869, -23731, -23592, -23452, -23311,
		-23169, -23027, -22883, -22739, -22594, -22448, -22301, -22153,
		-22004, -21855, -21705, -21554, -21402, -21249, -21096, -20942,
		-20787, -20631, -20474, -20317, -20159, -20000, -19840, -19680,
		-19519, -19357, -19194, -19031, -18867, -18702, -18537, -18371,
		-18204, -18036, -17868, -17699, -17530, -17360, -17189, -17017,
		-16845, -16672, -16499, -16325, -16150, -15975, -15799, -15623,
		-15446, -15268, -15090, -14911, -14732, -14552, -14372, -14191,
		-14009, -13827, -13645, -13462, -13278, -13094, -12909, -12724,
		-12539, -12353, -12166, -11980, -11792, -11604, -11416, -11227,
		-11038, -10849, -10659, -10469, -10278, -10087, -9895, -9703,
		-9511, -9319, -9126, -8932, -8739, -8545, -8351, -8156,
		-7961, -7766, -7571, -7375, -7179, -6982, -6786, -6589,
		-6392, -6195, -5997, -5799, -5601, -5403, -5205, -5006,
		-4807, -4608, -4409, -4210, -4011, -3811, -3611, -3411,
		-3211, -3011, -2811, -2610, -2410, -2209, -2009, -1808,
		-1607, -1406, -1206, -1005, -804, -603, -402, -201
};

int WebRtcSpl_ComplexFFT(int16_t frfi[], int stages, int mode) {
	int i, j, l, k, istep, n, m;
	int16_t wr, wi;
	int32_t tr32, ti32, qr32, qi32;

	/* The 1024-value is a constant given from the size of kSinTable1024[],
	 * and should not be changed depending on the input parameter 'stages'
	 */
	n = 1 << stages;
	if (n > 1024)
		return -1;

	l = 1;
	k = 10 - 1; /* Constant for given kSinTable1024[]. Do not change
		 depending on the input parameter 'stages' */

	if (mode == 0) {
		// mode==0: Low-complexity and Low-accuracy mode
		while (l < n) {
			istep = l << 1;

			for (m = 0; m < l; ++m) {
				j = m << k;

				/* The 256-value is a constant given as 1/4 of the size of
				 * kSinTable1024[], and should not be changed depending on the input
				 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
				 */
				wr = kSinTable1024[j + 256];
				wi = -kSinTable1024[j];

				for (i = m; i < n; i += istep) {
					j = i + l;

					tr32 = (wr * frfi[2 * j] - wi * frfi[2 * j + 1]) >> 15;

					ti32 = (wr * frfi[2 * j + 1] + wi * frfi[2 * j]) >> 15;

					qr32 = (int32_t)frfi[2 * i];
					qi32 = (int32_t)frfi[2 * i + 1];
					frfi[2 * j] = (int16_t)((qr32 - tr32) >> 1);
					frfi[2 * j + 1] = (int16_t)((qi32 - ti32) >> 1);
					frfi[2 * i] = (int16_t)((qr32 + tr32) >> 1);
					frfi[2 * i + 1] = (int16_t)((qi32 + ti32) >> 1);
				}
			}

			--k;
			l = istep;

		}

	}
	else {
		// mode==1: High-complexity and High-accuracy mode
		while (l < n) {
			istep = l << 1;

			for (m = 0; m < l; ++m) {
				j = m << k;

				/* The 256-value is a constant given as 1/4 of the size of
				 * kSinTable1024[], and should not be changed depending on the input
				 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
				 */
				wr = kSinTable1024[j + 256];
				wi = -kSinTable1024[j];

				for (i = m; i < n; i += istep) {
					j = i + l;

#ifdef WEBRTC_ARCH_ARM_V7
					register int32_t frfi_r;
					__asm __volatile(
					"pkhbt %[frfi_r], %[frfi_even], %[frfi_odd],"
						" lsl #16\n\t"
						"smlsd %[tr32], %[wri], %[frfi_r], %[cfftrnd]\n\t"
						"smladx %[ti32], %[wri], %[frfi_r], %[cfftrnd]\n\t"
						: [frfi_r] "=&r"(frfi_r),
						[tr32]"=&r"(tr32),
						[ti32]"=r"(ti32)
						: [frfi_even] "r"((int32_t)frfi[2 * j]),
						[frfi_odd]"r"((int32_t)frfi[2 * j + 1]),
						[wri]"r"(wri),
						[cfftrnd]"r"(CFFTRND));
#else
					tr32 = wr * frfi[2 * j] - wi * frfi[2 * j + 1] + CFFTRND;

					ti32 = wr * frfi[2 * j + 1] + wi * frfi[2 * j] + CFFTRND;
#endif

					tr32 >>= 15 - CFFTSFT;
					ti32 >>= 15 - CFFTSFT;

					qr32 = ((int32_t)frfi[2 * i]) * (1 << CFFTSFT);
					qi32 = ((int32_t)frfi[2 * i + 1]) * (1 << CFFTSFT);

					frfi[2 * j] = (int16_t)(
						(qr32 - tr32 + CFFTRND2) >> (1 + CFFTSFT));
					frfi[2 * j + 1] = (int16_t)(
						(qi32 - ti32 + CFFTRND2) >> (1 + CFFTSFT));
					frfi[2 * i] = (int16_t)(
						(qr32 + tr32 + CFFTRND2) >> (1 + CFFTSFT));
					frfi[2 * i + 1] = (int16_t)(
						(qi32 + ti32 + CFFTRND2) >> (1 + CFFTSFT));
				}
			}

			--k;
			l = istep;
		}
	}
	return 0;
}

int16_t WebRtcSpl_MaxAbsValueW16(const int16_t * vector, size_t length) {
	size_t i = 0;
	int absolute = 0, maximum = 0;

	for (i = 0; i < length; i++) {
		absolute = abs((int)vector[i]);

		if (absolute > maximum) {
			maximum = absolute;
		}
	}

	// Guard the case for abs(-32768).
	if (maximum > 32767) {
		maximum = 32767;
	}

	return (int16_t)maximum;
}

int16_t WebRtcSpl_DivW32W16ResW16(int32_t num, int16_t den)
{
	// Guard against division with 0
	if (den != 0)
	{
		return (int16_t)(num / den);
	}
	else
	{
		return (int16_t)0x7FFF;
	}
}

int16_t WebRtcSpl_MaxValueW16(const int16_t* vector, size_t length) {
	size_t i = 0;
	int value = 0, maximum = 0;

	for (i = 0; i < length; i++) {
		value = (int)vector[i];

		if (value > maximum) {
			maximum = value;
		}
	}

	// Guard the case for abs(-32768).
	if (maximum > 32767) {
		maximum = 32767;
	}

	return (int16_t)maximum;
}

int WebRtcSpl_ComplexIFFT(int16_t frfi[], int stages, int mode) {
	size_t i, j, l, istep, n, m;
	int k, scale, shift;
	int16_t wr, wi;
	int32_t tr32, ti32, qr32, qi32;
	int32_t tmp32, round2;

	/* The 1024-value is a constant given from the size of kSinTable1024[],
	 * and should not be changed depending on the input parameter 'stages'
	 */
	n = 1 << stages;
	if (n > 1024)
		return -1;

	scale = 0;

	l = 1;
	k = 10 - 1; /* Constant for given kSinTable1024[]. Do not change
		 depending on the input parameter 'stages' */

	while (l < n) {
		// variable scaling, depending upon data
		shift = 0;
		round2 = 8192;

		tmp32 = WebRtcSpl_MaxAbsValueW16(frfi, 2 * n);
		if (tmp32 > 13573) {
			shift++;
			scale++;
			round2 <<= 1;
		}
		if (tmp32 > 27146) {
			shift++;
			scale++;
			round2 <<= 1;
		}

		istep = l << 1;

		if (mode == 0) {
			// mode==0: Low-complexity and Low-accuracy mode
			for (m = 0; m < l; ++m) {
				j = m << k;

				/* The 256-value is a constant given as 1/4 of the size of
				 * kSinTable1024[], and should not be changed depending on the input
				 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
				 */
				wr = kSinTable1024[j + 256];
				wi = kSinTable1024[j];

				for (i = m; i < n; i += istep) {
					j = i + l;

					tr32 = (wr * frfi[2 * j] - wi * frfi[2 * j + 1]) >> 15;

					ti32 = (wr * frfi[2 * j + 1] + wi * frfi[2 * j]) >> 15;

					qr32 = (int32_t)frfi[2 * i];
					qi32 = (int32_t)frfi[2 * i + 1];
					frfi[2 * j] = (int16_t)((qr32 - tr32) >> shift);
					frfi[2 * j + 1] = (int16_t)((qi32 - ti32) >> shift);
					frfi[2 * i] = (int16_t)((qr32 + tr32) >> shift);
					frfi[2 * i + 1] = (int16_t)((qi32 + ti32) >> shift);
				}
			}
		}
		else {
			// mode==1: High-complexity and High-accuracy mode

			for (m = 0; m < l; ++m) {
				j = m << k;

				/* The 256-value is a constant given as 1/4 of the size of
				 * kSinTable1024[], and should not be changed depending on the input
				 * parameter 'stages'. It will result in 0 <= j < N_SINE_WAVE/2
				 */
				wr = kSinTable1024[j + 256];
				wi = kSinTable1024[j];

#ifdef WEBRTC_ARCH_ARM_V7
				int32_t wri = 0;
				__asm __volatile("pkhbt %0, %1, %2, lsl #16" : "=r"(wri) :
					"r"((int32_t)wr), "r"((int32_t)wi));
#endif

				for (i = m; i < n; i += istep) {
					j = i + l;

#ifdef WEBRTC_ARCH_ARM_V7
					register int32_t frfi_r;
					__asm __volatile(
					"pkhbt %[frfi_r], %[frfi_even], %[frfi_odd], lsl #16\n\t"
						"smlsd %[tr32], %[wri], %[frfi_r], %[cifftrnd]\n\t"
						"smladx %[ti32], %[wri], %[frfi_r], %[cifftrnd]\n\t"
						: [frfi_r] "=&r"(frfi_r),
						[tr32]"=&r"(tr32),
						[ti32]"=r"(ti32)
						: [frfi_even] "r"((int32_t)frfi[2 * j]),
						[frfi_odd]"r"((int32_t)frfi[2 * j + 1]),
						[wri]"r"(wri),
						[cifftrnd]"r"(CIFFTRND)
						);
#else

					tr32 = wr * frfi[2 * j] - wi * frfi[2 * j + 1] + CIFFTRND;

					ti32 = wr * frfi[2 * j + 1] + wi * frfi[2 * j] + CIFFTRND;
#endif
					tr32 >>= 15 - CIFFTSFT;
					ti32 >>= 15 - CIFFTSFT;

					qr32 = ((int32_t)frfi[2 * i]) * (1 << CIFFTSFT);
					qi32 = ((int32_t)frfi[2 * i + 1]) * (1 << CIFFTSFT);

					frfi[2 * j] = (int16_t)(
						(qr32 - tr32 + round2) >> (shift + CIFFTSFT));
					frfi[2 * j + 1] = (int16_t)(
						(qi32 - ti32 + round2) >> (shift + CIFFTSFT));
					frfi[2 * i] = (int16_t)(
						(qr32 + tr32 + round2) >> (shift + CIFFTSFT));
					frfi[2 * i + 1] = (int16_t)(
						(qi32 + ti32 + round2) >> (shift + CIFFTSFT));
				}
			}

		}
		--k;
		l = istep;
	}
	return scale;
}

/*
struct RealFFT {
	int order;
};

struct RealFFT* WebRtcSpl_CreateRealFFT(int order) {
	struct RealFFT* self = NULL;

	if (order > kMaxFFTOrder || order < 0) {
		return NULL;
	}

	self = malloc(sizeof(struct RealFFT));
	if (self == NULL) {
		return NULL;
	}
	self->order = order;

	return self;
}

void WebRtcSpl_FreeRealFFT(struct RealFFT* self) {
	if (self != NULL) {
	//	free(self);
	}
}
*/

// The C version FFT functions (i.e. WebRtcSpl_RealForwardFFT and
// WebRtcSpl_RealInverseFFT) are real-valued FFT wrappers for complex-valued
// FFT implementation in SPL.

int WebRtcSpl_RealForwardFFT(struct RealFFT* self,
	const int16_t* real_data_in,
	int16_t* complex_data_out) {
	int i = 0;
	int j = 0;
	int result = 0;
	int n = 1 << self->order;
	// The complex-value FFT implementation needs a buffer to hold 2^order
	// 16-bit COMPLEX numbers, for both time and frequency data.
	int16_t complex_buffer[2 << kMaxFFTOrder];

	// Insert zeros to the imaginary parts for complex forward FFT input.
	for (i = 0, j = 0; i < n; i += 1, j += 2) {
		complex_buffer[j] = real_data_in[i];
		complex_buffer[j + 1] = 0;
	};

	// FFT operation
	//WebRtcSpl_ComplexBitReverse(complex_buffer, self->order);
	//result = WebRtcSpl_ComplexFFT(complex_buffer, self->order, 1);
    result=fft_calc(complex_buffer,self->order,0,1);
   

    
	// For real FFT output, use only the first N + 2 elements from
	// complex forward FFT.
	memcpy(complex_data_out, complex_buffer, sizeof(int16_t) * (n + 2));

	return result;
}

int WebRtcSpl_RealInverseFFT(struct RealFFT* self,
	const int16_t * complex_data_in,
	int16_t * real_data_out) {
	int i = 0;
	int j = 0;
	int result = 0;
	int n = 1 << self->order;
	// Create the buffer specific to complex-valued FFT implementation.
	int16_t complex_buffer[2 << kMaxFFTOrder];

	// For n-point FFT, first copy the first n + 2 elements into complex
	// FFT, then construct the remaining n - 2 elements by real FFT's
	// conjugate-symmetric properties.
	memcpy(complex_buffer, complex_data_in, sizeof(int16_t) * (n + 2));
	for (i = n + 2; i < 2 * n; i += 2) {
		complex_buffer[i] = complex_data_in[2 * n - i];
		complex_buffer[i + 1] = -complex_data_in[2 * n - i + 1];
	}

	// IFFT Operation
	//WebRtcSpl_ComplexBitReverse(complex_buffer, self->order);
	//result = WebRtcSpl_ComplexIFFT(complex_buffer, self->order, 1);
    result=fft_calc(complex_buffer,self->order,1,0);
	// Strip out the imaginary parts of the complex inverse FFT output.
	for (i = 0, j = 0; i < n; i += 1, j += 2) {
		real_data_out[i] = complex_buffer[j];
	}

	return result;
}

int32_t WebRtcSpl_Energy(int16_t* vector, int vector_length, int* scale_factor)
{
	int32_t en = 0;
	int i;
	int scaling = WebRtcSpl_GetScalingSquare(vector, vector_length, vector_length);
	int looptimes = vector_length;
	int16_t* vectorptr = vector;

	for (i = 0; i < looptimes; i++)
	{
		en += WEBRTC_SPL_MUL_16_16_RSFT(*vectorptr, *vectorptr, scaling);
		vectorptr++;
	}
	*scale_factor = scaling;

	return en;
}

int16_t WebRtcSpl_GetSizeInBits(uint32_t n) {
	int bits;

	if (0xFFFF0000 & n) {
		bits = 16;
	}
	else {
		bits = 0;
	}
	if (0x0000FF00 & (n >> bits)) bits += 8;
	if (0x000000F0 & (n >> bits)) bits += 4;
	if (0x0000000C & (n >> bits)) bits += 2;
	if (0x00000002 & (n >> bits)) bits += 1;
	if (0x00000001 & (n >> bits)) bits += 1;

	return bits;
}

int WebRtcSpl_GetScalingSquare(int16_t* in_vector, int in_vector_length, int times)
{
	int nbits = WebRtcSpl_GetSizeInBits(times);
	int i;
	int16_t smax = -1;
	int16_t sabs;
	int16_t* sptr = in_vector;
	int t;
	int looptimes = in_vector_length;

	for (i = looptimes; i > 0; i--)
	{
		sabs = (*sptr > 0 ? *sptr++ : -*sptr++);
		smax = (sabs > smax ? sabs : smax);
	}
	t = WebRtcSpl_NormW32(WEBRTC_SPL_MUL(smax, smax));

	if (smax == 0)
	{
		return 0; // Since norm(0) returns 0
	}
	else
	{
		return (t > nbits) ? 0 : nbits - t;
	}
}




int32_t WebRtcSpl_SqrtFloor(int32_t value) {
	int32_t root = 0, try1;

	WEBRTC_SPL_SQRT_ITER(15);
	WEBRTC_SPL_SQRT_ITER(14);
	WEBRTC_SPL_SQRT_ITER(13);
	WEBRTC_SPL_SQRT_ITER(12);
	WEBRTC_SPL_SQRT_ITER(11);
	WEBRTC_SPL_SQRT_ITER(10);
	WEBRTC_SPL_SQRT_ITER(9);
	WEBRTC_SPL_SQRT_ITER(8);
	WEBRTC_SPL_SQRT_ITER(7);
	WEBRTC_SPL_SQRT_ITER(6);
	WEBRTC_SPL_SQRT_ITER(5);
	WEBRTC_SPL_SQRT_ITER(4);
	WEBRTC_SPL_SQRT_ITER(3);
	WEBRTC_SPL_SQRT_ITER(2);
	WEBRTC_SPL_SQRT_ITER(1);
	WEBRTC_SPL_SQRT_ITER(0);

	return root >> 1;
}

uint32_t WebRtcSpl_DivU32U16(uint32_t num, uint16_t den)
{
	// Guard against division with 0
	if (den != 0)
	{
		return (uint32_t)(num / den);
	}
	else
	{
		return (uint32_t)0xFFFFFFFF;
	}
}

int32_t WebRtcSpl_DivW32W16(int32_t num, int16_t den)
{
	// Guard against division with 0
	if (den != 0)
	{
		return (int32_t)(num / den);
	}
	else
	{
		return (int32_t)0x7FFFFFFF;
	}
}

void WebRtcSpl_MemSetW16(int16_t* ptr, int16_t set_value, int length)
{
	int j;
	int16_t* arrptr = ptr;

	for (j = length; j > 0; j--)
	{
		*arrptr++ = set_value;
	}
}

int16_t WebRtcSpl_ZerosArrayW16(int16_t* vector, int16_t length)
{
	WebRtcSpl_MemSetW16(vector, 0, length);
	return length;
}




static __inline int16_t WebRtcSpl_SatW32ToW16(int32_t value32) {
	int16_t out16 = (int16_t)value32;

	if (value32 > 32767)
		out16 = 32767;
	else if (value32 < -32768)
		out16 = -32768;

	return out16;
}

static __inline int16_t WebRtcSpl_AddSatW16(int16_t a, int16_t b) {
	return WebRtcSpl_SatW32ToW16((int32_t)a + (int32_t)b);
}
/*
static __inline int16_t WebRtcSpl_SubSatW16(int16_t var1, int16_t var2) {
	return WebRtcSpl_SatW32ToW16((int32_t)var1 - (int32_t)var2);
}

*/



static __inline int WebRtcSpl_NormW32(int32_t a) {
	int zeros;

	if (a == 0) {
		return 0;
	}
	else if (a < 0) {
		a = ~a;
	}

	if (!(0xFFFF8000 & a)) {
		zeros = 16;
	}
	else {
		zeros = 0;
	}
	if (!(0xFF800000 & (a << zeros))) zeros += 8;
	if (!(0xF8000000 & (a << zeros))) zeros += 4;
	if (!(0xE0000000 & (a << zeros))) zeros += 2;
	if (!(0xC0000000 & (a << zeros))) zeros += 1;

	return zeros;
}

static __inline int WebRtcSpl_NormU32(uint32_t a) {
	int zeros;

	if (a == 0) return 0;

	if (!(0xFFFF0000 & a)) {
		zeros = 16;
	}
	else {
		zeros = 0;
	}
	if (!(0xFF000000 & (a << zeros))) zeros += 8;
	if (!(0xF0000000 & (a << zeros))) zeros += 4;
	if (!(0xC0000000 & (a << zeros))) zeros += 2;
	if (!(0x80000000 & (a << zeros))) zeros += 1;

	return zeros;
}

static __inline int WebRtcSpl_NormW16(int16_t a) {
	int zeros;

	if (a == 0) {
		return 0;
	}
	else if (a < 0) {
		a = ~a;
	}

	if (!(0xFF80 & a)) {
		zeros = 8;
	}
	else {
		zeros = 0;
	}
	if (!(0xF800 & (a << zeros))) zeros += 4;
	if (!(0xE000 & (a << zeros))) zeros += 2;
	if (!(0xC000 & (a << zeros))) zeros += 1;

	return zeros;
}
/*
static __inline int32_t WebRtc_MulAccumW16(int16_t a, int16_t b, int32_t c) {
	return (a * b + c);
}


// The following functions have no optimized versions.
// TODO(kma): Consider saturating add/sub instructions in X86 platform.

static __inline int32_t WebRtcSpl_AddSatW32(int32_t l_var1, int32_t l_var2) {
	int32_t l_sum;

	// Perform long addition
	l_sum = l_var1 + l_var2;

	if (l_var1 < 0) {  // Check for underflow.
		if ((l_var2 < 0) && (l_sum >= 0)) {
			l_sum = (int32_t)0x80000000;
		}
	}
	else {  // Check for overflow.
		if ((l_var2 > 0) && (l_sum < 0)) {
			l_sum = (int32_t)0x7FFFFFFF;
		}
	}

	return l_sum;
}

static __inline int32_t WebRtcSpl_SubSatW32(int32_t l_var1, int32_t l_var2) {
	int32_t l_diff;

	// Perform subtraction.
	l_diff = l_var1 - l_var2;

	if (l_var1 < 0) {  // Check for underflow.
		if ((l_var2 > 0) && (l_diff > 0)) {
			l_diff = (int32_t)0x80000000;
		}
	}
	else {  // Check for overflow.
		if ((l_var2 < 0) && (l_diff < 0)) {
			l_diff = (int32_t)0x7FFFFFFF;
		}
	}

	return l_diff;
}
*/

static const int16_t WebRtcNsx_kLogTable[9] = {
  0, 177, 355, 532, 710, 887, 1065, 1242, 1420
};

static const int16_t WebRtcNsx_kCounterDiv[201] = {
  32767, 16384, 10923, 8192, 6554, 5461, 4681, 4096, 3641, 3277, 2979, 2731,
  2521, 2341, 2185, 2048, 1928, 1820, 1725, 1638, 1560, 1489, 1425, 1365, 1311,
  1260, 1214, 1170, 1130, 1092, 1057, 1024, 993, 964, 936, 910, 886, 862, 840,
  819, 799, 780, 762, 745, 728, 712, 697, 683, 669, 655, 643, 630, 618, 607,
  596, 585, 575, 565, 555, 546, 537, 529, 520, 512, 504, 496, 489, 482, 475,
  468, 462, 455, 449, 443, 437, 431, 426, 420, 415, 410, 405, 400, 395, 390,
  386, 381, 377, 372, 368, 364, 360, 356, 352, 349, 345, 341, 338, 334, 331,
  328, 324, 321, 318, 315, 312, 309, 306, 303, 301, 298, 295, 293, 290, 287,
  285, 282, 280, 278, 275, 273, 271, 269, 266, 264, 262, 260, 258, 256, 254,
  252, 250, 248, 246, 245, 243, 241, 239, 237, 236, 234, 232, 231, 229, 228,
  226, 224, 223, 221, 220, 218, 217, 216, 214, 213, 211, 210, 209, 207, 206,
  205, 204, 202, 201, 200, 199, 197, 196, 195, 194, 193, 192, 191, 189, 188,
  187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173,
  172, 172, 171, 170, 169, 168, 167, 166, 165, 165, 164, 163
};

static const int16_t WebRtcNsx_kLogTableFrac[256] = {
  0,   1,   3,   4,   6,   7,   9,  10,  11,  13,  14,  16,  17,  18,  20,  21,
  22,  24,  25,  26,  28,  29,  30,  32,  33,  34,  36,  37,  38,  40,  41,  42,
  44,  45,  46,  47,  49,  50,  51,  52,  54,  55,  56,  57,  59,  60,  61,  62,
  63,  65,  66,  67,  68,  69,  71,  72,  73,  74,  75,  77,  78,  79,  80,  81,
  82,  84,  85,  86,  87,  88,  89,  90,  92,  93,  94,  95,  96,  97,  98,  99,
  100, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 116,
  117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
  132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146,
  147, 148, 149, 150, 151, 152, 153, 154, 155, 155, 156, 157, 158, 159, 160,
  161, 162, 163, 164, 165, 166, 167, 168, 169, 169, 170, 171, 172, 173, 174,
  175, 176, 177, 178, 178, 179, 180, 181, 182, 183, 184, 185, 185, 186, 187,
  188, 189, 190, 191, 192, 192, 193, 194, 195, 196, 197, 198, 198, 199, 200,
  201, 202, 203, 203, 204, 205, 206, 207, 208, 208, 209, 210, 211, 212, 212,
  213, 214, 215, 216, 216, 217, 218, 219, 220, 220, 221, 222, 223, 224, 224,
  225, 226, 227, 228, 228, 229, 230, 231, 231, 232, 233, 234, 234, 235, 236,
  237, 238, 238, 239, 240, 241, 241, 242, 243, 244, 244, 245, 246, 247, 247,
  248, 249, 249, 250, 251, 252, 252, 253, 254, 255, 255
};


// Skip first frequency bins during estimation. (0 <= value < 64)
static const int kStartBand = 5;

static const int16_t kIndicatorTable[17] = {
  0, 2017, 3809, 5227, 6258, 6963, 7424, 7718,
  7901, 8014, 8084, 8126, 8152, 8168, 8177, 8183, 8187
};

// hybrib Hanning & flat window
/*
static const int16_t kBlocks80w128x[128] = {
  0,    536,   1072,   1606,   2139,   2669,   3196,   3720,   4240,   4756,   5266,
  5771,   6270,   6762,   7246,   7723,   8192,   8652,   9102,   9543,   9974,  10394,
  10803,  11200,  11585,  11958,  12318,  12665,  12998,  13318,  13623,  13913,  14189,
  14449,  14694,  14924,  15137,  15334,  15515,  15679,  15826,  15956,  16069,  16165,
  16244,  16305,  16349,  16375,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,  16384,
  16384,  16384,  16384,  16384,  16375,  16349,  16305,  16244,  16165,  16069,  15956,
  15826,  15679,  15515,  15334,  15137,  14924,  14694,  14449,  14189,  13913,  13623,
  13318,  12998,  12665,  12318,  11958,  11585,  11200,  10803,  10394,   9974,   9543,
  9102,   8652,   8192,   7723,   7246,   6762,   6270,   5771,   5266,   4756,   4240,
  3720,   3196,   2669,   2139,   1606,   1072,    536
};
*/
static const int16_t kBlocks80w128x[128] = {
		  0, 399, 798, 1196, 1594, 1990, 2386, 2780, 3172,
		3562, 3951, 4337, 4720, 5101, 5478, 5853, 6224,
		6591, 6954, 7313, 7668, 8019, 8364, 8705, 9040,
		9370, 9695, 10013, 10326, 10633, 10933, 11227, 11514,
		11795, 12068, 12335, 12594, 12845, 13089, 13325, 13553,
		13773, 13985, 14189, 14384, 14571, 14749, 14918, 15079,
		15231, 15373, 15506, 15631, 15746, 15851, 15947, 16034,
		16111, 16179, 16237, 16286, 16325, 16354, 16373, 16384,
		16373, 16354, 16325,  16286,  16237, 16179, 16111,
		16034,	15947,	15851,	15746,	15631,	15506,	15373,	15231,
		15079,	14918,	14749,	14571,	14384,	14189,	13985,	13773,
		13553,	13325,	13089,	12845,	12594,	12335,	12068,	11795,
		11514,	11227,	10933,	10633,	10326,	10013,	9695,	9370,
		9040,	8705,	8364,	8019,	7668,	7313,	6954,	6591,
		6224,	5853,	5478,	5101,	4720,	4337,	3951,	3562,
		3172,	2780,	2386,	1990,	1594,	1196,	798,	399
};


// hybrib Hanning & flat window
/*
static const int16_t kBlocks160w256x[256] = {
  0,   268,   536,   804,  1072,  1339,  1606,  1872,
  2139,  2404,  2669,  2933,  3196,  3459,  3720,  3981,
  4240,  4499,  4756,  5012,  5266,  5520,  5771,  6021,
  6270,  6517,  6762,  7005,  7246,  7486,  7723,  7959,
  8192,  8423,  8652,  8878,  9102,  9324,  9543,  9760,
  9974, 10185, 10394, 10600, 10803, 11003, 11200, 11394,
  11585, 11773, 11958, 12140, 12318, 12493, 12665, 12833,
  12998, 13160, 13318, 13472, 13623, 13770, 13913, 14053,
  14189, 14321, 14449, 14574, 14694, 14811, 14924, 15032,
  15137, 15237, 15334, 15426, 15515, 15599, 15679, 15754,
  15826, 15893, 15956, 16015, 16069, 16119, 16165, 16207,
  16244, 16277, 16305, 16329, 16349, 16364, 16375, 16382,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
  16384, 16382, 16375, 16364, 16349, 16329, 16305, 16277,
  16244, 16207, 16165, 16119, 16069, 16015, 15956, 15893,
  15826, 15754, 15679, 15599, 15515, 15426, 15334, 15237,
  15137, 15032, 14924, 14811, 14694, 14574, 14449, 14321,
  14189, 14053, 13913, 13770, 13623, 13472, 13318, 13160,
  12998, 12833, 12665, 12493, 12318, 12140, 11958, 11773,
  11585, 11394, 11200, 11003, 10803, 10600, 10394, 10185,
  9974,  9760,  9543,  9324,  9102,  8878,  8652,  8423,
  8192,  7959,  7723,  7486,  7246,  7005,  6762,  6517,
  6270,  6021,  5771,  5520,  5266,  5012,  4756,  4499,
  4240,  3981,  3720,  3459,  3196,  2933,  2669,  2404,
  2139,  1872,  1606,  1339,  1072,   804,   536,   268
};*/
static const int16_t kBlocks160w256x[256] = {0,201,402,603,804,1005,1205,1406,1606,1806,2006,2205,2404,2603,2801,2999,3196,3393,3590,3786,3981,4176,4370,4563,4756,4948,5139,
5330,5520,5708,5897,6084,6270,6455,6639,6823,7005,7186,7366,7545,7723,7900,8076,8250,8423,8595,8765,8935,9102,9269,9434,9598,9760,
9921,10080,10238,10394,10549,10702,10853,11003,11151,11297,11442,11585,11727,11866,12004,12140,12274,12406,12537,12665,12792,12916,
13039,13160,13279,13395,13510,13623,13733,13842,13949,14053,14155,14256,14354,14449,14543,14635,14724,14811,14896,14978,15059,15137,15213,
15286,15357,15426,15493,15557,15619,15679,15736,15791,15843,15893,15941,15986,16029,16069,16107,16143,16176,16207,16235,16261,16284,16305,
16324,16340,16353,16364,16373,16379,16383,16384,16383,16379,16373,16364,16353,16340,16324,16305,16284,16261,16235,16207,16176,16143,16107,
16069,16029,15986,15941,15893,15843,15791,15736,15679,15619,15557,15493,15426,15357,15286,15213,15137,15059,14978,14896,14811,14724,14635,
14543,14449,14354,14256,14155,14053,13949,13842,13733,13623,13510,13395,13279,13160,13039,12916,12792,12665,12537,12406,12274,12140,12004,
11866,11727,11585,11442,11297,11151,11003,10853,10702,10549,10394,10238,10080,9921,9760,9598,9434,9269,9102,8935,8765,8595,8423,8250,8076,
7900,7723,7545,7366,7186,7005,6823,6639,6455,6270,6084,5897,5708,5520,5330,5139,4948,4756,4563,4370,4176,3981,3786,3590,3393,3196,2999,2801,2603,2404,2205,2006,1806,1606,1406,1205,1005,804,603,402,201 };
// Gain factor1 table: Input value in Q8 and output value in Q13
// original floating point code
//  if (gain > blim) {
//    factor1 = 1.0 + 1.3 * (gain - blim);
//    if (gain * factor1 > 1.0) {
//      factor1 = 1.0 / gain;
//    }
//  } else {
//    factor1 = 1.0;
//  }
static const int16_t kFactor1Table[257] = {
  8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8233, 8274, 8315, 8355, 8396, 8436, 8475, 8515, 8554, 8592, 8631, 8669,
  8707, 8745, 8783, 8820, 8857, 8894, 8931, 8967, 9003, 9039, 9075, 9111, 9146, 9181,
  9216, 9251, 9286, 9320, 9354, 9388, 9422, 9456, 9489, 9523, 9556, 9589, 9622, 9655,
  9687, 9719, 9752, 9784, 9816, 9848, 9879, 9911, 9942, 9973, 10004, 10035, 10066,
  10097, 10128, 10158, 10188, 10218, 10249, 10279, 10308, 10338, 10368, 10397, 10426,
  10456, 10485, 10514, 10543, 10572, 10600, 10629, 10657, 10686, 10714, 10742, 10770,
  10798, 10826, 10854, 10882, 10847, 10810, 10774, 10737, 10701, 10666, 10631, 10596,
  10562, 10527, 10494, 10460, 10427, 10394, 10362, 10329, 10297, 10266, 10235, 10203,
  10173, 10142, 10112, 10082, 10052, 10023, 9994, 9965, 9936, 9908, 9879, 9851, 9824,
  9796, 9769, 9742, 9715, 9689, 9662, 9636, 9610, 9584, 9559, 9534, 9508, 9484, 9459,
  9434, 9410, 9386, 9362, 9338, 9314, 9291, 9268, 9245, 9222, 9199, 9176, 9154, 9132,
  9110, 9088, 9066, 9044, 9023, 9002, 8980, 8959, 8939, 8918, 8897, 8877, 8857, 8836,
  8816, 8796, 8777, 8757, 8738, 8718, 8699, 8680, 8661, 8642, 8623, 8605, 8586, 8568,
  8550, 8532, 8514, 8496, 8478, 8460, 8443, 8425, 8408, 8391, 8373, 8356, 8339, 8323,
  8306, 8289, 8273, 8256, 8240, 8224, 8208, 8192
};

// For Factor2 tables
// original floating point code
// if (gain > blim) {
//   factor2 = 1.0;
// } else {
//   factor2 = 1.0 - 0.3 * (blim - gain);
//   if (gain <= inst->denoiseBound) {
//     factor2 = 1.0 - 0.3 * (blim - inst->denoiseBound);
//   }
// }
//
// Gain factor table: Input value in Q8 and output value in Q13
static const int16_t kFactor2Aggressiveness1[257] = {
  7577, 7577, 7577, 7577, 7577, 7577,
  7577, 7577, 7577, 7577, 7577, 7577, 7577, 7577, 7577, 7577, 7577, 7596, 7614, 7632,
  7650, 7667, 7683, 7699, 7715, 7731, 7746, 7761, 7775, 7790, 7804, 7818, 7832, 7845,
  7858, 7871, 7884, 7897, 7910, 7922, 7934, 7946, 7958, 7970, 7982, 7993, 8004, 8016,
  8027, 8038, 8049, 8060, 8070, 8081, 8091, 8102, 8112, 8122, 8132, 8143, 8152, 8162,
  8172, 8182, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192
};

// Gain factor table: Input value in Q8 and output value in Q13
static const int16_t kFactor2Aggressiveness2[257] = {
  7270, 7270, 7270, 7270, 7270, 7306,
  7339, 7369, 7397, 7424, 7448, 7472, 7495, 7517, 7537, 7558, 7577, 7596, 7614, 7632,
  7650, 7667, 7683, 7699, 7715, 7731, 7746, 7761, 7775, 7790, 7804, 7818, 7832, 7845,
  7858, 7871, 7884, 7897, 7910, 7922, 7934, 7946, 7958, 7970, 7982, 7993, 8004, 8016,
  8027, 8038, 8049, 8060, 8070, 8081, 8091, 8102, 8112, 8122, 8132, 8143, 8152, 8162,
  8172, 8182, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192
};

// Gain factor table: Input value in Q8 and output value in Q13
static const int16_t kFactor2Aggressiveness3[257] = {
  7184, 7184, 7184, 7229, 7270, 7306,
  7339, 7369, 7397, 7424, 7448, 7472, 7495, 7517, 7537, 7558, 7577, 7596, 7614, 7632,
  7650, 7667, 7683, 7699, 7715, 7731, 7746, 7761, 7775, 7790, 7804, 7818, 7832, 7845,
  7858, 7871, 7884, 7897, 7910, 7922, 7934, 7946, 7958, 7970, 7982, 7993, 8004, 8016,
  8027, 8038, 8049, 8060, 8070, 8081, 8091, 8102, 8112, 8122, 8132, 8143, 8152, 8162,
  8172, 8182, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192,
  8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192
};

// sum of log2(i) from table index to inst->anaLen2 in Q5
// Note that the first table value is invalid, since log2(0) = -infinity
static const int16_t kSumLogIndex[66] = {
  0,  22917,  22917,  22885,  22834,  22770,  22696,  22613,
  22524,  22428,  22326,  22220,  22109,  21994,  21876,  21754,
  21629,  21501,  21370,  21237,  21101,  20963,  20822,  20679,
  20535,  20388,  20239,  20089,  19937,  19783,  19628,  19470,
  19312,  19152,  18991,  18828,  18664,  18498,  18331,  18164,
  17994,  17824,  17653,  17480,  17306,  17132,  16956,  16779,
  16602,  16423,  16243,  16063,  15881,  15699,  15515,  15331,
  15146,  14960,  14774,  14586,  14398,  14209,  14019,  13829,
  13637,  13445
};

// sum of log2(i)^2 from table index to inst->anaLen2 in Q2
// Note that the first table value is invalid, since log2(0) = -infinity
static const int16_t kSumSquareLogIndex[66] = {
  0,  16959,  16959,  16955,  16945,  16929,  16908,  16881,
  16850,  16814,  16773,  16729,  16681,  16630,  16575,  16517,
  16456,  16392,  16325,  16256,  16184,  16109,  16032,  15952,
  15870,  15786,  15700,  15612,  15521,  15429,  15334,  15238,
  15140,  15040,  14938,  14834,  14729,  14622,  14514,  14404,
  14292,  14179,  14064,  13947,  13830,  13710,  13590,  13468,
  13344,  13220,  13094,  12966,  12837,  12707,  12576,  12444,
  12310,  12175,  12039,  11902,  11763,  11624,  11483,  11341,
  11198,  11054
};

// log2(table index) in Q12
// Note that the first table value is invalid, since log2(0) = -infinity
static const int16_t kLogIndex[129] = {
  0,      0,   4096,   6492,   8192,   9511,  10588,  11499,
  12288,  12984,  13607,  14170,  14684,  15157,  15595,  16003,
  16384,  16742,  17080,  17400,  17703,  17991,  18266,  18529,
  18780,  19021,  19253,  19476,  19691,  19898,  20099,  20292,
  20480,  20662,  20838,  21010,  21176,  21338,  21496,  21649,
  21799,  21945,  22087,  22226,  22362,  22495,  22625,  22752,
  22876,  22998,  23117,  23234,  23349,  23462,  23572,  23680,
  23787,  23892,  23994,  24095,  24195,  24292,  24388,  24483,
  24576,  24668,  24758,  24847,  24934,  25021,  25106,  25189,
  25272,  25354,  25434,  25513,  25592,  25669,  25745,  25820,
  25895,  25968,  26041,  26112,  26183,  26253,  26322,  26390,
  26458,  26525,  26591,  26656,  26721,  26784,  26848,  26910,
  26972,  27033,  27094,  27154,  27213,  27272,  27330,  27388,
  27445,  27502,  27558,  27613,  27668,  27722,  27776,  27830,
  27883,  27935,  27988,  28039,  28090,  28141,  28191,  28241,
  28291,  28340,  28388,  28437,  28484,  28532,  28579,  28626,
  28672
};

// determinant of estimation matrix in Q0 corresponding to the log2 tables above
// Note that the first table value is invalid, since log2(0) = -infinity
static const int16_t kDeterminantEstMatrix[66] = {
  0,  29814,  25574,  22640,  20351,  18469,  16873,  15491,
  14277,  13199,  12233,  11362,  10571,   9851,   9192,   8587,
  8030,   7515,   7038,   6596,   6186,   5804,   5448,   5115,
  4805,   4514,   4242,   3988,   3749,   3524,   3314,   3116,
  2930,   2755,   2590,   2435,   2289,   2152,   2022,   1900,
  1785,   1677,   1575,   1478,   1388,   1302,   1221,   1145,
  1073,   1005,    942,    881,    825,    771,    721,    674,
  629,    587,    547,    510,    475,    442,    411,    382,
  355,    330
};

// Update the noise estimation information.
static void UpdateNoiseEstimate(NsxInst_t* inst, int offset) {
	int32_t tmp32no1 = 0;
	int32_t tmp32no2 = 0;
	int16_t tmp16 = 0;
	const int16_t kExp2Const = 11819; // Q13

	int i = 0;

	tmp16 = WebRtcSpl_MaxValueW16(inst->noiseEstLogQuantile + offset,
		inst->magnLen);
	// Guarantee a Q-domain as high as possible and still fit in int16
	inst->qNoise = 14 - (int)WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(
		kExp2Const, tmp16, 21);
	for (i = 0; i < inst->magnLen; i++) {
		// inst->quantile[i]=exp(inst->lquantile[offset+i]);
		// in Q21
		tmp32no2 = WEBRTC_SPL_MUL_16_16(kExp2Const,
			inst->noiseEstLogQuantile[offset + i]);
		tmp32no1 = (0x00200000 | (tmp32no2 & 0x001FFFFF)); // 2^21 + frac
		tmp16 = (int16_t)WEBRTC_SPL_RSHIFT_W32(tmp32no2, 21);
		tmp16 -= 21;// shift 21 to get result in Q0
		tmp16 += (int16_t)inst->qNoise; //shift to get result in Q(qNoise)
		if (tmp16 < 0) {
			tmp32no1 = WEBRTC_SPL_RSHIFT_W32(tmp32no1, -tmp16);
		}
		else {
			tmp32no1 = WEBRTC_SPL_LSHIFT_W32(tmp32no1, tmp16);
		}
		inst->noiseEstQuantile[i] = WebRtcSpl_SatW32ToW16(tmp32no1);
	}
}

// Noise Estimation
static void NoiseEstimationC(NsxInst_t * inst,
	uint16_t * magn,
	uint32_t * noise,
	int16_t * q_noise) {
	int16_t lmagn[HALF_ANAL_BLOCKL], counter, countDiv;
	int16_t countProd, delta, zeros, frac;
	int16_t log2, tabind, logval, tmp16, tmp16no1, tmp16no2;
	const int16_t log2_const = 22713; // Q15
	const int16_t width_factor = 21845;

	int i, s, offset;

	tabind = inst->stages - inst->normData;
	if (tabind < 0) {
		logval = -WebRtcNsx_kLogTable[-tabind];
	}
	else {
		logval = WebRtcNsx_kLogTable[tabind];
	}

	// lmagn(i)=log(magn(i))=log(2)*log2(magn(i))
	// magn is in Q(-stages), and the real lmagn values are:
	// real_lmagn(i)=log(magn(i)*2^stages)=log(magn(i))+log(2^stages)
	// lmagn in Q8
	for (i = 0; i < inst->magnLen; i++) {
		if (magn[i]) {
			zeros = WebRtcSpl_NormU32((uint32_t)magn[i]);
			frac = (int16_t)((((uint32_t)magn[i] << zeros)
				& 0x7FFFFFFF) >> 23);
			// log2(magn(i))
			log2 = (int16_t)(((31 - zeros) << 8)
				+ WebRtcNsx_kLogTableFrac[frac]);
			// log2(magn(i))*log(2)
			lmagn[i] = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(log2, log2_const, 15);
			// + log(2^stages)
			lmagn[i] += logval;
		}
		else {
			lmagn[i] = logval;//0;
		}
	}

	// loop over simultaneous estimates
	for (s = 0; s < SIMULT; s++) {
		offset = s * inst->magnLen;

		// Get counter values from state
		counter = inst->noiseEstCounter[s];
		countDiv = WebRtcNsx_kCounterDiv[counter];
		countProd = (int16_t)WEBRTC_SPL_MUL_16_16(counter, countDiv);

		// quant_est(...)
		for (i = 0; i < inst->magnLen; i++) {
			// compute delta
			if (inst->noiseEstDensity[offset + i] > 512) {
				// Get the value for delta by shifting intead of dividing.
				int factor = WebRtcSpl_NormW16(inst->noiseEstDensity[offset + i]);
				delta = (int16_t)(FACTOR_Q16 >> (14 - factor));
			}
			else {
				delta = FACTOR_Q7;
				if (inst->blockIndex < END_STARTUP_LONG) {
					// Smaller step size during startup. This prevents from using
					// unrealistic values causing overflow.
					delta = FACTOR_Q7_STARTUP;
				}
			}

			// update log quantile estimate
			tmp16 = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(delta, countDiv, 14);
			if (lmagn[i] > inst->noiseEstLogQuantile[offset + i]) {
				// +=QUANTILE*delta/(inst->counter[s]+1) QUANTILE=0.25, =1 in Q2
				// CounterDiv=1/(inst->counter[s]+1) in Q15
				tmp16 += 2;
				tmp16no1 = WEBRTC_SPL_RSHIFT_W16(tmp16, 2);
				inst->noiseEstLogQuantile[offset + i] += tmp16no1;
			}
			else {
				tmp16 += 1;
				tmp16no1 = WEBRTC_SPL_RSHIFT_W16(tmp16, 1);
				// *(1-QUANTILE), in Q2 QUANTILE=0.25, 1-0.25=0.75=3 in Q2
				tmp16no2 = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(tmp16no1, 3, 1);
				inst->noiseEstLogQuantile[offset + i] -= tmp16no2;
				if (inst->noiseEstLogQuantile[offset + i] < logval) {
					// This is the smallest fixed point representation we can
					// have, hence we limit the output.
					inst->noiseEstLogQuantile[offset + i] = logval;
				}
			}

			// update density estimate
			if (WEBRTC_SPL_ABS_W16(lmagn[i] - inst->noiseEstLogQuantile[offset + i])
				< WIDTH_Q8) {
				tmp16no1 = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(
					inst->noiseEstDensity[offset + i], countProd, 15);
				tmp16no2 = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(
					width_factor, countDiv, 15);
				inst->noiseEstDensity[offset + i] = tmp16no1 + tmp16no2;
			}
		}  // end loop over magnitude spectrum

		if (counter >= END_STARTUP_LONG) {
			inst->noiseEstCounter[s] = 0;
			if (inst->blockIndex >= END_STARTUP_LONG) {
				UpdateNoiseEstimate(inst, offset);
			}
		}
		inst->noiseEstCounter[s]++;

	}  // end loop over simultaneous estimates

	// Sequentially update the noise during startup
	if (inst->blockIndex < END_STARTUP_LONG) {
		UpdateNoiseEstimate(inst, offset);
	}

	for (i = 0; i < inst->magnLen; i++) {
		noise[i] = (uint32_t)(inst->noiseEstQuantile[i]); // Q(qNoise)
	}
	(*q_noise) = (int16_t)inst->qNoise;
}

// Filter the data in the frequency domain, and create spectrum.
static void PrepareSpectrumC(NsxInst_t * inst, int16_t * freq_buf) {
	int i = 0, j = 0;

	for (i = 0; i < inst->magnLen; i++) {
		inst->real[i] = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(inst->real[i],
			(int16_t)(inst->noiseSupFilter[i]), 14); // Q(normData-stages)
		inst->imag[i] = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(inst->imag[i],
			(int16_t)(inst->noiseSupFilter[i]), 14); // Q(normData-stages)
	}

	freq_buf[0] = inst->real[0];
	freq_buf[1] = -inst->imag[0];
	for (i = 1, j = 2; i < inst->anaLen2; i += 1, j += 2) {
		freq_buf[j] = inst->real[i];
		freq_buf[j + 1] = -inst->imag[i];
	}
	freq_buf[inst->anaLen] = inst->real[inst->anaLen2];
	freq_buf[inst->anaLen + 1] = -inst->imag[inst->anaLen2];
}

// Denormalize the real-valued signal |in|, the output from inverse FFT.
static __inline void Denormalize(NsxInst_t * inst, int16_t * in, int factor) {
	int i = 0;
	int32_t tmp32 = 0;
	for (i = 0; i < inst->anaLen; i += 1) {
		tmp32 = WEBRTC_SPL_SHIFT_W32((int32_t)in[i],
			factor - inst->normData);
		inst->real[i] = WebRtcSpl_SatW32ToW16(tmp32); // Q0
	}
//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D100, factor);
}

// For the noise supression process, synthesis, read out fully processed
// segment, and update synthesis buffer.
static void SynthesisUpdateC(NsxInst_t * inst,
	int16_t * out_frame,
	int16_t gain_factor) {
	int i = 0;
	int16_t tmp16a = 0;
	int16_t tmp16b = 0;
	int32_t tmp32 = 0;

	// synthesis
	for (i = 0; i < inst->anaLen; i++) {
		tmp16a = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(
			inst->window[i], inst->real[i], 14); // Q0, window in Q14
		tmp32 = WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(tmp16a, gain_factor, 13); // Q0
		// Down shift with rounding
		tmp16b = WebRtcSpl_SatW32ToW16(tmp32); // Q0
		inst->synthesisBuffer[i] = WEBRTC_SPL_ADD_SAT_W16(inst->synthesisBuffer[i],
			tmp16b); // Q0
	}

	// read out fully processed segment
	for (i = 0; i < inst->blockLen10ms; i++) {
		out_frame[i] = inst->synthesisBuffer[i]; // Q0
	}

	// update synthesis buffer
	WEBRTC_SPL_MEMCPY_W16(inst->synthesisBuffer,
		inst->synthesisBuffer + inst->blockLen10ms,
		inst->anaLen - inst->blockLen10ms);
	WebRtcSpl_ZerosArrayW16(inst->synthesisBuffer
		+ inst->anaLen - inst->blockLen10ms, inst->blockLen10ms);
}

// Update analysis buffer for lower band, and window data before FFT.
static void AnalysisUpdateC(NsxInst_t * inst,
	int16_t * out,
	int16_t * new_speech) {
	int i = 0;

	// For lower band update analysis buffer.
	WEBRTC_SPL_MEMCPY_W16(inst->analysisBuffer,
		inst->analysisBuffer + inst->blockLen10ms,
		inst->anaLen - inst->blockLen10ms);
	WEBRTC_SPL_MEMCPY_W16(inst->analysisBuffer
		+ inst->anaLen - inst->blockLen10ms, new_speech, inst->blockLen10ms);

	// Window data before FFT.
	for (i = 0; i < inst->anaLen; i++) {
		out[i] = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(
			inst->window[i], inst->analysisBuffer[i], 14); // Q0
	}
}

// Normalize the real-valued signal |in|, the input to forward FFT.
static __inline void NormalizeRealBuffer(NsxInst_t * inst,
	const int16_t * in,
	int16_t * out) {
	int i = 0;
	for (i = 0; i < inst->anaLen; ++i) {
		out[i] = WEBRTC_SPL_LSHIFT_W16(in[i], inst->normData); // Q(normData)
	}
}

// Declare function pointers.
NoiseEstimation WebRtcNsx_NoiseEstimation;
PrepareSpectrum WebRtcNsx_PrepareSpectrum;
SynthesisUpdate WebRtcNsx_SynthesisUpdate;
AnalysisUpdate WebRtcNsx_AnalysisUpdate;



void WebRtcNsx_CalcParametricNoiseEstimate(NsxInst_t * inst,
	int16_t pink_noise_exp_avg,
	int32_t pink_noise_num_avg,
	int freq_index,
	uint32_t * noise_estimate,
	uint32_t * noise_estimate_avg) {
	int32_t tmp32no1 = 0;
	int32_t tmp32no2 = 0;

	int16_t int_part = 0;
	int16_t frac_part = 0;

	// Use pink noise estimate
	// noise_estimate = 2^(pinkNoiseNumerator + pinkNoiseExp * log2(j))
	tmp32no2 = WEBRTC_SPL_MUL_16_16(pink_noise_exp_avg, kLogIndex[freq_index]); // Q26
	tmp32no2 = WEBRTC_SPL_RSHIFT_W32(tmp32no2, 15); // Q11
	tmp32no1 = pink_noise_num_avg - tmp32no2; // Q11

	// Calculate output: 2^tmp32no1
	// Output in Q(minNorm-stages)
	tmp32no1 += WEBRTC_SPL_LSHIFT_W32((int32_t)(inst->minNorm - inst->stages), 11);
	if (tmp32no1 > 0) {
		int_part = (int16_t)WEBRTC_SPL_RSHIFT_W32(tmp32no1, 11);
		frac_part = (int16_t)(tmp32no1 & 0x000007ff); // Q11
		// Piecewise linear approximation of 'b' in
		// 2^(int_part+frac_part) = 2^int_part * (1 + b)
		// 'b' is given in Q11 and below stored in frac_part.
		if (WEBRTC_SPL_RSHIFT_W16(frac_part, 10)) {
			// Upper fractional part
			tmp32no2 = WEBRTC_SPL_MUL_16_16(2048 - frac_part, 1244); // Q21
			tmp32no2 = 2048 - WEBRTC_SPL_RSHIFT_W32(tmp32no2, 10);
		}
		else {
			// Lower fractional part
			tmp32no2 = WEBRTC_SPL_RSHIFT_W32(WEBRTC_SPL_MUL_16_16(frac_part, 804), 10);
		}
		// Shift fractional part to Q(minNorm-stages)
		tmp32no2 = WEBRTC_SPL_SHIFT_W32(tmp32no2, int_part - 11);
		*noise_estimate_avg = WEBRTC_SPL_LSHIFT_U32(1, int_part) + (uint32_t)tmp32no2;
		// Scale up to initMagnEst, which is not block averaged
		*noise_estimate = (*noise_estimate_avg) * (uint32_t)(inst->blockIndex + 1);
	}
}

// Initialize state
int32_t WebRtcNsx_InitCore(NsxInst_t * inst, uint32_t fs) {
	int i;

	//check for valid pointer
	if (inst == NULL) {
		return -1;
	}
	//

	// Initialization of struct
	if (fs == 8000 || fs == 16000 || fs == 32000) {
		inst->fs = fs;
	}
	else {
		return -1;
	}

	if (fs == 8000) {
		inst->blockLen10ms = 64;
		inst->anaLen = 128;
		inst->stages = 7;
		inst->window = kBlocks80w128x;
		inst->thresholdLogLrt = 131072; //default threshold for LRT feature
		inst->maxLrt = 0x0040000;
		inst->minLrt = 52429;
	}
	else if (fs == 16000) {
		inst->blockLen10ms = 128;
		inst->anaLen = 256;
		inst->stages = 8;
		inst->window = kBlocks160w256x;
		inst->thresholdLogLrt = 212644; //default threshold for LRT feature
		inst->maxLrt = 0x0080000;
		inst->minLrt = 104858;
	}
	else if (fs == 32000) {
		inst->blockLen10ms = 160;
		inst->anaLen = 256;
		inst->stages = 8;
		inst->window = kBlocks160w256x;
		inst->thresholdLogLrt = 212644; //default threshold for LRT feature
		inst->maxLrt = 0x0080000;
		inst->minLrt = 104858;
	}
	inst->anaLen2 = WEBRTC_SPL_RSHIFT_W16(inst->anaLen, 1);
	inst->magnLen = inst->anaLen2 + 1;
/*
	if (inst->real_fft != NULL) {
		WebRtcSpl_FreeRealFFT(inst->real_fft);
	}
	inst->real_fft = WebRtcSpl_CreateRealFFT(inst->stages);
	if (inst->real_fft == NULL) {
		return -1;
	}
	*/
	inst->real_fft.order = (inst->stages);

	WebRtcSpl_ZerosArrayW16(inst->analysisBuffer, ANAL_BLOCKL_MAX);
	WebRtcSpl_ZerosArrayW16(inst->synthesisBuffer, ANAL_BLOCKL_MAX);

	// for HB processing
	//WebRtcSpl_ZerosArrayW16(inst->dataBufHBFX, ANAL_BLOCKL_MAX);
	// for quantile noise estimation
	WebRtcSpl_ZerosArrayW16(inst->noiseEstQuantile, HALF_ANAL_BLOCKL);
	for (i = 0; i < SIMULT * HALF_ANAL_BLOCKL; i++) {
		inst->noiseEstLogQuantile[i] = 2048; // Q8
		inst->noiseEstDensity[i] = 153; // Q9
	}
	for (i = 0; i < SIMULT; i++) {
		inst->noiseEstCounter[i] = (int16_t)(END_STARTUP_LONG * (i + 1)) / SIMULT;
	}

	// Initialize suppression filter with ones
	WebRtcSpl_MemSetW16((int16_t*)inst->noiseSupFilter, 16384, HALF_ANAL_BLOCKL);

	// Set the aggressiveness: default
	inst->aggrMode = 0;

	//initialize variables for new method
	inst->priorNonSpeechProb = 8192; // Q14(0.5) prior probability for speech/noise
	for (i = 0; i < HALF_ANAL_BLOCKL; i++) {
		inst->prevMagnU16[i] = 0;
		inst->prevNoiseU32[i] = 0; //previous noise-spectrum
		inst->logLrtTimeAvgW32[i] = 0; //smooth LR ratio
		inst->avgMagnPause[i] = 0; //conservative noise spectrum estimate
		inst->initMagnEst[i] = 0; //initial average magnitude spectrum
	}

	//feature quantities
	inst->thresholdSpecDiff = 50; //threshold for difference feature: determined on-line
	inst->thresholdSpecFlat = 20480; //threshold for flatness: determined on-line
	inst->featureLogLrt = inst->thresholdLogLrt; //average LRT factor (= threshold)
	inst->featureSpecFlat = inst->thresholdSpecFlat; //spectral flatness (= threshold)
	inst->featureSpecDiff = inst->thresholdSpecDiff; //spectral difference (= threshold)
	inst->weightLogLrt = 6; //default weighting par for LRT feature
	inst->weightSpecFlat = 0; //default weighting par for spectral flatness feature
	inst->weightSpecDiff = 0; //default weighting par for spectral difference feature

	inst->curAvgMagnEnergy = 0; //window time-average of input magnitude spectrum
	inst->timeAvgMagnEnergy = 0; //normalization for spectral difference
	inst->timeAvgMagnEnergyTmp = 0; //normalization for spectral difference

	//histogram quantities: used to estimate/update thresholds for features
	WebRtcSpl_ZerosArrayW16(inst->histLrt, HIST_PAR_EST);
	WebRtcSpl_ZerosArrayW16(inst->histSpecDiff, HIST_PAR_EST);
	WebRtcSpl_ZerosArrayW16(inst->histSpecFlat, HIST_PAR_EST);

	inst->blockIndex = -1; //frame counter

	//inst->modelUpdate    = 500;   //window for update
	inst->modelUpdate = (1 << STAT_UPDATES); //window for update
	inst->cntThresUpdate = 0; //counter feature thresholds updates

	inst->sumMagn = 0;
	inst->magnEnergy = 0;
	inst->prevQMagn = 0;
	inst->qNoise = 0;
	inst->prevQNoise = 0;

	inst->energyIn = 0;
	inst->scaleEnergyIn = 0;

	inst->whiteNoiseLevel = 0;
	inst->pinkNoiseNumerator = 0;
	inst->pinkNoiseExp = 0;
	inst->minNorm = 15; // Start with full scale
	inst->zeroInputSignal = 0;

	//default mode
	WebRtcNsx_set_policy_core(inst, 0);



	// Initialize function pointers.
	WebRtcNsx_NoiseEstimation = NoiseEstimationC;
	WebRtcNsx_PrepareSpectrum = PrepareSpectrumC;
	WebRtcNsx_SynthesisUpdate = SynthesisUpdateC;
	WebRtcNsx_AnalysisUpdate = AnalysisUpdateC;

#ifdef WEBRTC_DETECT_ARM_NEON
	uint64_t features = WebRtc_GetCPUFeaturesARM();
	if ((features & kCPUFeatureNEON) != 0) {
		WebRtcNsx_InitNeon();
	}
#elif defined(WEBRTC_ARCH_ARM_NEON)
	WebRtcNsx_InitNeon();
#endif

	inst->initFlag = 1;

	return 0;
}

int WebRtcNsx_set_policy_core(NsxInst_t * inst, int mode) {
	// allow for modes:0,1,2,3
	if (mode < 0 || mode > 3) {
		return -1;
	}

	inst->aggrMode = mode;
	if (mode == 0) {
		inst->overdrive = 256; // Q8(1.0)
		inst->denoiseBound = 8192; // Q14(0.5)
		inst->gainMap = 0; // No gain compensation
	}
	else if (mode == 1) {
		inst->overdrive = 256; // Q8(1.0)
		inst->denoiseBound = 4096; // Q14(0.25)
		inst->factor2Table = kFactor2Aggressiveness1;
		inst->gainMap = 1;
	}
	else if (mode == 2) {
		inst->overdrive = 282; // ~= Q8(1.1)
		inst->denoiseBound = 2048; // Q14(0.125)
		inst->factor2Table = kFactor2Aggressiveness2;
		inst->gainMap = 1;
	}
	else if (mode == 3) {
		//inst->overdrive = 320; // Q8(1.25)
		//inst->denoiseBound = 1475; // ~= Q14(0.09)
		inst->overdrive = 320; // Q8(1.25)
		inst->denoiseBound = 1475; // ~= Q14(0.09)
		inst->factor2Table = kFactor2Aggressiveness3;
		inst->gainMap = 1;
	}
	return 0;
}

// Extract thresholds for feature parameters
// histograms are computed over some window_size (given by window_pars)
// thresholds and weights are extracted every window
// flag 0 means update histogram only, flag 1 means compute the thresholds/weights
// threshold and weights are returned in: inst->priorModelPars
void WebRtcNsx_FeatureParameterExtraction(NsxInst_t * inst, int flag) {
	uint32_t tmpU32;
	uint32_t histIndex;
	uint32_t posPeak1SpecFlatFX, posPeak2SpecFlatFX;
	uint32_t posPeak1SpecDiffFX, posPeak2SpecDiffFX;

	int32_t tmp32;
	int32_t fluctLrtFX, thresFluctLrtFX;
	int32_t avgHistLrtFX, avgSquareHistLrtFX, avgHistLrtComplFX;

	int16_t j;
	int16_t numHistLrt;

	int i;
	int useFeatureSpecFlat, useFeatureSpecDiff, featureSum;
	int maxPeak1, maxPeak2;
	int weightPeak1SpecFlat, weightPeak2SpecFlat;
	int weightPeak1SpecDiff, weightPeak2SpecDiff;

	//update histograms
	if (!flag) {
		// LRT
		// Type casting to UWord32 is safe since negative values will not be wrapped to larger
		// values than HIST_PAR_EST
		histIndex = (uint32_t)(inst->featureLogLrt);
		if (histIndex < HIST_PAR_EST) {
			inst->histLrt[histIndex]++;
		}
		// Spectral flatness
		// (inst->featureSpecFlat*20)>>10 = (inst->featureSpecFlat*5)>>8
		histIndex = WEBRTC_SPL_RSHIFT_U32(inst->featureSpecFlat * 5, 8);
		if (histIndex < HIST_PAR_EST) {
			inst->histSpecFlat[histIndex]++;
		}
		// Spectral difference
		histIndex = HIST_PAR_EST;
		if (inst->timeAvgMagnEnergy > 0) {
			// Guard against division by zero
			// If timeAvgMagnEnergy == 0 we have no normalizing statistics and
			// therefore can't update the histogram
			histIndex = WEBRTC_SPL_UDIV((inst->featureSpecDiff * 5) >> inst->stages,
				inst->timeAvgMagnEnergy);
		}
		if (histIndex < HIST_PAR_EST) {
			inst->histSpecDiff[histIndex]++;
		}
	}

	// extract parameters for speech/noise probability
	if (flag) {
		useFeatureSpecDiff = 1;
		//for LRT feature:
		// compute the average over inst->featureExtractionParams.rangeAvgHistLrt
		avgHistLrtFX = 0;
		avgSquareHistLrtFX = 0;
		numHistLrt = 0;
		for (i = 0; i < BIN_SIZE_LRT; i++) {
			j = (2 * i + 1);
			tmp32 = WEBRTC_SPL_MUL_16_16(inst->histLrt[i], j);
			avgHistLrtFX += tmp32;
			numHistLrt += inst->histLrt[i];
			avgSquareHistLrtFX += WEBRTC_SPL_MUL_32_16(tmp32, j);
		}
		avgHistLrtComplFX = avgHistLrtFX;
		for (; i < HIST_PAR_EST; i++) {
			j = (2 * i + 1);
			tmp32 = WEBRTC_SPL_MUL_16_16(inst->histLrt[i], j);
			avgHistLrtComplFX += tmp32;
			avgSquareHistLrtFX += WEBRTC_SPL_MUL_32_16(tmp32, j);
		}
		fluctLrtFX = WEBRTC_SPL_MUL(avgSquareHistLrtFX, numHistLrt);
		fluctLrtFX -= WEBRTC_SPL_MUL(avgHistLrtFX, avgHistLrtComplFX);
		thresFluctLrtFX = THRES_FLUCT_LRT * numHistLrt;
		// get threshold for LRT feature:
		tmpU32 = (FACTOR_1_LRT_DIFF * (uint32_t)avgHistLrtFX);
		if ((fluctLrtFX < thresFluctLrtFX) || (numHistLrt == 0) ||
			(tmpU32 > (uint32_t)(100 * numHistLrt))) {
			//very low fluctuation, so likely noise
			inst->thresholdLogLrt = inst->maxLrt;
		}
		else {
			tmp32 = (int32_t)((tmpU32 << (9 + inst->stages)) / numHistLrt /
				25);
			// check if value is within min/max range
			inst->thresholdLogLrt = WEBRTC_SPL_SAT(inst->maxLrt,
				tmp32,
				inst->minLrt);
		}
		if (fluctLrtFX < thresFluctLrtFX) {
			// Do not use difference feature if fluctuation of LRT feature is very low:
			// most likely just noise state
			useFeatureSpecDiff = 0;
		}

		// for spectral flatness and spectral difference: compute the main peaks of histogram
		maxPeak1 = 0;
		maxPeak2 = 0;
		posPeak1SpecFlatFX = 0;
		posPeak2SpecFlatFX = 0;
		weightPeak1SpecFlat = 0;
		weightPeak2SpecFlat = 0;

		// peaks for flatness
		for (i = 0; i < HIST_PAR_EST; i++) {
			if (inst->histSpecFlat[i] > maxPeak1) {
				// Found new "first" peak
				maxPeak2 = maxPeak1;
				weightPeak2SpecFlat = weightPeak1SpecFlat;
				posPeak2SpecFlatFX = posPeak1SpecFlatFX;

				maxPeak1 = inst->histSpecFlat[i];
				weightPeak1SpecFlat = inst->histSpecFlat[i];
				posPeak1SpecFlatFX = (uint32_t)(2 * i + 1);
			}
			else if (inst->histSpecFlat[i] > maxPeak2) {
				// Found new "second" peak
				maxPeak2 = inst->histSpecFlat[i];
				weightPeak2SpecFlat = inst->histSpecFlat[i];
				posPeak2SpecFlatFX = (uint32_t)(2 * i + 1);
			}
		}

		// for spectral flatness feature
		useFeatureSpecFlat = 1;
		// merge the two peaks if they are close
		if ((posPeak1SpecFlatFX - posPeak2SpecFlatFX < LIM_PEAK_SPACE_FLAT_DIFF)
			&& (weightPeak2SpecFlat * LIM_PEAK_WEIGHT_FLAT_DIFF > weightPeak1SpecFlat)) {
			weightPeak1SpecFlat += weightPeak2SpecFlat;
			posPeak1SpecFlatFX = (posPeak1SpecFlatFX + posPeak2SpecFlatFX) >> 1;
		}
		//reject if weight of peaks is not large enough, or peak value too small
		if (weightPeak1SpecFlat < THRES_WEIGHT_FLAT_DIFF || posPeak1SpecFlatFX
			< THRES_PEAK_FLAT) {
			useFeatureSpecFlat = 0;
		}
		else { // if selected, get the threshold
	   // compute the threshold and check if value is within min/max range
			inst->thresholdSpecFlat = WEBRTC_SPL_SAT(MAX_FLAT_Q10, FACTOR_2_FLAT_Q10
				* posPeak1SpecFlatFX, MIN_FLAT_Q10); //Q10
		}
		// done with flatness feature

		if (useFeatureSpecDiff) {
			//compute two peaks for spectral difference
			maxPeak1 = 0;
			maxPeak2 = 0;
			posPeak1SpecDiffFX = 0;
			posPeak2SpecDiffFX = 0;
			weightPeak1SpecDiff = 0;
			weightPeak2SpecDiff = 0;
			// peaks for spectral difference
			for (i = 0; i < HIST_PAR_EST; i++) {
				if (inst->histSpecDiff[i] > maxPeak1) {
					// Found new "first" peak
					maxPeak2 = maxPeak1;
					weightPeak2SpecDiff = weightPeak1SpecDiff;
					posPeak2SpecDiffFX = posPeak1SpecDiffFX;

					maxPeak1 = inst->histSpecDiff[i];
					weightPeak1SpecDiff = inst->histSpecDiff[i];
					posPeak1SpecDiffFX = (uint32_t)(2 * i + 1);
				}
				else if (inst->histSpecDiff[i] > maxPeak2) {
					// Found new "second" peak
					maxPeak2 = inst->histSpecDiff[i];
					weightPeak2SpecDiff = inst->histSpecDiff[i];
					posPeak2SpecDiffFX = (uint32_t)(2 * i + 1);
				}
			}

			// merge the two peaks if they are close
			if ((posPeak1SpecDiffFX - posPeak2SpecDiffFX < LIM_PEAK_SPACE_FLAT_DIFF)
				&& (weightPeak2SpecDiff * LIM_PEAK_WEIGHT_FLAT_DIFF > weightPeak1SpecDiff)) {
				weightPeak1SpecDiff += weightPeak2SpecDiff;
				posPeak1SpecDiffFX = (posPeak1SpecDiffFX + posPeak2SpecDiffFX) >> 1;
			}
			// get the threshold value and check if value is within min/max range
			inst->thresholdSpecDiff = WEBRTC_SPL_SAT(MAX_DIFF, FACTOR_1_LRT_DIFF
				* posPeak1SpecDiffFX, MIN_DIFF); //5x bigger
//reject if weight of peaks is not large enough
			if (weightPeak1SpecDiff < THRES_WEIGHT_FLAT_DIFF) {
				useFeatureSpecDiff = 0;
			}
			// done with spectral difference feature
		}

		// select the weights between the features
		// inst->priorModelPars[4] is weight for LRT: always selected
		featureSum = 6 / (1 + useFeatureSpecFlat + useFeatureSpecDiff);
		inst->weightLogLrt = featureSum;
		inst->weightSpecFlat = useFeatureSpecFlat * featureSum;
		inst->weightSpecDiff = useFeatureSpecDiff * featureSum;

		// set histograms to zero for next update
		WebRtcSpl_ZerosArrayW16(inst->histLrt, HIST_PAR_EST);
		WebRtcSpl_ZerosArrayW16(inst->histSpecDiff, HIST_PAR_EST);
		WebRtcSpl_ZerosArrayW16(inst->histSpecFlat, HIST_PAR_EST);
	}  // end of flag == 1
}


// Compute spectral flatness on input spectrum
// magn is the magnitude spectrum
// spectral flatness is returned in inst->featureSpecFlat
void WebRtcNsx_ComputeSpectralFlatness(NsxInst_t * inst, uint16_t * magn) {
	uint32_t tmpU32;
	uint32_t avgSpectralFlatnessNum, avgSpectralFlatnessDen;

	int32_t tmp32;
	int32_t currentSpectralFlatness, logCurSpectralFlatness;

	int16_t zeros, frac, intPart;

	int i;

	// for flatness
	avgSpectralFlatnessNum = 0;
	avgSpectralFlatnessDen = inst->sumMagn - (uint32_t)magn[0]; // Q(normData-stages)

	// compute log of ratio of the geometric to arithmetic mean: check for log(0) case
	// flatness = exp( sum(log(magn[i]))/N - log(sum(magn[i])/N) )
	//          = exp( sum(log(magn[i]))/N ) * N / sum(magn[i])
	//          = 2^( sum(log2(magn[i]))/N - (log2(sum(magn[i])) - log2(N)) ) [This is used]
	for (i = 1; i < inst->magnLen; i++) {
		// First bin is excluded from spectrum measures. Number of bins is now a power of 2
		if (magn[i]) {
			zeros = WebRtcSpl_NormU32((uint32_t)magn[i]);
			frac = (int16_t)(((uint32_t)((uint32_t)(magn[i]) << zeros)
				& 0x7FFFFFFF) >> 23);
			// log2(magn(i))
			tmpU32 = (uint32_t)(((31 - zeros) << 8)
				+ WebRtcNsx_kLogTableFrac[frac]); // Q8
			avgSpectralFlatnessNum += tmpU32; // Q8
		}
		else {
			//if at least one frequency component is zero, treat separately
			tmpU32 = WEBRTC_SPL_UMUL_32_16(inst->featureSpecFlat, SPECT_FLAT_TAVG_Q14); // Q24
			inst->featureSpecFlat -= WEBRTC_SPL_RSHIFT_U32(tmpU32, 14); // Q10
			return;
		}
	}
	//ratio and inverse log: check for case of log(0)
	zeros = WebRtcSpl_NormU32(avgSpectralFlatnessDen);
	frac = (int16_t)(((avgSpectralFlatnessDen << zeros) & 0x7FFFFFFF) >> 23);
	// log2(avgSpectralFlatnessDen)
	tmp32 = (int32_t)(((31 - zeros) << 8) + WebRtcNsx_kLogTableFrac[frac]); // Q8
	logCurSpectralFlatness = (int32_t)avgSpectralFlatnessNum;
	logCurSpectralFlatness += ((int32_t)(inst->stages - 1) << (inst->stages + 7)); // Q(8+stages-1)
	logCurSpectralFlatness -= (tmp32 << (inst->stages - 1));
	logCurSpectralFlatness = WEBRTC_SPL_LSHIFT_W32(logCurSpectralFlatness, 10 - inst->stages); // Q17
	tmp32 = (int32_t)(0x00020000 | (WEBRTC_SPL_ABS_W32(logCurSpectralFlatness)
		& 0x0001FFFF)); //Q17
	intPart = -(int16_t)WEBRTC_SPL_RSHIFT_W32(logCurSpectralFlatness, 17);
	intPart += 7; // Shift 7 to get the output in Q10 (from Q17 = -17+10)
	if (intPart > 0) {
		currentSpectralFlatness = WEBRTC_SPL_RSHIFT_W32(tmp32, intPart);
	}
	else {
		currentSpectralFlatness = WEBRTC_SPL_LSHIFT_W32(tmp32, -intPart);
	}

	//time average update of spectral flatness feature
	tmp32 = currentSpectralFlatness - (int32_t)inst->featureSpecFlat; // Q10
	tmp32 = WEBRTC_SPL_MUL_32_16(SPECT_FLAT_TAVG_Q14, tmp32); // Q24
	inst->featureSpecFlat = (uint32_t)((int32_t)inst->featureSpecFlat
		+ WEBRTC_SPL_RSHIFT_W32(tmp32, 14)); // Q10
// done with flatness feature
}


// Compute the difference measure between input spectrum and a template/learned noise spectrum
// magn_tmp is the input spectrum
// the reference/template spectrum is  inst->magn_avg_pause[i]
// returns (normalized) spectral difference in inst->featureSpecDiff
void WebRtcNsx_ComputeSpectralDifference(NsxInst_t * inst, uint16_t * magnIn) {
	// This is to be calculated:
	// avgDiffNormMagn = var(magnIn) - cov(magnIn, magnAvgPause)^2 / var(magnAvgPause)

	uint32_t tmpU32no1, tmpU32no2;
	uint32_t varMagnUFX, varPauseUFX, avgDiffNormMagnUFX;

	int32_t tmp32no1, tmp32no2;
	int32_t avgPauseFX, avgMagnFX, covMagnPauseFX;
	int32_t maxPause, minPause;

	int16_t tmp16no1;

	int i, norm32, nShifts;

	avgPauseFX = 0;
	maxPause = 0;
	minPause = inst->avgMagnPause[0]; // Q(prevQMagn)
	// compute average quantities
	for (i = 0; i < inst->magnLen; i++) {
		// Compute mean of magn_pause
		avgPauseFX += inst->avgMagnPause[i]; // in Q(prevQMagn)
		maxPause = WEBRTC_SPL_MAX(maxPause, inst->avgMagnPause[i]);
		minPause = WEBRTC_SPL_MIN(minPause, inst->avgMagnPause[i]);
	}
	// normalize by replacing div of "inst->magnLen" with "inst->stages-1" shifts
	avgPauseFX = WEBRTC_SPL_RSHIFT_W32(avgPauseFX, inst->stages - 1);
	avgMagnFX = (int32_t)WEBRTC_SPL_RSHIFT_U32(inst->sumMagn, inst->stages - 1);
	// Largest possible deviation in magnPause for (co)var calculations
	tmp32no1 = WEBRTC_SPL_MAX(maxPause - avgPauseFX, avgPauseFX - minPause);
	// Get number of shifts to make sure we don't get wrap around in varPause
	nShifts = WEBRTC_SPL_MAX(0, 10 + inst->stages - WebRtcSpl_NormW32(tmp32no1));

	varMagnUFX = 0;
	varPauseUFX = 0;
	covMagnPauseFX = 0;
	for (i = 0; i < inst->magnLen; i++) {
		// Compute var and cov of magn and magn_pause
		tmp16no1 = (int16_t)((int32_t)magnIn[i] - avgMagnFX);
		tmp32no2 = inst->avgMagnPause[i] - avgPauseFX;
		varMagnUFX += (uint32_t)WEBRTC_SPL_MUL_16_16(tmp16no1, tmp16no1); // Q(2*qMagn)
		tmp32no1 = WEBRTC_SPL_MUL_32_16(tmp32no2, tmp16no1); // Q(prevQMagn+qMagn)
		covMagnPauseFX += tmp32no1; // Q(prevQMagn+qMagn)
		tmp32no1 = WEBRTC_SPL_RSHIFT_W32(tmp32no2, nShifts); // Q(prevQMagn-minPause)
		varPauseUFX += (uint32_t)WEBRTC_SPL_MUL(tmp32no1, tmp32no1); // Q(2*(prevQMagn-minPause))
	}
	//update of average magnitude spectrum: Q(-2*stages) and averaging replaced by shifts
	inst->curAvgMagnEnergy += WEBRTC_SPL_RSHIFT_U32(inst->magnEnergy, 2 * inst->normData
		+ inst->stages - 1);

	avgDiffNormMagnUFX = varMagnUFX; // Q(2*qMagn)
	if ((varPauseUFX) && (covMagnPauseFX)) {
		tmpU32no1 = (uint32_t)WEBRTC_SPL_ABS_W32(covMagnPauseFX); // Q(prevQMagn+qMagn)
		norm32 = WebRtcSpl_NormU32(tmpU32no1) - 16;
		if (norm32 > 0) {
			tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(tmpU32no1, norm32); // Q(prevQMagn+qMagn+norm32)
		}
		else {
			tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, -norm32); // Q(prevQMagn+qMagn+norm32)
		}
		tmpU32no2 = WEBRTC_SPL_UMUL(tmpU32no1, tmpU32no1); // Q(2*(prevQMagn+qMagn-norm32))

		nShifts += norm32;
		nShifts <<= 1;
		if (nShifts < 0) {
			varPauseUFX >>= (-nShifts); // Q(2*(qMagn+norm32+minPause))
			nShifts = 0;
		}
		if (varPauseUFX > 0) {
			// Q(2*(qMagn+norm32-16+minPause))
			tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no2, varPauseUFX);
			tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, nShifts);

			// Q(2*qMagn)
			avgDiffNormMagnUFX -= WEBRTC_SPL_MIN(avgDiffNormMagnUFX, tmpU32no1);
		}
		else {
			avgDiffNormMagnUFX = 0;
		}
	}
	//normalize and compute time average update of difference feature
	tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(avgDiffNormMagnUFX, 2 * inst->normData);
	if (inst->featureSpecDiff > tmpU32no1) {
		tmpU32no2 = WEBRTC_SPL_UMUL_32_16(inst->featureSpecDiff - tmpU32no1,
			SPECT_DIFF_TAVG_Q8); // Q(8-2*stages)
		inst->featureSpecDiff -= WEBRTC_SPL_RSHIFT_U32(tmpU32no2, 8); // Q(-2*stages)
	}
	else {
		tmpU32no2 = WEBRTC_SPL_UMUL_32_16(tmpU32no1 - inst->featureSpecDiff,
			SPECT_DIFF_TAVG_Q8); // Q(8-2*stages)
		inst->featureSpecDiff += WEBRTC_SPL_RSHIFT_U32(tmpU32no2, 8); // Q(-2*stages)
	}
}

// Compute speech/noise probability
// speech/noise probability is returned in: probSpeechFinal
//snrLocPrior is the prior SNR for each frequency (in Q11)
//snrLocPost is the post SNR for each frequency (in Q11)
void WebRtcNsx_SpeechNoiseProb(NsxInst_t * inst, uint16_t * nonSpeechProbFinal,
	uint32_t * priorLocSnr, uint32_t * postLocSnr) {
	uint32_t zeros, num, den, tmpU32no1, tmpU32no2, tmpU32no3;

	int32_t invLrtFX, indPriorFX, tmp32, tmp32no1, tmp32no2, besselTmpFX32;
	int32_t frac32, logTmp;
	int32_t logLrtTimeAvgKsumFX;

	int16_t indPriorFX16;
	int16_t tmp16, tmp16no1, tmp16no2, tmpIndFX, tableIndex, frac, intPart;

	int i, normTmp, normTmp2, nShifts;

	// compute feature based on average LR factor
	// this is the average over all frequencies of the smooth log LRT
	logLrtTimeAvgKsumFX = 0;
	for (i = 0; i < inst->magnLen; i++) {
		besselTmpFX32 = (int32_t)postLocSnr[i]; // Q11
		normTmp = WebRtcSpl_NormU32(postLocSnr[i]);
		num = WEBRTC_SPL_LSHIFT_U32(postLocSnr[i], normTmp); // Q(11+normTmp)
		if (normTmp > 10) {
			den = WEBRTC_SPL_LSHIFT_U32(priorLocSnr[i], normTmp - 11); // Q(normTmp)
		}
		else {
			den = WEBRTC_SPL_RSHIFT_U32(priorLocSnr[i], 11 - normTmp); // Q(normTmp)
		}
		if (den > 0) {
			besselTmpFX32 -= WEBRTC_SPL_UDIV(num, den); // Q11
		}
		else {
			besselTmpFX32 -= num; // Q11
		}

		// inst->logLrtTimeAvg[i] += LRT_TAVG * (besselTmp - log(snrLocPrior) - inst->logLrtTimeAvg[i]);
		// Here, LRT_TAVG = 0.5
		zeros = WebRtcSpl_NormU32(priorLocSnr[i]);
		frac32 = (int32_t)(((priorLocSnr[i] << zeros) & 0x7FFFFFFF) >> 19);
		tmp32 = WEBRTC_SPL_MUL(frac32, frac32);
		tmp32 = WEBRTC_SPL_RSHIFT_W32(WEBRTC_SPL_MUL(tmp32, -43), 19);
		tmp32 += WEBRTC_SPL_MUL_16_16_RSFT((int16_t)frac32, 5412, 12);
		frac32 = tmp32 + 37;
		// tmp32 = log2(priorLocSnr[i])
		tmp32 = (int32_t)(((31 - zeros) << 12) + frac32) - (11 << 12); // Q12
		logTmp = WEBRTC_SPL_RSHIFT_W32(WEBRTC_SPL_MUL_32_16(tmp32, 178), 8); // log2(priorLocSnr[i])*log(2)
		tmp32no1 = WEBRTC_SPL_RSHIFT_W32(logTmp + inst->logLrtTimeAvgW32[i], 1); // Q12
		inst->logLrtTimeAvgW32[i] += (besselTmpFX32 - tmp32no1); // Q12

		logLrtTimeAvgKsumFX += inst->logLrtTimeAvgW32[i]; // Q12
	}
	inst->featureLogLrt = WEBRTC_SPL_RSHIFT_W32(logLrtTimeAvgKsumFX * 5, inst->stages + 10); // 5 = BIN_SIZE_LRT / 2
	// done with computation of LR factor

	//
	//compute the indicator functions
	//

	// average LRT feature
	// FLOAT code
	// indicator0 = 0.5 * (tanh(widthPrior * (logLrtTimeAvgKsum - threshPrior0)) + 1.0);
	tmpIndFX = 16384; // Q14(1.0)
	tmp32no1 = logLrtTimeAvgKsumFX - inst->thresholdLogLrt; // Q12
	nShifts = 7 - inst->stages; // WIDTH_PR_MAP_SHIFT - inst->stages + 5;
	//use larger width in tanh map for pause regions
	if (tmp32no1 < 0) {
		tmpIndFX = 0;
		tmp32no1 = -tmp32no1;
		//widthPrior = widthPrior * 2.0;
		nShifts++;
	}
	tmp32no1 = WEBRTC_SPL_SHIFT_W32(tmp32no1, nShifts); // Q14
	// compute indicator function: sigmoid map
	tableIndex = (int16_t)WEBRTC_SPL_RSHIFT_W32(tmp32no1, 14);
	if ((tableIndex < 16) && (tableIndex >= 0)) {
		tmp16no2 = kIndicatorTable[tableIndex];
		tmp16no1 = kIndicatorTable[tableIndex + 1] - kIndicatorTable[tableIndex];
		frac = (int16_t)(tmp32no1 & 0x00003fff); // Q14
		tmp16no2 += (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(tmp16no1, frac, 14);
		if (tmpIndFX == 0) {
			tmpIndFX = 8192 - tmp16no2; // Q14
		}
		else {
			tmpIndFX = 8192 + tmp16no2; // Q14
		}
	}
	indPriorFX = WEBRTC_SPL_MUL_16_16(inst->weightLogLrt, tmpIndFX); // 6*Q14

	//spectral flatness feature
	if (inst->weightSpecFlat) {
		tmpU32no1 = WEBRTC_SPL_UMUL(inst->featureSpecFlat, 400); // Q10
		tmpIndFX = 16384; // Q14(1.0)
		//use larger width in tanh map for pause regions
		tmpU32no2 = inst->thresholdSpecFlat - tmpU32no1; //Q10
		nShifts = 4;
		if (inst->thresholdSpecFlat < tmpU32no1) {
			tmpIndFX = 0;
			tmpU32no2 = tmpU32no1 - inst->thresholdSpecFlat;
			//widthPrior = widthPrior * 2.0;
			nShifts++;
		}
		tmp32no1 = (int32_t)WebRtcSpl_DivU32U16(WEBRTC_SPL_LSHIFT_U32(tmpU32no2,
			nShifts), 25); //Q14
		tmpU32no1 = WebRtcSpl_DivU32U16(WEBRTC_SPL_LSHIFT_U32(tmpU32no2, nShifts), 25); //Q14
		// compute indicator function: sigmoid map
		// FLOAT code
		// indicator1 = 0.5 * (tanh(sgnMap * widthPrior * (threshPrior1 - tmpFloat1)) + 1.0);
		tableIndex = (int16_t)WEBRTC_SPL_RSHIFT_U32(tmpU32no1, 14);
		if (tableIndex < 16) {
			tmp16no2 = kIndicatorTable[tableIndex];
			tmp16no1 = kIndicatorTable[tableIndex + 1] - kIndicatorTable[tableIndex];
			frac = (int16_t)(tmpU32no1 & 0x00003fff); // Q14
			tmp16no2 += (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(tmp16no1, frac, 14);
			if (tmpIndFX) {
				tmpIndFX = 8192 + tmp16no2; // Q14
			}
			else {
				tmpIndFX = 8192 - tmp16no2; // Q14
			}
		}
		indPriorFX += WEBRTC_SPL_MUL_16_16(inst->weightSpecFlat, tmpIndFX); // 6*Q14
	}

	//for template spectral-difference
	if (inst->weightSpecDiff) {
		tmpU32no1 = 0;
		if (inst->featureSpecDiff) {
			normTmp = WEBRTC_SPL_MIN(20 - inst->stages,
				WebRtcSpl_NormU32(inst->featureSpecDiff));
			tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(inst->featureSpecDiff, normTmp); // Q(normTmp-2*stages)
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(inst->timeAvgMagnEnergy, 20 - inst->stages
				- normTmp);
			if (tmpU32no2 > 0) {
				// Q(20 - inst->stages)
				tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no1, tmpU32no2);
			}
			else {
				tmpU32no1 = (uint32_t)(0x7fffffff);
			}
		}
		tmpU32no3 = WEBRTC_SPL_UDIV(WEBRTC_SPL_LSHIFT_U32(inst->thresholdSpecDiff, 17), 25);
		tmpU32no2 = tmpU32no1 - tmpU32no3;
		nShifts = 1;
		tmpIndFX = 16384; // Q14(1.0)
		//use larger width in tanh map for pause regions
		if (tmpU32no2 & 0x80000000) {
			tmpIndFX = 0;
			tmpU32no2 = tmpU32no3 - tmpU32no1;
			//widthPrior = widthPrior * 2.0;
			nShifts--;
		}
		tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no2, nShifts);
		// compute indicator function: sigmoid map
		/* FLOAT code
		 indicator2 = 0.5 * (tanh(widthPrior * (tmpFloat1 - threshPrior2)) + 1.0);
		 */
		tableIndex = (int16_t)WEBRTC_SPL_RSHIFT_U32(tmpU32no1, 14);
		if (tableIndex < 16) {
			tmp16no2 = kIndicatorTable[tableIndex];
			tmp16no1 = kIndicatorTable[tableIndex + 1] - kIndicatorTable[tableIndex];
			frac = (int16_t)(tmpU32no1 & 0x00003fff); // Q14
			tmp16no2 += (int16_t)WEBRTC_SPL_MUL_16_16_RSFT_WITH_ROUND(
				tmp16no1, frac, 14);
			if (tmpIndFX) {
				tmpIndFX = 8192 + tmp16no2;
			}
			else {
				tmpIndFX = 8192 - tmp16no2;
			}
		}
		indPriorFX += WEBRTC_SPL_MUL_16_16(inst->weightSpecDiff, tmpIndFX); // 6*Q14
	}

	//combine the indicator function with the feature weights
	// FLOAT code
	// indPrior = 1 - (weightIndPrior0 * indicator0 + weightIndPrior1 * indicator1 + weightIndPrior2 * indicator2);
	indPriorFX16 = WebRtcSpl_DivW32W16ResW16(98307 - indPriorFX, 6); // Q14
	// done with computing indicator function

	//compute the prior probability
	// FLOAT code
	// inst->priorNonSpeechProb += PRIOR_UPDATE * (indPriorNonSpeech - inst->priorNonSpeechProb);
	tmp16 = indPriorFX16 - inst->priorNonSpeechProb; // Q14
	inst->priorNonSpeechProb += (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(
		PRIOR_UPDATE_Q14, tmp16, 14); // Q14

//final speech probability: combine prior model with LR factor:

	memset(nonSpeechProbFinal, 0, sizeof(uint16_t) * inst->magnLen);

	if (inst->priorNonSpeechProb > 0) {
		for (i = 0; i < inst->magnLen; i++) {
			// FLOAT code
			// invLrt = exp(inst->logLrtTimeAvg[i]);
			// invLrt = inst->priorSpeechProb * invLrt;
			// nonSpeechProbFinal[i] = (1.0 - inst->priorSpeechProb) / (1.0 - inst->priorSpeechProb + invLrt);
			// invLrt = (1.0 - inst->priorNonSpeechProb) * invLrt;
			// nonSpeechProbFinal[i] = inst->priorNonSpeechProb / (inst->priorNonSpeechProb + invLrt);
			if (inst->logLrtTimeAvgW32[i] < 65300) {
				tmp32no1 = WEBRTC_SPL_RSHIFT_W32(WEBRTC_SPL_MUL(inst->logLrtTimeAvgW32[i], 23637),
					14); // Q12
				intPart = (int16_t)WEBRTC_SPL_RSHIFT_W32(tmp32no1, 12);
				if (intPart < -8) {
					intPart = -8;
				}
				frac = (int16_t)(tmp32no1 & 0x00000fff); // Q12

				// Quadratic approximation of 2^frac
				tmp32no2 = WEBRTC_SPL_RSHIFT_W32(frac * frac * 44, 19); // Q12
				tmp32no2 += WEBRTC_SPL_MUL_16_16_RSFT(frac, 84, 7); // Q12
				invLrtFX = WEBRTC_SPL_LSHIFT_W32(1, 8 + intPart)
					+ WEBRTC_SPL_SHIFT_W32(tmp32no2, intPart - 4); // Q8

				normTmp = WebRtcSpl_NormW32(invLrtFX);
				normTmp2 = WebRtcSpl_NormW16((16384 - inst->priorNonSpeechProb));
				if (normTmp + normTmp2 >= 7) {
					if (normTmp + normTmp2 < 15) {
						invLrtFX = WEBRTC_SPL_RSHIFT_W32(invLrtFX, 15 - normTmp2 - normTmp);
						// Q(normTmp+normTmp2-7)
						tmp32no1 = WEBRTC_SPL_MUL_32_16(invLrtFX, (16384 - inst->priorNonSpeechProb));
						// Q(normTmp+normTmp2+7)
						invLrtFX = WEBRTC_SPL_SHIFT_W32(tmp32no1, 7 - normTmp - normTmp2); // Q14
					}
					else {
						tmp32no1 = WEBRTC_SPL_MUL_32_16(invLrtFX, (16384 - inst->priorNonSpeechProb)); // Q22
						invLrtFX = WEBRTC_SPL_RSHIFT_W32(tmp32no1, 8); // Q14
					}

					tmp32no1 = WEBRTC_SPL_LSHIFT_W32((int32_t)inst->priorNonSpeechProb, 8); // Q22

					nonSpeechProbFinal[i] = (uint16_t)WEBRTC_SPL_DIV(tmp32no1,
						(int32_t)inst->priorNonSpeechProb + invLrtFX); // Q8
				}
			}
		}
	}
}

// Transform input (speechFrame) to frequency domain magnitude (magnU16)
void WebRtcNsx_DataAnalysis(NsxInst_t * inst, short* speechFrame, uint16_t * magnU16) {

	uint32_t tmpU32no1, tmpU32no2;

	int32_t   tmp_1_w32 = 0;
	int32_t   tmp_2_w32 = 0;
	int32_t   sum_log_magn = 0;
	int32_t   sum_log_i_log_magn = 0;

	uint16_t  sum_log_magn_u16 = 0;
	uint16_t  tmp_u16 = 0;

	int16_t   sum_log_i = 0;
	int16_t   sum_log_i_square = 0;
	int16_t   frac = 0;
	int16_t   log2 = 0;
	int16_t   matrix_determinant = 0;
	int16_t   maxWinData;

	int i, j;
	int zeros;
	int net_norm = 0;
	int right_shifts_in_magnU16 = 0;
	int right_shifts_in_initMagnEst = 0;

	int16_t winData_buff[ANAL_BLOCKL_MAX * 2 + 16];
	int16_t realImag_buff[ANAL_BLOCKL_MAX * 2 + 16];

	// Align the structures to 32-byte boundary for the FFT function.
	int16_t* winData = (int16_t*)(((uintptr_t)winData_buff + 31) & ~31);
	int16_t * realImag = (int16_t*)(((uintptr_t)realImag_buff + 31) & ~31);

	// Update analysis buffer for lower band, and window data before FFT.
	WebRtcNsx_AnalysisUpdate(inst, winData, speechFrame);

	// Get input energy
	inst->energyIn = WebRtcSpl_Energy(winData, (int)inst->anaLen, &(inst->scaleEnergyIn));

	// Reset zero input flag
	inst->zeroInputSignal = 0;
	// Acquire norm for winData
	maxWinData = WebRtcSpl_MaxAbsValueW16(winData, inst->anaLen);
	inst->normData = WebRtcSpl_NormW16(maxWinData);
	if (maxWinData == 0) {
		// Treat zero input separately.
		inst->zeroInputSignal = 1;
		return;
	}

	// Determine the net normalization in the frequency domain
	net_norm = inst->stages - inst->normData;
	// Track lowest normalization factor and use it to prevent wrap around in shifting
	right_shifts_in_magnU16 = inst->normData - inst->minNorm;
	right_shifts_in_initMagnEst = WEBRTC_SPL_MAX(-right_shifts_in_magnU16, 0);
	inst->minNorm -= right_shifts_in_initMagnEst;
	right_shifts_in_magnU16 = WEBRTC_SPL_MAX(right_shifts_in_magnU16, 0);

	// create realImag as winData interleaved with zeros (= imag. part), normalize it
	NormalizeRealBuffer(inst, winData, realImag);

	// FFT output will be in winData[].
	WebRtcSpl_RealForwardFFT(&inst->real_fft, realImag, winData);

	inst->imag[0] = 0; // Q(normData-stages)
	inst->imag[inst->anaLen2] = 0;
	inst->real[0] = winData[0]; // Q(normData-stages)
	inst->real[inst->anaLen2] = winData[inst->anaLen];
	// Q(2*(normData-stages))
	inst->magnEnergy = (uint32_t)WEBRTC_SPL_MUL_16_16(inst->real[0], inst->real[0]);
	inst->magnEnergy += (uint32_t)WEBRTC_SPL_MUL_16_16(inst->real[inst->anaLen2],
		inst->real[inst->anaLen2]);
	magnU16[0] = (uint16_t)WEBRTC_SPL_ABS_W16(inst->real[0]); // Q(normData-stages)
	magnU16[inst->anaLen2] = (uint16_t)WEBRTC_SPL_ABS_W16(inst->real[inst->anaLen2]);
	inst->sumMagn = (uint32_t)magnU16[0]; // Q(normData-stages)
	inst->sumMagn += (uint32_t)magnU16[inst->anaLen2];

	if (inst->blockIndex >= END_STARTUP_SHORT) {
		for (i = 1, j = 2; i < inst->anaLen2; i += 1, j += 2) {
			inst->real[i] = winData[j];
			inst->imag[i] = -winData[j + 1];
			// magnitude spectrum
			// energy in Q(2*(normData-stages))
			tmpU32no1 = (uint32_t)WEBRTC_SPL_MUL_16_16(winData[j], winData[j]);
			tmpU32no1 += (uint32_t)WEBRTC_SPL_MUL_16_16(winData[j + 1], winData[j + 1]);
			inst->magnEnergy += tmpU32no1; // Q(2*(normData-stages))

			magnU16[i] = (uint16_t)WebRtcSpl_SqrtFloor(tmpU32no1); // Q(normData-stages)
			inst->sumMagn += (uint32_t)magnU16[i]; // Q(normData-stages)
		}
	}
	else {
		//
		// Gather information during startup for noise parameter estimation
		//

		// Switch initMagnEst to Q(minNorm-stages)
		inst->initMagnEst[0] = WEBRTC_SPL_RSHIFT_U32(inst->initMagnEst[0],
			right_shifts_in_initMagnEst);
		inst->initMagnEst[inst->anaLen2] =
			WEBRTC_SPL_RSHIFT_U32(inst->initMagnEst[inst->anaLen2],
				right_shifts_in_initMagnEst); // Q(minNorm-stages)

// Shift magnU16 to same domain as initMagnEst
		tmpU32no1 = WEBRTC_SPL_RSHIFT_W32((uint32_t)magnU16[0],
			right_shifts_in_magnU16); // Q(minNorm-stages)
		tmpU32no2 = WEBRTC_SPL_RSHIFT_W32((uint32_t)magnU16[inst->anaLen2],
			right_shifts_in_magnU16); // Q(minNorm-stages)

// Update initMagnEst
		inst->initMagnEst[0] += tmpU32no1; // Q(minNorm-stages)
		inst->initMagnEst[inst->anaLen2] += tmpU32no2; // Q(minNorm-stages)

		log2 = 0;
		if (magnU16[inst->anaLen2]) {
			// Calculate log2(magnU16[inst->anaLen2])
			zeros = WebRtcSpl_NormU32((uint32_t)magnU16[inst->anaLen2]);
			frac = (int16_t)((((uint32_t)magnU16[inst->anaLen2] << zeros) &
				0x7FFFFFFF) >> 23); // Q8
// log2(magnU16(i)) in Q8
			log2 = (int16_t)(((31 - zeros) << 8) + WebRtcNsx_kLogTableFrac[frac]);
		}

		sum_log_magn = (int32_t)log2; // Q8
		// sum_log_i_log_magn in Q17
		sum_log_i_log_magn = (WEBRTC_SPL_MUL_16_16(kLogIndex[inst->anaLen2], log2) >> 3);

		for (i = 1, j = 2; i < inst->anaLen2; i += 1, j += 2) {
			inst->real[i] = winData[j];
			inst->imag[i] = -winData[j + 1];
			// magnitude spectrum
			// energy in Q(2*(normData-stages))
			tmpU32no1 = (uint32_t)WEBRTC_SPL_MUL_16_16(winData[j], winData[j]);
			tmpU32no1 += (uint32_t)WEBRTC_SPL_MUL_16_16(winData[j + 1], winData[j + 1]);
			inst->magnEnergy += tmpU32no1; // Q(2*(normData-stages))

			magnU16[i] = (uint16_t)WebRtcSpl_SqrtFloor(tmpU32no1); // Q(normData-stages)
			inst->sumMagn += (uint32_t)magnU16[i]; // Q(normData-stages)

			// Switch initMagnEst to Q(minNorm-stages)
			inst->initMagnEst[i] = WEBRTC_SPL_RSHIFT_U32(inst->initMagnEst[i],
				right_shifts_in_initMagnEst);

			// Shift magnU16 to same domain as initMagnEst, i.e., Q(minNorm-stages)
			tmpU32no1 = WEBRTC_SPL_RSHIFT_W32((uint32_t)magnU16[i],
				right_shifts_in_magnU16);
			// Update initMagnEst
			inst->initMagnEst[i] += tmpU32no1; // Q(minNorm-stages)

			if (i >= kStartBand) {
				// For pink noise estimation. Collect data neglecting lower frequency band
				log2 = 0;
				if (magnU16[i]) {
					zeros = WebRtcSpl_NormU32((uint32_t)magnU16[i]);
					frac = (int16_t)((((uint32_t)magnU16[i] << zeros) &
						0x7FFFFFFF) >> 23);
					// log2(magnU16(i)) in Q8
					log2 = (int16_t)(((31 - zeros) << 8)
						+ WebRtcNsx_kLogTableFrac[frac]);
				}
				sum_log_magn += (int32_t)log2; // Q8
				// sum_log_i_log_magn in Q17
				sum_log_i_log_magn += (WEBRTC_SPL_MUL_16_16(kLogIndex[i], log2) >> 3);
			}
		}

		//
		//compute simplified noise model during startup
		//

		// Estimate White noise

		// Switch whiteNoiseLevel to Q(minNorm-stages)
		inst->whiteNoiseLevel = WEBRTC_SPL_RSHIFT_U32(inst->whiteNoiseLevel,
			right_shifts_in_initMagnEst);

		// Update the average magnitude spectrum, used as noise estimate.
		tmpU32no1 = WEBRTC_SPL_UMUL_32_16(inst->sumMagn, inst->overdrive);
		tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, inst->stages + 8);

		// Replacing division above with 'stages' shifts
		// Shift to same Q-domain as whiteNoiseLevel
		tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, right_shifts_in_magnU16);
		// This operation is safe from wrap around as long as END_STARTUP_SHORT < 128
		//assert(END_STARTUP_SHORT < 128);
		inst->whiteNoiseLevel += tmpU32no1; // Q(minNorm-stages)

		// Estimate Pink noise parameters
		// Denominator used in both parameter estimates.
		// The value is only dependent on the size of the frequency band (kStartBand)
		// and to reduce computational complexity stored in a table (kDeterminantEstMatrix[])
		//assert(kStartBand < 66);
		matrix_determinant = kDeterminantEstMatrix[kStartBand]; // Q0
		sum_log_i = kSumLogIndex[kStartBand]; // Q5
		sum_log_i_square = kSumSquareLogIndex[kStartBand]; // Q2
		if (inst->fs == 8000) {
			// Adjust values to shorter blocks in narrow band.
			tmp_1_w32 = (int32_t)matrix_determinant;
			tmp_1_w32 += WEBRTC_SPL_MUL_16_16_RSFT(kSumLogIndex[65], sum_log_i, 9);
			tmp_1_w32 -= WEBRTC_SPL_MUL_16_16_RSFT(kSumLogIndex[65], kSumLogIndex[65], 10);
			tmp_1_w32 -= WEBRTC_SPL_LSHIFT_W32((int32_t)sum_log_i_square, 4);
			tmp_1_w32 -= WEBRTC_SPL_MUL_16_16_RSFT((int16_t)
				(inst->magnLen - kStartBand), kSumSquareLogIndex[65], 2);
			matrix_determinant = (int16_t)tmp_1_w32;
			sum_log_i -= kSumLogIndex[65]; // Q5
			sum_log_i_square -= kSumSquareLogIndex[65]; // Q2
		}

		// Necessary number of shifts to fit sum_log_magn in a word16
		zeros = 16 - WebRtcSpl_NormW32(sum_log_magn);
		if (zeros < 0) {
			zeros = 0;
		}
		tmp_1_w32 = WEBRTC_SPL_LSHIFT_W32(sum_log_magn, 1); // Q9
		sum_log_magn_u16 = (uint16_t)WEBRTC_SPL_RSHIFT_W32(tmp_1_w32, zeros);//Q(9-zeros)

		// Calculate and update pinkNoiseNumerator. Result in Q11.
		tmp_2_w32 = WEBRTC_SPL_MUL_16_U16(sum_log_i_square, sum_log_magn_u16); // Q(11-zeros)
		tmpU32no1 = WEBRTC_SPL_RSHIFT_U32((uint32_t)sum_log_i_log_magn, 12); // Q5

		// Shift the largest value of sum_log_i and tmp32no3 before multiplication
		tmp_u16 = WEBRTC_SPL_LSHIFT_U16((uint16_t)sum_log_i, 1); // Q6
		if ((uint32_t)sum_log_i > tmpU32no1) {
			tmp_u16 = WEBRTC_SPL_RSHIFT_U16(tmp_u16, zeros);
		}
		else {
			tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, zeros);
		}
		tmp_2_w32 -= (int32_t)WEBRTC_SPL_UMUL_32_16(tmpU32no1, tmp_u16); // Q(11-zeros)
		matrix_determinant = WEBRTC_SPL_RSHIFT_W16(matrix_determinant, zeros); // Q(-zeros)
		tmp_2_w32 = WebRtcSpl_DivW32W16(tmp_2_w32, matrix_determinant); // Q11
		tmp_2_w32 += WEBRTC_SPL_LSHIFT_W32((int32_t)net_norm, 11); // Q11
		if (tmp_2_w32 < 0) {
			tmp_2_w32 = 0;
		}
		inst->pinkNoiseNumerator += tmp_2_w32; // Q11

		// Calculate and update pinkNoiseExp. Result in Q14.
		tmp_2_w32 = WEBRTC_SPL_MUL_16_U16(sum_log_i, sum_log_magn_u16); // Q(14-zeros)
		tmp_1_w32 = WEBRTC_SPL_RSHIFT_W32(sum_log_i_log_magn, 3 + zeros);
		tmp_1_w32 = WEBRTC_SPL_MUL((int32_t)(inst->magnLen - kStartBand),
			tmp_1_w32);
		tmp_2_w32 -= tmp_1_w32; // Q(14-zeros)
		if (tmp_2_w32 > 0) {
			// If the exponential parameter is negative force it to zero, which means a
			// flat spectrum.
			tmp_1_w32 = WebRtcSpl_DivW32W16(tmp_2_w32, matrix_determinant); // Q14
			inst->pinkNoiseExp += WEBRTC_SPL_SAT(16384, tmp_1_w32, 0); // Q14
		}
	}
}

void WebRtcNsx_DataSynthesis(NsxInst_t * inst, short* outFrame) {
	int32_t energyOut;

	int16_t realImag_buff[ANAL_BLOCKL_MAX * 2 + 16];
	int16_t rfft_out_buff[ANAL_BLOCKL_MAX * 2 + 16];

	// Align the structures to 32-byte boundary for the FFT function.
	int16_t* realImag = (int16_t*)(((uintptr_t)realImag_buff + 31) & ~31);
	int16_t * rfft_out = (int16_t*)(((uintptr_t)rfft_out_buff + 31) & ~31);

	int16_t tmp16no1, tmp16no2;
	int16_t energyRatio;
	int16_t gainFactor, gainFactor1, gainFactor2;

	int i;
	int outCIFFT;
	int scaleEnergyOut = 0;

	if (inst->zeroInputSignal) {
		// synthesize the special case of zero input
		// read out fully processed segment
		for (i = 0; i < inst->blockLen10ms; i++) {
			outFrame[i] = inst->synthesisBuffer[i]; // Q0
		}
		// update synthesis buffer
		WEBRTC_SPL_MEMCPY_W16(inst->synthesisBuffer,
			inst->synthesisBuffer + inst->blockLen10ms,
			inst->anaLen - inst->blockLen10ms);
		WebRtcSpl_ZerosArrayW16(inst->synthesisBuffer + inst->anaLen - inst->blockLen10ms,
			inst->blockLen10ms);
		return;
	}

	// Filter the data in the frequency domain, and create spectrum.
	WebRtcNsx_PrepareSpectrum(inst, realImag);

	// Inverse FFT output will be in rfft_out[].
	outCIFFT = WebRtcSpl_RealInverseFFT(&inst->real_fft, realImag, rfft_out);
//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D100, rfft_out);

	Denormalize(inst, rfft_out, outCIFFT);
	//DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D100, inst->real);
	//scale factor: only do it after END_STARTUP_LONG time
	gainFactor = 8192; // 8192 = Q13(1.0)
	if (inst->gainMap == 1 &&
		inst->blockIndex > END_STARTUP_LONG &&
		inst->energyIn > 0) {
		energyOut = WebRtcSpl_Energy(inst->real, (int)inst->anaLen, &scaleEnergyOut); // Q(-scaleEnergyOut)
		if (scaleEnergyOut == 0 && !(energyOut & 0x7f800000)) {
			energyOut = WEBRTC_SPL_SHIFT_W32(energyOut, 8 + scaleEnergyOut
				- inst->scaleEnergyIn);
		}
		else {
			inst->energyIn = WEBRTC_SPL_RSHIFT_W32(inst->energyIn, 8 + scaleEnergyOut
				- inst->scaleEnergyIn); // Q(-8-scaleEnergyOut)
		}

		//assert(inst->energyIn > 0);
		energyRatio = (int16_t)WEBRTC_SPL_DIV(energyOut
			+ WEBRTC_SPL_RSHIFT_W32(inst->energyIn, 1), inst->energyIn); // Q8
		// Limit the ratio to [0, 1] in Q8, i.e., [0, 256]
		energyRatio = WEBRTC_SPL_SAT(256, energyRatio, 0);

		// all done in lookup tables now
		//assert(energyRatio < 257);
		gainFactor1 = kFactor1Table[energyRatio]; // Q8
		gainFactor2 = inst->factor2Table[energyRatio]; // Q8

		//combine both scales with speech/noise prob: note prior (priorSpeechProb) is not frequency dependent

		// factor = inst->priorSpeechProb*factor1 + (1.0-inst->priorSpeechProb)*factor2; // original code
		tmp16no1 = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(16384 - inst->priorNonSpeechProb,
			gainFactor1, 14); // Q13 16384 = Q14(1.0)
		tmp16no2 = (int16_t)WEBRTC_SPL_MUL_16_16_RSFT(inst->priorNonSpeechProb,
			gainFactor2, 14); // Q13;
		gainFactor = tmp16no1 + tmp16no2; // Q13
	}  // out of flag_gain_map==1

	// Synthesis, read out fully processed segment, and update synthesis buffer.
	// DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D10a,outFrame );
	WebRtcNsx_SynthesisUpdate(inst, outFrame, gainFactor);
	// DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D10a,outFrame );
}

int WebRtcNsx_ProcessCore(NsxInst_t * inst, short* speechFrame, short* speechFrameHB,
	short* outFrame, short* outFrameHB) {
	// main routine for noise suppression

	uint32_t tmpU32no1, tmpU32no2, tmpU32no3;
	uint32_t satMax, maxNoiseU32;
	uint32_t tmpMagnU32, tmpNoiseU32;
	uint32_t nearMagnEst;
	uint32_t noiseUpdateU32;
	uint32_t noiseU32[HALF_ANAL_BLOCKL];
	uint32_t postLocSnr[HALF_ANAL_BLOCKL];
	uint32_t priorLocSnr[HALF_ANAL_BLOCKL];
	uint32_t prevNearSnr[HALF_ANAL_BLOCKL];
	uint32_t curNearSnr;
	uint32_t priorSnr;
	uint32_t noise_estimate = 0;
	uint32_t noise_estimate_avg = 0;
	uint32_t numerator = 0;

	int32_t tmp32no1, tmp32no2;
	int32_t pink_noise_num_avg = 0;

	uint16_t tmpU16no1;
	uint16_t magnU16[HALF_ANAL_BLOCKL];
	uint16_t prevNoiseU16[HALF_ANAL_BLOCKL];
	uint16_t nonSpeechProbFinal[HALF_ANAL_BLOCKL];
	uint16_t gammaNoise, prevGammaNoise;
	uint16_t noiseSupFilterTmp[HALF_ANAL_BLOCKL];

	int16_t qMagn, qNoise;
	//int16_t avgProbSpeechHB, gainModHB, avgFilterGainHB, gainTimeDomainHB;
	int16_t pink_noise_exp_avg = 0;

	int i;
	int nShifts, postShifts;
	int norm32no1, norm32no2;
	int flag, sign;
	int q_domain_to_use = 0;

	// Code for ARMv7-Neon platform assumes the following:
	//assert(inst->anaLen > 0);
	//assert(inst->anaLen2 > 0);
	//assert(inst->anaLen % 16 == 0);
	//assert(inst->anaLen2 % 8 == 0);
	//assert(inst->blockLen10ms > 0);
	//assert(inst->blockLen10ms % 16 == 0);
	//assert(inst->magnLen == inst->anaLen2 + 1);



	// Check that initialization has been done
	if (inst->initFlag != 1) {
		return -1;
	}
	// Check for valid pointers based on sampling rate
	if ((inst->fs == 32000) && (speechFrameHB == NULL)) {
		return -1;
	}

	// Store speechFrame and transform to frequency domain
	WebRtcNsx_DataAnalysis(inst, speechFrame, magnU16);
	if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
	{
		Bt_CVSDDataPolling();
	}
       
        
	if (inst->zeroInputSignal) {
		WebRtcNsx_DataSynthesis(inst, outFrame);

//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D10b, &outFrame);
		return 0;
	}

	// Update block index when we have something to process
	inst->blockIndex++;
	//

	// Norm of magn
	qMagn = inst->normData - inst->stages;

	// Compute spectral flatness on input spectrum
	WebRtcNsx_ComputeSpectralFlatness(inst, magnU16);

	// quantile noise estimate
	
	NoiseEstimationC(inst, magnU16, noiseU32, &qNoise);
	
	if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
	{
		Bt_CVSDDataPolling();
	}
	
	//noise estimate from previous frame
	for (i = 0; i < inst->magnLen; i++) {
		prevNoiseU16[i] = (uint16_t)WEBRTC_SPL_RSHIFT_U32(inst->prevNoiseU32[i], 11); // Q(prevQNoise)
	}

	if (inst->blockIndex < END_STARTUP_SHORT) {
		// Noise Q-domain to be used later; see description at end of section.
		q_domain_to_use = WEBRTC_SPL_MIN((int)qNoise, inst->minNorm - inst->stages);

		// Calculate frequency independent parts in parametric noise estimate and calculate
		// the estimate for the lower frequency band (same values for all frequency bins)
		if (inst->pinkNoiseExp) {
			pink_noise_exp_avg = (int16_t)WebRtcSpl_DivW32W16(inst->pinkNoiseExp,
				(int16_t)(inst->blockIndex + 1)); // Q14
			pink_noise_num_avg = WebRtcSpl_DivW32W16(inst->pinkNoiseNumerator,
				(int16_t)(inst->blockIndex + 1)); // Q11
			WebRtcNsx_CalcParametricNoiseEstimate(inst,
				pink_noise_exp_avg,
				pink_noise_num_avg,
				kStartBand,
				&noise_estimate,
				&noise_estimate_avg);
		}
		else {
			// Use white noise estimate if we have poor pink noise parameter estimates
			noise_estimate = inst->whiteNoiseLevel; // Q(minNorm-stages)
			noise_estimate_avg = noise_estimate / (inst->blockIndex + 1); // Q(minNorm-stages)
		}
		for (i = 0; i < inst->magnLen; i++) {
			// Estimate the background noise using the pink noise parameters if permitted
			if ((inst->pinkNoiseExp) && (i >= kStartBand)) {
				// Reset noise_estimate
				noise_estimate = 0;
				noise_estimate_avg = 0;
				// Calculate the parametric noise estimate for current frequency bin
				WebRtcNsx_CalcParametricNoiseEstimate(inst,
					pink_noise_exp_avg,
					pink_noise_num_avg,
					i,
					&noise_estimate,
					&noise_estimate_avg);
			}
			// Calculate parametric Wiener filter
			noiseSupFilterTmp[i] = inst->denoiseBound;
			if (inst->initMagnEst[i]) {
				// numerator = (initMagnEst - noise_estimate * overdrive)
				// Result in Q(8+minNorm-stages)
				tmpU32no1 = WEBRTC_SPL_UMUL_32_16(noise_estimate, inst->overdrive);
				numerator = WEBRTC_SPL_LSHIFT_U32(inst->initMagnEst[i], 8);
				if (numerator > tmpU32no1) {
					// Suppression filter coefficient larger than zero, so calculate.
					numerator -= tmpU32no1;

					// Determine number of left shifts in numerator for best accuracy after
					// division
					nShifts = WebRtcSpl_NormU32(numerator);
					nShifts = WEBRTC_SPL_SAT(6, nShifts, 0);

					// Shift numerator to Q(nShifts+8+minNorm-stages)
					numerator = WEBRTC_SPL_LSHIFT_U32(numerator, nShifts);

					// Shift denominator to Q(nShifts-6+minNorm-stages)
					tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(inst->initMagnEst[i], 6 - nShifts);
					if (tmpU32no1 == 0) {
						// This is only possible if numerator = 0, in which case
						// we don't need any division.
						tmpU32no1 = 1;
					}
					tmpU32no2 = WEBRTC_SPL_UDIV(numerator, tmpU32no1); // Q14
					noiseSupFilterTmp[i] = (uint16_t)WEBRTC_SPL_SAT(16384, tmpU32no2,
						(uint32_t)(inst->denoiseBound)); // Q14
				}
			}
			// Weight quantile noise 'noiseU32' with modeled noise 'noise_estimate_avg'
			// 'noiseU32 is in Q(qNoise) and 'noise_estimate' in Q(minNorm-stages)
			// To guarantee that we do not get wrap around when shifting to the same domain
			// we use the lowest one. Furthermore, we need to save 6 bits for the weighting.
			// 'noise_estimate_avg' can handle this operation by construction, but 'noiseU32'
			// may not.

			// Shift 'noiseU32' to 'q_domain_to_use'
			tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(noiseU32[i], (int)qNoise - q_domain_to_use);
			// Shift 'noise_estimate_avg' to 'q_domain_to_use'
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(noise_estimate_avg, inst->minNorm - inst->stages
				- q_domain_to_use);
			// Make a simple check to see if we have enough room for weighting 'tmpU32no1'
			// without wrap around
			nShifts = 0;
			if (tmpU32no1 & 0xfc000000) {
				tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, 6);
				tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(tmpU32no2, 6);
				nShifts = 6;
			}
			tmpU32no1 *= inst->blockIndex;
			tmpU32no2 *= (END_STARTUP_SHORT - inst->blockIndex);
			// Add them together and divide by startup length
			noiseU32[i] = WebRtcSpl_DivU32U16(tmpU32no1 + tmpU32no2, END_STARTUP_SHORT);
			// Shift back if necessary
			noiseU32[i] = WEBRTC_SPL_LSHIFT_U32(noiseU32[i], nShifts);
			
			if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
			{
				Bt_CVSDDataPolling();
			}
			
		}
		// Update new Q-domain for 'noiseU32'
		qNoise = q_domain_to_use;
	}
	// compute average signal during END_STARTUP_LONG time:
	// used to normalize spectral difference measure
	if (inst->blockIndex < END_STARTUP_LONG) {
		// substituting division with shift ending up in Q(-2*stages)
		inst->timeAvgMagnEnergyTmp
			+= WEBRTC_SPL_RSHIFT_U32(inst->magnEnergy,
				2 * inst->normData + inst->stages - 1);
		inst->timeAvgMagnEnergy = WebRtcSpl_DivU32U16(inst->timeAvgMagnEnergyTmp,
			inst->blockIndex + 1);
	}

	//start processing at frames == converged+1
	// STEP 1: compute prior and post SNR based on quantile noise estimates

	// compute direct decision (DD) estimate of prior SNR: needed for new method
	satMax = (uint32_t)1048575;// Largest possible value without getting overflow despite shifting 12 steps
	postShifts = 6 + qMagn - qNoise;
	nShifts = 5 - inst->prevQMagn + inst->prevQNoise;
	for (i = 0; i < inst->magnLen; i++) {
		// calculate post SNR: output in Q11
		postLocSnr[i] = 2048; // 1.0 in Q11
		tmpU32no1 = WEBRTC_SPL_LSHIFT_U32((uint32_t)magnU16[i], 6); // Q(6+qMagn)
		if (postShifts < 0) {
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(noiseU32[i], -postShifts); // Q(6+qMagn)
		}
		else {
			tmpU32no2 = WEBRTC_SPL_LSHIFT_U32(noiseU32[i], postShifts); // Q(6+qMagn)
		}
		if (tmpU32no1 > tmpU32no2) {
			// Current magnitude larger than noise
			tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(tmpU32no1, 11); // Q(17+qMagn)
			if (tmpU32no2 > 0) {
				tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no1, tmpU32no2); // Q11
				postLocSnr[i] = WEBRTC_SPL_MIN(satMax, tmpU32no1); // Q11
			}
			else {
				postLocSnr[i] = satMax;
			}
		}

		// calculate prevNearSnr[i] and save for later instead of recalculating it later
		nearMagnEst = WEBRTC_SPL_UMUL_16_16(inst->prevMagnU16[i], inst->noiseSupFilter[i]); // Q(prevQMagn+14)
		tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(nearMagnEst, 3); // Q(prevQMagn+17)
		tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(inst->prevNoiseU32[i], nShifts); // Q(prevQMagn+6)

		if (tmpU32no2 > 0) {
			tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no1, tmpU32no2); // Q11
			tmpU32no1 = WEBRTC_SPL_MIN(satMax, tmpU32no1); // Q11
		}
		else {
			tmpU32no1 = satMax; // Q11
		}
		prevNearSnr[i] = tmpU32no1; // Q11

		//directed decision update of priorSnr
		tmpU32no1 = WEBRTC_SPL_UMUL_32_16(prevNearSnr[i], DD_PR_SNR_Q11); // Q22
		tmpU32no2 = WEBRTC_SPL_UMUL_32_16(postLocSnr[i] - 2048, ONE_MINUS_DD_PR_SNR_Q11); // Q22
		priorSnr = tmpU32no1 + tmpU32no2 + 512; // Q22 (added 512 for rounding)
		// priorLocSnr = 1 + 2*priorSnr
		priorLocSnr[i] = 2048 + WEBRTC_SPL_RSHIFT_U32(priorSnr, 10); // Q11
		if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
		{
			Bt_CVSDDataPolling();
		}
	}  // end of loop over frequencies
	// done with step 1: DD computation of prior and post SNR

	// STEP 2: compute speech/noise likelihood

	//compute difference of input spectrum with learned/estimated noise spectrum
	WebRtcNsx_ComputeSpectralDifference(inst, magnU16);
	//compute histograms for determination of parameters (thresholds and weights for features)
	//parameters are extracted once every window time (=inst->modelUpdate)
	//counter update
	inst->cntThresUpdate++;
	flag = (int)(inst->cntThresUpdate == inst->modelUpdate);
	//update histogram
	WebRtcNsx_FeatureParameterExtraction(inst, flag);

	if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
	{
		Bt_CVSDDataPolling();
	}
	
	//compute model parameters
	if (flag) {
		inst->cntThresUpdate = 0; // Reset counter
		//update every window:
		// get normalization for spectral difference for next window estimate

		// Shift to Q(-2*stages)
		inst->curAvgMagnEnergy = WEBRTC_SPL_RSHIFT_U32(inst->curAvgMagnEnergy, STAT_UPDATES);

		tmpU32no1 = (inst->curAvgMagnEnergy + inst->timeAvgMagnEnergy + 1) >> 1; //Q(-2*stages)
		// Update featureSpecDiff
		if ((tmpU32no1 != inst->timeAvgMagnEnergy) && (inst->featureSpecDiff) &&
			(inst->timeAvgMagnEnergy > 0)) {
			norm32no1 = 0;
			tmpU32no3 = tmpU32no1;
			while (0xFFFF0000 & tmpU32no3) {
				tmpU32no3 >>= 1;
				norm32no1++;
			}
			tmpU32no2 = inst->featureSpecDiff;
			while (0xFFFF0000 & tmpU32no2) {
				tmpU32no2 >>= 1;
				norm32no1++;
			}
			tmpU32no3 = WEBRTC_SPL_UMUL(tmpU32no3, tmpU32no2);
			tmpU32no3 = WEBRTC_SPL_UDIV(tmpU32no3, inst->timeAvgMagnEnergy);
			if (WebRtcSpl_NormU32(tmpU32no3) < norm32no1) {
				inst->featureSpecDiff = 0x007FFFFF;
			}
			else {
				inst->featureSpecDiff = WEBRTC_SPL_MIN(0x007FFFFF,
					WEBRTC_SPL_LSHIFT_U32(tmpU32no3, norm32no1));
			}
		}

		inst->timeAvgMagnEnergy = tmpU32no1; // Q(-2*stages)
		inst->curAvgMagnEnergy = 0;
	}

	//compute speech/noise probability
	WebRtcNsx_SpeechNoiseProb(inst, nonSpeechProbFinal, priorLocSnr, postLocSnr);

	if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
	{
		Bt_CVSDDataPolling();
	}
	
	//time-avg parameter for noise update
	gammaNoise = NOISE_UPDATE_Q8; // Q8

	maxNoiseU32 = 0;
	postShifts = inst->prevQNoise - qMagn;
	nShifts = inst->prevQMagn - qMagn;
	for (i = 0; i < inst->magnLen; i++) {
		// temporary noise update: use it for speech frames if update value is less than previous
		// the formula has been rewritten into:
		// noiseUpdate = noisePrev[i] + (1 - gammaNoise) * nonSpeechProb * (magn[i] - noisePrev[i])

		if (postShifts < 0) {
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(magnU16[i], -postShifts); // Q(prevQNoise)
		}
		else {
			tmpU32no2 = WEBRTC_SPL_LSHIFT_U32(magnU16[i], postShifts); // Q(prevQNoise)
		}
		if (prevNoiseU16[i] > tmpU32no2) {
			sign = -1;
			tmpU32no1 = prevNoiseU16[i] - tmpU32no2;
		}
		else {
			sign = 1;
			tmpU32no1 = tmpU32no2 - prevNoiseU16[i];
		}
		noiseUpdateU32 = inst->prevNoiseU32[i]; // Q(prevQNoise+11)
		tmpU32no3 = 0;
		if ((tmpU32no1) && (nonSpeechProbFinal[i])) {
			// This value will be used later, if gammaNoise changes
			tmpU32no3 = WEBRTC_SPL_UMUL_32_16(tmpU32no1, nonSpeechProbFinal[i]); // Q(prevQNoise+8)
			if (0x7c000000 & tmpU32no3) {
				// Shifting required before multiplication
				tmpU32no2
					= WEBRTC_SPL_UMUL_32_16(WEBRTC_SPL_RSHIFT_U32(tmpU32no3, 5), gammaNoise); // Q(prevQNoise+11)
			}
			else {
				// We can do shifting after multiplication
				tmpU32no2
					= WEBRTC_SPL_RSHIFT_U32(WEBRTC_SPL_UMUL_32_16(tmpU32no3, gammaNoise), 5); // Q(prevQNoise+11)
			}
			if (sign > 0) {
				noiseUpdateU32 += tmpU32no2; // Q(prevQNoise+11)
			}
			else {
				// This operation is safe. We can never get wrap around, since worst
				// case scenario means magnU16 = 0
				noiseUpdateU32 -= tmpU32no2; // Q(prevQNoise+11)
			}
		}

		//increase gamma (i.e., less noise update) for frame likely to be speech
		prevGammaNoise = gammaNoise;
		gammaNoise = NOISE_UPDATE_Q8;
		//time-constant based on speech/noise state
		//increase gamma (i.e., less noise update) for frames likely to be speech
		if (nonSpeechProbFinal[i] < ONE_MINUS_PROB_RANGE_Q8) {
			gammaNoise = GAMMA_NOISE_TRANS_AND_SPEECH_Q8;
		}

		if (prevGammaNoise != gammaNoise) {
			// new noise update
			// this line is the same as above, only that the result is stored in a different variable and the gammaNoise
			// has changed
			//
			// noiseUpdate = noisePrev[i] + (1 - gammaNoise) * nonSpeechProb * (magn[i] - noisePrev[i])

			if (0x7c000000 & tmpU32no3) {
				// Shifting required before multiplication
				tmpU32no2
					= WEBRTC_SPL_UMUL_32_16(WEBRTC_SPL_RSHIFT_U32(tmpU32no3, 5), gammaNoise); // Q(prevQNoise+11)
			}
			else {
				// We can do shifting after multiplication
				tmpU32no2
					= WEBRTC_SPL_RSHIFT_U32(WEBRTC_SPL_UMUL_32_16(tmpU32no3, gammaNoise), 5); // Q(prevQNoise+11)
			}
			if (sign > 0) {
				tmpU32no1 = inst->prevNoiseU32[i] + tmpU32no2; // Q(prevQNoise+11)
			}
			else {
				tmpU32no1 = inst->prevNoiseU32[i] - tmpU32no2; // Q(prevQNoise+11)
			}
			if (noiseUpdateU32 > tmpU32no1) {
				noiseUpdateU32 = tmpU32no1; // Q(prevQNoise+11)
			}
		}
		noiseU32[i] = noiseUpdateU32; // Q(prevQNoise+11)
		if (noiseUpdateU32 > maxNoiseU32) {
			maxNoiseU32 = noiseUpdateU32;
		}

		// conservative noise update
		// // original FLOAT code
		// if (prob_speech < PROB_RANGE) {
		// inst->avgMagnPause[i] = inst->avgMagnPause[i] + (1.0 - gamma_pause)*(magn[i] - inst->avgMagnPause[i]);
		// }

		tmp32no2 = WEBRTC_SPL_SHIFT_W32(inst->avgMagnPause[i], -nShifts);
		if (nonSpeechProbFinal[i] > ONE_MINUS_PROB_RANGE_Q8) {
			if (nShifts < 0) {
				tmp32no1 = (int32_t)magnU16[i] - tmp32no2; // Q(qMagn)
				tmp32no1 = WEBRTC_SPL_MUL_32_16(tmp32no1, ONE_MINUS_GAMMA_PAUSE_Q8); // Q(8+prevQMagn+nShifts)
				tmp32no1 = WEBRTC_SPL_RSHIFT_W32(tmp32no1 + 128, 8); // Q(qMagn)
			}
			else {
				tmp32no1 = WEBRTC_SPL_LSHIFT_W32((int32_t)magnU16[i], nShifts)
					- inst->avgMagnPause[i]; // Q(qMagn+nShifts)
				tmp32no1 = WEBRTC_SPL_MUL_32_16(tmp32no1, ONE_MINUS_GAMMA_PAUSE_Q8); // Q(8+prevQMagn+nShifts)
				tmp32no1 = WEBRTC_SPL_RSHIFT_W32(tmp32no1 + (128 << nShifts), 8 + nShifts); // Q(qMagn)
			}
			tmp32no2 += tmp32no1; // Q(qMagn)
		}
		inst->avgMagnPause[i] = tmp32no2;
	}  // end of frequency loop

	norm32no1 = WebRtcSpl_NormU32(maxNoiseU32);
	qNoise = inst->prevQNoise + norm32no1 - 5;
	// done with step 2: noise update

	// STEP 3: compute dd update of prior snr and post snr based on new noise estimate
	nShifts = inst->prevQNoise + 11 - qMagn;
	for (i = 0; i < inst->magnLen; i++) {
		// FLOAT code
		// // post and prior SNR
		// curNearSnr = 0.0;
		// if (magn[i] > noise[i])
		// {
		// curNearSnr = magn[i] / (noise[i] + 0.0001) - 1.0;
		// }
		// // DD estimate is sum of two terms: current estimate and previous estimate
		// // directed decision update of snrPrior
		// snrPrior = DD_PR_SNR * prevNearSnr[i] + (1.0 - DD_PR_SNR) * curNearSnr;
		// // gain filter
		// tmpFloat1 = inst->overdrive + snrPrior;
		// tmpFloat2 = snrPrior / tmpFloat1;
		// theFilter[i] = tmpFloat2;

		// calculate curNearSnr again, this is necessary because a new noise estimate has been made since then. for the original
		curNearSnr = 0; // Q11
		if (nShifts < 0) {
			// This case is equivalent with magn < noise which implies curNearSnr = 0;
			tmpMagnU32 = (uint32_t)magnU16[i]; // Q(qMagn)
			tmpNoiseU32 = WEBRTC_SPL_LSHIFT_U32(noiseU32[i], -nShifts); // Q(qMagn)
		}
		else if (nShifts > 17) {
			tmpMagnU32 = WEBRTC_SPL_LSHIFT_U32(magnU16[i], 17); // Q(qMagn+17)
			tmpNoiseU32 = WEBRTC_SPL_RSHIFT_U32(noiseU32[i], nShifts - 17); // Q(qMagn+17)
		}
		else {
			tmpMagnU32 = WEBRTC_SPL_LSHIFT_U32((uint32_t)magnU16[i], nShifts); // Q(qNoise_prev+11)
			tmpNoiseU32 = noiseU32[i]; // Q(qNoise_prev+11)
		}
		if (tmpMagnU32 > tmpNoiseU32) {
			tmpU32no1 = tmpMagnU32 - tmpNoiseU32; // Q(qCur)
			norm32no2 = WEBRTC_SPL_MIN(11, WebRtcSpl_NormU32(tmpU32no1));
			tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(tmpU32no1, norm32no2); // Q(qCur+norm32no2)
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(tmpNoiseU32, 11 - norm32no2); // Q(qCur+norm32no2-11)
			if (tmpU32no2 > 0) {
				tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no1, tmpU32no2); // Q11
			}
			curNearSnr = WEBRTC_SPL_MIN(satMax, tmpU32no1); // Q11
		}

		//directed decision update of priorSnr
		// FLOAT
		// priorSnr = DD_PR_SNR * prevNearSnr + (1.0-DD_PR_SNR) * curNearSnr;

		tmpU32no1 = WEBRTC_SPL_UMUL_32_16(prevNearSnr[i], DD_PR_SNR_Q11); // Q22
		tmpU32no2 = WEBRTC_SPL_UMUL_32_16(curNearSnr, ONE_MINUS_DD_PR_SNR_Q11); // Q22
		priorSnr = tmpU32no1 + tmpU32no2; // Q22

		//gain filter
		tmpU32no1 = (uint32_t)(inst->overdrive)
			+ WEBRTC_SPL_RSHIFT_U32(priorSnr + 8192, 14); // Q8
		//assert(inst->overdrive > 0);
		tmpU16no1 = (uint16_t)WEBRTC_SPL_UDIV(priorSnr + (tmpU32no1 >> 1), tmpU32no1); // Q14
		inst->noiseSupFilter[i] = WEBRTC_SPL_SAT(16384, tmpU16no1, inst->denoiseBound); // 16384 = Q14(1.0) // Q14

		// Weight in the parametric Wiener filter during startup
		if (inst->blockIndex < END_STARTUP_SHORT) {
			// Weight the two suppression filters
			tmpU32no1 = WEBRTC_SPL_UMUL_16_16(inst->noiseSupFilter[i],
				(uint16_t)inst->blockIndex);
			tmpU32no2 = WEBRTC_SPL_UMUL_16_16(noiseSupFilterTmp[i],
				(uint16_t)(END_STARTUP_SHORT
					- inst->blockIndex));
			tmpU32no1 += tmpU32no2;
			inst->noiseSupFilter[i] = (uint16_t)WebRtcSpl_DivU32U16(tmpU32no1,
				END_STARTUP_SHORT);
		}
	}  // end of loop over frequencies
	//done with step3

	// save noise and magnitude spectrum for next frame
	inst->prevQNoise = qNoise;
	inst->prevQMagn = qMagn;
	if (norm32no1 > 5) {
		for (i = 0; i < inst->magnLen; i++) {
			inst->prevNoiseU32[i] = WEBRTC_SPL_LSHIFT_U32(noiseU32[i], norm32no1 - 5); // Q(qNoise+11)
			inst->prevMagnU16[i] = magnU16[i]; // Q(qMagn)
		}
	}
	else {
		for (i = 0; i < inst->magnLen; i++) {
			inst->prevNoiseU32[i] = WEBRTC_SPL_RSHIFT_U32(noiseU32[i], 5 - norm32no1); // Q(qNoise+11)
			inst->prevMagnU16[i] = magnU16[i]; // Q(qMagn)
		}
	}
	if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
	{
		Bt_CVSDDataPolling();
	}
	
	WebRtcNsx_DataSynthesis(inst, outFrame);

//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D109, outFrame);
	
	return 0;
}
int WebRtcNsx_ProcessCoreMsbc(NsxInst_t * inst, short* speechFrame, short* speechFrameHB,
	short* outFrame, short* outFrameHB) {
	// main routine for noise suppression

	uint32_t tmpU32no1, tmpU32no2;//, tmpU32no3;
	uint32_t satMax;//, maxNoiseU32;
//	uint32_t tmpMagnU32, tmpNoiseU32;
	uint32_t nearMagnEst;
//	uint32_t noiseUpdateU32;
	uint32_t noiseU32[HALF_ANAL_BLOCKL];
	uint32_t postLocSnr[HALF_ANAL_BLOCKL];
	uint32_t priorLocSnr[HALF_ANAL_BLOCKL];
	uint32_t prevNearSnr[HALF_ANAL_BLOCKL];
//	uint32_t curNearSnr;
	uint32_t priorSnr;
	uint32_t noise_estimate = 0;
	uint32_t noise_estimate_avg = 0;
	uint32_t numerator = 0;

//	int32_t tmp32no1, tmp32no2;
	int32_t pink_noise_num_avg = 0;

	uint16_t tmpU16no1;
	uint16_t magnU16[HALF_ANAL_BLOCKL];
	uint16_t prevNoiseU16[HALF_ANAL_BLOCKL];
//	uint16_t nonSpeechProbFinal[HALF_ANAL_BLOCKL];
//	uint16_t gammaNoise, prevGammaNoise;
	uint16_t noiseSupFilterTmp[HALF_ANAL_BLOCKL];

	int16_t qMagn, qNoise;
	//int16_t avgProbSpeechHB, gainModHB, avgFilterGainHB, gainTimeDomainHB;
	int16_t pink_noise_exp_avg = 0;

	int i;
	int nShifts, postShifts;
//	int norm32no1, norm32no2;
//	int flag, sign;
	int q_domain_to_use = 0;

	// Code for ARMv7-Neon platform assumes the following:
	//assert(inst->anaLen > 0);
	//assert(inst->anaLen2 > 0);
	//assert(inst->anaLen % 16 == 0);
	//assert(inst->anaLen2 % 8 == 0);
	//assert(inst->blockLen10ms > 0);
	//assert(inst->blockLen10ms % 16 == 0);
	//assert(inst->magnLen == inst->anaLen2 + 1);



	// Check that initialization has been done
	if (inst->initFlag != 1) {
		return -1;
	}
	// Check for valid pointers based on sampling rate
	if ((inst->fs == 32000) && (speechFrameHB == NULL)) {
		return -1;
	}

	// Store speechFrame and transform to frequency domain
	WebRtcNsx_DataAnalysis(inst, speechFrame, magnU16);
	if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
       	{	
       		Bt_CVSDDataPolling();
       	}
        
	if (inst->zeroInputSignal) {
		WebRtcNsx_DataSynthesis(inst, outFrame);

//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D10b, &outFrame);
		return 0;
	}

	// Update block index when we have something to process
	inst->blockIndex++;
	//

	// Norm of magn
	qMagn = inst->normData - inst->stages;

	// Compute spectral flatness on input spectrum
	WebRtcNsx_ComputeSpectralFlatness(inst, magnU16);

	// quantile noise estimate
	
	NoiseEstimationC(inst, magnU16, noiseU32, &qNoise);
	
	
	//noise estimate from previous frame
	for (i = 0; i < inst->magnLen; i++) {
		prevNoiseU16[i] = (uint16_t)WEBRTC_SPL_RSHIFT_U32(inst->prevNoiseU32[i], 11); // Q(prevQNoise)
	}

	if (inst->blockIndex < END_STARTUP_SHORT) {
		// Noise Q-domain to be used later; see description at end of section.
		q_domain_to_use = WEBRTC_SPL_MIN((int)qNoise, inst->minNorm - inst->stages);

		// Calculate frequency independent parts in parametric noise estimate and calculate
		// the estimate for the lower frequency band (same values for all frequency bins)
		if (inst->pinkNoiseExp) {
			pink_noise_exp_avg = (int16_t)WebRtcSpl_DivW32W16(inst->pinkNoiseExp,
				(int16_t)(inst->blockIndex + 1)); // Q14
			pink_noise_num_avg = WebRtcSpl_DivW32W16(inst->pinkNoiseNumerator,
				(int16_t)(inst->blockIndex + 1)); // Q11
			WebRtcNsx_CalcParametricNoiseEstimate(inst,
				pink_noise_exp_avg,
				pink_noise_num_avg,
				kStartBand,
				&noise_estimate,
				&noise_estimate_avg);
		}
		else {
			// Use white noise estimate if we have poor pink noise parameter estimates
			noise_estimate = inst->whiteNoiseLevel; // Q(minNorm-stages)
			noise_estimate_avg = noise_estimate / (inst->blockIndex + 1); // Q(minNorm-stages)
		}
		for (i = 0; i < inst->magnLen; i++) {
			// Estimate the background noise using the pink noise parameters if permitted
			if ((inst->pinkNoiseExp) && (i >= kStartBand)) {
				// Reset noise_estimate
				noise_estimate = 0;
				noise_estimate_avg = 0;
				// Calculate the parametric noise estimate for current frequency bin
				WebRtcNsx_CalcParametricNoiseEstimate(inst,
					pink_noise_exp_avg,
					pink_noise_num_avg,
					i,
					&noise_estimate,
					&noise_estimate_avg);
			}
			// Calculate parametric Wiener filter
			noiseSupFilterTmp[i] = inst->denoiseBound;
			if (inst->initMagnEst[i]) {
				// numerator = (initMagnEst - noise_estimate * overdrive)
				// Result in Q(8+minNorm-stages)
				tmpU32no1 = WEBRTC_SPL_UMUL_32_16(noise_estimate, inst->overdrive);
				numerator = WEBRTC_SPL_LSHIFT_U32(inst->initMagnEst[i], 8);
				if (numerator > tmpU32no1) {
					// Suppression filter coefficient larger than zero, so calculate.
					numerator -= tmpU32no1;

					// Determine number of left shifts in numerator for best accuracy after
					// division
					nShifts = WebRtcSpl_NormU32(numerator);
					nShifts = WEBRTC_SPL_SAT(6, nShifts, 0);

					// Shift numerator to Q(nShifts+8+minNorm-stages)
					numerator = WEBRTC_SPL_LSHIFT_U32(numerator, nShifts);

					// Shift denominator to Q(nShifts-6+minNorm-stages)
					tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(inst->initMagnEst[i], 6 - nShifts);
					if (tmpU32no1 == 0) {
						// This is only possible if numerator = 0, in which case
						// we don't need any division.
						tmpU32no1 = 1;
					}
					tmpU32no2 = WEBRTC_SPL_UDIV(numerator, tmpU32no1); // Q14
					noiseSupFilterTmp[i] = (uint16_t)WEBRTC_SPL_SAT(16384, tmpU32no2,
						(uint32_t)(inst->denoiseBound)); // Q14
				}
			}
			// Weight quantile noise 'noiseU32' with modeled noise 'noise_estimate_avg'
			// 'noiseU32 is in Q(qNoise) and 'noise_estimate' in Q(minNorm-stages)
			// To guarantee that we do not get wrap around when shifting to the same domain
			// we use the lowest one. Furthermore, we need to save 6 bits for the weighting.
			// 'noise_estimate_avg' can handle this operation by construction, but 'noiseU32'
			// may not.

			// Shift 'noiseU32' to 'q_domain_to_use'
			tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(noiseU32[i], (int)qNoise - q_domain_to_use);
			// Shift 'noise_estimate_avg' to 'q_domain_to_use'
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(noise_estimate_avg, inst->minNorm - inst->stages
				- q_domain_to_use);
			// Make a simple check to see if we have enough room for weighting 'tmpU32no1'
			// without wrap around
			nShifts = 0;
			if (tmpU32no1 & 0xfc000000) {
				tmpU32no1 = WEBRTC_SPL_RSHIFT_U32(tmpU32no1, 6);
				tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(tmpU32no2, 6);
				nShifts = 6;
			}
			tmpU32no1 *= inst->blockIndex;
			tmpU32no2 *= (END_STARTUP_SHORT - inst->blockIndex);
			// Add them together and divide by startup length
			noiseU32[i] = WebRtcSpl_DivU32U16(tmpU32no1 + tmpU32no2, END_STARTUP_SHORT);
			// Shift back if necessary
			noiseU32[i] = WEBRTC_SPL_LSHIFT_U32(noiseU32[i], nShifts);
			
		}
		// Update new Q-domain for 'noiseU32'
		qNoise = q_domain_to_use;
	}
	// compute average signal during END_STARTUP_LONG time:
	// used to normalize spectral difference measure
	if (inst->blockIndex < END_STARTUP_LONG) {
		// substituting division with shift ending up in Q(-2*stages)
		inst->timeAvgMagnEnergyTmp
			+= WEBRTC_SPL_RSHIFT_U32(inst->magnEnergy,
				2 * inst->normData + inst->stages - 1);
		inst->timeAvgMagnEnergy = WebRtcSpl_DivU32U16(inst->timeAvgMagnEnergyTmp,
			inst->blockIndex + 1);
	}

	//start processing at frames == converged+1
	// STEP 1: compute prior and post SNR based on quantile noise estimates

	// compute direct decision (DD) estimate of prior SNR: needed for new method
	satMax = (uint32_t)1048575;// Largest possible value without getting overflow despite shifting 12 steps
	postShifts = 6 + qMagn - qNoise;
	nShifts = 5 - inst->prevQMagn + inst->prevQNoise;
	for (i = 0; i < inst->magnLen; i++) {
		// calculate post SNR: output in Q11
		postLocSnr[i] = 2048; // 1.0 in Q11
		tmpU32no1 = WEBRTC_SPL_LSHIFT_U32((uint32_t)magnU16[i], 6); // Q(6+qMagn)
		if (postShifts < 0) {
			tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(noiseU32[i], -postShifts); // Q(6+qMagn)
		}
		else {
			tmpU32no2 = WEBRTC_SPL_LSHIFT_U32(noiseU32[i], postShifts); // Q(6+qMagn)
		}
		if (tmpU32no1 > tmpU32no2) {
			// Current magnitude larger than noise
			tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(tmpU32no1, 11); // Q(17+qMagn)
			if (tmpU32no2 > 0) {
				tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no1, tmpU32no2); // Q11
				postLocSnr[i] = WEBRTC_SPL_MIN(satMax, tmpU32no1); // Q11
			}
			else {
				postLocSnr[i] = satMax;
			}
		}

		// calculate prevNearSnr[i] and save for later instead of recalculating it later
		nearMagnEst = WEBRTC_SPL_UMUL_16_16(inst->prevMagnU16[i], inst->noiseSupFilter[i]); // Q(prevQMagn+14)
		tmpU32no1 = WEBRTC_SPL_LSHIFT_U32(nearMagnEst, 3); // Q(prevQMagn+17)
		tmpU32no2 = WEBRTC_SPL_RSHIFT_U32(inst->prevNoiseU32[i], nShifts); // Q(prevQMagn+6)

		if (tmpU32no2 > 0) {
			tmpU32no1 = WEBRTC_SPL_UDIV(tmpU32no1, tmpU32no2); // Q11
			tmpU32no1 = WEBRTC_SPL_MIN(satMax, tmpU32no1); // Q11
		}
		else {
			tmpU32no1 = satMax; // Q11
		}
		prevNearSnr[i] = tmpU32no1; // Q11

		//directed decision update of priorSnr
		tmpU32no1 = WEBRTC_SPL_UMUL_32_16(prevNearSnr[i], DD_PR_SNR_Q11); // Q22
		tmpU32no2 = WEBRTC_SPL_UMUL_32_16(postLocSnr[i] - 2048, ONE_MINUS_DD_PR_SNR_Q11); // Q22
		priorSnr = tmpU32no1 + tmpU32no2 + 512; // Q22 (added 512 for rounding)
		// priorLocSnr = 1 + 2*priorSnr
		priorLocSnr[i] = 2048 + WEBRTC_SPL_RSHIFT_U32(priorSnr, 10); // Q11
		if(HREAD(mem_codec_type) == CODEC_TYPE_CVSD)
       		{	
       			Bt_CVSDDataPolling();
       		}

		//gain filter
		tmpU32no1 = (uint32_t)(inst->overdrive)
			+ WEBRTC_SPL_RSHIFT_U32(priorSnr + 8192, 14); // Q8
		//assert(inst->overdrive > 0);
		tmpU16no1 = (uint16_t)WEBRTC_SPL_UDIV(priorSnr + (tmpU32no1 >> 1), tmpU32no1); // Q14
		inst->noiseSupFilter[i] = WEBRTC_SPL_SAT(16384, tmpU16no1, inst->denoiseBound); // 16384 = Q14(1.0) // Q14

		// Weight in the parametric Wiener filter during startup
		if (inst->blockIndex < END_STARTUP_SHORT) {
			// Weight the two suppression filters
			tmpU32no1 = WEBRTC_SPL_UMUL_16_16(inst->noiseSupFilter[i],
				(uint16_t)inst->blockIndex);
			tmpU32no2 = WEBRTC_SPL_UMUL_16_16(noiseSupFilterTmp[i],
				(uint16_t)(END_STARTUP_SHORT
					- inst->blockIndex));
			tmpU32no1 += tmpU32no2;
			inst->noiseSupFilter[i] = (uint16_t)WebRtcSpl_DivU32U16(tmpU32no1,
				END_STARTUP_SHORT);
		}
	}  // end of loop over frequencies
	//done with step3

	// save noise and magnitude spectrum for next frame
	inst->prevQNoise = qNoise;
	inst->prevQMagn = qMagn;
		for (i = 0; i < inst->magnLen; i++) {
			inst->prevNoiseU32[i] = WEBRTC_SPL_LSHIFT_W32(noiseU32[i], 11); // Q(qNoise+11)
			inst->prevMagnU16[i] = magnU16[i]; // Q(qMagn)
	}

	
	WebRtcNsx_DataSynthesis(inst, outFrame);

//	DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"Bt_EvtCallBack: 0x%04X", LOG_POINT_D109, outFrame);
	
	return 0;
}



int WebRtcNsx_Free(NsxHandle* nsxInst) {
  //WebRtcSpl_FreeRealFFT(((NsxInst_t*)nsxInst)->real_fft);
  //free(nsxInst);
  return 0;
}

int WebRtcNsx_Init(NsxHandle* nsxInst, uint32_t fs) {
  return WebRtcNsx_InitCore((NsxInst_t*)nsxInst, fs);
}

int WebRtcNsx_set_policy(NsxHandle* nsxInst, int mode) {
  return WebRtcNsx_set_policy_core((NsxInst_t*)nsxInst, mode);
}

int WebRtcNsx_Process(NsxHandle* nsxInst, short* speechFrame,
                      short* speechFrameHB, short* outFrame,
                      short* outFrameHB) {
     if(HREAD(mem_codec_type) == CODEC_TYPE_mSBC)
     {
     		 return WebRtcNsx_ProcessCoreMsbc(
      (NsxInst_t*)nsxInst, speechFrame, speechFrameHB, outFrame, outFrameHB);
     }
     else
     {
     		 return WebRtcNsx_ProcessCore(
      (NsxInst_t*)nsxInst, speechFrame, speechFrameHB, outFrame, outFrameHB);
     }
}
