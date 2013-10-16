/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef DSL_TIMEOUT_H
#define DSL_TIMEOUT_H

/** \file
   Timeout support
*/

#ifdef DSL_INTERN

#include "drv_dsl_cpe_api.h"

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */


typedef struct DSL_TimeoutElement_s DSL_TimeoutElement_t;

/**
   Structure for storing timeout event informations
*/
struct DSL_TimeoutElement_s
{
   /**
      Pointer to previous element */
   DSL_TimeoutElement_t *pPrevious;
   /**
      Pointer to next element */
   DSL_TimeoutElement_t *pNext;
   /**
      Marks whether the element is valid (DSL_TRUE) or not (DSL_FALSE) */
   DSL_boolean_t bValid;
   /**
      Time on which the timeout event has been started [msec] */
   DSL_DRV_TimeVal_t nStartTime;
   /**
      Timout value [s] */
   DSL_uint32_t nTimeout;
   /**
      Time on which the element will timeout [msec] */
   DSL_DRV_TimeVal_t nStopTime;
   /**
      Specifies an identifier that has to be given by the used if adding a
      timeout event and which will be returned on timeout of event. This
      value shall be used to react on the timeout. */
   DSL_int_t nEventType;
};

/**
   Structure for storing management information on all initialized timeout
   element lists.
*/
typedef struct
{
   /**
      Head of specific timeout event list */
   DSL_TimeoutElement_t *pListHead;
   /**
      Tail of specific timeout event list */
   DSL_TimeoutElement_t *pListTail;
} DSL_TimeoutList_t;


#define DSL_TIMEOUT_LIST_LOCK() \
do{ \
   if( DSL_DRV_MUTEX_LOCK(pContext->bspMutex) ) \
   { \
      DSL_DEBUG_SET_ERROR(DSL_ERROR); \
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL: ERROR: failed to lock access to the timeouts list!" DSL_DRV_CRLF)); \
      return DSL_ERROR; \
   } \
}while(0)

#define DSL_TIMEOUT_LIST_UNLOCK() \
do{ \
   DSL_DRV_MUTEX_UNLOCK(pContext->bspMutex); \
}while(0)

/** timeout context structure */
typedef struct
{
   /**
      Specifies whether to allocate new list elements during runtime if
      necessary or not */
   DSL_boolean_t bDynMemAlloc;
   /**
      Number of available list elements within one pointer list pTimeoutList */
   DSL_uint32_t nNrOfElements;
   /**
      Single timeout list control structure includes pointers to head and tail
      of each timeout list. */
   DSL_TimeoutList_t TimeoutList;
} DSL_TimeoutContext_t;


DSL_Error_t DSL_DRV_Timeout_Init(
   DSL_Context_t *pContext,
   DSL_uint32_t nNrOfElements,
   DSL_boolean_t bDynMemAlloc
);

DSL_Error_t DSL_DRV_Timeout_Shutdown(
   DSL_Context_t *pContext
);

DSL_uint32_t DSL_DRV_Timeout_GetTotalSizeOfLists(
   DSL_Context_t *pContext
);

DSL_Error_t DSL_DRV_Timeout_GetNextActiveEvent(
   DSL_Context_t *pContext,
   DSL_int_t *nEventType,
   DSL_uint32_t *nTimeoutID
);

DSL_Error_t DSL_DRV_Timeout_Reset(
   DSL_Context_t *pContext,
   DSL_uint32_t nTimeoutID,
   DSL_uint32_t nNewTimeout
);

DSL_uint32_t DSL_DRV_Timeout_AddEvent(
   DSL_Context_t *pContext,
   DSL_int_t nEventType,
   DSL_uint32_t nTimeout
);

DSL_Error_t DSL_DRV_Timeout_RemoveEvent(
   DSL_Context_t *pContext,
   DSL_uint32_t nTimeoutID
);

DSL_Error_t DSL_DRV_Timeout_RemoveAllEvents(
   DSL_Context_t *pContext
);


#ifndef DSL_DEBUG_DISABLE
DSL_Error_t DSL_DRV_Timeout_Debug_PrintTimeoutList(
   DSL_Context_t *pContext
);
#endif /* DSL_DEBUG_DISABLE*/

/** @} DRV_DSL_CPE_COMMON */

#endif /* #ifdef DSL_INTERN */

#endif /* DSL_TIMEOUT_H */


