/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_PM_CORE_H
#define _DRV_DSL_CPE_PM_CORE_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api.h"

#if defined(INCLUDE_DSL_PM)

/** \file
   This file specifies the internal functions that are used for the Performance
   Monitoring specific implementation of the ioctl interface.
   It is intendet to be used within the DSL CPE_API driver ONLY.
*/

/** \addtogroup DRV_DSL_CPE_PM
 @{ */

#if defined(INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS)
   #if !defined(INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS)
      #error "PM module can't support Channel thresholds while the Channel Counters are disabled!"
   #endif
#endif

#if defined(INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS)
   #if !defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS)
      #error "PM module can't support Line thresholds while the Line Counters are disabled!"
   #endif
#endif

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS)
   #if !defined(INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS)
      #error "PM module can't support Data Path thresholds while the Data Path Counters are disabled!"
   #endif
#endif

#if defined(INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS)
   #if !defined(INCLUDE_DSL_CPE_PM_RETX_COUNTERS)
      #error "PM module can't support ReTx thresholds while the ReTx Counters are disabled!"
   #endif
#endif

#if !defined(INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS) && \
    !defined(INCLUDE_DSL_CPE_PM_LINE_COUNTERS) && \
    !defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS) && \
    !defined(INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS) && \
    !defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS) && \
    !defined(INCLUDE_DSL_CPE_PM_RETX_COUNTERS) && \
     defined(INCLUDE_DSL_PM)
   #error "None of the PM module counters are enabled while the PM module is enabled!"
#endif

#if !defined(INCLUDE_DSL_CPE_PM_HISTORY) && !defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS) && \
    !defined(INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS) && defined(INCLUDE_DSL_PM)
   #error "Please enable PM History or PM Showtime or Total counters in the enabled PM module"
#endif

#if !defined (INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS) && defined(INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   #error "PM Channel Extended counters are not supported without common Channel counters!"
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS)*/

#if !defined (INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   #error "PM Channel Extended counters are supported only for Danube family!"
#endif /* !defined (INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
#define DSL_PM_COUNTER_ES_MAX_VALUE               ((DSL_uint16_t)0xFFFF)
#define DSL_PM_COUNTER_LOFS_MAX_VALUE             ((DSL_uint16_t)0xFFFF)
#define DSL_PM_COUNTER_LOSS_MAX_VALUE             ((DSL_uint16_t)0xFFFF)
#define DSL_PM_COUNTER_SES_MAX_VALUE              ((DSL_uint16_t)0xFFFF)
#define DSL_PM_COUNTER_UAS_MAX_VALUE              ((DSL_uint16_t)0xFFFF)

#define DSL_PM_COUNTER_FEC_MAX_VALUE              ((DSL_uint16_t)0xFFFF)
#define DSL_PM_COUNTER_CODEVIOLATIONS_MAX_VALUE   ((DSL_uint16_t)0xFFFF)
#else
#define DSL_PM_COUNTER_ES_MAX_VALUE               ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_LOFS_MAX_VALUE             ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_LOSS_MAX_VALUE             ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_SES_MAX_VALUE              ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_UAS_MAX_VALUE              ((DSL_uint32_t)0xFFFFFFFF)

#define DSL_PM_COUNTER_FEC_MAX_VALUE              ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_CODEVIOLATIONS_MAX_VALUE   ((DSL_uint32_t)0xFFFFFFFF)
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

#define DSL_PM_COUNTER_SUPERFRAME_MAX_VALUE       ((DSL_uint32_t)0xFFFFFFFF)

#define DSL_PM_COUNTER_CRCP_MAX_VALUE             ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_CRCPP_MAX_VALUE            ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_CVP_MAX_VALUE              ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_CVPP_MAX_VALUE             ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_HEC_MAX_VALUE              ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_IBE_MAX_VALUE              ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_TOTALCELL_MAX_VALUE        ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_UTOTALCELL_MAX_VALUE       ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_TX_UTOTALCELL_MAX_VALUE    ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_TX_IBE_MAX_VALUE           ((DSL_uint32_t)0xFFFFFFFF)

#define DSL_PM_COUNTER_FFINIT_MAX_VALUE           ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_FSINIT_MAX_VALUE           ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_FINIT_MAX_VALUE            ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_SINIT_MAX_VALUE            ((DSL_uint32_t)0xFFFFFFFF)

#define DSL_PM_COUNTER_LOS_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_LOF_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_LPR_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_LOM_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_SOS_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)

#define DSL_PM_COUNTER_NCD_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_LCD_FAILURE_MAX_VALUE      ((DSL_uint32_t)0xFFFFFFFF)

#define DSL_PM_COUNTER_RX_CORRUPTED_TOTAL_MAX_VALUE ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_RX_UNCORR_PROT_MAX_VALUE     ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_RX_RETX_MAX_VALUE            ((DSL_uint32_t)0xFFFFFFFF)
#define DSL_PM_COUNTER_RX_CORR_MAX_VALUE            ((DSL_uint32_t)0xFFFFFFFF)

/** PM module thread name*/
#define DSL_PM_NE_TASK_NAME   "cpe_pm_ne"
#define DSL_PM_FE_TASK_NAME   "cpe_pm_fe"

/** PM module thread minimum poll time (msec)*/
#define DSL_PM_COUNTER_MIN_POLLING_CYCLE   (500)

#define DSL_PM_MIN_BASIC_UPDATE_CYCLE   (1)
#define DSL_PM_MAX_BASIC_UPDATE_CYCLE   (10)

#define DSL_PM_MIN_FE_UPDATE_CYCLE_FACTOR   (1)
#define DSL_PM_MAX_FE_UPDATE_CYCLE_FACTOR   (10)

#define DSL_PM_MIN_FE_UPDATE_CYCLE_FACTOR_L2   (1)
#define DSL_PM_MAX_FE_UPDATE_CYCLE_FACTOR_L2   (20)

/** PM module thread default poll time (msec)*/
#define DSL_PM_COUNTER_POLLING_CYCLE   (1000)

#define DSL_PM_COUNTER_FE_POLLING_FACTOR   (10)

/** number of seconds in the 15 minutes interval. */
#define DSL_PM_15MIN   (900)

#if ((DSL_PM_15MIN * 1000) < DSL_PM_COUNTER_POLLING_CYCLE) || \
      ((DSL_PM_15MIN * 1000) < \
      (DSL_PM_COUNTER_FE_POLLING_FACTOR * DSL_PM_COUNTER_POLLING_CYCLE))
#error PM counters polling time is incorrect, please fix it
#endif

/** number of 15 min intervals per day */
#define DSL_PM_15MIN_PER_DAY   (96)

/** number of msec in the 1 day interval. */
#define DSL_PM_1DAY   (DSL_PM_15MIN*DSL_PM_15MIN_PER_DAY)

#define DSL_PM_MSEC   (1000)

#define DSL_DRV_PM_TIME_GET() DSL_DRV_TimeMSecGet()
#define DSL_DRV_PM_SYS_TIME_GET() DSL_DRV_SysTimeGet(0)

/** Helper macro for PM context retrieving from \ref DSL_Context_t structure. */
#define DSL_DRV_PM_CONTEXT(X)   ((DSL_PM_Context*)((X)->PM))

#define DSL_DRV_PM_INTERVAL_FAILURE_SET(x,failure)   ((x) |= (failure))
#define DSL_DRV_PM_INTERVAL_FAILURE_CLR(x,failure)   ((x) &= ((DSL_pmBF_IntervalFailures_t)(~(failure))))

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS(ptr, channel, dir) \
      (dir==DSL_NEAR_END ? &(ptr->data_ne[channel]) : &(ptr->data_fe[channel]))

   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_EXT(ptr, channel) \
      (&(ptr->data_ne_ext[channel]))

   /**
      Macro to get access to the current channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_CURR(channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recCurr.data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recCurr.data_fe[channel])

   /**
      Macro to get access to the current extended channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_CURR_EXT(channel) \
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recCurr.data_ne_ext[channel]

   /**
      Macro to get access to 15 min channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec15min[idx].data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec15min[idx].data_fe[channel])

   /**
      Macro to get access to 15 min extended channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN_EXT(idx,channel) \
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec15min[idx].data_ne_ext[channel]

   /**
      Macro to get access to 1 day channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec1day[idx].data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec1day[idx].data_fe[channel])

   /**
      Macro to get access to 1 day extended channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY_EXT(idx,channel) \
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec1day[idx].data_ne_ext[channel]

   /**
      Macro to get access to Showtime channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_SHOWTIME(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recShowtime[idx].data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recShowtime[idx].data_fe[channel])

   /**
      Macro to get access to Showtime extended channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_SHOWTIME_EXT(idx,channel) \
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recShowtime[idx].data_ne_ext[channel]

   /**
      Macro to get access to total channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_TOTAL(channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recTotal.data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recTotal.data_fe[channel])

   /**
      Macro to get access to total channel counters in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_COUNTERS_TOTAL_EXT(channel) \
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recTotal.data_ne_ext[channel]

   /**
         Macro to get access to the 15-min channel history data in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.hist15min)

   /**
      Macro to get access to the 1-day channel history data in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.hist1day)

   /**
      Macro to get access to the Showtime channel history data in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.histShowtime)


   /**
         Macro to get access to the channel threshold indication data in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_IND(channel, dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholdInd.ind_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholdInd.ind_fe[channel]))

   /**
         Macro to get access to the channel 15-min interval thresholds data in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_15MIN(channel, dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholds15min.data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholds15min.data_fe[channel]))

   /**
         Macro to get access to the channel 1-day interval thresholds data in the PM context */
   #define DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_1DAY(channel, dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholds1day.data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholds1day.data_fe[channel]))
#endif /** #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   #define DSL_DRV_PM_PTR_DATAPATH_COUNTERS(ptr, dir) \
      (dir==DSL_NEAR_END ? ptr->data_ne : ptr.data_fe)

   /**
      Macro to get access to the current channel counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_COUNTERS_CURR(channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recCurr.data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recCurr.data_fe[channel])

   /**
      Macro to get access to 15 min data path counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_COUNTERS_15MIN(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.rec15min[idx].data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.rec15min[idx].data_fe[channel]))

   /**
      Macro to get access to 1 day data path counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_COUNTERS_1DAY(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.rec1day[idx].data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.rec1day[idx].data_fe[channel]))

   /**
      Macro to get access to Showtime data path counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_COUNTERS_SHOWTIME(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recShowtime[idx].data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recShowtime[idx].data_fe[channel]))

   /**
      Macro to get access to total channel counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_COUNTERS_TOTAL(channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recTotal.data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recTotal.data_fe[channel]))

   /**
      Macro to get access to the 15-min data path history data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.hist15min)

   /**
      Macro to get access to the 1-day data path history data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.hist1day)

   /**
      Macro to get access to the 1-day data path history data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_HISTORY_SHOWTIME() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.histShowtime)

   /**
      Macro to get access to the data path threshold indication data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_IND(channel, dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholdInd.ind_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholdInd.ind_fe[channel]))

   /**
      Macro to get access to the data path 15-min interval thresholds data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_15MIN(channel, dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholds15min.data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholds15min.data_fe[channel]))

   /**
      Macro to get access to the data path 1-day interval thresholds data in the PM context
   */
   #define DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_1DAY(channel, dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholds1day.data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholds1day.data_fe[channel]))
#endif /** #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS(ptr, dir) \
      (dir==DSL_NEAR_END ? ptr->data_ne : ptr.data_fe)

   /**
      Macro to get access to the current data path failure counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_CURR(channel,dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recCurr.data_ne[channel]:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recCurr.data_fe[channel])

   /**
      Macro to get access to 15 min data path failure counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_15MIN(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.rec15min[idx].data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.rec15min[idx].data_fe[channel]))

   /**
      Macro to get access to 1 day data path failure counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_1DAY(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.rec1day[idx].data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.rec1day[idx].data_fe[channel]))

   /**
      Macro to get access to Showtime data path failure counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_SHOWTIME(idx,channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recShowtime[idx].data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recShowtime[idx].data_fe[channel]))

   /**
      Macro to get access to total data path failure counters in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_TOTAL(channel,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recTotal.data_ne[channel]):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recTotal.data_fe[channel]))

   /**
      Macro to get access to the 15-min data path failure history data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.hist15min)

   /**
      Macro to get access to the 1-day data path failure history data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.hist1day)

   /**
      Macro to get access to the 1-day data path failure history data in the PM context */
   #define DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_SHOWTIME() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.histShowtime)
#endif /** #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS(ptr, dir) \
      (dir==DSL_NEAR_END ? ptr->data_ne : ptr.data_fe)

   /**
      Macro to get access to the current line Event Showtime counters in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_CURR(dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recCurr.data_ne:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recCurr.data_fe)

   /**
      Macro to get access to 15 min line Event Showtime counters in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_15MIN(idx,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.rec15min[idx].data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.rec15min[idx].data_fe))

   /**
      Macro to get access to 1 day line Event Showtime counters in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_1DAY(idx,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.rec1day[idx].data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.rec1day[idx].data_fe))

   /**
      Macro to get access to Showtime line Event Showtime counters in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME(idx,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recShowtime[idx].data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recShowtime[idx].data_fe))

   /**
      Macro to get access to total line Event Showtime counters in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_TOTAL(dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recTotal.data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recTotal.data_fe))

   /**
      Macro to get access to the 15-min line Event Showtime history data in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.hist15min)

   /**
      Macro to get access to the 1-day line Event Showtime history data in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.hist1day)

   /**
      Macro to get access to the 1-day line Event Showtime history data in the PM context */
   #define DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_SHOWTIME() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.histShowtime)
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   #define DSL_DRV_PM_PTR_RETX_COUNTERS(ptr, dir) \
      (dir==DSL_NEAR_END ? ptr->data_ne : ptr.data_fe)

   /**
      Macro to get access to the current ReTx counters in the PM context */
   #define DSL_DRV_PM_PTR_RETX_COUNTERS_CURR(dir) \
      (dir==DSL_NEAR_END?\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recCurr.data_ne:\
      &DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recCurr.data_fe)

   /**
      Macro to get access to 15 min ReTx counters in the PM context */
   #define DSL_DRV_PM_PTR_RETX_COUNTERS_15MIN(idx,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.rec15min[idx].data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.rec15min[idx].data_fe))

   /**
      Macro to get access to 1 day ReTx counters in the PM context */
   #define DSL_DRV_PM_PTR_RETX_COUNTERS_1DAY(idx,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.rec1day[idx].data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.rec1day[idx].data_fe))

   /**
      Macro to get access to Showtime ReTx counters in the PM context */
   #define DSL_DRV_PM_PTR_RETX_COUNTERS_SHOWTIME(idx,dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recShowtime[idx].data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recShowtime[idx].data_fe))

   /**
      Macro to get access to total ReTx counters in the PM context */
   #define DSL_DRV_PM_PTR_RETX_COUNTERS_TOTAL(dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recTotal.data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recTotal.data_fe))

   /**
      Macro to get access to the 15-min ReTx history data in the PM context */
   #define DSL_DRV_PM_PTR_RETX_HISTORY_15MIN() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.hist15min)

   /**
      Macro to get access to the 1-day ReTx history data in the PM context */
   #define DSL_DRV_PM_PTR_RETX_HISTORY_1DAY() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.hist1day)

   /**
      Macro to get access to the Showtime ReTx history data in the PM context */
   #define DSL_DRV_PM_PTR_RETX_HISTORY_SHOWTIME() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.histShowtime)

   /**
      Macro to get access to the ReTx threshold indication data in the PM context */
   #define DSL_DRV_PM_PTR_RETX_THRESHOLD_IND(dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholdInd.ind_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholdInd.ind_fe))

   /**
      Macro to get access to the ReTx 15-min interval thresholds data in the PM context */
   #define DSL_DRV_PM_PTR_RETX_THRESHOLD_15MIN(dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholds15min.data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholds15min.data_fe))

   /**
      Macro to get access to the ReTx 1-day interval thresholds data in the PM context
   */
   #define DSL_DRV_PM_PTR_RETX_THRESHOLD_1DAY(dir) \
      (dir==DSL_NEAR_END?\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholds1day.data_ne):\
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholds1day.data_fe))
#endif /** #ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

#define DSL_DRV_PM_PTR_LINE_SEC_COUNTERS(ptr,dir) \
   (dir==DSL_NEAR_END ? &(ptr->sec_ne) : &(ptr->sec_fe))

/**
      Macro to get access to the current line sec counters in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_CURR(dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recCurr.sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recCurr.sec_fe))

/**
      Macro to get access to 15 min line sec counters in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_15MIN(idx,dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.rec15min[idx].sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.rec15min[idx].sec_fe))


/**
      Macro to get access to 1 day line sec counters in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_1DAY(idx,dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.rec1day[idx].sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.rec1day[idx].sec_fe))

/**
      Macro to get access to Showtime line sec counters in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_SHOWTIME(idx,dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recShowtime[idx].sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recShowtime[idx].sec_fe))

/**
   Macro to get access to total line sec counters in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_TOTAL(dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recTotal.sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recTotal.sec_fe))

/**
   Macro to get access to line sec aux data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_AUX_DATA(dir) \
  (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData.sec_data_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData.sec_data_fe))

/**
   Macro to get access to the 15-min line sec history data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN() \
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.hist15min)

/**
   Macro to get access to the 1-day line sec history data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY() \
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.hist1day)

/**
   Macro to get access to the Showtime line sec history data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_HISTORY_SHOWTIME() \
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.histShowtime)

/**
   Macro to get access to the 15-min line init history data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN() \
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.hist15min)

/**
   Macro to get access to the 1-day line init history data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY() \
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.hist1day)

/**
   Macro to get access to the Showtime line init history data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_INIT_HISTORY_SHOWTIME() \
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.histShowtime)


/**
   Macro to get access to the line sec threshold indication data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_IND(dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholdInd.ind_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholdInd.ind_fe))

/**
   Macro to get access to the line init threshold indication data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_IND(dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.thresholdInd.ind_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.thresholdInd.ind_fe))

   /**
      Macro to get access to the line 15-min interval thresholds data in the PM context
   */
#define DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_15MIN(dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholds15min.sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholds15min.sec_fe))

/**
   Macro to get access to the line 1-day interval thresholds data in the PM context
*/
#define DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_1DAY(dir) \
   (dir==DSL_NEAR_END?\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholds1day.sec_ne):\
   &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholds1day.sec_fe))


#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   #define DSL_DRV_PM_PTR_LINE_INIT_COUNTERS(ptr) &(ptr->init)

   /**
      Macro to get access to current line init counters in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_CURR() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.recCurr.init)

   /**
      Macro to get access to 15 min line init counters in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_15MIN(idx) \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.rec15min[idx].init)

   /**
      Macro to get access to 1 day line init counters in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_1DAY(idx) \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.rec1day[idx].init)

   /**
      Macro to get access to Showtime line init counters in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_SHOWTIME(idx) \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.recShowtime[idx].init)

   /**
      Macro to get access to total line init counters in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_TOTAL() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.recTotal.init)

   /**
      Macro to get access to the line 15-min interval thresholds data in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_15MIN() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.thresholds15min.init)

   /**
      Macro to get access to the line 1-day interval thresholds data in the PM context
   */
   #define DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_1DAY() \
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.thresholds1day.init)
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

typedef enum
{
   /** Cleaned*/
   DSL_PM_INTERVAL_FAILURE_CLEANED                 = 0x00000000,
   /** Interval Invalid due to the current timeframe is not complete*/
   DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE            = 0x00000001,
   /** Interval Invalid due to the switched off NE FW polling*/
   DSL_PM_INTERVAL_FAILURE_NE_POLLING_SWITCHED_OFF = 0x00000002,
   /** Interval Invalid due to the switched off FE FW polling*/
   DSL_PM_INTERVAL_FAILURE_FE_POLLING_SWITCHED_OFF = 0x00000004,
   /** Interval Invalid due to the inconsistency of the NE FW polling*/
   DSL_PM_INTERVAL_FAILURE_NE_POLLING_INCOMPLETE   = 0x00000008,
   /** Interval Invalid due to the inconsistency of the FE FW polling*/
   DSL_PM_INTERVAL_FAILURE_FE_POLLING_INCOMPLETE   = 0x00000010,
   /** Interval Invalid due to the inconsistency of the interval time*/
   DSL_PM_INTERVAL_FAILURE_INTERVAL_TIME_INVALID   = 0x00000020
} DSL_pmBF_IntervalFailures_t;

#define  DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK \
   (DSL_PM_INTERVAL_FAILURE_NE_POLLING_SWITCHED_OFF | \
    DSL_PM_INTERVAL_FAILURE_NE_POLLING_INCOMPLETE | DSL_PM_INTERVAL_FAILURE_INTERVAL_TIME_INVALID)

#define  DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK \
   (DSL_PM_INTERVAL_FAILURE_FE_POLLING_SWITCHED_OFF | \
    DSL_PM_INTERVAL_FAILURE_FE_POLLING_INCOMPLETE | DSL_PM_INTERVAL_FAILURE_INTERVAL_TIME_INVALID)

/** PM History control structure */
typedef struct
{
   /**
   Maximum number of items in the history. */
   DSL_uint32_t     historySize;
   /**
   Current item offset. */
   DSL_uint32_t     curItem;
   /**
   Current number of items in the history. */
   DSL_uint32_t     itemsNum;
} DSL_pmHistory_t;

/**
   Line Sec Counters data structure. Includes Second counters for the
   Far-End and Near-End directions
*/
typedef struct
{
   /** Far-End data*/
   DSL_PM_LineSecData_t  sec_fe;
   /** Near-End data*/
   DSL_PM_LineSecData_t  sec_ne;
} DSL_pmLineSecData_t;

/**
   Line Init Counters data structure. Includes Init counters for the
   Near-End direction only.
*/
typedef struct
{
   /** Near-End data*/
   DSL_PM_LineInitData_t init;
} DSL_pmLineInitData_t;


typedef struct
{
   DSL_uint32_t  nLOFBegin;
   DSL_uint32_t  nLOFIntern;
   DSL_uint32_t  nLOFCurr;
   DSL_uint32_t  nUASBegin;
   DSL_boolean_t bUASStart;
   DSL_uint32_t  nUASIntern;
   DSL_uint32_t  nUASCurr;
   DSL_uint32_t  nUASLast;
} DSL_pmLineSecAuxData_t;

typedef struct
{
   DSL_pmLineSecAuxData_t sec_data_ne;
   DSL_pmLineSecAuxData_t sec_data_fe;
} DSL_pmLineAuxData_t;

/**
   Line counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   15 minutes data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_LineThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_LineThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_pmLineThresholdCrossingData_t;

typedef struct
{
   /** Near-End data*/
   DSL_pmLineThresholdCrossingData_t ind_ne;
   /** Far-End data*/
   DSL_pmLineThresholdCrossingData_t ind_fe;
} DSL_pmLineThresholdCrossing_t;

/**
   PM Line Sec Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_LINE_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineSecData_t rec15min[DSL_PM_LINE_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_LINE_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_LINE_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_LINE_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineSecData_t rec1day[DSL_PM_LINE_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_LINE_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_LINE_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmLineSecData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_LINE_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineSecData_t recShowtime[DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
   /** Total data record*/
   DSL_pmLineSecData_t recTotal;
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /** 15-min interval thresholds*/
   DSL_pmLineSecData_t thresholds15min;
   /** 1-day interval thresholds*/
   DSL_pmLineSecData_t thresholds1day;
   /** Thresholds indication bitmask*/
   DSL_pmLineThresholdCrossing_t thresholdInd;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
   /** Line Aux data*/
   DSL_pmLineAuxData_t auxData;
} DSL_PM_LineSecCountersData_t;

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
/**
   PM Line Init Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_LINE_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineInitData_t rec15min[DSL_PM_LINE_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_LINE_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_LINE_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_LINE_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineInitData_t rec1day[DSL_PM_LINE_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_LINE_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_LINE_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmLineInitData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_LINE_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineInitData_t recShowtime[DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Total data record*/
   DSL_pmLineInitData_t recTotal;
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /** 15-min interval thresholds*/
   DSL_pmLineInitData_t thresholds15min;
   /** 1-day interval thresholds*/
   DSL_pmLineInitData_t thresholds1day;
   /** Thresholds indication bitmask*/
   DSL_pmLineThresholdCrossing_t thresholdInd;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
} DSL_PM_LineInitCountersData_t;
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS

/**
   Channel Counters data structure. Includes data for both Far-End and Near-End
   directions
*/
typedef struct
{
   /** Far-End data*/
   DSL_PM_ChannelData_t data_fe[DSL_CHANNELS_PER_LINE];
   /** Near-End data*/
   DSL_PM_ChannelData_t data_ne[DSL_CHANNELS_PER_LINE];
#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   /** Near-End extended data*/
   DSL_PM_ChannelDataExt_t data_ne_ext[DSL_CHANNELS_PER_LINE];
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
} DSL_pmChannelData_t;

/**
   Channel counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   15 minutes data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ChannelThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ChannelThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_pmChannelThresholdCrossingData_t;

typedef struct
{
   /** Near-End data*/
   DSL_pmChannelThresholdCrossingData_t ind_ne[DSL_CHANNELS_PER_LINE];
   /** Far-End data*/
   DSL_pmChannelThresholdCrossingData_t ind_fe[DSL_CHANNELS_PER_LINE];
} DSL_pmChannelThresholdCrossing_t;

/**
   PM Channel Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_CHANNEL_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmChannelData_t rec15min[DSL_PM_CHANNEL_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_CHANNEL_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_CHANNEL_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_CHANNEL_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmChannelData_t rec1day[DSL_PM_CHANNEL_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_CHANNEL_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_CHANNEL_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmChannelData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmChannelData_t recShowtime[DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Total data record*/
   DSL_pmChannelData_t recTotal;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /** 15-min interval thresholds*/
   DSL_pmChannelData_t thresholds15min;
   /** 1-day interval thresholds*/
   DSL_pmChannelData_t thresholds1day;
   /** Thresholds indication bitmask*/
   DSL_pmChannelThresholdCrossing_t thresholdInd;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS*/
} DSL_PM_ChannelCountersData_t;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
/**
   Data Path Counters data structure. Includes data for both Far-End and Near-End
   directions
*/
typedef struct
{
   /** Far-End data*/
   DSL_PM_DataPathData_t data_fe[DSL_CHANNELS_PER_LINE];
   /** Near-End data*/
   DSL_PM_DataPathData_t data_ne[DSL_CHANNELS_PER_LINE];
} DSL_pmDataPathData_t;

/**
   Data path counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   15 minutes data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_DataPathThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day data path counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_DataPathThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_pmDataPathThresholdCrossingData_t;

typedef struct
{
   /** Near-End data*/
   DSL_pmDataPathThresholdCrossingData_t ind_ne[DSL_CHANNELS_PER_LINE];
   /** Far-End data*/
   DSL_pmDataPathThresholdCrossingData_t ind_fe[DSL_CHANNELS_PER_LINE];
} DSL_pmDataPathThresholdCrossing_t;

/**
   PM Data Path Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_DATAPATH_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmDataPathData_t rec15min[DSL_PM_DATAPATH_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_DATAPATH_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_DATAPATH_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_DATAPATH_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmDataPathData_t rec1day[DSL_PM_DATAPATH_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_DATAPATH_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_DATAPATH_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmDataPathData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmDataPathData_t recShowtime[DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Total data record*/
   DSL_pmDataPathData_t recTotal;
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /** 15-min interval thresholds*/
   DSL_pmDataPathData_t thresholds15min;
   /** 1-day interval thresholds*/
   DSL_pmDataPathData_t thresholds1day;
   /** Thresholds indication bitmask*/
   DSL_pmDataPathThresholdCrossing_t thresholdInd;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /** #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS*/
} DSL_PM_DataPathCountersData_t;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
/**
   Line Event Showtime Counters data structure.
   Includes data for both Far-End and Near-End directions
*/
typedef struct
{
   /** Far-End data*/
   DSL_PM_LineEventShowtimeData_t data_fe;
   /** Near-End data*/
   DSL_PM_LineEventShowtimeData_t data_ne;
} DSL_pmLineEventShowtimeData_t;

/**
   PM Line Event Showtime Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineEventShowtimeData_t rec15min[DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineEventShowtimeData_t rec1day[DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmLineEventShowtimeData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmLineEventShowtimeData_t recShowtime[DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Total data record*/
   DSL_pmLineEventShowtimeData_t recTotal;
} DSL_PM_LineEventShowtimeCountersData_t;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
/**
   Data Path Failure Counters data structure.
   Includes data for both Far-End and Near-End directions
*/
typedef struct
{
   /** Far-End data*/
   DSL_PM_DataPathFailureData_t data_fe[DSL_CHANNELS_PER_LINE];
   /** Near-End data*/
   DSL_PM_DataPathFailureData_t data_ne[DSL_CHANNELS_PER_LINE];
} DSL_pmDataPathFailureData_t;

/**
   PM Data Path Failure Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmDataPathFailureData_t rec15min[DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmDataPathFailureData_t rec1day[DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmDataPathFailureData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmDataPathFailureData_t recShowtime[DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Total data record*/
   DSL_pmDataPathFailureData_t recTotal;
} DSL_PM_DataPathFailureCountersData_t;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
/**
   ReTx Counters data structure. Includes data for both Far-End and Near-End
   directions
*/
typedef struct
{
   /** Far-End data*/
   DSL_PM_ReTxData_t data_fe;
   /** Near-End data*/
   DSL_PM_ReTxData_t data_ne;
} DSL_pmReTxData_t;

/**
   ReTx counters threshold crossing indication structure.
*/
typedef struct
{
   /**
   15 minutes ReTx counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ReTxThresholdCrossing_t).*/
   DSL_uint32_t   n15Min;
   /**
   1 day ReTx counters threshold crossing indication bitmask
   (refer to \ref DSL_PM_BF_ReTxThresholdCrossing_t).*/
   DSL_uint32_t   n1Day;
} DSL_pmReTxThresholdCrossingData_t;

typedef struct
{
   /** Near-End data*/
   DSL_pmReTxThresholdCrossingData_t ind_ne;
   /** Far-End data*/
   DSL_pmReTxThresholdCrossingData_t ind_fe;
} DSL_pmReTxThresholdCrossing_t;

/**
   PM ReTx Counters Data
*/
typedef struct
{
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** History intervals information for 15-min*/
   DSL_pmHistory_t hist15min;
   /** History intervals information for 1-day*/
   DSL_pmHistory_t hist1day;
   /** 15-min interval records. Contains DSL_PM_RETX_15MIN_RECORDS_NUM history
   records and one current record*/
   DSL_pmReTxData_t rec15min[DSL_PM_RETX_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes history statistics*/
   DSL_uint32_t n15minTimeHist[DSL_PM_RETX_15MIN_RECORDS_NUM + 1];
   /** 15-min timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n15minInvalidHist[DSL_PM_RETX_15MIN_RECORDS_NUM + 1];
   /** 1-day interval records. Contains DSL_PM_RETX_1DAY_RECORDS_NUM history
   records and one current record*/
   DSL_pmReTxData_t rec1day[DSL_PM_RETX_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes history statistics*/
   DSL_uint32_t n1dayTimeHist[DSL_PM_RETX_1DAY_RECORDS_NUM + 1];
   /** 1-day timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t n1dayInvalidHist[DSL_PM_RETX_1DAY_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
   /** Current data record. Contains values requested from the FW in the last polling cycle*/
   DSL_pmReTxData_t recCurr;
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** History intervals information for the Showtime intervals*/
   DSL_pmHistory_t histShowtime;
   /** Showtime interval records. Contains DSL_PM_RETX_SHOWTIME_RECORDS_NUM history
   records and one current record*/
   DSL_pmReTxData_t recShowtime[DSL_PM_RETX_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes history statistics*/
   DSL_uint32_t nShowtimeTimeHist[DSL_PM_RETX_SHOWTIME_RECORDS_NUM + 1];
   /** Showtime timeframes invalid indication history statistics*/
   DSL_pmBF_IntervalFailures_t nShowtimeInvalidHist[DSL_PM_RETX_SHOWTIME_RECORDS_NUM + 1];
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Total data record*/
   DSL_pmReTxData_t recTotal;
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /** 15-min interval thresholds*/
   DSL_pmReTxData_t thresholds15min;
   /** 1-day interval thresholds*/
   DSL_pmReTxData_t thresholds1day;
   /** Thresholds indication bitmask*/
   DSL_pmReTxThresholdCrossing_t thresholdInd;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS */
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS */
} DSL_PM_ReTxCountersData_t;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS */

/**
   PM counters data. Encapsulates all required counters data information
*/
typedef struct
{
   /** PM Line Sec Counters data*/
   DSL_PM_LineSecCountersData_t lineSecCounters;
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   /** PM Line Init Counters data*/
   DSL_PM_LineInitCountersData_t lineInitCounters;
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   /** PM Line Event Showtime Counters data*/
   DSL_PM_LineEventShowtimeCountersData_t lineEventShowtimeCounters;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   /** PM Channel Counters data*/
   DSL_PM_ChannelCountersData_t channelCounters;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   /** PM Data Path Counters data*/
   DSL_PM_DataPathCountersData_t dataPathCounters;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   /** PM Data Path Failure Counters data*/
   DSL_PM_DataPathFailureCountersData_t dataPathFailureCounters;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   /** PM ReTx Counters data*/
   DSL_PM_ReTxCountersData_t reTxCounters;
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/
} DSL_PM_CountersData_t;

typedef enum
{
   DSL_PM_FWMODE_NA = -1,
   /** VDSL part of FW */
   DSL_PM_FWMODE_VDSL = 0,
   /** ADSL part of FW */
   DSL_PM_FWMODE_ADSL = 1
} DSL_PM_FwMode_t;

typedef struct
{
   /** thread control structure */
   DSL_DRV_ThreadCtrl_t Control;
   /** PM module thread activity flag*/
   DSL_boolean_t bRun;
   /** Pm module thread poll time, msec*/
   DSL_uint32_t nThreadPollTime;
   /** PM module */
   DSL_DRV_Event_t pmEvent;
} DSL_PM_Thread_t;

typedef struct
{
   DSL_pmLineSecData_t LineSecCounters;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   DSL_pmChannelData_t ChannelCounters;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   DSL_pmDataPathData_t DataPathCounters;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   DSL_pmLineEventShowtimeData_t LineEventShowtimeCounters;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/
} DSL_PM_CountersDump_t;

/**
   Structure for the PM Danube Device specific data
*/
#ifdef INCLUDE_DSL_CPE_API_DANUBE
typedef struct
{
   DSL_uint8_t dummy;
} DSL_PM_DeviceContext_t;
#endif /* INCLUDE_DSL_CPE_API_DANUBE*/

/**
   Structure for the PM Vinax Device specific data
*/
#ifdef INCLUDE_DSL_CPE_API_VINAX
typedef struct
{
   DSL_uint8_t dummy;
} DSL_PM_DeviceContext_t;
#endif /* INCLUDE_DSL_CPE_API_VINAX*/

typedef enum
{
   DSL_PM_HISTORY_15MIN,
   DSL_PM_HISTORY_1DAY,
   DSL_PM_HISTORY_SHOWTIME
} DSL_PM_HistoryType_t;

typedef enum
{
   DSL_PM_COUNTER_NA = -1,
   DSL_PM_COUNTER_CHANNEL,
   DSL_PM_COUNTER_LINE_SEC,
   DSL_PM_COUNTER_LINE_INIT,
   DSL_PM_COUNTER_LINE_EVENT_SHOWTIME,
   DSL_PM_COUNTER_DATA_PATH,
   DSL_PM_COUNTER_DATA_PATH_FAILURE,
   DSL_PM_COUNTER_RETX,
   /* Delimeter only*/
   DSL_PM_COUNTER_LAST
} DSL_PM_EpType_t;

typedef struct
{
   DSL_PM_EpType_t epType;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** 15-min record*/
   DSL_uint8_t *pRec15min;
   /** 15-min history*/
   DSL_pmHistory_t *pHist15min;
   /** 15-min records number*/
   DSL_uint32_t nRecNum15min;
   /** 15-min invalid intervals history*/
   DSL_pmBF_IntervalFailures_t *p15minInvalidHist;
   /** 1-day record*/
   DSL_uint8_t *pRec1day;
   /** 1-day history*/
   DSL_pmHistory_t *pHist1day;
   /** 1-day records number*/
   DSL_uint32_t nRecNum1day;
   /** 1-day invalid intervals history*/
   DSL_pmBF_IntervalFailures_t *p1dayInvalidHist;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** Showtime record*/
   DSL_uint8_t *pRecShowtime;
   /** Showtime history*/
   DSL_pmHistory_t *pHistShowtime;
   /** Showtime records number*/
   DSL_uint32_t nRecNumShowtime;
   /** Showtime invalid intervals history*/
   DSL_pmBF_IntervalFailures_t *pShowtimeInvalidHist;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   /** Total record*/
   DSL_uint8_t *pRecTotal;
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
   /** Endpoint record element size (bytes)*/
   DSL_uint32_t nEpRecElementSize;
} DSL_PM_EpData_t;

typedef enum
{
   /** History Interval type undefined*/
   DSL_PM_HISTORY_INTERVAL_NONE = 0,
   /** History Interval 15 min*/
   DSL_PM_HISTORY_INTERVAL_15MIN = 1,
   /** History Interval 1 day*/
   DSL_PM_HISTORY_INTERVAL_1DAY = 2
} DSL_PM_HistIntervalType_t;

typedef struct
{
   /** PM module initialization flag*/
   DSL_boolean_t bInit;
#if defined (INCLUDE_DSL_CPE_PM_HISTORY) && defined(INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)
   /** Burnin Mode Indication Flag*/
   DSL_boolean_t bBurninModeActive;
#endif /* defined (INCLUDE_DSL_CPE_PM_HISTORY) && defined(INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)*/
   /** Common PM module mutex*/
   DSL_DRV_Mutex_t pmMutex;
   /** PM module direction Near-End mutex*/
   DSL_DRV_Mutex_t pmNeMutex;
   /** PM module direction Far-End mutex*/
   DSL_DRV_Mutex_t pmFeMutex;
   /** PM module Near-End access mutex*/
   DSL_DRV_Mutex_t pmNeAccessMutex;
   /** PM module Far-End access mutex*/
   DSL_DRV_Mutex_t pmFeAccessMutex;
   /** Pm module ...*/
   DSL_PM_Thread_t pmThreadNe;
   /** Pm module ...*/
   DSL_PM_Thread_t pmThreadFe;
   /** PM module lock indication flag*/
   DSL_boolean_t bPmLock;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /** Current synchronization mode. */
   DSL_PM_SyncModeType_t syncMode;
   /** 15 minutes elapsed flag*/
   DSL_boolean_t b15minElapsed;
   /** External 15 min elapsed flag. Used with the external sync mode*/
   DSL_boolean_t bExternal15minElapsed;
   /** 1 day elapsed flag*/
   DSL_boolean_t b1dayElapsed;
   /** External 1 day elapsed flag. Used with the external sync mode*/
   DSL_boolean_t bExternal1dayElapsed;
   /** Synchronization time to be used, (msec) */
   DSL_uint32_t nCurr15MinTime;
   /** Synchronization time to be used, (msec)*/
   DSL_uint32_t nCurr1DayTime;
   /** Synchronization time to be used, (sec) */
   DSL_uint32_t nElapsed15MinTime;
   /** Synchronization time to be used, (sec)*/
   DSL_uint32_t nElapsed1DayTime;
   /** Number of seconds in the 15 minutes interval, default is 900. */
   DSL_uint32_t nPm15Min;
   /** Number of seconds in the 1 day interval*/
   DSL_uint32_t nPm1Day;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /** Showtime elapsed flag*/
   DSL_boolean_t bShowtimeProcessingStart;
   /** Showtime reached flag*/
   DSL_boolean_t bShowtimeInvTrigger;
   /** Current Showtime synchronization time to be used, (msec) */
   DSL_uint32_t nCurrShowtimeTime;
   /** Showtime synchronization time to be used, (msec) */
   DSL_uint32_t nElapsedShowtimeTime;
   /** Actual Line state*/
   DSL_LineStateValue_t nLineState;
#endif /** #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   /** Last time checked [msec] */
   DSL_uint32_t nLastMsTimeCheck;
   /** PM Module Total Elapsed time [sec]*/
   DSL_uint32_t nPmTotalElapsedTime;
   /** Far-End Request cycle*/
   DSL_uint32_t nFeRequestCycle;
   /** Indicates that the PM data is valid*/
   DSL_boolean_t bPmDataValid;
   /** Indicates that the PM module FW polling is blocked*/
   DSL_boolean_t bPmFwPollingBlock;
   /** Indicates PM module last showtime FW mode*/
   DSL_PM_FwMode_t nLastShowtime;
   /** Indicates PM module current showtime FW mode*/
   DSL_PM_FwMode_t nCurrShowtime;
   /** PM tick interval in milli seconds, default is 1000 msec, mimimum is 500 msec.
       Each tick the PM database is updated through reading of the counters out
       of the firmware. */
   DSL_uint32_t nPmTick;
   /** PM module configuration */
   DSL_PM_ConfigData_t nPmConfig;
   /** Current NE polling control setting*/
   DSL_boolean_t bNePollingDisable;
   /** Current FE polling control setting*/
   DSL_boolean_t bFePollingDisable;
   /** Current interval Failures*/
   DSL_pmBF_IntervalFailures_t nCurrFailures;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   /** Flag to indicate Channel Counters reset in the FW*/
   DSL_boolean_t bNeChannelCntReset;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
   /** PM module counters*/
   DSL_PM_CountersData_t *pCounters;
   /** PM module counters for save/restore functionality*/
   DSL_PM_CountersDump_t *pCountersDump;
} DSL_PM_Context;

/**
   PM module main control thread, Near-End Processing
*/
DSL_int_t DSL_DRV_PM_ThreadNe(DSL_DRV_ThreadParams_t *param);

/**
   PM module Far-End thread
*/
DSL_int_t DSL_DRV_PM_ThreadFe(DSL_DRV_ThreadParams_t *param);

/**
   Function to Lock/Unlock PM module direction specific mutex
*/
DSL_Error_t DSL_DRV_PM_DirectionMutexControl(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t bLock);

/**
   Function to Lock/Unlock PM module access mutex
*/
DSL_Error_t DSL_DRV_PM_AccessMutexControl(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t bLock);

/**
  Lock PM module processing
*/
DSL_Error_t DSL_DRV_PM_Lock(DSL_Context_t *pContext);

/**
  Unlock PM module processing
*/
DSL_Error_t DSL_DRV_PM_UnLock(DSL_Context_t *pContext);

#if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)
/**
   PM module hostory create fucntion
*/
DSL_void_t DSL_DRV_PM_HistoryCreate(
   DSL_pmHistory_t *hist,
   DSL_uint32_t historySize);

/** Function to delete PM history intervals*/
DSL_Error_t DSL_DRV_PM_HistoryDelete(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist);

/** Function to update PM history interval*/
DSL_Error_t DSL_DRV_PM_HistoryCurItemUpdate(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist);

/** Function to get current history fill level*/
DSL_Error_t DSL_DRV_PM_HistoryFillLevelGet(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist,
   DSL_uint32_t *pFillLevel);

/** Function to get history interval Index*/
DSL_Error_t DSL_DRV_PM_HistoryItemIdxGet(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist,
   DSL_uint32_t histInterval,
   DSL_int_t *pIdx);

/** Function to update all PM module history values*/
DSL_Error_t DSL_DRV_PM_HistoryUpdate(DSL_Context_t *pContext);
#endif /** #if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)*/

/** Function to update all PM module counters*/
DSL_Error_t DSL_DRV_PM_CountersUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime);

DSL_Error_t DSL_DRV_PM_CountersReset(
   DSL_Context_t *pContext,
   DSL_PM_EpType_t EpType,
   DSL_PM_ResetTypes_t ResetType);

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineSecCountersReset(
   DSL_Context_t *pContext,
   DSL_PM_ResetTypes_t ResetType);

DSL_Error_t DSL_DRV_PM_LineInitCountersReset(
   DSL_Context_t *pContext,
   DSL_PM_ResetTypes_t ResetType);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_LineSecCountersHistoryIntervalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters);

DSL_Error_t DSL_DRV_PM_LineInitCountersHistoryIntervalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#if defined(INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS)
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
DSL_Error_t DSL_DRV_PM_LineSecThresholdsUpdate(
   DSL_PM_LineSecData_t *pCounters,
   DSL_PM_LineSecData_t *pThreshsOld,
   DSL_PM_LineSecData_t *pThreshsNew,
   DSL_uint32_t *pInd);


DSL_Error_t DSL_DRV_PM_LineInitThresholdsUpdate(
   DSL_PM_LineInitData_t *pCounters,
   DSL_PM_LineInitData_t *pThreshsOld,
   DSL_PM_LineInitData_t *pThreshsNew,
   DSL_uint32_t *pInd);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif

#if defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS)
/*
   Function to reset Line Event Showtime Counters
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersReset(
   DSL_Context_t *pContext,
   DSL_PM_ResetTypes_t ResetType);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_LineEventShowtimeCounters_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* defined(INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS)
DSL_Error_t DSL_DRV_PM_ChannelCountersReset(
   DSL_Context_t *pContext,
   DSL_PM_ResetTypes_t ResetType);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_ChannelCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_ChannelCounters_t *pCounters);

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
DSL_Error_t DSL_DRV_PM_ChannelCountersExtHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_ChannelCountersExt_t *pCounters);
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* defined(INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS)
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
DSL_Error_t DSL_DRV_PM_ChannelThresholdsUpdate(
   DSL_PM_ChannelData_t *pCounters,
   DSL_PM_ChannelData_t *pThreshsOld,
   DSL_PM_ChannelData_t *pThreshsNew,
   DSL_uint32_t *pInd);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS)
DSL_Error_t DSL_DRV_PM_DataPathCountersReset(
   DSL_Context_t *pContext,
   DSL_PM_ResetTypes_t ResetType);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_DataPathCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_DataPathCounters_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* defined(INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersReset(
   DSL_Context_t *pContext,
   DSL_PM_ResetTypes_t ResetType);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_DataPathFailureCounters_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* defined(INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS)*/

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS)
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
DSL_Error_t DSL_DRV_PM_DataPathThresholdsUpdate(
   DSL_PM_DataPathData_t *pCounters,
   DSL_PM_DataPathData_t *pThreshsOld,
   DSL_PM_DataPathData_t *pThreshsNew,
   DSL_uint32_t *pInd);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
DSL_Error_t DSL_DRV_PM_ReTxThresholdsUpdate(
   DSL_PM_ReTxData_t *pCounters,
   DSL_PM_ReTxData_t *pThreshsOld,
   DSL_PM_ReTxData_t *pThreshsNew,
   DSL_uint32_t *pInd);
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_ReTxCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_ReTxCounters_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

/** Function to check the PM module availability*/
DSL_boolean_t DSL_DRV_PM_IsPmReady(
   DSL_Context_t *pContext);

/** Function to save all PM module counters before restarting the FW*/
DSL_Error_t DSL_DRV_PM_CountersSave(DSL_Context_t *pContext);

/** Function to restore all PM module counters in the FW after restart*/
DSL_Error_t DSL_DRV_PM_CountersRestore(
   DSL_Context_t *pContext);

DSL_Error_t DSL_DRV_PM_InternalStateReset(
   DSL_Context_t *pContext);

DSL_Error_t DSL_DRV_PM_Suspend(
   DSL_Context_t *pContext);

DSL_Error_t DSL_DRV_PM_FwPollingStop(
   DSL_Context_t *pContext);

DSL_Error_t DSL_DRV_PM_Resume(
   DSL_Context_t *pContext);

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
DSL_Error_t DSL_DRV_PM_InternalCountersGet(
   DSL_Context_t *pContext,
   DSL_void_t *pTo,
   DSL_void_t *pFrom,
   DSL_uint32_t nSize,
   DSL_XTUDir_t nDirection);
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

/** @} DRV_DSL_CPE_PM */

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DSL_PM */

#endif /* _DRV_DSL_CPE_PM_CORE_H */
