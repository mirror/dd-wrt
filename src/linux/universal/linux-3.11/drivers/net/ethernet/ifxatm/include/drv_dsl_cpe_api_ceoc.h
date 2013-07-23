/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_API_CEOC_H
#define _DRV_DSL_CPE_API_CEOC_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */

/**
   TODO: Move this definition to the configuration process
*/
/*#define INCLUDE_DSL_CEOC_INTERNAL_API*/

/** SNMP protocol identifier*/
#define DSL_CEOC_SNMP_PROTOCOL_ID   (0x814C)

/**
   CEOC message
*/
typedef struct
{
   /** Length of the Clear EOC message in Bytes. Min=1, Max=508 bytes*/
   DSL_uint16_t length;
   /** Clear EOC data*/
   DSL_uint8_t data[508];
} DSL_CEOC_Message_t;


#ifdef INCLUDE_DSL_CEOC_INTERNAL_API

#define DSL_CEOC_MAX_OPEN_INSTANCE   (10)

typedef DSL_Error_t (*DSL_CEOC_Callback_t) (DSL_uint16_t protIdent, DSL_CEOC_Message_t *pMsg);

typedef struct
{
   DSL_uint8_t currOpenInstance;
   DSL_Context_t *pContext;
} DSL_CEOC_InternalDynCtrl_t;

#endif /** #ifdef INCLUDE_DSL_CEOC_INTERNAL_API*/

/** @} DRV_DSL_CPE_COMMON */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_API_CEOC_H */
