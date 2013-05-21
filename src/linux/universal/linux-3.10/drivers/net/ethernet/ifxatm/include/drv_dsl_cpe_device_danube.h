/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_DEVICE_DANUBE_H
#define _DRV_DSL_CPE_DEVICE_DANUBE_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_cmv_danube.h"

#if defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
   /** DANUBE driver Simulator interface header file*/
   #include "drv_dsl_cpe_simulator_danube.h"
#else
/* Include for the low level driver interface header file */
#include "mei/ifxmips_mei_interface.h"
#endif /* defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/

#define DSL_MAX_LINE_NUMBER 1
#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   /** The length of showtome event logging buffer in the 16-bit words */
   #define DSL_DEV_SHOWTIME_EVENT_LOGGING_BUFFER_LENGTH 120
#endif

/** Timeout setting for far end status in mili seconds */
#define DSL_DEV_TIMEOUT_FE_STATUS               12000

/** Timeout setting for HDLC CEOC data, ms */
#define DSL_DEV_TIMEOUT_HDLC_CEOC               5000

#define DSL_DANUBE_FW_MAJOR_NUMBER      (2)
#define DSL_AMAZON_SE_FW_MAJOR_NUMBER   (3)
#define DSL_AR9_FW_MAJOR_NUMBER         (4)

/** \addtogroup DRV_DSL_DEVICE
 @{ */


/* The following defines specify firmware versions that are needed to be checked
   at runtime to get information if a dedicated feature is available from
   firmware point of view.
   
   #define DSL_MIN_FW_VERSION_xxx       -1, 3, -1, -1
                                         |  |   |   |
                                         |  |   |   internal version
                                         |  |   external version
                                         |  Minor version (feature step)
                                         Major version (device type)
   
   In case of using '-1' value the according firmware version part is don't care.
*/
#define DSL_MIN_ANNEX_A_FW_VERSION_PTM      -1, 1, -1, -1
#define DSL_MIN_ANNEX_B_FW_VERSION_PTM      -1, 4, -1, -1

#define DSL_MIN_ANNEX_A_FW_VERSION_STAT24   -1, 4, 4, -1
#define DSL_MIN_ANNEX_B_FW_VERSION_STAT24   -1, 4, 1, 2

#define DSL_MIN_FW_VERSION_CB       -1, 3, -1, -1
#define DSL_MIN_FW_VERSION_FTTXPOTS -1, 2, -1, -1
#define DSL_MIN_FW_VERSION_REBOOT   -1, 3, -1, -1

#define DSL_CPE_CMV_BYTE_SIZE   1 << 14
#define DSL_CPE_CMV_DWORD_SIZE  2 << 14
#define DSL_CPE_CMV_WORD_SIZE   0 << 14


#define DSL_ACT_FAILURE_MASK_GET(ActFailure, PrevFailure, nBitMask, nActFailureMask) \
   if ((ActFailure & nBitMask) != (PrevFailure & nBitMask)) \
   { \
      nActFailureMask |= nBitMask; \
   } 


/**
   Defines the return types of version processing functions
   \ref DSL_DEV_CheckFirmwareVersion
   \ref DSL_DEV_CheckDeviceVersion
*/
typedef enum
{
   /** Version processing is not possible due to an error */
   DSL_VERSION_ERROR = 0,
   /** Version is not compared because of different common firmware types
       Currently this indicates a mismatch of the ApplicationType (Annex) only */
   DSL_VERSION_MISMATCH = 1,
   /** The version actually used is smaller than the version to check */
   DSL_VERSION_SMALLER = 2,
   /** The version actually used is equal to the version to check */
   DSL_VERSION_EQUAL = 3,
   /** The version actually used is bigger than the version to check */
   DSL_VERSION_BIGGER = 4,
   /** Delimiter only */
   DSL_VERSION_LAST = 5
} DSL_DEV_VersionCheck_t;

/**
   Includes version information for used firmware.
   The complete version information is extractzed from CMV INFO 54 0/1
*/
typedef struct
{
   /**
   Defines whether the included version information is valid or not */
   DSL_boolean_t bValid;
   /*
   Information source
      CMV INFO 54 0, Bit 0..7
   Meaning
      - 1: Amazon
      - 2: Danube
      - 3: Amazon-SE
      - 4: AR9 */
   DSL_uint8_t nMajorVersion;
   /*
   Information source
      CMV INFO 54 0, Bit 8..15
   Meaning
      Defines the feature set (FS) which indicates the main feature release
      versions */
   DSL_uint8_t nMinorVersion;
   /*
   Information source
      CMV INFO 54 1, Bit 14..15
   Meaning
      - 0: Released
      - 1: Internal version */
   DSL_uint8_t nReleaseState;
   /*
   Information source
      CMV INFO 54 1, Bit 8..13
   Meaning
      - 1: Annex A firmware version
      - 2: Annex B firmware version */
   DSL_uint8_t nApplicationType;
   /*
   Information source
      CMV INFO 54 1, Bit 4..7
   Meaning
      Major number that will be increased for consecutive firmware version
      during development within one feature set. */
   DSL_uint8_t nExternalVersion;
   /*
   Information source
      CMV INFO 54 1, Bit 0..3
   Meaning
      Minor number that will be increased for consecutive firmware version
      during development within one feature set. */
   DSL_uint8_t nInternalVersion;
} DSL_FwVersion_t;

/**
   Includes version information for used chip.
*/
typedef struct
{
   /**
   Defines whether the included version information is valid or not */
   DSL_boolean_t bValid;
   /**
   Major version of used device */
   DSL_uint8_t nMajorVersion;
   /**
   Minor version of used device */
   DSL_uint8_t nMinorVersion;
} DSL_ChipVersion_t;

/**
   Includes version information for used Low Level Mei BSP Driver.
*/
typedef struct
{
   /**
   Defines whether the included version information is valid or not */
   DSL_boolean_t bValid;
   /**
   Major version of used low level driver */
   DSL_uint8_t nMajorVersion;
   /**
   Minor version of used low level driver */
   DSL_uint8_t nMinorVersion;
   /**
   Build revision of used low level driver */
   DSL_uint8_t nBuild;
} DSL_MeiSwVersion_t;

/**
   Version information structure that includes version information about the
   firmware, the device and the low level driver.
   The parts will be updated at different point
   - firmware version after successful firmware download
   - device and low level driver version at open of the DSL CPE API
*/
typedef struct
{
   /**
   Detailed firmware version information */
   DSL_FwVersion_t fwVersion;
   /**
   Detailed chip version information */
   DSL_ChipVersion_t chipVersion;
   /**
   Detailed MEI BSP Low level driver version information */
   DSL_MeiSwVersion_t meiSwVersion;
} DSL_Version_t;

typedef struct
{
   /** Saved counters */
   DSL_uint16_t nErrS_L_count[4];

#if defined(INCLUDE_DSL_PM)
   /** Line inititalization counters */
   DSL_PM_LineInitData_t lineInitCounters;
#endif

   /** internal adsl mode values for Danube */
   DSL_uint16_t nAdslMode, nAdslMode1;
   DSL_boolean_t bAdsl1;
   DSL_boolean_t bAdsl2p;
#ifdef INCLUDE_DSL_CEOC
   /** flag will be set if CEOC data is received */
   DSL_boolean_t bCeocRx;
   DSL_uint16_t nCeocReadIndex;
#endif
   DSL_SystemInterfaceConfigData_t sysCIF;
   DSL_boolean_t bSystemIfStatusValid;
   DSL_SystemInterfaceConfigData_t sysCIFSts;
   DSL_int16_t nInitSnrMarginDs;
   DSL_Version_t version;
} DSL_DEV_Data_t;

/*
   Structure used to build table which includes a list of Danube message IDs
*/
typedef struct
{
   /**
   Goup ID of message(s) */
   DSL_int_t nGroupId;
   /**
   16 Bit address (start of range) */
   DSL_uint16_t nAdrFirst;
   /**
   16 Bit address (stop of range) */
   DSL_uint16_t nAdrLast;
} DSL_DEV_MsgIdRange_t;

/** Dummy types. Not used for the DANUBE device*/
typedef struct
{
   DSL_uint8_t dummy;
} DSL_DEV_Clockout_t;

typedef struct
{
   DSL_uint8_t dummy;
} DSL_DEV_LineMode_t;

typedef struct
{
   DSL_uint8_t dummy;
} DSL_DEV_RxTxGainSelection_t;

typedef struct
{
   DSL_uint8_t firmwareCodeStart;
   DSL_uint8_t firmwareCodeStop;
} DSL_DANUBE_FirmwareFailureCodeRange;

typedef struct
{
   /* FW internal Failure code Range*/
   DSL_DANUBE_FirmwareFailureCodeRange codeRange;
   /* Corresponding G997.1 failure code*/
   DSL_G997_LineInit_t g997Code;
} DSL_DANUBE_FailReasonTable_t;

#define DSL_DANUBE_REGISTER_FAILURE_TABLE_ENTRY(first, last, mapped) \
   {{first, last}, mapped}

DSL_void_t DSL_DRV_DANUBE_CmvPrepare
(
   DSL_Context_t *pContext,
   DSL_uint8_t nOpcode,
   DSL_uint8_t nGroup,
   DSL_uint16_t nAddress,
   DSL_uint16_t nIndex,
   DSL_int_t nSize,
   DSL_uint16_t * pData,
   DSL_uint16_t *pMsg
);

DSL_Error_t DSL_DRV_DANUBE_CmvSend
(
   DSL_Context_t *pContext,
   DSL_uint16_t *pTxMsg,
   DSL_boolean_t bReply,
   DSL_uint16_t *pRxMsg
);

DSL_Error_t DSL_DRV_DANUBE_CmvRead
(
   DSL_Context_t *pContext,
   DSL_uint8_t nGroup,
   DSL_uint16_t nAddress,
   DSL_uint16_t nIndex,
   DSL_int_t nSize,
   DSL_uint16_t * pData
);

DSL_Error_t DSL_DRV_DANUBE_CmvWrite
(
   DSL_Context_t *pContext,
   DSL_uint8_t nGroup,
   DSL_uint16_t nAddress,
   DSL_uint16_t nIndex,
   DSL_int_t nSize,
   DSL_uint16_t * pData
);

/*
   Returns the current latency path configuration.

   \param pContext Pointer to DSL CPE API context structure, [I]
   \param nChannel Channel for which the configuration has to be returned, [I]
   \param nLPath   Returns the current configuration of the latency path, [O]

   \return error code

   \ingroup Internal
*/
DSL_Error_t DSL_DRV_DANUBE_ActLatencyGet(
   DSL_Context_t *pContext,
   DSL_uint16_t nChannel,
   DSL_AccessDir_t nDirection,
   DSL_LatencyPath_t *nLPath);

DSL_Error_t DSL_DRV_DANUBE_RateAdaptationSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_AccessDir_t nDirection,   
   DSL_OUT DSL_G997_RateAdaptationConfigData_t *pData);

DSL_Error_t DSL_DRV_DANUBE_TotalCountersWrite
(
   DSL_Context_t *pContext
);

DSL_Error_t DSL_DRV_DANUBE_ChannelStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_uint16_t nChannel,
   DSL_IN DSL_AccessDir_t nDirection,
   DSL_IN_OUT DSL_G997_ChannelStatusData_t *pData
);

/*
 * Check the L3 status
 * This function get the CPE Power Management Mode status
 * \return see DSL_G997_PowerManagement_t for possible values
 *    0: L0 Mode
 *    2: L2 Mode
 *    3: L3 Mode
 * \ingroup Internal
 */
DSL_Error_t DSL_DRV_DANUBE_L3StatusGet(
   DSL_Context_t *pContext,
   DSL_G997_PowerManagement_t *pnPowerMode
);

/**
   Hdlc mutex control

   \param   pContext      pointer to the DSL context
   \param   bLock         whether device should be locked or not
   \ingroup Internal
 */
DSL_Error_t DSL_DRV_DANUBE_HdlcMutexControl(
   DSL_Context_t *pContext,
   DSL_boolean_t bLock);

/**
   Send hdlc packets

   \param   pContext      pointer to the DSL context
   \param   bDeviceLock   whether device should be locked or not
   \param   pHdlcPkt      pointer to hdlc packet
   \param   nHdlcPktLen   the number of bytes to send
   \return  success or failure.
   \ingroup Internal
 */
DSL_Error_t DSL_DRV_DANUBE_HdlcWrite
(
   DSL_Context_t *pContext,
   DSL_boolean_t bDeviceLock,
   DSL_uint8_t *pHdlcPkt,
   DSL_int_t nHdlcPktLen
);

/**
   Read the hdlc packets

   \param   pContext       pointer to the DSL context
   \param   bDeviceLock    whether device should be locked or not
   \param   pHdlcPkt       pointer to hdlc packet
   \param   nMaxHdlcPktLen The maximum number of bytes to read
   \param   pnRead         pointer to a variable to save the number of bytes
                           which was read to
   \return  success or failure.
   \ingroup Internal
*/
DSL_Error_t DSL_DRV_DANUBE_HdlcRead
(
   DSL_Context_t *pContext,
   DSL_boolean_t bDeviceLock,
   DSL_uint8_t *pHdlcPkt,
   DSL_int_t nMaxHdlcPktLen,
   DSL_int_t *pnRead
);

#define DSL_ME_HDLC_IDLE              0x0
#define DSL_ME_HDLC_MSG_QUEUED        0x2
#define DSL_ME_HDLC_MSG_SENT          0x3
#define DSL_ME_HDLC_RESP_RCVD         0x4
#define DSL_ME_HDLC_RESP_TIMEOUT      0x5
#define DSL_ME_HDLC_MSG_NOT_SUPPORTED 0x7
#define DSL_ME_HDLC_UNRESOLVED 1
#define DSL_ME_HDLC_RESOLVED 2

/**
   This function writes inventory info to the device
*/
DSL_Error_t DSL_DRV_DANUBE_LineInventoryNeWrite(
   DSL_IN DSL_Context_t *pContext
);

/**
   This function reads inventory info from the device
*/
DSL_Error_t DSL_DRV_DANUBE_LineInventoryNeRead(
   DSL_IN DSL_Context_t *pContext
);

/**
   This function reads inventory info from the device
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DANUBE_LineInventoryFeRead(
   DSL_IN DSL_Context_t *pContext
);
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

/**
   This function updates channel status information within DSL Context
*/
DSL_Error_t DSL_DRV_DANUBE_ChannelStatusUpdate(
   DSL_IN DSL_Context_t *pContext);

/* G997 block */
/**
   This function stores DELT data to the DSL Context structure for futher readings
*/
DSL_Error_t DSL_DRV_DANUBE_G997_LoopDiagnosticCompleted
(
   DSL_Context_t *pContext
);

/**
   This function retreives the number of used tones in the current ADSL mode
*/
DSL_Error_t DSL_DRV_DANUBE_UsedTonesNumberGet
(
   DSL_Context_t *pContext,
   const DSL_AccessDir_t nDirection,
   DSL_uint16_t *pTonesNum
);

DSL_Error_t DSL_DRV_DANUBE_DualLatencyStatusGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_boolean_t *pDualLatencyActive);

DSL_Error_t DSL_DRV_DANUBE_AttAggregateDataRateGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,   
   DSL_uint32_t *pAttAggregateDataRate);

DSL_Error_t DSL_DRV_DANUBE_G997_LineStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatus_t *pData);

DSL_Error_t DSL_DRV_DANUBE_G997_LineTrainingStatusUpdate(
   DSL_IN DSL_Context_t *pContext);

DSL_Error_t DSL_DRV_DANUBE_G997_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN const DSL_uint8_t nChannel,
   DSL_OUT DSL_G997_FramingParameterStatusData_t *pData);

#if defined(INCLUDE_DSL_FRAMING_PARAMETERS) || defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)
DSL_Error_t DSL_DRV_DANUBE_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_FramingParameterStatus_t *pData);
#endif /* defined(INCLUDE_DSL_FRAMING_PARAMETERS) || defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS)*/

#if defined(INCLUDE_DSL_CEOC)
/* CEOC internal functions */
/** CEOC module low level driver events handler */
DSL_Error_t DSL_CEOC_EventCB
(
   DSL_Context_t *pContext
);
#endif

/** @} DRV_DSL_DEVICE */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_DANUBE_H */
