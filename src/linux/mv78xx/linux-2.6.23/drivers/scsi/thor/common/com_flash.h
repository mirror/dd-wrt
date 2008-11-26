#ifndef COM_FLASH_H
#define COM_FLASH_H

#include "com_define.h"

#define DRIVER_LENGTH					1024*16

typedef struct BUFFER_ALIGN_8 _Flash_DriverData
{
	MV_U16			Size;
	MV_U8			PageNumber;
	MV_BOOLEAN		isLastPage;
	MV_U16			Reserved[2];
	MV_U8			Data[DRIVER_LENGTH];
}
Flash_DriveData, *PFlash_DriveData;

#define MAX_FLASH_LENGTH				128*1024

typedef struct BUFFER_ALIGN_8 _Flash_Data
{
	MV_U16			Size;
	MV_U16			Reserved[3];
	MV_U8			Data[MAX_FLASH_LENGTH];
} 
Flash_Data, *PFlash_Data;


#endif