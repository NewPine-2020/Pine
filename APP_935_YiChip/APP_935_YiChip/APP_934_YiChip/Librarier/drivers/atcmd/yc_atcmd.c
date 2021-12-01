#include "yc_atcmd.h"
#include <string.h>
#include "ipc.h"
#include "yc11xx.h"
#include "yc_debug.h"
#include "yc11xx_qspi.h"

#if 0
uint8_t readdata[20]={0};
uint8_t datanum;
void Bt_BleAtCmdCallBack(uint8_t len,uint8_t *dataPtr)
{
	//uint8_t readdata[10]={0};
	uint16_t addr;

//	if(*(dataPtr+2)=='A'&*(dataPtr+3)=='T'&*(dataPtr+5)=='Y'&*(dataPtr+6)=='C')
	{
		addr=*(dataPtr+9)<<16|*(dataPtr+10)<<8|*(dataPtr+11);
		if(*(dataPtr+7)=='W')
		{
			DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"ATCMD: 0x%04X", LOG_POINT_C300, 0x0000);
			switch (*(dataPtr+8))
			{
				case AT_TYPE_BT_RAM:
					for(datanum=0;datanum<*(dataPtr+12);datanum++)
					{
						HWRITE(addr +datanum , *(dataPtr+13+datanum));
					}
					break;
				case AT_TYPE_M0_RAM:
					for(datanum=0;datanum<*(dataPtr+12);datanum++)
					{
						HWRITE(reg_map_m0(addr +datanum) , *(dataPtr+13+datanum));
					}
					break;
				case AT_TYPE_FlASH:
					{
						QSPI_WriteFlashData(addr,*(dataPtr+12),dataPtr+13);
					}
					break;
			}
			AT_Send_Ok();
		}
		else if(*(dataPtr+7)=='R')
		{
			DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"ATCMD: 0x%04X", LOG_POINT_C300, 0x0001);
			switch (*(dataPtr+8))
			{
				case AT_TYPE_BT_RAM:
					for(datanum=0;datanum<*(dataPtr+12);datanum++)
					{
						readdata[datanum]=HREAD(addr+datanum);
					}
					break;
				case AT_TYPE_M0_RAM:
					for(datanum=0;datanum<*(dataPtr+12);datanum++)
					{
						readdata[datanum]=HREAD3(addr+datanum);
					}
					break;
				case AT_TYPE_FlASH:
					QSPI_ReadFlashData(addr,*(dataPtr+12),readdata);
					datanum=*(dataPtr+12);
					break;
			}
			AT_Send_Ok();
			AT_Send_Read_Date(readdata,datanum);
		}
		else
		{
			AT_Send_Error();	
		}
	}
	//else
	//{
	//	AT_Send_Error();
	//}
}
void AT_Send_Ok(void)
{
	char send[8]={0};
	AtSendOKDev *reply;
	reply=(AtSendOKDev *)send;
	reply->handle[0]=HR_REG_8BIT(reg_map(mem_ota_notify_handle)) ;
	reply->handle[1]=0;
	strcpy(reply->head,"AT+OK\n");
	IPC_TxBleData((uint8_t*)send,8);
}
void AT_Send_Error(void)
{
	char send[11]={0};
	AtSendERRORDev *reply;
	reply=(AtSendERRORDev *)send;
	reply->handle[0]=HR_REG_8BIT(reg_map(mem_ota_notify_handle)) ;
	reply->handle[1]=0;
	strcpy(reply->head,"AT+ERROR\n");
	IPC_TxBleData((uint8_t*)send,11);
}
void AT_Send_Read_Date(uint8_t *data,uint8_t len)
{	
	uint8_t senddata[50]={0};
	uint8_t i=0;
	AtSendReadDateReplyDev *reply;

	reply=(AtSendReadDateReplyDev *)senddata;
	reply->handle[0]=HR_REG_8BIT(reg_map(mem_ota_notify_handle)) ;
	reply->handle[1]=0;
	strcpy(reply->head,"AT+DATE ");
	for(i=0;i<len;i++)
	{
		*(&(reply->data)+i)=*(data+i);
		DEBUG_LOG(LOG_LEVEL_CORE, "IPC" ,"ATCMD: 0x%04X", LOG_POINT_C301, *(&(reply->data)+i));
	}
	IPC_TxBleData(senddata,10+len);
}
#endif

