/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_CEOC_INTERN_H
#define _DRV_DSL_CPE_API_CEOC_INTERN_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

#include "drv_dsl_cpe_api_types.h"
#include "drv_dsl_cpe_api_ceoc.h"


extern DSL_Error_t DSL_CEOC_InternalDevOpen(
                              DSL_uint16_t nDev,
                              DSL_uint32_t **ppCeocDynCntrl);

extern DSL_Error_t DSL_CEOC_InternalDevClose(
                              DSL_uint32_t *pCeocDynCntrl);

extern DSL_Error_t DSL_CEOC_InternalMessageSend(
                              DSL_uint32_t *pCeocDynCntrl,
                              DSL_uint16_t protIdent,
                              DSL_CEOC_Message_t *pMsg);

extern DSL_Error_t DSL_CEOC_InternalCbRegister(
                              DSL_uint32_t *pCeocDynCntrl,
                              DSL_CEOC_Callback_t pCallBackFunction);


/** @} DRV_DSL_CPE_COMMON */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_API_CEOC_INTERN_H */
