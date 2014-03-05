/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#if defined(INCLUDE_DSL_PM)

#include "drv_dsl_cpe_pm_core.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_PM

DSL_boolean_t DSL_DRV_PM_IsPmReady(
   DSL_Context_t *pContext)
{
   if( DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module not started yet!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_FALSE;
   }

   if( !DSL_DRV_PM_CONTEXT(pContext)->bInit )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module not initialized yet!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_FALSE;
   }

   /* Check bPmLock flag*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bPmLock )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d: ERROR - PM module is temporary locked!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_FALSE;
   }

   return DSL_TRUE;
}

/** \addtogroup DRV_DSL_CPE_PM
 @{ */
static DSL_Error_t DSL_DRV_PM_SyncTimeUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t msecTimeFrame = DSL_PM_COUNTER_POLLING_CYCLE,
                nCurrMsTime = 0;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_uint32_t nCurrSysTime = 0, nPrevElapsedTime = 0;
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_int_t histShowtimeIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_SyncTimeUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   nCurrMsTime = DSL_DRV_PM_TIME_GET();

   /* Check for the time change*/
   if( DSL_DRV_PM_CONTEXT(pContext)->nLastMsTimeCheck > nCurrMsTime )
   {
      DSL_DRV_PM_CONTEXT(pContext)->nLastMsTimeCheck = nCurrMsTime;
   }

   if( DSL_DRV_PM_CONTEXT(pContext)->nLastMsTimeCheck != 0 )
   {
      /* Get elapsed time [msec] since the last entry*/
      msecTimeFrame = nCurrMsTime  - DSL_DRV_PM_CONTEXT(pContext)->nLastMsTimeCheck;
   }

   /* Get Total Elapsed Time Since the PM module startup*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime += msecTimeFrame;

   /* Set last time check to the current time*/
   DSL_DRV_PM_CONTEXT(pContext)->nLastMsTimeCheck = nCurrMsTime;

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /*
      Showtime interval processing
   */

   /* Update Line state information*/
   DSL_CTX_READ(pContext, nErrCode, nLineState, nLineState);

   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart != DSL_TRUE )
   {
      if( nLineState == DSL_LINESTATE_SHOWTIME_NO_SYNC ||
          nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC)
      {
         /* First showtime reached*/
         DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart = DSL_TRUE;
         /* Set current showtime elapsed time*/
         DSL_DRV_PM_CONTEXT(pContext)->nCurrShowtimeTime = 0;
      }
   }
   else
   {
      /* Check for the showtime intervel trigger*/
      if( (nLineState == DSL_LINESTATE_SHOWTIME_NO_SYNC ||
           nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
           (DSL_DRV_PM_CONTEXT(pContext)->nLineState != DSL_LINESTATE_SHOWTIME_NO_SYNC &&
            DSL_DRV_PM_CONTEXT(pContext)->nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC) )
      {
         /* Set showtime Interval Trigger flag*/
         DSL_DRV_PM_CONTEXT(pContext)->bShowtimeInvTrigger = DSL_TRUE;
      }
      else
      {
         /* Update current showtime elapsed time*/
         DSL_DRV_PM_CONTEXT(pContext)->nCurrShowtimeTime   += (msecTimeFrame/DSL_PM_MSEC);
         DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime =
            DSL_DRV_PM_CONTEXT(pContext)->nCurrShowtimeTime;
      }
   }

   /* Update line state in the PM module context*/
   DSL_DRV_PM_CONTEXT(pContext)->nLineState = nLineState;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   switch(DSL_DRV_PM_CONTEXT(pContext)->syncMode)
   {
   case DSL_PM_SYNC_MODE_FREE:
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
      /* add current working time in msec to avoid resync due to rounding*/
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime += msecTimeFrame;
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime  += msecTimeFrame;

      /* PM reported time will be in sec*/
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime =
         DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime/DSL_PM_MSEC;
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime  =
         DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime/DSL_PM_MSEC;

      /* Check if 15 minutes interval elapsed*/
      if( DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime >=
          DSL_DRV_PM_CONTEXT(pContext)->nPm15Min)
      {
         /* Set 15 min elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed  = DSL_TRUE;
         /* Set 15 min elapsed time*/
         DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime = DSL_DRV_PM_CONTEXT(pContext)->nPm15Min;
      }

      /* Check if 1 day interval elapsed*/
      if( DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime >=
          DSL_DRV_PM_CONTEXT(pContext)->nPm1Day)
      {
         /* Set 1 day elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed  = DSL_TRUE;
         /* Set 1 day elpased time*/
         DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime = DSL_DRV_PM_CONTEXT(pContext)->nPm1Day;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
      break;

   /*
      15-min and 1-day interval processing
   */
   case DSL_PM_SYNC_MODE_SYS_TIME:
      /* Get System Time*/
      nCurrSysTime = DSL_DRV_PM_SYS_TIME_GET();

      nPrevElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime;
      /* Get curent 15-min time [sec]*/
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime = (nCurrSysTime % (DSL_DRV_PM_CONTEXT(pContext)->nPm15Min));

      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime =
         DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime;

      /* Check if 15 min interval elapsed*/
      if( nPrevElapsedTime > DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime )
      {
         /* Set 15 min elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed     = DSL_TRUE;
         /* Set 15 min elapsed time*/
         DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime = DSL_DRV_PM_CONTEXT(pContext)->nPm15Min;
      }

      nPrevElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime;
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime = (nCurrSysTime % (DSL_DRV_PM_CONTEXT(pContext)->nPm1Day));

      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime  =
         DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime;

      /* Check if 1 day interval elapsed*/
      if( nPrevElapsedTime > DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime )
      {
         /* Set 1 day elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed     = DSL_TRUE;
         /* Set 1 day elpased time*/
         DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime = DSL_DRV_PM_CONTEXT(pContext)->nPm1Day;
      }
      break;

   case DSL_PM_SYNC_MODE_EXTERNAL:
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime += msecTimeFrame;
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime  += msecTimeFrame;

      /* PM reported time will be in sec*/
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime =
         DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime/DSL_PM_MSEC;
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime  =
         DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime/DSL_PM_MSEC;

      if( DSL_DRV_PM_CONTEXT(pContext)->bExternal15minElapsed )
      {
         DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: PM external 15 min sync..." DSL_DRV_CRLF  DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         /* Set 15 min elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed         = DSL_TRUE;
         /* Reset external 15 min elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->bExternal15minElapsed = DSL_FALSE;
      }

      if( DSL_DRV_PM_CONTEXT(pContext)->bExternal1dayElapsed )
      {
         DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: PM external 1 day sync..." DSL_DRV_CRLF  DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         /* Set 1 day elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed  = DSL_TRUE;
         /* Reset external 1 day elapsed flag*/
         DSL_DRV_PM_CONTEXT(pContext)->bExternal1dayElapsed = DSL_FALSE;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
      break;

   default:
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - unknown sync mode (%d)!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DRV_PM_CONTEXT(pContext)->syncMode));

      nErrCode = DSL_ERROR;
      break;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   /*
      Update Timeframes History statistics
   */
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_RETX_HISTORY_15MIN(), 0, &hist15minIdx);
   /* Update 15-min Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n15minTimeHist[hist15minIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime;

   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_RETX_HISTORY_1DAY(), 0, &hist1dayIdx);
   /* Update 1-day Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n1dayTimeHist[hist1dayIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryItemIdxGet(
      pContext, DSL_DRV_PM_PTR_RETX_HISTORY_SHOWTIME(), 0, &histShowtimeIdx);
   /* Update Showtime Timeframe history information*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.nShowtimeTimeHist[histShowtimeIdx] =
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Reset current showtime elapsed time*/
   if (DSL_DRV_PM_CONTEXT(pContext)->bShowtimeInvTrigger)
   {
      DSL_DRV_PM_CONTEXT(pContext)->nCurrShowtimeTime    = 0;
      DSL_DRV_PM_CONTEXT(pContext)->nElapsedShowtimeTime = 0;
   }
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Reset current 15-min interval time*/
   if (DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed)
   {
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime    = 0;
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime = 0;
   }

   /* Reset current 1-day interval time*/
   if (DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed)
   {
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime    = 0;
      DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime = 0;
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - DSL_DRV_PM_SyncTimeUpdate"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}


static DSL_Error_t DSL_DRV_PM_PollCycleUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_uint32_t nStartTime,
   DSL_uint32_t nStopTime)
{
   DSL_uint32_t msecTimeFrame;
   DSL_uint32_t nPollFactor;
   DSL_PM_Thread_t *pPmThread;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_PollCycleUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pContext == DSL_NULL || DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL)
   {
      return DSL_ERROR;
   }

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERR_SEMAPHORE_GET;
   }

   if( nDirection == DSL_NEAR_END )
   {
      pPmThread   = &(DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe);
      nPollFactor = 1;
   }
   else
   {
      pPmThread =  &(DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe);
      nPollFactor = DSL_DRV_PM_CONTEXT(pContext)->nFeRequestCycle;
   }

   if( nStopTime < nStartTime )
   {
      /* System time has been changed*/
      pPmThread->nThreadPollTime = DSL_PM_COUNTER_POLLING_CYCLE;
   }
   else
   {
      msecTimeFrame = nStopTime - nStartTime;

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM processing time frame (%u msec)"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), msecTimeFrame));

      if( msecTimeFrame > DSL_DRV_PM_CONTEXT(pContext)->nPmTick * nPollFactor )
      {
         DSL_DEBUG(DSL_DBG_WRN,
            (pContext, "DSL[%02d]: PM %s processing out of Poll Cycle!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection == DSL_NEAR_END ? "NE" : "FE"));

         pPmThread->nThreadPollTime = 10; /* msec*/
      }
      else
      {
         pPmThread->nThreadPollTime =
            DSL_DRV_PM_CONTEXT(pContext)->nPmTick * nPollFactor - msecTimeFrame;
      }
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: PM new %s Poll Time %d msec"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nDirection == DSL_NEAR_END ? "NE" : "FE",
      pPmThread->nThreadPollTime));

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_DRV_PM_FePollFactorUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_G997_PowerManagementStatusData_t powerMgmtStatus = {DSL_G997_PMS_NA};

   /* Get Power Management Status*/
   DSL_CTX_READ(pContext, nErrCode, powerMgmtStatus, powerMgmtStatus);

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERR_SEMAPHORE_GET;
   }

   if (powerMgmtStatus.nPowerManagementStatus == DSL_G997_PMS_L2)
   {
      DSL_DRV_PM_CONTEXT(pContext)->nFeRequestCycle =
         DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nFeUpdateCycleFactorL2;
   }
   else
   {
      DSL_DRV_PM_CONTEXT(pContext)->nFeRequestCycle =
         DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nFeUpdateCycleFactor;
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   return nErrCode;
}

#if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)
static DSL_Error_t DSL_DRV_PM_CountersOffsetUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_CountersOffsetUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_PM_CountersUpdate(
                 pContext, nDirection, (DSL_boolean_t)!b15min,
                 (DSL_boolean_t)!b1day, (DSL_boolean_t)!bTotal,
                 (DSL_boolean_t)!bShowtime);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_CountersOffsetUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_IntervalValidityStatusUpdate(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t nFailures = DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures;
   DSL_boolean_t bNePollingOff = DSL_FALSE, bFePollingOff = DSL_FALSE;
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_UNKNOWN;
   DSL_boolean_t bAdsl1 = DSL_FALSE;
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Get FW polling control flags*/
   bNePollingOff = DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.bNePollingOff;
   bFePollingOff = DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.bFePollingOff;

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   /* Check for the disabled NE polling*/
   if (bNePollingOff)
   {
      DSL_DRV_PM_INTERVAL_FAILURE_SET(nFailures, DSL_PM_INTERVAL_FAILURE_NE_POLLING_SWITCHED_OFF);
   }
   else
   {
      DSL_DRV_PM_INTERVAL_FAILURE_CLR(nFailures, DSL_PM_INTERVAL_FAILURE_NE_POLLING_SWITCHED_OFF);
   }

   /* Check for the disabled FE polling*/
   if (bFePollingOff)
   {
      DSL_DRV_PM_INTERVAL_FAILURE_SET(nFailures, DSL_PM_INTERVAL_FAILURE_FE_POLLING_SWITCHED_OFF);
   }
   else
   {
      DSL_DRV_PM_INTERVAL_FAILURE_CLR(nFailures, DSL_PM_INTERVAL_FAILURE_FE_POLLING_SWITCHED_OFF);
   }

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nLineState);
   /* Get ADSL1 mode indication*/
   DSL_CTX_READ(pContext, nErrCode, pDevCtx->data.bAdsl1, bAdsl1);

   if (nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC)
   {
      /* Check for the ADSL1 mode*/
      if (bAdsl1)
      {
         DSL_DRV_PM_INTERVAL_FAILURE_SET(nFailures, DSL_PM_INTERVAL_FAILURE_FE_POLLING_INCOMPLETE);
      }
      else
      {
         DSL_DRV_PM_INTERVAL_FAILURE_CLR(nFailures, DSL_PM_INTERVAL_FAILURE_FE_POLLING_INCOMPLETE);
      }
   }
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

   /* Update current Interval Failures*/
   if (nFailures != DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures)
   {
      DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures = nFailures;
   }

   /* Check for the Switched ON NE polling on the history intervals boundaries*/
   if (!bNePollingOff && DSL_DRV_PM_CONTEXT(pContext)->bNePollingDisable)
   {
      DSL_boolean_t b15min, b1day, bShowtime;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      b15min    = DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed ? DSL_TRUE : DSL_FALSE;
      b1day     = DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed ? DSL_TRUE : DSL_FALSE;
#else
      b15min    = DSL_FALSE;
      b1day     = DSL_FALSE;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      bShowtime = DSL_DRV_PM_CONTEXT(pContext)->bShowtimeInvTrigger ? DSL_TRUE : DSL_FALSE;
#else
      bShowtime = DSL_FALSE;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

      if (b15min || b1day || bShowtime)
      {
         /* Perform NE counters offset values update*/
         DSL_DRV_PM_CountersOffsetUpdate(
            pContext, DSL_NEAR_END, b15min, b1day, DSL_FALSE, bShowtime);
      }
   }

   /* Check for the Switched ON FE polling on the history intervals boundaries*/
   if (!bFePollingOff && DSL_DRV_PM_CONTEXT(pContext)->bFePollingDisable)
   {
      DSL_boolean_t b15min, b1day, bShowtime;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      b15min    = DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed ? DSL_TRUE : DSL_FALSE;
      b1day     = DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed ? DSL_TRUE : DSL_FALSE;
#else
      b15min    = DSL_FALSE;
      b1day     = DSL_FALSE;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      bShowtime = DSL_DRV_PM_CONTEXT(pContext)->bShowtimeInvTrigger ? DSL_TRUE : DSL_FALSE;
#else
      bShowtime = DSL_FALSE;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

      /* Perform FE counters offset values update*/
      if (b15min || b1day || bShowtime)
      {
         /* Perform FE counters offset values update*/
         DSL_DRV_PM_CountersOffsetUpdate(
            pContext, DSL_FAR_END, b15min, b1day, DSL_FALSE, bShowtime);
      }
   }

   /* Update Previous values for the polling control flags*/
   DSL_DRV_PM_CONTEXT(pContext)->bNePollingDisable = bNePollingOff;
   DSL_DRV_PM_CONTEXT(pContext)->bFePollingDisable = bFePollingOff;

   return nErrCode;
}
#endif /* defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_pm_core.h'
*/
DSL_int_t DSL_DRV_PM_ThreadNe(DSL_DRV_ThreadParams_t *param)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nOsRet = 0;
   DSL_Context_t *pContext = (DSL_Context_t*)param->nArg1;
   DSL_uint32_t startTime, stopTime;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ThreadNe" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check DSL CPE context pointer*/
   if( pContext == DSL_NULL || DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL )
      return -1;

   /* Check if the PM was initialized*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bInit != DSL_TRUE )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module not initialized!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return -1;
   }

   DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.bRun = DSL_TRUE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: PM Near-End thread started"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* main PM module Task*/
   while( DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.bRun )
   {
      /* Wake up each nThreadPollTime. nThreadPollTime is dynamically adjusted
         according to the PM processing time and PM poll cycle*/
      DSL_DRV_WAIT_EVENT_TIMEOUT( DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.pmEvent,
                              DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.nThreadPollTime);

      /* Get Start Time*/
      startTime = DSL_DRV_PM_TIME_GET();

      /* Lock PM module NE mutex*/
      nErrCode = DSL_DRV_PM_DirectionMutexControl(pContext, DSL_NEAR_END, DSL_TRUE);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM Near-End mutex lock failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      /* Update PM counters for the Near-End direction*/
      nErrCode = DSL_DRV_PM_CountersUpdate(
         pContext, DSL_NEAR_END, DSL_TRUE, DSL_TRUE, DSL_TRUE, DSL_TRUE);

      /* Unlock PM module NE mutex*/
      DSL_DRV_PM_DirectionMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

      /* Check Update status, Reboot on Error*/
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM NE counters update failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         /* Restart Autoboot*/
         nErrCode =  DSL_DRV_AutobootStateSet(
            pContext, DSL_AUTOBOOTSTATE_RESTART, DSL_AUTOBOOT_RESTART_POLL_TIME);

         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Autoboot state set failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }

      /* Update Sync Time*/
      nErrCode = DSL_DRV_PM_SyncTimeUpdate(pContext);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM Sync Time update failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

#if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)
      /* Update Interval validity status*/
      DSL_DRV_PM_IntervalValidityStatusUpdate(pContext);

      /* Update history for all counters*/
      nErrCode = DSL_DRV_PM_HistoryUpdate(pContext);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM NE history update failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }
#endif /* #if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)*/

      /* Get Stop Time*/
      stopTime = DSL_DRV_PM_TIME_GET();

      /* Update PM thread poll cycle*/
      DSL_DRV_PM_PollCycleUpdate(pContext, DSL_NEAR_END, startTime, stopTime);
   }

   /* Clear PM module bRun flag*/
   DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.bRun = DSL_FALSE;

   nOsRet = DSL_DRV_ErrorToOS(nErrCode);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ThreadNe"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nOsRet;
}

DSL_int_t DSL_DRV_PM_ThreadFe(DSL_DRV_ThreadParams_t *param)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nOsRet = 0;
   DSL_Context_t *pContext = (DSL_Context_t*)param->nArg1;
   DSL_uint32_t startTime, stopTime;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ThreadFe" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check DSL CPE context pointer*/
   if( pContext == DSL_NULL || DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL )
      return -1;

   /* Check if the PM was initialized*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bInit != DSL_TRUE )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module not initialized!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return -1;
   }

   DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.bRun = DSL_TRUE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: PM Far-End thread started"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* main PM module Task*/
   while( DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.bRun )
   {
      DSL_DRV_WAIT_EVENT_TIMEOUT( DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.pmEvent,
                              DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.nThreadPollTime);

      /* Get Start Time*/
      startTime = DSL_DRV_PM_TIME_GET();

      /* Lock PM module FE mutex*/
      nErrCode = DSL_DRV_PM_DirectionMutexControl(pContext, DSL_FAR_END, DSL_TRUE);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM Far-End mutex lock failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      /* Update PM counters for the Far-End direction*/
      nErrCode = DSL_DRV_PM_CountersUpdate(
         pContext, DSL_FAR_END, DSL_TRUE, DSL_TRUE, DSL_TRUE, DSL_TRUE);

      /* Unlock PM module FE mutex*/
      DSL_DRV_PM_DirectionMutexControl(pContext, DSL_FAR_END, DSL_FALSE);

      /* Check Update status, Reboot on Error*/
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM FE counters update failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         /* Restart Autoboot*/
         nErrCode =  DSL_DRV_AutobootStateSet(
            pContext, DSL_AUTOBOOTSTATE_RESTART, DSL_AUTOBOOT_RESTART_POLL_TIME);

         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Autoboot state set failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }

      /* Get Stop Time*/
      stopTime = DSL_DRV_PM_TIME_GET();

      /* Update FE Poll Cycle Factor*/
      DSL_DRV_PM_FePollFactorUpdate(pContext);

      /* Update PM thread poll cycle*/
      DSL_DRV_PM_PollCycleUpdate(pContext, DSL_FAR_END, startTime, stopTime);
   }

   /* Clear PM module bRun flag*/
   DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.bRun = DSL_FALSE;

   nOsRet = DSL_DRV_ErrorToOS(nErrCode);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ThreadFe"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nOsRet;
}

DSL_Error_t DSL_DRV_PM_DirectionMutexControl(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t bLock)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DirectionMutexControl,(nDirection=%d,%s)" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nDirection, bLock ? "LOCK" : "UNLOCK"));

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Lock PM module direction dependent mutex*/
   if( nDirection == DSL_NEAR_END )
   {
      if( bLock )
      {
         if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmNeMutex) )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't lock PM NE mutex!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            nErrCode = DSL_ERR_SEMAPHORE_GET;
         }
      }
      else
      {
          /* Unlock PM module NE Mutex*/
          DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmNeMutex);
      }
   }
   else
   {
      if( bLock )
      {
         if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmFeMutex) )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't lock PM FE mutex!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            nErrCode = DSL_ERR_SEMAPHORE_GET;
         }
      }
      else
      {
         /* Unlock PM module FE Mutex*/
         DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmFeMutex);
      }
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DirectionMutexControl, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_AccessMutexControl(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t bLock)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_AccessMutexControl,(nDirection=%d,%s)" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nDirection, bLock ? "LOCK" : "UNLOCK"));

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   /* Lock PM module direction dependent mutex*/
   if( nDirection == DSL_NEAR_END )
   {
      if( bLock )
      {
         if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmNeAccessMutex) )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't lock PM NE access mutex!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            nErrCode = DSL_ERR_SEMAPHORE_GET;
         }
      }
      else
      {
          /* Unlock PM module NE access Mutex*/
          DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmNeAccessMutex);
      }
   }
   else
   {
      if( bLock )
      {
         if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmFeAccessMutex) )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't lock PM FE access mutex!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            nErrCode = DSL_ERR_SEMAPHORE_GET;
         }
      }
      else
      {
         /* Unlock PM module FE access Mutex*/
         DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmFeAccessMutex);
      }
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_AccessMutexControl, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_Lock(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: IN - DSL_DRV_PM_Lock" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( !(DSL_DRV_PM_CONTEXT(pContext)->bPmLock) )
   {
      /* Lock PM module Near-End Mutex*/
      if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmNeMutex) )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock PM NE mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ERR_SEMAPHORE_GET;
      }

      /* Lock PM module Near-End Mutex*/
      if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmFeMutex) )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Couldn't lock PM FE mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return DSL_ERR_SEMAPHORE_GET;
      }

      /* Set bPmLock flag*/
      DSL_DRV_PM_CONTEXT(pContext)->bPmLock = DSL_TRUE;

      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM locked successfully" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM module already locked"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_Lock, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_UnLock(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_UnLock" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( DSL_DRV_PM_CONTEXT(pContext)->bPmLock )
   {
      /* Unlock PM module NE Mutex*/
      DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmNeMutex);

      /* Unlock PM module FE Mutex*/
      DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmFeMutex);

      /* Clear bPmLock flag*/
      DSL_DRV_PM_CONTEXT(pContext)->bPmLock = DSL_FALSE;

      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM unlocked successfully" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM already unlocked" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_UnLock, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_EpDataGet(
   DSL_Context_t *pContext,
   DSL_PM_EpData_t *pEpData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pEpData);
   DSL_CHECK_ERR_CODE();

   switch (pEpData->epType)
   {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   case DSL_PM_COUNTER_CHANNEL:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_CHANNEL_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_CHANNEL_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmChannelData_t);
      break;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   case DSL_PM_COUNTER_LINE_SEC:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_LINE_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_LINE_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_LINE_SEC_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_LINE_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmLineSecData_t);
      break;

   case DSL_PM_COUNTER_LINE_INIT:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_LINE_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_LINE_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_LINE_INIT_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_LINE_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmLineInitData_t);
      break;
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   case DSL_PM_COUNTER_LINE_EVENT_SHOWTIME:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmLineEventShowtimeData_t);
      break;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   case DSL_PM_COUNTER_DATA_PATH:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_DATAPATH_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_DATAPATH_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_DATAPATH_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmDataPathData_t);
      break;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   case DSL_PM_COUNTER_DATA_PATH_FAILURE:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmDataPathFailureData_t);
      break;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   case DSL_PM_COUNTER_RETX:
      #ifdef INCLUDE_DSL_CPE_PM_HISTORY
      pEpData->pRec15min         =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.rec15min);
      pEpData->pHist15min        = DSL_DRV_PM_PTR_RETX_HISTORY_15MIN();
      pEpData->nRecNum15min      = DSL_PM_RETX_15MIN_RECORDS_NUM;
      pEpData->p15minInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n15minInvalidHist;
      pEpData->pRec1day          =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.rec1day);
      pEpData->pHist1day         = DSL_DRV_PM_PTR_RETX_HISTORY_1DAY();
      pEpData->nRecNum1day       = DSL_PM_RETX_1DAY_RECORDS_NUM;
      pEpData->p1dayInvalidHist  =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n1dayInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
      #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      pEpData->pRecShowtime      =
         (DSL_uint8_t*)(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recShowtime);
      pEpData->pHistShowtime     = DSL_DRV_PM_PTR_RETX_HISTORY_SHOWTIME();
      pEpData->nRecNumShowtime   = DSL_PM_RETX_SHOWTIME_RECORDS_NUM;
      pEpData->pShowtimeInvalidHist =
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.nShowtimeInvalidHist;
      #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      pEpData->pRecTotal         =
         (DSL_uint8_t*)&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.recTotal);
      #endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
      pEpData->nEpRecElementSize = (DSL_uint32_t)sizeof(DSL_pmReTxData_t);
      break;
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

   default:
      nErrCode = DSL_ERROR;
      break;
   }

   return nErrCode;
}

#if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)
DSL_void_t DSL_DRV_PM_HistoryCreate(
   DSL_pmHistory_t *hist,
   DSL_uint32_t historySize)
{
   DSL_ASSERT((hist && historySize));

   hist->historySize = historySize;
   hist->curItem  = 0;
   hist->itemsNum = 1;
}

DSL_Error_t DSL_DRV_PM_HistoryDelete(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, hist);
   DSL_CHECK_ERR_CODE();

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   hist->curItem  = 0;
   hist->itemsNum = 1;

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_HistoryCurItemUpdate(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, hist);
   DSL_CHECK_ERR_CODE();

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,(pContext, "DSL[%02d]: ERROR - "
         "Couldn't lock PM mutex!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   hist->curItem = (hist->curItem < (hist->historySize-1)) ? (hist->curItem + 1) : 0;

   if( hist->itemsNum < hist->historySize )
   {
      hist->itemsNum++;
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_HistoryFillLevelGet(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist,
   DSL_uint32_t *pFillLevel)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, hist);
   DSL_CHECK_POINTER(pContext, pFillLevel);
   DSL_CHECK_ERR_CODE();

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,(pContext, "DSL[%02d]: ERROR - "
         "Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   *pFillLevel = hist->itemsNum - 1;

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_HistoryItemIdxGet(
   DSL_Context_t *pContext,
   DSL_pmHistory_t *hist,
   DSL_uint32_t histInterval,
   DSL_int_t *pIdx)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, hist);
   DSL_CHECK_POINTER(pContext, pIdx);
   DSL_CHECK_ERR_CODE();

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,(pContext, "DSL[%02d]: ERROR - "
         "Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   if((histInterval > hist->historySize) || (hist->historySize == 0))
   {
      DSL_DEBUG(DSL_DBG_ERR, (DSL_NULL,
         "DSL[%02d]: PMHistory_GetItem: invalid history item number (%u of %u)!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), histInterval, hist->historySize));

      nErrCode = DSL_ERROR;
   }

   if (nErrCode >= DSL_SUCCESS)
   {
      *pIdx = (DSL_int_t)((hist->historySize+hist->curItem-histInterval)
         % (hist->historySize));
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_HistoryEpUpdate(
   DSL_Context_t *pContext,
   DSL_PM_HistoryType_t HistoryType,
   DSL_PM_EpType_t epType)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_EpData_t nEpData;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_pmBF_IntervalFailures_t *pInvalidHist = DSL_NULL;
   DSL_uint8_t *pRec = DSL_NULL;
   DSL_int_t histIdx = 0;

   /* Get Endpoint data*/
   nEpData.epType = epType;
   nErrCode = DSL_DRV_PM_EpDataGet(pContext, &nEpData);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint (%d) data get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), epType));
      return nErrCode;
   }

   switch(HistoryType)
   {
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   case DSL_PM_HISTORY_15MIN:
      pHist        = nEpData.pHist15min;
      pInvalidHist = nEpData.p15minInvalidHist;
      pRec         = nEpData.pRec15min;
      break;
   case DSL_PM_HISTORY_1DAY:
      pHist        = nEpData.pHist1day;
      pInvalidHist = nEpData.p1dayInvalidHist;
      pRec         = nEpData.pRec1day;
      break;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   case DSL_PM_HISTORY_SHOWTIME:
      pHist        = nEpData.pHistShowtime;
      pInvalidHist = nEpData.pShowtimeInvalidHist;
      pRec         = nEpData.pRecShowtime;
      break;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS */
   default:
      return DSL_ERROR;
   }

   if (!pHist || !pInvalidHist || !pRec)
   {
      return DSL_ERROR;
   }

   /* Get curent history index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(pContext, pHist, 0, &histIdx);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint(%d) history item index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), epType));

      return nErrCode;
   }

   /* Clear Interval failure*/
   DSL_DRV_PM_INTERVAL_FAILURE_CLR(pInvalidHist[histIdx], DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE);

   /* Set current failures*/
   DSL_DRV_PM_INTERVAL_FAILURE_SET(pInvalidHist[histIdx], DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures);

   /* Update history*/
   nErrCode = DSL_DRV_PM_HistoryCurItemUpdate(pContext, pHist);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint(%d) history item update failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), epType));
      return nErrCode;
   }

   /* Get curent history index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(pContext, pHist, 0, &histIdx);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint(%d) history item index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), epType));
      return nErrCode;
   }

   /* set history memory to zeros */
   memset((pRec + histIdx*nEpData.nEpRecElementSize), 0, nEpData.nEpRecElementSize);

   /* Set invalid indication*/
   pInvalidHist[histIdx] = DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_HistoryUpdate(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_SyncEvent_t pmSyncEventData;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_HistoryUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Align history interval*/
   nErrCode = DSL_DRV_PM_DEV_HistoryIntervalAlign(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history interval align failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   /* Lock PM module FE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, DSL_FAR_END, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Far-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Check if Showtime interval elapsed*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeInvTrigger )
   {
      /* Reset bShowtimeElapsed flag*/
      DSL_DRV_PM_CONTEXT(pContext)->bShowtimeInvTrigger = DSL_FALSE;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_CHANNEL);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - channel showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_LINE_SEC);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line sec showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }

      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_LINE_INIT);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line init showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_DATA_PATH);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - data path showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_DATA_PATH_FAILURE);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - data path failure showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_LINE_EVENT_SHOWTIME);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line failure showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_SHOWTIME, DSL_PM_COUNTER_RETX);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - ReTx showtime history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Clear the PM sync event data*/
   pmSyncEventData.b15MinElapsed = DSL_FALSE;
   pmSyncEventData.b1DayElapsed  = DSL_FALSE;

   /* Check if 15-min interval elapsed*/
   if( DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed )
   {
      pmSyncEventData.b15MinElapsed = DSL_TRUE;

      /* Reset b15minElapsed flag*/
      DSL_DRV_PM_CONTEXT(pContext)->b15minElapsed = DSL_FALSE;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_CHANNEL);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - channel 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_LINE_SEC);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line sec 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }

      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_LINE_INIT);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line init 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_DATA_PATH);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - data path 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_DATA_PATH_FAILURE);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - data path failure 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_LINE_EVENT_SHOWTIME);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line failure 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_15MIN, DSL_PM_COUNTER_RETX);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - ReTx 15-min history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/
   }

   /* Check if 1-day interval elapsed*/
   if( DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed )
   {
      pmSyncEventData.b1DayElapsed  = DSL_TRUE;

      /* Reset b1dayElapsed flag*/
      DSL_DRV_PM_CONTEXT(pContext)->b1dayElapsed = DSL_FALSE;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_CHANNEL);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - channel 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_LINE_SEC);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line sec 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }

      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_LINE_INIT);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line init 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_DATA_PATH);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - data path 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_DATA_PATH_FAILURE);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - data path failure 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_LINE_EVENT_SHOWTIME);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - line failure 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryEpUpdate(
                    pContext, DSL_PM_HISTORY_1DAY, DSL_PM_COUNTER_RETX);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - ReTx 1-day history update failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/
   }

   if ((pmSyncEventData.b15MinElapsed) || (pmSyncEventData.b1DayElapsed))
   {
      /* Signal 15-min elapsed condition by the DSL_EVENT_S_PM_SYNC event*/
      nErrCode = DSL_DRV_EventGenerate(
                    pContext, 0, DSL_ACCESSDIR_NA, DSL_XTUDIR_NA,
                    DSL_EVENT_S_PM_SYNC,
                    (DSL_EventData_Union_t*)&pmSyncEventData,
                    sizeof(DSL_PM_SyncEvent_t));

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - DSL_EVENT_S_PM_SYNC event generate failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         goto on_hist_update_error;
      }
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

on_hist_update_error:
   /* Unlock PM module NE access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);
   /* Unlock PM module FE access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_FAR_END, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_HistoryUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #if defined(INCLUDE_DSL_CPE_PM_HISTORY) || defined(INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS)*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
DSL_Error_t DSL_DRV_PM_InternalCountersGet(
   DSL_Context_t *pContext,
   DSL_void_t *pTo,
   DSL_void_t *pFrom,
   DSL_uint32_t nSize,
   DSL_XTUDir_t nDirection)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   if (pTo && pFrom)
   {
      /* Copy Counters data*/
      memcpy(pTo, pFrom, nSize);
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - total counters data pointer is NULL"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      nErrCode = DSL_ERR_INTERNAL;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, nDirection, DSL_FALSE);

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

static DSL_void_t DSL_DRV_PM_CounterUpdate(
   DSL_uint32_t *pNewCounter,
   DSL_uint32_t *pCurCounter,
   DSL_uint32_t *pAbsCounter,
   DSL_uint32_t nMaxCounter,
   DSL_XTUDir_t nDirection)
{
   if(pNewCounter && pCurCounter && pAbsCounter)
   {
      /* Check for the Wrap Around condition*/
      if (*pNewCounter)
      {
         /* Update Absolute value*/
         *pAbsCounter =
            (*pNewCounter < *pCurCounter) ?
            (nDirection == DSL_FAR_END ? *pNewCounter :
            (*pNewCounter + (nMaxCounter - *pCurCounter))) :
            (*pNewCounter - *pCurCounter);
         *pCurCounter = *pNewCounter;
      }
   }
}

/*
   Function to reset Endpoint Counters
*/
DSL_Error_t DSL_DRV_PM_CountersReset(
   DSL_Context_t *pContext,
   DSL_PM_EpType_t EpType,
   DSL_PM_ResetTypes_t ResetType)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   DSL_PM_EpData_t EpData;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_CountersReset" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get Endpoint Data*/
   EpData.epType = EpType;
   nErrCode = DSL_DRV_PM_EpDataGet(pContext, &EpData);

   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint (%d) data get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), EpType));
      return nErrCode;
   }


#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, EpData.pHist15min, 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, EpData.pHist1day, 0, &hist1dayIdx);

   if( hist15minIdx < 0  || hist1dayIdx < 0 || nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint(%d) history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), EpType));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, EpData.pHistShowtime, 0, &histShowtimeIdx);

   if( histShowtimeIdx < 0 || nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Endpoint(%d) showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), EpType));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/


   switch(ResetType)
   {
   case DSL_PM_RESET_CURRENT:
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      /* Reset current 15-min and 1-day values */
      memset((EpData.pRec15min + hist15minIdx*EpData.nEpRecElementSize),
          0x0, EpData.nEpRecElementSize);
      memset((EpData.pRec1day + hist1dayIdx*EpData.nEpRecElementSize),
          0x0, EpData.nEpRecElementSize);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
      break;

   case DSL_PM_RESET_HISTORY:
   case DSL_PM_RESET_ALL:
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      /* Delete all history intervals*/
      nErrCode = DSL_DRV_PM_HistoryDelete(pContext, EpData.pHist15min);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM Endpoint(%d) history 15min delete failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), EpType));

         break;
      }

      nErrCode = DSL_DRV_PM_HistoryDelete(pContext, EpData.pHist1day);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM Endpoint(%d) history 15min delete failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), EpType));

         break;
      }

      if (ResetType == DSL_PM_RESET_ALL)
      {
         /* Reset 15-min and 1-day values curent and history*/
         memset(EpData.pRec15min,
                0x0, EpData.nEpRecElementSize * (EpData.nRecNum15min + 1));

         memset(EpData.pRec1day,
                0x0, EpData.nEpRecElementSize * (EpData.nRecNum1day + 1));
      }
      else
      {
         memcpy(EpData.pRec15min,
            (EpData.pRec15min + hist15minIdx*EpData.nEpRecElementSize),
            EpData.nEpRecElementSize);

         memcpy(EpData.pRec1day,
            (EpData.pRec1day + hist1dayIdx*EpData.nEpRecElementSize),
            EpData.nEpRecElementSize);

         /* Reset 15-min and 1-day history values*/
         memset((EpData.pRec15min + EpData.nEpRecElementSize),
                0x0, (EpData.nEpRecElementSize*EpData.nRecNum15min));

         memset((EpData.pRec1day + EpData.nEpRecElementSize),
                0x0, (EpData.nEpRecElementSize*EpData.nRecNum1day));
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   if (ResetType == DSL_PM_RESET_HISTORY)
      break;

   case DSL_PM_RESET_TOTAL:
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      memset(EpData.pRecTotal, 0x0, EpData.nEpRecElementSize);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
   if (ResetType == DSL_PM_RESET_TOTAL)
      break;

   case DSL_PM_RESET_HISTORY_SHOWTIME:
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      nErrCode = DSL_DRV_PM_HistoryDelete(pContext, EpData.pHistShowtime );
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM Endpoint(%d) history showtime delete failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), EpType));

         break;
      }

      if (ResetType == DSL_PM_RESET_ALL)
      {
         memset(EpData.pRecShowtime,
                0x0, EpData.nEpRecElementSize*(EpData.nRecNumShowtime + 1));
      }
      else
      {
         memcpy(EpData.pRecShowtime,
            (EpData.pRecShowtime + histShowtimeIdx*EpData.nEpRecElementSize),
            EpData.nEpRecElementSize);

         memset((EpData.pRecShowtime + EpData.nEpRecElementSize),
                0x0, (EpData.nEpRecElementSize * EpData.nRecNumShowtime));
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      break;

   case DSL_PM_RESET_CURRENT_SHOWTIME:
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      /* Reset current showtime values */
      memset((EpData.pRecShowtime + histShowtimeIdx*EpData.nEpRecElementSize),
          0x0, EpData.nEpRecElementSize);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      break;

   default:
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown reset type!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_ERROR;

      break;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_CountersReset" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/* Function to check the Channel Counters thresholds crossing for 15-min and 1-day
    interval counters. Appropriate event will be generated on the thresholds crossing
    condition*/
static DSL_Error_t DSL_DRV_PM_ChannelCountersThresholdsCrossingCheck(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_PM_ChannelData_t *p15mCounters,
   DSL_PM_ChannelData_t *p1dCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelThresholdCrossing_t channelThresholdCrossing;
   DSL_uint8_t i = 0;
   DSL_uint32_t *pEvtThresholdInd, *pThresholdInd;
   DSL_pmChannelThresholdCrossingData_t *pInd ;
   DSL_PM_ChannelData_t *pThresholds;
   DSL_PM_ChannelData_t *pCounters = DSL_NULL;
   DSL_boolean_t bEvent = DSL_FALSE;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, p15mCounters);
   DSL_CHECK_POINTER(pContext, p1dCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersThresholdsCrossingCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   pInd = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_IND(nChannel, nDirection);

   channelThresholdCrossing.n15Min = DSL_PM_CHANNELTHRESHCROSS_EMPTY;
   channelThresholdCrossing.n1Day  = DSL_PM_CHANNELTHRESHCROSS_EMPTY;

   for (i = 0; i < 2; i++)
   {
      if (i == 0)
      {
         pEvtThresholdInd = &(channelThresholdCrossing.n15Min);
         pThresholdInd    = &(pInd->n15Min);
         pThresholds      = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_15MIN(nChannel, nDirection);
         pCounters        = p15mCounters;
      }
      else
      {
         pEvtThresholdInd = &(channelThresholdCrossing.n1Day);
         pThresholdInd    = &(pInd->n1Day);
         pThresholds      = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_1DAY(nChannel, nDirection);
         pCounters        = p1dCounters;
      }

      if( pCounters->nFEC > pThresholds->nFEC)
         *pEvtThresholdInd |= DSL_PM_CHANNELTHRESHCROSS_FEC;

      if( pCounters->nCodeViolations > pThresholds->nCodeViolations )
         *pEvtThresholdInd |= DSL_PM_CHANNELTHRESHCROSS_CV;

      /* Check if new threshold crossing occured since the last check*/
      if( *pEvtThresholdInd != *pThresholdInd )
      {
         *pThresholdInd = *pEvtThresholdInd;
         bEvent = DSL_TRUE;
      }
   }

   /* Check if at least one threshold crossing occured*/
   if (bEvent)
   {
      /* Get current PM module time*/
      channelThresholdCrossing.nCurr15MinTime = (DSL_uint16_t)(DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime);
      channelThresholdCrossing.nCurr1DayTime  = DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;

      /* Generate DSL_EVENT_I_CHANNEL_THRESHOLD_CROSSING event*/
      nErrCode = DSL_DRV_EventGenerate(
                     pContext, nChannel, DSL_ACCESSDIR_NA, nDirection, DSL_EVENT_I_CHANNEL_THRESHOLD_CROSSING,
                     (DSL_EventData_Union_t*)&channelThresholdCrossing,
                     sizeof(DSL_PM_ChannelThresholdCrossing_t));
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_ChannelCountersThresholdsCrossingCheck, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   Function to update Channel Counters thresholds in the PM context
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholdsUpdate(
   DSL_PM_ChannelData_t *pCounters,
   DSL_PM_ChannelData_t *pThreshsOld,
   DSL_PM_ChannelData_t *pThreshsNew,
   DSL_uint32_t *pInd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( pCounters && pThreshsOld && pThreshsNew && pInd )
   {
      /* Set new nFEC threshold*/
      pThreshsOld->nFEC = pThreshsNew->nFEC;
      if( pCounters->nFEC <= pThreshsOld->nFEC )
         /* Clear nFEC indication bitmask*/
         *pInd &= ~DSL_PM_CHANNELTHRESHCROSS_FEC;

      /* Set new nCodeViolations threshold*/
      pThreshsOld->nCodeViolations = pThreshsNew->nCodeViolations;
      if( pCounters->nCodeViolations <= pThreshsOld->nCodeViolations )
         /* Clear nCodeViolations indication bitmask*/
         *pInd &= ~DSL_PM_CHANNELTHRESHCROSS_CV;
   }
   else
   {
      nErrCode = DSL_ERROR;
   }

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS*/

static DSL_Error_t DSL_DRV_PM_ChannelCountersCurrentGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_pmChannelData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelData_t *pChannelData = DSL_NULL;
#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   DSL_PM_ChannelDataExt_t *pChannelExtData = DSL_NULL;
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get channel counters pointer*/
   pChannelData = DSL_DRV_PM_PTR_CHANNEL_COUNTERS(pCounters, nChannel, nDirection);

   /* Call device specific stuff*/
   nErrCode = DSL_DRV_PM_DEV_ChannelCountersGet(
                 pContext, nChannel, nDirection, pChannelData);

   if (nErrCode < DSL_SUCCESS)
   {
      return nErrCode;
   }

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   if (nDirection == DSL_NEAR_END)
   {
      /* Get extended channel counters pointer*/
      pChannelExtData = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_EXT(pCounters, nChannel);

      /* Get extended channel counters*/
      nErrCode = DSL_DRV_PM_DEV_ChannelCountersExtGet(
                    pContext, nChannel, nDirection, pChannelExtData);
   }
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersCurrentGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_ChannelCountersUpdate(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_pmChannelData_t *pCounters,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelData_t *pNewCounters = DSL_NULL;
#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   DSL_PM_ChannelDataExt_t *pNewCountersExt = DSL_NULL;
   DSL_PM_ChannelDataExt_t *pCurrCountersExt;
   DSL_PM_ChannelDataExt_t nAbsCountersExt;
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_PM_ChannelDataExt_t *pTotalCountersExt;
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
# ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_ChannelDataExt_t *p15minCountersExt, *p1dayCountersExt;
# endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
# ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_ChannelDataExt_t *pShowtimeCountersExt;
# endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif
   DSL_PM_ChannelData_t *pCurrCounters;
   DSL_PM_ChannelData_t nAbsCounters;
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   DSL_PM_ChannelData_t *pTotalCounters;
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_ChannelData_t *p15minCounters, *p1dayCounters;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_ChannelData_t *pShowtimeCounters;
#endif

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Channel history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY */

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Channel Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   memset(&nAbsCounters, 0x0, sizeof(DSL_PM_ChannelData_t));

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   memset(&nAbsCountersExt, 0x0, sizeof(DSL_PM_ChannelDataExt_t));
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

   /* Get channel counters pointer*/
   pNewCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS(pCounters, nChannel, nDirection);

   /* Get counters from the PM module context*/
   pCurrCounters     = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_CURR(nChannel,nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minCounters    = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN(hist15minIdx,nChannel,nDirection);
   p1dayCounters     = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY(hist1dayIdx,nChannel,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   pTotalCounters    = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_TOTAL(nChannel,nDirection);
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_SHOWTIME(histShowtimeIdx,nChannel,nDirection);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   /* Get extended channel counters pointer*/
   pNewCountersExt = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_EXT(pCounters, nChannel);

   /* Get counters from the PM module context*/
   pCurrCountersExt = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_CURR_EXT(nChannel);
# ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minCountersExt = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN_EXT(hist15minIdx,nChannel);
   p1dayCountersExt  = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY_EXT(hist1dayIdx,nChannel);
# endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   pTotalCountersExt = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_TOTAL_EXT(nChannel);
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeCountersExt = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_SHOWTIME_EXT(histShowtimeIdx,nChannel);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

   /* Update nFEC*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nFEC), &(pCurrCounters->nFEC),
      &(nAbsCounters.nFEC), DSL_PM_COUNTER_FEC_MAX_VALUE, nDirection);

   /* Update nCodeViolations*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nCodeViolations), &(pCurrCounters->nCodeViolations),
      &(nAbsCounters.nCodeViolations), DSL_PM_COUNTER_CODEVIOLATIONS_MAX_VALUE, nDirection);

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   /* Check for the Wrap Around condition*/
   if (nDirection == DSL_NEAR_END)
   {
      /* Update nSuperFrame*/
      DSL_DRV_PM_CounterUpdate(
         &(pNewCountersExt->nSuperFrame), &(pCurrCountersExt->nSuperFrame),
         &(nAbsCountersExt.nSuperFrame), DSL_PM_COUNTER_SUPERFRAME_MAX_VALUE, DSL_NEAR_END);
   }
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/


#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if (b15min)
   {
      /* Update 15-min counters*/
      p15minCounters->nFEC            += nAbsCounters.nFEC;
      p15minCounters->nCodeViolations += nAbsCounters.nCodeViolations;
   }

   if (b1day)
   {
      /* Update 1-day counters*/
      p1dayCounters->nFEC             += nAbsCounters.nFEC;
      p1dayCounters->nCodeViolations  += nAbsCounters.nCodeViolations;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
   if (bTotal)
   {
      /* Update Total counters*/
      pTotalCounters->nFEC            += nAbsCounters.nFEC;
      pTotalCounters->nCodeViolations += nAbsCounters.nCodeViolations;
   }
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
   if (nDirection == DSL_NEAR_END)
   {
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
      if (b15min)
      {
         /* Update 15-min counters*/
         p15minCountersExt->nSuperFrame  += nAbsCountersExt.nSuperFrame;
      }

      if (b1day)
      {
         /* Update 1-day counters*/
         p1dayCountersExt->nSuperFrame   += nAbsCountersExt.nSuperFrame;
      }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
      if (bTotal)
      {
         pTotalCountersExt->nSuperFrame  += nAbsCountersExt.nSuperFrame;
      }
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/
   }
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/


#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart && bShowtime)
   {
      pShowtimeCounters->nFEC            += nAbsCounters.nFEC;
      pShowtimeCounters->nCodeViolations += nAbsCounters.nCodeViolations;
#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
      if (nDirection == DSL_NEAR_END)
      {
         pShowtimeCountersExt->nSuperFrame  += nAbsCountersExt.nSuperFrame;
      }
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/


#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /* Check for the Channel Counters 15-min and 1-day interval Thresholds Crossing*/
   nErrCode = DSL_DRV_PM_ChannelCountersThresholdsCrossingCheck(
                 pContext, nChannel, nDirection, p15minCounters, p1dayCounters);
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_ChannelCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_ChannelCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_ChannelData_t *pChCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY();

      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));
         nErrCode = DSL_ERROR;
         break;
      }

      /* Get History Item Index*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayTimeHist[histIdx];

      pChCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN(histIdx,pCounters->nChannel,pCounters->nDirection) :
         DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY(histIdx,pCounters->nChannel,pCounters->nDirection);

      if (pChCounters != DSL_NULL)
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pChCounters, sizeof(DSL_PM_ChannelData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExtHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_ChannelCountersExt_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_ChannelDataExt_t *pChCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY();

      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));
         nErrCode = DSL_ERROR;
         break;
      }

      /* Get History Item Index*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed time*/
      pCounters->interval.nElapsedTime =
         intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayTimeHist[histIdx];

      pChCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN_EXT(histIdx,pCounters->nChannel) :
         DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY_EXT(histIdx,pCounters->nChannel);

      if (pChCounters != DSL_NULL)
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pChCounters, sizeof(DSL_PM_ChannelDataExt_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

static DSL_Error_t DSL_DRV_PM_LineSecCountersCurrentGet(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_pmLineSecData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineSecData_t  *pLineSecCounters = DSL_NULL;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get pointer to the Line Sec counters according to the specified direction*/
   pLineSecCounters = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS(pCounters, nDirection);

   /* Get current Line Sec Counters*/
   nErrCode = DSL_DRV_PM_DEV_LineSecCountersGet(pContext, nDirection, pLineSecCounters);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_LineSecCountersUpdate, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
static DSL_Error_t DSL_DRV_PM_LineInitCountersCurrentGet(
   DSL_Context_t *pContext,
   DSL_pmLineInitData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineInitData_t *pLineInitCounters = DSL_NULL;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));


   /* Get pointer to the Line Init counters*/
   pLineInitCounters  = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS(pCounters);

   /* Get current Line Init Counters*/
   nErrCode = DSL_DRV_PM_DEV_LineInitCountersGet(pContext, pLineInitCounters);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_LineInitCountersUpdate, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/* Function to check the Line Sec Counters thresholds crossing for 15-min and 1-day
    interval counters. Appropriate event will be generated on the thresholds crossing
    condition*/
static DSL_Error_t DSL_DRV_PM_LineSecCountersThresholdsCrossingCheck(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_LineSecData_t *p15mLineSecCounters,
   DSL_PM_LineSecData_t *p1dLineSecCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineThresholdCrossing_t lineThresholdCrossing;
   DSL_uint32_t *pEvtThresholdInd, *pThresholdInd;
   DSL_pmLineThresholdCrossingData_t *pInd ;
   DSL_PM_LineSecData_t *pLineSecThresholds;
   DSL_PM_LineSecData_t *pLineSecCounters;
   DSL_boolean_t bEvent = DSL_FALSE;
   DSL_uint8_t i = 0;
#ifdef INCLUDE_DSL_ADSL_MIB
   DSL_MIB_ADSL_Thresholds_t nMibThresholds = DSL_MIB_TRAPS_NOTHING, nCurrMibThresholds;
#endif

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, p15mLineSecCounters);
   DSL_CHECK_POINTER(pContext, p1dLineSecCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCountersThresholdsCrossingCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   pInd = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_IND(nDirection);

   lineThresholdCrossing.n15Min = DSL_PM_LINETHRESHCROSS_EMPTY;
   lineThresholdCrossing.n1Day  = DSL_PM_LINETHRESHCROSS_EMPTY;

   for (i = 0; i< 2; i++)
   {
      if( i != 0 )
      {
         pEvtThresholdInd    = &(lineThresholdCrossing.n15Min);
         pThresholdInd       = &(pInd->n15Min);
         pLineSecThresholds  = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_15MIN(nDirection);
       pLineSecCounters    = p15mLineSecCounters;
      }
      else
      {
         pEvtThresholdInd    = &(lineThresholdCrossing.n1Day);
         pThresholdInd       = &(pInd->n1Day);
         pLineSecThresholds  = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_1DAY(nDirection);
       pLineSecCounters    = p1dLineSecCounters;
      }

      if( pLineSecCounters->nES > pLineSecThresholds->nES )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_ES;
         #ifdef INCLUDE_DSL_ADSL_MIB
         nMibThresholds |= (nDirection == DSL_NEAR_END) ?
            DSL_MIB_THRESHOLD_ATUR_ESS_FLAG : DSL_MIB_THRESHOLD_ATUC_ESS_FLAG;
         #endif
      }

      if( pLineSecCounters->nLOFS > pLineSecThresholds->nLOFS )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_LOFS;
         #ifdef INCLUDE_DSL_ADSL_MIB
         nMibThresholds |= (nDirection == DSL_NEAR_END) ?
            DSL_MIB_THRESHOLD_ATUR_LOFS_FLAG : DSL_MIB_THRESHOLD_ATUC_LOFS_FLAG;
         #endif
      }

      if( pLineSecCounters->nLOSS > pLineSecThresholds->nLOSS )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_LOSS;
         #ifdef INCLUDE_DSL_ADSL_MIB
         nMibThresholds |= (nDirection == DSL_NEAR_END) ?
            DSL_MIB_THRESHOLD_ATUR_LOSS_FLAG : DSL_MIB_THRESHOLD_ATUC_LOSS_FLAG;
         #endif
      }

      if( pLineSecCounters->nSES > pLineSecThresholds->nSES )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_SES;
         #ifdef INCLUDE_DSL_ADSL_MIB
         nMibThresholds |= (nDirection == DSL_NEAR_END) ?
            DSL_MIB_THRESHOLD_ATUR_15MIN_SESL_FLAG :
            DSL_MIB_THRESHOLD_ATUC_15MIN_SESL_FLAG;
         #endif
      }

      if( pLineSecCounters->nUAS > pLineSecThresholds->nUAS )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_UAS;
         #ifdef INCLUDE_DSL_ADSL_MIB
         nMibThresholds |= (nDirection == DSL_NEAR_END) ?
            DSL_MIB_THRESHOLD_ATUR_15MIN_UASL_FLAG :
            DSL_MIB_THRESHOLD_ATUC_15MIN_UASL_FLAG;
         #endif
      }

      /* Check if new threshold crossing occured since the last check*/
      if( *pEvtThresholdInd != *pThresholdInd )
      {
         *pThresholdInd = *pEvtThresholdInd;
       bEvent = DSL_TRUE;
      }
   }

   #ifdef INCLUDE_DSL_ADSL_MIB
   DSL_CTX_READ_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
      nCurrMibThresholds);
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
      nMibThresholds | nCurrMibThresholds);
   #endif

   /* Check if at least one threshold crossing occured*/
   if (bEvent)
   {
      /* Get current PM module time*/
      lineThresholdCrossing.nCurr15MinTime = (DSL_uint16_t)(DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime);
      lineThresholdCrossing.nCurr1DayTime  = DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;

      /* Generate DSL_EVENT_I_LINE_THRESHOLD_CROSSING event*/
      nErrCode = DSL_DRV_EventGenerate(
                    pContext, 0, DSL_ACCESSDIR_NA, nDirection, DSL_EVENT_I_LINE_THRESHOLD_CROSSING,
                    (DSL_EventData_Union_t*)&lineThresholdCrossing,
                    sizeof(DSL_PM_LineThresholdCrossing_t));
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_LineSecCountersThresholdsCrossingCheck, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/* Function to check the Line Sec Counters thresholds crossing for 15-min and 1-day
    interval counters. Appropriate event will be generated on the thresholds crossing
    condition*/
static DSL_Error_t DSL_DRV_PM_LineInitCountersThresholdsCrossingCheck(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_LineInitData_t *p15mLineInitCounters,
   DSL_PM_LineInitData_t *p1dLineInitCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineThresholdCrossing_t lineThresholdCrossing;
   DSL_uint32_t *pEvtThresholdInd, *pThresholdInd;
   DSL_pmLineThresholdCrossingData_t *pInd ;
   DSL_PM_LineInitData_t *pLineInitThresholds;
   DSL_PM_LineInitData_t *pLineInitCounters;
   DSL_boolean_t bEvent = DSL_FALSE;
   DSL_uint8_t i = 0;
#ifdef INCLUDE_DSL_ADSL_MIB
   DSL_MIB_ADSL_Thresholds_t nMibThresholds = DSL_MIB_TRAPS_NOTHING, nCurrMibThresholds;
#endif

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, p15mLineInitCounters);
   DSL_CHECK_POINTER(pContext, p1dLineInitCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCountersThresholdsCrossingCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   pInd = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_IND(nDirection);

   lineThresholdCrossing.n15Min = DSL_PM_LINETHRESHCROSS_EMPTY;
   lineThresholdCrossing.n1Day  = DSL_PM_LINETHRESHCROSS_EMPTY;

   for (i = 0; i< 2; i++)
   {
      if( i != 0 )
      {
         pEvtThresholdInd    = &(lineThresholdCrossing.n15Min);
         pThresholdInd       = &(pInd->n15Min);
         pLineInitThresholds = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_15MIN();
       pLineInitCounters   = p15mLineInitCounters;
      }
      else
      {
         pEvtThresholdInd    = &(lineThresholdCrossing.n1Day);
         pThresholdInd       = &(pInd->n1Day);
         pLineInitThresholds = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_1DAY();
       pLineInitCounters   = p1dLineInitCounters;
      }

      if( pLineInitCounters->nFailedFullInits > pLineInitThresholds->nFailedFullInits )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_FIFAIL;
      }

      if( pLineInitCounters->nFailedShortInits >
         pLineInitThresholds->nFailedShortInits )
      {
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_SIFAIL;
      }

      if( pLineInitCounters->nFullInits > pLineInitThresholds->nFullInits )
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_FI;

      if( pLineInitCounters->nShortInits > pLineInitThresholds->nShortInits )
      {
         #ifdef INCLUDE_DSL_ADSL_MIB
         nMibThresholds |= DSL_MIB_THRESHOLD_ATUC_15MIN_FAILED_FASTR_FLAG;
         #endif
         *pEvtThresholdInd |= DSL_PM_LINETHRESHCROSS_SI;
      }

      /* Check if new threshold crossing occured since the last check*/
      if( *pEvtThresholdInd != *pThresholdInd )
      {
         *pThresholdInd = *pEvtThresholdInd;
       bEvent = DSL_TRUE;
      }
   }

   #ifdef INCLUDE_DSL_ADSL_MIB
   DSL_CTX_READ_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
      nCurrMibThresholds);
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, MibAdslCtx.nThresholds,
      nMibThresholds | nCurrMibThresholds);
   #endif

   /* Check if at least one threshold crossing occured*/
   if (bEvent)
   {
      /* Get current PM module time*/
      lineThresholdCrossing.nCurr15MinTime = (DSL_uint16_t)(DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime);
      lineThresholdCrossing.nCurr1DayTime  = DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;

      /* Generate DSL_EVENT_I_LINE_THRESHOLD_CROSSING event*/
      nErrCode = DSL_DRV_EventGenerate(
                    pContext, 0, DSL_ACCESSDIR_NA, nDirection, DSL_EVENT_I_LINE_THRESHOLD_CROSSING,
                    (DSL_EventData_Union_t*)&lineThresholdCrossing,
                    sizeof(DSL_PM_LineThresholdCrossing_t));
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_LineInitCountersThresholdsCrossingCheck, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   Function to update Line Sec Counters thresholds in the PM context
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholdsUpdate(
   DSL_PM_LineSecData_t *pCounters,
   DSL_PM_LineSecData_t *pThreshsOld,
   DSL_PM_LineSecData_t *pThreshsNew,
   DSL_uint32_t *pInd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( pCounters && pThreshsOld && pThreshsNew && pInd )
   {
      /* Set new nES threshold*/
      pThreshsOld->nES = pThreshsNew->nES;
      if( pCounters->nES <= pThreshsOld->nES )
         /* Clear nES indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_ES;

      /* Set new nLOFS threshold*/
      pThreshsOld->nLOFS = pThreshsNew->nLOFS;
      if( pCounters->nLOFS <= pThreshsOld->nLOFS )
         /* Clear nLOFS indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_LOFS;

      /* Set new nLOSS threshold*/
      pThreshsOld->nLOSS = pThreshsNew->nLOSS;
      if( pCounters->nLOSS <= pThreshsOld->nLOSS )
         /* Clear nLOSS indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_LOSS;

      /* Set new nSES threshold*/
      pThreshsOld->nSES = pThreshsNew->nSES;
      if( pCounters->nSES <= pThreshsOld->nSES )
         /* Clear nSES indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_SES;

      /* Set new nUAS threshold*/
      pThreshsOld->nUAS = pThreshsNew->nUAS;
      if( pCounters->nUAS <= pThreshsOld->nUAS )
         /* Clear nUAS indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_UAS;
   }
   else
   {
      nErrCode = DSL_ERROR;
   }

   return nErrCode;
}

/*
   Function to update Line Init Counters thresholds in the PM context
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholdsUpdate(
   DSL_PM_LineInitData_t *pCounters,
   DSL_PM_LineInitData_t *pThreshsOld,
   DSL_PM_LineInitData_t *pThreshsNew,
   DSL_uint32_t *pInd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( pCounters && pThreshsOld && pThreshsNew && pInd )
   {
      /* Set new nFailedFullInits threshold*/
      pThreshsOld->nFailedFullInits = pThreshsNew->nFailedFullInits;
      if( pCounters->nFailedFullInits <= pThreshsOld->nFailedFullInits )
         /* Clear nFailedFullInits indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_FIFAIL;

      /* Set new nFailedShortInits threshold*/
      pThreshsOld->nFailedShortInits = pThreshsNew->nFailedShortInits;
      if( pCounters->nFailedShortInits <= pThreshsOld->nFailedShortInits )
         /* Clear nFailedShortInits indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_SIFAIL;

      /* Set new nFullInits threshold*/
      pThreshsOld->nFullInits = pThreshsNew->nFullInits;
      if( pCounters->nFullInits <= pThreshsOld->nFullInits )
         /* Clear nFullInits indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_FI;

      /* Set new nShortInits threshold*/
      pThreshsOld->nShortInits = pThreshsNew->nShortInits;
      if( pCounters->nShortInits <= pThreshsOld->nShortInits )
         /* Clear nShortInits indication bitmask*/
         *pInd &= ~DSL_PM_LINETHRESHCROSS_SI;
   }
   else
   {
      nErrCode = DSL_ERROR;
   }

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

static DSL_Error_t DSL_DRV_PM_LineSecCountersUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_pmLineSecData_t *pNewCounters,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_LineSecData_t  *p15minSecCounters = DSL_NULL, *p1daySecCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_LineSecData_t  *pShowtimeSecCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   DSL_PM_LineSecData_t  *pTotalSecCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
   DSL_PM_LineSecData_t  *pCurrSecCounters = DSL_NULL, *pNewSecCounters = DSL_NULL;
   DSL_PM_LineSecData_t nAbsSecCounters;

   DSL_pmLineSecAuxData_t *pLineSecAuxData;
   DSL_G997_BF_LineFailures_t nLineFailures = DSL_G997_LINEFAILURE_CLEANED;
   DSL_uint32_t nCurrTime = 0, nUasIncr = 0;
#ifndef DSL_CPE_PM_SECOND_IMPLEMENTATION
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
#endif

   DSL_CHECK_POINTER(pContext, pNewCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( nDirection == DSL_NEAR_END )
   {
      /* Get Line Failures information for the Near-End*/
      DSL_CTX_READ(pContext, nErrCode, nLineFailuresNe, nLineFailures);
   }
   else
   {
      /* Get Line Failures information for the Far-End*/
      DSL_CTX_READ(pContext, nErrCode, nLineFailuresFe, nLineFailures);
   }

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Line Sec history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Line Sec Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

   memset(&nAbsSecCounters, 0x0, sizeof(DSL_PM_LineSecData_t));

   /* Get counters from the PM module context*/
   pCurrSecCounters     = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_CURR(nDirection);
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minSecCounters    = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_15MIN(hist15minIdx,nDirection);
   p1daySecCounters     = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_1DAY(hist1dayIdx,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeSecCounters = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_SHOWTIME(histShowtimeIdx,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   pTotalSecCounters    = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_TOTAL(nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
   pLineSecAuxData      = DSL_DRV_PM_PTR_LINE_SEC_AUX_DATA(nDirection);

   pNewSecCounters = nDirection ==DSL_NEAR_END ? &(pNewCounters->sec_ne) :
                                                 &(pNewCounters->sec_fe);

   /*
      Special handling for the LOF counter
   */
   if( (nLineFailures & DSL_G997_LINEFAILURE_LOF) &&
       (!pLineSecAuxData->nLOFBegin) )
   {
      pLineSecAuxData->nLOFBegin = DSL_DRV_PM_TIME_GET()/DSL_PM_MSEC;
   }
   else if( !(nLineFailures & DSL_G997_LINEFAILURE_LOF) )
   {
      /* Update counters and clear timestamp */
      if( pLineSecAuxData->nLOFBegin )
      {
         nCurrTime = (DSL_DRV_PM_TIME_GET()/DSL_PM_MSEC);
         pLineSecAuxData->nLOFIntern += pLineSecAuxData->nLOFBegin > nCurrTime ?
                                        nCurrTime :
                                        nCurrTime - pLineSecAuxData->nLOFBegin;
         pLineSecAuxData->nLOFCurr    = 0;
         pLineSecAuxData->nLOFBegin   = 0;
      }
   }
   else
   {
      nCurrTime = (DSL_DRV_PM_TIME_GET()/DSL_PM_MSEC);
      pLineSecAuxData->nLOFCurr = pLineSecAuxData->nLOFBegin > nCurrTime ?
                                  nCurrTime :
                                  nCurrTime - pLineSecAuxData->nLOFBegin;
   }

   pNewSecCounters->nLOFS += (pLineSecAuxData->nLOFIntern + pLineSecAuxData->nLOFCurr);

#ifndef DSL_CPE_PM_SECOND_IMPLEMENTATION
   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,(pContext, "DSL[%02d]: ERROR - "
         "Couldn't lock PM mutex!"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Special handling for the UAS counter */
   if (nDirection == DSL_NEAR_END)
   {
      /* Get current line state*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

      if (((nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC)  &&
           (nCurrentState != DSL_LINESTATE_SHOWTIME_NO_SYNC)) &&
           (!pLineSecAuxData->bUASStart))
      {
         pLineSecAuxData->nUASBegin = DSL_DRV_PM_TIME_GET() / DSL_PM_MSEC;
         pLineSecAuxData->bUASStart = DSL_TRUE;
         pLineSecAuxData->nUASLast  = 0;
      }
      else
      {
         if (pLineSecAuxData->bUASStart)
         {
            nCurrTime = DSL_DRV_PM_TIME_GET() / DSL_PM_MSEC;

            if (pLineSecAuxData->nUASBegin == 0)
            {
               nUasIncr = nCurrTime;
            }
            else if (pLineSecAuxData->nUASBegin <= nCurrTime)
            {
               nUasIncr = nCurrTime - pLineSecAuxData->nUASBegin;
            }
            else
            {
               nUasIncr = nCurrTime + (0xFFFFFFFF - pLineSecAuxData->nUASBegin);
            }

            pLineSecAuxData->nUASLast += nUasIncr;
            pLineSecAuxData->nUASBegin = nCurrTime;
         }

         if ((nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC)  ||
             (nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
         {
            pLineSecAuxData->bUASStart = DSL_FALSE;
         }

         pLineSecAuxData->nUASIntern += nUasIncr;
      }
   }
   else
   {
      pLineSecAuxData = DSL_DRV_PM_PTR_LINE_SEC_AUX_DATA(DSL_NEAR_END);
   }

   pNewSecCounters->nUAS = pLineSecAuxData->nUASIntern;

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);
#endif /* #ifndef DSL_CPE_PM_SECOND_IMPLEMENTATION*/

   /* Update nES*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewSecCounters->nES), &(pCurrSecCounters->nES),
      &(nAbsSecCounters.nES), DSL_PM_COUNTER_ES_MAX_VALUE, nDirection);

   /* Update nLOFS*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewSecCounters->nLOFS), &(pCurrSecCounters->nLOFS),
      &(nAbsSecCounters.nLOFS), DSL_PM_COUNTER_LOFS_MAX_VALUE, nDirection);

   /* Update nLOSS*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewSecCounters->nLOSS), &(pCurrSecCounters->nLOSS),
      &(nAbsSecCounters.nLOSS), DSL_PM_COUNTER_LOSS_MAX_VALUE, nDirection);

   /* Update nLOSS*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewSecCounters->nSES), &(pCurrSecCounters->nSES),
      &(nAbsSecCounters.nSES), DSL_PM_COUNTER_SES_MAX_VALUE, nDirection);

   /* Update nUAS*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewSecCounters->nUAS), &(pCurrSecCounters->nUAS),
      &(nAbsSecCounters.nUAS), DSL_PM_COUNTER_UAS_MAX_VALUE, nDirection);

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if (b15min)
   {
      /* Update 15-min counters*/
      p15minSecCounters->nES   += nAbsSecCounters.nES;
      p15minSecCounters->nLOFS += nAbsSecCounters.nLOFS;
      p15minSecCounters->nLOSS += nAbsSecCounters.nLOSS;
      p15minSecCounters->nSES  += nAbsSecCounters.nSES;
      p15minSecCounters->nUAS  += nAbsSecCounters.nUAS;
   }

   if (b1day)
   {
      /* Update 1-day counters*/
      p1daySecCounters->nES   += nAbsSecCounters.nES;
      p1daySecCounters->nLOFS += nAbsSecCounters.nLOFS;
      p1daySecCounters->nLOSS += nAbsSecCounters.nLOSS;
      p1daySecCounters->nSES  += nAbsSecCounters.nSES;
      p1daySecCounters->nUAS  += nAbsSecCounters.nUAS;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart && bShowtime)
   {
      pShowtimeSecCounters->nES   += nAbsSecCounters.nES;
      pShowtimeSecCounters->nLOFS += nAbsSecCounters.nLOFS;
      pShowtimeSecCounters->nLOSS += nAbsSecCounters.nLOSS;
      pShowtimeSecCounters->nSES  += nAbsSecCounters.nSES;
      pShowtimeSecCounters->nUAS  += nAbsSecCounters.nUAS;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   if (bTotal)
   {
      /* Update Total counters*/
      pTotalSecCounters->nES   += nAbsSecCounters.nES;
      pTotalSecCounters->nLOFS += nAbsSecCounters.nLOFS;
      pTotalSecCounters->nLOSS += nAbsSecCounters.nLOSS;
      pTotalSecCounters->nSES  += nAbsSecCounters.nSES;
      pTotalSecCounters->nUAS  += nAbsSecCounters.nUAS;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS

   /* Check for the Channel Counters Thresholds Crossing*/
   nErrCode = DSL_DRV_PM_LineSecCountersThresholdsCrossingCheck(
                 pContext, nDirection,
                 p15minSecCounters,
                 p1daySecCounters);
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
static DSL_Error_t DSL_DRV_PM_LineInitCountersUpdate(
   DSL_Context_t *pContext,
   DSL_pmLineInitData_t *pNewCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   DSL_PM_LineInitData_t *pCurrInitCounters = DSL_NULL,
                         *pTotalInitCounters = DSL_NULL;
   DSL_PM_LineInitData_t nAbsInitCounters;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_LineInitData_t *p15minInitCounters = DSL_NULL, *p1dayInitCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_LineInitData_t *pShowtimeInitCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_CHECK_POINTER(pContext, pNewCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Line Init history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Line Init Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   memset(&nAbsInitCounters, 0x0, sizeof(DSL_PM_LineInitData_t));

   /* Get counters from the PM module context*/
   pCurrInitCounters     = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_CURR();
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minInitCounters    = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_15MIN(hist15minIdx);
   p1dayInitCounters     = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_1DAY(hist1dayIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeInitCounters = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_SHOWTIME(histShowtimeIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   pTotalInitCounters    = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_TOTAL();

   /* Update nFailedFullInits*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->init.nFailedFullInits), &(pCurrInitCounters->nFailedFullInits),
      &(nAbsInitCounters.nFailedFullInits), DSL_PM_COUNTER_FFINIT_MAX_VALUE, DSL_NEAR_END);

   /* Update nFailedShortInits*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->init.nFailedShortInits), &(pCurrInitCounters->nFailedShortInits),
      &(nAbsInitCounters.nFailedShortInits), DSL_PM_COUNTER_FSINIT_MAX_VALUE, DSL_NEAR_END);

   /* Update nFullInits*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->init.nFullInits), &(pCurrInitCounters->nFullInits),
      &(nAbsInitCounters.nFullInits), DSL_PM_COUNTER_FINIT_MAX_VALUE, DSL_NEAR_END);

   /* Update nShortInits*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->init.nShortInits), &(pCurrInitCounters->nShortInits),
      &(nAbsInitCounters.nShortInits), DSL_PM_COUNTER_SINIT_MAX_VALUE, DSL_NEAR_END);


#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Update 15-min counters*/
   p15minInitCounters->nFailedFullInits   += nAbsInitCounters.nFailedFullInits;
   p15minInitCounters->nFailedShortInits  += nAbsInitCounters.nFailedShortInits;
   p15minInitCounters->nFullInits         += nAbsInitCounters.nFullInits;
   p15minInitCounters->nShortInits        += nAbsInitCounters.nShortInits;

   /* Update 1-day counters*/
   p1dayInitCounters->nFailedFullInits    += nAbsInitCounters.nFailedFullInits;
   p1dayInitCounters->nFailedShortInits   += nAbsInitCounters.nFailedShortInits;
   p1dayInitCounters->nFullInits          += nAbsInitCounters.nFullInits;
   p1dayInitCounters->nShortInits         += nAbsInitCounters.nShortInits;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart )
   {
      pShowtimeInitCounters->nFailedFullInits  += nAbsInitCounters.nFailedFullInits;
      pShowtimeInitCounters->nFailedShortInits += nAbsInitCounters.nFailedShortInits;
      pShowtimeInitCounters->nFullInits        += nAbsInitCounters.nFullInits;
      pShowtimeInitCounters->nShortInits       += nAbsInitCounters.nShortInits;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   /* Update Total counters*/
   pTotalInitCounters->nFailedFullInits   += nAbsInitCounters.nFailedFullInits;
   pTotalInitCounters->nFailedShortInits  += nAbsInitCounters.nFailedShortInits;
   pTotalInitCounters->nFullInits         += nAbsInitCounters.nFullInits;
   pTotalInitCounters->nShortInits        += nAbsInitCounters.nShortInits;

#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS

   /* Check for the Channel Counters Thresholds Crossing*/
   nErrCode = DSL_DRV_PM_LineInitCountersThresholdsCrossingCheck(
                 pContext, DSL_NEAR_END,
                 p15minInitCounters,
                 p1dayInitCounters);
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_LineSecCountersHistoryIntervalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_LineSecData_t *pLineCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY();

      /* Get history fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));
         nErrCode = DSL_ERROR;
         break;
      }

      /* Get history item index*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n1dayTimeHist[histIdx];

      pLineCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_15MIN(histIdx, pCounters->nDirection) :
         DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_1DAY(histIdx, pCounters->nDirection);

      if( pLineCounters != DSL_NULL )
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pLineCounters, sizeof(DSL_PM_LineSecData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
#if defined(INCLUDE_DSL_CPE_API_VINAX)
      /* Report all counters as "0" except UAS*/
      pCounters->data.nES   = 0;
      pCounters->data.nLOFS = 0;
      pCounters->data.nLOSS = 0;
      pCounters->data.nSES  = 0;
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX)*/
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_LineInitCountersHistoryIntervalGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_LineInitData_t *pLineCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY();

      /* Get history fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!"
            DSL_DRV_CRLF, DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));
         nErrCode = DSL_ERROR;
         break;
      }

      /* Get history item index*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n1dayTimeHist[histIdx];

      pLineCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_15MIN(histIdx) :
         DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_1DAY(histIdx);

      if( pLineCounters != DSL_NULL )
      {
         DSL_pmBF_IntervalFailures_t nHistFailures;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            (nHistFailures & DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pLineCounters, sizeof(DSL_PM_LineInitData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
static DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersCurrentGet(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_LineEventShowtimeData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_PM_DEV_LineEventShowtimeCountersGet(
                 pContext,  nDirection, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_LineEventShowtimeCountersCurrentGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_LineEventShowtimeData_t *pNewCounters,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   DSL_PM_LineEventShowtimeData_t *pCurrCounters, *pTotalCounters;
   DSL_PM_LineEventShowtimeData_t nAbsCounters;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_LineEventShowtimeData_t *p15minCounters, *p1dayCounters;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_LineEventShowtimeData_t *pShowtimeCounters;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_CHECK_POINTER(pContext, pNewCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Line Event Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Line Event Showtime Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   memset(&nAbsCounters, 0x0, sizeof(DSL_PM_LineEventShowtimeData_t));

   /* Get counters from the PM module context*/
   pCurrCounters     = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_CURR(nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minCounters    = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_15MIN(hist15minIdx,nDirection);
   p1dayCounters     = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_1DAY(hist1dayIdx,nDirection);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeCounters = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME(histShowtimeIdx,nDirection);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   pTotalCounters    = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_TOTAL(nDirection);

   /* Update nLOF*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nLOF), &(pCurrCounters->nLOF),
      &(nAbsCounters.nLOF), DSL_PM_COUNTER_LOF_FAILURE_MAX_VALUE, nDirection);

   /* Update nLOS*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nLOS), &(pCurrCounters->nLOS),
      &(nAbsCounters.nLOS), DSL_PM_COUNTER_LOS_FAILURE_MAX_VALUE, nDirection);

   /* Update nLPR*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nLPR), &(pCurrCounters->nLPR),
      &(nAbsCounters.nLPR), DSL_PM_COUNTER_LPR_FAILURE_MAX_VALUE, nDirection);

   /* Update nLOM*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nLOM), &(pCurrCounters->nLOM),
      &(nAbsCounters.nLOM), DSL_PM_COUNTER_LOM_FAILURE_MAX_VALUE, nDirection);

   /* Update nSosSuccess*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nSosSuccess), &(pCurrCounters->nSosSuccess),
      &(nAbsCounters.nSosSuccess), DSL_PM_COUNTER_SOS_FAILURE_MAX_VALUE, nDirection);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if (b15min)
   {
      /* Update 15-min counters*/
      p15minCounters->nLOF += nAbsCounters.nLOF;
      p15minCounters->nLOS += nAbsCounters.nLOS;
      p15minCounters->nLPR += nAbsCounters.nLPR;
      p15minCounters->nLOM += nAbsCounters.nLOM;
      p15minCounters->nSosSuccess += nAbsCounters.nSosSuccess;
   }

   if (b1day)
   {
      /* Update 1-day counters*/
      p1dayCounters->nLOF += nAbsCounters.nLOF;
      p1dayCounters->nLOS += nAbsCounters.nLOS;
      p1dayCounters->nLPR += nAbsCounters.nLPR;
      p1dayCounters->nLOM += nAbsCounters.nLOM;
      p1dayCounters->nSosSuccess += nAbsCounters.nSosSuccess;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart && bShowtime)
   {
      pShowtimeCounters->nLOF += nAbsCounters.nLOF;
      pShowtimeCounters->nLOS += nAbsCounters.nLOS;
      pShowtimeCounters->nLPR += nAbsCounters.nLPR;
      pShowtimeCounters->nLOM += nAbsCounters.nLOM;
      pShowtimeCounters->nSosSuccess += nAbsCounters.nSosSuccess;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   if (bTotal)
   {
      /* Update Total counters*/
      pTotalCounters->nLOF += nAbsCounters.nLOF;
      pTotalCounters->nLOS += nAbsCounters.nLOS;
      pTotalCounters->nLPR += nAbsCounters.nLPR;
      pTotalCounters->nLOM += nAbsCounters.nLOM;
      pTotalCounters->nSosSuccess += nAbsCounters.nSosSuccess;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_LineEventShowtimeCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_LineEventShowtimeData_t *pLfCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY();

      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Get history item index for the specified interval*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed Time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n1dayTimeHist[histIdx];

      pLfCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_15MIN(histIdx,pCounters->nDirection) :
         DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_1DAY(histIdx,pCounters->nDirection);

      if( pLfCounters != DSL_NULL )
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pLfCounters, sizeof(DSL_PM_LineEventShowtimeData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/* Function to check the Data Path Counters thresholds crossing for 15-min or 1-day
    interval counters. Appropriate event will be generated on the thresholds crossing
    condition*/
static DSL_Error_t DSL_DRV_PM_DataPathCountersThresholdsCrossingCheck(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *p15mCounters,
   DSL_PM_DataPathData_t *p1dCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_DataPathThresholdCrossing_t dataPathThresholdCrossing;
   DSL_uint32_t *pEvtThresholdInd, *pThresholdInd;
   DSL_pmDataPathThresholdCrossingData_t *pInd ;
   DSL_PM_DataPathData_t *pThresholds;
   DSL_PM_DataPathData_t *pCounters = DSL_NULL;
   DSL_boolean_t bEvent = DSL_FALSE;
   DSL_uint8_t i = 0;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, p15mCounters);
   DSL_CHECK_POINTER(pContext, p1dCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCountersThresholdsCrossingCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   pInd = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_IND(nChannel, nDirection);

   dataPathThresholdCrossing.n15Min = DSL_PM_DATAPATHTHRESHCROSS_EMPTY;
   dataPathThresholdCrossing.n1Day  = DSL_PM_DATAPATHTHRESHCROSS_EMPTY;

   for (i = 0; i < 2; i++)
   {
      if (i == 0)
      {
         pEvtThresholdInd = &(dataPathThresholdCrossing.n15Min);
         pThresholdInd    = &(pInd->n15Min);
         pThresholds      = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_15MIN(nChannel, nDirection);
         pCounters        = p15mCounters;
      }
      else
      {
         pEvtThresholdInd = &(dataPathThresholdCrossing.n1Day);
         pThresholdInd    = &(pInd->n1Day);
         pThresholds      = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_1DAY(nChannel, nDirection);
         pCounters        = p1dCounters;
      }

      /* Check for nCRC_P threshold crossing*/
      if( pCounters->nCRC_P > pThresholds->nCRC_P )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_CRC_P;

      /* Check for nCRCP_P threshold crossing*/
      if( pCounters->nCRCP_P > pThresholds->nCRCP_P)
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_CRCP_P;

      /* Check for nCV_P threshold crossing*/
      if( pCounters->nCV_P > pThresholds->nCV_P )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_CV_P;

      /* Check for nCVP_P threshold crossing*/
      if( pCounters->nCVP_P > pThresholds->nCVP_P )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_CVP_P;

      /* Check for nHEC threshold crossing*/
      if( pCounters->nHEC > pThresholds->nHEC )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_HEC;

      /* Check for nIBE threshold crossing*/
      if( pCounters->nIBE > pThresholds->nIBE )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_IBE;

      /* Check for nTotalCells threshold crossing*/
      if( pCounters->nTotalCells > pThresholds->nTotalCells )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_TOTALCELL;

      /* Check for nUserTotalCells threshold crossing*/
      if( pCounters->nUserTotalCells > pThresholds->nUserTotalCells )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_USERCELL;

      /* Check for nTxUserTotalCells threshold crossing*/
      if( pCounters->nTxUserTotalCells > pThresholds->nTxUserTotalCells )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_TX_USER_TOTALCELL;

      /* Check for nTxIBE threshold crossing*/
      if( pCounters->nTxIBE > pThresholds->nTxIBE )
         *pEvtThresholdInd |= DSL_PM_DATAPATHTHRESHCROSS_TX_IBE;

      /* Check if new threshold crossing occured since the last check*/
      if( *pEvtThresholdInd != *pThresholdInd )
      {
         *pThresholdInd = *pEvtThresholdInd;
         bEvent = DSL_TRUE;
      }
   }

   /* Check if at least one threshold crossing occured*/
   if (bEvent)
   {
      /* Get current PM module time*/
      dataPathThresholdCrossing.nCurr15MinTime = (DSL_uint16_t)(DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime);
      dataPathThresholdCrossing.nCurr1DayTime  = DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;

      /* Generate DSL_EVENT_I_DATA_PATH_THRESHOLD_CROSSING event*/
      nErrCode = DSL_DRV_EventGenerate(
                     pContext, nChannel, DSL_ACCESSDIR_NA, nDirection,
                     DSL_EVENT_I_DATA_PATH_THRESHOLD_CROSSING,
                     (DSL_EventData_Union_t*)&dataPathThresholdCrossing,
                     sizeof(DSL_PM_DataPathThresholdCrossing_t));
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_DataPathCountersThresholdsCrossingCheck, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   Function to update Channel Counters thresholds in the PM context
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholdsUpdate(
   DSL_PM_DataPathData_t *pCounters,
   DSL_PM_DataPathData_t *pThreshsOld,
   DSL_PM_DataPathData_t *pThreshsNew,
   DSL_uint32_t *pInd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( pCounters && pThreshsOld && pThreshsNew && pInd )
   {
      /* Set new nCRC_P threshold*/
      pThreshsOld->nCRC_P = pThreshsNew->nCRC_P;
      if( pCounters->nCRC_P <= pThreshsOld->nCRC_P )
         /* Clear nCRC_P indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_CRC_P;

      /* Set new nCRCP_P threshold*/
      pThreshsOld->nCRCP_P = pThreshsNew->nCRCP_P;
      if( pCounters->nCRCP_P <= pThreshsOld->nCRCP_P )
         /* Clear nCRCP_P indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_CRCP_P;

      /* Set new nCV_P threshold*/
      pThreshsOld->nCV_P = pThreshsNew->nCV_P;
      if( pCounters->nCV_P <= pThreshsOld->nCV_P )
         /* Clear nCV_P indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_CV_P;

      /* Set new nCVP_P threshold*/
      pThreshsOld->nCVP_P = pThreshsNew->nCVP_P;
      if( pCounters->nCVP_P <= pThreshsOld->nCVP_P )
         /* Clear nCVP_P indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_CVP_P;

      /* Set new nHEC threshold*/
      pThreshsOld->nHEC = pThreshsNew->nHEC;
      if( pCounters->nHEC <= pThreshsOld->nHEC )
         /* Clear nHEC indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_HEC;

      /* Set new nIBE threshold*/
      pThreshsOld->nIBE = pThreshsNew->nIBE;
      if( pCounters->nIBE <= pThreshsOld->nIBE )
         /* Clear nIBE indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_IBE;

      /* Set new nTotalCells threshold*/
      pThreshsOld->nTotalCells = pThreshsNew->nTotalCells;
      if( pCounters->nTotalCells <= pThreshsOld->nTotalCells )
         /* Clear nTotalCells indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_TOTALCELL;

      /* Set new nUserTotalCells threshold*/
      pThreshsOld->nUserTotalCells = pThreshsNew->nUserTotalCells;
      if( pCounters->nUserTotalCells <= pThreshsOld->nUserTotalCells )
         /* Clear nUserTotalCells indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_USERCELL;

      /* Set new nTxUserTotalCells threshold*/
      pThreshsOld->nTxUserTotalCells = pThreshsNew->nTxUserTotalCells;
      if( pCounters->nTxUserTotalCells <= pThreshsOld->nTxUserTotalCells )
         /* Clear nTxUserTotalCells indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_TX_USER_TOTALCELL;

      /* Set new nTxIBE threshold*/
      pThreshsOld->nTxIBE = pThreshsNew->nTxIBE;
      if( pCounters->nTxIBE <= pThreshsOld->nTxIBE )
         /* Clear nTxIBE indication bitmask*/
         *pInd &= ~DSL_PM_DATAPATHTHRESHCROSS_TX_IBE;
   }
   else
   {
      nErrCode = DSL_ERROR;
   }

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS*/

static DSL_Error_t DSL_DRV_PM_DataPathCountersCurrentGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_PM_DEV_DataPathCountersGet(
                 pContext, nChannel, nDirection, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_DataPathCountersUpdate, nChannel=%d retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nChannel, nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_DataPathCountersUpdate(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *pNewCounters,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_PM_DataPathData_t *pCurrCounters, *pTotalCounters;
   DSL_PM_DataPathData_t nAbsCounters;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_DataPathData_t *p15minCounters, *p1dayCounters;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_DataPathData_t *pShowtimeCounters;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_CHECK_POINTER(pContext, pNewCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Data Path history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Data Path Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   memset(&nAbsCounters, 0x0, sizeof(DSL_PM_DataPathData_t));

   /* Get counters from the PM module context*/
   pCurrCounters     = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_CURR(nChannel,nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minCounters    = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_15MIN(hist15minIdx,nChannel,nDirection);
   p1dayCounters     = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_1DAY(hist1dayIdx,nChannel,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeCounters = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_SHOWTIME(histShowtimeIdx,nChannel,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   pTotalCounters    = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_TOTAL(nChannel,nDirection);

   /* Update nCRC_P*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nCRC_P), &(pCurrCounters->nCRC_P),
      &(nAbsCounters.nCRC_P), DSL_PM_COUNTER_CRCP_MAX_VALUE, nDirection);

   /* Update nCRCP_P*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nCRCP_P), &(pCurrCounters->nCRCP_P),
      &(nAbsCounters.nCRCP_P), DSL_PM_COUNTER_CRCPP_MAX_VALUE, nDirection);

   /* Update nCV_P*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nCV_P), &(pCurrCounters->nCV_P),
      &(nAbsCounters.nCV_P), DSL_PM_COUNTER_CVP_MAX_VALUE, nDirection);

   /* Update nCVP_P*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nCVP_P), &(pCurrCounters->nCVP_P),
      &(nAbsCounters.nCVP_P), DSL_PM_COUNTER_CVPP_MAX_VALUE, nDirection);

   /* Update nHEC*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nHEC), &(pCurrCounters->nHEC),
      &(nAbsCounters.nHEC), DSL_PM_COUNTER_HEC_MAX_VALUE, nDirection);

   /* Update nIBE*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nIBE), &(pCurrCounters->nIBE),
      &(nAbsCounters.nIBE), DSL_PM_COUNTER_IBE_MAX_VALUE, nDirection);

   /* Update nTotalCells*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nTotalCells), &(pCurrCounters->nTotalCells),
      &(nAbsCounters.nTotalCells), DSL_PM_COUNTER_TOTALCELL_MAX_VALUE, nDirection);

   /* Update nUserTotalCells*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nUserTotalCells), &(pCurrCounters->nUserTotalCells),
      &(nAbsCounters.nUserTotalCells), DSL_PM_COUNTER_UTOTALCELL_MAX_VALUE, nDirection);

   /* Update nTxUserTotalCells*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nTxUserTotalCells), &(pCurrCounters->nTxUserTotalCells),
      &(nAbsCounters.nTxUserTotalCells), DSL_PM_COUNTER_TX_UTOTALCELL_MAX_VALUE, nDirection);

   /* Update nTxIBE*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nTxIBE), &(pCurrCounters->nTxIBE),
      &(nAbsCounters.nTxIBE), DSL_PM_COUNTER_TX_IBE_MAX_VALUE, nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if (b15min)
   {
      /* Update 15-min counters*/
      p15minCounters->nCRC_P            += nAbsCounters.nCRC_P;
      p15minCounters->nCRCP_P           += nAbsCounters.nCRCP_P;
      p15minCounters->nCV_P             += nAbsCounters.nCV_P;
      p15minCounters->nCVP_P            += nAbsCounters.nCVP_P;
      p15minCounters->nHEC              += nAbsCounters.nHEC;
      p15minCounters->nIBE              += nAbsCounters.nIBE;
      p15minCounters->nTotalCells       += nAbsCounters.nTotalCells;
      p15minCounters->nUserTotalCells   += nAbsCounters.nUserTotalCells;
      p15minCounters->nTxUserTotalCells += nAbsCounters.nTxUserTotalCells;
      p15minCounters->nTxIBE            += nAbsCounters.nTxIBE;
   }

   if (b1day)
   {
      /* Update 1-day counters*/
      p1dayCounters->nCRC_P            += nAbsCounters.nCRC_P;
      p1dayCounters->nCRCP_P           += nAbsCounters.nCRCP_P;
      p1dayCounters->nCV_P             += nAbsCounters.nCV_P;
      p1dayCounters->nCVP_P            += nAbsCounters.nCVP_P;
      p1dayCounters->nHEC              += nAbsCounters.nHEC;
      p1dayCounters->nIBE              += nAbsCounters.nIBE;
      p1dayCounters->nTotalCells       += nAbsCounters.nTotalCells;
      p1dayCounters->nUserTotalCells   += nAbsCounters.nUserTotalCells;
      p1dayCounters->nTxUserTotalCells += nAbsCounters.nTxUserTotalCells;
      p1dayCounters->nTxIBE            += nAbsCounters.nTxIBE;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart && bShowtime)
   {
      pShowtimeCounters->nCRC_P            += nAbsCounters.nCRC_P;
      pShowtimeCounters->nCRCP_P           += nAbsCounters.nCRCP_P;
      pShowtimeCounters->nCV_P             += nAbsCounters.nCV_P;
      pShowtimeCounters->nCVP_P            += nAbsCounters.nCVP_P;
      pShowtimeCounters->nHEC              += nAbsCounters.nHEC;
      pShowtimeCounters->nIBE              += nAbsCounters.nIBE;
      pShowtimeCounters->nTotalCells       += nAbsCounters.nTotalCells;
      pShowtimeCounters->nUserTotalCells   += nAbsCounters.nUserTotalCells;
      pShowtimeCounters->nTxUserTotalCells += nAbsCounters.nTxUserTotalCells;
      pShowtimeCounters->nTxIBE            += nAbsCounters.nTxIBE;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   if (bTotal)
   {
      /* Update Total counters*/
      pTotalCounters->nCRC_P            += nAbsCounters.nCRC_P;
      pTotalCounters->nCRCP_P           += nAbsCounters.nCRCP_P;
      pTotalCounters->nCV_P             += nAbsCounters.nCV_P;
      pTotalCounters->nCVP_P            += nAbsCounters.nCVP_P;
      pTotalCounters->nHEC              += nAbsCounters.nHEC;
      pTotalCounters->nIBE              += nAbsCounters.nIBE;
      pTotalCounters->nTotalCells       += nAbsCounters.nTotalCells;
      pTotalCounters->nUserTotalCells   += nAbsCounters.nUserTotalCells;
      pTotalCounters->nTxUserTotalCells += nAbsCounters.nTxUserTotalCells;
      pTotalCounters->nTxIBE            += nAbsCounters.nTxIBE;
   }

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /* Check for the Channel Counters 15-min and 1-day interval Thresholds Crossing*/
   nErrCode = DSL_DRV_PM_DataPathCountersThresholdsCrossingCheck(
                 pContext, nChannel, nDirection, p15minCounters, p1dayCounters);
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_DataPathCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_DataPathCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_DataPathData_t *pDpCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY();

      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Get history item index for the specified interval*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed Time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n1dayTimeHist[histIdx];

      pDpCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_DATAPATH_COUNTERS_15MIN(histIdx,pCounters->nChannel,pCounters->nDirection) :
         DSL_DRV_PM_PTR_DATAPATH_COUNTERS_1DAY(histIdx,pCounters->nChannel,pCounters->nDirection);

      if( pDpCounters != DSL_NULL )
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pDpCounters, sizeof(DSL_PM_DataPathData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
static DSL_Error_t DSL_DRV_PM_DataPathFailureCountersCurrentGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_PM_DataPathFailureData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(nChannel);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_PM_DEV_DataPathFailureCountersGet(
                 pContext, nChannel, nDirection, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_DataPathFailureCountersCurrentGet, nChannel=%d retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nChannel, nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_DataPathFailureCountersUpdate(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   const DSL_XTUDir_t nDirection,
   DSL_PM_DataPathFailureData_t *pNewCounters,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_PM_DataPathFailureData_t *pCurrCounters, *pTotalCounters;
   DSL_PM_DataPathFailureData_t nAbsCounters;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_DataPathFailureData_t *p15minCounters, *p1dayCounters;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_DataPathFailureData_t *pShowtimeCounters;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_CHECK_POINTER(pContext, pNewCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Data Path failure history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - Data Path failure Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   memset(&nAbsCounters, 0x0, sizeof(DSL_PM_DataPathFailureData_t));

   /* Get counters from the PM module context*/
   pCurrCounters     = DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_CURR(nChannel,nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minCounters    = DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_15MIN(hist15minIdx,nChannel,nDirection);
   p1dayCounters     = DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_1DAY(hist1dayIdx,nChannel,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeCounters = DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_SHOWTIME(histShowtimeIdx,nChannel,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   pTotalCounters    = DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_TOTAL(nChannel,nDirection);

   /* Update nLCD*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nLCD), &(pCurrCounters->nLCD),
      &(nAbsCounters.nLCD), DSL_PM_COUNTER_LCD_FAILURE_MAX_VALUE, nDirection);

   /* Update nNCD*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nNCD), &(pCurrCounters->nNCD),
      &(nAbsCounters.nNCD), DSL_PM_COUNTER_NCD_FAILURE_MAX_VALUE, nDirection);

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if (b15min)
   {
      /* Update 15-min counters*/
      p15minCounters->nLCD += nAbsCounters.nLCD;
      p15minCounters->nNCD += nAbsCounters.nNCD;
   }

   if (b1day)
   {
      /* Update 1-day counters*/
      p1dayCounters->nLCD += nAbsCounters.nLCD;
      p1dayCounters->nNCD += nAbsCounters.nNCD;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart && bShowtime)
   {
      pShowtimeCounters->nLCD += nAbsCounters.nLCD;
      pShowtimeCounters->nNCD += nAbsCounters.nNCD;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   if (bTotal)
   {
      /* Update Total counters*/
      pTotalCounters->nLCD += nAbsCounters.nLCD;
      pTotalCounters->nNCD += nAbsCounters.nNCD;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_DataPathFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_DataPathFailureData_t *pDpCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY();

      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Get history item index for the specified interval*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed Time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n1dayTimeHist[histIdx];

      pDpCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_15MIN(histIdx,pCounters->nChannel,pCounters->nDirection) :
         DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_1DAY(histIdx,pCounters->nChannel,pCounters->nDirection);

      if( pDpCounters != DSL_NULL )
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pDpCounters, sizeof(DSL_PM_DataPathFailureData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/* Function to check the ReTx Counters thresholds crossing for 15-min or 1-day
    interval counters. Appropriate event will be generated on the thresholds crossing
    condition*/
static DSL_Error_t DSL_DRV_PM_ReTxCountersThresholdsCrossingCheck(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_ReTxData_t *p15mCounters,
   DSL_PM_ReTxData_t *p1dCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ReTxThresholdCrossing_t reTxThresholdCrossing;
   DSL_uint32_t *pEvtThresholdInd, *pThresholdInd;
   DSL_pmReTxThresholdCrossingData_t *pInd ;
   DSL_PM_ReTxData_t *pThresholds;
   DSL_PM_ReTxData_t *pCounters = DSL_NULL;
   DSL_boolean_t bEvent = DSL_FALSE;
   DSL_uint8_t i = 0;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, p15mCounters);
   DSL_CHECK_POINTER(pContext, p1dCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCountersThresholdsCrossingCheck" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   pInd = DSL_DRV_PM_PTR_RETX_THRESHOLD_IND(nDirection);

   reTxThresholdCrossing.n15Min = DSL_PM_RETXTHRESHCROSS_EMPTY;
   reTxThresholdCrossing.n1Day  = DSL_PM_RETXTHRESHCROSS_EMPTY;

   for (i = 0; i < 2; i++)
   {
      if (i == 0)
      {
         pEvtThresholdInd = &(reTxThresholdCrossing.n15Min);
         pThresholdInd    = &(pInd->n15Min);
         pThresholds      = DSL_DRV_PM_PTR_RETX_THRESHOLD_15MIN(nDirection);
         pCounters        = p15mCounters;
      }
      else
      {
         pEvtThresholdInd = &(reTxThresholdCrossing.n1Day);
         pThresholdInd    = &(pInd->n1Day);
         pThresholds      = DSL_DRV_PM_PTR_RETX_THRESHOLD_1DAY(nDirection);
         pCounters        = p1dCounters;
      }

      /* Check for nRxCorruptedTotal threshold crossing*/
      if( pCounters->nRxCorruptedTotal > pThresholds->nRxCorruptedTotal )
         *pEvtThresholdInd |= DSL_PM_RETXTHRESHCROSS_RX_CORRUPTED_TOTAL;

      /* Check for nRxUncorrectedProtected threshold crossing*/
      if( pCounters->nRxUncorrectedProtected > pThresholds->nRxUncorrectedProtected )
         *pEvtThresholdInd |= DSL_PM_RETXTHRESHCROSS_RX_UNCORR_PROT;

      /* Check for nRxRetransmitted threshold crossing*/
      if( pCounters->nRxRetransmitted > pThresholds->nRxRetransmitted )
         *pEvtThresholdInd |= DSL_PM_RETXTHRESHCROSS_RX_RETX;

      /* Check for nRxCorrected threshold crossing*/
      if( pCounters->nRxCorrected > pThresholds->nRxCorrected )
         *pEvtThresholdInd |= DSL_PM_RETXTHRESHCROSS_RX_CORR;

      /* Check if new threshold crossing occured since the last check*/
      if( *pEvtThresholdInd != *pThresholdInd )
      {
         *pThresholdInd = *pEvtThresholdInd;
         bEvent = DSL_TRUE;
      }
   }

   /* Check if at least one threshold crossing occured*/
   if (bEvent)
   {
      /* Get current PM module time*/
      reTxThresholdCrossing.nCurr15MinTime = (DSL_uint16_t)(DSL_DRV_PM_CONTEXT(pContext)->nElapsed15MinTime);
      reTxThresholdCrossing.nCurr1DayTime  = DSL_DRV_PM_CONTEXT(pContext)->nElapsed1DayTime;

      /* Generate DSL_EVENT_I_DATA_PATH_THRESHOLD_CROSSING event*/
      nErrCode = DSL_DRV_EventGenerate(
                     pContext, 0, DSL_ACCESSDIR_NA, nDirection,
                     DSL_EVENT_I_RETX_THRESHOLD_CROSSING,
                     (DSL_EventData_Union_t*)&reTxThresholdCrossing,
                     sizeof(DSL_PM_ReTxThresholdCrossing_t));
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_ReTxCountersThresholdsCrossingCheck, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   Function to update ReTx Counters thresholds in the PM context
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholdsUpdate(
   DSL_PM_ReTxData_t *pCounters,
   DSL_PM_ReTxData_t *pThreshsOld,
   DSL_PM_ReTxData_t *pThreshsNew,
   DSL_uint32_t *pInd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( pCounters && pThreshsOld && pThreshsNew && pInd )
   {
      /* Set new nRxCorruptedTotal threshold*/
      pThreshsOld->nRxCorruptedTotal = pThreshsNew->nRxCorruptedTotal;
      if( pCounters->nRxCorruptedTotal <= pThreshsOld->nRxCorruptedTotal )
         /* Clear nRxCorruptedTotal indication bitmask*/
         *pInd &= ~DSL_PM_RETXTHRESHCROSS_RX_CORRUPTED_TOTAL;

      /* Set new nRxUncorrectedProtected threshold*/
      pThreshsOld->nRxUncorrectedProtected = pThreshsNew->nRxUncorrectedProtected;
      if( pCounters->nRxUncorrectedProtected <= pThreshsOld->nRxUncorrectedProtected )
         /* Clear nRxUncorrectedProtected indication bitmask*/
         *pInd &= ~DSL_PM_RETXTHRESHCROSS_RX_UNCORR_PROT;

      /* Set new nRxRetransmitted threshold*/
      pThreshsOld->nRxRetransmitted = pThreshsNew->nRxRetransmitted;
      if( pCounters->nRxRetransmitted <= pThreshsOld->nRxRetransmitted )
         /* Clear nRxRetransmitted indication bitmask*/
         *pInd &= ~DSL_PM_RETXTHRESHCROSS_RX_RETX;

      /* Set new nRxCorrected threshold*/
      pThreshsOld->nRxCorrected = pThreshsNew->nRxCorrected;
      if( pCounters->nRxCorrected <= pThreshsOld->nRxCorrected )
         /* Clear nRxCorrected indication bitmask*/
         *pInd &= ~DSL_PM_RETXTHRESHCROSS_RX_CORR;
   }
   else
   {
      nErrCode = DSL_ERROR;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS*/

static DSL_Error_t DSL_DRV_PM_ReTxCountersCurrentGet(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_ReTxData_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCountersCurrentGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   nErrCode = DSL_DRV_PM_DEV_ReTxCountersGet(
                 pContext, nDirection, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_ReTxCountersCurrentGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_DRV_PM_ReTxCountersUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_PM_ReTxData_t *pNewCounters,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_int_t hist15minIdx = -1, hist1dayIdx = -1;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_int_t histShowtimeIdx = -1;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_PM_ReTxData_t *pCurrCounters, *pTotalCounters;
   DSL_PM_ReTxData_t nAbsCounters;
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   DSL_PM_ReTxData_t *p15minCounters, *p1dayCounters;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_PM_ReTxData_t *pShowtimeCounters;
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   DSL_CHECK_POINTER(pContext, pNewCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_RETX_HISTORY_15MIN(),
                 0, &hist15minIdx);

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_RETX_HISTORY_1DAY(),
                 0, &hist1dayIdx);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_RETX_HISTORY_SHOWTIME(),
                 0, &histShowtimeIdx);
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if( hist15minIdx < 0  || hist1dayIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - ReTx history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   if( histShowtimeIdx < 0 )
   {
      DSL_DEBUG( DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - ReTx Showtime history index error!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   memset(&nAbsCounters, 0x0, sizeof(DSL_PM_ReTxData_t));

   /* Get counters from the PM module context*/
   pCurrCounters     = DSL_DRV_PM_PTR_RETX_COUNTERS_CURR(nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   p15minCounters    = DSL_DRV_PM_PTR_RETX_COUNTERS_15MIN(hist15minIdx,nDirection);
   p1dayCounters     = DSL_DRV_PM_PTR_RETX_COUNTERS_1DAY(hist1dayIdx,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   pShowtimeCounters = DSL_DRV_PM_PTR_RETX_COUNTERS_SHOWTIME(histShowtimeIdx,nDirection);
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
   pTotalCounters    = DSL_DRV_PM_PTR_RETX_COUNTERS_TOTAL(nDirection);

   /* Update nRxCorrected*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nRxCorrected), &(pCurrCounters->nRxCorrected),
      &(nAbsCounters.nRxCorrected), DSL_PM_COUNTER_RX_CORR_MAX_VALUE, nDirection);

   /* Update nRxCorruptedTotal*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nRxCorruptedTotal), &(pCurrCounters->nRxCorruptedTotal),
      &(nAbsCounters.nRxCorruptedTotal), DSL_PM_COUNTER_RX_CORRUPTED_TOTAL_MAX_VALUE,
      nDirection);

   /* Update nRxRetransmitted*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nRxRetransmitted), &(pCurrCounters->nRxRetransmitted),
      &(nAbsCounters.nRxRetransmitted), DSL_PM_COUNTER_RX_RETX_MAX_VALUE, nDirection);

   /* Update nRxUncorrectedProtected*/
   DSL_DRV_PM_CounterUpdate(
      &(pNewCounters->nRxUncorrectedProtected), &(pCurrCounters->nRxUncorrectedProtected),
      &(nAbsCounters.nRxUncorrectedProtected), DSL_PM_COUNTER_RX_UNCORR_PROT_MAX_VALUE,
      nDirection);
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   if (b15min)
   {
      /* Update 15-min counters*/
      p15minCounters->nRxCorrected            += nAbsCounters.nRxCorrected;
      p15minCounters->nRxCorruptedTotal       += nAbsCounters.nRxCorruptedTotal;
      p15minCounters->nRxRetransmitted        += nAbsCounters.nRxRetransmitted;
      p15minCounters->nRxUncorrectedProtected += nAbsCounters.nRxUncorrectedProtected;
   }

   if (b1day)
   {
      /* Update 1-day counters*/
      p1dayCounters->nRxCorrected            += nAbsCounters.nRxCorrected;
      p1dayCounters->nRxCorruptedTotal       += nAbsCounters.nRxCorruptedTotal;
      p1dayCounters->nRxRetransmitted        += nAbsCounters.nRxRetransmitted;
      p1dayCounters->nRxUncorrectedProtected += nAbsCounters.nRxUncorrectedProtected;
   }
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Update Showtime counters*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart && bShowtime)
   {
      pShowtimeCounters->nRxCorrected            += nAbsCounters.nRxCorrected;
      pShowtimeCounters->nRxCorruptedTotal       += nAbsCounters.nRxCorruptedTotal;
      pShowtimeCounters->nRxRetransmitted        += nAbsCounters.nRxRetransmitted;
      pShowtimeCounters->nRxUncorrectedProtected += nAbsCounters.nRxUncorrectedProtected;
   }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   if (bTotal)
   {
      /* Update Total counters*/
      pTotalCounters->nRxCorrected            += nAbsCounters.nRxCorrected;
      pTotalCounters->nRxCorruptedTotal       += nAbsCounters.nRxCorruptedTotal;
      pTotalCounters->nRxRetransmitted        += nAbsCounters.nRxRetransmitted;
      pTotalCounters->nRxUncorrectedProtected += nAbsCounters.nRxUncorrectedProtected;
   }

#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   /* Check for the ReTx Counters 15-min and 1-day interval Thresholds Crossing*/
   nErrCode = DSL_DRV_PM_ReTxCountersThresholdsCrossingCheck(
                 pContext, nDirection, p15minCounters, p1dayCounters);
   #endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxCountersUpdate, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_Error_t DSL_DRV_PM_ReTxCountersHistoryIntervalGet(
   DSL_Context_t *pContext,
   DSL_PM_HistIntervalType_t intervalType,
   DSL_PM_ReTxCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0;
   DSL_pmHistory_t *pHist = DSL_NULL;
   DSL_PM_ReTxData_t *pReTxCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   if (pCounters->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check selected interval Type*/
   if ((intervalType != DSL_PM_HISTORY_INTERVAL_15MIN) &&
       (intervalType != DSL_PM_HISTORY_INTERVAL_1DAY))
   {
      return DSL_ERROR;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
      pHist = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_RETX_HISTORY_15MIN() :
         DSL_DRV_PM_PTR_RETX_HISTORY_1DAY();

      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, pHist, &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if( pCounters->nHistoryInterval > histFillLevel )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - no history data for the specified interval(%d)!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pCounters->nHistoryInterval));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Get history item index for the specified interval*/
      nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                    pContext, pHist,
                    pCounters->nHistoryInterval, &histIdx);

      if( nErrCode != DSL_SUCCESS || histIdx < 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERROR;
         break;
      }

      /* Set Elapsed Time*/
      pCounters->interval.nElapsedTime = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n15minTimeHist[histIdx] :
         DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n1dayTimeHist[histIdx];

      pReTxCounters = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
         DSL_DRV_PM_PTR_RETX_COUNTERS_15MIN(histIdx, pCounters->nDirection) :
         DSL_DRV_PM_PTR_RETX_COUNTERS_1DAY(histIdx, pCounters->nDirection);

      if( pReTxCounters != DSL_NULL )
      {
         DSL_pmBF_IntervalFailures_t nCurrFailures, nHistFailures;

         nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
            DSL_PM_INTERVAL_FAILURE_CLEANED;

         nHistFailures = intervalType == DSL_PM_HISTORY_INTERVAL_15MIN ?
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n15minInvalidHist[histIdx] :
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n1dayInvalidHist[histIdx];

         pCounters->interval.bValid  =
            ((nHistFailures | nCurrFailures) & failuresMask) == 0 ?
            DSL_TRUE : DSL_FALSE;
         pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
         memcpy(&(pCounters->data), pReTxCounters, sizeof(DSL_PM_ReTxData_t));
      }
      else
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         nErrCode = DSL_ERR_INTERNAL;
      }

      break;
   }
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   return nErrCode;
}
#endif /*  INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

DSL_Error_t DSL_DRV_PM_CountersUpdate(
   DSL_Context_t *pContext,
   const DSL_XTUDir_t nDirection,
   DSL_boolean_t b15min, DSL_boolean_t b1day,
   DSL_boolean_t bTotal, DSL_boolean_t bShowtime)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t nChannel;
   DSL_boolean_t bFwPollingEnabled = DSL_TRUE;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   DSL_pmChannelData_t channelData;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   DSL_PM_DataPathData_t dataPathData[DSL_CHANNELS_PER_LINE];
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   DSL_PM_DataPathFailureData_t dataPathFailureData[DSL_CHANNELS_PER_LINE];
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   DSL_PM_LineEventShowtimeData_t lineEventShowtimeData;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

   DSL_pmLineSecData_t lineSecData;
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   DSL_pmLineInitData_t lineInitData;
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   DSL_PM_ReTxData_t reTxData;
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ATU_DIRECTION(nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_CountersUpdate" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Reset Line Sec counters structure*/
   memset (&lineSecData, 0x0, sizeof(DSL_pmLineSecData_t));

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   /* Reset Channel counters structure*/
   memset(&channelData, 0x0, sizeof(DSL_pmChannelData_t));
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   memset(dataPathData, 0x0, sizeof(dataPathData));
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   /* Reset Data Path failures counters structure*/
   memset(&dataPathFailureData, 0x0, sizeof(DSL_PM_DataPathFailureData_t)*DSL_CHANNELS_PER_LINE);
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   /* Reset Line Event Showtime counters structure*/
   memset(&lineEventShowtimeData, 0x0, sizeof(DSL_PM_LineEventShowtimeData_t));
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   /* Reset ReTx counters structure*/
   memset(&reTxData, 0x0, sizeof(DSL_PM_ReTxData_t));
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Check for the Disabled FW polling*/
   bFwPollingEnabled = (DSL_boolean_t)(nDirection == DSL_NEAR_END ?
      !(DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.bNePollingOff) :
      !(DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.bFePollingOff));

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   /*
      Get Current counters values
   */
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   if( nDirection == DSL_NEAR_END )
   {
      /* Get current PM module Line Init counters*/
      nErrCode = DSL_DRV_PM_LineInitCountersCurrentGet(pContext, &lineInitData);
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Init Counters current get failed, nDirection=%d!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection));

         return nErrCode;
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   /* Get current PM module Bearer channel dependent counters*/
   for( nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++ )
   {
      /* Get current Data Path counters*/
      nErrCode = DSL_DRV_PM_DataPathFailureCountersCurrentGet(
                    pContext, nChannel, nDirection, &dataPathFailureData[nChannel]);
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Data Path failure Counters current get failed, nChannel=%d,nDirection=%d,!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nChannel, nDirection));
         return nErrCode;
      }
   }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

   if ( DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid && bFwPollingEnabled &&
       !DSL_DRV_PM_CONTEXT(pContext)->bPmFwPollingBlock
#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
      && DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
      )
   {
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
      /* Get current Line Event Showtime counters*/
      nErrCode = DSL_DRV_PM_LineEventShowtimeCountersCurrentGet(
         pContext, nDirection, &lineEventShowtimeData);
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line failure Counters current get failed, nDirection=%d,!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection));
         return nErrCode;
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

      /* Get current PM module Line Sec  counters*/
      nErrCode = DSL_DRV_PM_LineSecCountersCurrentGet(pContext, nDirection, &lineSecData);
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Sec Counters current get failed, nDirection=%d!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection));

         return nErrCode;
      }

      /* Get current PM module Bearer channel dependent counters*/
      for( nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++ )
      {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
         /* Get current Channel counters*/
         nErrCode = DSL_DRV_PM_ChannelCountersCurrentGet(
                       pContext, nChannel, nDirection, &channelData);
         if( nErrCode < DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Channel Counters current get failed, nChannel=%d,nDirection=%d,!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nChannel, nDirection));

            return nErrCode;
         }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
         /* Get current Data Path counters*/
         nErrCode = DSL_DRV_PM_DataPathCountersCurrentGet(
                       pContext, nChannel, nDirection, &dataPathData[nChannel]);
         if( nErrCode < DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Data Path Counters current get failed, nChannel=%d,nDirection=%d,!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nChannel, nDirection));
            return nErrCode;
         }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
      }

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
      /* Get current PM module ReTx counters. Currently counters are available
         for the Near-End only*/
      nErrCode = DSL_DRV_PM_ReTxCountersCurrentGet(pContext, nDirection, &reTxData);
      if( nErrCode < DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - ReTx Counters current get failed, nDirection=%d!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nDirection));

         return nErrCode;
      }
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/
   } /* if (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid && bFwPollingEnabled)*/


   /*
      Update all PM module counters
   */
   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   while(1)
   {
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
      if( nDirection == DSL_NEAR_END )
      {
         /* Update PM module Line Init Counters*/
         nErrCode = DSL_DRV_PM_LineInitCountersUpdate(pContext, &lineInitData);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Line Init counters update failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            break;
         }
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

      /* Update PM module Line Sec Counters*/
      nErrCode = DSL_DRV_PM_LineSecCountersUpdate(
                    pContext, nDirection, &lineSecData,
                    b15min, b1day, bTotal, bShowtime);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Sec counters update failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         break;
      }

      if (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid)
      {
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
         /* Update PM module Bearer channel dependent counters*/
         for( nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++ )
         {
            /* Update Data Path Failure Counters*/
            nErrCode = DSL_DRV_PM_DataPathFailureCountersUpdate(
                          pContext, nChannel, nDirection, &dataPathFailureData[nChannel],
                          b15min, b1day, bTotal, bShowtime);
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Data Path Failure counters update failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));

               break;
            }
         }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
         /* Update Line failure Counters*/
         nErrCode = DSL_DRV_PM_LineEventShowtimeCountersUpdate(
                       pContext, nDirection, &lineEventShowtimeData,
                       b15min, b1day, bTotal, bShowtime);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Line Event Showtime counters update failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            break;
         }
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

         /* Update PM module Bearer channel dependent counters*/
         for( nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++ )
         {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
            /* Update Channel Counters*/
            nErrCode = DSL_DRV_PM_ChannelCountersUpdate(
                          pContext, nChannel, nDirection, &channelData,
                          b15min, b1day, bTotal, bShowtime);
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Channel counters update failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));

               break;
            }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
            /* Update Data Path Counters*/
            nErrCode = DSL_DRV_PM_DataPathCountersUpdate(
                          pContext, nChannel, nDirection, &dataPathData[nChannel],
                          b15min, b1day, bTotal, bShowtime);
            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - Data Path counters update failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));

               break;
            }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
         }

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
         /* Update ReTx Counters*/
         nErrCode = DSL_DRV_PM_ReTxCountersUpdate(
                       pContext, nDirection, &reTxData,
                       b15min, b1day, bTotal, bShowtime);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - ReTx counters update failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            break;
         }
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/
      }
      break;
   } /* while(1)*/

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_CountersUpdate, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_CountersSave(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t i = 0, nChannel = 0;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_CountersSave" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   for (i = 0; i < 2; i++)
   {
      /* Get Line Counters from the FW*/
      nErrCode = DSL_DRV_PM_LineSecCountersCurrentGet(
                    pContext,
                    i == 0 ? DSL_NEAR_END : DSL_FAR_END,
                    &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineSecCounters));

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Sec Counters save failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
      /* Get Line Event Showtime Counters from the FW*/
      nErrCode = DSL_DRV_PM_DEV_LineEventShowtimeCountersGet(
                    pContext,
                    i == 0 ? DSL_NEAR_END : DSL_FAR_END,
                    i == 0 ?
                    &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineEventShowtimeCounters.data_ne) :
                    &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineEventShowtimeCounters.data_fe));

      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Event Showtime Counters save failed!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return nErrCode;
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

      /* Get current PM module Bearer channel dependent counters*/
      for( nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++ )
      {
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
         /* Get Channel Counters from the FW*/
         nErrCode = DSL_DRV_PM_ChannelCountersCurrentGet(
                       pContext, nChannel,
                       i == 0 ? DSL_NEAR_END : DSL_FAR_END,
                       &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->ChannelCounters));

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Channel Counters save failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return nErrCode;
         }
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
         nErrCode = DSL_DRV_PM_DataPathCountersCurrentGet(
                       pContext, nChannel,
                       i == 0 ? DSL_NEAR_END : DSL_FAR_END,
                       i == 0 ?
                       &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->DataPathCounters.data_ne[nChannel]):
                       &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->DataPathCounters.data_fe[nChannel]));

         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Data Path Counters save failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return nErrCode;
         }
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
      }
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_CountersSave, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_CountersRestore(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t fwUnavailableTime = 0;
   DSL_boolean_t bFwReLoaded = DSL_FALSE;
   DSL_uint8_t i = 0;
   DSL_XTUDir_t nDirection;
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   DSL_uint8_t nChannel = 0;
   DSL_PM_ChannelData_t *pChannelCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   DSL_PM_DataPathData_t *pDataPathCounters = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
   DSL_PM_LineSecData_t *pSecCounters = DSL_NULL;
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   DSL_PM_LineEventShowtimeData_t *pLineEventShowtimeCounters = DSL_NULL;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_CountersRestore" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL || DSL_DRV_PM_CONTEXT(pContext)->bInit == DSL_FALSE )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Counters restore failed, PM module not initializaed yet!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   /* Get FW unavailable time*/
   DSL_CTX_READ(pContext, nErrCode, nFwUnavailableTime, fwUnavailableTime);

   /* Reset FW unavailable time*/
   DSL_CTX_WRITE_SCALAR(pContext, nErrCode, nFwUnavailableTime, 0x0);

   /* Get bFwReLoaded flag*/
   DSL_CTX_READ(pContext, nErrCode, bFwReLoaded, bFwReLoaded);

   /* Restore all PM module counters*/
   if( bFwReLoaded )
   {
      /* Restart PM device specific stuff*/
      nErrCode = DSL_DRV_PM_DEV_Restart(pContext);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM device specific initialization failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
      /* Restore Channel Counters*/
      for( i = 0; i < 2; i++ )
      {
         nDirection = i ? DSL_NEAR_END : DSL_FAR_END;

         /* Update Channel Counters for the Near-End direction*/
         for (nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++)
         {
            /* Get Pointer to the Channel Saved Counters*/
            pChannelCounters =
               nDirection == DSL_NEAR_END ?
               &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->ChannelCounters.data_ne[nChannel]):
               &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->ChannelCounters.data_fe[nChannel]);

            /* Set Channel counters in the FW*/
            nErrCode = DSL_DRV_PM_DEV_ChannelCountersSet(
               pContext, nChannel, nDirection, pChannelCounters);

            if( nErrCode != DSL_SUCCESS )
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - PM %s Channel(%d) Counters Set failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext), nDirection == DSL_NEAR_END ? "NE" : "FE", nChannel));

               break;
            }
         }
      }

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Channel Counters restore failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
      /* Restore Line Event Showtime Counters*/
      for( i = 0; i < 2; i++ )
      {
         nDirection = i ? DSL_NEAR_END : DSL_FAR_END;

         /* Get Pointer to the Line Event Showtime Saved Counters*/
         pLineEventShowtimeCounters =
            nDirection == DSL_NEAR_END ?
            &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineEventShowtimeCounters.data_ne):
            &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineEventShowtimeCounters.data_fe);

         /* Set Channel counters in the FW*/
         nErrCode = DSL_DRV_PM_DEV_LineEventShowtimeCountersSet(
            pContext, nDirection, pLineEventShowtimeCounters);

         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - PM %s Line Event Showtime Counters Set failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nDirection == DSL_NEAR_END ? "NE" : "FE"));

            break;
         }
      }

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Event Showtime Counters restore failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

      /* Restore Line Sec counters*/
      for( i = 0; i < 2; i++ )
      {
         nDirection = i ? DSL_NEAR_END : DSL_FAR_END;

         /* Get Pointer to the Line Sec Saved Counters, NE direction*/
         pSecCounters =
            nDirection == DSL_NEAR_END ?
               &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineSecCounters.sec_ne):
               &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->LineSecCounters.sec_fe);

         /* Take into account FW unavailable time*/
         pSecCounters->nUAS += fwUnavailableTime;

         /* Set Line Sec counters in the FW*/
         nErrCode = DSL_DRV_PM_DEV_LineSecCountersSet(pContext, nDirection, pSecCounters);
         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
              (pContext, "DSL[%02d]: ERROR - PM %s Line Sec Counters Set failed!"DSL_DRV_CRLF,
              DSL_DEV_NUM(pContext), nDirection == DSL_NEAR_END ? "NE" : "FE"));

            break;
         }
      }

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - Line Sec Counters restore failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }

      /* Restore Data Path Counters*/
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
      /* Update Channel Counters for the Near-End direction*/
      for (nChannel = 0; nChannel < DSL_CHANNELS_PER_LINE; nChannel++)
      {
         /* Get Pointer to the Data Path Total Counters, NE direction*/
         pDataPathCounters =
            &(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump->DataPathCounters.data_ne[nChannel]);

         /* Restore Data Path counters, NE direction*/
         nErrCode = DSL_DRV_PM_DEV_DataPathCountersSet(
            pContext, 0, DSL_NEAR_END, pDataPathCounters);

         if( nErrCode != DSL_SUCCESS )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - PM NE Data Path Counters restore failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         }
      }
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/
   }

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_CountersRestore, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_InternalStateReset(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t nUASLastNe = 0, nUASLastFe = 0;

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_InternalStateReset" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL || DSL_DRV_PM_CONTEXT(pContext)->bInit == DSL_FALSE )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module not initializaed yet!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      return DSL_ERROR;
   }

   memset(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump, 0x0, sizeof(DSL_PM_CountersDump_t));

   memset(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recCurr.sec_fe), 0x0, sizeof(DSL_PM_LineSecData_t));
   memset(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.recCurr.sec_ne), 0x0, sizeof(DSL_PM_LineSecData_t));

   /* Get last UAS counter*/
   nUASLastNe =
      DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData.sec_data_ne.nUASLast;
   nUASLastFe =
      DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData.sec_data_fe.nUASLast;

   /* Reset Aux Line Sec counters*/
   memset(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData), 0x0, sizeof(DSL_pmLineAuxData_t));

   /* Update UAS Internal value with the last counted seconds between FAIL and SHOWTIME*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData.sec_data_ne.nUASIntern = nUASLastNe;
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.auxData.sec_data_fe.nUASIntern = nUASLastFe;

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   memset(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.recCurr), 0x0, sizeof(DSL_pmChannelData_t));
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   memset(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.recCurr), 0x0, sizeof(DSL_pmDataPathData_t));
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_InternalStateReset" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_FwPollingStop(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_DRV_PM_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_FwPollingStop" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Block FW polling*/
   DSL_DRV_PM_CONTEXT(pContext)->bPmFwPollingBlock = DSL_TRUE;

   DSL_DEBUG( DSL_DBG_MSG,(pContext, "DSL[%02d]: OUT - "
      "DSL_DRV_PM_FwPollingStop, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;

}

DSL_Error_t DSL_DRV_PM_Suspend(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if (DSL_DRV_PM_CONTEXT(pContext) != DSL_NULL &&
       DSL_DRV_PM_CONTEXT(pContext)->bPmLock != DSL_TRUE)
   {
      if (DSL_DRV_PM_CONTEXT(pContext)->bInit)
      {
         /* Lock PM module processing*/
         nErrCode = DSL_DRV_PM_Lock(pContext);
         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - Couldn't lock PM module!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return nErrCode;
         }

         /* Check if FW polling is allowed*/
         if (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid)
         {
            /* Save all PM counters*/
            nErrCode = DSL_DRV_PM_CountersSave(pContext);

            if (nErrCode != DSL_SUCCESS)
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - PM counters save failed!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));

               /* Reset Internal PM state*/
               DSL_DRV_PM_InternalStateReset(pContext);
            }
         }

         nErrCode = DSL_DRV_PM_DEV_Suspend(pContext);
         if (nErrCode != DSL_SUCCESS)
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - PM module device suspend failed!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));

            return nErrCode;
         }
      }
   }

   return nErrCode;
}

DSL_Error_t DSL_DRV_PM_Resume(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if (DSL_DRV_PM_CONTEXT(pContext) != DSL_NULL &&
       DSL_DRV_PM_CONTEXT(pContext)->bPmLock)
   {
      /* Unlock PM module processing*/
      nErrCode = DSL_DRV_PM_UnLock(pContext);
      if (nErrCode != DSL_SUCCESS)
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - PM module unlock failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         return nErrCode;
      }
   }

   /* Release FW polling*/
   DSL_DRV_PM_CONTEXT(pContext)->bPmFwPollingBlock = DSL_FALSE;

   return nErrCode;
}

/** @} DRV_DSL_CPE_PM */

#endif
