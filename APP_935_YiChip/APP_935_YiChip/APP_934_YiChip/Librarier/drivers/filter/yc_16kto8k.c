#include "yc_8kto48k.h"


#define FILTER_LEN	31

#define SMALL_FRAME_LEN	1
#define BIG_FRAME_LEN	2
typedef struct tResStruct
{
	int SmallFramLen;                //升采样时输入帧长
	int BigFramLen;                  //升采样时输出帧长
	short* ps16ResBuf;              //滤波器缓冲buf指针
}ResStruct;

static short gas16Res8kTo16k[FILTER_LEN] =
{
 -56,	0,	96,	0, - 220,	0,	461,	0, - 876,	0,	1606,	
 0, - 3171,	0,	10326,	16384,	10326,	0, - 3171,	
	0,	1606,	0, - 876,	0,	461,	0, - 220,	0,	96,	0, - 56
};

short* gpFilter16kTo8KOut;
uint16_t gFilter16kTo8KOutSize;
uint16_t gFilter16kTo8KOutIndex;

ResStruct gRes16kTo8K;

short gFilter16kTo8KData[FILTER_LEN + BIG_FRAME_LEN]={0};

void Filter16KTo8KInit(short * dataStartAddr, uint16_t size)
{
	uint8_t i;
	gpFilter16kTo8KOut = dataStartAddr;
	gFilter16kTo8KOutSize = size;
	gFilter16kTo8KOutIndex = 0;

	for(i = 0; i < FILTER_LEN + BIG_FRAME_LEN; i ++)
	{
		gFilter16kTo8KData[i] = 0;
	}

	
	ResStruct* pResHand = (ResStruct*)&gRes16kTo8K;
	pResHand->SmallFramLen = SMALL_FRAME_LEN;
	pResHand->BigFramLen = BIG_FRAME_LEN;
	pResHand->ps16ResBuf = gFilter16kTo8KData;
}

void Filter16KTo8KWork(short ps16InData0, short ps16InData1)
{
	int i, j, Temp;
	short* ps16Buf = 0;
	ResStruct* pResHand = (ResStruct*)&gRes16kTo8K;
	short ps16InData[2];

	ps16InData[0] = ps16InData0;
	ps16InData[1] = ps16InData1;
	
	ps16Buf = pResHand->ps16ResBuf;

	for (i = 0; i < 2; i++)
	{
		ps16Buf[FILTER_LEN + i ] = ps16InData[i];
	}
	
	for (i = 0; i < 2; i++)
	{
		Temp = 0;
		//单个输出的计算
		for (j = 0; j < FILTER_LEN; j++)
		{
			Temp += ps16Buf[i + j + 1] * gas16Res8kTo16k[j];

		}

		Temp = (Temp >> 15);

		if (Temp > 32767)
		{
			Temp = 32767;
		}

		if (Temp < -32768)
		{
			Temp = -32768;
		}
		ps16InData[i] = (short)Temp;

	}


	gpFilter16kTo8KOut[gFilter16kTo8KOutIndex++] = ps16InData[i];
	if(gFilter16kTo8KOutIndex >= gFilter16kTo8KOutSize)
	{
		gFilter16kTo8KOutIndex = 0;
	}

	//update samples
	Temp = pResHand->BigFramLen;
	for (i = 0; i < FILTER_LEN; i++)
	{
		ps16Buf[i] = ps16Buf[Temp + i];
	}

	//return;
}

uint16_t Filter16KTo8KGetWptr(void)
{
	return gFilter16kTo8KOutIndex;
}

