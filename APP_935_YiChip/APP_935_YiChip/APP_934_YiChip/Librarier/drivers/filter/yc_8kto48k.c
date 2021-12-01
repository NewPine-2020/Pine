#include "yc_8kto48k.h"


#define FILTER_ROW 6
#define FILTER_COL 6

short* gpFilter8kTo48KOut;
uint16_t gFilter8kTo48KOutSize;
uint16_t gFilter8kTo48KOutIndex;
uint8_t gFilter8kTo48KDataIndex;;
short gFilter8kTo48KData[FILTER_COL]={0};

int32_t g48KSampleDiff;

const short filter8kTo48KParams[FILTER_ROW][FILTER_COL]=
{-9,-281,1420,4058,824,-236,
-26,-278,2088,3815,347,-171,
-59,-191,2761,3362,10,-108,
-108,10,3362,2761,-191,-59,
-171,347,3815,2088,-278,-26,
-236,824,4058,1420,-281,-9};

void Filter8KTo48KInit(short * dataStartAddr, uint16_t size)
{
	uint8_t i;
	gpFilter8kTo48KOut = dataStartAddr;
	gFilter8kTo48KOutSize = size;
	gFilter8kTo48KOutIndex = 0;
	gFilter8kTo48KDataIndex = 0;

	for(i = 0; i < FILTER_COL; i ++)
	{
		gFilter8kTo48KData[i] = 0;
	}

	g48KSampleDiff = 0;
}

void Filter8KTo48KAddValue(int16_t interp_out)
{
	gpFilter8kTo48KOut[gFilter8kTo48KOutIndex++] = interp_out;
	if(gFilter8kTo48KOutIndex >= gFilter8kTo48KOutSize)
	{
		gFilter8kTo48KOutIndex = 0;
	}
	
         if(g48KSampleDiff != 0)
         {
		if(g48KSampleDiff > 0)
		{
			g48KSampleDiff --;
			if(gFilter8kTo48KOutIndex != 0)
			{
				gFilter8kTo48KOutIndex --;
			}
			else
			{
				gFilter8kTo48KOutIndex = gFilter8kTo48KOutSize-1;
			}
		}
		else
		{
			g48KSampleDiff ++;
		         gpFilter8kTo48KOut[gFilter8kTo48KOutIndex++] = interp_out;
			if(gFilter8kTo48KOutIndex >= gFilter8kTo48KOutSize)
			{
				gFilter8kTo48KOutIndex = 0;
			}
		}
         }
}
void Filter8KTo48KWork(short data)
{
	int sum ;
	//uint8_t ind;

	gFilter8kTo48KData[gFilter8kTo48KDataIndex] = data;
	for(int row =0; row < FILTER_ROW; row++)
	{
		sum = 0;
		for (int col=0,ind=gFilter8kTo48KDataIndex; col < FILTER_COL; col++)
		{
			sum += filter8kTo48KParams[row][col]*gFilter8kTo48KData[ind];
			if(ind == 0)
			{
				ind = FILTER_COL-1;
			}
			else
			{
				ind--;
			}
		}

		//Filter8KTo48KAddValue(((sum>>14)*3));
		//Filter8KTo48KAddValue(((sum>>15)*5));//6/4   5/4  vol too low
		Filter8KTo48KAddValue((((sum>>13)*11)>>3));//12/8    11/8   mips need more.
		//Filter8KTo48KAddValue(((sum>>14)*3));  //3/2  will zizi error
		/*
		gpFilter8kTo48KOut[gFilter8kTo48KOutIndex++] = ((sum>>14)*3);
		if(gFilter8kTo48KOutIndex >= gFilter8kTo48KOutSize)
		{
			gFilter8kTo48KOutIndex = 0;
		}
		*/
	}
	
	if ((++gFilter8kTo48KDataIndex) == FILTER_COL)
	{
		gFilter8kTo48KDataIndex = 0;
	}
}

uint16_t Filter8KTo48KGetWptr()
{
	return gFilter8kTo48KOutIndex;
}


void Filter8KTo48KUpdateSampleDiff(int32_t sampleDiff)
{
	g48KSampleDiff = sampleDiff;
}

