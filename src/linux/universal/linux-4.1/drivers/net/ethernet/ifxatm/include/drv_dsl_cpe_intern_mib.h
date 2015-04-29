/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_INTERN_MIB_H
#define _DRV_DSL_CPE_INTERN_MIB_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \addtogroup DRV_DSL_CPE_ADSL_MIB
 @{ */

#if defined(INCLUDE_DSL_ADSL_MIB)
/**
   ADSL MIB thresholds types
*/
typedef enum DSL_MIB_ADSL_Thresholds
{
   DSL_MIB_TRAPS_NOTHING = 0x0,
   DSL_MIB_TRAPS_MASK_ADSL = 0x1FF,

   DSL_MIB_THRESHOLD_ATUC_LOFS_FLAG =  0x1,   /* BIT 0th position */
   DSL_MIB_THRESHOLD_ATUC_LOSS_FLAG =  0x2,   /* BIT 1 */
   DSL_MIB_THRESHOLD_ATUC_ESS_FLAG  =  0x4,   /* BIT 2 */
   DSL_MIB_THRESHOLD_ATUC_RATE_CHANGE_FLAG      =  0x8,   /* BIT 3 */
   DSL_MIB_THRESHOLD_ATUR_LOFS_FLAG =  0x10,  /* BIT 4 */
   DSL_MIB_THRESHOLD_ATUR_LOSS_FLAG =  0x20,  /* BIT 5 */
   DSL_MIB_THRESHOLD_ATUR_LPRS_FLAG =  0x40,  /* BIT 6 */
   DSL_MIB_THRESHOLD_ATUR_ESS_FLAG  =  0x80,  /* BIT 7 */
   DSL_MIB_THRESHOLD_ATUR_RATE_CHANGE_FLAG      =  0x100,  /* BIT 8 */

   DSL_MIB_TRAPS_MASK_ADSL_EXT = 0x3E00,

   DSL_MIB_THRESHOLD_ATUC_15MIN_FAILED_FASTR_FLAG = 0x200,  /* BIT 9 */
   DSL_MIB_THRESHOLD_ATUC_15MIN_SESL_FLAG         = 0x400,  /* BIT 10 */
   DSL_MIB_THRESHOLD_ATUC_15MIN_UASL_FLAG         = 0x800,  /* BIT 11 */
   DSL_MIB_THRESHOLD_ATUR_15MIN_SESL_FLAG         = 0x1000,  /* BIT 12 */
   DSL_MIB_THRESHOLD_ATUR_15MIN_UASL_FLAG         = 0x2000  /* BIT 13 */
} DSL_MIB_ADSL_Thresholds_t;

/** ADSL Mib context structure */
typedef struct DSL_MIB_ADSL_Context
{
   /** thresholds bit-field */
   DSL_MIB_ADSL_Thresholds_t nThresholds;
} DSL_MIB_ADSL_Context_t;

#endif

/**
   IO dispatcher routine for DSL CPE API MIB module

   \param   bIsInKernel    where from the call is performed
   \param   nCommand       the ioctl command.
   \param   nArg           The address of data.

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_ADSL_MIB
*/

#ifndef SWIG

DSL_Error_t DSL_DRV_MIB_IoctlHandle
(
   DSL_Context_t *pContext,
   DSL_boolean_t bIsInKernel,
   DSL_uint_t nCommand,
   DSL_uint32_t nArg
);

#endif

/**
   Initialization routine for MIB module

   \param pContext Pointer to dsl library context structure, [I]

   \return  Return values are defined within the \ref DSL_Error_t definition
    - DSL_SUCCESS in case of success
    - DSL_ERROR if operation failed
    - or any other defined specific error code

   \ingroup DRV_DSL_CPE_INIT
*/

#ifndef SWIG
DSL_Error_t DSL_DRV_MIB_ModuleInit
(
   DSL_Context_t *pContext
);
#endif

/** @} DRV_DSL_CPE_ADSL_MIB */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_INTERN_MIB_H */
