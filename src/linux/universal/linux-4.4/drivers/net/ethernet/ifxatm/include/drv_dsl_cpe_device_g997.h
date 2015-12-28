/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_DEVICE_G997_H
#define _DRV_DSL_CPE_DEVICE_G997_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \addtogroup DRV_DSL_DEVICE
 @{ */

#if defined(INCLUDE_DSL_CPE_API_DANUBE) || defined(INCLUDE_DSL_G997_PER_TONE)
/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_BitAllocationNSCGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_BitAllocationNsc_t *pData
);
#endif /* defined(INCLUDE_DSL_CPE_API_DANUBE) || defined(INCLUDE_DSL_G997_PER_TONE)*/

#ifdef INCLUDE_DSL_G997_PER_TONE
/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_GainAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_GainAllocationNsc_t *pData
);

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_SnrAllocationNscGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_SnrAllocationNsc_t *pData
);
#endif /* INCLUDE_DSL_G997_PER_TONE*/

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_LineStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatus_t *pData
);

#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
DSL_Error_t DSL_DRV_DEV_G997_LineStatusPerBandGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineStatusPerBand_t *pData
);
#endif /* (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1) */

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DEV_G997_LineInventoryGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventory_t *pData
);
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_Error_t DSL_DRV_DEV_G997_LineInventorySet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineInventoryNe_t *pData);
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

#ifdef INCLUDE_DSL_G997_STATUS
/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_LineTransmissionStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LineTransmissionStatus_t *pData);
#endif /* INCLUDE_DSL_G997_STATUS*/

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_ChannelStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_ChannelStatus_t *pData
);

#ifdef INCLUDE_DSL_G997_STATUS
/*
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_LastStateTransmittedGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN_OUT DSL_G997_LastStateTransmitted_t *pData);
#endif /* INCLUDE_DSL_G997_STATUS*/

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
#ifdef INCLUDE_DSL_G997_FRAMING_PARAMETERS
DSL_Error_t DSL_DRV_DEV_G997_FramingParameterStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN const DSL_uint8_t nChannel,
   DSL_OUT DSL_G997_FramingParameterStatusData_t *pData
);
#endif /* INCLUDE_DSL_G997_FRAMING_PARAMETERS*/

#ifdef INCLUDE_DSL_DELT
/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltHlinScaleGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltHlinScaleData_t *pData);

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltHlinGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltHlinData_t *pData);

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltHlogGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltHlogData_t *pData);

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltQLNGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltQlnData_t *pData);

/**
   For a detailed description please refer to the drv_dsl_cpe_intern_g997.h
*/
DSL_Error_t DSL_DRV_DEV_G997_DeltSNRGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN const DSL_AccessDir_t nDirection,
   DSL_IN DSL_DeltDataType_t nDeltDataType,
   DSL_OUT DSL_G997_DeltSnrData_t *pData);
#endif /* INCLUDE_DSL_DELT*/

DSL_Error_t DSL_DRV_DEV_G997_PowerManagementStateForcedTrigger(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_G997_PowerManagementStateForcedTriggerData_t *pData);

/** @} DRV_DSL_DEVICE */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_DEVICE_G997_H */
