/******************************************************************************
*******************************************************************************
**
**	MODULE NAME:
**		HwIo (though global and available to all files)
**
**	FILE NAME:
**		HwIo.h
**
**	ABSTRACT:
**		This file contains defines for accessing the driver
**		through a device specific function/method (e.g., through
**		the "OID_TAPI_DEV_SPECIFIC" OID of the driver registered
**		SetInformationHandler function).
**		This is the device specific interface through which
**		applications may access certain driver specific information.
**
**	DETAILS:
**
**	NOTES:
**		This device specific hardware input/output (HwIo) interface to the
**		driver has legacy names (some are listed below).
**			Bd, BD								now HwIo, HW_IO
**			BackDoor, MiniBackDoor				now HwIo
**			pBackDoorBuf, pMiniBackDoorBuf		now pHwIo
**			BACK_DOOR_T, MINI_BACK_DOOR_T		now HW_IO_CMD_STRUC_T
**		See the text file "HwIoFromDevIo.txt" for more details.
**
**
*****************************************************************************
**
**	Copyright (c) 2000, 2001
**	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
**
*****************************************************************************

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
** $Archive: /Projects/Hasbani and Derivatives Linux/Reference Designs/Mackinac/Lineo 2.4.6 Beta/linux/include/asm-armnommu/arch-cx821xx/hwio.h $
** $Revision: 1.1 $
** $Date: 2003/06/29 14:28:18 $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.

#ifndef _HWIO_H_
#define _HWIO_H_


//* Ioctl Format:
//*
//* CTL_CODE( DeviceType, Function, Method, Access ) (
//*   ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)
//*
//* Note: Custom Functions must must have the MSB set i.e., 8xx)
//*
#define IOCTL_DEVICE_SPECIFIC		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_VERSION			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CNX_DEV_SPECIFIC		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEVICE_CONFIG			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEVICE_APP    		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CNX_NET_DEV_SPECIFIC	CTL_CODE(FILE_DEVICE_NETWORK, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CNX_NET_DATA			CTL_CODE(FILE_DEVICE_NETWORK, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CNX_NET_OPEN			CTL_CODE(FILE_DEVICE_NETWORK, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CNX_NET_CLOSE			CTL_CODE(FILE_DEVICE_NETWORK, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CNX_NET_ECHO_PARENT	CTL_CODE(FILE_DEVICE_NETWORK, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

///////////////////////////////////////////////////////////////////////////////
//
//	General Definitions (defines)
//
///////////////////////////////////////////////////////////////////////////////


//	The following define specifies that instead of a particular VC index,
//	this index value specifies to return statistics for all VCs on the link.
#define HW_IO_ATM_STATS_INDEX_FOR_LINK			0xFFFF

//	The following define specifies that instead of a particular VC index,
//	this index value specifies to return statistics for all VCs on the link.
#define HW_IO_AAL_STATS_INDEX_FOR_LINK			0xFFFF


// Mask for determining Message bits
#define DEBUG_MESSAGE_LEVEL				0x0000001D
#define DEBUG_MODULE_ID_MASK			0xffff0000
#define DBG_DISABLED					0x00000000
#define DBG_ENA_MESSAGES				0x00000001
#define DBG_ENA_BREAKPOINTS				0x00000002
#define DBG_ENA_MISC_MSG				0x00000004
#define DBG_ENA_WARNING_MSG				0x00000008
#define DBG_ENA_ERROR_MSG				0x00000010
#define DBG_ENA_MISC_BRK				0x00000020
#define DBG_ENA_WARNING_BRK				0x00000040
#define DBG_ENA_ERROR_BRK				0x00000080
#define DBG_ENA_DRVR_ENTRY_BRK			0x00000100
									//	0x00000200
									//	0x00000400
									//	0x00000800
									//	0x00001000
									//	0x00002000
									//	0x00004000
									//	0x00008000
#define DBG_ENABLE_CHIPAL				0x00010000
#define DBG_ENABLE_CARDAL				0x00020000
#define DBG_ENABLE_CARDMGMT				0x00040000
#define DBG_ENABLE_FRAMEAL				0x00080000
#define DBG_ENABLE_BUFMGMT				0x00100000
#define DBG_ENABLE_CELLDATATEST			0x00200000
#define DBG_ENABLE_CONDIS				0x00400000
									//	0x00800000
									//	0x01000000
									//	0x02000000
									//	0x04000000
									//	0x08000000
#define DBG_ENABLE_HW_IO_MSG_DUMP		0x10000000
									//	0x20000000
									//	0x40000000
									//	0x80000000
// The following are defined for use but are defined here for Appy purposes.
#define DBG_ENABLE_BIT_0			0x00000001
#define DBG_ENABLE_BIT_1			0x00000002
#define DBG_ENABLE_BIT_2			0x00000004
#define DBG_ENABLE_BIT_3			0x00000008
#define DBG_ENABLE_BIT_4			0x00000010
#define DBG_ENABLE_BIT_5			0x00000020
#define DBG_ENABLE_BIT_6			0x00000040
#define DBG_ENABLE_BIT_7			0x00000080
#define DBG_ENABLE_BIT_8			0x00000100
#define DBG_ENABLE_BIT_9			0x00000200
#define DBG_ENABLE_BIT_10			0x00000400
#define DBG_ENABLE_BIT_11			0x00000800
#define DBG_ENABLE_BIT_12			0x00001000
#define DBG_ENABLE_BIT_13			0x00002000
#define DBG_ENABLE_BIT_14			0x00004000
#define DBG_ENABLE_BIT_15			0x00008000
#define DBG_ENABLE_BIT_16			0x00010000
#define DBG_ENABLE_BIT_17			0x00020000
#define DBG_ENABLE_BIT_18			0x00040000
#define DBG_ENABLE_BIT_19			0x00080000
#define DBG_ENABLE_BIT_20			0x00100000
#define DBG_ENABLE_BIT_21			0x00200000
#define DBG_ENABLE_BIT_22			0x00400000
#define DBG_ENABLE_BIT_23			0x00800000
#define DBG_ENABLE_BIT_24			0x01000000
#define DBG_ENABLE_BIT_25			0x02000000
#define DBG_ENABLE_BIT_26			0x04000000
#define DBG_ENABLE_BIT_27			0x08000000
#define DBG_ENABLE_BIT_28			0x10000000
#define DBG_ENABLE_BIT_29			0x20000000
#define DBG_ENABLE_BIT_30			0x40000000
#define DBG_ENABLE_BIT_31			0x80000000




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	HW IO Structure Definition
//////////////////////////////////////////////

typedef struct DEVIO_HEADER_s
{
	IN	DWORD		InstanceId;
	IN	DWORD		ReqCode;
	IN	DWORD		TotalSize;
	OUT	DWORD		NeededSize;
	OUT	DWORD		ResultCode;
} DEVIO_HEADER_T,
  * PDEVIO_HEADER_T;


typedef struct HW_IO_CMD_STRUC_S
{
	IN	DWORD		InstanceId;
	IN	DWORD		ReqCode;
	IN	DWORD		TotalSize;
	OUT	DWORD		NeededSize;
	OUT	DWORD		ResultCode;
	I_O	DWORD		Params;
} HW_IO_CMD_STRUC_T, * PHW_IO_CMD_STRUC_T;

// The size of the HW_IO header is the size of the structure "HW_IO_CMD_STRUC_T"
// minus the size of the "Params" element (a DWORD).
#define HW_IO_HEADER_SIZE		(sizeof(HW_IO_CMD_STRUC_T) - sizeof(DWORD))

//	The following define gives the sizeof the TAPI Dev Specific header
//	structure, excluding the "Params" element which is overlayed by the
//	BackDoor structure/data.
#define TAPI_DEV_SPECIFIC_SIZE	(sizeof( NDIS_TAPI_DEV_SPECIFIC ) - sizeof( UCHAR * ))




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	HW IO Request Code Definitions
//////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// NOTES:
//	* !!!!!!!!!!!!!!!!!!!!!!!!   C A U T I O N !   !!!!!!!!!!!!!!!!!!!!!!!! *
//	*	Even though the request codes appear to be grouped logically in the
//	*	enumeration below DO NOT REMOVE any codes (enumeration elements)
//	*	from the enumeration list below!
//	*	Always ADD NEW ELEMENTS TO END OF LIST (before "HW_IO_LAST_CMD_CODE")!
//	*	The reason for never deleting any elements from the list and always
//	*	adding new elements to the end of the enumeration list is to maintain
//	*	backwards compatibility (i.e., allows Appy programs to communicate
//	*	with older drivers).
//	* !!!!!!!!!!!!!!!!!!!!!!!!   C A U T I O N !   !!!!!!!!!!!!!!!!!!!!!!!! *
///////////////////////////////////////////////////////////////////////////////

// these IO CMDS for IOCTL_DEVICE_SPECIFIC	
typedef enum HW_IO_CMDS_E
{
	HW_IO_FIRST_CMD_CODE			= 0,

	HW_IO_GET_BUFMGMT_CONFIG		= 0,
	HW_IO_SET_BUFMGMT_CONFIG		= 1,

	HW_IO_GET_HW_INTERFACE_STATS	= 2,
	HW_IO_CLEAR_HW_INTERFACE_STATS	= 3,

	HW_IO_GET_FRAMEAL_STATS			= 4,
	HW_IO_GET_FRAMEAL_ATM_STATS		= 5,
	HW_IO_GET_FRAMEAL_AAL_STATS		= 6,
	HW_IO_GET_FRAMEAL_CONFIG		= 7,
	HW_IO_SET_FRAMEAL_CONFIG		= 8,

	HW_IO_GET_ALARMS				= 9,
	HW_IO_GET_BIT_ALLOC_TABLE		= 10,
	HW_IO_GET_SNR_TABLE				= 11,
	HW_IO_GET_CONTROLLER_LOG_CLT	= 12,
	HW_IO_SET_CONTROLLER_LOG_CLT	= 13,
	HW_IO_GET_GHS_CAP				= 14,
	HW_IO_GET_GHS_LOCAL_CAP			= 15,
	HW_IO_SET_GHS_LOCAL_CAP			= 16,
	HW_IO_SET_AUTO_SENSE			= 17,	// HW_IO_GET_AUTO_SENSE is further down table
	HW_IO_GET_EEPROM_MAC_ADDR		= 18,
	HW_IO_SET_EEPROM_MAC_ADDR		= 19,

	HW_IO_USER_ACTIVATE_LINE		= 20,
	HW_IO_USER_DEACTIVATE_LINE		= 21,
	HW_IO_GET_CONNECT_STATUS		= 22,
	HW_IO_GET_DEBUG_FLAG			= 23,
	HW_IO_SET_DEBUG_FLAG			= 24,
	HW_IO_GET_PRODUCT_INFO			= 25,
	HW_IO_SIMULATE_INCOMING			= 26,
	HW_IO_SIMULATE_REMOTE_DISC		= 27,
	HW_IO_SIMULATE_CONNECTED		= 28,
	HW_IO_GET_VC_CONNECT_STATE		= 29,
	HW_IO_OPEN_HW_IO_INSTANCE		= 30,
	HW_IO_CLOSE_HW_IO_INSTANCE		= 31,
	HW_IO_GET_PRODUCT_VERSION		= 32,
	HW_IO_GET_CURRENT_MAC_ADDR		= 33,

	HW_IO_INIT_DATA_PUMP			= 34,
	HW_IO_GET_DP_VERSIONS			= 35,
	HW_IO_GET_CONN_CONFIG			= 36,
	HW_IO_SET_CONN_CONFIG			= 37,
	HW_IO_GET_MODEM_DATA			= 38,
	HW_IO_GET_DATA_PATH_SELECTION	= 39,
	HW_IO_SET_DATA_PATH_SELECTION	= 40,
	HW_IO_GET_LINE_STATUS			= 41,
	HW_IO_GET_LINE_STATE			= 42,
	HW_IO_GET_REGISTER				= 43,
	HW_IO_SET_REGISTER				= 44,
	HW_IO_GET_BUS_CONTROLLER		= 45,
	HW_IO_GET_PM_POWER_STATE		= 46,
	HW_IO_SET_PM_POWER_STATE		= 47,
	HW_IO_GET_TRANSCEIVER_STATUS	= 48,
	HW_IO_GET_ADSL_CONFIG			= 49,	// ** NOT USED **
	HW_IO_SET_ADSL_CONFIG			= 50,	// ** NOT USED **
	HW_IO_GET_PERFORMANCE			= 51,
	HW_IO_GET_TEXT_LOG				= 52,

	HW_IO_START_DIGITAL_LOOPBACK	= 53,
	HW_IO_STOP_DIGITAL_LOOPBACK		= 54,
	HW_IO_START_CELL_VERIFY			= 55,
	HW_IO_STOP_CELL_VERIFY			= 56,
	HW_IO_GET_CELL_VERIFY_STATS		= 57,
	HW_IO_START_ATM_BER				= 58,
	HW_IO_STOP_ATM_BER				= 59,
	HW_IO_INJECT_ATM_BER_ERRORS		= 60,
	HW_IO_GET_ATM_BER_STATUS		= 61,
	HW_IO_GET_ATM_BER_STATS			= 62,
	HW_IO_GET_ATM_BER_64BIT_STATS	= 63,
	HW_IO_CLEAR_ATM_BER_STATS		= 64,

	HW_IO_SET_TRACE					= 65,
	HW_IO_DEBUG						= 66,

	HW_IO_GET_LOG_START				= 67,
	HW_IO_GET_BUFMGMT_LOG				= HW_IO_GET_LOG_START,
	HW_IO_GET_FRAMEAL_LOG			= 68,
	HW_IO_GET_CHIPAL_LOG			= 69,
	HW_IO_GET_LOG_RESERVED_01		= 70,
	HW_IO_GET_LOG_RESERVED_02		= 71,
	HW_IO_GET_LOG_RESERVED_03		= 72,
	HW_IO_GET_LOG_RESERVED_04		= 73,
	HW_IO_GET_LOG_RESERVED_05		= 74,
	HW_IO_GET_LOG_RESERVED_06		= 75,
	HW_IO_GET_LOG_RESERVED_07		= 76,
	HW_IO_GET_LOG_RESERVED_08		= 77,
	HW_IO_GET_LOG_RESERVED_09		= 78,
	HW_IO_GET_LOG_RESERVED_10		= 79,
	HW_IO_GET_LOG_RESERVED_11		= 80,
	HW_IO_GET_LOG_RESERVED_12		= 81,
	HW_IO_GET_LOG_RESERVED_13		= 82,
	HW_IO_GET_LOG_RESERVED_14		= 83,
	HW_IO_GET_LOG_RESERVED_15		= 84,
	HW_IO_GET_LOG_RESERVED_16		= 85,
	HW_IO_GET_LOG_RESERVED_17		= 86,
	HW_IO_GET_LOG_RESERVED_18		= 87,
	HW_IO_GET_LOG_RESERVED_19		= 88,
	HW_IO_GET_LOG_RESERVED_20		= 89,
	HW_IO_GET_LOG_END					= HW_IO_GET_LOG_RESERVED_20,
	
	HW_IO_GET_AUTO_SENSE			= 90,	// HW_IO_SET_AUTO_SENSE is further up table
	HW_IO_GET_MODULATION			= 91,

	HW_IO_OAM_LOOPBACK_INIT			= 92,  // F5 PING send request
	HW_IO_OAM_LOOPBACK_RESPONSE		= 93,  // results of F5 PING request


	HW_IO_SERIAL_DATA_OPEN			= 94,
	HW_IO_SERIAL_DATA_CLOSE			= 95,
	HW_IO_SERIAL_DATA_READ	    	= 96,
	HW_IO_SERIAL_DATA_WRITE			= 97,

	HW_IO_GET_SWITCHHOOK_TYPE		= 98,
	HW_IO_SET_SWITCHHOOK_TYPE		= 99,
	HW_IO_GET_SWITCHHOOK_STATUS		= 100,
	HW_IO_SET_SOFTWARE_SWITCHHOOK	= 101,

	HW_IO_GET_HEAD_END				= 102,
	HW_IO_SET_HEAD_END				= 103,
	HW_IO_GET_HEAD_END_ENV			= 104,
	HW_IO_SET_HEAD_END_ENV			= 105,
	
	HW_IO_IS_LINE_STARTABLE			= 106,
	
   	HW_IO_READ_DRIVER_COMMAND_ASYNC	= 107,

	HW_IO_OPEN_CONTROL_CHANNEL			= 108,
	HW_IO_CLOSE_CONTROL_CHANNEL			= 109,
	HW_IO_OPEN_COMMUNICATIONS_CHANNEL	= 110,
	HW_IO_CLOSE_COMMUNICATIONS_CHANNEL	= 111,

	HW_IO_GET_LATENCY_MODE			= 112,
	HW_IO_GET_INTERLEAVE_DEPTH		= 113,
	
	// ALWAYS ADD NEW enumeration elements here!

	HW_IO_LAST_CMD_CODE				// ALWAYS the last enumeration element!

} HW_IO_CMDS_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	HW IO 'Params' Definitions
//////////////////////////////////////////////

#include "hwiodmt.h"

//
//	HW_IO_OPEN_CONTROL_CHANNEL
//	HW_IO_OPEN_COMMUNICATIONS_CHANNEL
//

//
typedef enum HW_IO_CHANNEL_TYPE_E
{
	HW_IO_CHANNEL_CONTROL=0,
	HW_IO_CHANNEL_APPY,
	HW_IO_CHANNEL_SERIAL_DATA
} HW_IO_CHANNEL_TYPE_T;
	


typedef struct HW_IO_CONFIG_OPEN_CHANNEL_S
{
	IN	HW_IO_CHANNEL_TYPE_T 	ChannelType;
	OUT DWORD					InstanceId;

} HW_IO_CONFIG_OPEN_CHANNEL_T;


	

//
//	HW_IO_GET_BUFMGMT_CONFIG
//	HW_IO_SET_BUFMGMT_CONFIG
//

typedef struct HW_IO_BUFMGMT_CONFIG_S
{
	OUT IN	DWORD		RxMaxFrameSize;		// OUT for HW_IO_GET_BUFMGMT_CONFIG
											// IN  for HW_IO_SET_BUFMGMT_CONFIG
	OUT IN	DWORD		TxMaxFrameSize;		// OUT for HW_IO_GET_BUFMGMT_CONFIG
											// IN  for HW_IO_SET_BUFMGMT_CONFIG

} HW_IO_BUFMGMT_CONFIG_T,
  * PHW_IO_BUFMGMT_CONFIG_T;



//
//	HW_IO_GET_HW_INTERFACE_STATS
//

typedef struct HW_IO_HW_INTERFACE_STATS_S
{
	OUT	DWORD		InterruptCountTx;
	OUT	DWORD		InterruptCountRx;
	OUT	DWORD		BytesTransmitted;
	OUT	DWORD		BytesReceived;
	OUT	DWORD		InterruptsMissedTx;
	OUT	DWORD		InterruptsMissedRx;
	OUT	DWORD		UnderRun;
	OUT	DWORD		OverRun;
} HW_IO_HW_INTERFACE_STATS_T,
  * PHW_IO_HW_INTERFACE_STATS_T;



//
//	HW_IO_CLEAR_HW_INTERFACE_STATS
//

// NO Parameters



//
//	HW_IO_GET_FRAMEAL_STATS
//

typedef struct HW_IO_FRAMEAL_STATS_S
{
	OUT	DWORD		NumRxBytes;
	OUT	DWORD		NumRxFrames;

	OUT	DWORD		NumTxBytes;
	OUT	DWORD		NumTxFrames;

	OUT	DWORD		NumAbortErrs;
	OUT	DWORD		NumCrcErrs;
	OUT	DWORD		NumTxUnderrunErrs;

} HW_IO_FRAMEAL_STATS_T,
  * PHW_IO_FRAMEAL_STATS_T;



//
//	HW_IO_GET_FRAMEAL_ATM_STATS
//

typedef struct HW_IO_FRAMEAL_ATM_STATS_S
{
	IN	DWORD		VcIndex;

	OUT	DWORD		NumTxBytes;
	OUT	DWORD		NumTxCells;
	OUT	DWORD		NumTxMgmtCells;
	OUT	DWORD		NumTxClpEqual0Cells;
	OUT	DWORD		NumTxClpEqual1Cells;

	OUT	DWORD		NumRxBytes;
	OUT	DWORD		NumRxCells;
	OUT	DWORD		NumRxMgmtCells;
	OUT	DWORD		NumRxClpEqual0Cells;
	OUT	DWORD		NumRxClpEqual1Cells;

	OUT	DWORD		NumRxHecErrs;				//	for Link ONLY!
	OUT	DWORD		NumRxCellAlignErrs;			//	for Link ONLY!
	OUT	DWORD		NumRxUnroutCellErrs;		//	for Link ONLY!

} HW_IO_FRAMEAL_ATM_STATS_T,
  * PHW_IO_FRAMEAL_ATM_STATS_T;



//
//	HW_IO_GET_FRAMEAL_AAL_STATS
//

typedef enum HW_IO_AAL_TYPES_E
{
	HW_IO_AAL_TYPE_START	=	0,	// 0

	HW_IO_AAL_TYPE_0		=	0,	// 0	=> AAL_TYPE_AAL0	1
	HW_IO_AAL_TYPE_1,				// 1	=> AAL_TYPE_AAL1	2
	HW_IO_AAL_TYPE_34,				// 2	=> AAL_TYPE_AAL34	4
	HW_IO_AAL_TYPE_5,				// 3	=> AAL_TYPE_AAL5	8

	HW_IO_AAL_TYPE_END				// 4

} HW_IO_AAL_TYPES_T;

typedef struct HW_IO_FRAMEAL_AAL_STATS_S
{
	IN	DWORD		VcIndex;

// ????????????
	OUT	HW_IO_AAL_TYPES_T		AalType;			//	for VC (not Link) ONLY!

	OUT	DWORD		NumTxGoodBytes;
	OUT	DWORD		NumTxGoodFrames;
	OUT	DWORD		NumTxDiscardedBytes;
	OUT	DWORD		NumTxDiscardedFrames;

	OUT	DWORD		NumRxGoodBytes;
	OUT	DWORD		NumRxGoodFrames;
	OUT	DWORD		NumRxDiscardedBytes;
	OUT	DWORD		NumRxDiscardedFrames;

	OUT	DWORD		NumRxCrcErrs;
	OUT	DWORD		NumRxInvalidLenErrs;
	OUT	DWORD		NumRxTimeoutErrs;

} HW_IO_FRAMEAL_AAL_STATS_T,
  * PHW_IO_FRAMEAL_AAL_STATS_T;



//
//	HW_IO_GET_FRAMEAL_CONFIG
//	HW_IO_SET_FRAMEAL_CONFIG
//

typedef enum HW_IO_ENCAPSULATION_E		// RfcEncapsulationMode values:
{
	HW_IO_ENCAPSULATION_START			= 0,

											// NOTE:
											//		These values are from the
											//		ADSL Forum 99.102.0 spec.
											//	These values MUST MATCH the values
											//	from this document!!
	HW_IO_ENCAPSULATION_PPPOA_VCMUX		= 0,	// PPP over ATM VXMUX					// was RFC2364_VCMUX
	HW_IO_ENCAPSULATION_PPPOA_LLC,				// PPP over ATM LLC						// was RFC2364_LLC
	HW_IO_ENCAPSULATION_BIPOA_LLC,				// Bridged IP over ATM LLCSNAP			// was RFC1483_LLC_BRIDGED
	HW_IO_ENCAPSULATION_RIPOA_LLC,				// Routed IP over ATM LLCSNAP			// was RFC1483_LLC_ROUTED
	HW_IO_ENCAPSULATION_BIPOA_VCMUX,			// Bridged IP over ATM VCMUX			// was RFC1483_VCMUX_BRIDGED
	HW_IO_ENCAPSULATION_RIPOA_VCMUX,			// Routed IP over ATM VCMUX				// was RFC1483_VCMUX_ROUTED
	HW_IO_ENCAPSULATION_IPOA,					// Classical IP over ATM				// was n/a
	HW_IO_ENCAPSULATION_NATIVE_ATM,				// Native ATM							// was n/a
	HW_IO_ENCAPSULATION_PROPRIETARY		= -1,	// Proprietary							// was n/a
	// The next three enumerations are added for CNXT-defined encapsulations,
	// they are not found in the ADSL Forum documentation.
	HW_IO_ENCAPSULATION_PPPOA_NONE		= 0x80,	// PPP over ATM no encapsulation		// was RFC2364_NONE
	HW_IO_ENCAPSULATION_BIPOA_NONE,				// Bridged IP over ATM no encapsulation	// was RFC1483_NULL
	HW_IO_ENCAPSULATION_RIPOA_NONE,				// Routed IP over ATM no encapsulation	// was RFC1483_NULL
	HW_IO_ENCAPSULATION_NONE			= 0xF8,	// Encapsulation performed above driver	// was n/a

	HW_IO_ENCAPSULATION_END

} HW_IO_ENCAPSULATION_T;

typedef OUT IN	HW_IO_ENCAPSULATION_T		HW_IO_FRAMEAL_CONFIG_T;		// OUT for HW_IO_GET_FRAMEAL_CONFIG,
																		// IN  for HW_IO_SET_FRAMEAL_CONFIG
typedef HW_IO_FRAMEAL_CONFIG_T				* PHW_IO_FRAMEAL_CONFIG_T;






//
//	HW_IO_GET_EEPROM_MAC_ADDR
//	HW_IO_SET_EEPROM_MAC_ADDR
//	also same for HW_IO_GET_CURRENT_MAC_ADDR
//
#define NUMBER_MAC_ELEMENTS 6

typedef BYTE							HW_IO_MAC_ADDRESS_ELEMENT_T;		// Type of data for each element
typedef HW_IO_MAC_ADDRESS_ELEMENT_T		HW_IO_MAC_ADDRESS_T [NUMBER_MAC_ELEMENTS];	// OUT for HW_IO_GET_EEPROM_MAC_ADDR
															// IN  for HW_IO_SET_EEPROM_MAC_ADDR
															// OUT for HW_IO_GET_CURRENT_MAC_ADDR
typedef HW_IO_MAC_ADDRESS_T		* PHW_IO_MAC_ADDRESS_T;



//
//	HW_IO_GET_DEBUG_FLAG
//	HW_IO_SET_DEBUG_FLAG
//

typedef DWORD					HW_IO_DEBUG_FLAG_T;		// OUT for HW_IO_GET_DEBUG_FLAG
														// IN  for HW_IO_SET_DEBUG_FLAG
typedef HW_IO_DEBUG_FLAG_T		* PHW_IO_DEBUG_FLAG_T;



//
//	HW_IO_GET_PRODUCT_INFO
//

typedef struct HW_IO_PRODUCT_INFO_S
{
	OUT	DWORD		DrvrSwVer;
	OUT	DWORD		DrvrTapiVer;
	OUT	DWORD		DrvrNdisVer;

} HW_IO_PRODUCT_INFO_T,
  * PHW_IO_PRODUCT_INFO_T;



//
//	HW_IO_SIMULATE_INCOMING
//	HW_IO_SIMULATE_REMOTE_DISC
//	HW_IO_SIMULATE_CONNECTED
//

typedef IN	DWORD				HW_IO_VC_INDEX_T;
typedef HW_IO_VC_INDEX_T		* PHW_IO_VC_INDEX_T;



//
//	HW_IO_GET_VC_CONNECT_STATE
//

typedef enum HW_IO_VC_CONNECT_STATE_CODES_E
{
	HW_IO_VC_STATE_START			= 0,

	HW_IO_VC_STATE_IDLE				= 0,
	HW_IO_VC_STATE_DIALTONE,			//1
	HW_IO_VC_STATE_DIALING,				//2
	HW_IO_VC_STATE_PROCEEDING,			//3
	HW_IO_VC_STATE_RINGBACK,			//4
	HW_IO_VC_STATE_OFFERING,			//5
	HW_IO_VC_STATE_ACCEPTED,			//6
	HW_IO_VC_STATE_CONNECTED,			//7
	HW_IO_VC_STATE_DISCONNECTED,		//8
	HW_IO_VC_STATE_BUSY,				//9

	HW_IO_VC_STATE_END					//10

} HW_IO_VC_CONNECT_STATE_CODES_T;

typedef struct HW_IO_VC_CONNECT_STATE_S
{
	OUT	DWORD								VcSelectIndex;
	OUT	HW_IO_VC_CONNECT_STATE_CODES_T		VcConnectState;

} HW_IO_VC_CONNECT_STATE_T,
  * PHW_IO_VC_CONNECT_STATE_T;



//
//	HW_IO_OPEN_HW_IO_INSTANCE
//	HW_IO_CLOSE_HW_IO_INSTANCE
//

typedef enum HW_IO_INSTANCE_CODES_E
{
	HW_IO_INSTANCE_START			= 0,

	HW_IO_INSTANCE_SUCCESS			= 0,
	HW_IO_INSTANCE_EMPTY,				//1
	HW_IO_INSTANCE_UNAVAILABLE,			//2
	HW_IO_INSTANCE_FAILURE,				//3

	HW_IO_INSTANCE_END					//4

} HW_IO_INSTANCE_CODES_T;

typedef IN	DWORD				HW_IO_INSTANCE_T;
typedef HW_IO_INSTANCE_T		* PHW_IO_INSTANCE_T;



//
//	HW_IO_GET_PRODUCT_VERSION
//

#define MAX_HW_IO_PRODUCT_VERSION_SIZE		40

typedef OUT	CHAR					HW_IO_PRODUCT_VERSION_T [MAX_HW_IO_PRODUCT_VERSION_SIZE];
typedef HW_IO_PRODUCT_VERSION_T		* PHW_IO_PRODUCT_VERSION_T;



//
//	HW_IO_GET_CURRENT_MAC_ADDR
//

// Use existing define "HW_IO_MAC_ADDRESS_T".
// See "HW_IO_GET_EEPROM_MAC_ADDR" and "HW_IO_SET_EEPROM_MAC_ADDR"
//     for define of "HW_IO_MAC_ADDRESS_T".



//
//	HW_IO_
//	HW_IO_
//

//	This request response will return as many driver text log reference
//	numbers and parameters (that are queued in the driver) as it can fit
//	into the response.
//	Each driver text log will be of type "HW_IO_DRIVER_LOG_T", as defined below.
//		xxx->FirstDword.Words.Size = ZZZ1 (Text Msg Size, in DWORDs, not including the
//			 								1st DWORD a combination of Size and Type)
//		xxx->FirstDword.Words.Type = ZZZ2 (string reference number, type HW_IO_TEXT_MSGS_T)
//		xxx->DwordArray[0] = XYZ1 (if one or more parameters to log)
//		...
//		xxx->DwordArray[n] = XYZn (last parameter to log, if any)

#define NUM_PARAMS_0				0
#define NUM_PARAMS_1				1
#define NUM_PARAMS_6				6
#define MAX_DWORDS_IN_TEXT_LOG_MSG	64
#define MAX_BYTES_IN_TEXT_LOG_MSG	(sizeof(DWORD) * MAX_DWORDS_IN_TEXT_LOG_MSG)
#define MAX_NUM_LOG_PARAMS			(MAX_DWORDS_IN_TEXT_LOG_MSG - 2)	// subtract 2 for 64-bit timestamp value

#define BY_1_WORD					16

#define MASK_OFF_MSW				0x0000FFFF
#define MASK_OFF_LSW				0xFFFF0000

typedef enum HW_IO_TEXT_MSGS_E
{
	//	TEXT_MSG  ==>  TM

	HW_IO_TM_ZERO_NOT_USED			= 0,	// value of zero is not used!

	HW_IO_TM_PACKED_STRING,					// NOT USED in string lookup table ("TextLogStrings")
											//	since there is no predefined string constant.

	HW_IO_TM_ADSL_PSD_TEST_MODE,
	HW_IO_TM_ADSL_RX_IDLE_CHAR,
	HW_IO_TM_ADSL_RX_IDLE_TIMER,
	HW_IO_TM_API_HW_IF_MODE,
// ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
	HW_IO_TM_ATM_SERVER,					// ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
	HW_IO_TM_ATM_TX_CELL_A_FILE,
	HW_IO_TM_ATM_TX_CELL_B_FILE,
	HW_IO_TM_CELL_BURST_SIZE,
	HW_IO_TM_DP_AUTO_DOWNLOAD,
	HW_IO_TM_DP_BOOT_FILENAME,
	HW_IO_TM_DP_BOOT_JUMPADDRESS,
	HW_IO_TM_DP_FILENAME,
	HW_IO_TM_DP_JUMP_ADDRESS,
	HW_IO_TM_FPGA_PROGRAM_ENABLE,
	HW_IO_TM_FPGA_PROGRAM_FILE,
	HW_IO_TM_LINE_AUTO_ACTIVATION,
	HW_IO_TM_LINE_PERSISTENCE_TIMER,
	HW_IO_TM_LINE_PERSISTENT_ACTIVATION,
	HW_IO_TM_LT_MODE,
	HW_IO_TM_MAX_TRANSMIT,
	HW_IO_TM_NUMBER_OF_LINES,
	HW_IO_TM_RX_BUF_ALLOC_LIMIT,
	HW_IO_TM_RX_BUF_RATIO_LIMIT,
	HW_IO_TM_RX_FIFO_SIZE,
	HW_IO_TM_RX_INTERRUPT_RATE,
	HW_IO_TM_RX_MAX_LATENCY,
	HW_IO_TM_RX_MIN_LATENCY,
	HW_IO_TM_RX_SPEED,
	HW_IO_TM_TX_FIFO_SIZE,
	HW_IO_TM_TX_INTERRUPT_RATE,
	HW_IO_TM_TX_MAX_LATENCY,
	HW_IO_TM_TX_MIN_LATENCY,
	HW_IO_TM_TX_SPEED,
	HW_IO_TM_INVALID_MAC_ADDR,
	HW_IO_TM_INVALID_STRING,

	HW_IO_TM_END
} HW_IO_TEXT_MSGS_T;

//
//	These enumeration values are to be typed as a DWORD.
//	Where the most significant word (upper 16 bits) defines how many
//	parameters (DWORDs) are to follow (to be printed/displayed after
//	the indicated text string) and the least significant word (lower
//	16 bits) defines the enumeration value used to indicate the text
//	string (to be printed/displayed).
//

typedef enum HW_IO_LOG_MSGS_E
{
	//	LOG_MSG  ==>  LM

	HW_IO_LM_ZERO_NOT_USED				= 0,	// value of zero is not used!

	// SPECIAL CASE (for HW_IO_LM_PACKED_STRING):
	//		The most significant word will be the number of DWORDs that
	//		follow (a variable number, possibly different each time),
	//		containing a packed string (sz).
	//		This enumeration value is NOT USED in string lookup table
	//		("TextLogStrings") since there is no predefined string constant.
	HW_IO_LM_PACKED_STRING				= HW_IO_TM_PACKED_STRING,

	HW_IO_LM_ADSL_PSD_TEST_MODE			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_ADSL_PSD_TEST_MODE,
	HW_IO_LM_ADSL_RX_IDLE_CHAR			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_ADSL_RX_IDLE_CHAR,
	HW_IO_LM_ADSL_RX_IDLE_TIMER			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_ADSL_RX_IDLE_TIMER,
	HW_IO_LM_API_HW_IF_MODE				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_API_HW_IF_MODE,
// ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
	HW_IO_LM_ATM_SERVER					= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_ATM_SERVER,	// ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
/**/HW_IO_LM_ATM_TX_CELL_A_FILE			= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | HW_IO_TM_ATM_TX_CELL_A_FILE,
/**/HW_IO_LM_ATM_TX_CELL_B_FILE			= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | HW_IO_TM_ATM_TX_CELL_B_FILE,
	HW_IO_LM_CELL_BURST_SIZE			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_CELL_BURST_SIZE,
	HW_IO_LM_DP_AUTO_DOWNLOAD			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_DP_AUTO_DOWNLOAD,
/**/HW_IO_LM_DP_BOOT_FILENAME			= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | HW_IO_TM_DP_BOOT_FILENAME,
	HW_IO_LM_DP_BOOT_JUMPADDRESS		= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_DP_BOOT_JUMPADDRESS,
/**/HW_IO_LM_DP_FILENAME				= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | HW_IO_TM_DP_FILENAME,
	HW_IO_LM_DP_JUMP_ADDRESS			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_DP_JUMP_ADDRESS,
	HW_IO_LM_FPGA_PROGRAM_ENABLE		= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_FPGA_PROGRAM_ENABLE,
/**/HW_IO_LM_FPGA_PROGRAM_FILE			= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | HW_IO_TM_FPGA_PROGRAM_FILE,
	HW_IO_LM_LINE_AUTO_ACTIVATION		= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_LINE_AUTO_ACTIVATION,
	HW_IO_LM_LINE_PERSISTENCE_TIMER		= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_LINE_PERSISTENCE_TIMER,
	HW_IO_LM_LINE_PERSISTENT_ACTIVATION	= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_LINE_PERSISTENT_ACTIVATION,
	HW_IO_LM_LT_MODE					= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_LT_MODE,
	HW_IO_LM_MAX_TRANSMIT				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_MAX_TRANSMIT,
	HW_IO_LM_NUMBER_OF_LINES			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_NUMBER_OF_LINES,
	HW_IO_LM_RX_BUF_ALLOC_LIMIT			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_BUF_ALLOC_LIMIT,
	HW_IO_LM_RX_BUF_RATIO_LIMIT			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_BUF_RATIO_LIMIT,
	HW_IO_LM_RX_FIFO_SIZE				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_FIFO_SIZE,
	HW_IO_LM_RX_INTERRUPT_RATE			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_INTERRUPT_RATE,
	HW_IO_LM_RX_MAX_LATENCY				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_MAX_LATENCY,
	HW_IO_LM_RX_MIN_LATENCY				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_MIN_LATENCY,
	HW_IO_LM_RX_SPEED					= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_RX_SPEED,
	HW_IO_LM_TX_FIFO_SIZE				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_TX_FIFO_SIZE,
	HW_IO_LM_TX_INTERRUPT_RATE			= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_TX_INTERRUPT_RATE,
	HW_IO_LM_TX_MAX_LATENCY				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_TX_MAX_LATENCY,
	HW_IO_LM_TX_MIN_LATENCY				= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_TX_MIN_LATENCY,
	HW_IO_LM_TX_SPEED					= (NUM_PARAMS_1 << BY_1_WORD) | HW_IO_TM_TX_SPEED,
	HW_IO_LM_INVALID_MAC_ADDR			= (NUM_PARAMS_6 << BY_1_WORD) | HW_IO_TM_INVALID_MAC_ADDR,
	HW_IO_LM_INVALID_STRING				= (NUM_PARAMS_0 << BY_1_WORD) | HW_IO_TM_INVALID_STRING,

	HW_IO_LM_END
} HW_IO_LOG_MSGS_T;

#define HW_IO_TEXT_LOG_ATM_TX_CELL_A_FILE	"Default value for AtmTxCellAFile used "
#define HW_IO_TEXT_LOG_ATM_TX_CELL_B_FILE	"Default value for AtmTxCellBFile used "
#define HW_IO_TEXT_LOG_DP_BOOT_FILENAME	"Default value for DpBootFilename used "
#define HW_IO_TEXT_LOG_DP_FILENAME			"Default value for DpFilename used "
#define HW_IO_TEXT_LOG_FPGA_PROGRAM_FILE	"Default value for FpgaProgramFile used "

typedef struct HW_IO_LOG_TYPE_SIZE_S
{
	WORD		Type;
	WORD		Size;
} HW_IO_LOG_TYPE_SIZE_T;

typedef union HW_IO_LOG_1ST_DWORD_U
{
	DWORD						Dword;
	HW_IO_LOG_TYPE_SIZE_T		Words;
} HW_IO_LOG_1ST_DWORD_T;

typedef struct HW_IO_DRIVER_LOG_S
{
	HW_IO_LOG_1ST_DWORD_T		FirstDword;
	DWORD						DwordArray [MAX_DWORDS_IN_TEXT_LOG_MSG];
} HW_IO_DRIVER_LOG_T;

typedef struct HW_IO_DRIVER_LOG_WITH_TIME_S
{
	HW_IO_LOG_1ST_DWORD_T		FirstDword;
	LONGLONG					SystemTime;
	DWORD						DwordArray [MAX_NUM_LOG_PARAMS];
} HW_IO_DRIVER_LOG_WITH_TIME_T;



//
//	HW_IO_GET_CONN_CONFIG
//	HW_IO_SET_CONN_CONFIG
//

typedef enum HW_IO_PCR_KBPS_CODES_E
{
	HW_IO_PCR_START				= 0,

	HW_IO_PCR_SAME_AS_LINE		= 0,

	HW_IO_PCR_16_KBPS,				// 1
	HW_IO_PCR_32_KBPS,
	HW_IO_PCR_48_KBPS,
	HW_IO_PCR_64_KBPS,				// 4
	HW_IO_PCR_80_KBPS,
	HW_IO_PCR_96_KBPS,
	HW_IO_PCR_112_KBPS,
	HW_IO_PCR_128_KBPS,				// 8
	HW_IO_PCR_144_KBPS,
	HW_IO_PCR_160_KBPS,
	HW_IO_PCR_176_KBPS,
	HW_IO_PCR_192_KBPS,				// 12
	HW_IO_PCR_208_KBPS,
	HW_IO_PCR_224_KBPS,
	HW_IO_PCR_240_KBPS,
	HW_IO_PCR_256_KBPS,				// 16
	HW_IO_PCR_272_KBPS,
	HW_IO_PCR_288_KBPS,
	HW_IO_PCR_304_KBPS,
	HW_IO_PCR_320_KBPS,				// 20
	HW_IO_PCR_336_KBPS,
	HW_IO_PCR_352_KBPS,
	HW_IO_PCR_368_KBPS,
	HW_IO_PCR_384_KBPS,				// 24
	HW_IO_PCR_400_KBPS,
	HW_IO_PCR_416_KBPS,
	HW_IO_PCR_432_KBPS,
	HW_IO_PCR_448_KBPS,				// 28
	HW_IO_PCR_464_KBPS,
	HW_IO_PCR_480_KBPS,
	HW_IO_PCR_496_KBPS,
	HW_IO_PCR_512_KBPS,				// 32
	HW_IO_PCR_528_KBPS,
	HW_IO_PCR_544_KBPS,
	HW_IO_PCR_560_KBPS,
	HW_IO_PCR_576_KBPS,				// 36
	HW_IO_PCR_592_KBPS,
	HW_IO_PCR_608_KBPS,
	HW_IO_PCR_624_KBPS,
	HW_IO_PCR_640_KBPS,				// 40
	HW_IO_PCR_656_KBPS,
	HW_IO_PCR_672_KBPS,
	HW_IO_PCR_688_KBPS,
	HW_IO_PCR_704_KBPS,				// 44
	HW_IO_PCR_720_KBPS,
	HW_IO_PCR_736_KBPS,
	HW_IO_PCR_752_KBPS,
	HW_IO_PCR_768_KBPS,				// 48
	HW_IO_PCR_784_KBPS,
	HW_IO_PCR_800_KBPS,
	HW_IO_PCR_816_KBPS,
	HW_IO_PCR_832_KBPS,				// 52

	HW_IO_PCR_END

} HW_IO_PCR_KBPS_CODES_T;

typedef struct HW_IO_CONN_VPI_VCI_S
{
	BYTE						Vpi;		// Virtual Path Identifier
	WORD						Vci;		// Virtual Channel Identifier
	HW_IO_PCR_KBPS_CODES_T		Pcr;		// Peak Cell Rate

} HW_IO_CONN_VPI_VCI_T,
  * PHW_IO_CONN_VPI_VCI_T;

typedef struct HW_IO_CONN_CONFIG_S
{
	OUT IN	DWORD						NumOfVc;	// OUT for HW_IO_GET_CONN_CONFIG
													// IN  for HW_IO_SET_CONN_CONFIG
//	OUT IN	HW_IO_CONN_VPI_VCI_T		VcArray [DEFAULT_NUMBER_OF_LINES];	// OUT for HW_IO_GET_CONN_CONFIG
																			// IN  for HW_IO_SET_CONN_CONFIG

} HW_IO_CONN_CONFIG_T,
  * PHW_IO_CONN_CONFIG_T;

typedef struct HW_IO_LOG_DATA_S
{
	OUT IN	DWORD		*BytesUsed;
	OUT IN	DWORD		BytesMax;
	OUT IN	DWORD		*Buffer;
} HW_IO_LOG_DATA_T;

//
//	HW_IO_GET_MODEM_DATA
//

#define MAX_HW_IO_MODEM_BLOCK_DWORDS		32

typedef BYTE						HW_IO_MODEM_BLOCK_DATA_T [MAX_HW_IO_MODEM_BLOCK_DWORDS * sizeof( DWORD )];
typedef HW_IO_MODEM_BLOCK_DATA_T	* PHW_IO_MODEM_BLOCK_DATA_T;

	//	The following definition gives the size (in BYTES) of the
	//	space left after the elements "ModemBlockStartAddr" and
	//	"ModemBlockLength" below.
#define HW_IO_MODEM_BLOCK_DATA_SIZE		sizeof( HW_IO_MODEM_BLOCK_DATA_T )

typedef struct HW_IO_MODEM_DATA_S
{
	IN	DWORD							ModemBlockStartAddr;
	IN	DWORD							ModemBlockLength;
	OUT	HW_IO_MODEM_BLOCK_DATA_T		ModemBlockData;

} HW_IO_MODEM_DATA_T,
  * PHW_IO_MODEM_DATA_T;



//
//	HW_IO_GET_REGISTER
//	HW_IO_SET_REGISTER
//

// RegDataSize
typedef enum HW_IO_REG_DATA_SIZE_E
{
	HW_IO_REG_DATA_SIZE_START		= 0,

	HW_IO_REG_DATA_SIZE_DWORD_4		= 0,
	HW_IO_REG_DATA_SIZE_WORD_2,			// 1
	HW_IO_REG_DATA_SIZE_BYTE_1,			// 2

	HW_IO_REG_DATA_SIZE_END				// 3

} HW_IO_REG_DATA_SIZE_T;
#define DEFAULT_HW_IO_REG_DATA_SIZE		HW_IO_REG_DATA_SIZE_DWORD_4

// RegAccess
typedef enum HW_IO_REG_ACCESS_E
{
	HW_IO_REG_ACCESS_START			= 0,

	HW_IO_REG_ACCESS_MEMORY			= 0,
	HW_IO_REG_ACCESS_HOBBES,			// 1
	HW_IO_REG_ACCESS_ALCATEL_ADSL,		// 2
	HW_IO_REG_ACCESS_BASIC2,			// 3
	HW_IO_REG_ACCESS_EEPROM,			// 4
	HW_IO_REG_ACCESS_ARM,				// 5
	HW_IO_REG_ACCESS_MICROIF,			// 6

	HW_IO_REG_ACCESS_END				// 7

} HW_IO_REG_ACCESS_T;
#define DEFAULT_HW_IO_REG_ACCESS		HW_IO_REG_ACCESS_MEMORY

#define MAX_HW_IO_REGISTER_BYTES		128

typedef struct HW_IO_REGISTER_S
{
	IN		DWORD						RegOffset;
	IN		DWORD						RegMask;
// ??????????????  use RegValue  -or-  VarLength and VarBuff  ??????????????????????
	OUT IN	DWORD						RegValue;	// OUT for HW_IO_GET_REGISTER
													// IN  for HW_IO_SET_REGISTER
	IN		HW_IO_REG_DATA_SIZE_T		RegDataSize;
	IN		HW_IO_REG_ACCESS_T			RegAccess;
// ??????????????  use RegValue  -or-  VarLength and VarBuff  ??????????????????????
	IN		DWORD						VarLength;
	OUT IN	BYTE						VarBuff [MAX_HW_IO_REGISTER_BYTES];	// OUT for HW_IO_GET_REGISTER
																			// IN  for HW_IO_SET_REGISTER

} HW_IO_REGISTER_T,
  * PHW_IO_REGISTER_T;



//
//	HW_IO_GET_BUS_CONTROLLER
//

#define	HW_IO_BUS_CTRLR_BASIC_2p1		"BASIC 2.1"
#define	HW_IO_BUS_CTRLR_BASIC_2p15		"BASIC 2.15"
#define	HW_IO_BUS_CTRLR_UNKNOWN			"*Unknown!*"

#define MAX_HW_IO_BUS_CONTROLLER_BYTES		40

typedef OUT	CHAR					HW_IO_BUS_CONTROLLER_T [MAX_HW_IO_BUS_CONTROLLER_BYTES];
typedef HW_IO_BUS_CONTROLLER_T		* PHW_IO_BUS_CONTROLLER_T;



//
//	HW_IO_GET_PM_POWER_STATE
//	HW_IO_SET_PM_POWER_STATE
//

typedef enum HW_IO_POWER_STATE_CODES_E
{
	HW_IO_PM_STATE_START	= 0,

	HW_IO_PM_STATE_D0		= HW_IO_PM_STATE_START,	/* Fully Operational */
	HW_IO_PM_STATE_D1,								/* PCI Clock Running, Process PME Event*/
	HW_IO_PM_STATE_D2,								/* PCI Clock Off, Process PME Event */
	HW_IO_PM_STATE_D3_HOT,							/* Power Off to Devices, Context Maintained*/
	HW_IO_PM_STATE_D3_COLD,							/* Power Off to Devices, Context lost */

	HW_IO_PM_STATE_END

} HW_IO_PM_POWER_STATE_CODES_T;

typedef DWORD						HW_IO_PM_POWER_STATE_T;		// OUT for HW_IO_GET_PM_POWER_STATE
																// IN  for HW_IO_SET_PM_POWER_STATE
typedef HW_IO_PM_POWER_STATE_T		* PHW_IO_PM_POWER_STATE_T;



//
//	HW_IO_START_DIGITAL_LOOPBACK
//

typedef struct HW_IO_DIGITAL_LOOPBACK_S
{
	IN	DWORD						TestDigLbVpi;		// Virtual Path Identifier
	IN	DWORD						TestDigLbVci;		// Virtual Channel Identifier
	IN	HW_IO_PCR_KBPS_CODES_T		TestDigLbPcr;		// Peak Cell Rate

} HW_IO_DIGITAL_LOOPBACK_T,
  * PHW_IO_DIGITAL_LOOPBACK_T;



//
//	HW_IO_STOP_DIGITAL_LOOPBACK
//

// NO Parameters



//
//	HW_IO_START_CELL_VERIFY
//

typedef enum HW_IO_CELL_VERIFY_PATTERNS_E
{
	HW_IO_CELL_VERIFY_PATTERN_START		= 0,

	HW_IO_CELL_VERIFY_PATTERN_1			= 0,

	HW_IO_CELL_VERIFY_PATTERN_END			// 1

} HW_IO_CELL_VERIFY_PATTERNS_T;

typedef struct HW_IO_CELL_VERIFY_S
{
	IN	DWORD								Vpi;		// Virtual Path Identifier
	IN	DWORD								Vci;		// Virtual Channel Identifier
	IN	HW_IO_CELL_VERIFY_PATTERNS_T		PatternIndex;
	IN	HW_IO_PCR_KBPS_CODES_T				Pcr;		// Peak Cell Rate

} HW_IO_CELL_VERIFY_T,
  * PHW_IO_CELL_VERIFY_T;



//
//	HW_IO_STOP_CELL_VERIFY
//

// NO Parameters



//
//	HW_IO_GET_CELL_VERIFY_STATS
//

typedef struct HW_IO_CELL_VERIFY_STATS_S
{
	OUT	DWORD		NumTestFramesTxd;
	OUT	DWORD		NumGoodTestFramesRxd;
	OUT	DWORD		NumOutOfOrderTestFramesRxd;

} HW_IO_CELL_VERIFY_STATS_T,
  * PHW_IO_CELL_VERIFY_STATS_T;



//
//	HW_IO_START_ATM_BER
//	HW_IO_GET_ATM_BER_STATUS
//

typedef enum HW_IO_ATM_BER_PATTERNS_E
{
	HW_IO_ATM_BER_PATTERN_START			= 0,

	HW_IO_ATM_BER_PATTERN_NO_PATTERN	= 0,
	HW_IO_ATM_BER_PATTERN_15BIT,
	HW_IO_ATM_BER_PATTERN_20BIT,
	HW_IO_ATM_BER_PATTERN_23BIT,
	HW_IO_ATM_BER_PATTERN_USER_SUPPLIED,

	HW_IO_ATM_BER_PATTERN__END

} HW_IO_ATM_BER_PATTERNS_T;

typedef enum HW_IO_ATM_BER_STATUS_CODES_E
{
	HW_IO_ATM_BER_START		= 0,

	HW_IO_ATM_BER_IDLE		= 0,
	HW_IO_ATM_BER_ACTIVE,
	HW_IO_ATM_BER_RXONLY,

	HW_IO_ATM_BER_END

} HW_IO_ATM_BER_STATUS_CODES_T;

typedef enum HW_IO_ATM_BER_INSERT_ERR_CODES_E
{
	HW_IO_ATM_BER_INJECT_START			= 0,

	HW_IO_ATM_BER_INJECT_NO_ERROR		= 0,
	HW_IO_ATM_BER_INJECT_SINGLE_ERROR,			// insert one error
	HW_IO_ATM_BER_INJECT_ERROR_M3		= 3,	// insert one error every 10^3 bits
	HW_IO_ATM_BER_INJECT_ERROR_M4,				// insert one error every 10^4 bits
	HW_IO_ATM_BER_INJECT_ERROR_M5,				// insert one error every 10^5 bits
	HW_IO_ATM_BER_INJECT_ERROR_M6,				// insert one error every 10^6 bits
	HW_IO_ATM_BER_INJECT_ERROR_M7,				// insert one error every 10^7 bits

	HW_IO_ATM_BER_INJECT_END

} HW_IO_ATM_BER_INSERT_ERR_CODES_T;

typedef struct HW_IO_ATM_BER_START_S
{
	IN	DWORD									Vpi;		// Virtual Path Identifier
	IN	DWORD									Vci;		// Virtual Channel Identifier
	IN	HW_IO_PCR_KBPS_CODES_T					Pcr;		// Peak Cell Rate
	IN	HW_IO_ATM_BER_PATTERNS_T				Pattern;
	IN	DWORD									UserPatternMask;
	IN	BOOLEAN									RXInvert;	// expect receive data to be inverted
	IN	BOOLEAN									TXInvert;	// Invert transmit data
	IN	HW_IO_ATM_BER_STATUS_CODES_T			Status;		// On start sets status
	IN	HW_IO_ATM_BER_INSERT_ERR_CODES_T		ErrInsertion;

} HW_IO_ATM_BER_START_T,
  * PHW_IO_ATM_BER_START_T,
  HW_IO_ATM_BER_STATUS_T,
  * PHW_IO_ATM_BER_STATUS_T;


//
//	HW_IO_STOP_ATM_BER
//

// NO Parameters



//
//	HW_IO_INJECT_ATM_BER_ERRORS
//

typedef IN	DWORD					HW_IO_ATM_BER_INJECT_T;
typedef HW_IO_ATM_BER_INJECT_T		* PHW_IO_ATM_BER_INJECT_T;



//
//	HW_IO_GET_ATM_BER_STATS
//
typedef struct HW_IO_ATM_BER_STATS_S
{
	DWORD		RXBits;			// multiply by 10000
	DWORD		RXErrorBits;
	DWORD		RXResyncs;
	DWORD		TXBits;			// multiply by 10000

} HW_IO_ATM_BER_STATS_T,
  * PHW_IO_ATM_BER_STATS_T;



//
//	HW_IO_GET_ATM_BER_64BIT_STATS
//

typedef struct HW_IO_ATM_BER_64BIT_STATS_S
{
	LONGLONG				RXBits;
	LONGLONG				RXErrorBits;
	LONGLONG				RXResyncs;
	LONGLONG				TXBits;

} HW_IO_ATM_BER_64BIT_STATS_T,
  * PHW_IO_ATM_BER_64BIT_STATS_T;



//
//	HW_IO_CLEAR_ATM_BER_STATS
//

// NO Parameters



//
//	HW_IO_DEBUG
//

// DETAILS:
//		On the control panel, use [ALT A] to enable the full set of tabs
//		Click the "Custom Driver Commands"
//		Enter 60012 for the opcode
//		Enter desired sub-code(s) in the data field.
//		The first sub-code will be/land in Buffer[0].

#define MAX_DWORDS_IN_DEBUG_MSG	20

typedef IN	DWORD			HW_IO_DEBUG_T [MAX_DWORDS_IN_DEBUG_MSG];
typedef HW_IO_DEBUG_T		* PHW_IO_DEBUG_T;



//
//	HW_IO_OAM_LOOPBACK_INIT
// 	HW_IO_OAM_LOOPBACK_RESPONSE
//
typedef enum  HW_IO_OAM_LOOPBACK_TYPE_E
{
	HW_IO_OAM_LOOPBACK_TYPE_F4_END_TO_END = 0,
	HW_IO_OAM_LOOPBACK_TYPE_F4_SEGMENT,
	HW_IO_OAM_LOOPBACK_TYPE_F5_END_TO_END,
	HW_IO_OAM_LOOPBACK_TYPE_F5_SEGMENT

} HW_IO_OAM_LOOPBACK_TYPE_T;
	
#define HW_IO_OAM_LOCATION_ID_LEN 	0x10
#define HW_IO_OAM_SOURCE_ID_LEN 	0x10


typedef enum  HW_IO_OAM_LOOPBACK_STATUS_E
{
	HW_IO_OAM_LOOPBACK_STATUS_PENDING = 0,
	HW_IO_OAM_LOOPBACK_STATUS_SUCCESS,
	HW_IO_OAM_LOOPBACK_STATUS_FAILURE,
	HW_IO_OAM_LOOPBACK_STATUS_INVALID_REQ
} HW_IO_OAM_LOOPBACK_STATUS_T;

typedef struct HW_IO_OAM_LOOPBACK_S
{
	WORD						VCI;     // optional value, set to zero, driver selects VCI for 
	HW_IO_OAM_LOOPBACK_TYPE_T 	Type;    // type of OAM ping requested
	DWORD						CorrelationTag;  // non-zero value 
	BYTE						LocationId[HW_IO_OAM_LOCATION_ID_LEN];		 //  optional value, if set to zero, driver will insert
	BYTE					 	SourceId[HW_IO_OAM_SOURCE_ID_LEN];		 //  optional value, if set to zero, driver will insert
	HW_IO_OAM_LOOPBACK_STATUS_T Status;  		 // Valid only for HW_IO_OAM_LOOPBACK_RESPONSE 
} HW_IO_OAM_LOOPBACK_T;


//
//	HW_IO_SERIAL_DATA_OPEN	
//	HW_IO_SERIAL_DATA_CLOSE
//	HW_IO_SERIAL_DATA_READ	
//	HW_IO_SERIAL_DATA_WRITE
//

#define HW_IO_SERIAL_BUFFER_SIZE 256

typedef struct HW_IO_SERIAL_DATA_S
{
	DWORD	BufferUsed;
	BYTE 	Data [HW_IO_SERIAL_BUFFER_SIZE];
} HW_IO_SERIAL_DATA_T;
	
	




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Return Codes Definitions
//////////////////////////////////////////////


#define UNKNOWN_STRING			"Unknown!"
#define UNKNOWN_4BIT_VALUE		0x0000000F
#define UNKNOWN_8BIT_VALUE		0x000000FF
#define UNKNOWN_16BIT_VALUE		0x0000FFFF
#define UNKNOWN_32BIT_VALUE		0xFFFFFFFF


//
//	The BackDoor Return Code is defined using a 32-bit value.
//	This 32-bit value is broken/divided into four parts.
//	The four individual parts are:
//
//		Field Name			Bit Positions			# of Bits
//		===============		===================		=========
//		Error Code			bits  0 through 11		12 bits
//		Module Instance		bits 12 through 15		 4 bits
//		Module				bits 16 through 27		12 bits
//		Error Level			bits 28 through 31		 4 bits
//
//	Each individual part named above has its own enumeration/definition
//	of possible values defined below.
//



typedef enum RESULT_CODES_E
{
	RESULT_START						= 0,

	RESULT_SUCCESS						= 0,
	RESULT_FAILURE,

	RESULT_REQ_NOT_SUPPORTED,
	RESULT_REQ_BUFF_TOO_SMALL,

	RESULT_HW_IO_INSTANCE_UNAVAIL,			//		0x004
	RESULT_HW_IO_INSTANCE_EMPTY,			// 5

	RESULT_DATA_UNAVAILABLE,
	RESULT_DATA_UNINITIALIZED,
	RESULT_DATA_FAILURE,					//		0x008

	RESULT_DEVICE_BUSY,
	RESULT_DEVICE_UNAVAILABLE,				// 10
	RESULT_DEVICE_UNINITIALIZED,
	RESULT_DEVICE_DOWNLOADING,				//		0x00C
	RESULT_DEVICE_CMD_IN_PROG,
	RESULT_DEVICE_FAILURE,
	RESULT_DEVICE_HW_TIMER_FAIL,			// 15

	RESULT_CALL_UNINITIALIZED,				//		0x010

	RESULT_RESOURCE_ALLOC_FAILURE,
	RESULT_RESOURCE_CONFLICT,

	RESULT_PARAM_OUT_OF_RANGE,
	RESULT_PARAM_INVALID,					// 20	0x014

	RESULT_MODULE_UNINITIALIZED,

	RESULT_ADAPTER_NOT_FOUND,
	RESULT_ADAPTER_UNINITIALIZED,

	RESULT_END								// 24	0x018

} RESULT_CODES_T;

#define	RESULT_MASK			0x00000FFF



//	NOTE:	For the 'Instance' field, each module has its own set of
//			values and the starting value is the same for every module!
//			(i.e., the values are overlayed for each module)
//	NOTE:	The "INSTANCE_END" value MUST be set to be one greater
//			than the largest value!
typedef enum INSTANCE_E
{
	INSTANCE_SUCCESS				= 0,
	INSTANCE_START					= 0x1000,

	INSTANCE_UNKNOWN				= 0x1000,
	INSTANCE_CARDAL					= 0x2000,		//	CardAL Instances
	INSTANCE_CARDMGMT				= 0x2000,		//	CardMgmt Instances
	INSTANCE_BASIC2					= 0x2000,		//	ChipAL Instances
	INSTANCE_P64					= 0x3000,
	INSTANCE_UTILITY				= 0x2000,		//	Utility Instances
	INSTANCE_ATM					= 0x2000,		//	FrameAL Instances
	INSTANCE_HDLC					= 0x3000,
	INSTANCE_WAN					= 0x2000,		//	BuffMgmt Instances
	INSTANCE_LAN					= 0x3000,
	INSTANCE_CO						= 0x4000,
	INSTANCE_CELLDATATEST			= 0x5000,

	INSTANCE_END					= 0x6000,		// CAUTION:	MUST BE one greater
													//			than largest instance!!

} INSTANCE_T;

#define	INSTANCE_MASK		0x0000F000


typedef enum MODULE_NAME_E
{
	MODULE_SUCCESS					= 0,
	MODULE_START					= 0x00010000,

	MODULE_UNKNOWN					= 0x00010000,
	MODULE_BUFFMGMT					= 0x00020000,
	MODULE_CARDAL					= 0x00030000,
	MODULE_CARDMGMT					= 0x00040000,
	MODULE_CHIPAL					= 0x00050000,
	MODULE_FRAMEAL					= 0x00060000,
	MODULE_UTILITY					= 0x00070000,

	MODULE_END						= 0x00080000

} MODULE_NAME_T;

#define	MODULE_MASK			0x0FFF0000



typedef enum ERROR_LEVEL_E
{
	LEVEL_SUCCESS					= 0,
	LEVEL_START						= 0x10000000,

	LEVEL_NORMAL					= 0x10000000,
	LEVEL_WARNING					= 0x20000000,
	LEVEL_FATAL						= 0x30000000,

	LEVEL_END						= 0x40000000

} ERROR_LEVEL_T;

#define	LEVEL_MASK			0xF0000000



typedef struct ERROR_FIELDS_S
{
	RESULT_CODES_T			Results	:12;		// Error Code			(Enum)
	BYTE					Instance:4;			// Instance of Module	(Enum)
	WORD					Module	:12;		// Module Type			(Enum)
	BYTE					Level	:4;			// Severity Level		(Enum)
} ERROR_FIELDS_T,
  * PERROR_FIELDS_T;

typedef union ERROR_CODE_U
{
	DWORD				dwValue;		// Complete Error Code In a DWORD Value
	ERROR_FIELDS_T		Fields;			// Error Code Broken Into 4 Fields
} ERROR_CODE_T,
  * PERROR_CODE_T;
/*
**
**	Notes:
**
**	To check the complete value
**
**	ERROR_CODE_T	Ec;
**	...
**	if (Ec.Value == Success)
**	..
**
**	To check an individual Element:
**
**	if (Ec.Error.Results == SpecificResult)
**
**	...
**
*/



#endif		//#ifndef _HWIO_H_

