/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_INTERN_CEOC_H
#define _DRV_DSL_CPE_INTERN_CEOC_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api_ceoc.h"

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

#define DSL_CEOC_RX_FIFO_ELEMENT_COUNT   (2)

#define DSL_CEOC_THREAD_POLLING_CYCLE   (1000)

/** Defines number of retries to send the EOC message*/
#define DSL_CEOC_MESSAGE_TX_RETRY_NUM   (10)

/** Defines time between EOC message send retries*/
#define DSL_CEOC_MESSAGE_TX_RETRY_PERIOD   (1000) /** msec*/

#define DSL_CEOC_TASK_NAME   "cpe_ceoc"

/** Helper macro for CEOC context retrieving from \ref DSL_Context_t structure. */
#define DSL_CEOC_CONTEXT(X)   ((DSL_CEOC_Context_t*)((X)->CEOC))

extern DSL_OpenContext_t *pOpenContextList;

typedef struct
{
   /** thread control structure */
   DSL_DRV_ThreadCtrl_t Control;
   /** CEOC module thread activity flag*/
   DSL_boolean_t bRun;
   /** CEOC module thread poll time*/
   DSL_uint32_t nThreadPollTime;
   /** CEOC module ...*/
   DSL_DRV_Event_t waitEvent;
} DSL_CEOC_Thread_t;

typedef struct
{
   /** CEOC module initialization flag*/
   DSL_boolean_t bInit;
   /** Common CEOC module mutex*/
   DSL_DRV_Mutex_t ceocMutex;
   /** CEOC module task*/
   DSL_CEOC_Thread_t ceocThread;
#ifdef INCLUDE_DSL_CEOC_INTERNAL_API
   /** Internal API CEOC module mutex*/
   DSL_DRV_Mutex_t ceocInternMutex;
   DSL_uint8_t openInstance;
   DSL_CEOC_Callback_t pCeocEventCallback[DSL_CEOC_MAX_OPEN_INSTANCE];
#endif /** #ifdef INCLUDE_DSL_CEOC_INTERNAL_API*/
   /** Device specific fields*/
#ifdef INCLUDE_DSL_CPE_API_VINAX
   /** VINAX CEOC handling uses the separate instance of the VINAX driver*/
   DSL_DEV_Handle_t lowHandle;
#endif /** #ifdef INCLUDE_DSL_CPE_API_VINAX*/
} DSL_CEOC_Context_t;


#ifndef SWIG
/**
   Initialization routine for CEOC module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
DSL_Error_t DSL_CEOC_Start(DSL_Context_t *pContext);

/**
   Deinitialization routine for CEOC module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
DSL_Error_t DSL_CEOC_Stop(DSL_Context_t *pContext);


/**
   Restart routine for CEOC module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
DSL_Error_t DSL_CEOC_Restart(DSL_Context_t *pContext);

DSL_Error_t DSL_CEOC_MessageSend(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg);

DSL_Error_t DSL_CEOC_FifoMessageRead(
   DSL_OpenContext_t *pOpenCtx,
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg);
#endif

/** @} DRV_DSL_CPE_COMMON */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_CEOC_H */
