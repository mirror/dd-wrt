/******************************************************************************

                              Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

#ifndef _DSL_CPE_DTI_H_
#define _DSL_CPE_DTI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dsl_cpe_control.h"

#define DSL_CPE_DTI_DEFAULT_TCP_PORT   (9000)

/** \file
   DSL daemon command line interface for DTI control
*/

/**
   Start the Debug and Trace Agent

   \param      pContext                DSL CPE API context pointer (used for callback handling)
   \param      numOfPhyDevices         number of used physical devices.
   \param      numOfLinesPerPhyDevice  lines per physical device
   \param      dtiListenPort           DTI Agent listen port number.
   \param      pDtiServerIp            DTI Agent IP address.

   \return
   - DSL_ERROR On error
   - DSL_SUCCESS On success
*/
DSL_Error_t DSL_CPE_Dti_Start(
                        DSL_CPE_Control_Context_t *pContext,
                        DSL_int_t      numOfPhyDevices, 
                        DSL_int_t      numOfLinesPerPhyDevice,
                        DSL_uint16_t   dtiListenPort,
                        DSL_char_t     *pDtiServerIp,
                        DSL_boolean_t  bEnableCliAutoMsg,
                        DSL_boolean_t  bEnableDevAutoMsg,
                        DSL_boolean_t  bEnableSingleThreadMode);


/**
   Stop the Debug and Trace Agent

   \param      pContext                DSL CPE API context pointer (used for callback handling)

   \return
   - DSL_ERROR On error
   - DSL_SUCCESS On success
*/
DSL_Error_t DSL_CPE_Dti_Stop(
                        DSL_CPE_Control_Context_t *pContext);

#ifdef __cplusplus
}
#endif

#endif /* _DSL_CPE_DTI_H_ */

