/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef VXWORKS

#include "dsl_cpe_vxworks.h"
#include "dsl_cpe_control.h"
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
   DSL_int_t nRet = 0;

   va_start(ap, fmt);   /* set ap pointer to 1st unnamed arg */

   /* For VxWorks printout will be only done to console only at the moment */
   nRet = vprintf(fmt, ap);

   va_end(ap);

   return nRet;
}

/**
   Enable the console line mode.
   In this mode the input from the device is available only after receiving NEWLINE . 
   This allows to modify the command line until the Enter key is pressed.
*/   
void DSL_CPE_KeypressSet (void)
{
#ifndef _lint
   int options;
   int iofd = fileno(stdin);

   options = ioctl(iofd, FIOGETOPTIONS, 0);
   ioctl(iofd, FIOSETOPTIONS, (int)(options & ~OPT_LINE));
#endif   
}

/**
   Disable the console line mode. 
   Plesae refer to \ref IFXOS_KeypressSet .
*/   
void DSL_CPE_KeypressReset (void)
{
#ifndef _lint
   int options;
   int iofd = fileno(stdin);

   options = ioctl(iofd, FIOGETOPTIONS, 0);
   ioctl(iofd, FIOSETOPTIONS, (int)(options | OPT_LINE));
#endif   
}


DSL_uint16_t DSL_CPE_Htons(DSL_uint16_t hVal)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
   return (((hVal >> 8) & 0xFF) | (hVal << 8));
#else
   return hVal;
#endif
}

DSL_uint32_t DSL_CPE_Htonl(DSL_uint32_t hVal)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN

   return (((hVal >> 24) & 0x000000ff) | ((hVal >> 8) & 0x0000ff00) | ((hVal << 8) & 0x00ff0000) | ((hVal << 24) & 0xff000000));
#else
   return hVal;
#endif
}

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

#ifdef DSL_DEBUG_TOOL_INTERFACE
DSL_char_t * DSL_CPE_OwnAddrStringGet(DSL_void_t)
{
   return DSL_NULL;
}
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#endif /* VXWORKS */

