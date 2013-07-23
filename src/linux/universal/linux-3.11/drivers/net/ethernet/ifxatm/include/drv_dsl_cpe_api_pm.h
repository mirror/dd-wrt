/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_PM_H
#define _DRV_DSL_CPE_API_PM_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"

/** \file
   Performance Monitoring interface
*/

/** \addtogroup DRV_DSL_CPE_PM
 @{ */

#ifndef DSL_PM_CHANNEL_15MIN_RECORDS_NUM
/** history size - 15 minuntes channel records */
#define DSL_PM_CHANNEL_15MIN_RECORDS_NUM     2
#endif

#ifndef DSL_PM_CHANNEL_1DAY_RECORDS_NUM
/** history size - one day channel records */
#define DSL_PM_CHANNEL_1DAY_RECORDS_NUM      2
#endif

#ifndef DSL_PM_LINE_15MIN_RECORDS_NUM
/** history size - 15 minutes line records */
#define DSL_PM_LINE_15MIN_RECORDS_NUM        2
#endif

#ifndef DSL_PM_LINE_1DAY_RECORDS_NUM
/** history size - one day line records */
#define DSL_PM_LINE_1DAY_RECORDS_NUM         2
#endif

#ifndef DSL_PM_DATAPATH_15MIN_RECORDS_NUM
/** history size - 15 minutes data path records */
#define DSL_PM_DATAPATH_15MIN_RECORDS_NUM    2
#endif

#ifndef DSL_PM_DATAPATH_1DAY_RECORDS_NUM
/** history size - one day data path records */
#define DSL_PM_DATAPATH_1DAY_RECORDS_NUM     2
#endif

#ifndef DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM
/** history size - 15 minutes line event showtime records */
#define DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM    2
#endif

#ifndef DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM
/** history size - one day line event showtime records */
#define DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM     2
#endif

#ifndef DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM
/** history size - 15 minutes data path failures records */
#define DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM    2
#endif

#ifndef DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM
/** history size - one day data path failures records */
#define DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM     2
#endif

#ifndef DSL_PM_RETX_15MIN_RECORDS_NUM
/** history size - 15 minutes ReTx records */
#define DSL_PM_RETX_15MIN_RECORDS_NUM    2
#endif

#ifndef DSL_PM_RETX_1DAY_RECORDS_NUM
/** history size - one day ReTx records */
#define DSL_PM_RETX_1DAY_RECORDS_NUM     2
#endif

/* $$PM_Showtime: Following three definitions has to be used within
   PM implementation for TR-98 showtime counters */
#ifndef DSL_PM_LINE_SHOWTIME_RECORDS_NUM
/** showtime interval numbers per line */
#define DSL_PM_LINE_SHOWTIME_RECORDS_NUM        2
#endif
#ifndef DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM
/** showtime interval numbers per channel */
#define DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM     2
#endif
#ifndef DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM
/** showtime interval numbers per data path */
#define DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM    2
#endif

#ifndef DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM
/** showtime interval numbers per line failures */
#define DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM    2
#endif

#ifndef DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM
/** showtime interval numbers per data path failures */
#define DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM    2
#endif

#ifndef DSL_PM_RETX_SHOWTIME_RECORDS_NUM
/** showtime interval numbers per ReTx */
#define DSL_PM_RETX_SHOWTIME_RECORDS_NUM    2
#endif

#ifndef DSL_PM_DEFAULT_SYNC_MODE
/**
   Default PM synchronization mode.
   This define can be set during compile time to change the default behavior.
*/
#ifndef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /** Here the sync mode should be fixed!!! */
   #define DSL_PM_DEFAULT_SYNC_MODE             DSL_PM_SYNC_MODE_SYS_TIME
#else
   #define DSL_PM_DEFAULT_SYNC_MODE             DSL_PM_SYNC_MODE_SYS_TIME
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif

/**
   Structure for configuration of basic PM behavior.
*/
typedef struct
{
   /**
   Provides the possibility to switch off the cyclic update of the PM related
   NearEnd counter values (Channel-, DataPath- and Line-Counters).
   - DSL_FALSE - Cyclic update for NearEnd counters is active (DEFAULT)
   - DSL_TRUE  - No cyclic update for NearEnd counters done */
   DSL_IN_OUT DSL_boolean_t bNePollingOff;
   /**
   Provides the possibility to switch off the cyclic update of the PM related
   FarEnd counter values (Channel-, DataPath- and Line-Counters).
   - DSL_FALSE - Cyclic update for FarEnd counters is active (DEFAULT)
   - DSL_TRUE  - No cyclic update for FarEnd counters done */
   DSL_IN_OUT DSL_boolean_t bFePollingOff;
   /**
   Configuration of basic PM counter update cycle.
   This value is used as update cycle of NearEnd counter values and as a base
   for the FarEnd factor configurations.
   It will be only taken into account if NE or FE polling is active
   (bNePollingOff = DSL_FALSE or bFePollingOff = DSL_FALSE).
   The value ranges from  1 s to 10 s with 1 s steps. 
   The default configuration value is 1 [s]. */
   DSL_IN_OUT DSL_uint8_t nBasicUpdateCycle;
   /**
   Configuration of FarEnd update cycle factor in showtime L0.
   The value specifies the used cycle time for FarEnd counter update in relation
   to the basic update cycle time nBasicUpdateCycle.
   Due to the fact that the far end counters has to be requested via EOC/OHC
   (whether by firmware or by API) the FE update cycle time should be always
   higher than the near end update cycle time.
   This value will be only taken into account if FE polling is active
   (bFePollingOff = DSL_FALSE).
   The value ranges from  1 to 10 with 1 steps. 
   The default configuration value is 3 which leads to 3s cycle in case of
   nBasicUpdateCycle=1s. */
   DSL_IN_OUT DSL_uint8_t nFeUpdateCycleFactor;
   /**
   Configuration of FarEnd update cycle factor in case of L2 Power Mode.
   This value has the same meaning as the previous one (nFeUpdateCycleFactor)
   but will be only used if L2 Power Management mode is active.
   Due to the fact that the Overhead Channel performance might be significantly
   reduced in case of L2 mode the FE counter request cycle has to be extended
   accordingly.
   This value will be only taken into account if FE polling is active
   (bFePollingOff = DSL_FALSE).
   The value ranges from  1 to 20 with 1 steps. 
   The default configuration value is 10 which leads to 10s cycle in case of
   nBasicUpdateCycle=1s.
   Configuration parameter not used for the Vinax device*/
   DSL_IN_OUT DSL_uint8_t nFeUpdateCycleFactorL2;
} DSL_PM_ConfigData_t;
   
/**
   Structure for configuration of basic PM behavior.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_CONFIG_GET
   - \ref DSL_FIO_PM_CONFIG_SET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains basic PM configuration data */
   DSL_IN_OUT DSL_PM_ConfigData_t data;
} DSL_PM_Config_t;

/* ************************************************************************** */
/* *  xDSL Line Endpoint interface                                          * */
/* ************************************************************************** */

/**
   Structure used to return additional information for total counters.
*/
typedef struct
{
   /**
   Elapsed time in the interval */
   DSL_OUT DSL_uint32_t nElapsedTime;
   /**
   Validity flag. API always sets this flag to DSL_TRUE */
   DSL_OUT DSL_boolean_t bValid;
} DSL_PM_TotalData_t;

/**
   Structure used to return additional information about a PM interval.
*/
typedef struct
{
   /**
   Elapsed time in the interval */
   DSL_OUT DSL_uint32_t nElapsedTime;
   /**
   Interval number */
   DSL_OUT DSL_uint8_t nNumber;
   /**
   Interval validity flag */
   DSL_OUT DSL_boolean_t bValid;
} DSL_PM_IntervalData_t;

/**
   History statistics structure for valid/invalid counters.
*/
typedef struct
{
   /**
   Number of previous intervals */
   DSL_OUT DSL_uint32_t nPrevIvs;
   /**
   Number of previous invalid intervals */
   DSL_OUT DSL_uint32_t nPrevInvalidIvs;
} DSL_PM_HistoryStatsData_t;

/**
   History statistics structure for line specific data usage.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_INIT_HISTORY_STATS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_INIT_HISTORY_STATS_1DAY_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains PM history information */
   DSL_OUT DSL_PM_HistoryStatsData_t data;
} DSL_PM_HistoryStats_t;

/**
   History statistics structure for line and direction specific data usage.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_SEC_HISTORY_STATS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_SEC_HISTORY_STATS_1DAY_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains direction specific PM history information */
   DSL_OUT DSL_PM_HistoryStatsData_t data;
} DSL_PM_HistoryStatsDir_t;

/**
   History statistics structure for line and direction specific data usage.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_CHANNEL_HISTORY_STATS_15MIN_GET
   - \ref DSL_FIO_PM_CHANNEL_HISTORY_STATS_1DAY_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains direction and channel specific PM history information */
   DSL_PM_HistoryStatsData_t data;
} DSL_PM_HistoryStatsChDir_t;

/**
   Line performance second counters (see chapter 7.2.1.1 and 7.2.1.2 of G.997.1).
*/
typedef struct
{
   /**
   Errored Seconds.
   This parameter is a count of 1 second intervals with one or
   more CRC 8 anomalies summed over all received bearer channels,
   or one or more LOS defects, or one or more SEF defects, or one or
   more LPR defects. */
   DSL_OUT DSL_uint32_t nES;
   /**
   Severely Errored Seconds.
   This parameter is a count of 1 second intervals with 18 or more
   CRC 8 anomalies summed over all received bearer channels, or one or
   more LOS defects, or one or more SEF defects, or one or more LPR defects.
   If a common CRC is applied over multiple bearer channels, then each related
   CRC-8 anomaly shall be counted only once for the whole set of bearer channels
   over which the CRC is applied. */
   DSL_OUT DSL_uint32_t nSES;
   /**
   Loss of Signal Seconds.
   This parameter is a count of 1 second intervals containing one or more LOS
   defects. */
   DSL_OUT DSL_uint32_t nLOSS;
   /**
   Unavailable Seconds (see chapter 7.2.7.13 of G.997.1.)
   This parameter is a count of 1 second intervals for which the ADSL line is
   unavailable. The ADSL line becomes unavailable at the onset of 10 contiguous
   SES Ls. The 10 SESs are included in unavailable time. Once unavailable, the
   ADSL line becomes available at the onset of 10 contiguous seconds with no
   SESs. The 10 seconds with no SESs are excluded from unavailable time. Some
   parameter counts are inhibited during unavailability */
   DSL_OUT DSL_uint32_t nUAS;
   /**
   Loss of Frame Seconds.
   This parameter is a count of 1 second intervals for which the ADSL line is in
   Loss of frame (LOF) state.
   An LOF failure is declared after 2.5 + 0.5 s of contiguous SEF defect, except
   when an LOS defect or failure is present (see LOS definition above).
   An LOF failure is cleared when LOS failure is declared, or after 10 + 0.5 s
   of no SEF defect. */
   DSL_OUT DSL_uint32_t nLOFS;
} DSL_PM_LineSecData_t;

/**
   Line performance second counters.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_SEC_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_SEC_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_LINE_SEC_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_LINE_SEC_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains line specific PM history counter data */
   DSL_OUT DSL_PM_LineSecData_t data;
} DSL_PM_LineSecCounters_t;

/**
   Line performance second counters (see chapter 7.2.1.1 and 7.2.1.2 of G.997.1).
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_LINE_SEC_COUNTERS_TOTAL_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains line specific PM history counter data */
   DSL_OUT DSL_PM_LineSecData_t data;
} DSL_PM_LineSecCountersTotal_t;

/**
   Line performance second counters thresholds (see chapter 7.2.1.1 and 7.2.1.2
   of G.997.1).
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_15MIN_SET
   - \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_1DAY_SET
   - \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_1DAY_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains line specific PM threshold configuration */
   DSL_CFG DSL_PM_LineSecData_t data;
} DSL_PM_LineSecThreshold_t;

/**
   Line failure counters.
*/
typedef struct
{
   /**
   Loss-of-signal failure.*/
   DSL_OUT DSL_uint32_t nLOS;
   /**
   Loss-of-frame failure.*/
   DSL_OUT DSL_uint32_t nLOF;
   /**
   Loss-of-power failure.*/
   DSL_OUT DSL_uint32_t nLPR;
   /**
   Loss-of-margin failure.*/
   DSL_OUT DSL_uint32_t nLOM;
   /**
   SOS performance monitoring parameter 
   Successful SOS count (SOS-SUCCESS)
   (see chapter 7.2.1.6/7 of G.997.1)
                                    
   This parameter is a count of the total number of successful SOS procedures 
   initiated by the near-end/far-end xTU on the line during the accumulation 
   period. Parameter procedures shall be as defined in 7.2.7.
   Successful SOS is defined in 12.1.4/G.993.2. */
   DSL_uint32_t nSosSuccess;
} DSL_PM_LineEventShowtimeData_t;

/**
   Line performance second counters.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains line specific PM history counter data */
   DSL_OUT DSL_PM_LineEventShowtimeData_t data;
} DSL_PM_LineEventShowtimeCounters_t;

#ifdef INCLUDE_DEPRECATED
typedef DSL_PM_LineEventShowtimeCounters_t DSL_PM_LineFailureCounters_t ;
#endif /* INCLUDE_DEPRECATED */

/**
   Line failure counters.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_TOTAL_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains line specific PM history counter data */
   DSL_OUT DSL_PM_LineEventShowtimeData_t data;
} DSL_PM_LineEventShowtimeCountersTotal_t;


#ifdef INCLUDE_DEPRECATED
typedef DSL_PM_LineEventShowtimeCountersTotal_t DSL_PM_LineFailureCountersTotal_t ;
#endif /* INCLUDE_DEPRECATED */

/**
   Line performance initialization counters (see chapter 7.2.1.3 of G.997.1).
*/
typedef struct
{
   /**
   Full Initializations.
   This parameter is a count of the total number of full initializations
   attempted on the line (successful and failed) during the accumulation period.
   Parameter procedures shall be as defined in chapter 7.2.7 of G.997.1. */
   DSL_OUT DSL_uint32_t nFullInits;
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
   DSL_OUT DSL_uint32_t nFailedFullInits;
   /**
   Short Initializations.
   This parameter is a count of the total number of fast retrains or short
   initializations attempted on the line (successful and failed) during the
   accumulation period. Parameter procedures shall be as defined in chapter
   7.2.7 of G.997.1.
   Fast Retrain is defined in Recommendation G.992.2.
   Short Initialization is defined in Recommendation G.992.3 and G.992.4. */
   DSL_OUT DSL_uint32_t nShortInits;
   /**
   Failed Short Initializations.
   This performance parameter is a count of the total number of failed fast
   retrains or short initializations during the accumulation period. A failed
   fast retrain or short initialization is when showtime is not reached at the
   end of the fast retrain or short initialization procedure, e.g., when:
      - A CRC error is detected.
      - A time out occurs.
      - A fast retrain profile is unknown.
   Parameter procedures shall be as defined in chapter 7.2.7 of G.997.1.*/
   DSL_OUT DSL_uint32_t nFailedShortInits;
} DSL_PM_LineInitData_t;

/**
   Line performance initialization counters.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_INIT_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_INIT_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_LINE_INIT_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Number of history interval (0 - current interval).
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_LINE_INIT_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains line specific PM initialization counter data */
   DSL_OUT DSL_PM_LineInitData_t data;
} DSL_PM_LineInitCounters_t;

/**
   Line performance initialization counters (see chapter 7.2.1.3 of G.997.1).
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_LINE_INIT_COUNTERS_TOTAL_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains line specific PM initialization counter data */
   DSL_OUT DSL_PM_LineInitData_t data;
} DSL_PM_LineInitCountersTotal_t;

/**
   Line performance initialization threshold (see chapter 7.2.1.3 of G.997.1).
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_15MIN_SET
   - \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_1DAY_SET
   - \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_15MIN_GET
   - \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_1DAY_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains line specific PM initialization threshold
   configuration data */
   DSL_OUT DSL_PM_LineInitData_t data;
} DSL_PM_LineInitThreshold_t;

/**
   Line threshold crossing bitfields.
   \remarks The enums corresponds to bit field definitions.
*/
typedef enum
{
   /**
   Empty entry */
   DSL_PM_LINETHRESHCROSS_EMPTY   = 0x00000000,
   /**
   FECS counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_FECS    = 0x00000001,
   /**
   ES counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_ES      = 0x00000002,
   /**
   SES counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_SES     = 0x00000004,
   /**
   LOSS counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_LOSS    = 0x00000008,
   /**
   UAS counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_UAS     = 0x00000010,
   /**
   Full initializations counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_FI      = 0x00000020,
   /**
   Failed full initializations counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_FIFAIL  = 0x00000040,
   /**
   Short initializations counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_SI      = 0x00000080,
   /**
   Failed short initializations counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_SIFAIL  = 0x00000100,
   /**
   LOFS counter threshold crossing. */
   DSL_PM_LINETHRESHCROSS_LOFS    = 0x00000200
} DSL_PM_BF_LineThresholdCrossing_t;


/**
   Line counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   Number of seconds elapsed in the current 15 min interval. */
   DSL_uint16_t   nCurr15MinTime;
   /**
   Number of seconds elapsed in the current 1 day interval. */
   DSL_uint32_t   nCurr1DayTime;
   /**
   15 minutes line counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_LineThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day line counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_LineThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_PM_LineThresholdCrossing_t;

/* ************************************************************************** */
/* *  xDSL Channel Endpoint interface                                       * */
/* ************************************************************************** */

/**
   Channel performance counters (see chapter 7.2.2.1 and 7.2.2.2 of G.997.1)
*/
typedef struct
{
   /**
   Code Violations.
   This parameter is a count of CRC 8 anomalies (the number of incorrect CRC)
   occurring in the bearer channel during the accumulation period. This parameter
   is subject to inhibiting - see chapter 7.2.7.13 of G.997.1.
   If the CRC is applied over multiple bearer channels, then each related CRC 8
   anomaly shall increment each of the counters related to the individual bearer channels. */
   DSL_OUT DSL_uint32_t nCodeViolations;
   /**
   Forward Error Corrections.
   This parameter is a count of FEC anomalies (the number of corrected code words)
   occurring in the bearer channel during the accumulation period. This parameter is
   subject to inhibiting - see chapter 7.2.7.13 of G.997.1.
   If FEC is applied over multiple bearer channels, then each related FEC anomaly
   shall increment each of the counters related to the individual bearer channels.*/
   DSL_OUT DSL_uint32_t nFEC;
} DSL_PM_ChannelData_t;

/**
   Channel performance counters (see chapter 7.2.2.1 and 7.2.2.2 of G.997.1).
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_CHANNEL_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_CHANNEL_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains channel specific PM counter data */
   DSL_OUT DSL_PM_ChannelData_t data;
} DSL_PM_ChannelCounters_t;

/**
   Extended Channel performance counters
*/
typedef struct
{
   /**
      ADSL1 superframe counter
   */
   DSL_OUT DSL_uint32_t nSuperFrame;
} DSL_PM_ChannelDataExt_t;

/**
   Channel performance extended counters.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_CHANNEL_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_CHANNEL_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains channel specific PM counter data */
   DSL_OUT DSL_PM_ChannelDataExt_t data;
} DSL_PM_ChannelCountersExt_t;

/**
   Channel performance thresholds (see chapter 7.2.2.1 and 7.2.2.2 of G.997.1).
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_CHANNEL_COUNTERS_TOTAL_GET
 */
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains channel specific PM counter data */
   DSL_OUT DSL_PM_ChannelData_t data;
} DSL_PM_ChannelCountersTotal_t;

/**
   Structure that is used for providing Extended Channel performance counters.
    \if INCLUDE_DSL_CPE_API_ADSL_SUPPORT==1
  This structure has to be used for ioctl
   - \ref DSL_FIO_MIB_ADSL_ATUC_CHAN_PERF_DATA_ENTRY_GET
   - \ref DSL_FIO_MIB_ADSL_ATUR_CHAN_PERF_DATA_ENTRY_GET
   \endif

   \note This structure is used within ADSL MIB interface only.
         Thus it is only used for Danube Family.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains channel specific PM counter data */
   DSL_OUT DSL_PM_ChannelDataExt_t data;
} DSL_PM_ChannelCountersExtTotal_t;

/**
   Line performance initialization threshold (see chapter 7.2.2.1 and 7.2.2.2
   of G.997.1).
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_15MIN_SET
   - \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_1DAY_SET
   - \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_15MIN_GET
   - \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_1DAY_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains line specific PM initialization threshold
   configuration data */
   DSL_OUT DSL_PM_ChannelData_t data;
} DSL_PM_ChannelThreshold_t;

/**
   Channel threshold crossing bitfields.
   \remarks The enums corresponds to bit field definitions.
*/
typedef enum
{
   /**
   Empty threshold crossing. */
   DSL_PM_CHANNELTHRESHCROSS_EMPTY = 0x00000000,
   /**
   FEC counter threshold crossing. */
   DSL_PM_CHANNELTHRESHCROSS_FEC   = 0x00000001,
   /**
   Code violations counter threshold crossing. */
   DSL_PM_CHANNELTHRESHCROSS_CV    = 0x00000002
} DSL_PM_BF_ChannelThresholdCrossing_t;

/**
   Channel counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   Number of seconds elapsed in the current 15 min interval. */
   DSL_uint16_t   nCurr15MinTime;
   /**
   Number of seconds elapsed in the current 1 day interval. */
   DSL_uint32_t   nCurr1DayTime;
   /**
   15 minutes channel counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ChannelThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day channel counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ChannelThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_PM_ChannelThresholdCrossing_t;

/* ************************************************************************** */
/* *  xDSL Data-Path Endpoint interface                                     * */
/* ************************************************************************** */

/**
   Data path performance counters (see chapter 7.2.4.1 and 7.2.4.2 of G.997.1).
*/
typedef struct
{
   /**
   HEC violation count.
   The HEC_violation_count performance parameter is a count of the number of
   occurrences of a HEC anomaly in the ATM Data Path.*/
   DSL_uint32_t nHEC;
   /**
   Delineated total cell count (nRXTotalCells).
   The delineated_total_cell_count performance parameter is a count of
   the total number of cells passed through the cell delineation and HEC function
   process operating on the ATM Data Path while in the SYNC state.*/
   DSL_uint32_t nTotalCells;
   /**
   User total cell count (nRXUserCells).
   The User_total_cell_count performance parameter is a count of the
   total number of cells in the ATM Data Path delivered at the V-C (for ATU C) or
   T-R (for ATU R) interface.*/
   DSL_uint32_t nUserTotalCells;
   /**
   Idle Cell Bit Error Count.
   The idle_bit_error_count performance parameter in a count of the number of bit
   errors in the idle cell payload received in the ATM Data Path.
   The idle cell payload is defined in Recommendations I.361 and I.432.*/
   DSL_uint32_t nIBE;

   /* PTM counters according to G997.1 chapter 7.2.5 */
   /**
   CRC error count.
   On CO: NE/FE - mandatory ; on CPE: NE mandatory / FE optional
   NEAR-END: The CRC P performance parameter is a count of the number of
   occurrences of a CRC n anomaly in the PTM Data Path at the near-end.
   FAR-END:  The far end CRC PFE performance parameter is a count at the
   far-end of the number of occurrences of a CRC n anomaly (as observed
   by the far-end) in the PTM Data Path. */
   DSL_uint32_t nCRC_P;
   /**
   CRCp error count.
   On CO: NE/FE - mandatory ; on CPE: NE mandatory / FE optional
   NEAR-END: The CRCP P performance parameter is a count of the number of
   occurrences of a CRC np anomaly in the PTM Data Path at the near-end.
   FAR-END:  The far end CRCP PFE performance parameter is a count at the
   far-end of the number of occurrences of a CRC np anomaly (as observed
   by the far-end) in the PTM Data Path. */
   DSL_uint32_t nCRCP_P;
   /**
   Coding Violations Count.
   On CO: NE/FE - mandatory ; on CPE: NE mandatory / FE optional
   NEAR-END: The CV P performance parameter is a count of the number of
   occurrences of a cv n anomaly in the PTM Data Path at the near-end.
   FAR-END:  The far end CV PFE performance parameter is a count at the
   far-end of the number of occurrences of a cv n anomaly (as observed
   by the far-end) in the PTM Data Path. */
   DSL_uint32_t nCV_P;
   /**
   Coding Violations P Count.
   On CO: NE/FE - mandatory ; on CPE: NE mandatory / FE optional
   NEAR-END: The CVP P performance parameter is a count of the number of
   occurrences of a cv np anomaly in the PTM Data Path at the near-end.
   FAR-END:  The far end CVP PFE performance parameter is a count of the
   number of occurrences of a cv np anomaly (as observed by the far-end)
   in the PTM Data Path. */
   DSL_uint32_t nCVP_P;
   /**
   Transmit ATM user cell count (nTxUserCells).
   ATM transmit user cell count for the latency path available for the Near-End
   only.*/
   DSL_uint32_t nTxUserTotalCells;
   /**
   Transmit ATM idle cell count (nTxIdleCells).
   ATM transmit idle cell count for the latency path available for the Near-End
   only.*/
   DSL_uint32_t nTxIBE;
} DSL_PM_DataPathData_t;

/**
   Data path performance counters (see chapter 7.2.4.1 and 7.2.4.2 of G.997.1).
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_DATA_PATH_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_DATA_PATH_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_DATA_PATH_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_DATA_PATH_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains data path specific PM counter data */
   DSL_OUT DSL_PM_DataPathData_t data;
} DSL_PM_DataPathCounters_t;

/**
   Data path performance counters (see chapter 7.2.4.1 and 7.2.4.2 of G.997.1).
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_DATA_PATH_COUNTERS_TOTAL_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains data path specific PM counter data */
   DSL_OUT DSL_PM_DataPathData_t data;
} DSL_PM_DataPathCountersTotal_t;

/**
   Data path performance counters (see chapter 7.2.4.1 and 7.2.4.2 of G.997.1).
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains data path specific PM threshold configuration data */
   DSL_OUT DSL_PM_DataPathData_t data;
} DSL_PM_DataPathThreshold_t;

/**
   Data path threshold crossing bitfields.
   \remarks The enums corresponds to bit field definitions.
*/
typedef enum
{
   /**
   Empty threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_EMPTY       = 0x00000000,
    /**
   HEC counter threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_HEC         = 0x00000001,
   /**
   Total cells counter threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_TOTALCELL   = 0x00000002,
   /**
   User cells counter threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_USERCELL    = 0x00000004,
   /**
   Idle Cell Bit Error counter threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_IBE         = 0x00000008,
   /**
   Count of non preemptive packets with CRC error in the
   bearer channel threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_CRC_P       = 0x00000010,
   /**
   Count of preemptive packets with CRC error in the bearer
   channel threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_CRCP_P      = 0x00000020,
   /**
   Count of non preemptive packets with coding violation
   in the bearer channel threshold crossing. */
   DSL_PM_DATAPATHTHRESHCROSS_CV_P        = 0x00000040,
   /**
   Count of preemptive packets with coding violation in
   the bearer channel threshold crossing.  */
   DSL_PM_DATAPATHTHRESHCROSS_CVP_P       = 0x00000080,
   /**
   Count of transmit ATM user cell in the bearer channel
   threshold crossing*/
   DSL_PM_DATAPATHTHRESHCROSS_TX_USER_TOTALCELL = 0x00000100,
   /**
   Count of transmit ATM idle cell in the bearer channel
   threshold crossing*/
   DSL_PM_DATAPATHTHRESHCROSS_TX_IBE = 0x00000200
} DSL_PM_BF_DataPathThresholdCrossing_t;

/**
   Data path counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   Number of seconds elapsed in the current 15 min interval. */
   DSL_uint16_t   nCurr15MinTime;
   /**
   Number of seconds elapsed in the current 1 day interval. */
   DSL_uint32_t   nCurr1DayTime;
   /**
   15 minutes data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_DataPathThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_DataPathThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_PM_DataPathThresholdCrossing_t;

/**
   Data path failure counters.
*/
typedef struct
{
   /**
   No Cell Delineation failure.*/
   DSL_uint32_t nNCD;
   /**
   Loss of Cell Delineation failure.*/
   DSL_uint32_t nLCD;
} DSL_PM_DataPathFailureData_t;

/**
   Data path failure counters.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_SHOWTIME_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_DATA_PATH_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains data path specific PM counter data */
   DSL_OUT DSL_PM_DataPathFailureData_t data;
} DSL_PM_DataPathFailureCounters_t;

/**
   Data path failure counters.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_TOTAL_GET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies for which (bearer) channel the function will apply */
   DSL_IN DSL_uint8_t nChannel;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains data path failure specific PM counter data */
   DSL_OUT DSL_PM_DataPathFailureData_t data;
} DSL_PM_DataPathFailureCountersTotal_t;

/* ************************************************************************** */
/* *  xDSL ReTx Endpoint interface                                       * */
/* ************************************************************************** */

/**
   Retransmission performance counters.
   
   \attention The implementation of the ReTx counters is PRELIMINARY only!
   The following system related conditions/requirements has to be taken into
   account (out of scope from DSL CPE API)
   - minimum version for PPE firmware: V0.32
   - minimum version of ADSL PHY firmware: 2.4.2.5.3.1
   - ReTx support is currently ONLY available for Danube firmware (not for
     Amazon-SE and also not for AR9)
*/
typedef struct
{
   /** 
   This anomaly occurs when a received DTU with detected errors is not corrected
   by the reception of a DTU, i.e. retransmission didn't correct the corrupted
   DTU. The corresponding anomaly counter counts all corrupt DTUs, also idle
   cells in case of ATM-TC. NEAR-END only. */
   DSL_OUT DSL_uint32_t nRxCorruptedTotal;
   /**
   This anomaly occurs when a received DTU, eligible for retransmission, with
   detected errors is not corrected by the reception of a DTU with the same
   SID, i.e. retransmission could not correct the to-be-protected corrupted DTU.
   The corresponding anomaly counter counts only corrupt DTUs which belong to
   the user data stream which shall be impulse noise protected by
   retransmission. NEAR-END only. */
   DSL_OUT DSL_uint32_t nRxUncorrectedProtected;
   /**
   This anomaly occurs when a received DTU (Data Transfer Unit) is detected to
   be a retransmission of a previous sent DTU. NEAR-END only. */
   DSL_OUT DSL_uint32_t nRxRetransmitted;
   /**
   This anomaly occurs when a received DTU with detected errors is corrected by
   the reception of a DTU with the same SID, i.e. a retransmission corrects the
   corrupted DTU. NEAR-END only. */
   DSL_OUT DSL_uint32_t nRxCorrected;
} DSL_PM_ReTxData_t;

/**
   Retransmission performance counters.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_RETX_COUNTERS_15MIN_GET
   - \ref DSL_FIO_PM_RETX_COUNTERS_1DAY_GET
   - \ref DSL_FIO_PM_RETX_COUNTERS_SHOWTIME_GET

   \note Even if the interface provides the direction parameter only near-end
   values are supported. Due to the fact that the ReTx stuff is not standardized
   at the moment it might be possible that there are also far end parameters
   supported in the future.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Number of history interval (0 - current interval)
   In case of requesting showtime counter values
   (\ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET) accumulated data is
   returned as follows
   - 0 (current interval): counter values since most recent DSL showtime
   - 1 : counter values since second most recent DSL showtime */
   DSL_IN DSL_uint32_t nHistoryInterval;
   /**
   Structure that contains additional information about the selected PM
   interval */
   DSL_OUT DSL_PM_IntervalData_t interval;
   /**
   Structure that contains channel specific PM counter data */
   DSL_OUT DSL_PM_ReTxData_t data;
} DSL_PM_ReTxCounters_t;

/**
   Retransmission performance counters.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_RETX_COUNTERS_TOTAL_GET

   \note Even if the interface provides the direction parameter only near-end
   values are supported. Due to the fact that the ReTx stuff is not standardized
   at the moment it might be possible that there are also far end parameters
   supported in the future.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains additional info for total PM counter */
   DSL_OUT DSL_PM_TotalData_t total;
   /**
   Structure that contains channel specific PM counter data */
   DSL_OUT DSL_PM_ReTxData_t data;
} DSL_PM_ReTxCountersTotal_t;

/**
   Retransmission performance initialization threshold.
   This structure has to be used for ioctl
   - \ref DSL_FIO_PM_RETX_THRESHOLDS_15MIN_SET
   - \ref DSL_FIO_PM_RETX_THRESHOLDS_1DAY_SET
   - \ref DSL_FIO_PM_RETX_THRESHOLDS_15MIN_GET
   - \ref DSL_FIO_PM_RETX_THRESHOLDS_1DAY_GET

   \note Even if the interface provides the direction parameter only near-end
   values are supported. Due to the fact that the ReTx stuff is not standardized
   at the moment it might be possible that there are also far end parameters
   supported in the future.
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies the direction (DSL_NEAR_END / DSL_FAR_END) to which the function
   will apply */
   DSL_IN DSL_XTUDir_t nDirection;
   /**
   Structure that contains line specific PM initialization threshold
   configuration data */
   DSL_OUT DSL_PM_ReTxData_t data;
} DSL_PM_ReTxThreshold_t;

/**
   Retransmission threshold crossing bitfields.
   \remarks The enums corresponds to bit field definitions.
*/
typedef enum
{
   /**
   Empty threshold crossing. */
   DSL_PM_RETXTHRESHCROSS_EMPTY = 0x00000000,
   /**
   nRxCorruptedTotal counter threshold crossing. */
   DSL_PM_RETXTHRESHCROSS_RX_CORRUPTED_TOTAL = 0x00000001,
   /**
   nRxUncorrectedProtected counter threshold crossing. */
   DSL_PM_RETXTHRESHCROSS_RX_UNCORR_PROT    = 0x00000002,
   /**
   nRxRetransmitted counter threshold crossing. */
   DSL_PM_RETXTHRESHCROSS_RX_RETX = 0x00000004,
   /**
   nRxCorrected counter threshold crossing. */
   DSL_PM_RETXTHRESHCROSS_RX_CORR = 0x00000008
} DSL_PM_BF_ReTxThresholdCrossing_t;

/**
   Retransmission counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   Number of seconds elapsed in the current 15 min interval. */
   DSL_uint16_t   nCurr15MinTime;
   /**
   Number of seconds elapsed in the current 1 day interval. */
   DSL_uint32_t   nCurr1DayTime;
   /**
   15 minutes channel counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ReTxThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day channel counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ReTxThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_PM_ReTxThresholdCrossing_t;

/* ************************************************************************** */
/* *  Common function interface                                             * */
/* ************************************************************************** */

/**
   Definitions for PM reset functionalities
*/
typedef enum
{
   /**
   Reset current, G.997.1 related counter values only.
   Related ioctl's are
   - DSL_FIO_PM_xxx_15MIN_GET with nHistoryInterval=0
   - DSL_FIO_PM_xxx_1DAY_GET with nHistoryInterval=0 */
   DSL_PM_RESET_CURRENT = 0,
   /**
   Reset history, G.997.1 related counter values only.
   Related ioctl's are
   - DSL_FIO_PM_xxx_15MIN_GET with nHistoryInterval=[1..max]
   - DSL_FIO_PM_xxx_1DAY_GET with nHistoryInterval=[1..max] */
   DSL_PM_RESET_HISTORY = 1,
   /**
   Reset the total counter values only.
   Related ioctl's are
   - DSL_FIO_PM_xxx_TOTAL_GET */
   DSL_PM_RESET_TOTAL = 2,
   /**
   Reset all PM counter values including
   - current and history for G.997.1 (15-minute and 1-day counters)
   - current and history for TR-98 (showtime counters)
   - total */
   DSL_PM_RESET_ALL = 3,
   /**
   Reset the current showtime, TR-98 related counter values only.
   Related ioctl's are
   - DSL_FIO_PM_xxx_SHOWTIME_GET with nHistoryInterval=0 */
   DSL_PM_RESET_CURRENT_SHOWTIME = 4,
   /**
   Reset the history showtime, TR-98 related counter values only.
   Related ioctl's are
   - DSL_FIO_PM_xxx_SHOWTIME_GET with nHistoryInterval=[1..max] */
   DSL_PM_RESET_HISTORY_SHOWTIME = 5
} DSL_PM_ResetTypes_t;

/**
   Bitmask definitions for detailed counter reset handling.
   \remarks The enums corresponds to bit field definitions.
*/
typedef enum
{
   /* Cleaned.
   No mask selected, NO counters are reset. */
   DSL_PM_RESETMASK_CLEANED = 0x00000000,
   /**
   Includes the line init counters for reset. Related ioctl's are
   DSL_FIO_PM_LINE_INIT_COUNTERS_xxx */
   DSL_PM_RESETMASK_LINE_INIT_COUNTERS = 0x00000001,
   /**
   Includes the line second counters for reset. Related ioctl's are
   DSL_FIO_PM_LINE_SEC_COUNTERS_xxx */
   DSL_PM_RESETMASK_LINE_SEC_COUNTERS = 0x00000002,
   /**
   Includes the line event showtime counters for reset. Related ioctl's are
   DSL_FIO_PM_LINE_FAILURE_COUNTERS_xxx */
   DSL_PM_RESETMASK_LINE_FAILURE_COUNTERS = 0x00000004,
   /**
   Includes the line event showtime counters for reset. Related ioctl's are
   DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_xxx */
   DSL_PM_RESETMASK_LINE_EVENT_SHOWTIME_COUNTERS = 0x00000008,
   /**
   Includes the channel counters for reset. Related ioctl's are
   DSL_FIO_PM_CHANNEL_COUNTERS_xxx */
   DSL_PM_RESETMASK_CHANNEL_COUNTERS = 0x00000100,
   /**
   Includes the data path counters for reset. Related ioctl's are
   DSL_FIO_PM_DATA_PATH_COUNTERS_xxx */
   DSL_PM_RESETMASK_DATA_PATH_COUNTERS = 0x00001000,
   /**
   Includes the data path failure counters for reset. Related ioctl's are
   DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_xxx */
   DSL_PM_RESETMASK_DATA_PATH_FAILURE_COUNTERS = 0x00002000,
   /**
   Includes the retransmission counters for reset. Related ioctl's are
   DSL_FIO_PM_RETX_COUNTERS_xxx */
   DSL_PM_RESETMASK_RETX_COUNTERS = 0x00010000,
} DSL_PM_BF_ResetMask_t;

/**
   Defines structure for reset of PM counters.
*/
typedef struct
{
   /**
   Specifies PM module reset type */
   DSL_IN DSL_PM_ResetTypes_t nResetType;
   /**
   Specifies to use a more detailed counter reset behavior.
   DSL_FALSE - Do not use detailed counter reset behavior (backward compatible mode)
   DSL_TRUE  - Use detailed counter reset behavior */
   DSL_IN DSL_boolean_t bUseDetailedReset;
   /**
   Provides a possibility to exclude dedicated counter groups from reset handling.
   If dedicated defined masks are activated the according counter values will
   be reset.
   \attention This parameter is only used in case of using detailed counter reset
              behavior (bUseDetailedReset=1). */
   DSL_IN DSL_PM_BF_ResetMask_t nResetMask;
} DSL_PM_ResetData_t;

/**
   Defines structure for reset of PM counters.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_RESET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains control information */
   DSL_IN DSL_PM_ResetData_t data;
} DSL_PM_Reset_t;

/**
   Defines structure for performing external trigger for elapsed intervals.
*/
typedef struct
{
   /**
   Specifies which type of interval has been elapsed.
   - DSL_FALSE: Only current 15 min interval has been elapsed
   - DSL_TRUE: Current 15 min and 1 day interval has been elapsed */
   DSL_IN DSL_boolean_t bOneDayElapsed;
} DSL_PM_ElapsedExtTriggerData_t;

/**
   Defines structure for performing external trigger for elapsed intervals.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_15MIN_ELAPSED_EXT_TRIGGER
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains control information */
   DSL_IN DSL_PM_ElapsedExtTriggerData_t data;
} DSL_PM_ElapsedExtTrigger_t;

/**
   Defines structure for performing a reset of the elapsed time.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_ELAPSED_TIME_RESET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
} DSL_PM_ElapsedTimeReset_t;

/**
   PM synchronization modes.
*/
typedef enum
{
   /**
   Free synchronization.
   PM will be synchronized to its startup time. After start of the PM no further
   synchronization to an external clock source is done. */
   DSL_PM_SYNC_MODE_FREE,
   /**
   System time synchronization.
   PM will be synchronized to the system time. The time base is derived from the
   DSL_SysTimeGet function (OS specific). */
   DSL_PM_SYNC_MODE_SYS_TIME,
   /**
   External synchronization.
   PM will be synchronized to the external time network time.
   The host application should call the the function DSL_PM_15MinElapsedExtTrigger
   each 15 minutes. In addition the bOneDayElapsed parameter should be set
   accordingly. */
   DSL_PM_SYNC_MODE_EXTERNAL
} DSL_PM_SyncModeType_t;

/**
   Defines structure for configuring the PM sync mode.
*/
typedef struct
{
   /**
   Specifies which type PM sync mode shall be used */
   DSL_IN DSL_PM_SyncModeType_t nMode;
} DSL_PM_SyncModeData_t;

/**
   Defines structure for configuring the PM sync mode.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_SYNC_MODE_SET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains control information */
   DSL_IN DSL_PM_SyncModeData_t data;
} DSL_PM_SyncMode_t;

/**
   Structure used for call back events to signal performance management
   synchronization events.
*/
typedef struct
{
   /**
   Indicates if the current 15 minutes interval has been closed and moved to
   the history buffer */
   DSL_boolean_t b15MinElapsed;
   /**
   Indicates if the current 1 day interval has been closed and moved to the
   history buffer */
   DSL_boolean_t b1DayElapsed;
} DSL_PM_SyncEvent_t;

/**
   Configuration structure for the burnin mode of the PM module.
*/
typedef struct
{
   /**
   PM tick interval in milli seconds, default is 1000 msec, mimimum is 500 msec.
   Each tick the PM database is updated through reading of the counters out
   of the firmware. */
   DSL_uint32_t    nPmTick;
   /**
   Number of seconds in the 15 minutes interval, default is 900.
   By decreasing this values the 15 minutes interval can be shorten. */
   DSL_uint32_t    nPm15Min;
   /**
   Number of 15 min intervals per day, default is 96 .
   By decreasing this values the 15 minutes per day interval can be shorten. */
   DSL_uint32_t    nPm15MinPerDay;
} DSL_PM_BurninModeType_t;

/**
   Defines structure for configuring the PM burn in mode
*/
typedef struct
{
   /**
   Specifies whether to switch burnin mode on or off.
   - DSL_FALSE: Burnin mode off (normal operation). In this case fixed, internal
     configuration is used which is equal to the default which are specified
     within \ref DSL_PM_BurninModeType_t
   - DSL_TRUE: Burnin mode on (debug mode). In this case the specified user
     values from nMode configuration settings are used. */
   DSL_IN DSL_boolean_t bActivate;
   /**
   Specifies the burn in mode configurations settings. These values are only
   active if the burn in mode is activated (bActivate = DSL_TRUE) */
   DSL_IN DSL_PM_BurninModeType_t nMode;
} DSL_PM_BurninModeData_t;

/**
   Defines structure for configuring the PM burn in mode.
   This structure has to be used for ioctl
   \ref DSL_FIO_PM_BURNIN_MODE_SET
*/
typedef struct
{
   /**
   Driver control/status structure */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Structure that contains control information */
   DSL_IN DSL_PM_BurninModeData_t data;
} DSL_PM_BurninMode_t;

#if defined(DSL_PM_DEBUG_MODE_ENABLE) && (DSL_PM_DEBUG_MODE_ENABLE > 0)
/**
   Configuration structure for the PM dump functions.
*/
typedef struct
{
   /**
   Specifies whether to print only the PM memory and data structure statistic
   (DSL_TRUE) or to print also the content of the counters (DSL_FALSE).
   If set to DSL_FALSE the output might be very extensive in accordance to the
   configured number of devices. */
   DSL_boolean_t bPrintMemStatOnly;
} DSL_PM_DumpConfig_t;

/**
   Defines structure for performing PM dump functionality.
*/
typedef struct
{
   /**
   Driver control/status structure. */
   DSL_IN_OUT DSL_AccessCtl_t accessCtl;
   /**
   Specifies whether switch burnin mode on or off
   - DSL_FALSE: Burnin mode off (normal operation)
   - DSL_TRUE: burnin mode on (debug mode) */
   DSL_IN DSL_PM_DumpConfig_t dumpConfig;
} DSL_PM_Dump_t;

#endif /* defined(DSL_PM_DEBUG_MODE_ENABLE) && (DSL_PM_DEBUG_MODE_ENABLE > 0) */


/** @} DRV_DSL_CPE_PM */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_API_PM_H */
