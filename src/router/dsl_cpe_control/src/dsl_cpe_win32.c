/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Operating System access implementation
*/

/*#define DSL_INTERN*/

#include "dsl_cpe_os.h"

#if defined(WIN32)

#include <time.h>
#include <process.h>

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_OS

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_Error_t DSL_SetEnv(const DSL_char_t *sName, const DSL_char_t *sValue)
{
   return ((SetEnvironmentVariable(sName, sValue) == 0) ? DSL_ERROR : DSL_SUCCESS);
}

/**
   Installing of system handler routines.

   \remarks not used
*/
DSL_void_t DSL_HandlerInstall(DSL_void_t)
{
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
int DSL_CPE_debug_printf(DSL_char_t *fmt, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nRet = 0;

   va_start(ap, fmt);   /* set ap pointer to 1st unnamed arg */

   /* For VxWorks printout will be only done to console only at the moment */
   nRet = vprintf(fmt, ap);

   va_end(ap);

   return nRet;
}

DSL_uint16_t DSL_CPE_Htons(DSL_uint16_t hVal)
{
   return htons(hVal);
}

DSL_uint32_t DSL_CPE_Htonl(DSL_uint32_t hVal)
{
   return htonl(hVal);
}

#ifdef DSL_DEBUG_TOOL_INTERFACE
DSL_char_t * DSL_CPE_OwnAddrStringGet(DSL_void_t)
{
   return DSL_NULL;
}
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_CPE_FPrintf(DSL_CPE_File_t *stream, const DSL_char_t *format, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nRet = 0;

   va_start(ap, format);   /* set ap pointer to 1st unnamed arg */

   nRet = vfprintf(stream, format, ap);
   fflush(stream);

   va_end(ap);

   return nRet;
}


#endif /* WIN32 */
