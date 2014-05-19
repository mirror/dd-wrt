/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_INTERN_H
#define _DRV_DSL_CPE_INTERN_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_autoboot.h"

#include "drv_dsl_cpe_fifo.h"

/** \file
   This file specifies the internal functions that are used for common
   implementation of the ioctl interface.
   It is intendet to be used within the DSL CPE_API driver ONLY.
*/

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

#if defined(INCLUDE_DSL_PM)
   #include "drv_dsl_cpe_api_pm.h"
   #include "drv_dsl_cpe_intern_pm.h"
   #include "drv_dsl_cpe_device_pm.h"
#endif

#if defined(INCLUDE_DSL_ADSL_MIB)
   #include "drv_dsl_cpe_intern_mib.h"
#endif

#ifdef INCLUDE_DSL_CPE_API_VINAX
   #include "drv_dsl_cpe_intern_sar.h"
#endif

/*
   Automatic generation of wrapper function for cli interfcae is not working
   correctly at the moment. Therefore some functions are excluded from automatic
   generation at the moment using the following preprocessor definition.
*/
#ifdef SWIG
#define SWIG_TMP
#endif

/**
   This structure is intended to save upper-software instance data
*/
typedef struct DSL_OpenContext
{
   /** member for list organization */
   struct DSL_OpenContext *pNext;
   /** Device context pointer */
   DSL_devCtx_t *pDevCtx;
   /** Event mechanism */
   /** Event access mutex */
   DSL_DRV_Mutex_t eventMutex;
   /** Wait queue for the event mechanism */
   DSL_DRV_WaitQueue_t eventWaitQueue;
   /** FIFO for the event mechanism */
   DSL_FIFO *eventFifo;
   /** FIFO buffer */
   DSL_uint8_t *eventFifoBuf;
   /** Enable/Disable event handling*/
   DSL_boolean_t bEventActivation;
   /** Configured event mask */
   DSL_uint32_t nEventMask;
   /** Flag to signal the FIFO overflow condition*/
   DSL_boolean_t bFifoFull;
   /**
   Specifies the resource activation mask. */
   DSL_BF_ResourceActivationType_t nResourceActivationMask;
   /** CEOC instance specific data */
#ifdef INCLUDE_DSL_CEOC
   /** CEOC FIFO for the SNMP protocol*/
   DSL_FIFO *rxSnmpFifo;
   /** Rx FIFO buffer for the SNMP protocol*/
   DSL_uint8_t *rxSnmpFifoBuf;
   /** Event access mutex */
   DSL_DRV_Mutex_t rxSnmpFifoMutex;
#endif /** #ifdef INCLUDE_DSL_CEOC*/
} DSL_OpenContext_t;

#include "drv_dsl_cpe_intern_g997.h"

#if defined(INCLUDE_DSL_CEOC)
   #include "drv_dsl_cpe_intern_ceoc.h"
   #include "drv_dsl_cpe_device_ceoc.h"
#endif /** #if defined(INCLUDE_DSL_CEOC)*/

/**
   Defines all possible xDSL transmission modes
*/
typedef enum
{
   /**
   Zero has been chosen to indicate 'not initialized' after memset of context
   structure after startup for example */
   DSL_XDSLMODE_UNKNOWN = 0,
   /**
   ITU-T G.992.1, ADSL1 */
   DSL_XDSLMODE_G_992_1,
   /**
   ANSI 1.413, (ADSL1 only) */
   DSL_XDSLMODE_T1_413,
   /**
   ITU-T G.992.2, ADSL1 lite */
   DSL_XDSLMODE_G_992_2,
   /**
   ITU-T G.992.3, ADSL2 */
   DSL_XDSLMODE_G_992_3,
   /**
   ITU-T G.992.4, ADSL2 lite */
   DSL_XDSLMODE_G_992_4,
   /**
   ITU-T G.992.5, ADSL2+ */
   DSL_XDSLMODE_G_992_5,
   /**
   ITU-T G.993.1, VDSL2 */
   DSL_XDSLMODE_G_993_1,
   DSL_XDSLMODE_LAST
} DSL_xDslMode_t;

/**
   Defines all possible annex types
*/
typedef enum
{
   /**
   Zero has been chosen to indicate 'not initialized' after memset of context
   structure after startup for example */
   DSL_ANNEX_UNKNOWN = 0,
   /**
   Annex A is used for ADSL and VDSL */
   DSL_ANNEX_A = 1,
   /**
   Annex B is used for ADSL and VDSL */
   DSL_ANNEX_B = 2,
   /**
   Annex C is used for VDSL only */
   DSL_ANNEX_C = 3,
   /**
   Annex I is used for ADSL only */
   DSL_ANNEX_I = 4,
   /**
   Annex I is used for ADSL only */
   DSL_ANNEX_J = 5,
   /**
   Annex I is used for ADSL only */
   DSL_ANNEX_L = 6,
   /**
   Annex I is used for ADSL only */
   DSL_ANNEX_M = 7,
   DSL_ANNEX_LAST
} DSL_AnnexType_t;

typedef enum
{
   /* Turn Data LED OFF*/
   DSL_DATA_LED_OFF = 0,
   /* Turn Data LED ON*/
   DSL_DATA_LED_ON  = 1,
   /* Turn Data LED to BLINK. Data LED blinking is only possible during
      showtime.*/
   DAL_DATA_LED_BLINK = 2,
   /* Stop Data LED blink*/
   DAL_DATA_LED_STOP_BLINK = 3
} DSL_DataLedBehavior_t;

typedef struct
{
   /* Data LED behavior.*/
   DSL_DataLedBehavior_t nLedBehavior;
   /* Timeout value expressed in [ms].
      Optional parameter to specify Data LED timeout before consecutive
      Data LED blink triggers. Used only in conjunction with DAL_DATA_LED_BLINK.
      Valid range is: 1...10000 with 1 ms steps,
      Default value is 1000 ms*/
   DSL_uint32_t nBlinkTimeout;
} DSL_DataLedSimControlData_t;

/**
   The driver context contains global information.
*/
struct DSL_Context
{
   /** Was DSL CPE API initialized? */
   DSL_boolean_t bInitComplete;
   /** Back pointer to the device context structure */
   DSL_devCtx_t *pDevCtx;
   /** Device mutex */
   DSL_DRV_Mutex_t  bspMutex;
   /** Data access mutex */
   DSL_DRV_Mutex_t dataMutex;
   /** Initialization mutex */
   DSL_DRV_Mutex_t initMutex;
#if defined(INCLUDE_DSL_CPE_API_DANUBE)
   /** HDLC processing mutex */
   DSL_DRV_Mutex_t hdlcMutex;
#endif /* defined(INCLUDE_DSL_CPE_API_DANUBE)*/
   /** Last error code */
   DSL_Error_t nErrNo;
   /** Pointer to the stored FW binary*/
   DSL_uint8_t *pFirmware;
   /** Size of the stored firmware binary*/
   DSL_uint32_t nFirmwareSize;
   /** Pointer to the 2'nd stored FW binary*/
   DSL_uint8_t *pFirmware2;
   /** Size of the 2nd stored firmware binary*/
   DSL_uint32_t nFirmwareSize2;
   /** Autoboot thread control structure */
   DSL_DRV_ThreadCtrl_t AutobootControl;
#if defined(INCLUDE_DSL_CPE_API_DANUBE)
   DSL_boolean_t bMeiReboot;
#endif /* defined(INCLUDE_DSL_CPE_API_DANUBE)*/
   /** External Trigger to reboot autoboot handling, pass through the
       EXCEPTION handling*/
   DSL_boolean_t bAutobootReboot;
   /** External Trigger to restart autoboot handling*/
   DSL_boolean_t bAutobootRestart;
   /** External Trigger to resume autoboot handling*/
   DSL_boolean_t bAutobootContinue;
   /** Autoboot queue */
   DSL_DRV_Event_t autobootEvent;
   /** Autoboot thread activity flag */
   DSL_boolean_t bAutobootThreadStarted;
   /** Autoboot thread shutdown flag */
   DSL_boolean_t bAutobootThreadShutdown;
   /** Autoboot thread poll time */
   DSL_uint32_t nAutobootPollTime;
   /** Autoboot state */
   DSL_Autoboot_State_t nAutobootState;
   /** Autoboot status*/
   DSL_AutobootStatusData_t nAutobootStatus;
   /** Autoboot configuration data*/
   DSL_AutobootConfigData_t nAutobootConfig;
   /** Autoboot stop request */
   DSL_boolean_t bAutobootStopPending;
   /** Firmware load request flag */
   DSL_boolean_t bAutobootFwLoadPending;
   /** Autoboot pending start flag */
   DSL_boolean_t bAutobootStartPending;
   /** Autoboot start time  [msec] */
   DSL_uint32_t autobootStartTime;
   /** Autoboot timeout in the current state [sec] */
   DSL_int_t nAutobootTimeoutLimit;
   /** Autoboot has entered the showtime*/
   DSL_boolean_t bGotShowtime;
   /** Autoboot has got a FULL_INIT state*/
   DSL_boolean_t bGotFullInit;
   /** Autoboot has got a shortinit response */
   DSL_boolean_t bGotShortInitResponse;
   /** Autoboot startup mode */
   DSL_AutobootCtrlSet_t nAutobootStartupMode;
   /* Flag to inform the Autoboot that the FW request has been handeld*/
   DSL_boolean_t bFwRequestHandled;
   /* Flag to inform the Autoboot that the FW was reloaded*/
   DSL_boolean_t bFwReLoaded;
   /** L3 Power state forced flag */
   DSL_boolean_t bPowerManagementL3Forced;

   /** Firmware unavailable duration */
   DSL_uint32_t nFwUnavailableTime;

#if defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))
   /** Timeout lists context includes information of a specified
       timeout list. */
   DSL_TimeoutContext_t TimeoutListsContext;
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))*/
#ifdef INCLUDE_DSL_CPE_API_VINAX
   /** EAPS timeout ID*/
   DSL_uint32_t nEapsTimeoutId;
#endif /** INCLUDE_DSL_CPE_API_VINAX*/
   /** Flag that indicates about presence of firmware load handler */
   DSL_boolean_t bFirmwareEventAssigned;
#ifdef INCLUDE_DSL_CPE_API_DANUBE
   /**
      Configuration data for interoperability issues. */
   DSL_InteropFeatureConfigData_t interopFeatureConfigData;
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/
   /** Line features configuration data. for the UPSTREAM and DOWNSTREM direction*/
   DSL_LineFeatureData_t lineFeatureDataCfg[DSL_ACCESSDIR_LAST];
   /** Line features Status data. for the UPSTREAM and DOWNSTREM direction*/
   DSL_LineFeatureData_t lineFeatureDataSts[DSL_ACCESSDIR_LAST];

   /** Line state */
   DSL_LineStateValue_t nLineState;
   /** Whether connection with a far end is established or not */
   /** Disconnection time */
   DSL_DRV_TimeVal_t disconnectTime;
   /** Showtime state reached time */
   DSL_DRV_TimeVal_t showtimeReachedTime;
   /** Showtime indication flag*/
   DSL_boolean_t bShowtimeReached;

   /** Test Mode Control*/
   DSL_TestModeControlSet_t nTestModeControl;

#ifdef INCLUDE_DSL_DELT
   /** Line configuration */
   DSL_boolean_t bLoopDiagnosticsCompleted;
   /** The loop diagnostic auto mode counter */
   DSL_uint8_t nLoopAutoCount;
#endif /* INCLUDE_DSL_DELT*/
   /** The minimal value of SNRM. Autoboot will be restarted in case of actual
       SNRM value becomes less than this minimal SNRM value */
   DSL_int16_t nMinSnrmDs;
#ifdef INCLUDE_DSL_CPE_API_DANUBE
   /** Far-End Line Status Backup Data*/
   DSL_G997_LineStatusBackupData_t lineStatusFe;
#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
   /** Last Exception Codes*/
   DSL_DBG_LastExceptionCodesData_t LastExceptionCodes;
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/
   /** Near end inventory information */
   DSL_G997_LineInventoryData_t lineInventoryNe;
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   /** Far end inventory information */
   DSL_G997_LineInventoryData_t lineInventoryFe;
   /** Far-end inventory availability indication flag*/
   DSL_boolean_t bFeLineInventoryValid;
   /** Far-end inventory incomplete indication flag*/
   DSL_boolean_t bFeLineInventoryIncomplete;
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
   /**
   Auxiliary inventory information according to
   ITU G.993.2 chapter 11.2.3.6 */
   DSL_AuxInventoryNe_t auxInventoryNe;

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
   /**
      Far-end Auxiliary inventory information
   */
   DSL_AuxLineInventoryData_t auxInventoryFe;
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */
   /**
      G997 Rate Adaptation Mode*/
   DSL_G997_RA_MODE_t rateAdaptationMode[DSL_ACCESSDIR_LAST];

   /** Line activation configuration. It will be applied during next
       link activation */
   DSL_G997_LineActivateData_t lineActivateConfig;

   /** Power management status of link. Updated from firmware
       and autoboot thread state */
   DSL_G997_PowerManagementStatusData_t powerMgmtStatus;

   /** Line Init Status updated from firmware
       during training */
   DSL_G997_LineInitStatusData_t lineInitStatus;
   /** Number of link initialization retries */
   DSL_uint32_t lineInitRetryCount;

   /** Channel status */
   DSL_uint32_t ActualInterleaveDelayUs[DSL_CHANNELS_PER_LINE];
   DSL_uint32_t ActualInterleaveDelayDs[DSL_CHANNELS_PER_LINE];
   DSL_uint8_t ActualImpulseNoiseProtectionUs[DSL_CHANNELS_PER_LINE];
   DSL_uint8_t ActualImpulseNoiseProtectionDs[DSL_CHANNELS_PER_LINE];

   /** Actual Data Rate per direction, per channel */
   DSL_uint32_t nChannelActualDataRate[DSL_ACCESSDIR_LAST][DSL_CHANNELS_PER_LINE];
   DSL_uint32_t nChannelActualDataRatePrev[DSL_ACCESSDIR_LAST][DSL_CHANNELS_PER_LINE];

   /** Previous Data Rate per direction, per channel */
   DSL_uint32_t nChannelPreviousDataRate[DSL_ACCESSDIR_LAST][DSL_CHANNELS_PER_LINE];

   /** Previous Data Rate valid flag */
   DSL_boolean_t bPrevDataRateValid;
   /** Current Latency path for channel */
   DSL_LatencyPath_t nLPath[DSL_ACCESSDIR_LAST][DSL_CHANNELS_PER_LINE];
   /** Upstream Channel Data Rate thresholds*/
   DSL_G997_ChannelDataRateThresholdData_t channelDataRateThreshold[DSL_ACCESSDIR_LAST];
   /** Line Options Configuration Data*/
   DSL_int32_t lineOptionsConfig[DSL_OPT_LAST];
   /** G997 block */
   /** XTSE configuration data */
   DSL_uint8_t xtseCfg[DSL_G997_NUM_XTSE_OCTETS];
   /** XTSE configuration status  */
   DSL_uint8_t xtseCurr[DSL_G997_NUM_XTSE_OCTETS];
   /** XTSE theoretically possible configuration data*/
   DSL_uint8_t xtsePoss[DSL_G997_NUM_XTSE_OCTETS];

#ifdef INCLUDE_DSL_DELT
   /** Pointer to DELT data storage */
   DSL_G997_DeltData_t *DELT;
   #ifdef INCLUDE_DSL_CPE_API_VINAX
   DSL_G997_DeltShowtimeData_t *DELT_SHOWTIME;
   #endif
#endif /* #ifdef INCLUDE_DSL_DELT*/
   /** Internal used decoded values derived from xSTE octets */
   /** Current mode */
   DSL_xDslMode_t nXDslMode;
   /** Current Annex type */
   DSL_AnnexType_t nAnnexType;

   /** LED */
/*#if defined(INCLUDE_ADSL_LED) && defined(INCLUDE_DSL_CPE_API_AMAZON_SE)*/
#if defined(INCLUDE_ADSL_LED)
   /** Led polling queue */
   DSL_DRV_Event_t ledPollingEvent;
   /** This flag indicates that Status Led should be turned on */
   DSL_boolean_t bLedStatusOn;
   /** This flag indicates that Status Led should flash */
   DSL_boolean_t bLedNeedToFlash;
   /** This flag indicates the status of the Led module initialization*/
   DSL_boolean_t bLedInit;
#endif

#if defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_DATA_LED_SIMULATOR)
   DSL_DataLedSimControlData_t nDataLedSimControlData;
   DSL_DRV_Event_t dataLedSimEvent;
#endif /* defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_DATA_LED_SIMULATOR)*/

   /** Statistics */
   /** Software UAS counter */
   DSL_uint16_t nUAS;

   /** Current SNR margin*/
   DSL_int16_t nSnrmDs;
   /** Current Near End failures */
   DSL_G997_BF_LineFailures_t nLineFailuresNe;
   /** Current Far End failures */
   DSL_G997_BF_LineFailures_t nLineFailuresFe;
   /** Near End failures event generation mask */
   DSL_G997_BF_LineFailures_t nLineFailuresNeAlarmMask;
   /** Far End failures event generation mask */
   DSL_G997_BF_LineFailures_t nLineFailuresFeAlarmMask;
   DSL_G997_BF_DataPathFailures_t nDataPathFailuresNe[DSL_CHANNELS_PER_LINE];
   DSL_G997_BF_DataPathFailures_t nDataPathFailuresFe[DSL_CHANNELS_PER_LINE];
   DSL_G997_BF_DataPathFailures_t nDataPathFailuresNeAlarmMask;
   DSL_G997_BF_DataPathFailures_t nDataPathFailuresFeAlarmMask;

#ifdef INCLUDE_DSL_CPE_API_VINAX
   DSL_LinePathCounterTotalData_t nTotalLinePathCounters[DSL_CHANNELS_PER_LINE];
   DSL_DataPathCounterTotalData_t nTotalDataPathCounters[DSL_CHANNELS_PER_LINE];
#endif /** INCLUDE_DSL_CPE_API_VINAX*/

#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   /** Showtime event logging buffer */
   DSL_uint16_t loggingBuffer[DSL_DEV_SHOWTIME_EVENT_LOGGING_BUFFER_LENGTH];
#endif

#if defined(INCLUDE_DSL_CEOC)
   DSL_void_t *CEOC;
#endif /** #if defined(INCLUDE_DSL_CEOC)*/

#if defined(INCLUDE_DSL_PM)
   /** PM module Context*/
   DSL_void_t *PM;
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   /**
   Full Initializations.
   This parameter is a count of the total number of full initializations
   attempted on the line (successful and failed) during the accumulation period.
   Parameter procedures shall be as defined in chapter 7.2.7 of G.997.1. */
   DSL_uint32_t nFullInits;
   /**
   Failed Full Initializations.
   This performance parameter is a count of the total number of failed full
   initializations during the accumulation period. A failed full initialization
   is when showtime is not reached at the end of the full initialization
   procedure, e.g., when:
      - A CRC error is detected.
      - A time out occurs.
      - Unexpected message content is received.
   Parameter procedures shall be as defined in in chapter 7.2.7 of G.997.1. */
   DSL_uint32_t nFailedFullInits;
#endif /** INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   /**
   Line Failure Counters*/
   DSL_pmLineEventShowtimeData_t pmLineEventShowtimeCounters;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   /**
    Data Path Failure Counters data structure*/
   DSL_pmDataPathFailureData_t pmDataPathFailureCounters;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#endif /* defined(INCLUDE_DSL_PM) */

/* ADSL MIB Module stuff */
#if defined(INCLUDE_DSL_ADSL_MIB)
   /** ADSL Mib context */
   DSL_MIB_ADSL_Context_t MibAdslCtx;
#endif
#ifdef INCLUDE_DSL_BONDING
   /**
   Bonding Configuration Data*/
   DSL_BND_ConfigData_t BndConfig;
#endif /* INCLUDE_DSL_BONDING*/
};

#define DSL_EVENT2MASK(evt) ((DSL_uint32_t)(0x1 << ((DSL_uint32_t)evt)))

#define DSL_DEV_NUM(X)   (X->pDevCtx->nNum)

#ifdef DRV_DSL_CPE_FORCE_MACROS
   /** Macro to read values from the context in the safe way */
   #define DSL_CTX_READ(ctx, err, attr, val) \
      do { \
         if (sizeof(ctx->attr) != sizeof(val)) \
         { \
            DSL_DEBUG( DSL_DBG_ERR, (ctx, DSL_DRV_CRLF""DSL_DRV_CRLF"DSL_CTX_READ - Context field " \
               "and value size mismatch! Data will be NOT copied!!!"DSL_DRV_CRLF)); \
            err = DSL_ERR_MEMORY; \
         } \
         else \
         { \
            if(DSL_DRV_MUTEX_LOCK(ctx->dataMutex)) \
            { \
              DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't lock data mutex"DSL_DRV_CRLF)); \
              err = DSL_ERR_SEMAPHORE_GET; \
            } \
            else \
            { \
               memcpy(&val, &ctx->attr, sizeof(ctx->attr)); \
               DSL_DRV_MUTEX_UNLOCK(ctx->dataMutex); \
               err = DSL_SUCCESS; \
            } \
         } \
      } while (0)

   /** Macro to read scalar values from the context in the safe way */
   #define DSL_CTX_READ_SCALAR(ctx, err, attr, val) \
      do { \
         if (sizeof(ctx->attr) != sizeof(val)) \
         { \
            DSL_DEBUG( DSL_DBG_ERR, (ctx, DSL_DRV_CRLF""DSL_DRV_CRLF"DSL_CTX_READ_SCALAR - Context field " \
               "and value size mismatch! Data will be NOT copied!!!"DSL_DRV_CRLF)); \
            err = DSL_ERR_MEMORY; \
         } \
         else \
         { \
            if(DSL_DRV_MUTEX_LOCK(ctx->dataMutex)) \
            { \
              DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't lock data mutex"DSL_DRV_CRLF)); \
              err = DSL_ERR_SEMAPHORE_GET; \
            } \
            else \
            { \
               memcpy(&val, &ctx->attr, sizeof(ctx->attr)); \
               DSL_DRV_MUTEX_UNLOCK(ctx->dataMutex); \
               err = DSL_SUCCESS; \
            } \
         } \
      } while (0)



   #define _DSL_CTX_WRITE(ctx, err, attr, newval) \
      do { \
         if (sizeof(ctx->attr) != sizeof(newval)) \
         { \
            DSL_DEBUG( DSL_DBG_ERR, (ctx, DSL_DRV_CRLF""DSL_DRV_CRLF"DSL_CTX_WRITE - Context field " \
               "and value size mismatch! Data will be NOT copied!!!"DSL_DRV_CRLF)); \
            err = DSL_ERR_MEMORY; \
         } \
         else \
         { \
            if(DSL_DRV_MUTEX_LOCK(ctx->dataMutex)) \
            { \
               DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't lock data mutex " \
                  "(pContext=%p, nOffset=%08x"DSL_DRV_CRLF, ctx, \
                  ((DSL_uint8_t*)&(ctx->attr)) - ((DSL_uint8_t*)ctx))); \
               err = DSL_ERR_SEMAPHORE_GET; \
            } \
            else \
            { \
               memcpy(&ctx->attr, &newval, sizeof(ctx->attr)); \
               err = DSL_SUCCESS; \
               DSL_DRV_MUTEX_UNLOCK(ctx->dataMutex); \
            } \
         } \
      } while (0)

   /** Macro to set scalar values to context fields in the safe way */
   #define DSL_CTX_WRITE_SCALAR(ctx, err, attr, newval) \
      do { \
         if(DSL_DRV_MUTEX_LOCK(ctx->dataMutex)) \
         { \
            DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't lock data mutex " \
               "(pContext=%p, nOffset=%08x"DSL_DRV_CRLF, ctx, \
               ((DSL_uint8_t*)&(ctx->attr)) - ((DSL_uint8_t*)ctx))); \
            err = DSL_ERR_SEMAPHORE_GET; \
         } \
         else \
         { \
            if(sizeof(ctx->attr) == 1) \
            { \
               *((DSL_uint8_t*)&(ctx->attr)) = (DSL_uint8_t)newval; \
            } \
            else if (sizeof(ctx->attr) == 2) \
            { \
               *((DSL_uint16_t*)&(ctx->attr)) = (DSL_uint16_t)newval; \
            } \
            else if (sizeof(ctx->attr) == 4) \
            { \
               *((DSL_uint32_t*)&(ctx->attr)) = (DSL_uint32_t)newval; \
            } \
            else \
            { \
               DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't read scalar " \
                  "data with size different from 1, 2 or 4 bytes!!!"DSL_DRV_CRLF)); \
            } \
            err = DSL_SUCCESS; \
            DSL_DRV_MUTEX_UNLOCK(ctx->dataMutex); \
         } \
      } while (0)

   /** Macro to set values to context fields in the safe way */
   #define DSL_CTX_WRITE(ctx, err, attr, newval) \
      _DSL_CTX_WRITE(ctx, err, attr, newval)
#else
   DSL_Error_t
   _DSL_CTX_ASSIGN_8(
      DSL_Context_t *pContext,
      DSL_uint8_t nFrom,
      DSL_void_t *pTo
   );

   DSL_Error_t
   _DSL_CTX_ASSIGN_16(
      DSL_Context_t *pContext,
      DSL_uint16_t nFrom,
      DSL_void_t *pTo
   );

   DSL_Error_t
   _DSL_CTX_ASSIGN_32(
      DSL_Context_t *pContext,
      DSL_uint32_t nFrom,
      DSL_void_t *pTo
   );

   DSL_Error_t
   _DSL_CTX_ASSIGN_ANY(
      DSL_Context_t *pContext,
      DSL_void_t *pFrom,
      DSL_void_t *pTo,
      DSL_uint32_t nSize
   );

   /* Macro to read values from the context in the safe way */
   #define DSL_CTX_READ_SCALAR(ctx, err, attr, val) \
      if (sizeof(ctx->attr) == 1) \
      { \
         err = _DSL_CTX_ASSIGN_8(ctx, (DSL_uint8_t)(ctx->attr), &(val)); \
      } \
      if (sizeof(ctx->attr) == 2) \
      { \
         err = _DSL_CTX_ASSIGN_16(ctx, (DSL_uint16_t)(ctx->attr), &(val)); \
      } \
      if (sizeof(ctx->attr) == 4) \
      { \
         err = _DSL_CTX_ASSIGN_32(ctx, (DSL_uint32_t)(ctx->attr), &(val)); \
      } \
      if (sizeof(ctx->attr) != 1 && sizeof(ctx->attr) != 2 \
         && sizeof(ctx->attr) != 4) \
      { \
         DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't read scalar " \
            "data with size different from 1, 2 or 4 bytes!!!"DSL_DRV_CRLF)); \
      }

   /* Macro to read values from the context in the safe way */
   #define DSL_CTX_READ(ctx, err, attr, val)  do { \
         if (sizeof(ctx->attr) != sizeof(val)) \
         { \
            DSL_DEBUG( DSL_DBG_ERR, (ctx, DSL_DRV_CRLF""DSL_DRV_CRLF"DSL_CTX_READ - Context field " \
               "and value size mismatch! Data will be NOT copied!!!"DSL_DRV_CRLF)); \
            err = DSL_ERR_MEMORY; \
         } \
         else \
         { \
            err = _DSL_CTX_ASSIGN_ANY(ctx, &(ctx->attr), &(val), sizeof(ctx->attr)); \
         } \
      } while (0)

   /* Macro to set values to context fields in the safe way */
   #define DSL_CTX_WRITE(ctx, err, attr, val) do { \
         if (sizeof(ctx->attr) != sizeof(val)) \
         { \
            DSL_DEBUG( DSL_DBG_ERR, (ctx, DSL_DRV_CRLF""DSL_DRV_CRLF"DSL_CTX_WRITE - Context field " \
               "and value size mismatch! Data will be NOT copied!!!"DSL_DRV_CRLF)); \
            err = DSL_ERR_MEMORY; \
         } \
         else \
         { \
            err = _DSL_CTX_ASSIGN_ANY(ctx, &(val), &(ctx->attr), sizeof(ctx->attr)); \
         } \
      } while (0)

   /* Macro to set scalar values to context fields in the safe way */
   #define DSL_CTX_WRITE_SCALAR(ctx, err, attr, val) \
      if (sizeof(ctx->attr) == 1) \
      { \
         err = _DSL_CTX_ASSIGN_8(ctx, (DSL_uint8_t)(val), &(ctx->attr)); \
      } \
      if (sizeof(ctx->attr) == 2) \
      { \
         err = _DSL_CTX_ASSIGN_16(ctx, (DSL_uint16_t)(val), &(ctx->attr)); \
      } \
      if (sizeof(ctx->attr) == 4) \
      { \
         err = _DSL_CTX_ASSIGN_32(ctx, (DSL_uint32_t)(val), &(ctx->attr)); \
      } \
      if (sizeof(ctx->attr) != 1 && sizeof(ctx->attr) != 2 \
         && sizeof(ctx->attr) != 4) \
      { \
         DSL_DEBUG( DSL_DBG_ERR, (ctx, "Couldn't write scalar " \
            "data with size different from 1, 2 or 4 bytes!!!"DSL_DRV_CRLF)); \
      }
#endif

/**
   Returns the number of elements of the array.

   \param array   the array variable name, [I]
*/
#define DSL_ARRAY_LENGTH(array) ((DSL_uint32_t)(sizeof(array)/sizeof((array)[0])))


/* ************************************************************************** */
/* * This functions are directly used from ioctl interface                  * */
/* * ==> All of this functions needs wrapper code for cli which has to be   * */
/* *     generated automatically or manual if automatic generation is not   * */
/* *     possible                                                           * */
/* ************************************************************************** */

/**
   This function generates Event according to the nEventType.

   \param pContext   - Pointer to dsl cpe library context structure, [I]
   \param nChannel   - Bearer channel, [I]
   \param nAccessDir - Access direction, DSL_UPSTREAM or DSL_DOWNSTREAM, [I]
   \param nXtuDir    - XTU direction, DSL_FAR_END or DSL_NEAR_END, [I]
   \param nEventType - Event type, [I]
   \param pData      - pointer to the event data. If no data present, specify DSL_NULL, [I]
   \param nDataSize  - Size of the event data. sizeof(one of the event union element size), [I]


   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_EventGenerate(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_AccessDir_t nAccessDir,
   DSL_XTUDir_t nXtuDir,
   DSL_EventType_t nEventType,
   DSL_EventData_Union_t *pData,
   DSL_uint16_t nDataSize);
#endif

#if defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM) || defined(INCLUDE_DSL_CPE_API_VINAX)
#ifndef SWIG
DSL_Error_t DSL_DRV_HandleLinitValue(
   DSL_Context_t *pContext,
   const DSL_G997_LineInit_t nLinit,
   const DSL_G997_LineInitSubStatus_t nSub);
#endif
#endif /* defined(INCLUDE_DSL_G997_STATUS) || defined(INCLUDE_DSL_G997_ALARM)*/

#ifndef SWIG
DSL_Error_t DSL_DRV_AutobootStatusSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_AutobootStatGet_t nStatus,
   DSL_IN DSL_FirmwareRequestType_t nFirmwareRequestType);
#endif

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
#ifndef SWIG
DSL_Error_t DSL_DRV_ResourceUsageStatisticsGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_ResourceUsageStatistics_t *pData);
#endif
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

#ifndef SWIG
/**
   Check for a valid XTSE settings
*/
DSL_Error_t DSL_DRV_XtseSettingsCheck(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_uint8_t *pXTSE);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_INIT
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_Init(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_Init_t *pData
);
#endif

#ifndef SWIG
DSL_Error_t DSL_DRV_ModulesInit(
   DSL_IN DSL_Context_t *pContext);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_AUTOBOOT_LOAD_FIRMWARE
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_AutobootLoadFirmware(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_AutobootLoadFirmware_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_AUTOBOOT_CONTROL_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_AutobootControlSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_AutobootControl_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_AUTOBOOT_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_AutobootConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_AutobootConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_AUTOBOOT_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_AutobootConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_AutobootConfig_t *pData
);
#endif


/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_AUTOBOOT_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_AutobootStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_AutobootStatus_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_TEST_MODE_CONTROL_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_TestModeControlSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_TestModeControl_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_TEST_MODE_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_TestModeStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_TestModeStatus_t *pData
);
#endif

/** @} DRV_DSL_CPE_INIT */


/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_PATH_COUNTER_TOTAL_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LinePathCounterTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LinePathCounterTotal_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_DATA_PATH_COUNTER_TOTAL_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_DataPathCounterTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_DataPathCounterTotal_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_VERSION_INFORMATION_GET
*/
DSL_Error_t DSL_DRV_VersionInformationGet(
    DSL_IN DSL_Context_t *pContext,
    DSL_IN_OUT DSL_VersionInformation_t *pData
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_STATE_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LineStateGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LineState_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_FEATURE_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LineFeatureConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LineFeature_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_FEATURE_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LineFeatureConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LineFeature_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_FEATURE_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LineFeatureStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LineFeature_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_INTEROP_FEATURE_CONFIG_SET
*/
#ifdef INCLUDE_DSL_CPE_API_DANUBE
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_InteropFeatureConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_InteropFeatureConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_INTEROP_FEATURE_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CPE_API_DANUBE
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_InteropFeatureConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_InteropFeatureConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_FRAMING_PARAMETER_STATUS_GET
*/
#ifdef INCLUDE_DSL_FRAMING_PARAMETERS
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_FramingParameterStatus_t *pData
);
#endif
#endif /* INCLUDE_DSL_FRAMING_PARAMETERS*/

#if defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS) || defined(INCLUDE_DSL_CPE_API_DANUBE)
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_AdslBandLimitsGet(
   DSL_Context_t *pContext,
   DSL_AccessDir_t nDirection,
   DSL_Band_t *pData
);
#endif
#endif /* defined(INCLUDE_DSL_CPE_MISC_LINE_STATUS) || defined(INCLUDE_DSL_CPE_API_DANUBE)*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BAND_BORDER_STATUS_GET
*/
#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BandBorderStatusGet(
   DSL_Context_t *pContext,
   DSL_BandBorderStatus_t *pData
);
#endif
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_MISC_LINE_STATUS_GET
*/
#ifdef INCLUDE_DSL_CPE_MISC_LINE_STATUS
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_MiscLineStatusGet(
   DSL_Context_t *pContext,
   DSL_MiscLineStatus_t *pData
);
#endif
#endif /* INCLUDE_DSL_CPE_MISC_LINE_STATUS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_OPTIONS_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LineOptionsConfigSet(
   DSL_Context_t *pContext,
   DSL_LineOptionsConfig_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LINE_OPTIONS_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LineOptionsConfigGet(
   DSL_Context_t *pContext,
   DSL_LineOptionsConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_DBG_LAST_EXCEPTION_CODES_GET
*/
#ifdef INCLUDE_DSL_CPE_API_DANUBE
#ifdef INCLUDE_DEVICE_EXCEPTION_CODES
#ifndef SWIG_TMP
DSL_Error_t DSL_DBG_LastExceptionCodesGet(
   DSL_Context_t *pContext,
   DSL_DBG_LastExceptionCodes_t *pData);
#endif
#endif /* INCLUDE_DEVICE_EXCEPTION_CODES*/
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LOW_LEVEL_CONFIGURATION_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LowLevelConfigurationSet(
   DSL_Context_t *pContext,
   DSL_LowLevelConfiguration_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LOW_LEVEL_CONFIGURATION_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LowLevelConfigurationGet(
   DSL_Context_t *pContext,
   DSL_LowLevelConfiguration_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_AUX_LINE_INVENTORY_GET
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_AuxLineInventoryGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_AuxLineInventory_t *pData
);
#endif
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BAND_PLAN_SUPPORT_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BandPlanStatusGet(
   DSL_Context_t *pContext,
   DSL_BandPlanStatus_t *pData);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BAND_PLAN_SUPPORT_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_BandPlanSupportedGet(
   DSL_Context_t *pContext,
   DSL_BandPlanSupport_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_EfmMacConfigCheck(
   DSL_Context_t *pContext,
   DSL_EFM_MacConfigData_t *pData);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_EFM_MAC_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_EfmMacConfigSet(
   DSL_Context_t *pContext,
   DSL_EFM_MacConfig_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_EFM_MAC_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_EfmMacConfigGet(
   DSL_Context_t *pContext,
   DSL_EFM_MacConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_LOOP_LENGTH_STATUS_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_LoopLengthStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_LoopLengthStatus_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_UTOPIA_ADDRESS_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_UtopiaAddressConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PhyAddressConfig_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_UTOPIA_ADDRESS_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_UtopiaAddressConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PhyAddressConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_UTOPIA_BUS_WIDTH_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_UtopiaBusWidthConfigSet(
   DSL_Context_t *pContext,
   DSL_UtopiaBusWidthConfig_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_UTOPIA_BUS_WIDTH_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_UtopiaBusWidthConfigGet(
   DSL_Context_t *pContext,
   DSL_UtopiaBusWidthConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_POSPHY_ADDRESS_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_PosphyAddressConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PhyAddressConfig_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_POSPHY_ADDRESS_CONFIG_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_PosphyAddressConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PhyAddressConfig_t *pData
);
#endif
#endif /* #if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_SystemInterfaceConfigCheck(
   DSL_Context_t *pContext,
   DSL_SystemInterfaceConfigData_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_SYSTEM_INTERFACE_CONFIG_SET
*/
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_SystemInterfaceConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_SYSTEM_INTERFACE_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_SystemInterfaceConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_SystemInterfaceConfig_t *pData
);
#endif
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/
#endif /* INCLUDE_DSL_CONFIG_GET*/

/** @} DRV_DSL_CPE_COMMON */


/** \addtogroup DRV_DSL_CPE_EVENT
 @{ */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_EVENT_STATUS_MASK_CONFIG_SET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_EventStatusMaskConfigSet(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_EventStatusMask_t *pData
);
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_EVENT_STATUS_MASK_CONFIG_GET
*/
#ifndef SWIG_TMP
DSL_Error_t DSL_DRV_EventStatusMaskConfigGet(
   DSL_OpenContext_t *pOpenCtx,
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_EventStatusMask_t *pData
);
#endif

/** @} DRV_DSL_CPE_EVENT */


/** \addtogroup DRV_DSL_CPE_DEBUG
 @{ */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_DBG_MODULE_LEVEL_SET

   \note CLI Debug functions should be not generated automatically
*/
#ifndef SWIG
#ifndef DSL_DEBUG_DISABLE
DSL_Error_t DSL_DRV_DBG_ModuleLevelSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_DBG_ModuleLevel_t *pData
);
#endif
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_DBG_MODULE_LEVEL_GET

   \note CLI Debug functions should be not generated automatically
*/
#ifndef SWIG
#ifndef DSL_DEBUG_DISABLE
DSL_Error_t DSL_DRV_DBG_ModuleLevelGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_DBG_ModuleLevel_t *pData
);
#endif
#endif

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_DBG_DEVICE_MESSAGE_SEND

   \note CLI Debug functions should be not generated automatically
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_DBG_DeviceMessageSend(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_DeviceMessage_t *pMsg
);
#endif

/** @} DRV_DSL_CPE_DEBUG */


/* ************************************************************************** */
/* * This functions are used internally only                                * */
/* * No generation of cli wrapper functions necessary.                      * */
/* * ==> SWIG preprocessor define hat to be set for ALL.                    * */
/* ************************************************************************** */

/**
   This function implements a driver internal (kernel space) specific cleanup
   functionality which is only necessary in case of using this DSL CPE API
   driver from within the kernel space.

   \param pOpenContext
      Pointer of type 'DSL_OpenContext_t' to be released.
      If the function return with DSL_SUCCESS the pointer and all associated
      data are successfully released.

   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
*/
#ifndef SWIG
DSL_void_t DSL_DRV_Cleanup(DSL_void_t);
#endif

/**
   This function returns the context for the specific device

   \param nNum
      Device number
   \param pRefContext
      Reference to NULL pointer of type 'DSL_OpenContext_t'.

   \return
   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
   - VINAX:
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_HandleGet(
   DSL_IN     DSL_int_t     nNum,
   DSL_IN_OUT DSL_OpenContext_t **pRefOpenContext
);
#endif

/**
   This function implements a driver internal (kernel space) specific open
   functionality which is only necessary in case of using this DSL CPE API driver
   from within the kernel space. It refers to the fd open in case of using ioctl
   from application layer.

   \param nNum
      Device number
   \param pRefContext
      Reference to NULL pointer of type 'DSL_OpenContext_t'. The memory allocation
      and deletion will be done by the driver.
      If the function return with DSL_SUCCESS the pointer points to successfully
      allocated memory and has to be used for calling the DSL CPE_API functions
      later on.
      \attention For ALL later API function calls the pointer has to be used
                 (NOT the reference!!!)

   \return
   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_HandleInit(
   DSL_IN     DSL_int_t     nNum,
   DSL_IN_OUT DSL_OpenContext_t **pRefOpenContext
);
#endif

/**
   This function implements a driver internal (kernel space) specific close
   functionality which is only necessary in case of using this DSL CPE API
   driver from within the kernel space. It refers to the fd close in case of
   using ioctl from application layer.

   \param pOpenContext
      Pointer of type 'DSL_OpenContext_t' to be released.
      If the function return with DSL_SUCCESS the pointer and all associated
      data are successfully released.

   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_HandleDelete(
   DSL_IN DSL_OpenContext_t *pOpenContext
);
#endif

/**
   This function implements a driver internal (kernel space) specific cleanup
   functionality. It releases all memory allocated by structures linked with
   passed device context.

   \param pRefContext
      pointer to the device context
   \param bForce
      force decrement module usage count to 1 and memory release

   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_HandleCleanup(
   DSL_IN DSL_devCtx_t *pDevContext,
   DSL_IN DSL_boolean_t bForce
);
#endif

/**
   This routine resets a DSL line

   \param pContext
      pointer to the DSL context
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_LinkReset(
   DSL_Context_t *pContext
);
#endif

/**
   This routine prepares pointers in the DSL_Init_t structure
   to further use in the DSL_DRV_Init()

   \param   bIsInKernel    where from the initial call is performed
   \param   pInit          pointer to the DSL_Init_t structure
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_InitDataPrepare(
   DSL_Context_t *pContext,
   DSL_boolean_t bIsInKernel,
   DSL_Init_t *pInit
);
#endif

/**
   This routine releases a memory allocated by DSL_DRV_InitDataPrepare()

   \param   pInit          pointer to the DSL_Init_t structure
*/
#ifndef SWIG
DSL_void_t DSL_DRV_InitDataFree(
   DSL_Context_t *pContext,
   DSL_Init_t *pInit
);
#endif

/**
   This routine releases a memory used by DSL context structure

   \param pContext
      pointer to the DSL context that should be released
*/
#ifndef SWIG
DSL_void_t DSL_DRV_Free(
   DSL_IN DSL_Context_t *pContext
);
#endif


/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

/**
   IO dispatcher routine for DSL CPE API common modules

   \param   bIsInKernel    where from the call is performed
   \param   nCommand       the ioctl command.
   \param   nArg           The address of data.

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_COMMON
 */
#ifndef SWIG
DSL_Error_t DSL_DRV_IoctlHandle(
   DSL_OpenContext_t *pOpenCtx,
   DSL_Context_t *pContext,
   DSL_boolean_t bIsInKernel,
   DSL_uint_t nCommand,
   DSL_uint32_t nArg
);
#endif

/**
   Updates nLineState field in pContext structure with current
   state of a modem

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_COMMON
 */
#ifndef SWIG
DSL_Error_t DSL_DRV_LineStateUpdate(
   DSL_Context_t *pContext
);
#endif

/**
   This function returns a handle to low-level device driver

   \param   nNum           The number of a device

   \return  The handle to the device

   \ingroup DRV_DSL_CPE_COMMON
 */
#ifndef SWIG
DSL_DEV_Handle_t DSL_DRV_LowDeviceGet(DSL_devCtx_t *pDevCtx);
#endif

/**
   Initialization routine to enable dual latency before firmware start

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_DualLatency_FirmwareInit(
   DSL_Context_t *pContext
);
#endif

/**
   This routine handles a timeout that occured.

   \param pContext   DSL CPE library context
   \param nEventType Include information on which timeout has been occured.
                     The possible values are defined by
                     \ref DSL_TimeoutEvent_t, [I]
   \param nTimeoutID Includes the timeout element id. This unique value
                     identifies the timeout event and might be compared to
                     a stored value returned from \ref DSL_Timeout_AddEvent, [I]

   \return
   - DSL_Success Timeout event handled successfully
   - DSL_Error Error during handling of timeout event
*/
#if defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))
#ifndef SWIG
DSL_Error_t DSL_DRV_OnTimeoutEvent(
   DSL_Context_t *pContext,
   DSL_int_t nEventType,
   DSL_uint32_t nTimeoutID);
#endif
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))*/

/** @} DRV_DSL_CPE_COMMON */

#if (defined(INCLUDE_DSL_ADSL_MIB))
   #include "drv_dsl_cpe_intern_mib.h"
#endif

#if defined(INCLUDE_ADSL_LED) && defined(INCLUDE_DSL_CPE_API_DANUBE)
   #include "drv_dsl_cpe_intern_led.h"
#endif /* defined(INCLUDE_ADSL_LED) && defined(INCLUDE_DSL_CPE_API_DANUBE)*/

#if (defined(INCLUDE_DSL_CEOC))
   #include "drv_dsl_cpe_intern_ceoc.h"
#endif

#if (defined(INCLUDE_DSL_PM))
   #include "drv_dsl_cpe_intern_pm.h"
#endif

#define DSL_DEVICE_LOWHANDLE(X) (X)->pDevCtx->lowHandle

/**
   Macro for checking if the given pointer is valid, which means NOT equal
   DSL_NULL.
   \param pPtr   Pointer which will be checked against DSL_NULL, [I]
*/
#ifdef DRV_DSL_CPE_FORCE_MACROS
   #define _DSL_CHECK_CTX_POINTER(pPtr) do { \
      if ((pPtr) == DSL_NULL) { DSL_DEBUG_SET_ERROR(DSL_ERR_POINTER); \
           DSL_DEBUG_HDR(DSL_DBG_ERR, (pPtr, "Invalid context pointer!")); \
           nErrCode |= DSL_ERR_POINTER; } } while (0)

   #define DSL_CHECK_CTX_POINTER(pPtr) _DSL_CHECK_CTX_POINTER(pPtr)
#else
   DSL_Error_t
   _DSL_CHECK_CTX_POINTER(
      DSL_Context_t *pContext
   );

   #define DSL_CHECK_CTX_POINTER(pPtr) (nErrCode |= _DSL_CHECK_CTX_POINTER(pPtr))
#endif

/**
   Macro for checking if the given pointers are valid, which means NOT equal
   DSL_NULL.
   The first pointer should be the library context pointer and the second a
   various one.
   \param pCtx   Context pointer which will be checked against DSL_NULL, [I]
   \param pPtr   Additional pointer which will be checked against DSL_NULL, [I]
*/
#ifdef DRV_DSL_CPE_FORCE_MACROS
   #define _DSL_CHECK_POINTER(pCtx, pPtr) \
      do { DSL_CHECK_CTX_POINTER(pCtx); \
           if (nErrCode == DSL_SUCCESS && pPtr == DSL_NULL) { \
           pCtx->nErrNo = DSL_ERR_POINTER; \
           DSL_DEBUG_HDR(DSL_DBG_ERR, (pCtx, "Invalid data pointer!")); \
           nErrCode |= DSL_ERR_POINTER; } } while(0)

   #define DSL_CHECK_POINTER(pCtx, pPtr) _DSL_CHECK_POINTER(pCtx, pPtr)
#else
   DSL_Error_t
   _DSL_CHECK_POINTER(
      DSL_Context_t *pContext,
      DSL_void_t *pPtr
   );

   #define DSL_CHECK_POINTER(pCtx, pPtr) (nErrCode |= _DSL_CHECK_POINTER(pCtx, pPtr))
#endif

#ifdef DRV_DSL_CPE_FORCE_MACROS
   #define _DSL_CHECK_MODEM_IS_READY() \
      do { DSL_CHECK_POINTER(pContext, pContext->pDevCtx); \
         if (nErrCode != DSL_SUCCESS) \
         { \
            DSL_DEBUG(DSL_DBG_ERR, (pContext, DSL_DRV_CRLF"getting mei driver semaphore " \
               "failed!")); \
            nErrCode |= DSL_ERROR; \
         } \
         if ((nErrCode == DSL_SUCCESS) && (DSL_DRV_DEV_ModemIsReady(pContext) == DSL_FALSE)) \
         { \
            DSL_DEBUG_HDR(DSL_DBG_ERR, (pContext, "Modem is not ready!")); \
            nErrCode |= DSL_ERR_MODEM_NOT_READY; \
         } \
      } \
      while (0)

   #define DSL_CHECK_MODEM_IS_READY() _DSL_CHECK_MODEM_IS_READY()
#else
   DSL_Error_t
   _DSL_CHECK_MODEM_IS_READY(
      DSL_Context_t *pContext
   );

   #define DSL_CHECK_MODEM_IS_READY() (nErrCode |= _DSL_CHECK_MODEM_IS_READY(pContext))
#endif

/**
   Macro for checking the given bearer channel number. If the channel number
   is out of the possible range it returns an nErrCode and prints a debug
   message.
   \param nChannel  Bearer channel number which will be checked for valid range, [I]
*/
#ifdef DRV_DSL_CPE_FORCE_MACROS
   #define _DSL_CHECK_CHANNEL_RANGE(nChannel) \
     do { \
        if (nChannel >= DSL_CHANNELS_PER_LINE) \
        { \
           DSL_DEBUG_SET_ERROR(DSL_ERR_CHANNEL_RANGE); \
           DSL_DEBUG(DSL_DBG_ERR, \
              (pContext, "DSL: invalid bearer channel number %d! (only bearer " \
               "channels in the range of 0..%d are supported)" DSL_DRV_CRLF, \
              nChannel, (DSL_CHANNELS_PER_LINE - 1))); \
           nErrCode |= DSL_ERR_CHANNEL_RANGE; \
        } \
     } while(0)
   #define DSL_CHECK_CHANNEL_RANGE(nChannel) _DSL_CHECK_CHANNEL_RANGE(nChannel)
#else
   DSL_Error_t
   _DSL_CHECK_CHANNEL_RANGE(
      DSL_Context_t *pContext,
      DSL_uint16_t nChannel
   );

   #define DSL_CHECK_CHANNEL_RANGE(nChannel) (nErrCode |= \
      _DSL_CHECK_CHANNEL_RANGE(pContext, nChannel))
#endif

#ifdef DRV_DSL_CPE_FORCE_MACROS
   #define _DSL_CHECK_ATU_DIRECTION(nDirection) \
      do { \
         if ((nDirection != DSL_NEAR_END) && (nDirection != DSL_FAR_END)) \
         { \
            DSL_DEBUG(DSL_DBG_ERR, \
               (pContext, "DSL: invalid xTU direction!" DSL_DRV_CRLF)); \
            nErrCode |= DSL_ERR_DIRECTION; \
         } \
      } while(0)
   #define DSL_CHECK_ATU_DIRECTION(nDirection) _DSL_CHECK_ATU_DIRECTION(nDirection)
#else
   DSL_Error_t
   _DSL_CHECK_ATU_DIRECTION(
      DSL_Context_t *pContext,
      DSL_XTUDir_t nDirection
   );

   #define DSL_CHECK_ATU_DIRECTION(nDirection) (nErrCode |= \
      _DSL_CHECK_ATU_DIRECTION(pContext, nDirection))
#endif

#ifdef DRV_DSL_CPE_FORCE_MACROS
   #define _DSL_CHECK_DIRECTION(nDirection) \
      do { \
         if ((nDirection != DSL_DOWNSTREAM) && (nDirection != DSL_UPSTREAM)) \
         { \
            DSL_DEBUG(DSL_DBG_ERR, \
               (pContext, "DSL: invalid direction!" DSL_DRV_CRLF)); \
            nErrCode |= DSL_ERR_DIRECTION; \
         } \
      } while(0)
   #define DSL_CHECK_DIRECTION(nDirection) _DSL_CHECK_DIRECTION(nDirection)
#else
   DSL_Error_t
   _DSL_CHECK_DIRECTION(
      DSL_Context_t *pContext,
      DSL_AccessDir_t nDirection
   );

   #define DSL_CHECK_DIRECTION(nDirection) (nErrCode |= \
      _DSL_CHECK_DIRECTION(pContext, nDirection))
#endif

#define DSL_CHECK_ERR_CODE() if (nErrCode != DSL_SUCCESS) { return nErrCode; }

/**
   Firmware download routine

   \param pContext      Pointer to dsl library context structure, [I]
   \param pFirmware1    1st firmware pointer, [I]
   \param nSize1        Size of 1st firmware, [I]
   \param pFirmware2    2nd firmware pointer, [I]
   \param nSize2        Size of 2nd firmware, [I]
   \param pLoff         Offset from firmware beginning, [I]
   \param pCurrentOff   Offset from block beggining after operation is done, [O]
   \param bLastChunk    Last chunk flag, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_FwDownload(
   DSL_Context_t *pContext,
   const DSL_char_t *pFw1,
   DSL_uint32_t nSize1,
   const DSL_char_t *pFw2,
   DSL_uint32_t nSize2,
   DSL_int32_t *pLoff,
   DSL_int32_t *pCurrentOff,
   DSL_boolean_t bLastChunk
);
#endif

/**
   This function is intended for internal use to have possibility to
   set line state from different modules
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_LineStateSet(
   DSL_Context_t *pContext,
   DSL_LineStateValue_t nNewLineState
);
#endif

/**
   This function retrieves different information about line in showtime state
   and updates DSL context structure with actual values if needed
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_ShowtimeStatusUpdate(
   DSL_Context_t *pContext,
   DSL_boolean_t bInit
);
#endif

/**
   This function resets DSL Context data to its default values
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_CtxDataUpdate(
   DSL_Context_t *pContext
);
#endif

/* Events stuff */

/**
   This function places an event into FIFO. It also wakes up a poll routine.

   \param pEvent        Pointer to event structure to be added to FIFO [I]
   \param nDataSize     The size of event data [I]

   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_EventQueue(
   DSL_Context_t *pContext,
   DSL_IN DSL_EventStatusData_t *pEvent,
   DSL_IN DSL_uint32_t nDataSize
);
#endif

/**
   This function gets an event from upper layer software FIFO

   \param pOpenContext  Pointer to upper layer context structure, [I]
   \param pEvent        Pointer to store event structure from FIFO [I]

   Return values are defined within the \ref DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
   - or any other defined specific error code

   \remarks
   Supported by
   - Danube: ADSL-CPE
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_EventUnqueue(
   DSL_IN DSL_OpenContext_t *pOpenContext,
   DSL_IN DSL_EventStatusData_t *pEvent
);
#endif

/* Helpers stuff */

/**
   This function converts a given value in unsigned 16-bit value in 2'complement
   format to its according signed 16-bit format.
   The type of 2'complement can be specified, for example:
   nsVal16 = 0x200, nBits = 10 ==> nuVal16 = -512

   \param nsVal16
      Specifies the unsigned 16-bit value that shall be converted, [I]
   \param nBits
      Specifies how many bits are used for 2'complement conversion.
      The bit that includes the sign is included, [I]
      \note It makes only sense to use this function in a range of
            2 <= nBits <= 15
            In case of nBits = 16 a simple cast will do the right thing.
   \param nuVal16
      Returns the converted 16-bit int value, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_TwosComplement16_HexToInt(
   DSL_Context_t *pContext,
   DSL_uint16_t nuVal16,
   DSL_char_t nBits,
   DSL_int16_t *nsVal16
);
#endif

/*
   DSL IOCTL handler helpers stuff
*/

/** This type represents handler function signature */
typedef DSL_Error_t (*DSL_DRV_IoctlHandlerHelperFunc_t)(
   DSL_Context_t *pContext,
   DSL_void_t *pArg);

typedef DSL_Error_t (*DSL_DRV_IoctlInstanceHandlerHelperFunc_t)(
   DSL_OpenContext_t *pOpenCtx,
   DSL_Context_t *pContext,
   DSL_void_t *pArg);

/** The type represents handler function type */
typedef enum DSL_IoctlHandlerHelperType {
   DSL_IOCTL_HELPER_UNKNOWN = 0,
   DSL_IOCTL_HELPER_GET = 1,
   DSL_IOCTL_HELPER_SET = 2
} DSL_IoctlHandlerHelperType_t;

/**
   IOCTL Table type definition
*/
typedef struct
{
   /* IOCTL command*/
   DSL_uint_t nCommand;
   /* IOCTL Access Type (Get or Set)*/
   DSL_IoctlHandlerHelperType_t accessType;
   /* IOCTL Instance specific*/
   DSL_boolean_t bInstanceCall;
   /* IOCTL function*/
   DSL_void_t *pFunc;
   /* IOCTL argument size*/
   DSL_uint32_t nArgSz;
} DSL_IOCTL_Table_t;

#define DSL_IOCTL_REGISTER(cmd, type, instance, func, argSz) \
           {cmd, type, instance, (DSL_void_t*)func, argSz}

/* Just a temporary solution not to change the general LED module
   implementation*/
extern int stop_led_module;
extern DSL_uint16_t flash;
extern DSL_uint16_t off;
extern DSL_uint16_t on;

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_H */
