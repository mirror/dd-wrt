/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_DEVICE_PM_H
#define _DRV_DSL_CPE_DEVICE_PM_H

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef SWIG


/**
   This function initializes PM module device specific parameters

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_Start(DSL_Context_t *pContext);

/**
   This function restarts PM module device specific stuff

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_Restart(DSL_Context_t *pContext);

/**
   PM module device specific suspend procedure

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_Suspend(DSL_Context_t *pContext);

/**
   PM module device specific handling for the showtime entry point

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_ShowtimeReachedHandle(DSL_Context_t *pContext);

#ifdef INCLUDE_DSL_CPE_API_VINAX
/**
   PM module device specific EAPS timeout handling

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_OnEapsTimeout(DSL_Context_t *pContext);
#endif /** INCLUDE_DSL_CPE_API_VINAX*/

#ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS
/**
   This function gets current Channel Counters
   directly from the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Channel for access, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Channel Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_ChannelCountersGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_ChannelData_t *pCounters);

/**
   This function gets current Extended Channel Counters
   directly from the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Channel for access, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Extended Channel Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
#if defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)
DSL_Error_t DSL_DRV_PM_DEV_ChannelCountersExtGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_ChannelDataExt_t *pCounters);
#endif /* defined (INCLUDE_DSL_CPE_PM_CHANNEL_EXT_COUNTERS)*/

/**
   This function writes saved Channel Counters
   directly to the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Channel for access, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Channel Counters, [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_ChannelCountersSet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_ChannelData_t *pCounters);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_CHANNEL_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS
/**
   This function gets current Data Path Counters
   directly from the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Specifies channel to proceed, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Channel Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_DataPathCountersGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *pCounters);

/**
   This function sets saved Data Path Counters
   directly to the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Specifies channel to proceed, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Channel Counters, [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_DataPathCountersSet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_DataPathData_t *pCounters);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS
/**
   This function gets current Data Path Failure Counters
   from the CPE API Context

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Specifies channel to proceed, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Data Path Failure Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_DataPathFailureCountersGet(
   DSL_Context_t *pContext,
   DSL_uint8_t nChannel,
   DSL_XTUDir_t nDirection,
   DSL_PM_DataPathFailureData_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_DATA_PATH_FAILURE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS
/**
   This function gets current Line Event Showtime Counters
   from the CPE API Context

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Specifies channel to proceed, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Line Event Showtime Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_LineEventShowtimeCountersGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineEventShowtimeData_t *pCounters);

/**
   This function sets Line Event Showtime Counters
   from the CPE API Context to the FW

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nChannel   Specifies channel to proceed, [I]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Line Event Showtime Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_LineEventShowtimeCountersSet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineEventShowtimeData_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_LINE_EVENT_SHOWTIME_COUNTERS*/

/**
   This function gets current Line Second Counters
   directly from the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Line Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_LineSecCountersGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineSecData_t *pCounters);

/**
   This function sets saved Line Second Counters
   directly to the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param nDirection Specifies direction, [O]
   \param pCounters  Pointer to Line Counters, [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_LineSecCountersSet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_LineSecData_t *pCounters);

#ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS
/**
   This function gets current Line Init Counters
   directly from the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param pCounters  Pointer to Line init Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_LineInitCountersGet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitData_t *pCounters);

/**
   This function restores Line Init Counters
   in the FW

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param pCounters  Pointer to Line init Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_LineInitCountersSet(
   DSL_Context_t *pContext,
   DSL_PM_LineInitData_t *pCounters);
#endif /** #ifdef INCLUDE_DSL_CPE_PM_LINE_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
/**
   This function gets current ReTx Counters
   directly from the device

   \param pContext   Pointer to dsl library context structure, [I/O]
   \param pCounters  Pointer to Line init Counters, [O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_ReTxCountersGet(
   DSL_Context_t *pContext,
   DSL_XTUDir_t nDirection,
   DSL_PM_ReTxData_t *pCounters);
#endif /* INCLUDE_DSL_CPE_PM_RETX_COUNTERS*/

#ifdef INCLUDE_DSL_CPE_PM_HISTORY
/**
   This function alignes current PM history interval

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_DRV_PM_DEV_HistoryIntervalAlign(
   DSL_Context_t *pContext);
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/

#endif /* #ifndef SWIG*/

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_DEVICE_PM_H */
