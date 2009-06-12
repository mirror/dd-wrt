/****************************************************************************
 *
 *  Name:		cnxtflash.h
 *
 *  Description:	Flash programmer header file
 *
 *  Copyright (c) 1999-2002 Conexant Systems, Inc.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *
 ***************************************************************************/
#ifndef _FLASH_H_
#define _FLASH_H_

#define CNXT_FLASH_MAX_NAME_LEN	20

typedef enum CNXT_FLASH_SEGMENTS 
{
	CNXT_FLASH_SEGMENT_1 = 0,
	CNXT_FLASH_SEGMENT_2 = 1,
	CNXT_FLASH_SEGMENT_3 = 2,
	CNXT_FLASH_SEGMENT_4 = 3,
	CNXT_FLASH_SEGMENT_5 = 4,
	CNXT_FLASH_SEGMENT_6 = 5,
	CNXT_FLASH_SEGMENT_7 = 6,
	CNXT_FLASH_SEGMENT_8 = 7
} CNXT_FLASH_SEGMENT_E;

typedef enum CNXT_FLASH_OWNERS
{
	CNXT_FLASH_OWNER_1 = 0,
	CNXT_FLASH_OWNER_2 = 1,
	CNXT_FLASH_OWNER_3 = 2,
	CNXT_FLASH_OWNER_4 = 3,
	CNXT_FLASH_OWNER_5 = 4,
	CNXT_FLASH_OWNER_6 = 5,
	CNXT_FLASH_OWNER_7 = 6,
	CNXT_FLASH_OWNER_8 = 7,
	CNXT_FLASH_UNUSED = 8
} CNXT_FLASH_OWNER_E;

typedef enum CNXT_FLASH_STATUS
{
	CNXT_FLASH_SUCCESS,
	CNXT_FLASH_DATA_ERROR,
	CNXT_FLASH_OWNER_ERROR
} CNXT_FLASH_STATUS_E;

typedef struct CNXT_FLASH_SEGMENT
{
	CNXT_FLASH_SEGMENT_E 	segmentNumber;		
	DWORD			segmentSize;
	DWORD			startFlashAddress;
	DWORD			startDataAddress;
	CNXT_FLASH_OWNER_E	owner;	
	DWORD			version;
	DWORD			reserved1;
	DWORD			reserved2;
	DWORD			dataLength;
	DWORD 			headerCRC;
	DWORD			dataCRC;
} CNXT_FLASH_SEGMENT_T, *PCNXT_FLASH_SEGMENT_T;

CNXT_FLASH_STATUS_E CnxtFlashOpen( CNXT_FLASH_SEGMENT_E segment );

BOOL CnxtFlashReadRequest( 
	CNXT_FLASH_SEGMENT_E segment, 
	UINT16 * psdramStartAddr, 
	UINT32 size 
);

BOOL CnxtFlashWriteRequest( 
	CNXT_FLASH_SEGMENT_E segment, 
	UINT16 * psdramStartAddr, 
	UINT32 size 
);


#define CNXT_BSP_FLASH_SEGMENT	CNXT_FLASH_SEGMENT_1

#if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE
#define CNXT_ADSL_FLASH_SEGMENT	CNXT_FLASH_SEGMENT_2
#endif

#if CONFIG_CNXT_VOIP || CONFIG_CNXT_VOIP_MODULE
#define CNXT_VOIP_FLASH_SEGMENT	CNXT_FLASH_SEGMENT_3
#endif

#endif 





