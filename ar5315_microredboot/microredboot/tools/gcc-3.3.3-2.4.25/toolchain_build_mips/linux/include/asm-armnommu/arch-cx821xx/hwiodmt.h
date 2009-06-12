/******************************************************************************
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
********************************************************************************
**	Copyright (c) 1997, 1998, 1999, 2000, 2001 Conexant Systems, Inc.
**	(formerly Rockwell Semiconductor Systems)
********************************************************************************

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

******************************************************************************

$Header: /cvs/sw/linux-2.4.x/include/asm-armnommu/arch-cx821xx/hwiodmt.h,v 1.1 2003/06/29 14:28:18 gerg Exp $
$Author: gerg $
*******************************************************************************/


/**/	//This is a Page Eject character.
#ifndef _HWIODMT_H_
#define _HWIODMT_H_





//
//	HW_IO_GET_ALARMS
//

typedef enum HW_IO_ADSL_ALARM_CODES_E
{
	HW_IO_ADSL_ALARM_START		= 0,

	HW_IO_ADSL_ALARM_NO_ALARM	= 0,	// no alarm
	HW_IO_ADSL_ALARM_LOF,				// lof_alarm
	HW_IO_ADSL_ALARM_MAR,				// low_snr_margin_alarm
	HW_IO_ADSL_ALARM_ES,				// es_alarm
	HW_IO_ADSL_ALARM_SES,				// severe error second
	HW_IO_ADSL_ALARM_LOS,				// los_alarm
	HW_IO_ADSL_ALARM_LCD,				// lcd_alarm
	HW_IO_ADSL_ALARM_SELF,				// self test
	HW_IO_ADSL_ALARM_FR,				// Fast Retrain						(ALARM NOT SUPPORTED)
	HW_IO_ADSL_ALARM_ELOF,				// excessive LOF within 15 minute
	HW_IO_ADSL_ALARM_ESES,				// excessive SES within 15 minute
	HW_IO_ADSL_ALARM_EUAS,				// excessive UAS within 15 minute
	HW_IO_ADSL_ALARM_ELOS,				// excessive LOS within 15 minute
	HW_IO_ADSL_ALARM_ELCD,				// excessive LCD within 15 minute
	HW_IO_ADSL_ALARM_EFR,				// excessive Fast Retrain within 15 minutes (ALARM NOT SUPPORTED)

	HW_IO_ADSL_ALARM_END

} HW_IO_ADSL_ALARM_CODES_T;

// size of list of alarms
#define NUM_HW_IO_ADSL_ALARMS 12

typedef struct HW_IO_ADSL_ALARM_LIST_S
{
	// if a slot has no alarm, that slot will contain HW_IO_ADSL_ALARM_NO_ALARM (0)
	HW_IO_ADSL_ALARM_CODES_T		AlarmList [NUM_HW_IO_ADSL_ALARMS];

} HW_IO_ADSL_ALARM_LIST_T,
  * PHW_IO_ADSL_ALARM_LIST_T;

typedef struct HW_IO_ALARMS_S
{
	OUT	HW_IO_ADSL_ALARM_LIST_T		UpStreamAlarms;
	OUT	HW_IO_ADSL_ALARM_LIST_T		DownStreamAlarms;

} HW_IO_ALARMS_T,
  * PHW_IO_ALARMS_T;



//
//	HW_IO_GET_BIT_ALLOC_TABLE
//

typedef enum HW_IO_BIT_ALLOC_BIN_STATUS_E
{
	BIT_ALLOC_BIN_START,

	UP_STREAM_BIN		= BIT_ALLOC_BIN_START,
	DOWN_STREAM_BIN,
	NO_ASSIGNED_BIN,

	BIT_ALLOC_BIN_END

} HW_IO_BIT_ALLOC_BIN_STATUS_T;

typedef struct HW_IO_BIT_ALLOC_BIN_ENTRY_S
{
	WORD								BinNumber;
	HW_IO_BIT_ALLOC_BIN_STATUS_T		BinStatus;
	WORD								BitsAssigned;
	WORD								BitCapacity;

} HW_IO_BIT_ALLOC_BIN_ENTRY_T,
  * PHW_IO_BIT_ALLOC_BIN_ENTRY_T;

// BitCapacity contains an implied decimal point between the upper and lower bytes.

#define NUM_HW_IO_BIT_ALLOC_BINS		16

typedef struct HW_IO_BIT_ALLOC_TABLE_S
{
	OUT	WORD							StartingBin;
	OUT	WORD							EndingBin;

	OUT	HW_IO_BIT_ALLOC_BIN_ENTRY_T		Entry [NUM_HW_IO_BIT_ALLOC_BINS];

} HW_IO_BIT_ALLOC_TABLE_T,
  * PHW_IO_BIT_ALLOC_TABLE_T;



//
//	HW_IO_GET_SNR_TABLE
//

typedef struct HW_IO_SNR_ENTRY_S
{
	WORD					BinNumber;
	WORD					BinSNR;

} HW_IO_SNR_ENTRY_T,
  * PHW_IO_SNR_ENTRY_T;

#define NUM_HW_IO_SNR_ENTRIES		16

typedef struct HW_IO_SNR_TABLE_S
{
	WORD					StartingBin;
	WORD					EndingBin;
	HW_IO_SNR_ENTRY_T		Entry [NUM_HW_IO_SNR_ENTRIES];

} HW_IO_SNR_TABLE_T,
  * PHW_IO_SNR_TABLE_T;



//
//	HW_IO_GET_CONTROLLER_LOG_CLT
//	HW_IO_SET_CONTROLLER_LOG_CLT
//

typedef OUT IN	BOOLEAN					HW_IO_CONTROLLER_LOG_CLT_T;		// OUT for HW_IO_GET_CONTROLLER_LOG_CLT
																		// IN  for HW_IO_SET_CONTROLLER_LOG_CLT
typedef HW_IO_CONTROLLER_LOG_CLT_T		* PHW_IO_CONTROLLER_LOG_CLT_T;



//
//	HW_IO_GET_GHS_CAP
//
//				G9221AnnexA.Negotiated		G9222AnnexAB.Negotiated
//	ANSI				FALSE						FALSE
//	G.LITE				FALSE						TRUE
//	G.DMT				TRUE						FALSE
//	invalid				TRUE						TRUE
//	
//				G9221AnnexA.LocalEndpoint	G9222AnnexAB.LocalEndpoint
//	ANSI				FALSE						FALSE
//	G.LITE				FALSE						TRUE
//	G.DMT				TRUE						FALSE
//	either				TRUE						TRUE
//
//
typedef struct HW_IO_GHS_CAP_ENTRY_S
{
	BOOLEAN		LocalEndpoint;
	BOOLEAN		RemoteEndpoint;
	BOOLEAN		Negiotiated;

} HW_IO_GHS_CAP_ENTRY_T,
  * PHW_IO_GHS_CAP_ENTRY_T;

typedef struct HW_IO_GHS_CAP_S
{
	OUT	HW_IO_GHS_CAP_ENTRY_T		V8;
	OUT	HW_IO_GHS_CAP_ENTRY_T		V8bis;
	OUT	HW_IO_GHS_CAP_ENTRY_T		SilentPeriod;
	OUT	HW_IO_GHS_CAP_ENTRY_T		GPLOAM;
	OUT	HW_IO_GHS_CAP_ENTRY_T		G9221AnnexA;
	OUT	HW_IO_GHS_CAP_ENTRY_T		G9221AnnexB;
	OUT	HW_IO_GHS_CAP_ENTRY_T		G9221AnnexC;
	OUT	HW_IO_GHS_CAP_ENTRY_T		G9222AnnexAB;
	OUT	HW_IO_GHS_CAP_ENTRY_T		G9222AnnexC;
	OUT	BYTE						RemoteCountry;
	OUT	BYTE						RemoteVendorID [4];
	OUT	BYTE						RemoteSpecInfo [2];
	OUT	BOOLEAN						Valid;		//Indicates whether remaining parms are valid

} HW_IO_GHS_CAP_T,
  * PHW_IO_GHS_CAP_T;



//
//	HW_IO_GET_GHS_LOCAL_CAP
//	HW_IO_SET_GHS_LOCAL_CAP
//

// AdslLocalG922Cap
typedef enum HW_IO_LOCAL_G922_CAP_E			// AdslLocalG922Cap values:
{
	HW_IO_GHS_CAPABILITIES_V8				= 1,
	HW_IO_GHS_CAPABILITIES_V8bis			= 2,
	HW_IO_GHS_CAPABILITIES_SilentPeriod		= 4,
	HW_IO_GHS_CAPABILITIES_GPLOAM			= 8
} HW_IO_LOCAL_G922_CAP_T;


// AdslLocalG922AnnexCap
typedef enum HW_IO_LOCAL_G922_ANNEX_CAP_E		// AdslLocalG922AnnexCap values:
{
	HW_IO_GHS_CAPABILITIES_922_1A			= 1,
	HW_IO_GHS_CAPABILITIES_922_1B			= 2,
	HW_IO_GHS_CAPABILITIES_922_1C			= 4,
	HW_IO_GHS_CAPABILITIES_922_2AB			= 8,
	HW_IO_GHS_CAPABILITIES_922_2C			= 0x10
} HW_IO_LOCAL_G922_ANNEX_CAP_T;



typedef struct HW_IO_GHS_LOCAL_CAP_S
{
	OUT IN	WORD		AdslLocalG922Cap;		// OUT for HW_IO_GET_GHS_LOCAL_CAP
												// IN  for HW_IO_SET_GHS_LOCAL_CAP
	OUT IN	WORD		AdslLocalG922AnnexCap;	// OUT for HW_IO_GET_GHS_LOCAL_CAP
												// IN  for HW_IO_SET_GHS_LOCAL_CAP

} HW_IO_GHS_LOCAL_CAP_T,
  * PHW_IO_GHS_LOCAL_CAP_T;



//
// HW_IO_GET_MODULATION
//

typedef enum HW_IO_MODULATION_CODES_E		// AdslModulation values:
{
	HW_IO_MODULATION_INVALID,
	HW_IO_MODULATION_ANSI_T413,
	HW_IO_MODULATION_GDMT,
	HW_IO_MODULATION_GLITE
} HW_IO_MODULATION_CODES_T;

typedef OUT	HW_IO_MODULATION_CODES_T		HW_IO_MODULATION_T;
typedef		HW_IO_MODULATION_T				* PHW_IO_MODULATION_T;



//
//	HW_IO_GET_AUTO_SENSE
//	HW_IO_SET_AUTO_SENSE
//	**** THESE MUST REMAIN IN SYNC WITH THE ENUMERATED VALUES IN XCVRINFO.H SYS_HANDSHAKE_TYPE and SYS_WIRING_TYPE
//

// Use DWORD for compatability with control panel Custom Driver Commands tab
// AdslSysAutoHandshake
typedef enum HW_IO_ADSL_DP_RATE_MODE_E		// AdslDpRateMode values:
{
	HW_IO_GHS_TONE_MODE			= 0,		// (Auto selection with G.hs preferred - default setting)
	HW_IO_TONE_GHS_MODE			= 1,		// (Auto selection with T1.413 preferred)
	HW_IO_G_HANDSHAKE_MODE		= 2,		// (G.hs only)
	HW_IO_G_TONE_MODE			= 3,		// (T1.413 only)
	HW_IO_G_DMT_ONLY			= 4, 		// (G.hs, G.dmt Only)
	HW_IO_G_LITE_ONLY			= 5,		// (G.hs, G.lite Only
	HW_IO_ADSL_DP_RATE_END
} HW_IO_ADSL_DP_RATE_MODE_T;


typedef enum HW_IO_ADSL_WIRING_SELECTION_E	// AdslSysWiring values:
{
	HW_IO_WIRING_COMBO_AUTO		= 0,		// "Automatic"
	HW_IO_WIRING_COMBO1			= 1,		// "Line Tip/Ring"
	HW_IO_WIRING_COMBO2			= 2,		// "Line A/A1"
	HW_IO_WIRING_COMBO3			= 3,		// "Aux Tip/Ring"
	HW_IO_WIRING_COMBO4			= 4,		// "Aux A/A1"
	HW_IO_WIRING_COMBO_END					//  Always keep at end of list
} HW_IO_ADSL_WIRING_SELECTION_T;


typedef struct HW_IO_AUTO_SENSE_S
{
	IN	HW_IO_ADSL_DP_RATE_MODE_T			AdslSysAutoHandshake;
	IN	HW_IO_ADSL_WIRING_SELECTION_T		AdslSysAutoWiring;
	IN	DWORD								AdslSysAutoWiresFirst;
} HW_IO_AUTO_SENSE_T,
  * PHW_IO_AUTO_SENSE_T;



//
//	HW_IO_GET_AUTO_SENSE_STATUS
//	**** THESE MUST REMAIN IN SYNC WITH THE ENUMERATED VALUES IN XCVRINFO.H SYS_HANDSHAKE_TYPE and SYS_WIRING_TYPE and SYS_AUTO_SENSING_STATE_TYPE
//

typedef enum HW_IO_ADSL_DP_AUTO_SENSING_STATE_E
{
	HW_IO_AUTO_SENSE_AUTO,
	HW_IO_AUTO_SENSE_LOCKED,
	HW_IO_AUTO_SENSE_WIRING_LOCKED
} HW_IO_ADSL_DP_AUTO_SENSING_STATE_T;

typedef struct HW_IO_AUTO_SENSE_STATUS_S
{
	IN	HW_IO_ADSL_DP_RATE_MODE_T			AdslSysAutoHandshake;
	IN	HW_IO_ADSL_WIRING_SELECTION_T		AdslSysAutoWiring;
	IN	HW_IO_ADSL_DP_AUTO_SENSING_STATE_T	AdslSysAutoSenseState;

} HW_IO_AUTO_SENSE_STATUS_T,
  * PHW_IO_AUTO_SENSE_STATUS_T;



//
//	HW_IO_USER_ACTIVATE_LINE
//	HW_IO_USER_DEACTIVATE_LINE
//

// NO Parameters



//
//	HW_IO_GET_CONNECT_STATUS
//

typedef enum HW_IO_CONNECT_STATUS_CODES_E
{
	HW_IO_CONNECT_STATUS_START			= 0,

	HW_IO_CONNECT_STATUS_IDLE			= 0,
	HW_IO_CONNECT_STATUS_HANDSHAKE,			//1
	HW_IO_CONNECT_STATUS_TRAINING,			//2
	HW_IO_CONNECT_STATUS_DATAMODE,			//3
	HW_IO_CONNECT_STATUS_RETRAIN,			//4
	HW_IO_CONNECT_STATUS_TESTMODE,			//5
	HW_IO_CONNECT_STATUS_POWERDOWN,			//6
	HW_IO_CONNECT_STATUS_FAILURECODE,		//7

	HW_IO_CONNECT_STATUS_END				//8

} HW_IO_CONNECT_STATUS_CODES_T;

typedef OUT	DWORD					HW_IO_CONNECT_STATUS_T;
typedef HW_IO_CONNECT_STATUS_T		* PHW_IO_CONNECT_STATUS_T;




//
//	HW_IO_INIT_DATA_PUMP
//

// NO Parameters



//
//	HW_IO_GET_DP_VERSIONS
//

typedef struct HW_IO_DATA_PUMP_VERSIONS_S
{
	OUT	DWORD		AdslVersionNear;		// 0 to 0xFF
	OUT	DWORD		AdslVersionFar;			// 0 to 0xFF
	OUT	DWORD		AdslVendorNear;			// 0 to 0xFFFF
	OUT	DWORD		AdslVendorFar;			// 0 to 0xFFFF
	OUT	DWORD		AdslDpSwVerMajor;		// 0 to 0xF
	OUT	DWORD		AdslDpSwVerMinor;		// 0 to 0xF
	OUT	DWORD		AdslDpSwVerSubMinor;	// 0 to 0xFF

} HW_IO_DATA_PUMP_VERSIONS_T,
  * PHW_IO_DATA_PUMP_VERSIONS_T;



//
//  HW_IO_GET_CONTROLLER_VERSION
//

#define MAX_DATAPUMP_VERSION_LENGTH 40

typedef struct HW_IO_CONTROLLER_VERSION_S
{
	OUT CHAR		AdslDpSwVersion[MAX_DATAPUMP_VERSION_LENGTH];	
} HW_IO_CONTROLLER_VERSION_T,  * PHW_IO_CONTROLLER_VERSION_T;





//
//  HW_IO_GET_RETRAIN_PARAMETERS
//  HW_IO_SET_RETRAIN_PARAMETERS
//

typedef struct _HW_IO_RETRAIN_PARAMETERS_S
{
										// Threshold of near-end SNR Margin used 
										// to detect and accumulate SNR Margin Errors
										// Valid values: 0 - 63
	I_O		DWORD	dwAdslSnrThresholdNear;	

										// Defines the threshold of near-end SNR Margin 
										// Errors detected prior to the initiation of a retrain. 
										// Valid values: 0 - 100
	I_O		DWORD	dwAdslSnrMarRetrainThresholdNear;
	
										// Defines the number of far-end physical layer CRC 
										// errors which can occur during a one-second period 
										// before the far-end CRC Errors accumulator is incremented  
										// Valid values: 0 - 100
	I_O		DWORD	dwAdslCrcThresholdFar;	

										// If the far-end CRC Error threshold is crossed, then the 
										// far-end CRC Errors" accumulator is incremented by this amount  
										// Valid values: 0 - 100
	I_O		DWORD	dwAdslCrcIncrementFar;	

										// Defines the threshold of far-end CRC Errored Seconds
										// detected prior to the initiation of a retrain   
										// Valid values: 0 - 100
	I_O		DWORD	dwAdslCrcRetrainThresholdFar;
	
										// Defines the number of near-end physical layer CRC errors that 
										// can occur before the Severely Errored Second accumulator is incremented  
										// Valid values: 0 - 100
	I_O		DWORD	dwAdslCrcThresholdNear;	
	
										// Defines the threshold of near-end consecutive Severely Errored Seconds 
										// detected prior to the initiation of a retrain.   
										// Valid values: 0 - 100
	I_O		DWORD	dwAdslSesRetrainThresholdNear;	

} HW_IO_RETRAIN_PARAMETERS_S,  * PHW_IO_RETRAIN_PARAMETERS_S;


//
//  HW_IO_GET_CONTROLLER_BUILD
//

#define MAX_CONTROLLER_BUILD_LENGTH 40

typedef struct HW_IO_CONTROLLER_BUILD_S
{
	OUT CHAR		ControllerBuild[MAX_CONTROLLER_BUILD_LENGTH];	
} HW_IO_CONTROLLER_BUILD_T,  * PHW_IO_CONTROLLER_BUILD_T;




// 
// HW_IO_GET_G9921_ANNEX_SELECTION
// HW_IO_SET_G9921_ANNEX_SELECTION
//
typedef enum HW_IO_G9921_SETTINGS_
{
	G9921_ANNEX_START = 0,
	G9921_ANNEX_A = 0,
	G9921_ANNEX_B = 1,
	G9921_ANNEX_C = 2,
	G9921_ANNEX_END
} HW_IO_G9921_SETTINGS_E;


typedef struct HW_IO_G9921_ANNEX_
{
	HW_IO_G9921_SETTINGS_E	Setting;
} HW_IO_G9921_ANNEX_T;


// 
// HWIO_GET_COMPATIBLE_ANNEX
// 
typedef enum HW_IO_G9921_COMPATIBLE_SETTINGS_
{
	G9921_ANNEX_COMPATIBLE_BIT_MASK_START = 1,
	G9921_ANNEX_A_COMPATIBLE_BIT_MASK = 1<<G9921_ANNEX_A,
	G9921_ANNEX_B_COMPATIBLE_BIT_MASK = 1<<G9921_ANNEX_B,
	G9921_ANNEX_C_COMPATIBLE_BIT_MASK = 1<<G9921_ANNEX_C,
	G9921_ANNEX_COMPATIBLE_END
} HW_IO_G9921_COMPATIBLE_SETTINGS_E;

typedef OUT	HW_IO_G9921_COMPATIBLE_SETTINGS_E		HW_IO_COMPATIBLE_ANNEX_VALUES_T;
typedef		HW_IO_COMPATIBLE_ANNEX_VALUES_T			* PHW_IO_COMPATIBLE_ANNEX_VALUES_T;


//
//	HW_IO_GET_LATENCY_MODE
//

typedef enum _HW_IO_LATENCY_MODES_E
{
	HW_IO_LATENCY_FAST				= 0,
	HW_IO_LATENCY_INTERLEAVED,
	HW_IO_LATENCY_UNKNOWN
} HW_IO_LATENCY_MODES_E;


typedef struct _HW_IO_LATENCY_MODE_S
{
	OUT HW_IO_LATENCY_MODES_E UpstreamLatency;
	OUT HW_IO_LATENCY_MODES_E DownstreamLatency;
} HW_IO_LATENCY_MODE_S,
  * PHW_IO_LATENCY_MODE_S;


//
//	HW_IO_GET_INTERLEAVE_DEPTH
//

typedef struct _HW_IO_INTERLEAVE_DEPTH_S
{
	OUT DWORD dwUpstreamDepth;
	OUT DWORD dwDownstreamDepth;
} HW_IO_INTERLEAVE_DEPTH_S,
  * PHW_IO_INTERLEAVE_DEPTH_S;

//
//	HW_IO_GET_DATA_PATH_SELECTION
//	HW_IO_SET_DATA_PATH_SELECTION
//

typedef enum HW_IO_DATA_PATH_CODES_E
{
	HW_IO_DATA_PATH_START			= 0,

	HW_IO_DATA_PATH_AUTOMATIC		= 0,
	HW_IO_DATA_PATH_FAST,				// 1
	HW_IO_DATA_PATH_INTERLEAVED,		// 2
	HW_IO_DATA_PATH_UNKNOWN,			// 3

	HW_IO_DATA_PATH_END					// 4

} HW_IO_DATA_PATH_CODES_T;
#define DEFAULT_HW_IO_DATA_PATH_SELECTION		HW_IO_DATA_PATH_FAST

typedef OUT IN	HW_IO_DATA_PATH_CODES_T		HW_IO_DATA_PATH_SELECTION_T;	// OUT for HW_IO_GET_DATA_PATH_SELECTION
																			// IN  for HW_IO_SET_DATA_PATH_SELECTION
typedef HW_IO_DATA_PATH_SELECTION_T			* PHW_IO_DATA_PATH_SELECTION_T;



//
//	HW_IO_GET_LINE_STATUS
//

typedef enum HW_IO_MODEM_STATUS_CODES_E
{
	HW_IO_MODEM_STATUS_START					= 0,

	HW_IO_MODEM_STATUS_DOWN						= 0,
	HW_IO_MODEM_STATUS_ACTIVATION,					// 1
	HW_IO_MODEM_STATUS_TRANSCEIVER_TRAINING,
	HW_IO_MODEM_STATUS_CHANNEL_ANALYSIS,
	HW_IO_MODEM_STATUS_EXCHANGE,
	HW_IO_MODEM_STATUS_ACTIVATED,					// 5
	HW_IO_MODEM_STATUS_WAITING_INIT,
	HW_IO_MODEM_STATUS_INITIALIZING,
	HW_IO_MODEM_STATUS_UNKNOWN,

	HW_IO_MODEM_STATUS_END							// 9

} HW_IO_MODEM_STATUS_CODES_T;

typedef struct HW_IO_LINE_STATUS_S
{
	OUT	DWORD							LineSpeedUpOrFar;
	OUT	DWORD							LineSpeedDownOrNear;
	OUT	HW_IO_MODEM_STATUS_CODES_T		LineState;

} HW_IO_LINE_STATUS_T,
  * PHW_IO_LINE_STATUS_T;



//
//	HW_IO_GET_LINE_STATE
//

typedef OUT	HW_IO_MODEM_STATUS_CODES_T		HW_IO_LINE_STATE_T;
typedef HW_IO_LINE_STATE_T					* PHW_IO_LINE_STATE_T;



//
//	HW_IO_GET_XCEIVER_STATUS
//

typedef struct			// Matches DATE_TYPE in types.h
{
	SHORT	year		:16;
	WORD	day			: 8;
	WORD	dayofweek	: 4;
	WORD	month		: 4;
} DATE_T,
  * PDATE_T;

typedef struct			// Matches TIME_TYPE in types.h
{
	BYTE		hr;
	BYTE		min;
	BYTE		sec;
} TIME_T,
  * PTIME_T;

typedef struct
{						// See DPUUTIL.C: UTIL_Get_ext_int16_str for how to print.
	SHORT	Mar_Cur;	// Stored as hundredths
	SHORT	Mar_Min;	// Stored as hundredths
	WORD	Mar_Min_Bin;
} SNR_MARGIN_T,
  * PSNR_MARGIN_T;

typedef struct HW_IO_XCEIVER_STATUS_S
{
	OUT	DATE_T				Date;
	OUT	TIME_T				Time;
	OUT	BYTE				Transmit_State;
	OUT	BYTE				Receive_State;
	OUT	BYTE				Process_State;
	OUT	SNR_MARGIN_T		Up_SNR_Margin;
	OUT	SNR_MARGIN_T		Down_SNR_Margin;
	OUT	SHORT				Up_Attn;			// Stored as hundredths
	OUT	SHORT				Down_Attn;			// Stored as hundredths
	OUT	CHAR				Tx_Power;
	OUT	WORD				Up_Bits_Per_Frame;
	OUT	WORD				Down_Bits_Per_Frame;
	OUT	WORD				Startup_Attempts;
	OUT	DWORD				Up_CRC_Errors;
	OUT	DWORD				Down_CRC_Errors;
	OUT	DWORD				Up_FEC_Errors;
	OUT	DWORD				Down_FEC_Errors;
	OUT	DWORD				Up_HEC_Errors;
	OUT	DWORD				Down_HEC_Errors;
	OUT	DWORD				Retrain_Attempts;

} HW_IO_XCEIVER_STATUS_T,
  * PHW_IO_XCEIVER_STATUS_T;





//
//	HW_IO_GET_TRANSCEIVER_STATUS
//

//typedef struct			// Matches DATE_TYPE in types.h
//{
//	SHORT	year		:16;
//	WORD	day			: 8;
//	WORD	dayofweek	: 4;
//	WORD	month		: 4;
//} DATE_T,
//  * PDATE_T;
//
//typedef struct			// Matches TIME_TYPE in types.h
//{
//	BYTE		hr;
//	BYTE		min;
//	BYTE		sec;
//} TIME_T,
//  * PTIME_T;
//
//typedef struct
//{						// See DPUUTIL.C: UTIL_Get_ext_int16_str for how to print.
//	SHORT	Mar_Cur;	// Stored 8.8 (8 bits integer, 8 bits fraction)
//	SHORT	Mar_Min;	// Stored 8.8 (8 bits integer, 8 bits fraction)
//	WORD	Mar_Min_Bin;
//} SNR_MARGIN_T,
//  * PSNR_MARGIN_T;

typedef struct HW_IO_TRANSCEIVER_STATUS_S
{
	OUT	DATE_T				Date;
	OUT	TIME_T				Time;
	OUT	BYTE				Transmit_State;
	OUT	BYTE				Receive_State;
	OUT	BYTE				Process_State;
	OUT	SNR_MARGIN_T		Up_SNR_Margin;
	OUT	SNR_MARGIN_T		Down_SNR_Margin;
	OUT	SHORT				Up_Attn;
	OUT	SHORT				Down_Attn;
	OUT	CHAR				Tx_Power;
	OUT	WORD				Up_Bits_Per_Frame;
	OUT	WORD				Down_Bits_Per_Frame;
	OUT	WORD				Startup_Attempts;
	OUT	DWORD				Up_CRC_Errors;
	OUT	DWORD				Down_CRC_Errors;
	OUT	DWORD				Up_FEC_Errors;
	OUT	DWORD				Down_FEC_Errors;
	OUT	DWORD				Up_HEC_Errors;
	OUT	DWORD				Down_HEC_Errors;
	OUT	DWORD				Retrain_Attempts;

} HW_IO_TRANSCEIVER_STATUS_T,
  * PHW_IO_TRANSCEIVER_STATUS_T;






//
//	HW_IO_GET_PERFORMANCE
//

typedef struct HW_IO_PERFORMANCE_S
{
	OUT	DWORD							R_relCapacityOccupationDnstr;
	OUT	DWORD							R_noiseMarginDnstr;
	OUT	DWORD							R_outputPowerUpstr;
	OUT	DWORD							R_attenuationDnstr;
	OUT	DWORD							R_relCapacityOccupationUpstr;
	OUT	DWORD							R_noiseMarginUpstr;
	OUT	DWORD							R_outputPowerDnstr;
	OUT	DWORD							R_attenuationUpstr;
	OUT	DWORD							R_ChanDataIntNear;
	OUT	DWORD							R_ChanDataFastNear;
	OUT	DWORD							R_ChanDataIntFar;
	OUT	DWORD							R_ChanDataFastFar;
	OUT	HW_IO_MODEM_STATUS_CODES_T		M_ModemStatus;
	OUT	DWORD							M_NsecValidBerFast;
	OUT	DWORD							M_AccBerFast;

} HW_IO_PERFORMANCE_T,
  * PHW_IO_PERFORMANCE_T;



//
//	HW_IO_SET_TRACE
//

typedef enum
{
	HW_IO_DIAGNOS_WRITE_BUS		= 0x0001,	// Send DiagnosticWrites	to Bus
	
	HW_IO_DIAGNOS_LOG_TRACE		= 0x0002,	// Trace Diagnostic log		to debug port
	HW_IO_DBG_PORT_TRACE		= 0x0004,	// trace RAM Buffer			to debug port
	
	HW_IO_AOC_TRACE				= 0x0008,	// trace AOC				to RAM Buffer
	HW_IO_EOC_TRACE				= 0x0010,	// trace EOC				to RAM Buffer
	HW_IO_AFE_WRITE_TRACE		= 0x0020,	// trace AFE				to RAM Buffer
	HW_IO_DMT_FSM_TRACE			= 0x0040,	// trace DMT FSM			to RAM Buffer
	HW_IO_AUTOSENSE_TRACE		= 0x0080	// trace Autosense			to RAM Buffer
	
} HW_IO_TRACE_CODES_T;

typedef IN	DWORD			HW_IO_TRACE_T;
typedef HW_IO_TRACE_T		* PHW_IO_TRACE_T;



//
//	HW_IO_GET_SWITCHHOOK_TYPE
//	HW_IO_SET_SWITCHHOOK_TYPE
//

typedef enum HW_IO_SWITCHHOOK_TYPE_S
{
	HW_IO_SWITCHHOOK_TYPE_FIRST			= 0,
	HW_IO_SWITCHHOOK_TYPE_START			= HW_IO_SWITCHHOOK_TYPE_FIRST-1,

	HW_IO_SWITCHHOOK_HARDWARE,
	HW_IO_SWITCHHOOK_SOFTWARE,

	HW_IO_SWITCHHOOK_TYPE_END,
	HW_IO_SWITCHHOOK_TYPE_LAST			= HW_IO_SWITCHHOOK_TYPE_END-1
} HW_IO_SWITCHHOOK_TYPE_T;
typedef HW_IO_SWITCHHOOK_TYPE_T	* PHW_IO_SWITCHHOOK_TYPE_T;
#define HW_IO_SWITCHHOOK_TYPE_SIZE	(HW_IO_SWITCHHOOK_TYPE_END-HW_IO_SWITCHHOOK_TYPE_FIRST)

typedef IN DWORD HW_IO_GET_SWITCHHOOK_TYPE_T ;									// HW_IO_SWITCHHOOK_TYPE_T
typedef HW_IO_GET_SWITCHHOOK_TYPE_T * PHW_IO_GET_SWITCHHOOK_TYPE_T ;

typedef IN DWORD HW_IO_SET_SWITCHHOOK_TYPE_T ;									// HW_IO_SWITCHHOOK_TYPE_T
typedef HW_IO_SET_SWITCHHOOK_TYPE_T * PHW_IO_SET_SWITCHHOOK_TYPE_T ;




//
//	HW_IO_GET_SWITCHHOOK_STATUS
//

typedef enum HW_IO_SWITCHHOOK_STATUS_S
{
	HW_IO_SWITCHHOOK_STATUS_FIRST			= 0,
	HW_IO_SWITCHHOOK_STATUS_START			= HW_IO_SWITCHHOOK_STATUS_FIRST-1,

	HW_IO_SWITCHHOOK_ONHOOK,
	HW_IO_SWITCHHOOK_OFFHOOK,

	HW_IO_SWITCHHOOK_STATUS_END,
	HW_IO_SWITCHHOOK_STATUS_LAST			= HW_IO_SWITCHHOOK_STATUS_END-1
} HW_IO_SWITCHHOOK_STATUS_T;
typedef HW_IO_SWITCHHOOK_STATUS_T	* PHW_IO_SWITCHHOOK_STATUS_T;
#define HW_IO_SWITCHHOOK_STATUS_SIZE	(HW_IO_SWITCHHOOK_STATUS_END-HW_IO_SWITCHHOOK_STATUS_FIRST)

typedef OUT DWORD HW_IO_GET_SWITCHHOOK_STATUS_T ;								// HW_IO_SWITCHHOOK_STATUS_T
typedef HW_IO_GET_SWITCHHOOK_STATUS_T * PHW_IO_GET_SWITCHHOOK_STATUST ;



//
//	HW_IO_SET_SOFTWARE_SWITCHHOOK
//

typedef IN DWORD HW_IO_SET_SOFTWARE_SWITCHHOOK_T ;								// HW_IO_SWITCHHOOK_STATUS_T


//
//	HW_IO_GET_HEAD_END
//	HW_IO_SET_HEAD_END
//

										// Vendor codes as listed in 
										// Annex D,	ANSI T1.413	- 1998
typedef enum _HW_IO_ADSL_VENDORS_E
{
	HW_IO_VENDOR_ID_NONE 					= 0x0000,	// Unknown vendor.
	HW_IO_VENDOR_ID_WESTELL 				= 0x0002,	// Westell Inc.
	HW_IO_VENDOR_ID_ECI 					= 0x0003,	// ECI Telecom
	HW_IO_VENDOR_ID_TEXAS_INSTRUMENTS		= 0x0004,	// Texas Instruments
	HW_IO_VENDOR_ID_INTEL					= 0x0005,	// Intel
	HW_IO_VENDOR_ID_AMATI					= 0x0006,	// Amati Communications	Corp.
	HW_IO_VENDOR_ID_GENERAL_DATA 			= 0x0007,	// General Data	Communication
	HW_IO_VENDOR_ID_LEVEL_ONE				= 0x0008,	// Level One Communications
	HW_IO_VENDOR_ID_CRYSTAL 				= 0x0009,	// Crystal Semiconductor
	HW_IO_VENDOR_ID_ATT_NETWORK 			= 0x000A,	// AT&T	- Network Systems
	HW_IO_VENDOR_ID_AWARE					= 0x000B,	// Aware, Inc.
	HW_IO_VENDOR_ID_BROOKTREE				= 0x000C,	// Brooktree
	HW_IO_VENDOR_ID_NEC 					= 0x000D,	// NEC
	HW_IO_VENDOR_ID_SAMSUNG					= 0x000E,	// Samsung
	HW_IO_VENDOR_ID_NORTHERN_TELECOM		= 0x000F,	// Northern	Telecom, Inc.
	HW_IO_VENDOR_ID_PAIRGAIN				= 0x0010,	// PairGain	Technologies
	HW_IO_VENDOR_ID_ATT_PAIRGAIN			= 0x0011,	// AT&T	- Paradyne		
	HW_IO_VENDOR_ID_ADTRAN					= 0x0012,	// Adtran				
	HW_IO_VENDOR_ID_INC						= 0x0013,	// INC					
	HW_IO_VENDOR_ID_ADC						= 0x0014,	// ADC Telecommunications
	HW_IO_VENDOR_ID_MOTOROLA				= 0x0015,	// Motorola
	HW_IO_VENDOR_ID_IBM						= 0x0016,	// IBM Corp.
	HW_IO_VENDOR_ID_NEWBRIDGE				= 0x0017,	// Newbridge Networks Corp.
	HW_IO_VENDOR_ID_DSC						= 0x0018,	// DSC
	HW_IO_VENDOR_ID_TELTREND				= 0x0019,	// TelTrend
	HW_IO_VENDOR_ID_EXAR					= 0x001A,	// Exar	Corp.
	HW_IO_VENDOR_ID_SIEMENS					= 0x001B,	// Siemens Stromberg-Carlson
	HW_IO_VENDOR_ID_ANALOG_DEVICES			= 0x001C,	// Analog Devices
	HW_IO_VENDOR_ID_NOKIA					= 0x001D,	// Nokia
	HW_IO_VENDOR_ID_ERICSSON				= 0x001E,	// Ericsson	Systems
	HW_IO_VENDOR_ID_TELLABS					= 0x001F,	// Tellabs Operations, Inc.
	HW_IO_VENDOR_ID_ORCKIT					= 0x0020,	// Orckit Communications, Inc
	HW_IO_VENDOR_ID_AWA						= 0x0021,	// AWA
	HW_IO_VENDOR_ID_ALCATEL					= 0x0022,	// Alcatel Network Systems In
	HW_IO_VENDOR_ID_NATIONAL				= 0x0023,	// National	Semiconductor Sys
	HW_IO_VENDOR_ID_ITALTEL					= 0x0024,	// Italtel
	HW_IO_VENDOR_ID_SAT						= 0x0025,	// SAT
	HW_IO_VENDOR_ID_FUJITSU					= 0x0026,	// Fujitsu Network Transmissi
	HW_IO_VENDOR_ID_MITEL					= 0x0027,	// MITEL
	HW_IO_VENDOR_ID_CONKLIN					= 0x0028,	// Conklin Instrument Corp.
	HW_IO_VENDOR_ID_DIAMOND					= 0x0029,	// Diamond Lane
	HW_IO_VENDOR_ID_CABLETRON				= 0x002A,	// Cabletron Systems, Inc.
	HW_IO_VENDOR_ID_DAVICOM					= 0x002B,	// Davicom Semiconductor, Inc
	HW_IO_VENDOR_ID_METALINK				= 0x002C,	// Metalink
	HW_IO_VENDOR_ID_PULSECOM				= 0x002D,	// Pulsecom
	HW_IO_VENDOR_ID_US_ROBOTICS				= 0x002E,	// US Robotics
	HW_IO_VENDOR_ID_AG_COMMUNICATIONS		= 0x002F,	// AG Communications Systems
	HW_IO_VENDOR_ID_CONEXANT				= 0x0030,	// Conexant	Systems	Inc.
	HW_IO_VENDOR_ID_HARRIS					= 0x0031,	// Harris
	HW_IO_VENDOR_ID_HAYES					= 0x0032,	// Hayes Microcomputer Produc
	HW_IO_VENDOR_ID_CO_OPTIC				= 0x0033,	// Co-optic
	HW_IO_VENDOR_ID_NETSPEED				= 0x0034,	// Netspeed	Inc.
	HW_IO_VENDOR_ID_3COM					= 0x0035,	// 3-Com
	HW_IO_VENDOR_ID_COPPER_MTN				= 0x0036,	// Copper Mountain Inc.
	HW_IO_VENDOR_ID_SILICON_AUTO 			= 0x0037,	// Silicon Automation Systems
	HW_IO_VENDOR_ID_ASCOM					= 0x0038,	// Ascom
	HW_IO_VENDOR_ID_GLOBESPAN				= 0x0039,	// Globespan Inc.
	HW_IO_VENDOR_ID_SGS_THOMPSON			= 0x003A,	// ST Microelectronics
	HW_IO_VENDOR_ID_COPPERCOM				= 0x003B,	// Coppercom"
	HW_IO_VENDOR_ID_COMPAQ					= 0x003C,	// Compaq Computer Corp."
	HW_IO_VENDOR_ID_INTEGRATED_TECH			= 0x003D,	// Integrated Technology Expr
	HW_IO_VENDOR_ID_BAY_NETWORKS			= 0x003E,	// Bay Networks, Inc."
	HW_IO_VENDOR_ID_NEXT_LEVEL				= 0x003F,	// Next	Level Communications"
	HW_IO_VENDOR_ID_MULTI_TECH				= 0x0040,	// Multitech Systems, Inc."
	HW_IO_VENDOR_ID_AMD						= 0x0041,	// AMD"
	HW_IO_VENDOR_ID_SUMITOMO				= 0x0042,	// Sumitomo Electric"
	HW_IO_VENDOR_ID_PHILLIPS_M_N			= 0x0043,	// Phillips M&N Electronics"
	HW_IO_VENDOR_ID_EFFICIENT_NETWORKS		= 0x0044,	// Efficient Networks, Inc."
	HW_IO_VENDOR_ID_INTERSPEED				= 0x0045,	// Interspeed"
	HW_IO_VENDOR_ID_CISCO					= 0x0046,	// Cisco Systems"
	HW_IO_VENDOR_ID_TOLLGRADE				= 0x0047,	// Tollgrade Communciations, 
	HW_IO_VENDOR_ID_CAYMAN					= 0x0048,	// Cayman Systems"
	HW_IO_VENDOR_ID_FLOWPOINT				= 0x0049,	// FlowPoint Corp."
	HW_IO_VENDOR_ID_ICCOM					= 0x004A,	// I.C.COM"
	HW_IO_VENDOR_ID_MATSUSHITA	 			= 0x004B,	// Matsushita"
	HW_IO_VENDOR_ID_SIEMENS_SEMICO			= 0x004C,	// Siemens Semiconductor"
	HW_IO_VENDOR_ID_DIGITAL_LINK			= 0x004D,	// Digital Link"
	HW_IO_VENDOR_ID_DIGITEL					= 0x004E,	// Digitel"
	HW_IO_VENDOR_ID_ALCATEL_MICRO			= 0x004F,	// Alcatel Microelectronics"
	HW_IO_VENDOR_ID_CENTILLIUM				= 0x0050,	// Centillium Corp."
	HW_IO_VENDOR_ID_APPLIED_DIGITAL			= 0x0051,	// Applied Digital Access, In
	HW_IO_VENDOR_ID_SMART_LINK   			= 0x0052,	// Smart Link, Ltd."
	HW_IO_VENDOR_ID_MEDIALINCS				= 0xB6DB,	// Medialincs (Korean)
	HW_IO_VENDOR_ID_UNKNOWN	   				= 0xFFFF	// Unknown Vendor
} HW_IO_ADSL_VENDORS_E;



typedef OUT	DWORD				HW_IO_GET_HEAD_END_T;
typedef HW_IO_GET_HEAD_END_T		* PHW_IO_GET_HEAD_END_T;

typedef IN	DWORD				HW_IO_SET_HEAD_END_T;
typedef HW_IO_SET_HEAD_END_T		* PHW_IO_SET_HEAD_END_T;




//
//	HW_IO_GET_HEAD_END_ENV
//	HW_IO_SET_HEAD_END_ENV
//


typedef enum _HW_IO_HEAD_END_ENVIRONMENT_E
{
	HW_IO_HEAD_END_ENV_NON_SPECIFIC = 0,	// Non-Specific"
	HW_IO_HEAD_END_ENV_NO_LINE_DRIVER		// No Line Driver (BNA)
} HW_IO_HEAD_END_ENVIRONMENT_E;


typedef OUT	DWORD				HW_IO_GET_HEAD_END_ENV_T;
typedef HW_IO_GET_HEAD_END_ENV_T		* PHW_IO_GET_HEAD_END_ENV_T;

typedef IN	DWORD				HW_IO_SET_HEAD_END_ENV_T;
typedef HW_IO_SET_HEAD_END_ENV_T		* PHW_IO_SET_HEAD_END_ENV_T;


//
// HW_IO_EOC_COMMAND
//

typedef struct HW_IO_EOC_COMMAND_S
{
	DWORD		EOCCommandType;
	DWORD		EOCXcvrNumber;
	DWORD		EOCState;
	DWORD		EOCRegister;
	DWORD		EOCRegSize;
	BYTE		EOCData;				

} HW_IO_EOC_COMMAND_T,
  * PHW_IO_EOC_COMMAND_T;

typedef enum _HW_IO_EOC_CODES_E
{
	
	EOC_SUSPEND_POLLING,
	EOC_SET_HOLD_STATE,
	EOC_RETURN_NORMAL_STATE,
	EOC_SEND_SELF_TEST,
	EOC_REQUEST_CORRUPT_CRC_START,
	EOC_REQUEST_CORRUPT_CRC_STOP,
	EOC_NOTIFY_CORRUPT_CRC_START,
	EOC_NOTIFY_CORRUPT_CRC_STOP,
	EOC_READ_REG,
	EOC_WRITE_REG,
	EOC_POLL_READ_REG,
	EOC_POLL_DYING_GASP

} HW_IO_EOC_CODES_E;

//
//	HW_IO_IS_LINE_STARTABLE
//

typedef OUT DWORD 				HW_IO_LINE_STARTABLE_T;
typedef HW_IO_LINE_STARTABLE_T		* PHW_IO_LINE_STARTABLE_T;


//
//	HW_IO_AFE_EEPROM
//	HW_IO_AFE_REV	
//
typedef OUT	DWORD				HW_IO_GET_AFE_EEPROM_T;
typedef OUT	DWORD				HW_IO_GET_AFE_REV_T;

//
//	HW_IO_GET_SERIAL_DATA_PATH
//	HW_IO_SET_SERIAL_DATA_PATH
//

typedef IN OUT	DWORD			HW_IO_SERIAL_DATA_PATH_T;



#endif		//#ifndef _HWIODMT_H_

