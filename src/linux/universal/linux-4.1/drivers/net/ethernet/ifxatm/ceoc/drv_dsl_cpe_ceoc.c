/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#if defined(INCLUDE_DSL_CEOC)

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_CEOC

static DSL_Error_t DSL_CEOC_FifoMessageWrite(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg);

#ifdef INCLUDE_DSL_CEOC_INTERNAL_API
static DSL_Error_t DSL_CEOC_CallbackHandle(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg);
#endif

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */


static DSL_int_t DSL_CEOC_Thread(DSL_DRV_ThreadParams_t *param)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t nOsRet = 0;
   DSL_Context_t *pContext = (DSL_Context_t*)param->nArg1;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;
   DSL_CEOC_Message_t ceocMsg;
   DSL_uint16_t protIdent = 0x0;

   /* Check DSL CPE context pointer*/
   if( pContext == DSL_NULL || DSL_CEOC_CONTEXT(pContext) == DSL_NULL )
      return -1;

   /* Check if the CEOC was initialized*/
   if( DSL_CEOC_CONTEXT(pContext)->bInit != DSL_TRUE )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC module not initialized!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return -1;
   }

   DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun = DSL_TRUE;

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: CEOC thread started"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* main CEOC module Task*/
   while( DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun )
   {
      DSL_DRV_WAIT_EVENT_TIMEOUT( DSL_CEOC_CONTEXT(pContext)->ceocThread.waitEvent,
                              DSL_CEOC_CONTEXT(pContext)->ceocThread.nThreadPollTime);

      /* Only proceed if the specified line is in SHOWTIME state*/
      DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

      /* Only proceed if the specified line is in SHOWTIME state*/
      if ((nCurrentState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
          (nCurrentState != DSL_LINESTATE_SHOWTIME_NO_SYNC))
      {
         continue;
      }

      /* Clear message length*/
      ceocMsg.length = 0x0;
      if( DSL_CEOC_DEV_MessageReceive(pContext, &protIdent, &ceocMsg) < DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - CEOC message receive failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERROR;
         
         break;
      }

      if( !ceocMsg.length )
      {
         /* No data available, continue polling*/
         continue;
      }

       /* EOC message available. Write the received message to the internal FIFO*/
      if( DSL_CEOC_FifoMessageWrite(pContext, protIdent, &ceocMsg) < DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR, (pContext,
            "DSL[%02d]: ERROR - CEOC message write failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERROR;
         
         break;
      }

      /* Proceed message for the Internal CEOC API*/
      #ifdef INCLUDE_DSL_CEOC_INTERNAL_API
      if( DSL_CEOC_CallbackHandle(pContext,protIdent,&ceocMsg) < DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - CallBack Handle failed!!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERROR;
         
         break;
      }
      #endif
   }

   /* Clear CEOC module bRun flag*/
   DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun = DSL_FALSE;

   nOsRet = DSL_DRV_ErrorToOS(nErrCode);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: CEOC thread stoped"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   return nOsRet;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_ceoc.h'
*/
DSL_Error_t DSL_CEOC_Start(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CEOC_Context_t *pCeocContext = DSL_NULL;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: CEOC module starting..." DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if(pContext->CEOC != DSL_NULL)
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - CEOC already started" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_SUCCESS;
   }

   /* Create CEOC context */
   pCeocContext = (DSL_CEOC_Context_t*)DSL_DRV_Malloc(sizeof(DSL_CEOC_Context_t));
   if(!pCeocContext)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: DSL_CEOC_Start: no memory for internal context" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }
   memset(pCeocContext, 0, sizeof(DSL_CEOC_Context_t));

   /* Initialize pointer to the CEOC context in the DSL CPE context*/
   pContext->CEOC = (DSL_void_t*)pCeocContext;

   /* init CEOC module common mutex */
   DSL_DRV_MUTEX_INIT(DSL_CEOC_CONTEXT(pContext)->ceocMutex);

#ifdef INCLUDE_DSL_CEOC_INTERNAL_API
   /* init CEOC module internal API mutex */
   DSL_DRV_MUTEX_INIT(DSL_CEOC_CONTEXT(pContext)->ceocInternMutex);
#endif /* #ifdef INCLUDE_DSL_CEOC_INTERNAL_API*/

   /* Initialize CEOC module device specific parameters*/
   if( DSL_CEOC_DEV_Start(pContext) != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC module device specific init failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   DSL_CEOC_CONTEXT(pContext)->bInit = DSL_TRUE;

   /*
      Init CEOC module threads
   */
   DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun = DSL_FALSE;
   DSL_CEOC_CONTEXT(pContext)->ceocThread.nThreadPollTime = DSL_CEOC_THREAD_POLLING_CYCLE;
   DSL_DRV_INIT_EVENT("ceocev_ne", DSL_CEOC_CONTEXT(pContext)->ceocThread.waitEvent);

   /* Start CEOC module thread*/
   nErrCode = (DSL_Error_t)DSL_DRV_THREAD(&DSL_CEOC_CONTEXT(pContext)->ceocThread.Control,
                              "ceocex_ne", DSL_CEOC_Thread, (DSL_uint32_t)pContext);

   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: ERROR - CEOC module thread start failed, retCode(%d)!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), nErrCode));
   }
   else
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: CEOC module started..." DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_ceoc.h'
*/
DSL_Error_t DSL_CEOC_Stop(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_CTX_POINTER(pContext);
   DSL_CHECK_ERR_CODE();

   if( !DSL_CEOC_CONTEXT(pContext) )
   {
      return DSL_SUCCESS;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Stopping CEOC module..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( DSL_CEOC_CONTEXT(pContext)->bInit == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: CEOC module not initialized yet!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_SUCCESS;
   }

   if( DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun == DSL_FALSE )
   {
      DSL_DEBUG(DSL_DBG_WRN,
         (pContext, "DSL[%02d]: WARNING - CEOC module thread already stopped"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }
   else
   {
      /* Signal CEOC thread to stop*/
      DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun = DSL_FALSE;

      DSL_DRV_WAKEUP_EVENT(DSL_CEOC_CONTEXT(pContext)->ceocThread.waitEvent);
      DSL_DRV_WAIT_COMPLETION(&DSL_CEOC_CONTEXT(pContext)->ceocThread.Control);

      DSL_DEBUG(DSL_DBG_MSG,
         (pContext, "DSL[%02d]: CEOC thread has stopped... (%lu)"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));
   }

   /* Call device specific CEOC de-initialization stuff*/
   nErrCode = DSL_CEOC_DEV_Stop(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC module device deinitialization failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   /* Free CEOC Module resources*/
   if( DSL_CEOC_CONTEXT(pContext) != DSL_NULL )
   {
      DSL_DRV_MemFree( DSL_CEOC_CONTEXT(pContext) );
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: CEOC module has stopped... (%lu)"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), DSL_DRV_TimeMSecGet()));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_ceoc.h'
*/
DSL_Error_t DSL_CEOC_Restart(DSL_Context_t *pContext)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: Restarting CEOC module..."DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Check CEOC module bRun flag*/
   if( DSL_CEOC_CONTEXT(pContext)->ceocThread.bRun == DSL_FALSE )
   {
      DSL_DRV_INIT_EVENT("ceocev_ne", DSL_CEOC_CONTEXT(pContext)->ceocThread.waitEvent);

      /* Start CEOC module thread*/
      nErrCode = (DSL_Error_t)DSL_DRV_THREAD(&DSL_CEOC_CONTEXT(pContext)->ceocThread.Control,
                                 "ceocex_ne", DSL_CEOC_Thread, (DSL_uint32_t)pContext);

      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_MSG,
            (pContext, "DSL[%02d]: ERROR - CEOC module thread start failed, retCode(%d)!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), nErrCode));
      }
   }
   else
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC thread should be stoped before restarting!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   /* Call device specific CEOC restart stuff*/
   nErrCode = DSL_CEOC_DEV_Restart(pContext);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC device specific restart failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return nErrCode;
   }

   return nErrCode;
}


/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_ceoc.h'
*/
DSL_Error_t DSL_CEOC_MessageSend(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_LineStateValue_t nCurrentState = DSL_LINESTATE_UNKNOWN;

   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_MessageSend"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if( DSL_CEOC_CONTEXT(pContext) == DSL_NULL )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC module not started!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   if( !DSL_CEOC_CONTEXT(pContext)->bInit )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC module not initialized!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* Check message length*/
   if( pMsg->length > DSL_G997_SNMP_MESSAGE_LENGTH || pMsg->length == 0 )
   {
      DSL_DEBUG(DSL_DBG_ERR, (pContext, "DSL[%02d]: ERROR - EOC message length should "
         "be in range [1...%d]!" DSL_DRV_CRLF, DSL_DEV_NUM(pContext), 
         DSL_G997_SNMP_MESSAGE_LENGTH));
         
      return DSL_ERR_INVALID_PARAMETER;
   }

   /* Only proceed if the specified line is in SHOWTIME state*/
   DSL_CTX_READ_SCALAR(pContext, nErrCode, nLineState, nCurrentState);

   /* Lock CEOC common mutex*/
   if( DSL_DRV_MUTEX_LOCK( DSL_CEOC_CONTEXT(pContext)->ceocMutex ) )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - EOC common mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   if ((nCurrentState == DSL_LINESTATE_SHOWTIME_TC_SYNC) ||
       (nCurrentState == DSL_LINESTATE_SHOWTIME_NO_SYNC))
   {
      /* Call device specific stuff*/
      nErrCode = DSL_CEOC_DEV_MessageSend(pContext, protIdent, pMsg);
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
           (pContext, "DSL[%02d]: ERROR - EOC message send failed!"DSL_DRV_CRLF,
           DSL_DEV_NUM(pContext)));
      }
   }
   else
   {
      nErrCode = DSL_ERR_ONLY_AVAILABLE_IN_SHOWTIME;
      DSL_DEBUG(DSL_DBG_ERR,
        (pContext, "DSL[%02d]: ERROR - function only available in the SHOWTIME!"DSL_DRV_CRLF,
        DSL_DEV_NUM(pContext)));
   }

   /* Unlock CEOC common mutex*/
   DSL_DRV_MUTEX_UNLOCK( DSL_CEOC_CONTEXT(pContext)->ceocMutex );


   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_MessageSend, rerCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_CEOC_FifoSnmpMessageRead(
   DSL_OpenContext_t *pOpenCtx,
   DSL_Context_t *pContext,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t *pElmnt = DSL_NULL;

   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: IN - DSL_CEOC_FifoSnmpMessageRead" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (DSL_DRV_MUTEX_LOCK(pOpenCtx->rxSnmpFifoMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC mutex lock failed!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERR_SEMAPHORE_GET;
   }

   if ((pOpenCtx->rxSnmpFifo != DSL_NULL) &&
       (pOpenCtx->rxSnmpFifoBuf != DSL_NULL) &&
       ((pOpenCtx->nResourceActivationMask & DSL_RESOURCE_ACTIVATION_SNMP) == DSL_RESOURCE_ACTIVATION_CLEANED))
   {
      if (DSL_Fifo_isEmpty(pOpenCtx->rxSnmpFifo))
      {
         DSL_DEBUG( DSL_DBG_WRN,
            (pContext, "DSL[%02d]: WARNING - CEOC Rx SNMP fifo is empty!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_WRN_SNMP_NO_DATA;
      }
      else
      {
         pElmnt = DSL_Fifo_readElement(pOpenCtx->rxSnmpFifo);

         if( pElmnt == DSL_NULL )
         {
            DSL_DEBUG( DSL_DBG_ERR,
               (pContext, "DSL[%02d]: ERROR - received NULL pointer to the CEOC SNMP fifo!"DSL_DRV_CRLF,
               DSL_DEV_NUM(pContext)));
               
            nErrCode = DSL_ERROR;
         }
         else
         {
            /* copy an element */
            memcpy(pMsg, pElmnt, sizeof(DSL_G997_SnmpData_t));
         }
      }
   }
   else
   {
      nErrCode = DSL_WRN_SNMP_NO_DATA;
   }

   DSL_DRV_MUTEX_UNLOCK(pOpenCtx->rxSnmpFifoMutex);

   DSL_DEBUG(DSL_DBG_MSG, (pContext,
      "DSL[%02d]: OUT - DSL_CEOC_FifoSnmpMessageRead, retCode=%d" DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_CEOC_FifoSnmpMessageWrite(
   DSL_Context_t *pContext,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t *pElmnt = DSL_NULL;
   DSL_OpenContext_t *pCurr;

   DSL_CHECK_POINTER(pContext, pContext->pDevCtx);
   DSL_CHECK_ERR_CODE();
   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_FifoSnmpMessageWrite"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Lock Open Context list*/
   if (DSL_DRV_MUTEX_LOCK(pContext->pDevCtx->openContextListMutex))
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (DSL_NULL, "DSL[%02d]: ERROR - Couldn't lock Context List mutex!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
      
      return DSL_ERR_SEMAPHORE_GET;
   }

   for (pCurr = pOpenContextList; pCurr != DSL_NULL; pCurr = pCurr->pNext)
   {
      if (DSL_DRV_MUTEX_LOCK(pCurr->rxSnmpFifoMutex))
      {
         DSL_DEBUG( DSL_DBG_ERR,
            (DSL_NULL, "DSL[%02d]: ERROR - Couldn't lock Rx SNMP FIFO mutex!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
            
         nErrCode = DSL_ERR_SEMAPHORE_GET;
         
         break;
      }

      if ((pCurr->rxSnmpFifo != DSL_NULL) &&
          (pCurr->rxSnmpFifoBuf != DSL_NULL) &&
          ((pCurr->nResourceActivationMask & DSL_RESOURCE_ACTIVATION_SNMP) == DSL_RESOURCE_ACTIVATION_CLEANED))
      {
         /* Check if there is a free space in the SNMP FIFO*/
         if ( DSL_Fifo_isFull(pCurr->rxSnmpFifo) )
         {
            DSL_DEBUG( DSL_DBG_WRN,
              (pContext, "DSL[%02d]: WARNING - CEOC Rx SNMP fifo is full!"DSL_DRV_CRLF,
              DSL_DEV_NUM(pContext)));
         }
         else
         {
            if ((pElmnt = DSL_Fifo_writeElement(pCurr->rxSnmpFifo)) == DSL_NULL)
            {
               DSL_DEBUG( DSL_DBG_ERR,
                  (pContext, "DSL[%02d]: ERROR - received NULL pointer to the CEOC SNMP fifo!"DSL_DRV_CRLF,
                  DSL_DEV_NUM(pContext)));
                  
               nErrCode = DSL_ERROR;
            }
            else
            {
               /* copy an element */
               /*  DSL_G997_SnmpData_t and DSL_CEOC_Message_t structures are identical,
                    we can use any*/
               memcpy(pElmnt, pMsg, sizeof(DSL_G997_SnmpData_t));
            }
          }
      }

      DSL_DRV_MUTEX_UNLOCK(pCurr->rxSnmpFifoMutex);
   }

   /* Unlock Open Context list*/
   DSL_DRV_MUTEX_UNLOCK(pContext->pDevCtx->openContextListMutex);

   if (nErrCode == DSL_SUCCESS)
   {
      /* Generate DSL_EVENT_S_SNMP_MESSAGE_AVAILABLE event*/
      nErrCode = DSL_DRV_EventGenerate(
                    pContext, 0, DSL_ACCESSDIR_NA, DSL_XTUDIR_NA,
                    DSL_EVENT_S_SNMP_MESSAGE_AVAILABLE, DSL_NULL, 0);
 
      if( nErrCode != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: ERROR - DSL_EVENT_S_SNMP_MESSAGE_AVAILABLE "
            "event generation failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext)));
      }
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_FifoSnmpMessageWrite, rerCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'drv_dsl_cpe_intern_ceoc.h'
*/
DSL_Error_t DSL_CEOC_FifoMessageRead(
   DSL_OpenContext_t *pOpenCtx,
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_FifoMessageRead"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   switch( protIdent )
   {
   case DSL_CEOC_SNMP_PROTOCOL_ID:
      nErrCode = DSL_CEOC_FifoSnmpMessageRead(pOpenCtx, pContext, pMsg );
      break;

   default:
      nErrCode = DSL_WRN_EOC_UNSUPPORTED_PROTOCOLL_ID;
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: ERROR - Unsupported protocol identifier(0x%04X)!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), protIdent));
         
      break;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_FifoMessageRead, retCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

static DSL_Error_t DSL_CEOC_FifoMessageWrite(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: IN - DSL_CEOC_FifoMessageWrite"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   switch( protIdent )
   {
   case DSL_CEOC_SNMP_PROTOCOL_ID:
      nErrCode = DSL_CEOC_FifoSnmpMessageWrite( pContext, pMsg );
      break;

   default:
      nErrCode = DSL_WRN_EOC_UNSUPPORTED_PROTOCOLL_ID;
      DSL_DEBUG(DSL_DBG_WRN, (pContext,
         "DSL[%02d]: ERROR - Unsupported protocol identifier(0x%04X)!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), protIdent));
      break;
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, "DSL[%02d]: OUT - DSL_CEOC_FifoMessageWrite, rerCode=%d"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

#ifdef INCLUDE_DSL_CEOC_INTERNAL_API
static DSL_Error_t DSL_CEOC_CallbackHandle(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_uint8_t i = 0;

   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   if( DSL_CEOC_CONTEXT(pContext) == DSL_NULL )
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC module not started!"DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* Lock CEOC internal API mutex*/
   if( DSL_DRV_MUTEX_LOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex ) )
   {
      return DSL_ERROR;
   }

   for( i = 0; i < DSL_DRV_MAX_DEVICE_NUMBER; i++ )
   {
      if( !(DSL_CEOC_CONTEXT(pContext)->pCeocEventCallback[i]) )
         continue;

      if( DSL_CEOC_CONTEXT(pContext)->pCeocEventCallback[i](protIdent, pMsg) != DSL_SUCCESS )
      {
         DSL_DEBUG(DSL_DBG_ERR,
            (pContext, "DSL[%02d]: WRN - CEOC CallBack(%d) handle failed!"DSL_DRV_CRLF,
            DSL_DEV_NUM(pContext), i));
      }
   }

   /* Unlock CEOC internal API mutex*/
   DSL_DRV_MUTEX_UNLOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex );

   return nErrCode;
}
#endif

/* @} DRV_DSL_CPE_COMMON */

#endif /* INCLUDE_DSL_CEOC */
