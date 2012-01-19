/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"
#include "drv_dsl_cpe_timeout.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_CPE_API

#if defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))

/** \file
   Timeout support
*/

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

/*
   Initializes the timeout element lists with given number of max. init
   elements. The number of lists that will be created is defined by
   \ref DSL_MAX_TIMEOUT_LISTS.

   \param pContext      Pointer to dsl cpe library context structure, [I]
   \param nNrOfElements Specifies the number of init timeout elements for each
                        timeout list, [I]
   \param bDynMemAlloc  Specifies if the timout element list should be
                        dynamically extended (DSL_TRUE) if necessary or not
                        (DSL_FALSE), [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
*/
DSL_Error_t DSL_DRV_Timeout_Init(
   DSL_Context_t *pContext,
   DSL_uint32_t nNrOfElements,
   DSL_boolean_t bDynMemAlloc)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL;
   DSL_TimeoutElement_t *pPreListElement = DSL_NULL;
   DSL_uint32_t j = 0;
   DSL_char_t            fname[64];

   /* Allocate and initialize all defined timeout list */
   pPreListElement = DSL_NULL;

   DSL_DRV_snprintf(fname, sizeof(fname), "tmo_lst");

   DSL_TIMEOUT_LIST_LOCK();
   for (j = 0; j < nNrOfElements; j++)
   {
      pListElement = DSL_DRV_Malloc(sizeof(DSL_TimeoutElement_t));
      if (pListElement == DSL_NULL)
      {
         DSL_DEBUG_SET_ERROR(DSL_ERROR);
         DSL_DEBUG( DSL_DBG_ERR, (pContext,
            "DSL[%02d]: Error during memory allocation for timeout element "
            "(ElementIdx=%d)!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext), j));
            
         return DSL_ERROR;
      }
      else
      {
         memset((DSL_void_t *)pListElement, 0, sizeof(DSL_TimeoutElement_t));

         if (j == 0)
         {
            pTCtx->TimeoutList.pListHead = pListElement;
         }
         else
         {
            if(pPreListElement)
            {
               pPreListElement->pNext = pListElement;
               pListElement->pPrevious = pPreListElement;
            }
         }

         if (j == (nNrOfElements - 1))
         {
            pTCtx->TimeoutList.pListTail = pListElement;
         }

         pPreListElement = pListElement;
      }
   }
   DSL_TIMEOUT_LIST_UNLOCK();

   pTCtx->bDynMemAlloc = bDynMemAlloc;
   pTCtx->nNrOfElements = nNrOfElements;

   return DSL_SUCCESS;
}

/*
   Deletes all allocated memory for timeout event handling.

   \param pContext Pointer to dsl library context structure, [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
*/
DSL_Error_t DSL_DRV_Timeout_Shutdown(
   DSL_Context_t *pContext)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL;
   DSL_TimeoutElement_t *pFreeListElement = DSL_NULL;

   pListElement = pTCtx->TimeoutList.pListHead;

   DSL_TIMEOUT_LIST_LOCK();
   while (pListElement != DSL_NULL)
   {
      pFreeListElement = pListElement;
      pListElement = pListElement->pNext;

      if (pFreeListElement != DSL_NULL)
      {
         DSL_DRV_MemFree(pFreeListElement);
      }
   };

   pTCtx->TimeoutList.pListHead = DSL_NULL;
   DSL_TIMEOUT_LIST_UNLOCK();

   return DSL_SUCCESS;
}

/*
   This routine returns the actually memory size of all timeout events lists in
   number of bytes.
   \note The size of the management control structures are not included because
         theay are already included within the size of the DSL API context
         structure (statically allocated)

   \param pContext   Pointer to dsl library context structure, [I]

   \return
   Returns the size of the all timeout event lists in number of bytes.
   In case of an error the size is equal to 0.
*/
DSL_uint32_t DSL_DRV_Timeout_GetTotalSizeOfLists(
   DSL_Context_t *pContext)
{
   DSL_uint32_t nSize = 0U;
   DSL_TimeoutContext_t *pTCtx;

   if(pContext == DSL_NULL)
      return 0;

   pTCtx = &pContext->TimeoutListsContext;

   nSize = pTCtx->nNrOfElements *
           sizeof(DSL_TimeoutElement_t);

   return nSize;
}

/*
   This routine returns the next timeout element that has been timed out.

   \param pContext   Pointer to dsl library context structure, [I]
   \param nEventType Returns the timeout event ID which has been returned from
                     function \ref DSL_Timeout_AddEvent. If there is no timed out
                     event actually pending the value will be 0, [O]
   \param nTimeoutID Returns the timeout element id. This unique value identifies
                     the timeout event. It might be used to remove the timeout
                     event later on (Note: this value was also returned from
                     function which adds this event formerly), [I]

   \return
   - DSL_Success In case of timeout element has been timed out. In this case
     the 'nEventType' value includes the type of the timed out event as
     specified during adding of event.
   - DSL_Error In case if there is actually no timed out element
*/
DSL_Error_t DSL_DRV_Timeout_GetNextActiveEvent(
   DSL_Context_t *pContext,
   DSL_int_t *nEventType,
   DSL_uint32_t *nTimeoutID)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListHead = DSL_NULL;
   DSL_DRV_TimeVal_t nActTime;

   nActTime = DSL_DRV_TimeMSecGet();
   DSL_TIMEOUT_LIST_LOCK();
   pListHead = pTCtx->TimeoutList.pListHead;

   if (pListHead->bValid == DSL_TRUE)
   {
      if (pListHead->nStopTime <= nActTime)
      {
         *nEventType = pListHead->nEventType;
         *nTimeoutID = (DSL_uint32_t)pListHead;
         DSL_TIMEOUT_LIST_UNLOCK();
         return DSL_SUCCESS;
      }
   }

   DSL_TIMEOUT_LIST_UNLOCK();
   return DSL_ERROR;
}

/*
   This routine adjusts the timeout event time.

   \param pContext   Pointer to dsl library context structure, [I]
   \param nTimeoutID Returns the timeout element id. This unique value identifies
                     the timeout event. It might be used to remove the timeout
                     event later on (Note: this value was also returned from
                     function which adds this event formerly), [I]
   \param nNewTimeout Specifies the timeout time in mili seconds. The actual system
                     time will be automatically requested, [I]

   \return
   - DSL_Success In case of timeout element has been timed out. In this case
     the 'nEventType' value includes the type of the timed out event as
     specified during adding of event.
   - DSL_Error In case if there is actually no timed out element
*/
DSL_Error_t DSL_DRV_Timeout_Reset(
   DSL_Context_t *pContext,
   DSL_uint32_t nTimeoutID,
   DSL_uint32_t nNewTimeout)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL, *pListResetElement = DSL_NULL;
   DSL_uint32_t i = 0;

   DSL_TIMEOUT_LIST_LOCK();
   pListElement = pTCtx->TimeoutList.pListHead;
   pListResetElement = (DSL_TimeoutElement_t *)nTimeoutID;

   if (nTimeoutID != 0)
   {
      for (i = 0; i < pTCtx->nNrOfElements; i++)
      {
         if (pListElement == pListResetElement)
            break;

         pListElement = pListElement->pNext;

         if (pListElement == DSL_NULL)
         {
            DSL_TIMEOUT_LIST_UNLOCK();
            DSL_DEBUG_SET_ERROR(DSL_ERROR);
            DSL_DEBUG( DSL_DBG_ERR, (pContext,
               "DSL[%02d]: Error in 'DSL_Timeout_Reset'- Element ("
               "nTimeoutID=0x%08X) could not be reset (not found)!" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), nTimeoutID));
               
            return DSL_ERROR;
         }
      }
   }

   pListElement->nStopTime = pListElement->nStartTime + nNewTimeout;

   DSL_DEBUG( DSL_DBG_MSG, (pContext,
      "DSL[%02d]: Reset timeout event (nEventType=%d, "
      "pListElement=0x%08X) successfully!" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), pListElement->nEventType,
      (DSL_uint32_t)pListElement));

   DSL_TIMEOUT_LIST_UNLOCK();
   return DSL_SUCCESS;
}

/*
   This routine adds a timeout element to the list of avialable timeouts.
   The timeout element will be taken from the available list of empty timeouts
   (from tail of list) set with necessary values and added to the list of active
   elements to have a list of increasing timeout values.

   \param pContext   Pointer to dsl library context structure, [I]
   \param nEventType Specifies the timeout event type which will be returned
                     later on in case of timeout has been timed out. The value
                     has to be unequal to 0, [I]
   \param nTimeout   Specifies the timeout time in mili seconds. The actual system
                     time will be automatically requested, [I]

   \return
   returns the nTimeoutID of the element which has to be used for removing the
   timeout event later on. In case of an error the ID is equal to 0.
*/
DSL_uint32_t DSL_DRV_Timeout_AddEvent(
   DSL_Context_t *pContext,
   DSL_int_t nEventType,
   DSL_uint32_t nTimeout)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL;
   DSL_TimeoutElement_t *pListInsertElement = DSL_NULL;
   DSL_TimeoutElement_t *pPreListElement = DSL_NULL;
   /*DSL_TimeoutElement_t *pNextListElement = DSL_NULL;*/
   DSL_uint32_t nRetVal = 0;
   DSL_boolean_t bInsert = DSL_FALSE;
   DSL_boolean_t bFirst = DSL_FALSE, bLast = DSL_FALSE;
   DSL_uint32_t i = 0;
   DSL_DRV_TimeVal_t nActTime;

   nActTime = DSL_DRV_TimeMSecGet();

   /* Lock Timeout list*/
   if( DSL_DRV_MUTEX_LOCK(pContext->bspMutex) )
   {
      DSL_DEBUG_SET_ERROR(DSL_ERROR);
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL: ERROR: failed to lock access to the timeouts list!"
         DSL_DRV_CRLF));
      return 0;
   }
   
   /* Check if timeout elements are available by examining 'bValid' flag.
      If the last element is also used, there are no more free elements
      In case of choosing dynamic memory allocation in 'DSL_Timeout_Init'
      a new element will be automatically allocated otherwise an error will
      be returned.  */
   if (pTCtx->TimeoutList.pListTail->bValid == DSL_TRUE)
   {
      if (pTCtx->bDynMemAlloc == DSL_FALSE)
      {
         DSL_TIMEOUT_LIST_UNLOCK();
         DSL_DEBUG_SET_ERROR(DSL_ERROR);
         DSL_DEBUG( DSL_DBG_ERR, (pContext,
            "DSL[%02d]: Error in 'DSL_Timeout_AddEvent' ("
            "nEventType=%d) - No free timeout elements available!" DSL_DRV_CRLF ,
            DSL_DEV_NUM(pContext), nEventType));
            
         return 0;
      }
      else
      {
         DSL_TIMEOUT_LIST_UNLOCK();
         DSL_DEBUG_SET_ERROR(DSL_ERROR);
         DSL_DEBUG( DSL_DBG_ERR, (pContext,
            "DSL[%02d]: Error in 'DSL_Timeout_AddEvent'- Dynamic memory allocation "
            "not supported yet!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
            
         return 0;
      }
   }

   pListElement = pTCtx->TimeoutList.pListTail;
   pPreListElement = pListElement->pPrevious;

   /* if there is more than one element in the list,
      remove last element from list */
   if (pPreListElement)
   {
      pPreListElement->pNext = DSL_NULL;
      pTCtx->TimeoutList.pListTail = pPreListElement;
   }

   /* Set values in removed list element */
   pListElement->bValid = DSL_TRUE;

   pListElement->nStartTime = nActTime;
   pListElement->nTimeout = nTimeout;
   pListElement->nStopTime = nActTime + nTimeout;
   pListElement->nEventType = nEventType;
   nRetVal = (DSL_uint32_t)pListElement;

   /* if there is more than one element in the list,
      add the removed element in place of ascending timeout order.
      Otherwise no insert is needed at all. */
   if (pPreListElement)
   {
      pListInsertElement = pTCtx->TimeoutList.pListHead;
      bInsert = DSL_FALSE;
      for (i = 0; i < pTCtx->nNrOfElements; i++)
      {
         if (pListInsertElement->bValid == DSL_FALSE)
         {
            bInsert = DSL_TRUE;
            if (pListInsertElement->pPrevious == DSL_NULL)
            {
               bFirst = DSL_TRUE;
            }
         }
         else
         {
            if (pListElement->nStopTime < pListInsertElement->nStopTime)
            {
               bInsert = DSL_TRUE;
               if (pListInsertElement->pPrevious == DSL_NULL)
               {
                  bFirst = DSL_TRUE;
               }
            }
         }

         if ( (bInsert == DSL_FALSE) && (pListInsertElement->pNext == DSL_NULL) )
         {
            bInsert = DSL_TRUE;
            bLast = DSL_TRUE;
         }

         if (bInsert == DSL_FALSE)
         {
            /* check next element */
            pListInsertElement = pListInsertElement->pNext;
         }
         else
         {
            /* Found entry point */
            pPreListElement = pListInsertElement->pPrevious;
            /*pNextListElement = pListInsertElement->pNext;*/

            if (bFirst == DSL_TRUE)
            {
               pListInsertElement->pPrevious = pListElement;
               pListElement->pNext = pListInsertElement;
               pTCtx->TimeoutList.pListHead = pListElement;
               pListElement->pPrevious = DSL_NULL;
            }
            else if (bLast == DSL_TRUE)
            {
               pListInsertElement->pNext = pListElement;
               pListElement->pPrevious = pListInsertElement;
               pTCtx->TimeoutList.pListTail = pListElement;
               pListElement->pNext = DSL_NULL;
            }
            else
            {
               /* Intermediate */
               pPreListElement->pNext = pListElement;
               pListElement->pPrevious = pPreListElement;
               pListElement->pNext = pListInsertElement;
               pListInsertElement->pPrevious = pListElement;
            }
            break;
         }
      }
   }
   else
   {
      bInsert = DSL_TRUE;
   }

   if (bInsert == DSL_FALSE)
   {
      DSL_DEBUG_SET_ERROR(DSL_ERROR);
      DSL_DEBUG( DSL_DBG_ERR, (pContext,
         "DSL[%02d]: Error in 'DSL_Timeout_AddEvent'- Element ("
         "nEventType=%d)could not be added!" DSL_DRV_CRLF ,
         DSL_DEV_NUM(pContext), nEventType));
         
      nRetVal = 0;
   }
   else
   {
      /*DSL_DEBUG( DSL_DBG_LOCAL, (pContext,
         "DSL: Added timeout event (ListIdx=%d, nLine=%hu, nEventType=%d, "
         "pListElement=0x%08X) successfully!" DSL_DRV_CRLF , nListIdx, nLine, nEventType,
         (DSL_uint32_t)pListElement));*/
   }
   DSL_TIMEOUT_LIST_UNLOCK();

   return nRetVal;
}

/*
   This routine removes a given timeout element from the list of available
   timeout elements and adds the (empty) element at the end of the list.

   \param pContext   Pointer to dsl library context structure, [I]
   \param nTimeoutID Identifies the timeout element ID. This unique value was
                     was also returned from the function
                     (\ref DSL_Timeout_AddEvent) which adds this event formerly.
                     If the first element from the list should be removed it is
                     also possible to set this value to 0, [I]

   \return
   - DSL_Success Timeout event handled successfully
   - DSL_Error Error during handling of timeout event
*/
DSL_Error_t DSL_DRV_Timeout_RemoveEvent(
   DSL_Context_t *pContext,
   DSL_uint32_t nTimeoutID)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL;
   DSL_TimeoutElement_t *pListTail = DSL_NULL;
   DSL_TimeoutElement_t *pListRemoveElement = DSL_NULL;
   DSL_TimeoutElement_t *pPreListElement = DSL_NULL;
   DSL_TimeoutElement_t *pNextListElement = DSL_NULL;
   DSL_boolean_t bFirst = DSL_FALSE, bLast = DSL_FALSE;
   DSL_boolean_t bRemove = DSL_FALSE;
   DSL_uint32_t i = 0;

   DSL_TIMEOUT_LIST_LOCK();
   pListElement = pTCtx->TimeoutList.pListHead;
   pListRemoveElement = (DSL_TimeoutElement_t *)nTimeoutID;

   if (nTimeoutID == 0)
   {
      bFirst = DSL_TRUE;
      bRemove = DSL_TRUE;
      /* the first can also be the last */
      if (pTCtx->nNrOfElements == 1)
         bLast = DSL_TRUE;
   }
   else
   {
      for (i = 0; i < pTCtx->nNrOfElements; i++)
      {
         if (pListElement == pListRemoveElement)
         {
            bRemove = DSL_TRUE;
         }

         if (bRemove == DSL_TRUE)
         {
            if (pListElement->pNext == DSL_NULL)
            {
               bLast = DSL_TRUE;
            }
            else if (pListElement->pPrevious == DSL_NULL)
            {
               bFirst = DSL_TRUE;
            }
            break;
         }

         pListElement = pListElement->pNext;

         if (pListElement == DSL_NULL)
         {
            DSL_TIMEOUT_LIST_UNLOCK();
            DSL_DEBUG_SET_ERROR(DSL_ERROR);
            DSL_DEBUG( DSL_DBG_ERR, (pContext,
               "DSL[%02d]: Error in 'DSL_Timeout_RemoveEvent'- Element ("
               "nTimeoutID=0x%08X) could not be removed (not found)!" DSL_DRV_CRLF ,
               DSL_DEV_NUM(pContext), nTimeoutID));
               
            return DSL_ERROR;
         }
      }
   }

   pListTail = pTCtx->TimeoutList.pListTail;
   pPreListElement = pListElement->pPrevious;
   pNextListElement = pListElement->pNext;

   if (bLast == DSL_TRUE)
   {
      /* Do nothing here because:
         a) values will be reset afterwards
         b) timeout element is already on right position (at the end) */
   }
   else
   {
      if (bFirst == DSL_TRUE)
      {
         pTCtx->TimeoutList.pListHead = pNextListElement;
         pNextListElement->pPrevious = DSL_NULL;
      }
      else
      {
         /* Intermediate */
         pPreListElement->pNext = pNextListElement;
         pNextListElement->pPrevious = pPreListElement;
      }

      pListTail->pNext = pListElement;
      pListElement->pPrevious = pListTail;
      pListElement->pNext = DSL_NULL;
      pTCtx->TimeoutList.pListTail = pListElement;
   }

   /*DSL_DEBUG( DSL_DBG_LOCAL, (pContext,
      "DSL: Removed timeout event (ListIdx=%d, nLine=%hu, nEventType=%d, "
      "pListElement=0x%08X) successfully!" DSL_DRV_CRLF ,
      nListIdx, pListElement->nLine, pListElement->nEventType,
      (DSL_uint32_t)pListElement));*/

   /* Reset content of timeout element */
   pListElement->bValid = DSL_FALSE;

   pListElement->nStartTime = 0x0;
   pListElement->nTimeout = 0;
   pListElement->nStopTime = 0x0;
   pListElement->nEventType = 0;
   DSL_TIMEOUT_LIST_UNLOCK();

   return DSL_SUCCESS;
}

/*
   This routine removes valid element from the list of available
   timeout elements.

   \param pContext   Pointer to dsl library context structure, [I]

   \return
   - DSL_Success in case of success
   - DSL_Error if operation failed
*/
DSL_Error_t DSL_DRV_Timeout_RemoveAllEvents( DSL_Context_t *pContext )
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL;

   pListElement = pTCtx->TimeoutList.pListHead;

   DSL_TIMEOUT_LIST_LOCK();
   while (pListElement != DSL_NULL)
   {
      /* skip not valid */
      if (pListElement->bValid == DSL_FALSE)
      {
         pListElement = pListElement->pNext;
         continue;
      }
      /* Reset content of timeout element */
      pListElement->bValid = DSL_FALSE;
      pListElement->nStartTime = 0x0;
      pListElement->nTimeout = 0;
      pListElement->nStopTime = 0x0;
      pListElement->nEventType = 0;

      pListElement = pListElement->pNext;
   };
   DSL_TIMEOUT_LIST_UNLOCK();

   return DSL_SUCCESS;
}

#ifndef DSL_DEBUG_DISABLE
/*
   Print out contents of timout context structure and all of its timeout element
   list content.
   \note This function is implemented for debug purpose only.

   \param pContext Pointer to dsl library context structure, [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS in case of success
   - DSL_ERROR if operation failed
*/
DSL_Error_t DSL_DRV_Timeout_Debug_PrintTimeoutList(
   DSL_Context_t *pContext)
{
   DSL_TimeoutContext_t *pTCtx = &pContext->TimeoutListsContext;
   DSL_TimeoutElement_t *pListElement = DSL_NULL;
   DSL_TimeoutElement_t *pNextListElement = DSL_NULL;
   DSL_uint32_t j = 0;
   DSL_boolean_t bEnd = DSL_FALSE;

   DSL_DEBUG(DSL_DBG_MSG,
               (pContext, "DSL[%02d]: ****** Content of timeout event list ******" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
   DSL_DEBUG(DSL_DBG_MSG,
               (pContext, "DSL[%02d]: nNrOfElements=%d" DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext), pTCtx->nNrOfElements));


   DSL_TIMEOUT_LIST_LOCK();

   pListElement = pTCtx->TimeoutList.pListHead;

   DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: Content of list" DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: pListHead=0x%08X" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), (DSL_uint32_t)(pTCtx->TimeoutList.pListHead)));
   DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: pListTail=0x%08X" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), (DSL_uint32_t)(pTCtx->TimeoutList.pListTail)));

   for (j = 0; j < pTCtx->nNrOfElements; j++)
   {
      pNextListElement = pListElement->pNext;

      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    Content of element %d" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), j));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    pList=0x%08X" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), (DSL_uint32_t)(pListElement)));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    pPrevious=0x%08X" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), (DSL_uint32_t)(pListElement->pPrevious)));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    pNext=0x%08X" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), (DSL_uint32_t)(pListElement->pNext)));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    bValid=%d" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), (DSL_int_t)(pListElement->bValid)));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    nStartTime=%u" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pListElement->nStartTime));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    nTimeout=%u" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pListElement->nTimeout));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    nStopTime=%u" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pListElement->nStopTime));
      DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]:    nEventType=%d" DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), pListElement->nEventType));

      if (bEnd == DSL_TRUE)
      {
         break;
      }

      pListElement = pNextListElement;

      if (pListElement->pNext == DSL_NULL)
      {
         bEnd = DSL_TRUE;
      }
   }

   DSL_TIMEOUT_LIST_UNLOCK();

   return DSL_SUCCESS;
}
#endif /* DSL_DEBUG_DISABLE*/


/** @} DRV_DSL_CPE_COMMON */

#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) || (defined(INCLUDE_DSL_CPE_API_DANUBE) && defined(INCLUDE_DSL_G997_LINE_INVENTORY))*/
