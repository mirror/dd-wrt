/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_DEBUG_H
#define _DRV_DSL_CPE_DEBUG_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   Debug support
*/

/** \addtogroup DRV_DSL_CPE_DEBUG
 @{ */

/**
   Max level of debug printouts. Compiler will automatically omit all debug
   code during optimizations
*/
#ifndef DSL_DBG_MAX_LEVEL
#define DSL_DBG_MAX_LEVEL DSL_DBGLVL_ERR
#endif

#define DSL_DBG_MAX_ENTRIES 16

#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_NO_BLOCK

/**
   Structure that is used to build up a list of debug modules and its names.
   Structure that is used for printing out the names of available debug levels only.
*/
typedef struct
{
   /** Debug level according to debug level definition */
   DSL_debugLevels_t nDbgLvl;
   /** Name of debug level */
   DSL_char_t *pcName;
} DSL_debugLevelEntry_t;

/*$$
#if defined(VXWORKS) || defined(LINUX)
  #include "stdio.h"
#endif
*/

#define  DSL_DBG_MAX_DEBUG_PRINT_CHAR  256

#undef DSL_DBG_PRINTF
/** debug outputs mapped to standard special DSL printf() function */
#define  DSL_DBG_PRINTF (void) DSL_DRV_debug_printf


/* Macro __FUNCTION__ defined in C99, use it if available, define empty macro if not. */

#if defined( __FUNCTION__)
/** debug print function */
#define DSL_DBG_PRINTF_FUNCTION  DSL_DBG_PRINTF(DSL_NULL, __FUNCTION__)
#else
/** debug print function */
#define DSL_DBG_PRINTF_FUNCTION  do {} while(0)
#endif

#ifndef SWIG
#ifndef DSL_DEBUG_DISABLE
/**
  Set the nErrNo value of the context.
*/
DSL_void_t DSL_DRV_ErrorSet(DSL_void_t *pContext, DSL_Error_t code);
#endif /* DSL_DEBUG_DISABLE*/
#endif /* SWIG */


#ifdef DSL_DEBUG_DISABLE
   /**
       Set the nErrno value of the context.
   */
   #define DSL_DEBUG_SET_ERROR(code) ((void)0)

  /*
   *  Kill debug defines for maximum speed.
   */

   /** Macro to print debug info contained in "body" with a prepended header that
       contains current file name, current function name (if available) and
       current source line no. */
   #define DSL_DEBUG_HDR(level, body)      ((void)0)
   /** Macro to just print debug body without header if line is in specified range */
   #define DSL_DEBUG(level, body)   ((void)0)
   /** Terminate execution if assertion fails */
   #define DSL_ASSERT(exp)                 ((void)0)

#else

   #define DSL_DEBUG_SET_ERROR(code) DSL_DRV_ErrorSet(pContext, code);

/**
   Macro to print debug info contained in "body" with a prepended header that
   contains current file name, current function name (if available) and
   current source line no.
   This macro does not exist for dumping information to a external
   application using a registered callback. Thus it should be only used for
   critical errors that will be printed to the standard output (console)
   in any case. */
#define DSL_DEBUG_HDR(level, body) \
{ \
   /*lint -save -e568 -e685 -e774 */ \
   /* Warning 568 non-negative quantity is never less than zero */ \
   /* Warning 685 Relational operator '<=' always evaluates to 'true' */ \
   if ( (DSL_DBG_MAX_LEVEL >= level) && \
      ((((level) <= DSL_g_dbgLvl[DSL_DBG_BLOCK].nDbgLvl) && (DSL_g_globalDbgLvl == DSL_DBG_LOCAL)) \
      || (((level) <= DSL_g_globalDbgLvl) && (DSL_g_globalDbgLvl != DSL_DBG_LOCAL)))) \
   { \
      DSL_DBG_PRINTF(DSL_NULL, __FILE__":"); \
      DSL_DBG_PRINTF_FUNCTION; \
      DSL_DBG_PRINTF(DSL_NULL, ":Line %i:" DSL_DRV_CRLF,__LINE__); \
      DSL_DBG_PRINTF body; \
   } \
}

/**
   Macro to dump debug body if the debug level is set to an appropriate value.
   The handling of the debug output is related to the former registration of
   a callback function as follows:

   Callback function is registered
      The debug string will be passed to the upper layer software using the
      callback itself in case of appropriate debug level.

   NO callback function is registered
      The debug string will be printed to the standard output (console) in
      case of appropriate debug level.

   \attention The implementation of this macro needs a valid context pointer
              from the DSL API. Thus you shall use this macro AFTER complete
              initialization of the DSL API only!
*/
#define DSL_DEBUG(level, body) \
{ \
   /*lint -save -e568 -e685 -e774 */ \
   /* Warning 568 non-negative quantity is never less than zero */ \
   /* Warning 685 Relational operator '<=' always evaluates to 'true' */ \
   if ( (DSL_DBG_MAX_LEVEL >= level) && \
        (((((level) <= DSL_g_dbgLvl[DSL_DBG_BLOCK].nDbgLvl) && (DSL_g_globalDbgLvl == DSL_DBG_LOCAL))) \
     || (((level) <= DSL_g_globalDbgLvl) && (DSL_g_globalDbgLvl != DSL_DBG_LOCAL))) ) \
   { \
      DSL_DBG_PRINTF body; \
   } \
}

/** Macro to get debug level for actual debug block */
#define DSL_DEBUG_LEVEL_GET   (DSL_g_dbgLvl[DSL_DBG_BLOCK].nDbgLvl)


/** Terminate execution if assertion fails */
#define DSL_ASSERT(exp) \
{ \
   if (!(exp)) \
   { \
      DSL_DBG_PRINTF(DSL_NULL, __FILE__":"); \
      DSL_DBG_PRINTF_FUNCTION; \
      DSL_DBG_PRINTF(DSL_NULL, ":Line %i:" DSL_DRV_CRLF,__LINE__); \
      DSL_DBG_PRINTF(DSL_NULL, "Assertion failed." DSL_DRV_CRLF ); \
      while(1) ; \
   } \
}

#ifndef DSL_DEBUG_DISABLE
DSL_char_t* DSL_DBG_IoctlName(DSL_uint_t nIoctlCode);
#endif /* DSL_DEBUG_DISABLE*/

/* Import global variables from lib_dsl_debug.c */

extern DSL_debugLevelEntry_t DSL_g_dbgLvl[DSL_DBG_MAX_ENTRIES];
extern DSL_debugLevels_t DSL_g_globalDbgLvl;
extern DSL_uint16_t DSL_g_dbgStartLine;
extern DSL_uint16_t DSL_g_dbgStopLine;
extern DSL_debugLevelEntry_t DSL_g_dbgLvlNames[];
extern const DSL_uint8_t DSL_g_dbgLvlNumber;
extern const DSL_char_t* DSL_DBG_PRN_DIR[];
extern const DSL_char_t* DSL_DBG_PRN_AUTOBOOT_STATUS[];
extern const DSL_char_t* DSL_DBG_PRN_AUTOBOOT_STATE[];


#endif /* ifdef DSL_DEBUG_DISABLE */


/** @} DRV_DSL_CPE_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* _DRV_DSL_CPE_DEBUG_H */
