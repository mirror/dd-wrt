/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef RTEMS

#include "dsl_cpe_rtems.h"
#include "dsl_cpe_control.h"
#include "dsl_cpe_os.h"
#include "dsl_cpe_simulator.h"

/** \file
   Operating System access implementation
*/

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
int DSL_CPE_debug_printf(DSL_char_t *fmt, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   DSL_int_t nLength = 0, nRet = 0;
   DSL_boolean_t bPrint = DSL_FALSE;
   DSL_char_t msg[DSL_DBG_MAX_DEBUG_PRINT_CHAR + 1] = "\0";

   va_start(ap, fmt);   /* set ap pointer to 1st unnamed arg */

   if (bPrint == DSL_FALSE)
   {
      nRet = vsprintf(msg, fmt, ap);

      if (nRet < 0)
      {
         nLength = DSL_DBG_MAX_DEBUG_PRINT_CHAR;
         printf("DSL CPE API: WARNING - printout truncated in 'DSL_CPE_debug_printf'!" DSL_DRV_CRLF );
      }
      else
      {
         nLength = nRet;
      }

      nRet = printf(msg);
   }

   va_end(ap);

   return nRet;
}

/**
   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dsl_cpe_os.h'
*/
DSL_int_t DSL_Ioctl(const DSL_int_t fd, const DSL_uint32_t cmd, DSL_int_t param)
{
   return DSL_DRV_Ioctls(0,cmd, param);
}

#endif /* RTEMS */


