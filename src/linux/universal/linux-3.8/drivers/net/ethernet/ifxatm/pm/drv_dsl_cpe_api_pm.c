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
#include "drv_dsl_cpe_device_pm.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_PM

/** \addtogroup DRV_DSL_CPE_PM
 @{ */

#ifdef INCLUDE_DSL_CPE_PM_CONFIG
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ConfigSet(
   DSL_Context_t *pContext,
   DSL_PM_Config_t *pConfig)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pConfig);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Check Basic Update Cycle*/
   if ((pConfig->data.nBasicUpdateCycle < DSL_PM_MIN_BASIC_UPDATE_CYCLE) ||
       (pConfig->data.nBasicUpdateCycle > DSL_PM_MAX_BASIC_UPDATE_CYCLE))
   {
      return DSL_ERR_INVALID_PARAMETER;
   }

   /* Check FE Update Cycle Factor*/
   if ((pConfig->data.nFeUpdateCycleFactor < DSL_PM_MIN_FE_UPDATE_CYCLE_FACTOR) ||
       (pConfig->data.nFeUpdateCycleFactor > DSL_PM_MAX_FE_UPDATE_CYCLE_FACTOR))
   {
      return DSL_ERR_INVALID_PARAMETER;
   }

   /* Check FE Update Cycle Factor for the L2 mode*/
   if ((pConfig->data.nFeUpdateCycleFactorL2 < DSL_PM_MIN_FE_UPDATE_CYCLE_FACTOR_L2) ||
       (pConfig->data.nFeUpdateCycleFactorL2 > DSL_PM_MAX_FE_UPDATE_CYCLE_FACTOR_L2))
   {
      return DSL_ERR_INVALID_PARAMETER;
   }

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Set PM config data*/
   memcpy(&(DSL_DRV_PM_CONTEXT(pContext)->nPmConfig), &(pConfig->data),
      sizeof(DSL_PM_ConfigData_t));

#if defined (INCLUDE_DSL_CPE_PM_HISTORY) && defined(INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)
   /* Check for the Activated Burnin Mode*/
   if (DSL_DRV_PM_CONTEXT(pContext)->bBurninModeActive)
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - Poll Cycle not updated in the Burnin Mode!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      /* Poll Cycle will be updated automatically after disabling Burnin Mode*/         
      nErrCode = DSL_WRN_PM_POLL_CYCLE_NOT_UPDATED_IN_BURNIN_MODE;
   }
   else
#endif /* defined (INCLUDE_DSL_CPE_PM_HISTORY) && defined(INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS)*/
   {
      /* Set new PM Tick*/
      DSL_DRV_PM_CONTEXT(pContext)->nPmTick =
         DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nBasicUpdateCycle * DSL_PM_MSEC;
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ConfigGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CONFIG_GET
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ConfigGet(
   DSL_Context_t *pContext,
   DSL_PM_Config_t *pConfig)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pConfig);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get configuration data*/
   memcpy(&(pConfig->data), &(DSL_DRV_PM_CONTEXT(pContext)->nPmConfig),
      sizeof(DSL_PM_ConfigData_t));

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ConfigGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/
#endif /* INCLUDE_DSL_CPE_PM_CONFIG*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_Start(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_Context *pPmContext = DSL_NULL;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: PM module starting..." DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if(pContext->PM != DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - PM module already started" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_SUCCESS;
   }

   /* Create PM context */
   pPmContext = (DSL_PM_Context*)DSL_DRV_Malloc(sizeof(DSL_PM_Context));
   if(!pPmContext)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: PM_Start: no memory for internal context" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }
   DSL_DRV_MemSet(pPmContext, 0, sizeof(DSL_PM_Context));

   /* Initialize pointer to the PM context in the DSL CPE context*/
   pContext->PM = (DSL_void_t*)pPmContext;

   /* Create PM counters data*/
   DSL_DRV_PM_CONTEXT(pContext)->pCounters =
      (DSL_PM_CountersData_t*)DSL_DRV_Malloc(sizeof(DSL_PM_CountersData_t));
   if (DSL_DRV_PM_CONTEXT(pContext)->pCounters == DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: PM_Start: no memory for PM counters!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }
   DSL_DRV_MemSet(DSL_DRV_PM_CONTEXT(pContext)->pCounters, 0x0, sizeof(DSL_PM_CountersData_t));

   /* Create PM counters dump data*/
   DSL_DRV_PM_CONTEXT(pContext)->pCountersDump =
      (DSL_PM_CountersDump_t*)DSL_DRV_Malloc(sizeof(DSL_PM_CountersDump_t));
   if (DSL_DRV_PM_CONTEXT(pContext)->pCountersDump == DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: PM_Start: no memory for PM dump counters!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }
   DSL_DRV_MemSet(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump, 0x0, sizeof(DSL_PM_CountersDump_t));

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Set PM module line state information*/
   DSL_DRV_PM_CONTEXT(pContext)->nLineState     = DSL_LINESTATE_NOT_INITIALIZED;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

   /* init PM module common mutex */
   DSL_DRV_MUTEX_INIT(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);
   /* init PM module direction Near-End mutex */
   DSL_DRV_MUTEX_INIT(DSL_DRV_PM_CONTEXT(pContext)->pmNeMutex);
   /* init PM module direction Far-End mutex */
   DSL_DRV_MUTEX_INIT(DSL_DRV_PM_CONTEXT(pContext)->pmFeMutex);
   /* init PM module Near-End access mutex */
   DSL_DRV_MUTEX_INIT(DSL_DRV_PM_CONTEXT(pContext)->pmNeAccessMutex);
   /* init PM module Far-End access mutex */
   DSL_DRV_MUTEX_INIT(DSL_DRV_PM_CONTEXT(pContext)->pmFeAccessMutex);

   /* Enable PM Near-End polling*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.bNePollingOff = DSL_FALSE;
   /* Enable PM Far-End polling*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.bFePollingOff = DSL_FALSE;
   /* Set default poll cycle time [sec]*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nBasicUpdateCycle = 1;
   /* Set default FE poll cycle factor (applies to SHOWTIME state)*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nFeUpdateCycleFactor = 3;
   /* Set default FE L2 poll cycle factor (applies to L2 mode)*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nFeUpdateCycleFactorL2 = 10;

   /* Set Current FE polling Factor*/
   DSL_DRV_PM_CONTEXT(pContext)->nFeRequestCycle =
      DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nFeUpdateCycleFactor;
   /* Set current PM tick*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmTick =
      DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nBasicUpdateCycle * DSL_PM_MSEC;

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Set PM module default Sync Mode*/
   DSL_DRV_PM_CONTEXT(pContext)->syncMode = DSL_PM_DEFAULT_SYNC_MODE;

   /* Set PM module timeframes default values*/
   DSL_DRV_PM_CONTEXT(pContext)->nPm15Min = DSL_PM_15MIN;
   DSL_DRV_PM_CONTEXT(pContext)->nPm1Day  = DSL_PM_15MIN * DSL_PM_15MIN_PER_DAY;
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Create Line Sec Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.hist15min),
      DSL_PM_LINE_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.hist1day),
      DSL_PM_LINE_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;

   /* Create Line Init Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.hist15min),
      DSL_PM_LINE_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.hist1day),
      DSL_PM_LINE_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;

   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
   /* Set default 15-min Line Sec thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholds15min),
      0xFF,
      sizeof(DSL_pmLineSecData_t));

   /* Set default 1-day Line Sec thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.thresholds1day),
      0xFF,
      sizeof(DSL_pmLineSecData_t));

   /* Set default 15-min Line Init thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.thresholds15min),
      0xFF,
      sizeof(DSL_pmLineInitData_t));

   /* Set default 1-day Line Init thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.thresholds1day),
      0xFF,
      sizeof(DSL_pmLineInitData_t));
   #endif /* INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   /* Create Line Sec counters SHOWTIME history*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.histShowtime),
      DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1);

   /* Create Line Init counters SHOWTIME history*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.histShowtime),
      DSL_PM_LINE_SHOWTIME_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.nShowtimeInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.nShowtimeInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Create Channel Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.hist15min),
      DSL_PM_CHANNEL_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.hist1day),
      DSL_PM_CHANNEL_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;

   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
   /* Set default 15-min thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholds15min),
      0xFF,
      sizeof(DSL_pmChannelData_t));

   /* Set default 1-day thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.thresholds1day),
      0xFF,
      sizeof(DSL_pmChannelData_t));
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.histShowtime),
      DSL_PM_CHANNEL_SHOWTIME_RECORDS_NUM + 1);
   
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.nShowtimeInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Create Data Path Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.hist15min),
      DSL_PM_DATAPATH_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.hist1day),
      DSL_PM_DATAPATH_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;

   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
   /* Set default 15-min thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholds15min),
      0xFF,
      sizeof(DSL_pmDataPathData_t));

   /* Set default 1-day thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.thresholds1day),
      0xFF,
      sizeof(DSL_pmDataPathData_t));
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.histShowtime),
      DSL_PM_DATAPATH_SHOWTIME_RECORDS_NUM + 1);
   
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.nShowtimeInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Create Data Path Failure Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.hist15min),
      DSL_PM_DATAPATH_FAILURE_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.hist1day),
      DSL_PM_DATAPATH_FAILURE_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.histShowtime),
      DSL_PM_DATAPATH_FAILURE_SHOWTIME_RECORDS_NUM + 1);
   
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.nShowtimeInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Create Line Event Showtime Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.hist15min),
      DSL_PM_LINE_EVENT_SHOWTIME_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.hist1day),
      DSL_PM_LINE_EVENT_SHOWTIME_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.histShowtime),
      DSL_PM_LINE_EVENT_SHOWTIME_SHOWTIME_RECORDS_NUM + 1);
   
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.nShowtimeInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   #ifdef INCLUDE_DSL_CPE_PM_HISTORY
   /* Create ReTx Counters History*/
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.hist15min),
      DSL_PM_RETX_15MIN_RECORDS_NUM + 1);
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.hist1day),
      DSL_PM_RETX_1DAY_RECORDS_NUM + 1);

   DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n15minInvalidHist[0] =
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n1dayInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;

   #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
   #ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
   /* Set default 15-min thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholds15min),
      0xFF,
      sizeof(DSL_pmReTxData_t));

   /* Set default 1-day thresholds*/
   DSL_DRV_MemSet(
      &(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.thresholds1day),
      0xFF,
      sizeof(DSL_pmReTxData_t));
   #endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS*/
   #endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
   #endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

   #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
   DSL_DRV_PM_HistoryCreate(&(DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.histShowtime),
      DSL_PM_RETX_SHOWTIME_RECORDS_NUM + 1);
   
   DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.nShowtimeInvalidHist[0] =
      DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE;
   #endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

   /* Initialize PM module device specific parameters*/
   if( DSL_DRV_PM_DEV_Start(pContext) != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM module device specific init failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }

   DSL_DRV_PM_CONTEXT(pContext)->bInit = DSL_TRUE;

   /*
      Init PM module threads
   */
   DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.bRun = DSL_FALSE;
   DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.nThreadPollTime =
      DSL_DRV_PM_CONTEXT(pContext)->nPmTick;
   DSL_DRV_INIT_EVENT("pmev_ne", DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.pmEvent);

   /* Start PM module Near-End thread*/
   nErrCode = (DSL_Error_t)DSL_DRV_THREAD(&DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.Control,
                 "pmex_ne", DSL_DRV_PM_ThreadNe, (DSL_uint32_t)pContext);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - PM module NE thread start failed, retCode(%d)!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nErrCode));
   }

   DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.bRun = DSL_FALSE;
   DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.nThreadPollTime =
      DSL_DRV_PM_CONTEXT(pContext)->nPmTick *
      DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nFeUpdateCycleFactor;
   DSL_DRV_INIT_EVENT("pmev_fe", DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.pmEvent);

   /* Start PM module Far-End thread*/
   nErrCode = (DSL_Error_t)DSL_DRV_THREAD(&DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.Control,
                 "pmex_fe", DSL_DRV_PM_ThreadFe, (DSL_uint32_t)pContext);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - PM module FE thread start failed, retCode(%d)!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nErrCode));
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: PM module started..." DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_Stop(
   DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   if( DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL)
   {
      return DSL_SUCCESS;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Stopping PM module..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( DSL_DRV_PM_CONTEXT(pContext) == DSL_NULL || DSL_DRV_PM_CONTEXT(pContext)->bInit == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: No PM context found or PM module was not initialized"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_SUCCESS;
   }

   /* Check bPmLock flag*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bPmLock )
   {
      /*... Unlock if locked to allow PM module stop*/
      DSL_DRV_PM_UnLock(pContext);
   }

   /* Check the PM module Far-End thread active flag*/
   if( DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.bRun != DSL_TRUE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: PM module Near-End thread already stopped"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      /* Signal PM Far-End thread to stop*/
      DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.bRun = DSL_FALSE;

      DSL_DRV_WAKEUP_EVENT(DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.pmEvent);
      DSL_DRV_WAIT_COMPLETION(&DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.Control);

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM Far-End thread has stopped... (%lu)"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));
   }

   /* Check the PM module Near-End thread active flag*/
   if( DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.bRun != DSL_TRUE )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM module Near-End thread already stopped"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      /* Signal PM Near-End thread to stop*/
      DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.bRun = DSL_FALSE;

      DSL_DRV_WAKEUP_EVENT(DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.pmEvent);
      DSL_DRV_WAIT_COMPLETION(&DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.Control);

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: PM Near-End thread has stopped... (%lu)"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));
   }

   if (DSL_DRV_PM_CONTEXT(pContext)->pCounters != DSL_NULL)
   {
      DSL_DRV_MemFree(DSL_DRV_PM_CONTEXT(pContext)->pCounters);
   }

   if (DSL_DRV_PM_CONTEXT(pContext)->pCountersDump != DSL_NULL)
   {
      DSL_DRV_MemFree(DSL_DRV_PM_CONTEXT(pContext)->pCountersDump);
   }

   /* Free PM Module resources*/
   DSL_DRV_MemFree(pContext->PM);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: PM module has stopped... (%lu)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_Reset(
   DSL_Context_t *pContext,
   DSL_PM_Reset_t *pResetType)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_BF_ResetMask_t nResetMask;
   DSL_PM_EpType_t nEp2Reset[DSL_PM_COUNTER_LAST];
   DSL_uint8_t nEpCount = 0, nEpIdx = 0;

   DSL_CHECK_POINTER(pContext, pResetType);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_PM_Reset"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module processing*/
   nErrCode = DSL_DRV_PM_Lock(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - PM module lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* Get Reset Mask*/
   nResetMask = pResetType->data.bUseDetailedReset ?
                   pResetType->data.nResetMask : 0xFFFFFFFF;

   /* Specify Endpoints to reset according to the selected mask*/
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
   nEp2Reset[nEpCount++] = nResetMask & DSL_PM_RESETMASK_CHANNEL_COUNTERS ?
                              DSL_PM_COUNTER_CHANNEL : DSL_PM_COUNTER_NA;
#endif /* INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
   nEp2Reset[nEpCount++] = nResetMask & DSL_PM_RESETMASK_LINE_SEC_COUNTERS ?
                              DSL_PM_COUNTER_LINE_SEC : DSL_PM_COUNTER_NA;

   nEp2Reset[nEpCount++] = nResetMask & DSL_PM_RESETMASK_LINE_INIT_COUNTERS ?
                              DSL_PM_COUNTER_LINE_INIT : DSL_PM_COUNTER_NA;
#endif /* INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
   nEp2Reset[nEpCount++] = nResetMask & DSL_PM_RESETMASK_DATA_PATH_COUNTERS ?
                              DSL_PM_COUNTER_DATA_PATH : DSL_PM_COUNTER_NA;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
   nEp2Reset[nEpCount++] = nResetMask & DSL_PM_RESETMASK_DATA_PATH_FAILURE_COUNTERS ?
                              DSL_PM_COUNTER_DATA_PATH_FAILURE : DSL_PM_COUNTER_NA;
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
   nEp2Reset[nEpCount++] =
      nResetMask & (DSL_PM_RESETMASK_LINE_EVENT_SHOWTIME_COUNTERS | DSL_PM_RESETMASK_LINE_FAILURE_COUNTERS) ?
      DSL_PM_COUNTER_LINE_EVENT_SHOWTIME : DSL_PM_COUNTER_NA;
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
   nEp2Reset[nEpCount++] = nResetMask & DSL_PM_RESETMASK_RETX_COUNTERS ?
                              DSL_PM_COUNTER_RETX : DSL_PM_COUNTER_NA;
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

   /* Reset Endpoint Counters*/
   for (nEpIdx = 0; nEpIdx < nEpCount; nEpIdx++)
   {
      if (nEp2Reset[nEpIdx] == DSL_PM_COUNTER_NA)
         continue;
         
      /* Reset Selected Endpoint Counters*/
      nErrCode = DSL_DRV_PM_CountersReset(
                    pContext, nEp2Reset[nEpIdx], pResetType->data.nResetType);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: ERROR - Endpoint %d counters reset failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nEp2Reset[nEpIdx]));
      }
   }

   /* Unlock PM module processing*/
   DSL_DRV_PM_UnLock(pContext);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_PM_Reset"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_PM_15MIN_ELAPSED_EXT_TRIGGER
*/
DSL_Error_t DSL_DRV_PM_15MinElapsedExtTrigger(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ElapsedExtTrigger_t *pTrigger)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_boolean_t b15min = DSL_FALSE, b1day = DSL_FALSE;

   DSL_CHECK_POINTER(pContext, pTrigger);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_15MinElapsedExtTrigger" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   if (pTrigger->data.bOneDayElapsed)
   {
      /* Trigger both 15-min and 1-day intervals*/
      b15min = b1day = DSL_TRUE;
   }
   else
   {
      /* Trigger 15-min interval*/
      b15min = DSL_TRUE;
   }

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERR_SEMAPHORE_GET;
   }

   /* Check for external 1-day elapsed trigger*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bExternal1dayElapsed == DSL_FALSE )
   {
      if (b1day)
      {
         DSL_DRV_PM_CONTEXT(pContext)->bExternal1dayElapsed = DSL_TRUE;
      }
   }
   else
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: Previous 1-day elapsed trigger not proceeded yet"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_WRN_PM_PREVIOUS_EXTERNAL_TRIGGER_NOT_HANDLED;
   }

   /* Check for external 15-min elapsed trigger*/
   if( DSL_DRV_PM_CONTEXT(pContext)->bExternal15minElapsed == DSL_FALSE )
   {
      if (b15min)
      {
         DSL_DRV_PM_CONTEXT(pContext)->bExternal15minElapsed = DSL_TRUE;
      }
   }
   else
   {
      DSL_DEBUG( DSL_DBG_WRN,
         (pContext, "DSL[%02d]: Previous 15-min elapsed trigger not proceeded yet"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      nErrCode = DSL_WRN_PM_PREVIOUS_EXTERNAL_TRIGGER_NOT_HANDLED;
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_15MinElapsedExtTrigger, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return(nErrCode);
}


/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ElapsedTimeReset(
   DSL_Context_t *pContext,
   DSL_PM_ElapsedTimeReset_t *pReset)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pReset);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ElapsedTimeReset"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
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

   if (DSL_DRV_PM_CONTEXT(pContext)->syncMode != DSL_PM_SYNC_MODE_SYS_TIME)
   {
      /* Reset PM module elapsed time*/
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime =
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime  = 0;
   }
   else
   {
      nErrCode = DSL_WRN_PM_NOT_ALLOWED_IN_CURRENT_SYNC_MODE;
   }

   /* Reset Total Elapsed time*/
   DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime = 0;

   /* Reset Showtime elapsed time*/
   DSL_DRV_PM_CONTEXT(pContext)->nCurrShowtimeTime = 0;

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ElapsedTimeReset, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_SyncModeSet(
   DSL_Context_t *pContext,
   DSL_PM_SyncMode_t *pMode)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_SyncModeType_t mode;
   DSL_DRV_TimeVal_t currTime;

   DSL_CHECK_POINTER(pContext, pMode);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_SyncModeSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Check for default Sync Mode*/
   mode = pMode == DSL_NULL ? DSL_PM_DEFAULT_SYNC_MODE : pMode->data.nMode;

   if ( mode == DSL_DRV_PM_CONTEXT(pContext)->syncMode )
   {
      return DSL_SUCCESS;
   }

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERR_SEMAPHORE_GET;
   }

   switch(mode)
   {
   case DSL_PM_SYNC_MODE_FREE:
   case DSL_PM_SYNC_MODE_EXTERNAL:
      /* Reset current 15 min and 1 day time*/
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime = 0;
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime  = 0;
      break;
   case DSL_PM_SYNC_MODE_SYS_TIME:
      currTime = DSL_DRV_PM_SYS_TIME_GET();
      /* Sync to System time*/
      DSL_DRV_PM_CONTEXT(pContext)->nCurr15MinTime = currTime % (DSL_DRV_PM_CONTEXT(pContext)->nPm15Min);
      DSL_DRV_PM_CONTEXT(pContext)->nCurr1DayTime  = currTime % (DSL_DRV_PM_CONTEXT(pContext)->nPm1Day);
      break;

   default:
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Unknown Sync Mode (%d)"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), mode));
      nErrCode = DSL_ERROR;
      break;
   }

   if( nErrCode == DSL_SUCCESS )
   {
      /* Set new Sync mode in the PM module context*/
      DSL_DRV_PM_CONTEXT(pContext)->syncMode = mode;
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_SyncModeSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CONFIG_GET
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_SyncModeGet(
   DSL_Context_t *pContext,
   DSL_PM_SyncMode_t *pMode)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pMode);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_SyncModeGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
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

   pMode->data.nMode = DSL_DRV_PM_CONTEXT(pContext)->syncMode;

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_SyncModeGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_BurninModeSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_BurninMode_t *pBurninMode)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pBurninMode);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_BurninModeSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   if( pBurninMode->data.bActivate )
   {
      if( pBurninMode->data.nMode.nPmTick <  DSL_PM_COUNTER_MIN_POLLING_CYCLE ||
          pBurninMode->data.nMode.nPmTick == 0 ||
          pBurninMode->data.nMode.nPm15Min == 0 ||
          pBurninMode->data.nMode.nPm15MinPerDay == 0 )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - DSL_PM_BurninModeSet: invalid mode!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERR_INVALID_PARAMETER;
      }
   
      if( pBurninMode->data.nMode.nPmTick > (pBurninMode->data.nMode.nPm15Min*1000) )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - nPmTick can't be greater than nPm15Min!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         return DSL_ERR_INVALID_PARAMETER;
      }
   }

   /* Lock PM module Mutex*/
   if( DSL_DRV_MUTEX_LOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex) )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Couldn't lock PM mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERR_SEMAPHORE_GET;
   }

   if( pBurninMode->data.bActivate )
   {
      /* Set new timeframes values*/
      DSL_DRV_PM_CONTEXT(pContext)->nPm15Min = pBurninMode->data.nMode.nPm15Min;
      DSL_DRV_PM_CONTEXT(pContext)->nPm1Day  =
         pBurninMode->data.nMode.nPm15Min * pBurninMode->data.nMode.nPm15MinPerDay;
      DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.nThreadPollTime =
         DSL_DRV_PM_CONTEXT(pContext)->nPmTick = pBurninMode->data.nMode.nPmTick;
      DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.nThreadPollTime =
         DSL_DRV_PM_CONTEXT(pContext)->nPmTick * DSL_DRV_PM_CONTEXT(pContext)->nFeRequestCycle;
      DSL_DRV_PM_CONTEXT(pContext)->bBurninModeActive = DSL_TRUE;
   }
   else
   {
      /* restore default settings*/
      DSL_DRV_PM_CONTEXT(pContext)->nPm15Min = DSL_PM_15MIN;
      DSL_DRV_PM_CONTEXT(pContext)->nPm1Day  = DSL_PM_15MIN * DSL_PM_15MIN_PER_DAY;
      DSL_DRV_PM_CONTEXT(pContext)->pmThreadNe.nThreadPollTime =
         DSL_DRV_PM_CONTEXT(pContext)->nPmTick =
         DSL_DRV_PM_CONTEXT(pContext)->nPmConfig.nBasicUpdateCycle;
      DSL_DRV_PM_CONTEXT(pContext)->pmThreadFe.nThreadPollTime =
         DSL_DRV_PM_CONTEXT(pContext)->nPmTick * DSL_DRV_PM_CONTEXT(pContext)->nFeRequestCycle;
      DSL_DRV_PM_CONTEXT(pContext)->bBurninModeActive = DSL_FALSE;
   }

   /* Unlock PM module Mutex*/
   DSL_DRV_MUTEX_UNLOCK(DSL_DRV_PM_CONTEXT(pContext)->pmMutex);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_BurninModeSet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

/* ************************************************************************** */
/* *  xDSL Channel Endpoint interface (internal)                            * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;
   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_PM_ChannelHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n15minInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.n1dayInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCounters15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_ChannelCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExt15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExt_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersExt15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_ChannelCountersExtHistoryIntervalGet(
              pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersExt15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCounters_t   *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_ChannelCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExt1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExt_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersExt1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_ChannelCountersExtHistoryIntervalGet(
              pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersExt1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelData_t *pChCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pChCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_TOTAL(pCounters->nChannel,pCounters->nDirection);

   /* Get Total Channel Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pChCounters,
              sizeof(DSL_PM_ChannelData_t), pCounters->nDirection);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }
 
   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExtTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExtTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelDataExt_t *pChCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersExtTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pCounters->nDirection != DSL_NEAR_END)
   {
      return DSL_ERR_NOT_SUPPORTED_BY_FIRMWARE;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pChCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_TOTAL_EXT(pCounters->nChannel);

   /* Get Total Channel Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pChCounters,
              sizeof(DSL_PM_ChannelDataExt_t), pCounters->nDirection);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersExtTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds15MinSet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_ChannelData_t *pChCounters, *pThresholds;
   DSL_pmChannelThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelThresholds15MinSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_15MIN(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history item index get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   pChCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_15MIN(histIdx,pThreshs->nChannel,pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_IND(pThreshs->nChannel, pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_15MIN(pThreshs->nChannel, pThreshs->nDirection);

   /* Update Channel thresholds*/
   nErrCode = DSL_DRV_PM_ChannelThresholdsUpdate(
                 pChCounters, pThresholds, &(pThreshs->data), &(pInd->n15Min));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Channel 15-min thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelThresholds15MinSet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds1DaySet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_ChannelData_t *pChCounters, *pThresholds;
   DSL_pmChannelThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelThresholds1DaySet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_1DAY(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history item index get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      return nErrCode;
   }

   pChCounters = DSL_DRV_PM_PTR_CHANNEL_COUNTERS_1DAY(histIdx,pThreshs->nChannel,pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_IND(pThreshs->nChannel, pThreshs->nDirection);

   pThresholds = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_1DAY(pThreshs->nChannel, pThreshs->nDirection);

   /* Update Channel thresholds*/
   nErrCode = DSL_DRV_PM_ChannelThresholdsUpdate(
                 pChCounters, pThresholds, &(pThreshs->data), &(pInd->n1Day));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Channel 1-day thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelThresholds1DaySet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelThresholds15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_15MIN(pThreshs->nChannel, pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_ChannelData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Channel 15-min thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelThresholds15MinGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelThresholds1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ChannelData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelThresholds1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }


   pThresholds = DSL_DRV_PM_PTR_CHANNEL_THRESHOLD_1DAY(pThreshs->nChannel, pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_ChannelData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Channel 1-day thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ChannelThresholds1DayGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_ChannelData_t *pChCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersShowtimeGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   DSL_DRV_MemSet( &(pCounters->data), 0x0, sizeof(DSL_PM_ChannelData_t));

   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.nShowtimeTimeHist[histIdx];

         pChCounters =
            DSL_DRV_PM_PTR_CHANNEL_COUNTERS_SHOWTIME(histIdx,pCounters->nChannel,pCounters->nDirection);

         if (pChCounters != DSL_NULL)
         {
            pCounters->data.nFEC            += pChCounters->nFEC;
            pCounters->data.nCodeViolations += pChCounters->nCodeViolations;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->channelCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
      }
      
      break;
   }
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}


#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ChannelCountersExtShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_ChannelCountersExt_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_ChannelDataExt_t *pChCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ChannelCountersExtShowtimeGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   DSL_DRV_MemSet( &(pCounters->data), 0x0, sizeof(DSL_PM_ChannelDataExt_t));
   
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
      (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_CHANNEL_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->channelCounters.nShowtimeTimeHist[histIdx];

         pChCounters =
            DSL_DRV_PM_PTR_CHANNEL_COUNTERS_SHOWTIME_EXT(histIdx,pCounters->nChannel);

         if (pChCounters != DSL_NULL)
         {
            pCounters->data.nSuperFrame     += pChCounters->nSuperFrame;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->channelCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
      }

      break;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_ChannelCountersExtShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

/* ************************************************************************** */
/* *  xDSL Data-Path Endpoint interface (internal)                          * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n15minInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathHistoryStats1DayGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.n1dayInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathHistoryStats1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathCounters15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_DataPathCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_DataPathCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_DataPathData_t *pDpCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pDpCounters = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_TOTAL(pCounters->nChannel,pCounters->nDirection);

   /* Get Total Data Path Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pDpCounters,
              sizeof(DSL_PM_DataPathData_t), pCounters->nDirection);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds15MinSet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_DataPathData_t *pDpCounters, *pThresholds;
   DSL_pmDataPathThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathThresholds15MinSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_15MIN(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pDpCounters = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_15MIN(histIdx,pThreshs->nChannel,pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_IND(pThreshs->nChannel, pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_15MIN(pThreshs->nChannel, pThreshs->nDirection);

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_DataPathThresholdsUpdate(
                 pDpCounters, pThresholds, &(pThreshs->data), &(pInd->n15Min));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Data Path 15-min thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathThresholds15MinSet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds1DaySet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_DataPathData_t *pDpCounters, *pThresholds;
   DSL_pmDataPathThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathThresholds1DaySet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_1DAY(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   pDpCounters = DSL_DRV_PM_PTR_DATAPATH_COUNTERS_1DAY(histIdx,pThreshs->nChannel,pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_IND(pThreshs->nChannel, pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_1DAY(pThreshs->nChannel, pThreshs->nDirection);

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_DataPathThresholdsUpdate(
                 pDpCounters, pThresholds, &(pThreshs->data), &(pInd->n1Day));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Data Path 1-day thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathThresholds1DaySet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_DataPathData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathThresholds15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_15MIN(pThreshs->nChannel, pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_DataPathData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Data Path 15-min thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathThresholds15MinGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_DataPathThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_DataPathData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_CHANNEL_RANGE(pThreshs->nChannel);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathThresholds1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_DATAPATH_THRESHOLD_1DAY(pThreshs->nChannel, pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_DataPathData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Data Path 1-day thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathThresholds1DayGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathCountersShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_DataPathData_t *pDpCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   DSL_DRV_MemSet( &(pCounters->data), 0x0, sizeof(DSL_PM_DataPathData_t));
   
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get history item index for the specified interval*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_DATAPATH_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathCounters.nShowtimeTimeHist[histIdx];

         pDpCounters =
            DSL_DRV_PM_PTR_DATAPATH_COUNTERS_SHOWTIME(histIdx,pCounters->nChannel,pCounters->nDirection);

         if( pDpCounters != DSL_NULL )
         {
            pCounters->data.nCRC_P            += pDpCounters->nCRC_P;
            pCounters->data.nCRCP_P           += pDpCounters->nCRCP_P;
            pCounters->data.nCV_P             += pDpCounters->nCV_P;
            pCounters->data.nCVP_P            += pDpCounters->nCVP_P;
            pCounters->data.nHEC              += pDpCounters->nHEC;
            pCounters->data.nIBE              += pDpCounters->nIBE;
            pCounters->data.nTotalCells       += pDpCounters->nTotalCells;
            pCounters->data.nUserTotalCells   += pDpCounters->nUserTotalCells;
            pCounters->data.nTxUserTotalCells += pDpCounters->nTxUserTotalCells;
            pCounters->data.nTxIBE            += pDpCounters->nTxIBE;            
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->dataPathCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
      }

      break;
   }
   
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_DataPathCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

/* ************************************************************************** */
/* *  xDSL Data-Path Failure Endpoint interface (internal)                  * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_15MIN(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n15minInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureHistoryStats1DayGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_1DAY(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.n1dayInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureHistoryStats1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCounters15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_DataPathFailureCountersHistoryIntervalGet(
              pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_DataPathFailureCountersHistoryIntervalGet(
              pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathFailureCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_DataPathFailureData_t *pDpCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pDpCounters = DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_TOTAL(pCounters->nChannel,pCounters->nDirection);

   /* Get Total Data Path Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pDpCounters,
              sizeof(DSL_PM_DataPathFailureData_t), pCounters->nDirection);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_DataPathFailureCountersShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_DataPathFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_DataPathFailureData_t *pDpCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_CHANNEL_RANGE(pCounters->nChannel);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_DataPathFailureCountersShowtimeGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   DSL_DRV_MemSet(&(pCounters->data), 0x0, sizeof(DSL_PM_DataPathFailureData_t));
   
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get history item index for the specified interval*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_DATAPATH_FAILURE_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->dataPathFailureCounters.nShowtimeTimeHist[histIdx];

         pDpCounters =
            DSL_DRV_PM_PTR_DATAPATH_FAILURE_COUNTERS_SHOWTIME(histIdx,pCounters->nChannel,pCounters->nDirection);

         if( pDpCounters != DSL_NULL )
         {
            pCounters->data.nNCD += pDpCounters->nNCD;
            pCounters->data.nLCD += pDpCounters->nLCD;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->dataPathFailureCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
      }

      break;
   }
   
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_DataPathFailureCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/


/* ************************************************************************** */
/* *  xDSL Line Endpoint interface (internal)                               * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n15minInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.n1dayInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_LineSecCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_LineSecCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_LineSecCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds15MinSet(
   DSL_Context_t *pContext,
   DSL_PM_LineSecThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_LineSecData_t *pLineSecCounters, *pThresholds;
   DSL_pmLineThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecThresholds15MinSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_15MIN(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pLineSecCounters = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_15MIN(histIdx, pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_IND(pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_15MIN(pThreshs->nDirection);

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_LineSecThresholdsUpdate(
                 pLineSecCounters, pThresholds, &(pThreshs->data), &(pInd->n15Min));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Sec 15-min thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecThresholds15MinSet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds1DaySet(
   DSL_Context_t *pContext,
   DSL_PM_LineSecThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_LineSecData_t *pLineSecCounters, *pThresholds;
   DSL_pmLineThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecThresholds1DaySet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_1DAY(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pLineSecCounters = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_1DAY(histIdx, pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_IND(pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_1DAY(pThreshs->nDirection);

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_LineSecThresholdsUpdate(
                 pLineSecCounters, pThresholds, &(pThreshs->data), &(pInd->n1Day));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Sec 1-day thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecThresholds1DaySet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_LineSecThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineSecData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecThresholds15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_15MIN(pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_LineSecData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Sec 15-min thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecThresholds15MinGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineSecData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecThresholds1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_LINE_SEC_THRESHOLD_1DAY(pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_LineSecData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Sec 1-day thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecThresholds1DayGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_LineSecCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineSecData_t *pLineCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pLineCounters = DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_TOTAL(pCounters->nDirection);

   /* Get Total Line Sec Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pLineCounters,
              sizeof(DSL_PM_LineSecData_t), pCounters->nDirection);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;

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

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineSecCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineSecCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineSecCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_LineSecData_t *pLineCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineSecCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   DSL_DRV_MemSet(&(pCounters->data), 0x0, sizeof(DSL_PM_LineSecData_t));
      
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get history fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get history item index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_SEC_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineSecCounters.nShowtimeTimeHist[histIdx];

         pLineCounters =
            DSL_DRV_PM_PTR_LINE_SEC_COUNTERS_SHOWTIME(histIdx, pCounters->nDirection);

         if( pLineCounters != DSL_NULL )
         {
            pCounters->data.nES   += pLineCounters->nES;
            pCounters->data.nLOFS += pLineCounters->nLOFS;
            pCounters->data.nLOSS += pLineCounters->nLOSS;
            pCounters->data.nSES  += pLineCounters->nSES;
            pCounters->data.nUAS  += pLineCounters->nUAS;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->lineSecCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
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

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_LineSecCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStats_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0;
   
   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
      pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN(), &histFillLevel);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pStats->data.nPrevIvs        = histFillLevel;
   pStats->data.nPrevInvalidIvs =
      (DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN())->historySize - (histFillLevel - 1);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStats_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0;
   
   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitHistoryStats1DayGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
      pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY(), &histFillLevel);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pStats->data.nPrevIvs        = histFillLevel;
   pStats->data.nPrevInvalidIvs =
      (DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY())->historySize - (histFillLevel - 1);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitHistoryStats1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitCounters15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_LineInitCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitCounters1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_LineInitCountersHistoryIntervalGet(
                 pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds15MinSet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_LineInitData_t *pLineInitCounters, *pThresholds;
   DSL_pmLineThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitThresholds15MinSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_15MIN(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
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

   pLineInitCounters = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_15MIN(histIdx);

   pInd = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_IND(DSL_NEAR_END);
   pThresholds = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_15MIN();

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_LineInitThresholdsUpdate(
                 pLineInitCounters, pThresholds, &(pThreshs->data), &(pInd->n15Min));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Init 15-min thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitThresholds15MinSet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds1DaySet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_LineInitData_t *pLineInitCounters, *pThresholds;
   DSL_pmLineThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitThresholds1DaySet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_1DAY(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
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

   pLineInitCounters = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_1DAY(histIdx);

   pInd = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_IND(DSL_NEAR_END);
   pThresholds = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_1DAY();

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_LineInitThresholdsUpdate(
                 pLineInitCounters, pThresholds, &(pThreshs->data), &(pInd->n1Day));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Init 1-day thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitThresholds1DaySet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineInitData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitThresholds15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

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

   pThresholds = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_15MIN();

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_LineInitData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Init 15-min thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitThresholds15MinGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitThresholds1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineInitData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitThresholds1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

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

   pThresholds = DSL_DRV_PM_PTR_LINE_INIT_THRESHOLD_1DAY();

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_LineInitData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Line Init 1-day thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitThresholds1DayGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_THRESHOLDS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineInitData_t *pLinitCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pLinitCounters = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_TOTAL();

   /* Get Total Line Init Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pLinitCounters,
              sizeof(DSL_PM_LineInitData_t), DSL_NEAR_END);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;
   
   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineInitCountersShowtimeGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_LineInitCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_PM_LineInitData_t *pLineCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineInitCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;
   DSL_DRV_MemSet(&(pCounters->data), 0x0, sizeof(DSL_PM_LineInitData_t));

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
      /* Get history fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_SHOWTIME(), &histFillLevel);

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

      if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
      {
         DSL_DEBUG(DSL_DBG_WRN,
            (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));

         pCounters->interval.bValid       = DSL_FALSE;
         pCounters->interval.nElapsedTime = 0;
         pCounters->interval.nNumber      = 0;

         nErrCode = DSL_WRN_PM_NO_SHOWTIME_DATA;
         break;
      }

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get history item index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_INIT_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }                                                                   

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineInitCounters.nShowtimeTimeHist[histIdx];

         pLineCounters = DSL_DRV_PM_PTR_LINE_INIT_COUNTERS_SHOWTIME(histIdx);

         if( pLineCounters != DSL_NULL )
         {
            pCounters->data.nFullInits        += pLineCounters->nFullInits;
            pCounters->data.nFailedFullInits  += pLineCounters->nFailedFullInits;
            pCounters->data.nShortInits       += pLineCounters->nShortInits;
            pCounters->data.nFailedShortInits += pLineCounters->nFailedShortInits;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            (DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->lineInitCounters.nShowtimeInvalidHist[histIdx] &
               DSL_PM_INTERVAL_FAILURE_NOT_COMPLETE) == 0 ? DSL_TRUE : DSL_FALSE);
      }
      
      break;
   }
   
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, DSL_NEAR_END, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineInitCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* #ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

/* ************************************************************************** */
/* *  xDSL Line Event Showtime Endpoint interface (internal)                          * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_15MIN(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n15minInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   nErrCode = DSL_DRV_PM_LineEventShowtimeHistoryStats15MinGet(pContext, pStats);

   if (nErrCode >= DSL_SUCCESS)
   {
      nErrCode = DSL_WRN_DEPRECATED;
   }

   return nErrCode;   
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeHistoryStats1DayGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM Near-End access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_1DAY(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.n1dayInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeHistoryStats1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   nErrCode = DSL_DRV_PM_LineEventShowtimeHistoryStats1DayGet(pContext, pStats);

   if (nErrCode >= DSL_SUCCESS)
   {
      nErrCode = DSL_WRN_DEPRECATED;
   }

   return nErrCode;   
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCounters15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_LineEventShowtimeCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode = DSL_DRV_PM_LineEventShowtimeCountersHistoryIntervalGet(
              pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCounters15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_LineFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   nErrCode = DSL_DRV_PM_LineEventShowtimeCounters15MinGet(
      pContext, (DSL_PM_LineEventShowtimeCounters_t *)pCounters);

   if (nErrCode >= DSL_SUCCESS)
   {
      nErrCode = DSL_WRN_DEPRECATED;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_LineEventShowtimeCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode = DSL_DRV_PM_LineEventShowtimeCountersHistoryIntervalGet(
              pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_LineFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   nErrCode = DSL_DRV_PM_LineEventShowtimeCounters1DayGet(
      pContext, (DSL_PM_LineEventShowtimeCounters_t *)pCounters);

   if (nErrCode >= DSL_SUCCESS)
   {
      nErrCode = DSL_WRN_DEPRECATED;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_LineEventShowtimeCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_LineEventShowtimeData_t *pLfCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pLfCounters = DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_TOTAL(pCounters->nDirection);

   /* Get Total Data Path Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pLfCounters,
              sizeof(DSL_PM_LineEventShowtimeData_t), pCounters->nDirection);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_LineFailureCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   nErrCode = DSL_DRV_PM_LineEventShowtimeCountersTotalGet(
      pContext, (DSL_PM_LineEventShowtimeCountersTotal_t *)pCounters);

   if (nErrCode >= DSL_SUCCESS)
   {
      nErrCode = DSL_WRN_DEPRECATED;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_LineEventShowtimeCountersShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_LineEventShowtimeCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_LineEventShowtimeData_t *pLfCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_LineEventShowtimeCountersShowtimeGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get Interval Failures mask*/
   failuresMask = pCounters->nDirection == DSL_NEAR_END ?
      DSL_DRV_PM_INTERVAL_FAILURES_NE_MASK :
      DSL_DRV_PM_INTERVAL_FAILURES_FE_MASK;

   DSL_DRV_MemSet( &(pCounters->data), 0x0, sizeof(DSL_PM_LineEventShowtimeData_t));

   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get history item index for the specified interval*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->lineEventShowtimeCounters.nShowtimeTimeHist[histIdx];

         pLfCounters =
            DSL_DRV_PM_PTR_LINE_EVENT_SHOWTIME_COUNTERS_SHOWTIME(histIdx, pCounters->nDirection);

         if( pLfCounters != DSL_NULL )
         {
            pCounters->data.nLOF += pLfCounters->nLOF;
            pCounters->data.nLOS += pLfCounters->nLOS;
            pCounters->data.nLPR += pLfCounters->nLPR;
            pCounters->data.nLOM += pLfCounters->nLOM;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->lineEventShowtimeCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
      }

      break;
   }
   
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_LineEventShowtimeCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DEPRECATED
#ifdef INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS
DSL_Error_t DSL_DRV_PM_LineFailureCountersShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_LineFailureCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   nErrCode = DSL_DRV_PM_LineEventShowtimeCountersShowtimeGet(
      pContext, (DSL_PM_LineEventShowtimeCounters_t *)pCounters);

   if (nErrCode >= DSL_SUCCESS)
   {
      nErrCode = DSL_WRN_DEPRECATED;
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_LINE_FAILURE_COUNTERS*/
#endif /* INCLUDE_DEPRECATED */
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

/* ************************************************************************** */
/* *  xDSL ReTx Endpoint interface (internal)                          * */
/* ************************************************************************** */
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxHistoryStats15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxHistoryStats15MinGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pStats->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_RETX_HISTORY_15MIN(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_RETX_HISTORY_15MIN(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n15minInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxHistoryStats15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxHistoryStats1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_HistoryStatsChDir_t *pStats)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint32_t histFillLevel = 0, histInv = 0, prevInvIntervals = 0;
   DSL_int_t histIdx = -1;

   DSL_CHECK_POINTER(pContext, pStats);
   DSL_CHECK_ATU_DIRECTION(pStats->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxHistoryStats1DayGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pStats->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module NE access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM access mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   while(1)
   {
      /* Get Hisory fill level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
         pContext, DSL_DRV_PM_PTR_RETX_HISTORY_1DAY(), &histFillLevel);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - history fill level get failed!!" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
         
         break;
      }

      /* Set the number of previous intervals*/
      pStats->data.nPrevIvs = histFillLevel;

      for( histInv = 1; histInv <= histFillLevel; histInv++ )
      {
         /* Get History Item Index*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_RETX_HISTORY_1DAY(),
                       histInv, &histIdx);

         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index error!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
         
            nErrCode = DSL_ERROR;
            break;
         }

         prevInvIntervals +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.n1dayInvalidHist[histIdx] != 0 ?
            1 : 0;
      }

      break;
   }

   /* Set the number of previous invalid intervals*/
   pStats->data.nPrevInvalidIvs = prevInvIntervals;

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pStats->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxHistoryStats1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxCounters15MinGet(
   DSL_Context_t *pContext,
   DSL_PM_ReTxCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCounters15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 15-min interval counters*/
   nErrCode =  DSL_DRV_PM_ReTxCountersHistoryIntervalGet(
                  pContext, DSL_PM_HISTORY_INTERVAL_15MIN, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxCounters15MinGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxCounters1DayGet(
   DSL_Context_t *pContext,
   DSL_PM_ReTxCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCounters1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get 1-day interval counters*/
   nErrCode =  DSL_DRV_PM_ReTxCountersHistoryIntervalGet(
                  pContext, DSL_PM_HISTORY_INTERVAL_1DAY, pCounters);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxCounters1DayGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#ifdef INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxCountersTotalGet(
   DSL_Context_t *pContext,
   DSL_PM_ReTxCountersTotal_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ReTxData_t *pReTxCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCountersTotalGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pCounters->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Fill Total Counters elapsed time*/
   pCounters->total.nElapsedTime = DSL_DRV_PM_CONTEXT(pContext)->nPmTotalElapsedTime/DSL_PM_MSEC;

   pReTxCounters = DSL_DRV_PM_PTR_RETX_COUNTERS_TOTAL(pCounters->nDirection);

   /* Get Total Data Path Counters data*/
   nErrCode = DSL_DRV_PM_InternalCountersGet(
              pContext, (DSL_void_t*)&(pCounters->data), (DSL_void_t*)pReTxCounters,
              sizeof(DSL_PM_ReTxData_t), pCounters->nDirection);

   pCounters->total.bValid = nErrCode == DSL_SUCCESS ? DSL_TRUE : DSL_FALSE;

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxCountersTotalGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_TOTAL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
#ifdef INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds15MinSet(
   DSL_Context_t *pContext,
   DSL_PM_ReTxThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_ReTxData_t *pReTxCounters, *pThresholds;
   DSL_pmReTxThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxThresholds15MinSet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pThreshs->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_RETX_HISTORY_15MIN(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pReTxCounters = DSL_DRV_PM_PTR_RETX_COUNTERS_15MIN(histIdx, pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_RETX_THRESHOLD_IND(pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_RETX_THRESHOLD_15MIN(pThreshs->nDirection);

   /* Update ReTx thresholds*/
   nErrCode = DSL_DRV_PM_ReTxThresholdsUpdate(
                 pReTxCounters, pThresholds, &(pThreshs->data), &(pInd->n15Min));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx 15-min thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxThresholds15MinSet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds1DaySet(
   DSL_Context_t *pContext,
   DSL_PM_ReTxThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t histIdx = -1;
   DSL_PM_ReTxData_t *pReTxCounters, *pThresholds;
   DSL_pmReTxThresholdCrossingData_t *pInd ;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxThresholds1DaySet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pThreshs->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Get history item index*/
   nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                 pContext, DSL_DRV_PM_PTR_RETX_HISTORY_1DAY(), 0, &histIdx);

   if( nErrCode != DSL_SUCCESS || histIdx < 0)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return nErrCode;
   }

   pReTxCounters = DSL_DRV_PM_PTR_RETX_COUNTERS_1DAY(histIdx, pThreshs->nDirection);

   pInd = DSL_DRV_PM_PTR_RETX_THRESHOLD_IND(pThreshs->nDirection);
   pThresholds = DSL_DRV_PM_PTR_RETX_THRESHOLD_1DAY(pThreshs->nDirection);

   /* Update Data Path thresholds*/
   nErrCode = DSL_DRV_PM_ReTxThresholdsUpdate(
                 pReTxCounters, pThresholds, &(pThreshs->data), &(pInd->n1Day));

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx 1-day thresholds update failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxThresholds1DaySet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds15MinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ReTxData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxThresholds15MinGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pThreshs->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_RETX_THRESHOLD_15MIN(pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_ReTxData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx 15-min thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxThresholds15MinGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxThresholds1DayGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_PM_ReTxThreshold_t *pThreshs)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_PM_ReTxData_t *pThresholds;

   DSL_CHECK_POINTER(pContext, pThreshs);
   DSL_CHECK_ATU_DIRECTION(pThreshs->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxThresholds1DayGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pThreshs->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
   }

   /* Check if the PM module is ready*/
   if (!DSL_DRV_PM_IsPmReady(pContext))
   {
      return DSL_ERROR;
   }

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   pThresholds = DSL_DRV_PM_PTR_RETX_THRESHOLD_1DAY(pThreshs->nDirection);

   if( pThresholds )
   {
      memcpy( &(pThreshs->data), pThresholds, sizeof(DSL_PM_ReTxData_t));
   }
   else
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx 1-day thresholds get failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }

   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pThreshs->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_DRV_PM_ReTxThresholds1DayGet, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_OPTIONAL_PARAMETERS*/
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS*/

#ifdef INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS
/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_pm.h'
*/
DSL_Error_t DSL_DRV_PM_ReTxCountersShowtimeGet(
   DSL_Context_t *pContext,
   DSL_PM_ReTxCounters_t *pCounters)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_pmBF_IntervalFailures_t failuresMask = DSL_PM_INTERVAL_FAILURE_CLEANED;
   DSL_int_t histIdx = -1;
   DSL_uint32_t histFillLevel = 0, histInv = 0;
   DSL_pmBF_IntervalFailures_t nCurrFailures;
   DSL_PM_ReTxData_t *pReTxCounters;

   DSL_CHECK_POINTER(pContext, pCounters);
   DSL_CHECK_ATU_DIRECTION(pCounters->nDirection);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG( DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_DRV_PM_ReTxCountersShowtimeGet" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pCounters->nDirection == DSL_FAR_END)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - ReTx counters are not supported for the Far-End!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return DSL_ERR_NOT_SUPPORTED_BY_DEFINITION;
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

   DSL_DRV_MemSet( &(pCounters->data), 0x0, sizeof(DSL_PM_ReTxData_t));
   
   if( DSL_DRV_PM_CONTEXT(pContext)->bShowtimeProcessingStart == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: No showtime data for the specified interval"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));

      pCounters->interval.bValid       = DSL_FALSE;
      pCounters->interval.nElapsedTime = 0;
      pCounters->interval.nNumber      = 0;

      return DSL_WRN_PM_NO_SHOWTIME_DATA;
   }

   /* Clear the output structure*/
   pCounters->interval.nElapsedTime = 0;
   pCounters->interval.nNumber = (DSL_uint8_t)(pCounters->nHistoryInterval);
   pCounters->interval.bValid = DSL_TRUE;

   /* Lock PM module access mutex*/
   nErrCode = DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_TRUE);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - PM direction mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   nCurrFailures = pCounters->nHistoryInterval == 0 ? DSL_DRV_PM_CONTEXT(pContext)->nCurrFailures :
      DSL_PM_INTERVAL_FAILURE_CLEANED;

   while(1)
   {
      /* Get History Fill Level*/
      nErrCode = DSL_DRV_PM_HistoryFillLevelGet(
                    pContext, DSL_DRV_PM_PTR_RETX_HISTORY_SHOWTIME(), &histFillLevel);

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

      for( histInv = 0; histInv <= pCounters->nHistoryInterval; histInv++ )
      {
         /* Get history item index for the specified interval*/
         nErrCode = DSL_DRV_PM_HistoryItemIdxGet(
                       pContext, DSL_DRV_PM_PTR_RETX_HISTORY_SHOWTIME(),
                       histInv, &histIdx);
         if( nErrCode != DSL_SUCCESS || histIdx < 0 )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history index get failed!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            break;
         }

         /* Set Showtime Elapsed time*/
         pCounters->interval.nElapsedTime +=
            DSL_DRV_PM_CONTEXT(pContext)->pCounters->reTxCounters.nShowtimeTimeHist[histIdx];

         pReTxCounters =
            DSL_DRV_PM_PTR_RETX_COUNTERS_SHOWTIME(histIdx, pCounters->nDirection);

         if( pReTxCounters != DSL_NULL )
         {
            pCounters->data.nRxCorrected            += pReTxCounters->nRxCorrected;
            pCounters->data.nRxCorruptedTotal       += pReTxCounters->nRxCorruptedTotal;
            pCounters->data.nRxRetransmitted        += pReTxCounters->nRxRetransmitted;
            pCounters->data.nRxUncorrectedProtected += pReTxCounters->nRxUncorrectedProtected;
         }
         else
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - history interval data pointer is NULL"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
            
            nErrCode = DSL_ERR_INTERNAL;
         
            break;
         }

         pCounters->interval.bValid  &= (
            ((DSL_DRV_PM_CONTEXT(pContext)->
               pCounters->reTxCounters.nShowtimeInvalidHist[histIdx] |
               nCurrFailures) & failuresMask) == 0 ? DSL_TRUE : DSL_FALSE);
      }

      break;
   }
   
   /* Unlock PM module access mutex*/
   DSL_DRV_PM_AccessMutexControl(pContext, pCounters->nDirection, DSL_FALSE);

   if ( (nErrCode == DSL_SUCCESS) && (DSL_DRV_PM_CONTEXT(pContext)->bPmDataValid != DSL_TRUE))
   {
      nErrCode = DSL_WRN_INCOMPLETE_RETURN_VALUES;
   }

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_DRV_PM_ReTxCountersShowtimeGet, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_PM_SHOWTIME_COUNTERS*/
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

/** @} DRV_DSL_CPE_PM */

#endif /* INCLUDE_DSL_PM */
