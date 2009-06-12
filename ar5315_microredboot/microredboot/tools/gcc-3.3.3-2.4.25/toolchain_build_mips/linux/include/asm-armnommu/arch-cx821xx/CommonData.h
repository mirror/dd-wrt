/****************************************************************************
**
**	FILE NAME:
**		CommonData.h
**
**	ABSTRACT:
**		This file contains type definitions and structures that are common
**		between the application and the driver
**
**
*******************************************************************************
**
**  Copyright 1997-2002 Conexant Systems, Inc.
**
*******************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Hasbani and Derivatives Linux/Reference Designs/Mackinac/Lineo 2.4.6 Beta/linux/include/asm-armnommu/arch-cx821xx/CommonData.h $
**	$Revision: 1.1 $
**	$Date: 2003/06/29 14:28:18 $
*******************************************************************************
******************************************************************************/

#ifndef _COMMON_DATA_H_
#define  _COMMON_DATA_H_


/*******************************************************************************
	AUTOSENSE
*******************************************************************************/
// these must match values in xcvrinfo.h which is a DMT core file which can't
// on the Linux shared file directory and naming structure
#define AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO	4	// maximum number of combinations of relay settings
#define AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO		4	// maximum number of GPIOs that must be asserted to select a relay combo


/*******************************************************************************
	PHYSICAL DRIVER TYPE
*******************************************************************************/
#define NO_VENDOR_ID				0xFFFF
#define NO_DEVICE_ID				0xFFFF
#define NO_PHYSICAL_DRIVER			0
#define PHYSICAL_DRIVER_TIGRIS		1
#define PHYSICAL_DRIVER_YUKON		2

#define NUM_ADSL_OPTIONS		20

typedef enum PHYSICAL_DRIVER_TYPE_E
{
	DRIVER_TYPE_NONE	= NO_PHYSICAL_DRIVER,
	DRIVER_TYPE_TIGRIS	= PHYSICAL_DRIVER_TIGRIS,
	DRIVER_TYPE_YUKON	= PHYSICAL_DRIVER_YUKON
} PHYSICAL_DRIVER_TYPE_T;





/*******************************************************************************
	IO control commands
*******************************************************************************/

#define TIGR_MAGIC '%'

#define TIG_SET_PARAMS		_IOW(TIGR_MAGIC, 1, struct atmif_sioc)
#define TIG_GET_PARAMS		_IOW(TIGR_MAGIC, 2, struct atmif_sioc)
#define TIG_OAM_INIT		_IOW(TIGR_MAGIC, 3, struct atmif_sioc)
#define TIG_OAM_RESP		_IOW(TIGR_MAGIC, 4, struct atmif_sioc)
#define TIG_AAL5_VC_STATS	_IOW(TIGR_MAGIC, 5, struct atmif_sioc)
#define TIG_ATM_VC_STATS	_IOW(TIGR_MAGIC, 6, struct atmif_sioc)

#define NUM_LOG_PARMS 6
#define NUM_LOG_ENTRIES (2*1000)


/*******************************************************************************
	TIG_GET_PRINT_BUFF
*******************************************************************************/

typedef struct
{
	char		*pFormat ;	// NULL terminates list
	DWORD		Data[NUM_LOG_PARMS] ;
} LOG_ENTRY_T ;

typedef struct
{
	BOOLEAN		 Cumulative ;			// Cumulative vs Delta (only those lines since last read)
	char		*pString_Buffer ;		// holds multiple strings
	DWORD		 String_Buffer_Size ;	// size in bytes

	LOG_ENTRY_T *pBuff ;
	DWORD		 Buff_Size ;
} TIG_PRINT_DESC ;



/*******************************************************************************
	TIG_GET_DEBUG_DATA
*******************************************************************************/
typedef struct
{
	DWORD		 Data[16] ;
} TIG_DEBUG_DATA_DESC ;



/*******************************************************************************
	TIG_LOAD_RECORD
*******************************************************************************/
// Intel 32 hex data record definitions
#define DATA_REC			0
#define EXT_SEG_ADDR_REC	2
#define EXT_LIN_ADDR_REC	4
#define EXT_END_OF_FILE		1
#define EXT_GOTO_CMD_REC	3


// record format for transfer to the driver
typedef struct TIG_LOAD_RECORD_S
{
	DWORD		CommandStatus;
	USHORT		RecordType; // type field from the record
	DWORD		RecordAddress;  // store address for the record
	DWORD		RecordLength;  // byte length not including the header - 4096 max
	char		RecordData [1]; // first byte of the record data
} TIG_LOAD_RECORD_T;



/*******************************************************************************
	TIG_DEVICE_SPEC
*******************************************************************************/
// wrapper for device specific IOCTL
typedef struct TIG_DEVICE_SPEC_S
{
	DWORD		CommandStatus;
	ULONG		RequiredSize;	// number of bytes required for the response.
	ULONG		ReturnSize;		// number of bytes returned
	char		BackDoorBuf [1];	// command specific data area
} TIG_DEVICE_SPEC_T;



/*******************************************************************************
	TIG_SET_PARAMS
*******************************************************************************/
//
// User configurable parameters structure
//
typedef struct _TIG_USER_PARAMS_
{
	// Chip abstraction layer settings
	unsigned long	RxMaxLatency;
	unsigned long	RxMinLatency;
	unsigned long	TxMaxLatency;
	unsigned long	TxMinLatency;
	unsigned long	RxInterruptRate;
	unsigned long	TxInterruptRate;
	unsigned long	RxSpeed;
	unsigned long	TxSpeed;

	unsigned long	AutoSenseHandshake;
	unsigned long	AutoSenseWiresFirst;
	unsigned long	AutoWiringSelection;
	unsigned long	AutoWiringRelayDelay;
	unsigned long	AutoWiringRelayEnrg;
	unsigned long	AutoWiringNumCombos;
	unsigned char	AutoWiringComboGPIO [AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO][AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO];
	unsigned char	AutoWiringOrder[AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO];

	unsigned long	AdslHeadEnd;
	unsigned long	AdslHeadEndEnvironment;
	unsigned long	LineAutoActivation;
	unsigned long	VendorNearId;

	// Buffer management settings
	unsigned long	RXBufAllocLimit;
	unsigned long	RXBufRatioLimit;
	unsigned long	RxMaxFrameSize;
	unsigned long	TxMaxFrameSize;
	unsigned long	MaxTransmit;

	// Frame abstraction settings
	unsigned long	RfcEncapsulationMode;
	unsigned long	CellBurstSize;
	unsigned long	PeakCellRate;

	unsigned long	OverrideMacAddress;
	unsigned char	MACAddress[6];

	unsigned long	SendOAMCell;

	unsigned short	AdslOptions[NUM_ADSL_OPTIONS];


}TIG_USER_PARAMS, *PTIG_USER_PARAMS;




#endif //  _COMMONDATA_H_
