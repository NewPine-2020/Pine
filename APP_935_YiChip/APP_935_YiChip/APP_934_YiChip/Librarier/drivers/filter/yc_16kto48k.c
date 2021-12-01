#include "yc_16kto48k.h"


#define FILTER_ROW 3
#define FILTER_COL 12

short* gpFilter16kTo48KOut;
uint16_t gFilter16kTo48KOutSize;
uint16_t gFilter16kTo48KOutIndex;
uint8_t gFilter16kTo48KDataIndex;;
short gFilter16kTo48KData[FILTER_COL]={0};

const short filter16kTo48KParams[FILTER_ROW][FILTER_COL]=
{ 40,123,-317,650,-1160,2263,6945,-170,-297,346,-259,164,
120,-54,-86,456,-1331,5069,5069,-1331,456,-86,-54,120,
164,-259,346,-297,-170,6945,2263,-1160,650,-317,123,40 };

void Filter16KTo48KInit(short * dataStartAddr, uint16_t size)
{
	uint8_t i;
	gpFilter16kTo48KOut = dataStartAddr;
	gFilter16kTo48KOutSize = size;
	gFilter16kTo48KOutIndex = 0;
	gFilter16kTo48KDataIndex = 0;

	for(i = 0; i < FILTER_COL; i ++)
	{
		gFilter16kTo48KData[i] = 0;
	}
}
void Filter16KTo48KWork(short data)
{
	int sum ;
	//uint8_t ind;

	gFilter16kTo48KData[gFilter16kTo48KDataIndex] = data;
	for(int row =0; row < FILTER_ROW; row++)
	{
		sum = 0;
		for (int col=0,ind=gFilter16kTo48KDataIndex; col < FILTER_COL; col++)
		{
			sum += filter16kTo48KParams[row][col]*gFilter16kTo48KData[ind];
			if(ind == 0)
			{
				ind = FILTER_COL-1;
			}
			else
			{
				ind--;
			}
		}

		gpFilter16kTo48KOut[gFilter16kTo48KOutIndex++] = (short)((sum>>13));
		if(gFilter16kTo48KOutIndex >= gFilter16kTo48KOutSize)
		{
			gFilter16kTo48KOutIndex = 0;
		}
	}
	
	if ((++gFilter16kTo48KDataIndex) == FILTER_COL)
	{
		gFilter16kTo48KDataIndex = 0;
	}
}

uint16_t Filter16KTo48KGetWptr()
{
	return gFilter16kTo48KOutIndex;
}

