#include "yc_mediafarrow.h"
#include "yc11xx.h"
#include "yc_debug.h"


// For 44.1 or 48K
int32_t uk_base = 0;

int32_t gMediaSampleDiff;

//#define FILTER_BYPASS
short xEQ[2][3] = {  0, 0, 0,0 ,0 ,0 };  /* x(n), x(n-1), x(n-2). Must be static */
short yEQ[2][3] = {  0, 0, 0,0 ,0 ,0 };  /* y(n), y(n-1), y(n-2). Must be static */

const int32_t acc_uk_48K = 1<<30;

#define FILTER_STATE_LEN_48K 8
int16_t src_state_48K[FILTER_STATE_LEN_48K] = {0};

#if 1  //3333
#define FILTER_STATE_LEN 8

const int32_t acc_uk = 1<<30;
int32_t delta_uk = 0;
const int16_t CoefB0[8] = { 0,	0,	0,	8192,	0,	0,	0,	0 };
const int16_t CoefB1[8] = { 78, - 819,	4915,	2048, - 8192,	2458, - 546,	59 };
const int16_t CoefB2[8] = {-46, - 614,	6144, - 11150,	6144, - 614,	46,	0 };
const int16_t CoefB3[8] = { -91,	808, - 546, - 2788,	5006, - 3038,	728, - 80 };
const int16_t CoefB4[8] = { -57,	683, - 2219,	3186, - 2219,	683, - 57,	0 };
const int16_t CoefB5[8] = { 11,	23, - 307,	796, - 967,	614, - 193,	23 };
const int16_t CoefB6[8] = { 11, - 68,	171, - 228,	171, - 68,	11,	0 };
const int16_t CoefB7[8] = { 2, - 11,	34, - 57,	57, - 34,	11, - 2 };

short* gpFilterMediaFarrowOut;
uint16_t gFilterMediaFarrowOutSize;
uint16_t gFilterMediaFarrowOutIndex;

int16_t src_state[8] = {0};
int32_t uk_tmp = 0;

void FilterMediaFarrowInit(uint8_t dac_clk, short * dataStartAddr, uint16_t size)
{
	uint8_t i;
	gpFilterMediaFarrowOut = dataStartAddr;
	gFilterMediaFarrowOutSize = size;
	gFilterMediaFarrowOutIndex = 0;

	for(i=0; i < FILTER_STATE_LEN; i++)
	{
		src_state[i] = 0;
	}
	for(i=0; i < FILTER_STATE_LEN_48K; i++)
	{
		src_state_48K[i] = 0;
	}
	for(int i =0; i < 2; i ++) 
	{
		for(int j =0; j < 3; j ++) 
		{
			xEQ[i][j] = 0;
			yEQ[i][j] = 0;
		}
	}
	uk_tmp = 0;
	// Our 44.1 dac real is 768/136/128(Digital)=44.117647K
	// filter uk unit is 1024*1024*1024, so delta_uk is (44.117647/44.1 - 1)*1024*1024*1024
	if(dac_clk == 44)
	{
		//uk_base = -400160;
		uk_base = 0;// Use 0 will always add packet, if user can't here it, just use it, it will get high tone quality. 
	}
	else
	{
		uk_base = 0;
	}
	delta_uk = uk_base;
	//delta_uk = 0;
	gMediaSampleDiff = 0;
}

void FilterMediaFarrowWork_44K(int16_t din_point)
{
	int32_t interp_out;
	int32_t uk = 0;
	int32_t temp0 = 0, temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0, temp5 = 0, temp6 = 0, temp7 = 0;
	int32_t uk_alter_tmp;
	int ii, jj;

	if (delta_uk <= 0) {
		uk_tmp = uk_tmp + acc_uk + delta_uk;
		if (uk_tmp >= 1 << 30)
			uk_tmp = uk_tmp - (1 << 30);

		src_state[0] = din_point;

		temp0 = src_state[3];
		for (ii = 0; ii < 8; ii++)
		{
			temp1 += CoefB1[ii] * src_state[ii];
			temp2 += CoefB2[ii] * src_state[ii];
			temp3 += CoefB3[ii] * src_state[ii];
			temp4 += CoefB4[ii] * src_state[ii];
			temp5 += CoefB5[ii] * src_state[ii];
			temp6 += CoefB6[ii] * src_state[ii];
			temp7 += CoefB7[ii] * src_state[ii];
		}

		for (jj = 6; jj > -1; jj--)
			src_state[jj + 1] = src_state[jj];

    temp1 = temp1>>13;
    temp2 = temp2>>13;
    temp3 = temp3>>13;
    temp4 = temp4>>13;
	temp5 = temp5 >> 13;
	temp6 = temp6 >> 13;
	temp7 = temp7 >> 13;

	uk = uk_tmp >> 22;


	interp_out = temp7 * uk;
	interp_out = interp_out >> 8;

	interp_out = (interp_out + temp6) * uk;
	interp_out = interp_out >> 8;

	interp_out = (interp_out + temp5) * uk;
	interp_out = interp_out >> 8;

	interp_out = (interp_out + temp4) * uk;
	interp_out = interp_out >> 8;


    interp_out = (interp_out+temp3)*uk;
    interp_out = interp_out>>8;

    interp_out = (interp_out+temp2)*uk;
    interp_out = interp_out>>8;

    interp_out = (interp_out+temp1)*uk;
    interp_out = interp_out>>8;

	interp_out = interp_out + temp0;

		if (interp_out > 32767) interp_out = 32767;
		else if (interp_out < -32767) interp_out = -32767;

			FilterMediaFarrowValue(interp_out);

		uk_alter_tmp = uk_tmp + acc_uk + delta_uk;

		if (uk_alter_tmp < 1 << 30) {

			uk = uk_alter_tmp >> 22;

			interp_out = temp7 * uk;
			interp_out = interp_out >> 8;

			interp_out = (interp_out + temp6) * uk;
			interp_out = interp_out >> 8;

			interp_out = (interp_out + temp5) * uk;
			interp_out = interp_out >> 8;

			interp_out = (interp_out + temp4) * uk;
			interp_out = interp_out >> 8;


			interp_out = (interp_out + temp3) * uk;
			interp_out = interp_out >> 8;

			interp_out = (interp_out + temp2) * uk;
			interp_out = interp_out >> 8;

			interp_out = (interp_out + temp1) * uk;
			interp_out = interp_out >> 8;

			interp_out = interp_out + temp0;

    
        if(interp_out > 32767) interp_out = 32767;
        else if(interp_out < -32767) interp_out = -32767;
    
			FilterMediaFarrowValue(interp_out);

			uk_tmp = uk_alter_tmp;
		}
	}
	else {
	
		if ((uk_tmp + delta_uk) >= 1 << 30) {
			uk_tmp = uk_tmp - (1 << 30);
			src_state[0] = din_point;
			for (jj = 6; jj > -1; jj--)
				src_state[jj + 1] = src_state[jj];
		}
		else {
			uk_tmp = uk_tmp + delta_uk;
			
			src_state[0] = din_point;

			temp0 = src_state[3];
			for (ii = 0; ii < 8; ii++)
			{
				temp1 += CoefB1[ii] * src_state[ii];
				temp2 += CoefB2[ii] * src_state[ii];
				temp3 += CoefB3[ii] * src_state[ii];
				temp4 += CoefB4[ii] * src_state[ii];
				temp5 += CoefB5[ii] * src_state[ii];
				temp6 += CoefB6[ii] * src_state[ii];
				temp7 += CoefB7[ii] * src_state[ii];
			}

			for (jj = 6; jj > -1; jj--)
				src_state[jj + 1] = src_state[jj];

			temp1 = temp1 >> 13;
			temp2 = temp2 >> 13;
			temp3 = temp3 >> 13;
			temp4 = temp4 >> 13;
			temp5 = temp5 >> 13;
			temp6 = temp6 >> 13;
			temp7 = temp7 >> 13;

		 uk = uk_tmp >> 22;

		 interp_out = temp7 * uk;
		 interp_out = interp_out >> 8;

		 interp_out = (interp_out + temp6) * uk;
		 interp_out = interp_out >> 8;

		 interp_out = (interp_out + temp5) * uk;
		 interp_out = interp_out >> 8;

		 interp_out = (interp_out + temp4) * uk;
		 interp_out = interp_out >> 8;


		 interp_out = (interp_out + temp3) * uk;
		 interp_out = interp_out >> 8;

		 interp_out = (interp_out + temp2) * uk;
		 interp_out = interp_out >> 8;

		 interp_out = (interp_out + temp1) * uk;
		 interp_out = interp_out >> 8;

		 interp_out = interp_out + temp0;

			if (interp_out > 32767) interp_out = 32767;
			else if (interp_out < -32767) interp_out = -32767;

			FilterMediaFarrowValue(interp_out);
		}
	}
}

#endif


#if 0
#define FILTER_STATE_LEN 8

const int32_t acc_uk = 1<<30;
int32_t delta_uk = 0;
const int16_t CoefB0[8] = {0,0,0,0,8192,0,0,0};
const int16_t CoefB1[8] = {-158,868,-2816,8087,-1314,-5798,1334,-212};
const int16_t CoefB2[8] = {-43,365,-1636,7935,-13014,7310,-1033,117};
const int16_t CoefB3[8] = {349,-2000,5922,-8679,5287,-42,-1068,243};
const int16_t CoefB4[8] = {-148,767,-1470,848,848,-1470,767,-148};

short* gpFilterMediaFarrowOut;
uint16_t gFilterMediaFarrowOutSize;
uint16_t gFilterMediaFarrowOutIndex;

int16_t src_state[8] = {0};
int32_t uk_tmp = 0;

void FilterMediaFarrowInit(uint8_t dac_clk, short * dataStartAddr, uint16_t size)
{
	uint8_t i;
	gpFilterMediaFarrowOut = dataStartAddr;
	gFilterMediaFarrowOutSize = size;
	gFilterMediaFarrowOutIndex = 0;

	for(i=0; i < FILTER_STATE_LEN_48K; i++)
	{
		src_state_48K[i] = 0;
	}
	for(int i =0; i < 2; i ++) 
	{
		for(int j =0; j < 3; j ++) 
		{
			xEQ[i][j] = 0;
			yEQ[i][j] = 0;
		}
	}

	for(i=0; i < FILTER_STATE_LEN; i++)
	{
		src_state[i] = 0;
	}
	uk_tmp = 0;
	
	// Our 44.1 dac real is 768/136/128(Digital)=44.117647K
	// filter uk unit is 1024*1024*1024, so delta_uk is (44.117647/44.1 - 1)*1024*1024*1024
	if(dac_clk == 44)
	{
		//uk_base = -400160;
		uk_base = 0;// Use 0 will always add packet, if user can't here it, just use it, it will get high tone quality. 
	}
	else
	{
		uk_base = 0;
	}
	delta_uk = uk_base;
	//delta_uk = 0;
	gMediaSampleDiff = 0;
}
#if 0 // 000
void FilterMediaFarrowWork_44K(int16_t din_point)
{
	int32_t interp_out;
	int32_t uk = 0;
	int32_t temp0, temp1, temp2;
	int32_t uk_alter_tmp;

	if (delta_uk <= 0) {
		uk_tmp = uk_tmp + acc_uk+ delta_uk;
		if (uk_tmp >= 1 << 30)
			uk_tmp = uk_tmp - (1 << 30);

		src_state[0] = din_point;

		temp0 = src_state[2] * 2;
		temp1 = -src_state[0] + src_state[1] * 3 - src_state[2] - src_state[3];
		temp2 = src_state[0] - src_state[1] - src_state[2] + src_state[3];

		for (int i = 2;i > -1;i--)
			src_state[i + 1] = src_state[i];

		temp0 = (temp0 + (1 << 3)) >> 4;
		temp1 = (temp1 + (1 << 3)) >> 4;
		temp2 = (temp2 + (1 << 3)) >> 4;

		uk = uk_tmp >> 22;

		interp_out = temp2 * uk;
		interp_out = (interp_out + (1 << 7)) >> 8;

		interp_out = (interp_out + temp1)*uk;
		interp_out = (interp_out + (1 << 7)) >> 8;

		interp_out = interp_out + temp0;

		if (interp_out > 8191)
			interp_out = 8191;
		else if (interp_out < -8191)
			interp_out = -8191;

		gpFilterMediaFarrowOut[gFilterMediaFarrowOutIndex++] = interp_out << 2 ;
		if (gFilterMediaFarrowOutIndex >= gFilterMediaFarrowOutSize)
		{
			gFilterMediaFarrowOutIndex=0;
		}

		uk_alter_tmp = uk_tmp + acc_uk + delta_uk;

		if (uk_alter_tmp < 1 << 30) {

			uk = uk_alter_tmp >> 22;

			interp_out = temp2 * uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = (interp_out + temp1)*uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = interp_out + temp0;

			if (interp_out > 8191) interp_out = 8191;
			else if (interp_out < -8191) interp_out = -8191;

			gpFilterMediaFarrowOut[gFilterMediaFarrowOutIndex++] = interp_out << 2 ;
			if (gFilterMediaFarrowOutIndex >= gFilterMediaFarrowOutSize)
			{
				gFilterMediaFarrowOutIndex=0;
			}

			uk_tmp = uk_alter_tmp;
		}
	}
	else {
	
		if ((uk_tmp + delta_uk) >= 1 << 30) {
			uk_tmp = uk_tmp - (1 << 30);
			src_state[0] = din_point;
			for (int i = 2;i > -1;i--)
				src_state[i + 1] = src_state[i];
		}
		else {
			uk_tmp = uk_tmp + delta_uk;
			
			src_state[0] = din_point;

			temp0 = src_state[2] * 2;
			temp1 = -src_state[0] + src_state[1] * 3 - src_state[2] - src_state[3];
			temp2 = src_state[0] - src_state[1] - src_state[2] + src_state[3];

			for (int i = 2;i > -1;i--)
				src_state[i + 1] = src_state[i];

			temp0 = (temp0 + (1 << 3)) >> 4;
			temp1 = (temp1 + (1 << 3)) >> 4;
			temp2 = (temp2 + (1 << 3)) >> 4;

			uk = uk_tmp >> 22;

			interp_out = temp2 * uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = (interp_out + temp1)*uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = interp_out + temp0;

			if (interp_out > 8191)
				interp_out = 8191;
			else if (interp_out < -8191)
				interp_out = -8191;

			gpFilterMediaFarrowOut[gFilterMediaFarrowOutIndex++] = interp_out << 2 ;
			if (gFilterMediaFarrowOutIndex >= gFilterMediaFarrowOutSize)
			{
				gFilterMediaFarrowOutIndex=0;
			}
		}
	}
}
#endif
#if 1  //2222
void FilterMediaFarrowWork_44K(int16_t din_point)
{
	int32_t interp_out;
	int32_t uk = 0;
	int32_t temp0=0, temp1=0, temp2=0, temp3=0, temp4=0;
	int32_t uk_alter_tmp;
	int ii, jj;

	if (delta_uk <= 0) {
		uk_tmp = uk_tmp + acc_uk + delta_uk;
		if (uk_tmp >= 1 << 30)
			uk_tmp = uk_tmp - (1 << 30);

		src_state[0] = din_point;

		temp0 = src_state[4];
		for (ii = 0; ii < 8; ii++)
		{
			temp1 += CoefB1[ii] * src_state[ii];
			temp2 += CoefB2[ii] * src_state[ii];
			temp3 += CoefB3[ii] * src_state[ii];
			temp4 += CoefB4[ii] * src_state[ii];
		}

		for (jj = 6; jj > -1; jj--)
			src_state[jj + 1] = src_state[jj];

    temp1 = temp1>>13;
    temp2 = temp2>>13;
    temp3 = temp3>>13;
    temp4 = temp4>>13;

		uk = uk_tmp >> 22;

    interp_out = temp4*uk;
    interp_out = interp_out>>8;

    interp_out = (interp_out+temp3)*uk;
    interp_out = interp_out>>8;

    interp_out = (interp_out+temp2)*uk;
    interp_out = interp_out>>8;

    interp_out = (interp_out+temp1)*uk;
    interp_out = interp_out>>8;

		interp_out = interp_out + temp0;

		if (interp_out > 32767) interp_out = 32767;
		else if (interp_out < -32767) interp_out = -32767;

		FilterMediaFarrowValue(interp_out);

		uk_alter_tmp = uk_tmp + acc_uk + delta_uk;

		if (uk_alter_tmp < 1 << 30) {

			uk = uk_alter_tmp >> 22;

        interp_out = temp4*uk;
        interp_out = interp_out>>8;
    
        interp_out = (interp_out+temp3)*uk;
        interp_out = interp_out>>8;
    
        interp_out = (interp_out+temp2)*uk;
        interp_out = interp_out>>8;
    
        interp_out = (interp_out+temp1)*uk;
        interp_out = interp_out>>8;
    
        interp_out = interp_out+temp0;
    
        if(interp_out > 32767) interp_out = 32767;
        else if(interp_out < -32767) interp_out = -32767;

		FilterMediaFarrowValue(interp_out);

			uk_tmp = uk_alter_tmp;
		}
	}
	else {
	
		if ((uk_tmp + delta_uk) >= 1 << 30) {
			uk_tmp = uk_tmp - (1 << 30);
			src_state[0] = din_point;
			for (jj = 6; jj > -1; jj--)
				src_state[jj + 1] = src_state[jj];
		}
		else {
			uk_tmp = uk_tmp + delta_uk;
			
			src_state[0] = din_point;

			temp0 = src_state[4];
			for (ii = 0; ii < 8; ii++)
			{
				temp1 += CoefB1[ii] * src_state[ii];
				temp2 += CoefB2[ii] * src_state[ii];
				temp3 += CoefB3[ii] * src_state[ii];
				temp4 += CoefB4[ii] * src_state[ii];
			}

			for (jj = 6; jj > -1; jj--)
				src_state[jj + 1] = src_state[jj];

         temp1 = temp1>>13;
         temp2 = temp2>>13;
         temp3 = temp3>>13;
         temp4 = temp4>>13;

		 uk = uk_tmp >> 22;

        interp_out = temp4*uk;
        interp_out = interp_out>>8;
    
        interp_out = (interp_out+temp3)*uk;
        interp_out = interp_out>>8;
    
        interp_out = (interp_out+temp2)*uk;
        interp_out = interp_out>>8;
    
        interp_out = (interp_out+temp1)*uk;
        interp_out = interp_out>>8;
    
        interp_out = interp_out+temp0;

			if (interp_out > 32767) interp_out = 32767;
			else if (interp_out < -32767) interp_out = -32767;


		FilterMediaFarrowValue(interp_out);
		}
	}
}
#endif

#endif



void FilterMediaFarrowWork_48K(int16_t din_point)
{
	int32_t interp_out;
	int32_t uk = 0;
	int32_t temp0, temp1, temp2;
	int32_t uk_alter_tmp;

	if (delta_uk <= 0) {
		uk_tmp = uk_tmp + acc_uk_48K+ delta_uk;
		if (uk_tmp >= 1 << 30)
			uk_tmp = uk_tmp - (1 << 30);

		src_state_48K[0] = din_point;

		temp0 = src_state_48K[2] * 2;
		temp1 = -src_state_48K[0] + src_state_48K[1] * 3 - src_state_48K[2] - src_state_48K[3];
		temp2 = src_state_48K[0] - src_state_48K[1] - src_state_48K[2] + src_state_48K[3];

		for (int i = 2;i > -1;i--)
			src_state_48K[i + 1] = src_state_48K[i];

		temp0 = (temp0 + (1 << 3)) >> 4;
		temp1 = (temp1 + (1 << 3)) >> 4;
		temp2 = (temp2 + (1 << 3)) >> 4;

		uk = uk_tmp >> 22;

		interp_out = temp2 * uk;
		interp_out = (interp_out + (1 << 7)) >> 8;

		interp_out = (interp_out + temp1)*uk;
		interp_out = (interp_out + (1 << 7)) >> 8;

		interp_out = interp_out + temp0;

		if (interp_out > 8191)
			interp_out = 8191;
		else if (interp_out < -8191)
			interp_out = -8191;

		FilterMediaFarrowValue(interp_out << 2);

		uk_alter_tmp = uk_tmp + acc_uk_48K + delta_uk;

		if (uk_alter_tmp < 1 << 30) {

			uk = uk_alter_tmp >> 22;

			interp_out = temp2 * uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = (interp_out + temp1)*uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = interp_out + temp0;

			if (interp_out > 8191) interp_out = 8191;
			else if (interp_out < -8191) interp_out = -8191;

			FilterMediaFarrowValue(interp_out << 2);

			uk_tmp = uk_alter_tmp;
		}
	}
	else {
	
		if ((uk_tmp + delta_uk) >= 1 << 30) {
			uk_tmp = uk_tmp - (1 << 30);
			src_state_48K[0] = din_point;
			for (int i = 2;i > -1;i--)
				src_state_48K[i + 1] = src_state_48K[i];
		}
		else {
			uk_tmp = uk_tmp + delta_uk;
			
			src_state_48K[0] = din_point;

			temp0 = src_state_48K[2] * 2;
			temp1 = -src_state_48K[0] + src_state_48K[1] * 3 - src_state_48K[2] - src_state_48K[3];
			temp2 = src_state_48K[0] - src_state_48K[1] - src_state_48K[2] + src_state_48K[3];

			for (int i = 2;i > -1;i--)
				src_state_48K[i + 1] = src_state_48K[i];

			temp0 = (temp0 + (1 << 3)) >> 4;
			temp1 = (temp1 + (1 << 3)) >> 4;
			temp2 = (temp2 + (1 << 3)) >> 4;

			uk = uk_tmp >> 22;

			interp_out = temp2 * uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = (interp_out + temp1)*uk;
			interp_out = (interp_out + (1 << 7)) >> 8;

			interp_out = interp_out + temp0;

			if (interp_out > 8191)
				interp_out = 8191;
			else if (interp_out < -8191)
				interp_out = -8191;

			FilterMediaFarrowValue(interp_out << 2);
		}
	}
}







void FilterMediaFarrowWork(uint8_t dac_clk, int16_t din_point)
{
	if(dac_clk == 44)
	{
		FilterMediaFarrowWork_44K(din_point);
	}
	else
	{
		FilterMediaFarrowWork_48K(din_point);
	}
}

uint16_t FilterMediaFarrowGetWptr(void)
{
	return gFilterMediaFarrowOutIndex;
}

void FilterMediaFarrowValue(int32_t interp_out)
{
	gpFilterMediaFarrowOut[gFilterMediaFarrowOutIndex++] = interp_out ;
	if (gFilterMediaFarrowOutIndex >= gFilterMediaFarrowOutSize)
	{
		gFilterMediaFarrowOutIndex=0;
	}
	
         if(gMediaSampleDiff != 0)
         {
		if(gMediaSampleDiff > 0)
		{
			gMediaSampleDiff --;
			if(gFilterMediaFarrowOutIndex != 0)
			{
				gFilterMediaFarrowOutIndex --;
			}
			else
			{
				gFilterMediaFarrowOutIndex = gFilterMediaFarrowOutSize-1;
			}
		}
		else
		{
			gMediaSampleDiff ++;
			gpFilterMediaFarrowOut[gFilterMediaFarrowOutIndex++] = interp_out ;
			if (gFilterMediaFarrowOutIndex >= gFilterMediaFarrowOutSize)
			{
				gFilterMediaFarrowOutIndex=0;
			}
		}
         }
}
void FilterMediaFarrowUpdateSampleDiff(int32_t sampleDiff)
{
	gMediaSampleDiff = sampleDiff;
}
void FilterMediaFarrowUpdateDeltaUk(int32_t uk)
{
	delta_uk = uk_base + uk;
}

