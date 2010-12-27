/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DSL_CPE_DEBUG_H
#define _DSL_CPE_DEBUG_H

/** Defines the max. string length for group names (including termination '0') */
#define DSL_MAX_CMV_NAME_LENGTH     5

/** Defines the max. string length for group names (excluding termination '0') */
#define DSL_MAX_CMV_NAME_LENGTH_NO_TERM     4

/**
   Max level of debug printouts. Compiler will automatically omit all debug
   code during optimizations
*/
#ifndef DSL_CCA_DBG_MAX_LEVEL
#define DSL_CCA_DBG_MAX_LEVEL DSL_CCA_DBG_ERR
#endif

#define DSL_CCA_DBG_MAX_ENTRIES 9

#ifdef DSL_CCA_DBG_BLOCK
#undef DSL_CCA_DBG_BLOCK
#endif

#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_NO_BLOCK

/** Definitions for defining the debug level.
    The smaller the value the less debug output will be printed
*/
typedef enum
{
   /** no output */
   DSL_CCA_DBG_NONE = 0x00,
   /** */
   DSL_CCA_DBG_PRN = 0x01,
   /** errors will be printed */
   DSL_CCA_DBG_ERR = 0x02,
   /** warnings and errors are printed */
   DSL_CCA_DBG_WRN = 0x40,
   /** verbose output */
   DSL_CCA_DBG_MSG = 0x80,
   /** */
   DSL_CCA_DBG_LOCAL = 0xFF
} DSL_CCA_debugLevels_t;

/**
   Defines all available debug modules
*/
typedef enum
{
   /** no block selected */
   DSL_CCA_DBG_NO_BLOCK = 0,
   /** Application function block */
   DSL_CCA_DBG_APP = 1,
   /** Operating system block */
   DSL_CCA_DBG_OS = 2,
   /** CLI specific block */
   DSL_CCA_DBG_CLI = 3,
   /** Pipe specific block */
   DSL_CCA_DBG_PIPE = 4,
   /** SOAP specific block */
   DSL_CCA_DBG_SOAP = 5,
   /** Console specific block */
   DSL_CCA_DBG_CONSOLE = 6,
   /** TCP Message specific block */
   DSL_CCA_DBG_TCPMSG = 7,
   /** Last entry for debug blocks - only used as delimiter! */
   DSL_CCA_DBG_LAST_BLOCK = 8
} DSL_CCA_debugModules_t;

/**
   Structure that is used to build up a list of debug modules and its names.
   Structure that is used for printing out the names of available debug levels only.
*/
typedef struct
{
   /** Debug level according to debug level definition */
   DSL_CCA_debugLevels_t nDbgLvl;
   /** Name of debug level */
   DSL_char_t *pcName;
} DSL_CCA_debugLevelEntry_t;

#define  DSL_CCA_DBG_MAX_PRINT_CHAR  256

#undef DSL_CCA_DBG_PRINTF
/** debug outputs mapped to standard special DSL printf() function */
#define  DSL_CCA_DBG_PRINTF (void) DSL_CPE_debug_printf


/* Macro __FUNCTION__ defined in C99, use it if available, define empty macro if not. */

#if defined( __FUNCTION__)
/** debug print function */
#define DSL_CCA_DBG_PRINTF_FUNCTION  DSL_CCA_DBG_PRINTF(DSL_NULL, __FUNCTION__)
#else
/** debug print function */
#define DSL_CCA_DBG_PRINTF_FUNCTION  do {} while(0)
#endif

#ifdef DSL_CPE_DEBUG_DISABLE

  /*
   *  Kill debug defines for maximum speed and minimum code size.
   */

   /** Macro to print debug info contained in "body" with a prepended header that
       contains current file name, current function name (if available) and
       current source line no. */
   #define DSL_CCA_DEBUG_HDR(level, body)      ((void)0)
   /** Macro to just print debug body without header. */
   #define DSL_CCA_DEBUG(level, body)          ((void)0)
   /** Terminate execution if assertion fails */
   #define DSL_CCA_ASSERT(exp)                 ((void)0)

#else

/**
   Macro to print debug info contained in "body" with a prepended header that
   contains current file name, current function name (if available) and
   current source line no.
   This macro does not exist for dumping information to a external
   application using a registered callback. Thus it should be only used for
   critical errors that will be printed to the standard output (console)
   in any case. */
#define DSL_CCA_DEBUG_HDR(level, body) \
{ \
   /*lint -save -e568 -e685 -e774 -e506 */ \
   /* Warning 568 non-negative quantity is never less than zero */ \
   /* Warning 685 Relational operator '<=' always evaluates to 'true' */ \
   if ( (DSL_CCA_DBG_MAX_LEVEL >= level) && \
      ((((level) <= DSL_CCA_g_dbgLvl[DSL_CCA_DBG_BLOCK].nDbgLvl) && (DSL_CCA_g_globalDbgLvl == DSL_CCA_DBG_LOCAL)) \
      || (((level) <= DSL_CCA_g_globalDbgLvl) && (DSL_CCA_g_globalDbgLvl != DSL_CCA_DBG_LOCAL)))) \
   { \
      DSL_CCA_DBG_PRINTF(DSL_NULL, __FILE__":"); \
      DSL_CCA_DBG_PRINTF_FUNCTION; \
      DSL_CCA_DBG_PRINTF(DSL_NULL, ":Line %i:" DSL_CPE_CRLF,__LINE__); \
      DSL_CCA_DBG_PRINTF body; \
   } \
   /*lint -restore */ \
}

/**
   Macro to dump debug body if line is in specified range for debug output
   and if the debug level is set to an appropriate value.
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
#define DSL_CCA_DEBUG(level, body) \
{ \
   /*lint -save -e568 -e685 -e774 -e506 */ \
   /* Warning 568 non-negative quantity is never less than zero */ \
   /* Warning 685 Relational operator '<=' always evaluates to 'true' */ \
   if ( (DSL_CCA_DBG_MAX_LEVEL >= level) ) \
   { \
      if ( ((((level) <= DSL_CCA_g_dbgLvl[DSL_CCA_DBG_BLOCK].nDbgLvl) && (DSL_CCA_g_globalDbgLvl == DSL_CCA_DBG_LOCAL))) \
        || (((level) <= DSL_CCA_g_globalDbgLvl) && (DSL_CCA_g_globalDbgLvl != DSL_CCA_DBG_LOCAL)) ) \
      { \
         DSL_CCA_DBG_PRINTF body; \
      } \
   } \
   /*lint -restore */ \
}


/** Macro to get debug level for actual debug block */
#define DSL_CCA_DEBUG_LEVEL_GET   (DSL_CCA_g_dbgLvl[DSL_CCA_DBG_BLOCK].nDbgLvl)


/** Terminate execution if assertion fails */
#define DSL_CCA_ASSERT(exp) \
{ \
   if (!(exp)) \
   { \
      DSL_CCA_DBG_PRINTF(DSL_NULL, __FILE__":"); \
      DSL_CCA_DBG_PRINTF_FUNCTION; \
      DSL_CCA_DBG_PRINTF(DSL_NULL, ":Line %i:" DSL_CPE_CRLF,__LINE__); \
      DSL_CCA_DBG_PRINTF(DSL_NULL, "Assertion failed." DSL_CPE_CRLF ); \
      while(1) ; \
   } \
}

/* Import global variables from lib_dsl_debug.c */

extern DSL_CCA_debugLevelEntry_t DSL_CCA_g_dbgLvl[DSL_CCA_DBG_MAX_ENTRIES];
extern DSL_CCA_debugLevels_t DSL_CCA_g_globalDbgLvl;
#ifndef _lint
extern DSL_CCA_debugLevelEntry_t DSL_CCA_g_dbgLvlNames[];
extern const DSL_uint8_t DSL_CCA_g_dbgLvlNumber;
#endif /* _lint*/
#endif /* ifdef DSL_CPE_DEBUG_DISABLE */


/**
  A structure for event type<->string conversion tables.
*/
typedef struct
{
   /** group name string */
   DSL_char_t *psGroupName;
   /** group ID */
   DSL_int_t nGroupId;
} DSL_CmvGroupEntry_t;


DSL_Error_t DSL_CMV_Read
(
   DSL_CPE_Control_Context_t *pContext,
   DSL_char_t *str_group,
   DSL_uint16_t address,
   DSL_uint16_t index,
   DSL_int_t size,
   DSL_uint16_t *pData
);

DSL_Error_t DSL_CMV_Write
(
   DSL_CPE_Control_Context_t *pContext,
   DSL_char_t *str_group,
   DSL_uint16_t address,
   DSL_uint16_t index,
   DSL_int_t size,
   DSL_uint16_t *pData
);

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
void DSL_CMV_Prepare
(
   DSL_uint8_t opcode,
   DSL_uint8_t group,
   DSL_uint16_t address,
   DSL_uint16_t index,
   DSL_int_t size,
   DSL_uint16_t *data,
   DSL_uint16_t *Message
);
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #include "dsl_cpe_debug_danube.h"
#elif defined (INCLUDE_DSL_CPE_API_VINAX)
   #include "dsl_cpe_debug_vinax.h"
#else
   #error "xDSL chip not specified!"
#endif

/** TCP messages debug stuff, for Danube for the time present */

#ifdef DSL_DEBUG_TOOL_INTERFACE

/* Tcp debug messages stuff */

typedef struct {
   DSL_int_t fd;
   DSL_sockaddr_in_t client_addr;
   DSL_void_t *pDevData;
} DSL_CPE_TcpDebugClientInfo_t;

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
typedef struct {
   DSL_int_t fd;
   DSL_sockaddr_in_t client_addr;
   DSL_char_t *buf;
   DSL_char_t *pPos;
   DSL_CPE_File_t *out;
} DSL_CPE_TcpDebugCliClientInfo_t;
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

/**
   This structure is used to get resource usage statistics
   data
*/
typedef struct
{
   /**
   Total memory allocated statically (bytes) */
   DSL_uint32_t staticMemUsage;
   /**
   Total memory allocated dynamically (bytes) */
   DSL_uint32_t dynamicMemUsage;
} DSL_CPE_TcpDebugResourceUsageStatisticsData_t;

DSL_void_t DSL_CPE_DEV_DeviceDataFree(DSL_void_t *pDevData);

DSL_int_t DSL_CPE_DEV_TcpMessageHandle
(
   DSL_CPE_TcpDebugClientInfo_t *pDevData
);

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_DEV_TcpDebugMessageResourceUsageGet(
   DSL_CPE_TcpDebugClientInfo_t *clientInfo,
   DSL_uint32_t *pStaticMemUsage,
   DSL_uint32_t *pDynamicMemUsage);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_TcpDebugMessageResourceUsageGet (
   DSL_CPE_Control_Context_t * pContext,
   DSL_CPE_TcpDebugResourceUsageStatisticsData_t *pResUsage);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

DSL_Error_t DSL_CPE_TcpDebugMessageIntfStart (
   DSL_CPE_Control_Context_t * pContext);

DSL_Error_t DSL_CPE_TcpDebugCliIntfStart (
   DSL_CPE_Control_Context_t * pContext);

/** TCP messages server ip address */
extern DSL_char_t *sTcpMessagesSocketAddr;

/** TCP messages server port */
#define DSL_CPE_TCP_MESSAGES_PORT 2000

/** TCP CLI server port */
#define DSL_CPE_TCP_CLI_PORT 2001

/** TCP CLI command maximum length */
#define DSL_CPE_TCP_CLI_COMMAND_LENGTH_MAX 2048

#endif /* #ifdef DSL_DEBUG_TOOL_INTERFACE*/

#endif /* _DSL_CPE_DEBUG_H */

