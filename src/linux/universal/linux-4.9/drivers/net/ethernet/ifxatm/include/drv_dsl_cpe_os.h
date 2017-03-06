/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_OS_H
#define _DRV_DSL_CPE_OS_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \addtogroup DRV_DSL_CPE_COMMON
 @{ */


#ifndef _lint
#ifdef __LINUX__
#ifdef __KERNEL__
#include "drv_dsl_cpe_os_linux.h"
#else
#error "This file could not be included from the user space application sources"
#endif
#elif defined(VXWORKS)
#include "drv_dsl_cpe_os_vxworks.h"
#elif defined(RTEMS)
#include "drv_dsl_cpe_os_rtems.h"
#elif defined(WIN32)
#include "drv_dsl_cpe_os_win32.h"
#else
#error "Please define OS"
#endif
#else
#include "drv_dsl_cpe_os_lint_map.h"
#endif /** #ifndef _lint*/


#define DSL_DRV_MemSet(Ptr, val, nSz)     memset(Ptr, val, nSz)

/**
   This function is a special implementation to realize both printing to the
   standard output (console) or writing the data to the callback function
   \ref DSL_MessageDump_Callback_t which will be done if a callback function
   was registered before including a appropriate configuration during
   registering.
   \attention For VxWorks there is no functionality to call a registered
   callback function instead of printing to the console.
   It will be printed to the console in any case (missing 'vsnprintf'
   in VxWorks)!

   \param pContext Pointer to dsl library context structure, [I]
   \param fmt      Pointer to format string (please refer to documentation of
                   'sprintf' function), [I]

   \return
   return the number of characters written, not including the terminating null
   character, or a negative value if an output error occurs.
*/
#ifndef SWIG

int DSL_DRV_debug_printf(DSL_Context_t const *pContext, DSL_char_t const *fmt, ...);

#endif /* SWIG */

/**
   This function converts DSL CPE API error code to the appropriate error code
   typical for the OS.

   \param nError   DSL CPE API error code, [I]

   \return
   return the appropriate OS error code for the passed with nError parameter
   DSL CPE API error code.
*/
#ifndef SWIG

int DSL_DRV_ErrorToOS(DSL_Error_t nError);

#endif /* SWIG */

/**
   This function copies input memory block from user/kernel space to kernel space.

   \param bIsInKernel whether the memory block is already in the kernel space, [I]
   \param pDest       Destination address, [I]
   \param pSrc        Source address, [I]
   \param nSize       Memory block size, [I]

   \return
   pointer equal to pDest
*/
#ifndef SWIG

DSL_void_t* DSL_IoctlMemCpyFrom
(
   DSL_boolean_t bIsInKernel,
   DSL_void_t    *pDest,
   DSL_void_t    *pSrc,
   DSL_DRV_size_t    nSize
);

#endif /* SWIG */

/**
   This function copies output memory block to user/kernel space from kernel space.

   \param bIsInKernel whether the memory block should be copied to kernel space, [I]
   \param pDest       Destination address, [I]
   \param pSrc        Source address, [I]
   \param nSize       Memory block size, [I]

   \return
   pointer equal to pDest
*/
#ifndef SWIG

DSL_void_t* DSL_IoctlMemCpyTo
(
   DSL_boolean_t bIsInKernel,
   DSL_void_t    *pDest,
   DSL_void_t    *pSrc,
   DSL_DRV_size_t    nSize
);

#endif /* SWIG */

/**
   This function allocates a memory, resulting block may be non-continuous
   in the physical memory.

   \param nSize       Memory block size, [I]

   \return
   pointer to allocated memory block, DSL_NULL in case of error
*/
#ifndef SWIG

DSL_void_t* DSL_DRV_VMalloc
(
   DSL_DRV_size_t    nSize
);

#endif /* SWIG */

/**
   This function releases a virtual memory.

   \param pPtr       Memory block to release. [I]
*/
#ifndef SWIG

DSL_void_t DSL_DRV_VFree
(
   DSL_void_t*    pPtr
);

#endif /* SWIG */

/**
   This function implements snprintf OS-specific analog for DSL CPE API.

   \param pPtr       Memory block to release. [I]
*/
#ifndef SWIG

DSL_int_t DSL_DRV_snprintf
(
   DSL_char_t        *pStr,
   DSL_DRV_size_t     nStrSz,
   const DSL_char_t  *pFormat,  ...
);

#endif /* SWIG */

/**
   This function implements sscanf OS-specific analog for DSL CPE API.

   \param pPtr       Memory block to release. [I]
*/
#ifndef SWIG

DSL_int_t DSL_sscanf( DSL_char_t *buf, DSL_char_t const *fmt, ...);

#endif /* SWIG */

/** @} DRV_DSL_CPE_COMMON */

#ifdef __cplusplus
}
#endif

#endif
