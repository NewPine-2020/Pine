#include "yc_8kto16k.h"


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

short* gpFilter8kTo16KOut;
uint16_t gFilter8kTo16KOutSize;
uint16_t gFilter8kTo16KOutIndex;

ResStruct gRes8kTo16K;

short gFilter8kTo16KData[FILTER_LEN + BIG_FRAME_LEN]={0};

void Filter8KTo16KInit(short * dataStartAddr, uint16_t size)
{
	uint8_t i;
	gpFilter8kTo16KOut = dataStartAddr;
	gFilter8kTo16KOutSize = size;
	gFilter8kTo16KOutIndex = 0;

	for(i = 0; i < FILTER_LEN + BIG_FRAME_LEN; i ++)
	{
		gFilter8kTo16KData[i] = 0;
	}

	
	ResStruct* pResHand = (ResStruct*)&gRes8kTo16K;
	pResHand->SmallFramLen = SMALL_FRAME_LEN;
	pResHand->BigFramLen = BIG_FRAME_LEN;
	pResHand->ps16ResBuf = gFilter8kTo16KData;
}
void Filter8KTo16KWork(short data)
{
	int i, j, Temp;
	short* ps16Buf = 0;
	ResStruct* pResHand = (ResStruct*)&gRes8kTo16K;

	ps16Buf = pResHand->ps16ResBuf;

	for (i = 0; i < pResHand->BigFramLen; i++)
	{
		ps16Buf[FILTER_LEN + i] = 0;
	}

	//
	for (i = 0; i < pResHand->SmallFramLen; i++)
	{
		ps16Buf[FILTER_LEN + i * 2] = data;
	}

	for (i = 0; i < pResHand->BigFramLen; i++)
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
		
		gpFilter8kTo16KOut[gFilter8kTo16KOutIndex++] = (short)Temp ;
		if(gFilter8kTo16KOutIndex >= gFilter8kTo16KOutSize)
		{
			gFilter8kTo16KOutIndex = 0;
		}
	}

	//update samples
	Temp = pResHand->BigFramLen;
	for (i = 0; i < FILTER_LEN; i++)
	{
		ps16Buf[i] = ps16Buf[Temp + i];
	}

	//return;
}

uint16_t Filter8KTo16KGetWptr(void)
{
	return gFilter8kTo16KOutIndex;
}

