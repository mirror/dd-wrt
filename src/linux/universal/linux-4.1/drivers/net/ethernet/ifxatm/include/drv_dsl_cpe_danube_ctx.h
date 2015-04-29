/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef DRV_DSL_CPE_DANUBE_CTX_H
#define DRV_DSL_CPE_DANUBE_CTX_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   DANUBE internal interface
*/

/** \addtogroup DRV_DSL_DEVICE
 @{ */

/**
   Device configuration structure
*/
struct DSL_DeviceConfig
{
   DSL_SystemInterfaceConfigData_t sysCIF;
};

typedef struct 
{
   DSL_uint8_t dummy;
} DSL_DeviceLowLevelConfig_t;

/** @} DRV_DSL_DEVICE */

#ifdef __cplusplus
}
#endif

#endif /* DRV_DSL_CPE_DANUBE_CTX_H */
