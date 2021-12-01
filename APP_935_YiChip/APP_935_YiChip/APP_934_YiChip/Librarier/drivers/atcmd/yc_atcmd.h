#ifndef YC_ATCMD_H
#define YC_ATCMD_H


#include "type.h"

//#define ATNotifyHandle 0x03

typedef enum
{
	AT_TYPE_BT_RAM=0,
	AT_TYPE_M0_RAM,
	AT_TYPE_FlASH,
}ATCMDTYPE;
typedef struct
{
	uint8_t handle[2];
	char head[6];
}AtSendOKDev;
typedef struct
{
	uint8_t handle[2];
	char head[9];
}AtSendERRORDev;
typedef struct
{
	uint8_t handle[2];
	char head[8];
	uint8_t data;
}AtSendReadDateReplyDev;

void AT_Send_Ok(void);
void AT_Send_Error(void);
void AT_Send_Read_Date(uint8_t *data,uint8_t len);
void Bt_BleAtCmdCallBack(uint8_t len,uint8_t *dataPtr);


#endif
