/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#if defined(INCLUDE_DSL_CEOC_INTERNAL_API)

#include "drv_dsl_cpe_intern_ceoc.h"

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_CEOC

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

DSL_Error_t DSL_CEOC_InternalDevOpen(
   DSL_uint16_t nDev,
   DSL_uint32_t **ppCeocDynCntrl)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_devCtx_t *pDevCtx = DSL_NULL;
   DSL_Context_t *pContext = DSL_NULL;
   DSL_CEOC_InternalDynCtrl_t *pDynCtrl = DSL_NULL;

   if( nDev >= DSL_DRV_MAX_DEVICE_NUMBER )
   {
      return DSL_ERROR;
   }

   /* Get Device context*/
   pDevCtx  = &ifxDevices[nDev];
   /* Get DSL CPE context pointer*/
   pContext = pDevCtx->pContext;

   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   if(*ppCeocDynCntrl != DSL_NULL)
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - expecting zero context pointer '*ppCeocDynCntrl'!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   pDynCtrl = DSL_DRV_Malloc(sizeof(DSL_CEOC_InternalDynCtrl_t));

   if( pDynCtrl == DSL_NULL )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - can't allocate memory for '*ppCeocDynCntrl'!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   /* reset allocated memory structure*/
   memset((DSL_void_t*)pDynCtrl, 0, sizeof(DSL_CEOC_InternalDynCtrl_t));

   /* Lock CEOC internal API mutex*/
   if( DSL_DRV_MUTEX_LOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex ) )
   {
      return DSL_ERROR;
   }

   /* Update open instance count*/
   pDynCtrl->currOpenInstance = DSL_CEOC_CONTEXT(pContext)->openInstance++;
   pDynCtrl->pContext         = pContext;

   /* Unlock CEOC internal API mutex*/
   DSL_DRV_MUTEX_UNLOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex );

   *ppCeocDynCntrl = (DSL_uint32_t*)pDynCtrl;

   return nErrCode;
}

DSL_Error_t DSL_CEOC_InternalDevClose(
   DSL_uint32_t *pCeocDynCntrl)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Context_t *pContext = DSL_NULL;
   DSL_CEOC_InternalDynCtrl_t *pDynCtrl = (DSL_CEOC_InternalDynCtrl_t*)pCeocDynCntrl;

   if( pCeocDynCntrl == DSL_NULL )
   {
      return DSL_ERROR;
   }

   pContext = pDynCtrl->pContext;
   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   /* Lock CEOC internal API mutex*/
   if( DSL_DRV_MUTEX_LOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex ) )
   {
      return DSL_ERROR;
   }

   if (pDynCtrl->currOpenInstance > DSL_CEOC_CONTEXT(pContext)->openInstance )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Attempt to close not existing device(%d)!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pDynCtrl->currOpenInstance));

      return DSL_ERROR;   
   }

   /* Clear CEOC CallBack*/
   if( DSL_CEOC_CONTEXT(pContext)->pCeocEventCallback[pDynCtrl->currOpenInstance] )
   {
      DSL_CEOC_CONTEXT(pContext)->pCeocEventCallback[pDynCtrl->currOpenInstance] = DSL_NULL;
   }

   DSL_CEOC_CONTEXT(pContext)->openInstance--;

   /* Unlock CEOC internal API mutex*/
   DSL_DRV_MUTEX_UNLOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex );

   DSL_DRV_MemFree(pCeocDynCntrl);

   return nErrCode;
}

DSL_Error_t DSL_CEOC_InternalMessageSend(
   DSL_uint32_t *pCeocDynCntrl,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Context_t *pContext;
   DSL_CEOC_InternalDynCtrl_t *pDynCtrl = (DSL_CEOC_InternalDynCtrl_t*)pCeocDynCntrl;

   if( pDynCtrl == DSL_NULL )
   {
      return DSL_ERROR;
   }

   pContext = pDynCtrl->pContext;
   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_POINTER(pContext, pMsg);
   DSL_CHECK_ERR_CODE();

   /* Send CEOC message*/
   nErrCode = DSL_CEOC_MessageSend(pContext, protIdent,pMsg);
   if( nErrCode != DSL_SUCCESS )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - CEOC message send failed!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
   }

   return nErrCode;
}

DSL_Error_t DSL_CEOC_InternalCbRegister(
   DSL_uint32_t *pCeocDynCntrl,
   DSL_CEOC_Callback_t pCallBackFunction)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_Context_t *pContext = DSL_NULL;
   DSL_CEOC_InternalDynCtrl_t *pDynCtrl = (DSL_CEOC_InternalDynCtrl_t*)pCeocDynCntrl;

   if( pDynCtrl == DSL_NULL )
   {
      return DSL_ERROR;
   }

   pContext = pDynCtrl->pContext;
   DSL_CHECK_POINTER(pContext, DSL_CEOC_CONTEXT(pContext));
   DSL_CHECK_ERR_CODE();

   if( pCallBackFunction == DSL_NULL )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - can't register CEOC callback, callback pointer is NULL!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      return DSL_ERROR;
   }

   if (pDynCtrl->currOpenInstance > DSL_CEOC_CONTEXT(pContext)->openInstance )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - Attempt to close not existing device(%d)!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext), pDynCtrl->currOpenInstance));

      return DSL_ERROR;   
   }

   /* Lock CEOC internal API mutex*/
   if( DSL_DRV_MUTEX_LOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex ) )
   {
      return DSL_ERROR;
   }

   /* Register CEOC callback*/
   if( DSL_CEOC_CONTEXT(pContext)->pCeocEventCallback[pDynCtrl->currOpenInstance] )
   {
      DSL_DEBUG( DSL_DBG_ERR,
         (pContext, "DSL[%02d]: ERROR - can't register CEOC callback, elready registered!" DSL_DRV_CRLF,
         DSL_DEV_NUM(pContext)));
         
      nErrCode = DSL_ERROR;
   }
   else
   {
      DSL_CEOC_CONTEXT(pContext)->pCeocEventCallback[pDynCtrl->currOpenInstance] = pCallBackFunction;
   }

   /* Unlock CEOC internal API mutex*/
   DSL_DRV_MUTEX_UNLOCK( DSL_CEOC_CONTEXT(pContext)->ceocInternMutex );

   return nErrCode;
}

/** @} DRV_DSL_CPE_COMMON */

#endif /* INCLUDE_DSL_CEOC_INTERNAL_API */
