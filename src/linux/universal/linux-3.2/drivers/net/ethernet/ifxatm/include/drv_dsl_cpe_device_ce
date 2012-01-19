/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_DEVICE_CEOC_H
#define _DRV_DSL_CPE_DEVICE_CEOC_H

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef SWIG

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */


/*
   CEOC module firmware initialization routine

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/
DSL_Error_t DSL_DRV_DANUBE_CEOC_FirmwareInit(DSL_Context_t *pContext);

/**
   This function initializes CEOC module device specific parameters

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_CEOC_DEV_Start(DSL_Context_t *pContext);

/**
   This function de-initializes CEOC module device specific parameters

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_CEOC_DEV_Stop(DSL_Context_t *pContext);

/**
   This function re-initializes CEOC module device specific parameters

   \param pContext   Pointer to dsl library context structure, [I/O]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS (0) in case of success
   - DSL_ERROR (-1) if operation failed
   - or any other defined specific error code
*/
DSL_Error_t DSL_CEOC_DEV_Restart(DSL_Context_t *pContext);

DSL_Error_t DSL_CEOC_DEV_MessageSend(
   DSL_Context_t *pContext,
   DSL_uint16_t protIdent,
   DSL_CEOC_Message_t *pMsg);

DSL_Error_t DSL_CEOC_DEV_MessageReceive(
   DSL_Context_t *pContext,
   DSL_uint16_t *protIdent,
   DSL_CEOC_Message_t *pMsg);

/** @} DRV_DSL_CPE_COMMON */

#endif /** #ifndef SWIG*/

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_CEOC_H */
