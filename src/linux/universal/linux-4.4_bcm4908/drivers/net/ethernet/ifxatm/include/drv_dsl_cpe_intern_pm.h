/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_INTERN_PM_H
#define _DRV_DSL_CPE_INTERN_PM_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_api_pm.h"

/** \file
   This file specifies the internal functions that are used for the Performance
   Monitoring specific implementation of the ioctl interface.
   It is intendet to be used within the DSL CPE_API driver ONLY.
*/

/** \addtogroup DRV_DSL_CPE_PM
 @{ */


#ifdef INCLUDE_DSL_CPE_PM_CONFIG
DSL_Error_t DSL_DRV_PM_ConfigSet(
   DSL_Context_t *pContext,
   DSL_PM_Config_t *pConfig);
#endif /* INCLUDE_DSL_CPE_PM_CONFIG*/

#ifdef INCLUDE_DSL_CPE_PM_CONFIG
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_PM_ConfigGet(
   DSL_Context_t *pContext,
   DSL_PM_Config_t *pConfig);
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_CPE_PM_CONFIG*/

/**
   Initialization routine for PM module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_PM_Start(
   DSL_Context_t *pContext);
#endif

/**
   Deinitialization routine for PM module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
#ifndef SWIG
DSL_Error_t DSL_DRV_PM_Stop(
   DSL_Context_t *pContext);
#endif

/* ************************************************************************** */
/* *  xDSL Line Endpoint interface (internal)                               * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsDir_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStats_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsDir_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStats_t *pStats
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters
);


/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters
);

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecCountersTotal_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCountersTotal_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_15MIN_SET
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds15MinSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_15MIN_SET
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds15MinSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_1DAY_SET
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds1DaySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_1DAY_SET
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds1DaySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_THRESHOLDS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_THRESHOLDS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitThreshold_t *pThreshs
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_SEC_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_LineSecCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_INIT_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_LineInitCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

/* ************************************************************************** */
/* *  xDSL Line Event Showtime Endpoint interface (internal)                  * */
/* ************************************************************************** */

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineEventShowtimeCounters_t *pCounters
);
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineFailureCounters_t *pCounters
);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineEventShowtimeCounters_t *pCounters
);
#ifdef INCLUDE_DEPRECATED
DSL_Error_t DSL_DRV_PM_LineFailureCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineFailureCounters_t *pCounters
);
#endif /* INCLUDE_DEPRECATED */

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineEventShowtimeCountersTotal_t *pCounters
);
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineFailureCountersTotal_t *pCounters
);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /** INCLUDE_DEPRECATED */
#endif /** INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineEventShowtimeCounters_t *pCounters
);
#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineFailureCounters_t *pCounters
);
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /** INCLUDE_DEPRECATED */
#endif /** INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS */

/* ************************************************************************** */
/* *  xDSL Channel Endpoint interface (internal)                            * */
/* ************************************************************************** */

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelCounters_t *pCounters
);

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
DSL_Error_t DSL_DRV_PM_ChannelCountersExt15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExt_t *pCounters);
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelCounters_t   *pCounters
);

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExt1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExt_t *pCounters
);
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelCountersTotal_t *pCounters
);

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/**
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExtTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExtTotal_t *pCounters
);
#endif /** defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_15MIN_SET
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds15MinSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_1DAY_SET
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds1DaySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_THRESHOLDS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelThreshold_t *pThreshs
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS */

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ChannelCounters_t *pCounters
);


#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_EXT_GET
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExtShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExt_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS */

/* ************************************************************************** */
/* *  xDSL Data-Path Endpoint interface (internal)                          * */
/* ************************************************************************** */

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathCounters_t *pCounters
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathCounters_t *pCounters
);

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathCountersTotal_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_THRESHOLDS_15MIN_SET
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds15MinSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_THRESHOLDS_1DAY_SET
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds1DaySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_THRESHOLDS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_THRESHOLDS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathThreshold_t *pThreshs
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS */

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathCounters_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS */

/* ************************************************************************** */
/* *  xDSL Data-Path Failure Endpoint interface (internal)                  * */
/* ************************************************************************** */

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathFailureCounters_t *pCounters
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathFailureCounters_t *pCounters
);

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathFailureCountersTotal_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DATA_PATH_FAILURE_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathFailureCounters_t *pCounters
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS */

/* ************************************************************************** */
/* *  xDSL ReTx Endpoint interface (internal)                               * */
/* ************************************************************************** */

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_HISTORY_STATS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxHistoryStats15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_HISTORY_STATS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxHistoryStats1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_HistoryStatsChDir_t *pStats
);
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_COUNTERS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxCounters_t *pCounters
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_COUNTERS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxCounters_t *pCounters
);

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_COUNTERS_TOTAL_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxCountersTotalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxCountersTotal_t *pCounters
);
#endif /** INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_THRESHOLDS_15MIN_SET
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds15MinSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_THRESHOLDS_1DAY_SET
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds1DaySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_THRESHOLDS_15MIN_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxThreshold_t *pThreshs
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_THRESHOLDS_1DAY_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxThreshold_t *pThreshs
);
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RETX_COUNTERS_SHOWTIME_GET
*/
DSL_Error_t DSL_DRV_PM_ReTxCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxCounters_t *pCounters
);
#endif /** INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /** INCLUDE_DSL_CPE_PM_RETX_COUNTERS */

/* ************************************************************************** */
/* *  Common function interface (internal)                                  * */
/* ************************************************************************** */

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_RESET
*/
DSL_Error_t DSL_DRV_PM_Reset(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_Reset_t *pResetType
);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_15MIN_ELAPSED_EXT_TRIGGER
*/
DSL_Error_t DSL_DRV_PM_15MinElapsedExtTrigger(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ElapsedExtTrigger_t *pTrigger
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_ELAPSED_TIME_RESET
*/
DSL_Error_t DSL_DRV_PM_ElapsedTimeReset(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ElapsedTimeReset_t *pReset
);

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_SYNC_MODE_SET
*/
DSL_Error_t DSL_DRV_PM_SyncModeSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_SyncMode_t *pMode
);

#ifdef INCLUDE_DSL_CONFIG_GET
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_SYNC_MODE_GET
*/
DSL_Error_t DSL_DRV_PM_SyncModeGet(
   DSL_Context_t *pContext,
   DSL_PM_SyncMode_t *pMode
);
#endif /** INCLUDE_DSL_CONFIG_GET*/

/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_BURNIN_MODE_SET
*/
DSL_Error_t DSL_DRV_PM_BurninModeSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_BurninMode_t *pBurninMode
);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#if defined(DSL_PM_DEBUG_MODE_ENABLE) && (DSL_PM_DEBUG_MODE_ENABLE > 0)
/**
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_DUMP
*/
DSL_Error_t DSL_DRV_PM_Dump(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_Dump_t *pData
);

#endif /* defined(DSL_PM_DEBUG_MODE_ENABLE) && (DSL_PM_DEBUG_MODE_ENABLE > 0) */

/** @} DRV_DSL_CPE_PM */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_PM_H */
