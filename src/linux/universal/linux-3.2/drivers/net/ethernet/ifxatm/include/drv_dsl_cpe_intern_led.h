/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_INTERN_LED_H
#define _DRV_DSL_CPE_INTERN_LED_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

/**
   Initialization routine for LED module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/

#ifndef SWIG

DSL_Error_t
DSL_DRV_LED_ModuleInit
(
   DSL_Context_t *pContext
);

#endif

/**
   LED module initialization routine before firmware start

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/

#ifndef SWIG

DSL_Error_t
DSL_DRV_LED_FirmwareInit
(
   DSL_Context_t *pContext
);

#endif

/** @} DRV_DSL_CPE_COMMON */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_LED_H */
