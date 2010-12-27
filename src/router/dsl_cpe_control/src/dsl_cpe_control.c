/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
Includes
*/
#include "dsl_cpe_control.h"
#include "dsl_cpe_cli.h"
#include "dsl_cpe_cli_console.h"
#include "dsl_cpe_debug.h"
#include "drv_dsl_cpe_api_ioctl.h"
#include "dsl_cpe_simulator.h"

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
#include "drv_dsl_cpe_cmv_danube.h"
#endif

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
#include "dsl_cpe_dti.h"
#endif

#ifdef INCLUDE_DSL_BONDING
#include "dsl_cpe_bnd.h"
#endif /* INCLUDE_DSL_BONDING*/

#define DSL_CPE_STATIC static

#include "dsl_cpe_init_cfg.h"

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_APP

#ifdef LINUX
struct termios stored_stdout_settings;
struct termios stored_stdin_settings;
#endif /* LINUX */

#ifdef RTEMS
#ifdef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
#include "AmazonSE_310801.h"
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/
DSL_CPE_ThreadCtrl DslMainControl;
static DSL_int_t errno = 0;
#endif /*RTEMS*/

DSL_boolean_t bScriptError = DSL_FALSE;

/* the console only works in forground mode, so no daemonize ! */
#ifndef INCLUDE_DSL_API_CONSOLE

#if defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
DSL_char_t *g_sRemoteTcpServerIp = DSL_NULL;
#endif /* defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/

/* 'daemon()' function is not working correctly therefore this feature has been
   disabled here.
   You might use the '&' operator to start the dsl_cpe_control application
   within background */
#undef USE_DAEMONIZE
#if 0
#ifdef __UCLIBC__
#define UCLIBC_VER ((__UCLIBC_MAJOR__*10000) + (__UCLIBC_MINOR__*100) + __UCLIBC_SUBLEVEL__)
#if (UCLIBC_VER >= 926)
      /* Older versions of uClibc than our tested version (0.9.26) seams to have
         problems with the daemon function */
#define USE_DAEMONIZE   1
#endif
#else
   /* assume that other libraries will work! */
/*#define USE_DAEMONIZE   1*/
#endif /* __UCLIBC__ */
#endif

#endif /* INCLUDE_DSL_API_CONSOLE */

#ifdef INCLUDE_DSL_CPE_API_VINAX
DSL_CPE_STATIC  DSL_char_t *sLowLevCfgName = DSL_NULL;
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_CPE_STATIC  const DSL_CPE_EVT_CodeString_t eventString[] =
{
   {DSL_EVENT_I_LINE_FAILURES, "DSL_EVENT_I_LINE_FAILURES"},
   {DSL_EVENT_I_DATA_PATH_FAILURES, "DSL_EVENT_I_DATA_PATH_FAILURES"},
   {DSL_EVENT_I_LINE_THRESHOLD_CROSSING, "DSL_EVENT_I_LINE_THRESHOLD_CROSSING"},
   {DSL_EVENT_I_CHANNEL_THRESHOLD_CROSSING, "DSL_EVENT_I_CHANNEL_THRESHOLD_CROSSING"},
   {DSL_EVENT_I_DATA_PATH_THRESHOLD_CROSSING, "DSL_EVENT_I_DATA_PATH_THRESHOLD_CROSSING"},
   {DSL_EVENT_I_RETX_THRESHOLD_CROSSING, "DSL_EVENT_I_RETX_THRESHOLD_CROSSING"},
   {DSL_EVENT_I_CHANNEL_DATARATE_SHIFT_THRESHOLD_CROSSING, "DSL_EVENT_I_CHANNEL_DATARATE_SHIFT_THRESHOLD_CROSSING"},
   {DSL_EVENT_S_LINIT_FAILURE, "DSL_EVENT_S_LINIT_FAILURE"},
   {DSL_EVENT_S_LINE_STATE, "DSL_EVENT_S_LINE_STATE"},
   {DSL_EVENT_S_LINE_POWERMANAGEMENT_STATE, "DSL_EVENT_S_LINE_POWERMANAGEMENT_STATE"},
   {DSL_EVENT_S_CHANNEL_DATARATE, "DSL_EVENT_S_CHANNEL_DATARATE"},
   {DSL_EVENT_S_FIRMWARE_ERROR, "DSL_EVENT_S_FIRMWARE_ERROR"},
   {DSL_EVENT_S_INIT_READY, "DSL_EVENT_S_INIT_READY"},
   {DSL_EVENT_S_FE_INVENTORY_AVAILABLE, "DSL_EVENT_S_FE_INVENTORY_AVAILABLE"},
   {DSL_EVENT_S_SYSTEM_STATUS, "DSL_EVENT_S_SYSTEM_STATUS"},
   {DSL_EVENT_S_PM_SYNC, "DSL_EVENT_S_PM_SYNC"},
   {DSL_EVENT_S_LINE_TRANSMISSION_STATUS, "DSL_EVENT_S_LINE_TRANSMISSION_STATUS"},
   {DSL_EVENT_S_SHOWTIME_LOGGING, "DSL_EVENT_S_SHOWTIME_LOGGING"},
   {DSL_EVENT_S_FIRMWARE_REQUEST, "DSL_EVENT_S_FIRMWARE_REQUEST"},
   {DSL_EVENT_S_FIRMWARE_DOWNLOAD_STATUS, "DSL_EVENT_S_FIRMWARE_DOWNLOAD_STATUS"},
   {DSL_EVENT_S_AUTOBOOT_STATUS, "DSL_EVENT_S_AUTOBOOT_STATUS"},
   {DSL_EVENT_S_SNMP_MESSAGE_AVAILABLE, "DSL_EVENT_S_SNMP_MESSAGE_AVAILABLE"},
   {DSL_EVENT_S_SYSTEM_INTERFACE_STATUS, "DSL_EVENT_S_SYSTEM_INTERFACE_STATUS"},
   {DSL_EVENT_LAST, DSL_NULL}
};
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

/*
local prototypes
*/
DSL_CPE_STATIC  DSL_void_t DSL_CPE_ArgParse (
   DSL_int32_t argc,
   DSL_char_t * argv[]
);

DSL_CPE_STATIC  DSL_void_t DSL_CPE_Help (
   DSL_char_t * sApplicationName
);

DSL_CPE_STATIC  DSL_void_t DSL_CPE_Termination(void);


DSL_CPE_STATIC  DSL_CPE_Control_Context_t *gDSLContext = DSL_NULL;

#ifndef DSL_CPE_REMOVE_PIPE_SUPPORT
extern DSL_Error_t DSL_CPE_Pipe_Init (DSL_CPE_Control_Context_t * pContext);
#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
extern DSL_Error_t DSL_CPE_Pipe_StaticResourceUsageGet(DSL_uint32_t *pStatResUsage);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/
#endif

DSL_char_t *g_sFirmwareName1 = DSL_NULL;
DSL_char_t *g_sFirmwareName2 = DSL_NULL;
#ifdef INCLUDE_SCRIPT_NOTIFICATION
DSL_char_t *g_sRcScript = DSL_NULL;
DSL_CPE_STATIC  DSL_boolean_t bScriptWarn = DSL_FALSE;
#endif

DSL_boolean_t g_bWaitBeforeLinkActivation[DSL_CPE_MAX_DEVICE_NUMBER];
DSL_boolean_t g_bWaitBeforeConfigWrite[DSL_CPE_MAX_DEVICE_NUMBER];
DSL_boolean_t g_bWaitBeforeRestart[DSL_CPE_MAX_DEVICE_NUMBER];

static DSL_LineStateValue_t g_nPrevLineState[DSL_CPE_MAX_DEVICE_NUMBER];

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
#  ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
DSL_char_t *g_sAdslScript = DSL_NULL;
DSL_char_t *g_sVdslScript = DSL_NULL;
#  else
#include "dsl_cpe_autoboot_script_adsl.h"
DSL_char_t *g_sAdslScript = g_sAdslScript_static;
#ifdef INCLUDE_DSL_CPE_API_VINAX
#include "dsl_cpe_autoboot_script_vdsl.h"
DSL_char_t *g_sVdslScript = g_sVdslScript_static;
#else
DSL_char_t *g_sVdslScript = DSL_NULL;
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#   endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/
DSL_boolean_t g_bAutoContinueWaitBeforeLinkActivation[DSL_CPE_MAX_DEVICE_NUMBER];
DSL_boolean_t g_bAutoContinueWaitBeforeConfigWrite[DSL_CPE_MAX_DEVICE_NUMBER];
DSL_boolean_t g_bAutoContinueWaitBeforeRestart[DSL_CPE_MAX_DEVICE_NUMBER];
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT*/

/*
local variables
*/
DSL_CPE_STATIC  DSL_int32_t bHelp = -1;
DSL_CPE_STATIC  DSL_int32_t bGetVersion = -1;
DSL_CPE_STATIC  DSL_int32_t bInit = -1;
DSL_CPE_STATIC  DSL_int32_t bXtuOctets = -1;

/* Events and resource handling is enabled for all available types by default! */
DSL_CPE_STATIC  DSL_boolean_t bEventActivation = DSL_TRUE;
DSL_CPE_STATIC  DSL_uint32_t nResourceActivationMask = 0x00000000;

#if defined (INCLUDE_DSL_CPE_API_DANUBE) && !defined(RTEMS)
   DSL_int32_t bOptimize = 1;
#else
   DSL_int32_t bOptimize = -1;
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/

#if defined(RTEMS)
DSL_CPE_STATIC  DSL_int32_t bMsgDump = 1;
#else
DSL_CPE_STATIC  DSL_int32_t bMsgDump = -1;
#endif /* defined(RTEMS)*/

#ifdef DSL_DEBUG_TOOL_INTERFACE
DSL_CPE_STATIC  DSL_int32_t bTcpMessageIntf = -1;
#endif

#ifdef INCLUDE_DSL_CPE_DTI_SUPPORT
DSL_CPE_STATIC  DSL_int32_t bDTI = -1;
#endif

#ifdef USE_DAEMONIZE
DSL_CPE_STATIC  DSL_int_t bNotSilent = 1;
#endif /* USE_DAEMONIZE */

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_CPE_STATIC  DSL_int32_t bConsole = -1;
DSL_CPE_Console_Context_t *pConsoleContext = DSL_NULL;
#endif

DSL_CPE_STATIC  DSL_int_t g_bFirmware1 = -1;
DSL_CPE_STATIC  DSL_int_t g_bFirmware2 = -1;

DSL_CPE_STATIC  DSL_G997_XTUSystemEnablingData_t g_nXtseInit;
DSL_CPE_STATIC  DSL_uint8_t g_nMsgDumpDbgLvl;

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
typedef struct
{
   /** reference to registered CLI */
   DSL_CLI_Context_t *pCLIContext;
   /** running status of pipe task */
   volatile DSL_boolean_t bRun;
} dummy_console_t;

DSL_CPE_STATIC  dummy_console_t dummy_console;

DSL_CPE_STATIC  DSL_Error_t DSL_CPE_Control_Exit (DSL_void_t * pContext);
#endif

#ifdef LINUX
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "/opt/ifx/firmware/ModemHWE.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  ""
#elif defined(INCLUDE_DSL_CPE_API_VINAX)
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "/opt/ifx/firmware/vcpe_hw.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  "/opt/ifx/firmware/acpe_hw.bin"
#else
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "firmware0.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  "firmware1.bin"
#endif
#elif VXWORKS
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "modemhwe.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  ""
#elif defined(INCLUDE_DSL_CPE_API_VINAX)
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "vcpe_hw.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  "acpe_hw.bin"
#else
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "firmware0.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  "firmware1.bin"
#endif
#elif RTEMS
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "cgi_pFileData_modemfw_bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  ""
#elif WIN32
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "..\\firmware\\modemhwe.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  ""
#elif defined(INCLUDE_DSL_CPE_API_VINAX)
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "..\\firmware\\vcpe_hw.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  "..\\firmware\\acpe_hw.bin"
#else
   #define DSL_CPE_DEFAULT_FIRMWARE_1  "firmware0.bin"
   #define DSL_CPE_DEFAULT_FIRMWARE_2  "firmware1.bin"
#endif
#else
   #define DSL_CPE_DEFAULT_FIRMWARE_1  ""
   #define DSL_CPE_DEFAULT_FIRMWARE_2  ""
#endif

const DSL_char_t *sDefaultFirmwareName1 = (DSL_char_t *)DSL_CPE_DEFAULT_FIRMWARE_1;
const DSL_char_t *sDefaultFirmwareName2 = (DSL_char_t *)DSL_CPE_DEFAULT_FIRMWARE_2;

#ifdef INCLUDE_SCRIPT_NOTIFICATION
   #if defined (INCLUDE_DSL_CPE_API_DANUBE)
      #define DSL_CPE_DEFAULT_RC_SCRIPT "./adslrc.sh"
   #elif defined(INCLUDE_DSL_CPE_API_VINAX)
      #define DSL_CPE_DEFAULT_RC_SCRIPT "./vdslrc.sh"
   #else
      #define DSL_CPE_DEFAULT_RC_SCRIPT "./xdslrc.sh"
   #endif
   const DSL_char_t *sDefaultRcScript = DSL_CPE_DEFAULT_RC_SCRIPT;
#endif

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
DSL_char_t *sSoapRemoteServer = DSL_NULL;
#endif

#ifdef DSL_DEBUG_TOOL_INTERFACE
DSL_char_t *sTcpMessagesSocketAddr = DSL_NULL;
#endif

#ifdef INCLUDE_DSL_CPE_DTI_SUPPORT
DSL_char_t *sDtiSocketAddr = DSL_NULL;
#endif

/*DSL_InitData_t gInitCfgData;*/


DSL_CPE_STATIC  struct option long_options[] = {
   {"help      ", 0, 0, 'h'},
   {"version   ", 0, 0, 'v'},
   {"init      ", 1, 0, 'i'},
#ifdef INCLUDE_DSL_CPE_API_VINAX
   #if (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
   {"backward  ", 0, 0, 'b'},
   #endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/
   {"low_cfg   ", 1, 0, 'l'},
#endif
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   {"console   ", 0, 0, 'c'},
#endif
   {"event_cnf ", 1, 0, 'e'},
   {"msg_dump  ", 1, 0, 'm'},
#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   {"auto_scr_1", 1, 0, 'a'},
#ifdef INCLUDE_DSL_CPE_API_VINAX
   {"auto_scr_2", 1, 0, 'A'},
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#endif
#endif
#ifdef USE_DAEMONIZE
   {"silent    ", 0, 0, 'q'},
#endif /* USE_DAEMONIZE */
   {"firmware1 ", 1, 0, 'f'},
#ifdef INCLUDE_DSL_CPE_API_VINAX
   {"firmware2 ", 1, 0, 'F'},
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   {"opt_off   ", 1, 0, 'o'},
#endif
#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   {"soap      ", 1, 0, 's'},
#endif
#ifdef INCLUDE_SCRIPT_NOTIFICATION
   {"notif     ", 1, 0, 'n'},
#endif
#if defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
   {"remoteTcp ", 1, 0, 'r'},
#endif /* defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/
#ifdef DSL_DEBUG_TOOL_INTERFACE
   {"tcpmsg    ", 1, 0, 't'},
#endif
#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
   {"dti       ", 1, 0, 'd'},
#endif /* defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/
   {0, 0, 0, 0}
};

/* 1 colon means there is a required parameter */
/* 2 colons means there is an optional parameter */
DSL_CPE_STATIC  const DSL_char_t GETOPT_LONG_OPTSTRING[] = "hvi::"
#ifdef INCLUDE_DSL_CPE_API_VINAX
   #if (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
   "b"
   #endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/
   "l:"
#endif
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   "c"
#endif
   "e:m::"
#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   "a:"
#ifdef INCLUDE_DSL_CPE_API_VINAX
   "A:"
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#endif
#endif
#ifdef USE_DAEMONIZE
   "q"
#endif
   "f:"
#ifdef INCLUDE_DSL_CPE_API_VINAX
   "F:"
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   "o"
#endif
#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   "s::"
#endif
#ifdef INCLUDE_SCRIPT_NOTIFICATION
   "n:"
#endif
#if defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
   "r:"
#endif /* defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/
#ifdef DSL_DEBUG_TOOL_INTERFACE
   "t::"
#endif
#ifdef INCLUDE_DSL_CPE_DTI_SUPPORT
   "d::"
#endif
   ;

/*lint -save -e786 */ \
DSL_CPE_STATIC  DSL_char_t description[][90] = {
   {"help screen"},
   {"display version"},
   {"init device w/ <xtu> Bits seperated by underscore (e.g. -i05_01_04_00_04_01_00_00)"},
#ifdef INCLUDE_DSL_CPE_API_VINAX
   #if (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
   {"CLI backward compatible mode"},
   #endif /*(DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
   {"low level configuration file"},
#endif
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   {"start console"},
#endif
   {"configure instance activation handling <enable/disable>[_mask] (e.g. -e1_1)"},
   {"enable message dump"},
#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   {"autoboot start script for ADSL (empty by default)"},
#ifdef INCLUDE_DSL_CPE_API_VINAX
   {"autoboot start script for VDSL (empty by default)"},
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#endif
#endif
#ifdef USE_DAEMONIZE
   "silent mode, no output from background",
#endif
   {"firmware file, default " DSL_CPE_DEFAULT_FIRMWARE_1},
#ifdef INCLUDE_DSL_CPE_API_VINAX
   {"2nd firmware file, default " DSL_CPE_DEFAULT_FIRMWARE_2},
#endif /* INCLUDE_DSL_CPE_API_VINAX*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   {"deactivate footprint optimizations"},
#endif
#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   {"SOAP server"},
#endif
#ifdef INCLUDE_SCRIPT_NOTIFICATION
   {"notification script name, default " DSL_CPE_DEFAULT_RC_SCRIPT},
#endif
#if defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
   {"remote TCP debug server IP address"},
#endif /* defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/
#ifdef DSL_DEBUG_TOOL_INTERFACE
   {"enable dbgtool support on tcp ports 2000/2001, listen only on <ipaddr> (optional)"},
#endif
#ifdef INCLUDE_DSL_CPE_DTI_SUPPORT
   {"enable DTI support on default tcp port 9000, listen only on <ipaddr> (optional)"},
#endif
   {0}
};

/*lint -restore */

#ifndef DSL_CPE_DEBUG_DISABLE
DSL_CPE_STATIC  const DSL_char_t *dsl_cpe_ctl_version = "@(#)" PACKAGE_VERSION;
#endif

DSL_CPE_Control_Context_t *DSL_CPE_GetGlobalContext (
   DSL_void_t
)
{
   if (gDSLContext == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR, requesting for global context, which is zero" DSL_CPE_CRLF));
   }

   return gDSLContext;
}

void DSL_CPE_Echo (
   DSL_char_t *buf)
{
   DSL_char_t *str;
   DSL_char_t cmd[64];

   if (buf == DSL_NULL)
   {
      return;
   }

   /* remove leading whitespaces */
   for (str = buf; str && *str && isspace((int)(*str)); ++str)
   {
      ;
   }

   sscanf (str, "%s", cmd);
   if (strcmp (cmd, "echo") == 0)
   {
      str += 4;
   }
   if (str[0] == ' ') str++;

   DSL_CPE_FPrintf (DSL_CPE_STDOUT, "%s \n", str);

   return;
}

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
/* Use this to format arrays of unspecified contents as hex values */
DSL_void_t DSL_CPE_ArraySPrintF(
   DSL_char_t *pDst,
   DSL_void_t *pSrc,
   DSL_uint16_t nSrcSize,
   DSL_uint16_t nSrcElementSize,
   DSL_CPE_ArrayPrintFormat_t nFormat)
{
   DSL_uint16_t i = 0, j = 0, elements = 0;
   DSL_uint32_t nVal = 0;
   DSL_char_t c;
   DSL_int_t ret = 0;

   elements = nSrcSize/nSrcElementSize;

   if (nFormat == DSL_ARRAY_FORMAT_HEX)
   {
      ret = sprintf(pDst, "(");
      pDst++;
   }

   for (i = 0; i < elements; i++)
   {
      if ((i != 0) && (nFormat == DSL_ARRAY_FORMAT_HEX))
      {
         ret = sprintf(pDst, ",");
         pDst ++;
      }

      if (nFormat == DSL_ARRAY_FORMAT_HEX)
      {
         switch (nSrcElementSize)
         {
         case 1:
            ret = sprintf(pDst, "%02X", ((DSL_uint8_t*)pSrc)[i]);
            break;
         case 2:
            ret = sprintf(pDst, "%04X", ((DSL_uint16_t*)pSrc)[i]);
            break;
         case 4:
            ret = sprintf(pDst, "%08X", ((DSL_uint32_t*)pSrc)[i]);
            break;
         default:
            ret = sprintf(pDst, "xx");
            break;
         }

         if(ret > 0)
         {
            pDst += ret;
         }
      }
      else
      {
         switch (nSrcElementSize)
         {
         case 1:
            nVal = (DSL_uint32_t)(((DSL_uint8_t*)pSrc)[i]);
            break;
         case 2:
            nVal = (DSL_uint32_t)(((DSL_uint16_t*)pSrc)[i]);
            break;
         case 4:
            nVal = (DSL_uint32_t)(((DSL_uint32_t*)pSrc)[i]);
            break;
         default:
            nVal = 0;
            break;
         }

         for (j = 0; j < nSrcElementSize; j++)
         {
            c = (DSL_char_t)(nVal >> j);
            if (isprint((int)c) != 0)
            {
               ret = sprintf(pDst, "%c", c);
               pDst += ret;
            }
         }
      }
   }

   if (nFormat == DSL_ARRAY_FORMAT_HEX)
   {
      ret = sprintf(pDst, ")");
   }
}

/**
   Parses the given character string for given token and sets the resulting
   pointer to the given offset of tokens within the given string.

   \param DSL_char_t *pCommands
      Specifies a string that shall be parsed
   \param DSL_int_t nParamNr
      Specifies the number of tokens that shall be found and of which the
      pointer has to be incresed.
   \param DSL_char_t *pSeps
      Specifies the separators to be srearched for
   \param DSL_char_t *pCmdOffset
      Returns a pointer to the command string including token offset if found
*/
DSL_Error_t DSL_CPE_MoveCharPtr(
   DSL_char_t *pCommands,
   DSL_int_t nParamNr,
   DSL_char_t *pSeps,
   DSL_char_t **pCmdOffset)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_int_t i = 0;
   DSL_char_t *pTmp = DSL_NULL;

   pTmp = pCommands;

   for (i = 0; i < nParamNr; i++)
   {
      if (pTmp == DSL_NULL) break;

      pTmp = strpbrk(pTmp, pSeps);
      if (pTmp != DSL_NULL)
      {
         pTmp++;
      }
      else
      {
         break;
      }
   }

   if (i < nParamNr)
   {
      nRet = DSL_ERROR;
      *pCmdOffset = pCommands;
   }
   else
   {
      nRet = DSL_SUCCESS;
      *pCmdOffset = pTmp;
   }

   return nRet;
}


#ifdef INCLUDE_DSL_CPE_API_VINAX
DSL_Error_t DSL_CPE_GetMacAdrFromString(
   DSL_char_t *pString,
   DSL_CPE_MacAddress_t *pMacAdr)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_char_t string[20] = { 0 };
   DSL_char_t sMacAddr[3] = {0};
   DSL_char_t seps[] = ":";
   DSL_char_t *token;
   DSL_int_t i = 0;

   strncpy (string, pString, sizeof(string)-1);
   string[sizeof(string)-1] = 0;

   /* Get first token */
   token = strtok (string, seps);
   if (token != DSL_NULL)
   {
      for (i = 0; i < DSL_MAC_ADDRESS_OCTETS; i++)
      {
         DSL_CPE_sscanf (token, "%s", (unsigned char *)sMacAddr);

         DSL_CPE_sscanf (token, "%bx", (unsigned char *)&(pMacAdr->nAdr[i]));

         if ( ((strcmp (&sMacAddr[1], "0") != 0) && ( (pMacAdr->nAdr[i] & 0xF) == 0)) )
         {
            i=0;
            break;
         }

         sMacAddr[1] = '\0';
         if ( ((strcmp (&sMacAddr[0], "0") != 0) && ( ((pMacAdr->nAdr[i] >> 4) & 0xF) == 0)) )
         {
            i=0;
            break;
         }

         /* Get next token */
         token = strtok(DSL_NULL, seps);

         /* Exit scanning if no further information is included */
         if (token == DSL_NULL)
         {
            break;
         }
      }
   }

   if (i < (DSL_MAC_ADDRESS_OCTETS - 1))
   {
      nRet = DSL_ERROR;
   }
   else
   {
      nRet = DSL_SUCCESS;
   }

   return nRet;
}
#endif /* INCLUDE_DSL_CPE_API_VINAX */
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

/**
   Parse all arguments and enable requested features.

   \param argc - number of parameters
   \param argv - array of parameter strings
*/
DSL_CPE_STATIC  DSL_void_t DSL_CPE_ArgParse (
   DSL_int32_t argc,
   DSL_char_t * argv[]
)
{
   DSL_char_t *pEndPtr = "\0";

   DSL_char_t string[30] = { 0 };
   DSL_int_t i = 0;
   DSL_uint32_t nVal = 0;
   DSL_char_t seps[]   = "_";
   DSL_char_t *token, *errMask;
   DSL_int_t option_index = 1;

   while (1)
   {
      DSL_int_t c;

      /* 1 colon means there is a required parameter */
      /* 2 colons means there is an optional parameter */
      c = getopt_long (argc, argv, GETOPT_LONG_OPTSTRING, long_options,
         &option_index);

      if (c == -1)
      {
         break;
      }

      switch (c)
      {
      case 0:
         /* optarg found */
         break;

      case 'h':
         bHelp = 1;
         break;

      case 'v':
         bGetVersion = 1;
         break;

#if defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
      case 'r':
         g_sRemoteTcpServerIp = DSL_CPE_Malloc(strlen (optarg) + 1);
         if (g_sRemoteTcpServerIp)
         {
            strcpy (g_sRemoteTcpServerIp, optarg);

            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "using %s as the remote TCP debug server IP address" DSL_CPE_CRLF,
               optarg));
         }
         break;
#endif /* defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/

#ifdef DSL_DEBUG_TOOL_INTERFACE
      case 't':
         bTcpMessageIntf = 1;
         if (optarg != NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "using %s as an IP address for tcp messages debug server" DSL_CPE_CRLF,
               optarg));

            sTcpMessagesSocketAddr = DSL_CPE_Malloc(strlen (optarg) + 1);
            if (sTcpMessagesSocketAddr)
            {
               strcpy (sTcpMessagesSocketAddr, optarg);
            }
         }
         else
         {
            sTcpMessagesSocketAddr = DSL_CPE_OwnAddrStringGet();
            if (sTcpMessagesSocketAddr == DSL_NULL)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Own IP address get failed!" DSL_CPE_CRLF));
            }
            else
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "using %s as an IP address for tcp messages debug server" DSL_CPE_CRLF,
                  sTcpMessagesSocketAddr));
            }
         }
         break;
#endif

#ifdef INCLUDE_DSL_CPE_DTI_SUPPORT
      case 'd':
         bDTI = 1;
         if (optarg != NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "using %s as an IP address for DTI Agent" DSL_CPE_CRLF,
               optarg));

            sDtiSocketAddr = DSL_CPE_Malloc(strlen (optarg) + 1);
            if (sDtiSocketAddr)
            {
               strcpy (sDtiSocketAddr, optarg);
            }
         }
         else
         {
            sDtiSocketAddr = DSL_CPE_OwnAddrStringGet();
            if (sDtiSocketAddr == DSL_NULL)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Own IP address get failed!" DSL_CPE_CRLF));
            }
            else
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "using %s as an IP address for DTI Agent" DSL_CPE_CRLF,
                  sDtiSocketAddr));
            }
         }
         break;
#endif /* INCLUDE_DSL_CPE_DTI_SUPPORT*/

#ifdef USE_DAEMONIZE
      case 'q':
         bNotSilent = 0;
         break;
#endif /* USE_DAEMONIZE */

      case 'i':
         bInit = 1;
         memset (&g_nXtseInit.XTSE, 0, sizeof(g_nXtseInit.XTSE));

         if (optarg != DSL_NULL)
         {
            strncpy (string, optarg, sizeof(string)-1);
            string[sizeof(string)-1]=0;

            token = strtok (string, seps);
            if (token != DSL_NULL)
            {
               for (i = 0; i < DSL_G997_NUM_XTSE_OCTETS; i++)
               {
                  DSL_CPE_sscanf (token, "%x", &nVal);
                  g_nXtseInit.XTSE[i] = (DSL_uint8_t) nVal;

                  /* Get next token */
                  token = strtok(DSL_NULL, seps);

                  /* Exit scanning if no further information is included */
                  if (token == DSL_NULL)
                  {
                     break;
                  }
               }
            }
            if (i > 0) bXtuOctets = 1;
         }
         break;

#ifdef INCLUDE_DSL_CPE_API_VINAX
      #if (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
      case 'b':
         {
            DSL_CPE_Control_Context_t *pCtrlCtx = DSL_NULL;
            pCtrlCtx = DSL_CPE_GetGlobalContext();
            if (pCtrlCtx != DSL_NULL)
            {
               pCtrlCtx->bBackwardCompMode = DSL_TRUE;
            }
         }
         break;
      #endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/

      case 'l':
          if (sLowLevCfgName)
          {
             DSL_CPE_Free(sLowLevCfgName);
          }
          sLowLevCfgName = DSL_CPE_Malloc (strlen (optarg) + 1);
          strcpy (sLowLevCfgName, optarg);
          break;
#endif

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
      case 'c':
         bConsole = 1;
         break;
#endif

#ifdef INCLUDE_SCRIPT_NOTIFICATION
      case 'n':
         if (g_sRcScript)
         {
            DSL_CPE_Free(g_sRcScript);
         }

         if (optarg != NULL)
         {
            g_sRcScript = DSL_CPE_Malloc (strlen (optarg) + 1);
            if (g_sRcScript)
            {
               strcpy (g_sRcScript, optarg);
            }
         }
         else
         {
            g_sRcScript = DSL_CPE_Malloc (strlen (sDefaultRcScript) + 1);
            if (g_sRcScript)
            {
               strcpy (g_sRcScript, sDefaultRcScript);
            }
         }
         break;
#endif /* #ifdef INCLUDE_SCRIPT_NOTIFICATION */

      case 'f':
         g_bFirmware1 = 1;
         if (g_sFirmwareName1)
         {
            DSL_CPE_Free(g_sFirmwareName1);
         }
         g_sFirmwareName1 = DSL_CPE_Malloc (strlen (optarg) + 1);
         if (g_sFirmwareName1)
         {
            strcpy (g_sFirmwareName1, optarg);
         }
         break;

      case 'F':
         g_bFirmware2 = 1;
         if (g_sFirmwareName2)
         {
            DSL_CPE_Free(g_sFirmwareName2);
         }
         g_sFirmwareName2 = DSL_CPE_Malloc (strlen (optarg) + 1);
         if (g_sFirmwareName2)
         {
            strcpy (g_sFirmwareName2, optarg);
         }
         break;

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
      case 'o':
         bOptimize = -1;
         break;
#endif

      case 'e':
         if (optarg != NULL)
         {
            strncpy (string, optarg, sizeof(string)-1);
            string[sizeof(string)-1]=0;
            token = strtok (string, seps);

            if (token != DSL_NULL)
            {
               /* Get events enable/disable flag*/
               DSL_CPE_sscanf (token, "%d", &nVal);
               bEventActivation = nVal ? DSL_TRUE : DSL_FALSE;

               /* Get next token, event mask */
               token = strtok(DSL_NULL, seps);
               if (token != DSL_NULL)
               {
                  nResourceActivationMask = (DSL_uint32_t)strtoul (token, &errMask, 16);

                  if (nResourceActivationMask == ULONG_MAX && errno != 0)
                  {
                     DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                        "invalid resource mask %s specified, all resources will "
                        "be disabled" DSL_CPE_CRLF, optarg));
                     nResourceActivationMask = 0xFFFFFFFF;
                  }
               }
            }

            if (bEventActivation == DSL_FALSE)
            {
               /* Mask (disable) all resources */
               nResourceActivationMask = 0xFFFFFFFF;
            }
         }
         break;

      case 'm':
         bMsgDump = 1;
         if (optarg != DSL_NULL)
         {
            g_nMsgDumpDbgLvl = (DSL_uint8_t)(strtoul (optarg, &pEndPtr, 0));
         }
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Using message dump (debug level: 0x%02X)" DSL_CPE_CRLF, g_nMsgDumpDbgLvl));
         break;

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
      case 'a':
         if (g_sAdslScript)
         {
            DSL_CPE_Free(g_sAdslScript);
            g_sAdslScript = DSL_NULL;
         }
         g_sAdslScript = DSL_CPE_Malloc (strlen (optarg) + 1);
         if (g_sAdslScript)
         {
            strcpy (g_sAdslScript, optarg);
         }
         break;

      case 'A':

         if (g_sVdslScript)
         {
            DSL_CPE_Free(g_sVdslScript);
            g_sVdslScript = DSL_NULL;
         }
         g_sVdslScript = DSL_CPE_Malloc (strlen (optarg) + 1);
         if (g_sVdslScript)
         {
            strcpy (g_sVdslScript, optarg);
         }
         break;
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT */

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
      case 's':
         if (optarg != NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "using SOAP server - %s" DSL_CPE_CRLF, optarg));

            sSoapRemoteServer = DSL_CPE_Malloc(strlen (optarg) + 1);
            if (sSoapRemoteServer)
            {
               strcpy (sSoapRemoteServer, optarg);
            }
         }
         break;
#endif
      default:
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Sorry, there is a unrecognized option: %c" DSL_CPE_CRLF, c));
         break;
      }
   }

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   if( (g_sAdslScript != DSL_NULL)
#ifdef INCLUDE_DSL_CPE_API_VINAX
      || (g_sVdslScript != DSL_NULL)
#endif
      )
   {
      DSL_uint32_t nDevice = 0;
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         g_bWaitBeforeLinkActivation[nDevice] = DSL_TRUE;
         g_bWaitBeforeConfigWrite[nDevice]    = DSL_TRUE;
         g_bWaitBeforeRestart[nDevice]        = DSL_TRUE;
      }
   }
   else
   {
      DSL_uint32_t nDevice = 0;
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         g_bWaitBeforeLinkActivation[nDevice] = DSL_FALSE;
         g_bWaitBeforeConfigWrite[nDevice]    = DSL_FALSE;
         g_bWaitBeforeRestart[nDevice]        = DSL_FALSE;
      }
   }
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */
}

/**
Print usage help.

\param sApplicationName    name of the application

*/
DSL_CPE_STATIC  DSL_void_t DSL_CPE_Help (
   DSL_char_t *sApplicationName
)
{
   struct option *ptr;
   DSL_char_t *desc = description[0];

   ptr = long_options;

   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "Welcome to DSL CPI API control application" DSL_CPE_CRLF));

   if (strlen(sApplicationName) == 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "usage: %s [options]" DSL_CPE_CRLF, sApplicationName));
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "usage: [options]" DSL_CPE_CRLF, sApplicationName));
   }
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "following options are available:" DSL_CPE_CRLF));
#ifdef DEBUG
   sleep (1);
#endif

   while (ptr->name)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         " --%s\t(-%c)\t- %s" DSL_CPE_CRLF, ptr->name, ptr->val, desc));
#ifdef DEBUG
      sleep (1);
#endif
      ptr++;
      desc += sizeof (description[0]);
   }
   return;
}

#ifdef DSL_CPE_SOAP_FW_UPDATE
DSL_Error_t DSL_CPE_SoapFirmwareUpdate(
   DSL_CPE_Firmware_t *pFirmware,
   DSL_CPE_Firmware_t *pFirmware2)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_CPE_Control_Context_t *pCtrlCtx = DSL_NULL;

   pCtrlCtx = DSL_CPE_GetGlobalContext();

   if (pCtrlCtx == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Invalid context pointer!" DSL_CPE_CRLF));
      return DSL_ERROR;
   }

   DSL_CPE_LockGet(&pCtrlCtx->semFwUpdate);

   nRet = DSL_CPE_SoapFirmwareStore(pCtrlCtx, pFirmware, &(pCtrlCtx->firmware));
   nRet = DSL_CPE_SoapFirmwareStore(pCtrlCtx, pFirmware2, &(pCtrlCtx->firmware2));

   DSL_CPE_LockSet(&pCtrlCtx->semFwUpdate);

   return nRet;
}

DSL_Error_t DSL_CPE_SoapFirmwareStore(
   DSL_CPE_Control_Context_t *pContext,
   DSL_CPE_Firmware_t *pSrcFirmware,
   DSL_CPE_Firmware_t *pDestFirmware)
{

   if ((pSrcFirmware->pData != DSL_NULL) && (pSrcFirmware->nSize > 0))
   {
      if (pDestFirmware->pData != DSL_NULL)
      {
         DSL_CPE_Free(pDestFirmware->pData);
         pDestFirmware->pData = DSL_NULL;
         pDestFirmware->nSize = 0;
      }

      pDestFirmware->pData = DSL_CPE_Malloc(pSrcFirmware->nSize);
      if (pDestFirmware->pData != DSL_NULL)
      {
         memcpy(pDestFirmware->pData, pSrcFirmware->pData, pSrcFirmware->nSize);
         pDestFirmware->nSize = pSrcFirmware->nSize;
      }
      else
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Failed to allocate memory to store firmware binary of size %d bytes"
            DSL_CPE_CRLF, pSrcFirmware->nSize));
         return DSL_ERROR;
      }
   }

   return DSL_SUCCESS;
}
#endif /* DSL_CPE_SOAP_FW_UPDATE */

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_char_t g_nDevNumStr[20];

DSL_char_t *DSL_CPE_Fd2DevStr(DSL_int_t fd)
{
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   DSL_int_t i;
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
   DSL_char_t *pDevSrt = g_nDevNumStr;
   DSL_CPE_Control_Context_t *pCtx = DSL_NULL;

   sprintf (pDevSrt, "%s", "");

   /* Get Global Context pointer*/
   pCtx = DSL_CPE_GetGlobalContext();

   if (pCtx == DSL_NULL)
   {
      return pDevSrt;
   }

#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   if (!(pCtx->bBackwardCompMode))
   {
      for (i = 0; i < DSL_CPE_MAX_DEVICE_NUMBER; i++)
      {
         if (fd == pCtx->fd[i])
         {
            sprintf (pDevSrt, " nDevice=%d", i);
            return pDevSrt;
         }
      }
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/

   return pDevSrt;
}

DSL_boolean_t DSL_CPE_IsFileExists(DSL_char_t *path)
{
   DSL_CPE_File_t *fd = DSL_NULL;

   fd = DSL_CPE_FOpen (path, "r");

   if (fd == DSL_NULL)
   {
      return DSL_FALSE;
   }
   else
   {
      DSL_CPE_FClose(fd);
      return DSL_TRUE;
   }
}

DSL_int_t DSL_CPE_CliDeviceCommandExecute(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_char_t *cmd,
   DSL_char_t *arg,
   DSL_CPE_File_t *out)
{
   DSL_int_t ret = 0;
   DSL_int_t fd[DSL_CPE_MAX_DEVICE_NUMBER], i, nDeviceNumber = -1;
   DSL_char_t help[] = "-h";
   DSL_boolean_t bHelp = DSL_FALSE;
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   DSL_int_t nDevNum = 0;
   DSL_char_t dummy_arg[10] = "";
#endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/

   /* Check Context pointer*/
   if (pContext == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "Expecting non-zero context pointer!" DSL_CPE_CRLF));
      return -1;
   }

   if (nDevice >= DSL_CPE_MAX_DEVICE_NUMBER)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "Invalid device number (%d) specified!" DSL_CPE_CRLF,
         nDeviceNumber));

      return -1;
   }

   /* Reset fd*/
   for (i = 0; i < DSL_CPE_MAX_DEVICE_NUMBER; i++)
   {
      fd[i] = -1;
   }

   /* Check if the Backward compatible mode was specified*/
   if (pContext->bBackwardCompMode)
   {
      nDeviceNumber = (nDevice == -1) ? pContext->nDevNum : nDevice;
      
      if (nDeviceNumber >= DSL_CPE_MAX_DEVICE_NUMBER)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "Invalid device number (%d) specified!" DSL_CPE_CRLF,
            nDeviceNumber));

         DSL_CPE_FPrintf (out, "nReturn=%d" DSL_CPE_CRLF, -1);

         return -1;
      }

      /* Get fd for the specified nDeviceNumber*/
      fd[0] = pContext->fd[nDeviceNumber];
   }
   else
   {
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
      if(arg != DSL_NULL)
      {
         arg = DSL_CPE_CLI_WhitespaceRemove(arg);

         if (DSL_CPE_CLI_CheckParamNumber(arg, 1, DSL_CLI_MIN) == DSL_FALSE)
         {
            bHelp = DSL_TRUE;
         }
         else
         {
            /* Get device number*/
            DSL_CPE_sscanf (arg, "%d", &nDevNum);

            /* Check device number*/   
            if (nDevNum >= DSL_CPE_MAX_DEVICE_NUMBER)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
                  (DSL_CPE_PREFIX "Invalid device number (%d) specified, valid range 0...%d!" DSL_CPE_CRLF,
                  nDevNum, (DSL_CPE_MAX_DEVICE_NUMBER-1)));

               DSL_CPE_FPrintf (out, "nReturn=%d" DSL_CPE_CRLF, -1);

               return -1;
            }

            /* Move to the next parameter*/
            while (1)
            {
               if (isspace((int)(*arg)))
               {
                  break;
               }
         
               if (*arg == '\0')
               {
                  arg = dummy_arg;
                  break;
               }
      
               arg++;
            }
         }
      }
      else
      {
         bHelp = DSL_TRUE;
      }

      if (nDevice == -1)
      {
         /* Normal CLI command*/
         if (nDevNum == -1)
         {
            /* CLI command for all available devices*/
            for (i = 0; i < DSL_CPE_MAX_DEVICE_NUMBER; i++)
            {
               fd[i] = pContext->fd[i];
            }
         }
         else
         {
            fd[0] = pContext->fd[nDevNum];
         }
      }
      else
      {
         if (nDevice == nDevNum)
         {
            fd[0] = pContext->fd[nDevNum];
         }
         else if (nDevNum == -1)
         {
            fd[0] = pContext->fd[nDevice];
         }
         else
         {
            /* Skip CLI handling*/
            return 0;
         }
      }
#else
      /* Get fd for the default Device*/
      fd[0] = pContext->fd[0];
#endif
   }

   for (i = 0; i < DSL_CPE_MAX_DEVICE_NUMBER; i++)
   {
      if (fd[i] == -1)
         continue;

      if ((ret = DSL_CPE_CLI_CommandExecute(fd[i], cmd, bHelp ? help : arg, out)) < 0)
         return ret;
   }

   return ret;
}
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

DSL_Error_t DSL_CPE_LoadFirmwareFromFile(
   DSL_char_t *psFirmwareName,
   DSL_uint8_t **pFirmware,
   DSL_uint32_t *pnFirmwareSize)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_CPE_File_t *fd_image = DSL_NULL;
   DSL_CPE_stat_t file_stat;

   *pnFirmwareSize = 0;
   if (*pFirmware != DSL_NULL)
   {
      DSL_CPE_Free(*pFirmware);
      *pFirmware = DSL_NULL;
   }

#ifdef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
   *pFirmware      = cgi_pFileData_modemfw_bin;
   *pnFirmwareSize = FIRMWARE_SIZE;
#else
   for (;;)
   {
      fd_image = DSL_CPE_FOpen (psFirmwareName, "rb");
      if (fd_image == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "open %s fail" DSL_CPE_CRLF, psFirmwareName));
         nRet = DSL_ERROR;
         break;
      }

      DSL_CPE_FStat (psFirmwareName, &file_stat);
      *pnFirmwareSize = file_stat.st_size;
      *pFirmware = DSL_CPE_Malloc (*pnFirmwareSize);
      if (DSL_CPE_FRead (*pFirmware, sizeof (DSL_uint8_t), *pnFirmwareSize, fd_image) <= 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
            "\n firmware_image not present"));
         nRet = DSL_ERROR;
         break;
      }

      break;
   }

   if (fd_image != DSL_NULL)
   {
      DSL_CPE_FClose (fd_image);
      fd_image = DSL_NULL;
   }
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/
   return nRet;
}

DSL_Error_t DSL_CPE_DownloadFirmware(
   DSL_int_t fd,
   DSL_FirmwareRequestType_t nFwReqType,
   DSL_char_t *pcFw,
   DSL_char_t *pcFw2)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_CPE_Control_Context_t *pContext = DSL_NULL;
   DSL_AutobootLoadFirmware_t ldFw;
   DSL_AutobootControl_t pAcs;
   DSL_AutobootStatus_t pAsg;
   DSL_CPE_File_t *fd_image = DSL_NULL;
   DSL_CPE_stat_t file_stat;
   DSL_int_t nSize = 0, nReadSize = DSL_CPE_FW_CHUNK_SIZE;
   DSL_int_t nWriteSize = 0;
   DSL_uint8_t *pChunkData = DSL_NULL;
#ifdef DSL_CPE_SOAP_FW_UPDATE
   DSL_boolean_t bSetFw1 = DSL_TRUE;
#if defined(INCLUDE_DSL_CPE_API_VINAX)
   DSL_boolean_t bSetFw2 = DSL_TRUE;
#endif
#endif /* DSL_CPE_SOAP_FW_UPDATE*/

   memset(&ldFw, 0, sizeof(DSL_AutobootLoadFirmware_t));

   pContext = DSL_CPE_GetGlobalContext();
   if (pContext == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifdef DSL_CPE_SOAP_FW_UPDATE
   DSL_CPE_LockGet(&pContext->semFwUpdate);

   /*
      If firmware binaries are given by SOAP interface this will be used in any
      case.
   */
   if ((pContext->firmware.pData != DSL_NULL) && (pContext->firmware.nSize > 0))
   {
      ldFw.data.pFirmware = pContext->firmware.pData;
      ldFw.data.nFirmwareSize = pContext->firmware.nSize;
      pcFw = DSL_NULL;
      bSetFw1 = DSL_FALSE;
   }

#if defined(INCLUDE_DSL_CPE_API_VINAX)
   if ((pContext->firmware2.pData != DSL_NULL) && (pContext->firmware2.nSize > 0))
   {
      ldFw.data.pFirmware2 = pContext->firmware2.pData;
      ldFw.data.nFirmwareSize2 = pContext->firmware2.nSize;
      pcFw2 = DSL_NULL;
      bSetFw2 = DSL_FALSE;
   }
#endif /* INCLUDE_DSL_CPE_API_VINAX*/

#endif /* DSL_CPE_SOAP_FW_UPDATE */

   /*
      If firmware name is not defined by current call and no SOAP defined
      firmware is available choose last defined file (or default if startup
      value was not changed).
   */
#ifdef DSL_CPE_SOAP_FW_UPDATE
   if ( (pcFw == DSL_NULL) && (bSetFw1 == DSL_TRUE) )
#else
   if ( pcFw == DSL_NULL )
#endif /* DSL_CPE_SOAP_FW_UPDATE*/
   {
      if ((g_bFirmware1 != -1) || (strlen(g_sFirmwareName1) > 0))
      {
         pcFw = g_sFirmwareName1;
      }
   }

#if defined(INCLUDE_DSL_CPE_API_VINAX)
#ifdef DSL_CPE_SOAP_FW_UPDATE
   if ( (pcFw2 == DSL_NULL) && (bSetFw2 == DSL_TRUE) )
#else
   if ( pcFw2 == DSL_NULL )
#endif /* DSL_CPE_SOAP_FW_UPDATE*/
   {
      if ((g_bFirmware2 != -1) || (strlen(g_sFirmwareName2) > 0))
      {
         pcFw2 = g_sFirmwareName2;
      }
   }
#endif

   do
   {
#ifdef DSL_CPE_SOAP_FW_UPDATE
      if ( (bOptimize == 1) && (bSetFw1 == DSL_TRUE) )
#else
      if ( bOptimize == 1 )
#endif /* DSL_CPE_SOAP_FW_UPDATE*/
      {
         /*
            Footprint optimized download uses chunks for firmware binary download.
            This is to reduce the necessary runtime memory allocation for download
            procedure
         */

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Using optimized fw "
            "download..." DSL_CPE_CRLF));

         if (pcFw == DSL_NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               " firmware file not found!" DSL_CPE_CRLF, pcFw));
            nRet = DSL_ERROR;
            break;
         }

         fd_image = DSL_CPE_FOpen (pcFw, "r");
         if (fd_image == DSL_NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX " open %s failed."
               DSL_CPE_CRLF, pcFw));
            nRet = DSL_ERROR;
            break;
         }

         DSL_CPE_FStat (pcFw, &file_stat);
         nSize = file_stat.st_size;

         pChunkData = DSL_CPE_Malloc((DSL_uint32_t)nReadSize);
         if (pChunkData == DSL_NULL)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Memory allocation "
               "failed (optimized FW download)" DSL_CPE_CRLF));
            nRet = DSL_ERROR;
            break;
         }

         ldFw.data.pFirmware = pChunkData;

         while (nSize > 0)
         {
            if (nSize > DSL_CPE_FW_CHUNK_SIZE)
            {
               nReadSize = DSL_CPE_FW_CHUNK_SIZE;
            }
            else
            {
               nReadSize = nSize;
            }

            if (DSL_CPE_FRead (pChunkData, sizeof (DSL_uint8_t), (DSL_uint32_t)nReadSize,
               fd_image) <= 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "firmware_image not present" DSL_CPE_CRLF));
               nRet = DSL_ERROR;
               break;
            }

            ldFw.data.nFirmwareSize   = (DSL_uint32_t)nReadSize;
            ldFw.data.nFirmwareOffset = (DSL_uint32_t)nWriteSize;

            if (nSize - nReadSize <= 0)
            {
               ldFw.data.bLastChunk = DSL_TRUE;
            }
            else
            {
               ldFw.data.bLastChunk = DSL_FALSE;
            }

            ldFw.data.bChunkDonwloadEnabled = DSL_TRUE;

            nRet = (DSL_Error_t) DSL_CPE_Ioctl(fd,
               DSL_FIO_AUTOBOOT_LOAD_FIRMWARE, (DSL_int_t) &ldFw);

            if (nRet < DSL_SUCCESS)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Autoboot Load Firmware (using chunks) failed!, nRet = %d!"
                  DSL_CPE_CRLF, ldFw.accessCtl.nReturn));

               break;
            }

            nSize -= nReadSize;
            nWriteSize += nReadSize;
         }

         if (nRet < DSL_SUCCESS)
         {
            break;
         }
         else
         {
            /* $$TD: Temporary only because better solution would be to start
               the link directly within driver after last chunk is downloaded */
            DSL_CPE_Sleep(1);
            nRet = (DSL_Error_t) DSL_CPE_Ioctl(fd,
               DSL_FIO_AUTOBOOT_STATUS_GET, (DSL_int_t) &pAsg);

            if (pAsg.data.nStatus == DSL_AUTOBOOT_STATUS_STOPPED)
            {
               pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_START;
               nRet = (DSL_Error_t) DSL_CPE_Ioctl(fd,
                  DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);
               nRet = pAcs.accessCtl.nReturn;
            }
         }

         /* also store the new firmware binary name within global
            configuration to be used for next download */
         if (pcFw != g_sFirmwareName1)
         {
            /* also store the new firmware binary name within global
            configuration to be used for next download */
            if (ldFw.data.pFirmware != DSL_NULL)
            {
               DSL_CPE_Free(g_sFirmwareName1);
               g_sFirmwareName1 = DSL_CPE_Malloc (strlen (pcFw) + 1);
               if (g_sFirmwareName1)
               {
                  strcpy (g_sFirmwareName1, pcFw);
               }
            }
         }
      }
      else
      {
         /* simple download without any memory optimizations just loads the
            necessary firmware binaries completely into memory. */

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Using normal fw "
            "download..." DSL_CPE_CRLF));

#if defined(INCLUDE_DSL_CPE_API_VINAX)
         if ((nFwReqType == DSL_FW_REQUEST_VDSL) || (nFwReqType == DSL_FW_REQUEST_NA))
#else
         if ((nFwReqType == DSL_FW_REQUEST_ADSL) || (nFwReqType == DSL_FW_REQUEST_NA))
#endif
         {
            if (pcFw != DSL_NULL)
            {
               nRet = DSL_CPE_LoadFirmwareFromFile(pcFw, &ldFw.data.pFirmware,
                  &ldFw.data.nFirmwareSize);
               if (nRet < DSL_SUCCESS)
               {
                  nRet = DSL_ERROR;
               }
               else
               {
                  if (pcFw != g_sFirmwareName1)
                  {
                     /* also store the new firmware binary name within global
                        configuration to be used for next download */
                     if (ldFw.data.pFirmware != DSL_NULL)
                     {
                        DSL_CPE_Free(g_sFirmwareName1);
                        g_sFirmwareName1 = DSL_CPE_Malloc (strlen (pcFw) + 1);
                        if (g_sFirmwareName1)
                        {
                           strcpy (g_sFirmwareName1, pcFw);
                        }
                     }
                  }
               }
            }
         }
#if defined(INCLUDE_DSL_CPE_API_VINAX)
         if ((nFwReqType == DSL_FW_REQUEST_ADSL) || (nFwReqType == DSL_FW_REQUEST_NA))
         {
            if (pcFw2 != DSL_NULL)
            {
               nRet = DSL_CPE_LoadFirmwareFromFile(pcFw2, &ldFw.data.pFirmware2,
                  &ldFw.data.nFirmwareSize2);
               if (nRet < DSL_SUCCESS)
               {
                  nRet = DSL_ERROR;
               }
               else
               {
                  if (pcFw2 != g_sFirmwareName2)
                  {
                     /* also store the new firmware binary name within global
                        configuration to be used for next download */
                     if (ldFw.data.pFirmware2 != DSL_NULL)
                     {
                        DSL_CPE_Free(g_sFirmwareName2);
                        g_sFirmwareName2 = DSL_CPE_Malloc (strlen (pcFw2) + 1);
                        if (g_sFirmwareName2)
                        {
                           strcpy (g_sFirmwareName2, pcFw2);
                        }
                     }
                  }
               }
            }
         }
#endif

         if ( ((ldFw.data.pFirmware  != DSL_NULL) && (ldFw.data.nFirmwareSize)) ||
              ((ldFw.data.pFirmware2 != DSL_NULL) && (ldFw.data.nFirmwareSize2)) )
         {
            ldFw.data.bLastChunk = DSL_TRUE;
            ldFw.data.bChunkDonwloadEnabled = DSL_FALSE;

            nRet = (DSL_Error_t) DSL_CPE_Ioctl(fd,
               DSL_FIO_AUTOBOOT_LOAD_FIRMWARE, (DSL_int_t) &ldFw);

            if (nRet < DSL_SUCCESS)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Autoboot Load Firmware failed!, nRet = %d!" DSL_CPE_CRLF,
                  ldFw.accessCtl.nReturn));
               break;
            }
         }
         else
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "Both FW binaries are not specified!" DSL_CPE_CRLF));
            nRet = DSL_ERROR;
            break;
         }
      }
   } while (0);

#ifdef DSL_CPE_SOAP_FW_UPDATE
   DSL_CPE_LockSet(&pContext->semFwUpdate);
#endif /* DSL_CPE_SOAP_FW_UPDATE */

   /* take care to clean up all memory that might be allocated during function
      processing */
   if (fd_image != DSL_NULL)
   {
      DSL_CPE_FClose(fd_image);
   }

#ifndef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
   if (pChunkData != DSL_NULL)
   {
      DSL_CPE_Free(pChunkData);
      pChunkData = DSL_NULL;
      ldFw.data.pFirmware = DSL_NULL;
   }

#ifdef DSL_CPE_SOAP_FW_UPDATE
   if (ldFw.data.pFirmware && bSetFw1)
#else
   if (ldFw.data.pFirmware)
#endif /* DSL_CPE_SOAP_FW_UPDATE*/
   {
      DSL_CPE_Free(ldFw.data.pFirmware);
   }

#if defined(INCLUDE_DSL_CPE_API_VINAX)
#ifdef DSL_CPE_SOAP_FW_UPDATE
   if (ldFw.data.pFirmware2 && bSetFw2)
#else
   if (ldFw.data.pFirmware2)
#endif /* DSL_CPE_SOAP_FW_UPDATE*/
   {
      DSL_CPE_Free(ldFw.data.pFirmware2);
   }
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX) */
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/

#if defined(INCLUDE_DSL_CPE_API_VINAX)
   if (nRet >= DSL_SUCCESS)
   {
      /* Check Low Level configuration*/
      nRet = DSL_CPE_LowLevelConfigurationCheck(fd);
   }
#endif /* defined(INCLUDE_DSL_CPE_API_VINAX)*/

   return nRet;
}

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_CPE_STATIC  DSL_char_t const * DSL_CPE_Event_Type2String(DSL_EventType_t *pEventType)
{
   DSL_int32_t i;
   DSL_CPE_EVT_CodeString_t const *pCS;

   pCS = eventString;
   for (i = 0; pCS[i].string; i++)
   {
      if (pCS[i].eventType == *pEventType)
      {
         return pCS[i].string;
      }
   }

   return "Unknown";
}
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
DSL_CPE_STATIC  DSL_boolean_t DSL_CPE_ScriptSectionTagFind(
   DSL_char_t *pLine,
   DSL_CPE_ScriptSection_t searchSection)
{
   DSL_boolean_t bTagOpenFound = DSL_FALSE,
                 bTagCloseFound = DSL_FALSE,
                 bTagNameFound = DSL_FALSE,
                 bRet = DSL_FALSE;
   DSL_char_t *pTag = DSL_NULL;
   DSL_uint8_t nTagSize = 0;

   if (pLine == DSL_NULL)
   {
      return DSL_FALSE;
   }

   /* find tag open*/
   while (*pLine != '\0')
   {
      if (*pLine == '[')
      {
         /* tag open found*/
         bTagOpenFound = DSL_TRUE;
         /* move to the next character*/
         pLine++;
         break;
      }
      /* move to the next character*/
      pLine++;
   }

   /* find tag name*/
   if (bTagOpenFound)
   {
      /* remove leading whitespaces*/
      while (isspace((DSL_int_t)*pLine) && *pLine != '\0')
      {
         pLine++;
      }

      if (searchSection == DSL_SCRIPT_SECTION_WAIT_FOR_CONFIFURATION)
      {
         pTag = DSL_CPE_SCRIPT_WAIT_FOR_CONFIGURATION_TAG;
         nTagSize = sizeof(DSL_CPE_SCRIPT_WAIT_FOR_CONFIGURATION_TAG)-1;
      }
      else if (searchSection == DSL_SCRIPT_SECTION_WAIT_FOR_LINK_ACTIVATE)
      {
         pTag = DSL_CPE_SCRIPT_WAIT_FOR_LINK_ACTIVATE_TAG;
         nTagSize = sizeof(DSL_CPE_SCRIPT_WAIT_FOR_LINK_ACTIVATE_TAG)-1;
      }
      else if (searchSection == DSL_SCRIPT_SECTION_COMMON)
      {
        pTag = DSL_CPE_SCRIPT_COMMON_TAG;
        nTagSize = sizeof(DSL_CPE_SCRIPT_COMMON_TAG)-1;
      }
      else if (searchSection == DSL_SCRIPT_SECTION_WAIT_BEFORE_RESTART)
      {
        pTag = DSL_CPE_SCRIPT_WAIT_BEFORE_RESTART_TAG;
        nTagSize = sizeof(DSL_CPE_SCRIPT_WAIT_BEFORE_RESTART_TAG)-1;
      }

      if (pTag)
      {
         /* compare case insensitive*/
         if (DSL_CPE_STRNCASECMP(pTag, pLine, nTagSize) == 0)
         {
            bTagNameFound = DSL_TRUE;
         }
      }
   }

   /* find tag close*/
   if (bTagNameFound)
   {
      /* remove ending whitespaces*/
      while (isspace((DSL_int_t)*pLine) && *pLine != '\0')
      {
         pLine++;
      }

      while (*pLine != '\0')
      {
         if (*pLine == ']')
         {
            /* tag open found*/
            bTagCloseFound = DSL_TRUE;
            break;
         }
         /* move to the next character*/
         pLine++;
      }
   }

   if (bTagOpenFound && bTagNameFound && bTagCloseFound && pTag)
   {
      bRet = DSL_TRUE;
   }

   return bRet;
}

DSL_CPE_STATIC  DSL_boolean_t DSL_CPE_ScriptSectionOpenFind(
   DSL_char_t *pLine)
{
   DSL_boolean_t bRet = DSL_FALSE;

   if (pLine == DSL_NULL)
   {
      return DSL_FALSE;
   }

   /* find section open*/
   while (*pLine != '\0')
   {
      if (*pLine == '{')
      {
         /* section open found*/
         bRet = DSL_TRUE;
         break;
      }
      /* move to the next character*/
      pLine++;
   }

   return bRet;
}

DSL_CPE_STATIC  DSL_boolean_t DSL_CPE_ScriptSectionCloseFind(
   DSL_char_t *pLine)
{
   DSL_boolean_t bRet = DSL_FALSE;

   if (pLine == DSL_NULL)
   {
      return DSL_FALSE;
   }

   /* find section close*/
   while (*pLine != '\0')
   {
      if (*pLine == '}')
      {
         /* section close found*/
         bRet = DSL_TRUE;
         break;
      }
      /* move to the next character*/
      pLine++;
   }

   return bRet;
}

/**
   Parses a batch file and performs necessary actions
*/
DSL_Error_t DSL_CPE_ScriptExecute (
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_CPE_File_t *pFile,
   DSL_CPE_ScriptSection_t searchSection)
{
   DSL_char_t buf[128];
   DSL_char_t *pLine = DSL_NULL;
   DSL_char_t str_command[sizeof(buf)] = { 0 };
   DSL_boolean_t bTagFound = DSL_FALSE,
                 bSectionOpenFound = DSL_FALSE;
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   DSL_char_t str_group[20] = { 0 };
   DSL_char_t str_value[128] = { 0 };
#if defined(INCLUDE_DSL_CPE_CMV_SCRIPTS_MATH)
   DSL_char_t op1[40], op2[40];
   DSL_uint32_t var32[8];
   DSL_uint16_t Message[DSL_MAX_CMV_MSG_LENGTH];
#endif /* defined(INCLUDE_DSL_CPE_CMV_SCRIPTS_MATH)*/
   DSL_int_t address, index;
   DSL_int_t n = 0;
   DSL_uint16_t value = 0;
   DSL_uint16_t var16[8];
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_DeviceMessage_t msg;
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/
#ifndef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
   DSL_char_t *pFile__;
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/

   if (!pFile)
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
   /* scan one line into buffer */
   while ((DSL_CPE_FGets (buf, sizeof(buf), pFile)) != DSL_NULL)
   {
      pLine = buf;

      /* if the line is empty or a special "#" symbol detected,
         then go on to the next */
      if ((sscanf (buf, "%s", &str_command) == 0) || (buf[0] == '#'))
      {
         continue;
      }
#else
   pFile__ = (DSL_char_t*) pFile;

   while (1)
   {
      sscanf (pFile__, "%[^\n]s", &buf);

      while(*pFile__++ != '\n');

      if ((sscanf (buf, "%s", &str_command) == 0) || (buf[0] == '#'))
      {
         continue;
      }

      if (strcmp(str_command, "EOF") == 0)
      {
         break;
      }

      pLine = buf;
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/

      /* remove leading whitespaces*/
      while((DSL_int_t)isspace(*pLine))
      {
         pLine++;
      }

      if (!bTagFound)
      {
         /* find search section tag*/
         if (!DSL_CPE_ScriptSectionTagFind(pLine, searchSection))
         {
            continue;
         }
         else
         {
            /* search section tag found*/
            bTagFound = DSL_TRUE;
         }
      }

      /* find search section open*/
      if (bTagFound && !bSectionOpenFound)
      {
         if (DSL_CPE_ScriptSectionOpenFind(pLine))
         {
            /* search section open found*/
            bSectionOpenFound = DSL_TRUE;
         }
         continue;
      }

      /* Check for search section close*/
      if (bSectionOpenFound)
      {
         if (DSL_CPE_ScriptSectionCloseFind(pLine))
         {
            /* Complete section proceeded, reset flags*/
            bTagFound = DSL_FALSE;
            bSectionOpenFound = DSL_FALSE;
            continue;
         }
      }

      if (strcmp (str_command, "echo") == 0)       /* echo command */
      {
         DSL_CPE_Echo (buf);
         memset (str_command, 0, sizeof(str_command));
         memset (buf, 0, sizeof(buf));
         continue;
      }

#if defined (INCLUDE_DSL_CPE_API_DANUBE)
      memset(&msg, 0, sizeof(DSL_DeviceMessage_t));

      /* if the line is empty, then go on to the next */
      if (strcmp (str_command, "cr") == 0)      /* cr command */
      {
         sscanf (buf, "%s %s %d %d %s", &str_command[0], &str_group[0], &address,
            &index, &str_value[0]);

         if (strncmp (str_value, "$", 1) != 0)
         {
            return DSL_ERR_INVALID_PARAMETER;
         }

         n = strtoul (str_value + 1, DSL_NULL, 10);

         if (n >= sizeof(var16)/sizeof(var16[0]))
         {
            return DSL_ERR_INVALID_PARAMETER;
         }

         nErrCode = DSL_CMV_Read (pContext, str_group, address, index, 1, &var16[n]);
         DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "read %s %d %d, value=%04x, "
            "retCode=%d\n", str_group, address, index, var16[n], nErrCode);
      }
      else if (strcmp (str_command, "cw") == 0) /* cw command */
      {
         sscanf (buf, "%s %s %d %d %s", &str_command[0], &str_group[0], &address,
            &index, &str_value[0]);

         if (strncmp (str_value, "$", 1) == 0)
         {
            n = strtoul (str_value + 1, DSL_NULL, 10);

            if (n >= sizeof(var16)/sizeof(var16[0]))
            {
               return DSL_ERR_INVALID_PARAMETER;
            }

            value = var16[n];
         }
         else if (strncmp (str_value, "#", 1) == 0)
         {
            value = strtoul (str_value + 1, DSL_NULL, 16);
         }
         else
         {
            value = strtoul (str_value, DSL_NULL, 16);
         }
         if (DSL_CMV_Write (pContext, str_group, address, index, 1, &value) != DSL_SUCCESS)
         {
            return DSL_ERR_MSG_EXCHANGE;
         }
      }
#if defined(INCLUDE_DSL_CPE_CMV_SCRIPTS_MATH)
      else if (strcmp (str_command, "mw") == 0) /* debug memory write command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &str_value[0]);

         address = strtoul (op1, DSL_NULL, 0);
         if (strncmp (str_value, "$", 1) == 0)
         {
            n = strtoul (str_value + 1, DSL_NULL, 10);

            if (n >= sizeof(var16)/sizeof(var16[0]))
            {
               return DSL_ERR_INVALID_PARAMETER;
            }

            value = var16[n];
         }
         else if (strncmp (str_value, "#", 1) == 0)
         {
            value = strtoul (str_value + 1, DSL_NULL, 0);
         }
         else
         {
            value = strtoul (str_value, DSL_NULL, 0);
         }

         DSL_CMV_Prepare (DSL_CMV_OPCODE_H2D_DEBUG_WRITE_DM, 0x0, address >> 16,
            (address) & (0xffff), 1, &value, Message);

         msg.data.pMsg = (DSL_uint8_t *)Message;
         msg.data.nSizeTx = 8;
         if (DSL_CPE_Ioctl (pContext->fd[pContext->nDevNum], DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int)&Message) < 0)
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "mw %08x fail", address);
            return DSL_ERR_MSG_EXCHANGE;
         }
      }
      else if (strcmp (str_command, "mr") == 0) /* debug memory read command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &str_value[0]);
         address = strtoul (op1, DSL_NULL, 0);
         n = strtoul (str_value + 1, DSL_NULL, 10);

         if (n >= sizeof(var16)/sizeof(var16[0]))
         {
            return DSL_ERR_INVALID_PARAMETER;
         }

         if (strncmp (str_value, "$", 1) != 0)
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "error:mr parameter type "
               "mismatch!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
         DSL_CMV_Prepare (DSL_CMV_OPCODE_H2D_DEBUG_READ_DM, 0x0, address >> 16,
            (address) & (0xffff), 1, DSL_NULL, Message);

         msg.data.pMsg = (DSL_uint8_t *)Message;
         msg.data.nSizeTx = 8;
         msg.data.nSizeRx = 10;
         if (DSL_CPE_Ioctl (pContext->fd[pContext->nDevNum], DSL_FIO_DBG_DEVICE_MESSAGE_SEND, (int)&Message) < 0)
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "mw %08x fail", address);
            return DSL_ERR_MSG_EXCHANGE;
         }
         value = Message[4];
         var16[n] = value;
      }


      /* meiw command */
/*$$
      else if (strcmp (str_command, "meiw") == 0)
      {
         meireg regrdwr;

         sscanf (buf, "%s %s %s", &str_command, &op1, &op2);
         regrdwr.iAddress = strtoul (op1, DSL_NULL, 0) + MEI_SPACE_ACCESS;


         if (strncmp (op2, "#", 1) == 0)
            regrdwr.iData = strtoul (op1 + 1, DSL_NULL, 0);
         else if (strncmp (op2, "$", 1) == 0)
            regrdwr.iData = var16[strtoul (op2 + 1, DSL_NULL, 0)];
         else if (strncmp (op2, "@", 1) == 0)
            regrdwr.iData = var32[strtoul (op2 + 1, DSL_NULL, 0)];
         else
            regrdwr.iData = strtoul (op1, DSL_NULL, 0);
         // printf("address=%08x\n", regrdwr.iAddress);

         msg.data.pMsg = (DSL_uint8_t *)Message;
         msg.data.nSizeTx = 8;
         msg.data.nSizeRx = 10;

         if (ioctl (pContext->fd, DANUBE_MEI_CMV_WRITE, &regrdwr) < 0)
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "meiw fail\n");
            break;;
         }

      }
*/

      /* meir command */
/*$$
      else if (strcmp (str_command, "meir") == 0)
      {
         meireg regrdwr;

         sscanf (buf, "%s %s %s", &str_command, &op1, &op2);
         regrdwr.iAddress = strtoul (op1, DSL_NULL, 0) + MEI_SPACE_ACCESS;
         if (ioctl (pContext->fd, DANUBE_MEI_CMV_READ, &regrdwr) < 0)
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "meir fail\n");
            break;
         }
         if (strncmp (op2, "$", 1) == 0)
            var16[strtoul (op2 + 1, DSL_NULL, 0)] = regrdwr.iData;
         else if (strncmp (op2, "@", 1) == 0)
            var32[strtoul (op2 + 1, DSL_NULL, 0)] = regrdwr.iData;
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "meir grammar error!" DSL_CPE_CRLF);
            break;
         }

      }
*/

      else if (strcmp (str_command, "lst") == 0)        /* list command */
      {
         sscanf (buf, "%s %s", &str_command[0], &str_value[0]);

         n = strtoul (str_value + 1, DSL_NULL, 0);

         if (strncmp (str_value, "$", 1) == 0)
         {
            if (n >= sizeof(var16)/sizeof(var16[0]))
            {
               return DSL_ERR_INVALID_PARAMETER;
            }

            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "$%d=0x%04x\n", n, var16[n]);
         }
         else if (strncmp (str_value, "@", 1) == 0)
         {
            if (n >= sizeof(var32)/sizeof(var32[0]))
            {
               return DSL_ERR_INVALID_PARAMETER;
            }

            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "$%d=0x%08x\n", n, var32[n]);
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "lst grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }
      else if (strcmp (str_command, "mov") == 0)        /* move command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] =
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] =
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] = strtoul (op2 + 1, DSL_NULL, 0);
            }
            else
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] = strtoul (op2, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] =
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] =
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] = strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }
      else if (strcmp (str_command, "or") == 0) /* or command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] |=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] |=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] |= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] |=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] |=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] |= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }

      else if (strcmp (str_command, "and") == 0)        /* and command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] &=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] &=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] &= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] &=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] &=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] &= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }
      else if (strcmp (str_command, "not") == 0)        /* not command */
      {
         sscanf (buf, "%s %s", &str_command[0], &op1[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            var16[strtoul (op1 + 1, DSL_NULL, 0)] =
               ~var16[strtoul (op1 + 1, DSL_NULL, 0)];
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            var32[strtoul (op1 + 1, DSL_NULL, 0)] =
               ~var32[strtoul (op1 + 1, DSL_NULL, 0)];
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }
      else if (strcmp (str_command, "shl") == 0)        /* shift left command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] <<= var16[strtoul (op2 + 1,
                     DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] <<= var32[strtoul (op2 + 1,
                     DSL_NULL, 0)];
            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] <<= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] <<= var16[strtoul (op2 + 1,
                     DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] <<= var32[strtoul (op2 + 1,
                     DSL_NULL, 0)];
            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] <<= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }

      else if (strcmp (str_command, "shr") == 0)        /* shift right command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] >>= var16[strtoul (op2 + 1,
                     DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] >>= var32[strtoul (op2 + 1,
                     DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] >>= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] >>= var16[strtoul (op2 + 1,
                     DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] >>= var32[strtoul (op2 + 1,
                     DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] >>= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }

      else if (strcmp (str_command, "add") == 0)        /* addition command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] +=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] +=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] += strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] +=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] +=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] += strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }
      else if (strcmp (str_command, "sub") == 0)        /* subtraction command */
      {
         sscanf (buf, "%s %s %s", &str_command[0], &op1[0], &op2[0]);
         if (strncmp (op1, "$", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] -=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] -=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var16[strtoul (op1 + 1, DSL_NULL, 0)] -= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else if (strncmp (op1, "@", 1) == 0)
         {
            if (strncmp (op2, "$", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] -=
                  var16[strtoul (op2 + 1, DSL_NULL, 0)];
            }
            else if (strncmp (op2, "@", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] -=
                  var32[strtoul (op2 + 1, DSL_NULL, 0)];

            }
            else if (strncmp (op2, "#", 1) == 0)
            {
               var32[strtoul (op1 + 1, DSL_NULL, 0)] -= strtoul (op2 + 1, DSL_NULL, 0);
            }
         }
         else
         {
            DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "grammar error!" DSL_CPE_CRLF);
            return DSL_ERR_INVALID_PARAMETER;
         }
      }
#endif /* defined(INCLUDE_DSL_CPE_CMV_SCRIPTS_MATH)*/
      else
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/
      {
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
         /* Check if a CLI command is given */
         DSL_CPE_CliDeviceCommandExecute (pContext, nDevice, str_command,
            buf + strlen(str_command) + 1, DSL_CPE_STDOUT);
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
      }

      memset (str_command, 0, sizeof(str_command));
      memset (buf, 0, sizeof(buf));
   }
   return DSL_SUCCESS;
}
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
/**
   Performs an execution of debug batch file
*/
DSL_Error_t DSL_CPE_ScriptFileParse(
   DSL_CPE_Control_Context_t * pContext,
   DSL_int_t nDevice,
   DSL_char_t *sFileName,
   DSL_CPE_ScriptSection_t searchSection)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_CPE_File_t *pFile;

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Parsing script file (%s)..."
      DSL_CPE_CRLF, sFileName));
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/

   do
   {
      if (sFileName == DSL_NULL)
      {
         nErrCode = DSL_ERR_POINTER;
         break;
      }

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
      pFile = DSL_CPE_FOpen(sFileName, "r");
      if (pFile == DSL_NULL)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR - %s script file open failed! Have you used absolute script file path?" DSL_CPE_CRLF,
            sFileName));

         nErrCode = DSL_ERROR;
         break;
      }
#else
      pFile = (DSL_CPE_File_t*)sFileName;
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/

      nErrCode = DSL_CPE_ScriptExecute(pContext, nDevice, pFile, searchSection);

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
      DSL_CPE_FClose(pFile);
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT*/
   } while (0);


   if (nErrCode != DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Script file execution failed (nErrCode=%d)!" DSL_CPE_CRLF, nErrCode));
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "Script file execution performed successfully." DSL_CPE_CRLF));
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */

#ifdef INCLUDE_SCRIPT_NOTIFICATION
DSL_CPE_STATIC  DSL_void_t DSL_CPE_ScriptRun()
{
   if (g_sRcScript != DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "Run script file (%s)"
         DSL_CPE_CRLF, g_sRcScript));

      if (DSL_CPE_System(g_sRcScript) != DSL_SUCCESS
         && bScriptWarn == DSL_FALSE)
      {
         bScriptWarn = DSL_TRUE;
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "notification script file (%s) execution failed!" DSL_CPE_CRLF,
               g_sRcScript));
      }
   }

   return;
}
#endif /* #ifdef INCLUDE_SCRIPT_NOTIFICATION */

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_ResourceUsageStatisticsGet(
   DSL_CPE_Control_Context_t *pContext,
   DSL_CPE_ResourceUsageStatisticsData_t *pData)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_uint32_t staticMemUsageTotal = 0, dynamicMemUsageTotal = 0, currMemUsage = 0;
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_CLI_ResourceUsageStatisticsData_t cliResourceUsageStatisticsData;
#ifndef DSL_CPE_REMOVE_PIPE_SUPPORT
   DSL_uint32_t pipeStatResourceUsage = 0;
#endif /* DSL_CPE_REMOVE_PIPE_SUPPORT*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
#ifdef DSL_DEBUG_TOOL_INTERFACE
   DSL_CPE_TcpDebugResourceUsageStatisticsData_t TcpDebugResUsage;
#endif /*DSL_DEBUG_TOOL_INTERFACE*/

   if (pContext == DSL_NULL || pData == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   nRet = DSL_CPE_CLI_ResourceUsageGet(&cliResourceUsageStatisticsData);
   if (nRet != DSL_SUCCESS)
   {
      return nRet;
   }
#ifndef DSL_CPE_REMOVE_PIPE_SUPPORT
   nRet =  DSL_CPE_Pipe_StaticResourceUsageGet(&pipeStatResourceUsage);
   if (nRet != DSL_SUCCESS)
   {
      return nRet;
   }
#endif /* DSL_CPE_REMOVE_PIPE_SUPPORT*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

#ifdef DSL_DEBUG_TOOL_INTERFACE
   nRet = DSL_CPE_TcpDebugMessageResourceUsageGet(pContext, &TcpDebugResUsage);
   if (nRet != DSL_SUCCESS)
   {
      return nRet;
   }
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

   /* *********************************************************************** */
   /* *** Print statistic on memory usage of DSL CPE Control Application  *** */
   /* *********************************************************************** */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX DSL_CPE_CRLF ));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "*****************************************************" DSL_CPE_CRLF ));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " DSL CPE Control Application version: %s" DSL_CPE_CRLF, PACKAGE_VERSION));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "*****************************************************" DSL_CPE_CRLF ));
   /*
      Global Context static memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
   " Global Context static memory usage" DSL_CPE_CRLF ));
   currMemUsage = sizeof(DSL_CPE_Control_Context_t);
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "   context structure            : %10d bytes" DSL_CPE_CRLF, (int)currMemUsage));
   staticMemUsageTotal += currMemUsage;
   /*
      Auxiliary data static memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " Auxiliary data static memory usage" DSL_CPE_CRLF ));
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   currMemUsage = sizeof(eventString);
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "   events                       : %10d bytes" DSL_CPE_CRLF, (int)currMemUsage));
   staticMemUsageTotal += currMemUsage;
#endif /* #ifdef INCLUDE_DSL_CPE_CLI_SUPPORT*/
   currMemUsage = sizeof(DSL_InitData_t);
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "   initial configuration data   : %10d bytes" DSL_CPE_CRLF, (int)currMemUsage));
   staticMemUsageTotal += currMemUsage;
   currMemUsage = sizeof(long_options) + sizeof(GETOPT_LONG_OPTSTRING) + sizeof(description);
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "   help screen/startup options  : %10d bytes" DSL_CPE_CRLF, (int)currMemUsage));
   staticMemUsageTotal += currMemUsage;
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   /*
      CLI static memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " CLI static memory usage        : %10d bytes" DSL_CPE_CRLF, cliResourceUsageStatisticsData.staticMemUsage));
   staticMemUsageTotal += cliResourceUsageStatisticsData.staticMemUsage;
#ifndef DSL_CPE_REMOVE_PIPE_SUPPORT
   /*
      Pipe static memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " Pipe static memory usage       : %10d bytes" DSL_CPE_CRLF, pipeStatResourceUsage));
   staticMemUsageTotal += pipeStatResourceUsage;
#endif /* DSL_CPE_REMOVE_PIPE_SUPPORT*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "                                 --------------" DSL_CPE_CRLF ));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " Total static memory usage      : %10d bytes" DSL_CPE_CRLF, staticMemUsageTotal));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX DSL_CPE_CRLF ));
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   /*
      CLI dynamic memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " CLI dynamic memory usage       : %10d bytes" DSL_CPE_CRLF, cliResourceUsageStatisticsData.dynamicMemUsage));
   dynamicMemUsageTotal += cliResourceUsageStatisticsData.dynamicMemUsage;
   if (pConsoleContext != DSL_NULL)
   {
      currMemUsage = sizeof(DSL_CPE_Console_Context_t);
      dynamicMemUsageTotal += currMemUsage;
   }
   /*
      Console dynamic memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " Console dynamic memory usage   : %10d bytes" DSL_CPE_CRLF, currMemUsage));
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/

#ifdef DSL_DEBUG_TOOL_INTERFACE
   /*
      Console dynamic memory usage
   */
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " Debug tool dynamic memory usage: %10d bytes" DSL_CPE_CRLF, TcpDebugResUsage.dynamicMemUsage));
   dynamicMemUsageTotal += TcpDebugResUsage.dynamicMemUsage;
#endif /* DSL_DEBUG_TOOL_INTERFACE*/
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      "                                 --------------" DSL_CPE_CRLF ));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX
      " Total dynamic memory usage     : %10d bytes" DSL_CPE_CRLF, dynamicMemUsageTotal));
   DSL_CCA_DEBUG( DSL_CCA_DBG_PRN, (DSL_CPE_PREFIX DSL_CPE_CRLF ));

   pData->staticMemUsage  = staticMemUsageTotal;
   pData->dynamicMemUsage = dynamicMemUsageTotal;

   return nRet;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

#ifdef INCLUDE_DSL_CPE_API_VINAX
DSL_Error_t DSL_CPE_LowLevelConfigurationCheck(
   DSL_int_t fd)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_int_t ret = 0, i = 0;
   DSL_VersionInformation_t verInf;
   DSL_LowLevelConfiguration_t llCfg;
   DSL_VNX_FwVersion_t FwVersion = {0};
   DSL_char_t seps[] = ".";
   DSL_char_t *token;
   DSL_uint8_t *pVer = (DSL_uint8_t*)&FwVersion;
   DSL_boolean_t bCfgChanged = DSL_FALSE;

   /* Get version info*/
   ret = DSL_CPE_Ioctl (fd, DSL_FIO_VERSION_INFORMATION_GET, (int) &verInf);

   if ((ret < 0) && (verInf.accessCtl.nReturn < DSL_SUCCESS))
   {
      return DSL_ERROR;
   }

   if (strcmp (verInf.data.DSL_ChipSetFWVersion, "n/a") == 0)
   {
      /* No FW version available*/
      return DSL_SUCCESS;
   }
   else
   {
      /* Get first token */
      token = strtok (verInf.data.DSL_ChipSetFWVersion, seps);
      if (token != DSL_NULL)
      {
         for(i = 0; i < 6; i++)
         {
            DSL_CPE_sscanf (token, "%u", pVer);

            /* Get next token */
            token = strtok(DSL_NULL, seps);

            /* Exit scanning if no further information is included */
            if (token == DSL_NULL)
            {
               break;
            }
            pVer++;
         }

         if (i < 5)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
               (DSL_CPE_PREFIX "ERROR - Inconsistent FW version received!" DSL_CPE_CRLF));

            return DSL_ERROR;
         }
      }
   }

   /* Clean LL cfg structure*/
   memset(&llCfg, 0, sizeof(DSL_LowLevelConfiguration_t));

   /* Get LL configuration from the driver*/
   ret = DSL_CPE_Ioctl (fd, DSL_FIO_LOW_LEVEL_CONFIGURATION_GET, (int) &llCfg);

   if ((ret < 0) && (llCfg.accessCtl.nReturn < DSL_SUCCESS))
   {
      return DSL_ERROR;
   }

   while(1)
   {
      /* Check Hybrid type*/
      if ((FwVersion.nApplication == 2) && (FwVersion.nFeatureSet < 10))
      {
         if (llCfg.data.nHybrid != DSL_DEV_HYBRID_AD1_138_17_CPE_R2)
         {
            /* Set default Hybrid type*/
            llCfg.data.nHybrid = DSL_DEV_HYBRID_AD1_138_17_CPE_R2;
            nErrCode = DSL_WRN_CHECK_HYBRID_CONFIGURATION;
            /* Cfg changed*/
            bCfgChanged = DSL_TRUE;

            DSL_CCA_DEBUG(DSL_CCA_DBG_WRN,
               (DSL_CPE_PREFIX "WARNING - Setting default Hybrid (FW value = 0x10) type "
                               "for the FW feature set < 10" DSL_CPE_CRLF));

            break;
         }
      }

      /* Future checks if necessary*/

      break;
   }

   /* Check for the changed LL configuration*/
   if (bCfgChanged)
   {
      /* Set updated LL configuration*/
      ret = DSL_CPE_Ioctl (fd, DSL_FIO_LOW_LEVEL_CONFIGURATION_SET, (int) &llCfg);

      if ((ret < 0) && (llCfg.accessCtl.nReturn < DSL_SUCCESS))
      {
         return DSL_ERROR;
      }
   }

   return nErrCode;
}
#endif /* INCLUDE_DSL_CPE_API_VINAX*/

DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_InitReadyHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s" DSL_CPE_CRLF ,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)));
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d" DSL_CPE_CRLF ,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), nDevice);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}

DSL_CPE_STATIC DSL_int_t DSL_CPE_Event_S_AutobootStatusHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t fd,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   DSL_CPE_ScriptSection_t searchSection;
   DSL_AutobootStatus_t status;
   DSL_AutobootStatusData_t StatusData;
   DSL_AutobootControl_t control;
   DSL_int_t ret = 0;
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */

   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nStatus=%d nFirmwareRequestType=%d" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.pData->autobootStatus.nStatus, pEvent->data.pData->autobootStatus.nFirmwareRequestType);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice%d nStatus=%d nFirmwareRequestType=%d" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.pData->autobootStatus.nStatus, pEvent->data.pData->autobootStatus.nFirmwareRequestType);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   memcpy(&StatusData, pEvent->data.pData, sizeof(DSL_AutobootStatusData_t));

   switch (StatusData.nStatus)
   {
   case DSL_AUTOBOOT_STATUS_CONFIG_WRITE_WAIT:
      searchSection = DSL_SCRIPT_SECTION_WAIT_FOR_CONFIFURATION;
      break;
   case DSL_AUTOBOOT_STATUS_LINK_ACTIVATE_WAIT:
      searchSection = DSL_SCRIPT_SECTION_WAIT_FOR_LINK_ACTIVATE;
      break;
   case DSL_AUTOBOOT_STATUS_RESTART_WAIT:
      searchSection = DSL_SCRIPT_SECTION_WAIT_BEFORE_RESTART;
#ifdef INCLUDE_DSL_BONDING
      /* Bonding specific handling*/
      if (DSL_CPE_BND_AutobootStatusRestartWait(fd, nDevice) < DSL_SUCCESS)
      {
         return -1;
      }
#endif /* INCLUDE_DSL_BONDING*/
      break;
   default:
      return 0;
   }

   if (pContext->nFwReqType == DSL_FW_REQUEST_ADSL)
   {
      if (g_sAdslScript != DSL_NULL)
      {
         DSL_CPE_ScriptFileParse(pContext, nDevice, g_sAdslScript, searchSection);
      }
      else
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "ADSL script not specified" DSL_CPE_CRLF));
      }
   }

#ifdef INCLUDE_DSL_CPE_API_VINAX
   if (pContext->nFwReqType == DSL_FW_REQUEST_VDSL)
   {
      if (g_sVdslScript != DSL_NULL)
      {
         DSL_CPE_ScriptFileParse(pContext, nDevice, g_sVdslScript, searchSection);
      }
      else
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
            "VDSL script not specified" DSL_CPE_CRLF));
      }
   }
#endif /* INCLUDE_DSL_CPE_API_VINAX */

   /* Get autoboot status directly after script execution*/
   memset (&status, 0, sizeof(DSL_AutobootStatus_t));

   ret = DSL_CPE_Ioctl(fd, DSL_FIO_AUTOBOOT_STATUS_GET,(DSL_int_t) &status);

   if (ret >= 0)
   {
      if (((status.data.nStatus == DSL_AUTOBOOT_STATUS_CONFIG_WRITE_WAIT)  && g_bAutoContinueWaitBeforeConfigWrite[nDevice]) ||
          ((status.data.nStatus == DSL_AUTOBOOT_STATUS_LINK_ACTIVATE_WAIT) && g_bAutoContinueWaitBeforeLinkActivation[nDevice]) ||
          ((status.data.nStatus == DSL_AUTOBOOT_STATUS_RESTART_WAIT) && g_bAutoContinueWaitBeforeRestart[nDevice]))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_WRN,
            (DSL_CPE_PREFIX "Continue autoboot handling..." DSL_CPE_CRLF));

         memset (&control, 0, sizeof(DSL_AutobootControl_t));

         control.data.nCommand = DSL_AUTOBOOT_CTRL_CONTINUE;

         ret = DSL_CPE_Ioctl (fd, DSL_FIO_AUTOBOOT_CONTROL_SET,(DSL_int_t) &control);
      }
   }
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */

   return 0;
}

#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
DSL_CPE_STATIC DSL_int_t DSL_CPE_Event_S_PmSyncHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s b15MinElapsed=%u b1DayElapsed=%u" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.pData->sync.b15MinElapsed,
         pEvent->data.pData->sync.b1DayElapsed);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d b15MinElapsed=%u b1DayElapsed=%u" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.pData->sync.b15MinElapsed,
         pEvent->data.pData->sync.b1DayElapsed);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /*INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /*INCLUDE_DSL_PM*/

DSL_CPE_STATIC DSL_int_t DSL_CPE_Event_S_LineStateHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   DSL_LineStateValue_t nLineState = DSL_LINESTATE_NOT_INITIALIZED;
#ifdef INCLUDE_SCRIPT_NOTIFICATION
   DSL_boolean_t bExec = DSL_FALSE;
#endif

   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

   nLineState = pEvent->data.pData->lineStateData.nLineState;

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s " "nLineState=%08X" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.pData->lineStateData.nLineState);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nLineState=%08X" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.pData->lineStateData.nLineState);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_SCRIPT_NOTIFICATION
   if (g_sRcScript != DSL_NULL)
   {
      if ( (nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
           (g_nPrevLineState[nDevice] != DSL_LINESTATE_SHOWTIME_TC_SYNC) )
      {
         if (DSL_CPE_SetEnv("DSL_INTERFACE_STATUS", "UP") == DSL_SUCCESS)
         {
            bExec = DSL_TRUE;
         }
      }
      else if ( (nLineState != DSL_LINESTATE_SHOWTIME_TC_SYNC) &&
                (g_nPrevLineState[nDevice] == DSL_LINESTATE_SHOWTIME_TC_SYNC) )
      {
         if (DSL_CPE_SetEnv("DSL_INTERFACE_STATUS", "DOWN") == DSL_SUCCESS)
         {
            bExec = DSL_TRUE;
         }
      }

      if (bExec != DSL_FALSE)
      {
         if (DSL_CPE_SetEnv("DSL_NOTIFICATION_TYPE", "DSL_INTERFACE_STATUS") == DSL_SUCCESS)
         {
            DSL_CPE_ScriptRun();
         }
      }
   }
#endif

#ifdef INCLUDE_DSL_BONDING
   if ( DSL_CPE_BND_LineStateHandle(
           (DSL_CPE_BND_Context_t *)pContext->pBnd,
           pContext->fd[nDevice], nDevice, nLineState, g_nPrevLineState[nDevice]) < DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "BND - Line State handling failed!" DSL_CPE_CRLF));
   }
#endif /* INCLUDE_DSL_BONDING*/

   g_nPrevLineState[nDevice] = nLineState;

   return 0;
}

#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_SystemInterfaceStatusHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_SCRIPT_NOTIFICATION
   if (g_sRcScript != DSL_NULL)
   {
      if (DSL_CPE_SetEnv("DSL_NOTIFICATION_TYPE", "DSL_STATUS") == DSL_SUCCESS)
      {
         nErrCode = DSL_CPE_SetEnv("DSL_XTU_STATUS", "ADSL");
         if (nErrCode == DSL_SUCCESS)
         {
            switch (pEvent->data.pData->systemInterfaceStatusData.nTcLayer)
            {
               case DSL_TC_ATM:
                  nErrCode = DSL_CPE_SetEnv("DSL_TC_LAYER_STATUS", "ATM");
                  break;
               case DSL_TC_EFM:
               case DSL_TC_EFM_FORCED:
                  nErrCode = DSL_CPE_SetEnv("DSL_TC_LAYER_STATUS", "EFM");
                  break;
               default:
                  nErrCode = DSL_CPE_SetEnv("DSL_TC_LAYER_STATUS", "UNKNOWN");
                  break;
            }
         }

         if (nErrCode == DSL_SUCCESS)
         {
            switch (pEvent->data.pData->systemInterfaceStatusData.nEfmTcConfigUs)
            {
               case DSL_EMF_TC_NORMAL:
                  nErrCode = DSL_CPE_SetEnv("DSL_EFM_TC_CONFIG_US", "NORMAL");
                  break;
               case DSL_EMF_TC_PRE_EMPTION:
                  nErrCode = DSL_CPE_SetEnv("DSL_EFM_TC_CONFIG_US", "PRE_EMPTION");
                  break;
               default:
                  nErrCode = DSL_CPE_SetEnv("DSL_EFM_TC_CONFIG_US", "UNKNOWN");
                  break;
            }
         }

         if (nErrCode == DSL_SUCCESS)
         {
            switch (pEvent->data.pData->systemInterfaceStatusData.nEfmTcConfigDs)
            {
               case DSL_EMF_TC_NORMAL:
                  nErrCode = DSL_CPE_SetEnv("DSL_EFM_TC_CONFIG_DS", "NORMAL");
                  break;
               default:
                  nErrCode = DSL_CPE_SetEnv("DSL_EFM_TC_CONFIG_DS", "UNKNOWN");
                  break;
            }
         }

         if (nErrCode == DSL_SUCCESS)
         {
            DSL_CPE_ScriptRun();
         }
      }
   }
#endif

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nTcLayer=%d nEfmTcConfigUs=0x%08X nEfmTcConfigDs=0x%08X "
         "nSystemIf=%d" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.pData->systemInterfaceStatusData.nTcLayer,
         pEvent->data.pData->systemInterfaceStatusData.nEfmTcConfigUs,
         pEvent->data.pData->systemInterfaceStatusData.nEfmTcConfigDs,
         pEvent->data.pData->systemInterfaceStatusData.nSystemIf);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nTcLayer=%d nEfmTcConfigUs=0x%08X nEfmTcConfigDs=0x%08X "
         "nSystemIf=%d" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.pData->systemInterfaceStatusData.nTcLayer,
         pEvent->data.pData->systemInterfaceStatusData.nEfmTcConfigUs,
         pEvent->data.pData->systemInterfaceStatusData.nEfmTcConfigDs,
         pEvent->data.pData->systemInterfaceStatusData.nSystemIf);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return nErrCode;
}
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/

#ifdef INCLUDE_DSL_G997_ALARM
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_I_DataPathFailuresHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nChannel=%d nAccessDir=%s nDataPathFailures=%08X" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->dataPathFailuresData.nDataPathFailures);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nChannel=%d nAccessDir=%s nDataPathFailures=%08X" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->dataPathFailuresData.nDataPathFailures);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_I_LineFailuresHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nChannel=%d nAccessDir=%s nLineFailures=%08X" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->lineFailuresData.nLineFailures);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nChannel=%d nAccessDir=%s nLineFailures=%08X" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->lineFailuresData.nLineFailures);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_ChannelDataRateHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
#ifdef INCLUDE_SCRIPT_NOTIFICATION
   DSL_char_t sVarName[20];
   DSL_char_t sVarVal[11];
#endif
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_SCRIPT_NOTIFICATION
   if (g_sRcScript != DSL_NULL)
   {
      if (DSL_CPE_SetEnv("DSL_NOTIFICATION_TYPE",
                         pEvent->data.nAccessDir == DSL_DOWNSTREAM ?
                         "DSL_DATARATE_STATUS_DS" : "DSL_DATARATE_STATUS_US") == DSL_SUCCESS)
      {
         sprintf(sVarName, "DSL_DATARATE_%s_BC%d",
            pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DS" : "US",
            pEvent->data.nChannel == 0 ? 0 : pEvent->data.nChannel == 1 ? 1 : 0);
         sprintf(sVarVal, "%lu", pEvent->data.pData->channelStatusData.ActualDataRate);

         if (DSL_CPE_SetEnv(sVarName, sVarVal) == DSL_SUCCESS)
         {
            DSL_CPE_ScriptRun();
         }
      }
   }
#endif

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nChannel=%d nAccessDir=%s ActualDataRate=%d PreviousDataRate=%d "
         "ActualInterleaveDelay=%d ActualImpulseNoiseProtection=%d" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->channelStatusData.ActualDataRate,
         pEvent->data.pData->channelStatusData.PreviousDataRate,
         pEvent->data.pData->channelStatusData.ActualInterleaveDelay,
         pEvent->data.pData->channelStatusData.ActualImpulseNoiseProtection);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nChannel=%d nAccessDir=%s ActualDataRate=%d PreviousDataRate=%d "
         "ActualInterleaveDelay=%d ActualImpulseNoiseProtection=%d" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->channelStatusData.ActualDataRate,
         pEvent->data.pData->channelStatusData.PreviousDataRate,
         pEvent->data.pData->channelStatusData.ActualInterleaveDelay,
         pEvent->data.pData->channelStatusData.ActualImpulseNoiseProtection);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}

#ifdef INCLUDE_DSL_G997_ALARM
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_ChannelDataRateShiftThresholdCrossingHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL || pContext == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nChannel=%d nAccessDir=%s"
         " nDataRateThresholdType=DSL_G997_DATARATE_THRESHOLD_%sSHIFT" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->dataRateThresholdCrossing.nDataRateThresholdType ==
         DSL_G997_DATARATE_THRESHOLD_DOWNSHIFT ? "DOWN": "UP");
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nChannel=%d nAccessDir=%s"
         " nDataRateThresholdType=DSL_G997_DATARATE_THRESHOLD_%sSHIFT" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nAccessDir == DSL_DOWNSTREAM ? "DSL_DOWNSTREAM" : "DSL_UPSTREAM",
         pEvent->data.pData->dataRateThresholdCrossing.nDataRateThresholdType ==
         DSL_G997_DATARATE_THRESHOLD_DOWNSHIFT ? "DOWN": "UP");
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

#ifdef INCLUDE_DSL_G997_ALARM
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_LinitFailureHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_char_t *pStatus = DSL_NULL;
#endif

   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   switch(pEvent->data.pData->lineInitStatusData)
   {
   case LINIT_SUCCESSFUL:
      pStatus = "LINIT_SUCCESSFUL";
      break;
   case LINIT_CONFIG_ERROR:
      pStatus = "LINIT_CONFIG_ERROR";
      break;
   case LINIT_CONFIG_NOT_FEASIBLE:
      pStatus = "LINIT_CONFIG_NOT_FEASIBLE";
      break;
   case LINIT_COMMUNICATION_PROBLEM:
      pStatus = "LINIT_COMMUNICATION_PROBLEM";
      break;
   case LINIT_NO_PEER_XTU:
      pStatus = "LINIT_NO_PEER_XTU";
      break;
   case LINIT_UNKNOWN:
      pStatus = "LINIT_UNKNOWN";
      break;

   default:
      pStatus = "LINIT_UNKNOWN";
      break;
   }

   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s lineInitStatusData=%s" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), pStatus);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d lineInitStatusData=%s" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), nDevice, pStatus);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /* INCLUDE_DSL_G997_ALARM*/

DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_FirmwareRequestHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t fd,
   DSL_int_t nDevice,
   DSL_EventType_t nEvent,
   DSL_FirmwareRequestType_t nFwReqType)
{
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_uint32_t nFwLoadRetryCnt = 0;

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_char_t *pFwReqType = "DSL_FW_REQUEST_NA";

   switch (nFwReqType)
   {
      case DSL_FW_REQUEST_ADSL:
         pFwReqType = "DSL_FW_REQUEST_ADSL";
         break;
      case DSL_FW_REQUEST_VDSL:
         pFwReqType = "DSL_FW_REQUEST_VDSL";
         break;
      case DSL_FW_REQUEST_NA:
      default:
         pFwReqType = "DSL_FW_REQUEST_NA";
         break;
   }

   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "sEventType=%s nFirmwareRequestType=%s"DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&nEvent), pFwReqType);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "sEventType=%s nDevice=%d nFirmwareRequestType=%s" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&nEvent), nDevice, pFwReqType);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/

#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   /* Try to reload FW several times in case of any fail*/
   for (nFwLoadRetryCnt = 0; nFwLoadRetryCnt < DSL_CPE_MAX_FW_RELOAD_RETRY_COUNT; nFwLoadRetryCnt++)
   {
      nRet = DSL_CPE_DownloadFirmware(fd, nFwReqType, DSL_NULL, DSL_NULL);

      if (nRet >= DSL_SUCCESS)
      {
         break;
      }
   }

   if (nRet != DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - FW download failed on the %d retry!"DSL_CPE_CRLF,
         DSL_CPE_MAX_FW_RELOAD_RETRY_COUNT));
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_CRLF));

   return 0;
}

DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_FirmwareDownloadStatusHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventType_t nEvent,
   DSL_FwDownloadStatusData_t nFwDwnlStatus)
{
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "sEventType=%s nError=%d nFwType=%d",
         DSL_CPE_Event_Type2String(&nEvent),
         nFwDwnlStatus.nError,
         nFwDwnlStatus.nFwType);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "sEventType=%s nDevice=%d nError=%d nFwType=%d",
         DSL_CPE_Event_Type2String(&nEvent),
         nDevice,
         nFwDwnlStatus.nError,
         nFwDwnlStatus.nFwType);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   if (nFwDwnlStatus.nError == DSL_FW_LOAD_SUCCESS)
   {
      pContext->nFwReqType = nFwDwnlStatus.nFwType;
   }
   else
   {
      pContext->nFwReqType = DSL_FW_REQUEST_NA;
   }
#endif

   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_CRLF));

   return 0;
}

#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_FeInventoryAvailableHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t fd,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   DSL_G997_LineInventory_t lineInventory;
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_char_t buf[256];
#endif
   DSL_int_t ret = 0;

   if( pEvent == DSL_NULL )
   {
      return DSL_ERROR;
   }

   memset(&lineInventory, 0x00, sizeof(DSL_G997_LineInventory_t));
   /* Specify direction */
   lineInventory.nDirection = DSL_FAR_END;
   ret = DSL_CPE_Ioctl(fd, DSL_FIO_G997_LINE_INVENTORY_GET,
      (DSL_int_t) &lineInventory);

   if( ret < 0 )
   {
      if (lineInventory.accessCtl.nReturn < DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "FE line inventory get failed!, ret = %d!"DSL_CPE_CRLF,
            lineInventory.accessCtl.nReturn));
         return -1;
      }
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "sEventType=%s ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)));
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "sEventType=%s nDevice=%d ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), nDevice);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/

   DSL_CPE_ArraySPrintF(buf, lineInventory.data.G994VendorID,
      sizeof(lineInventory.data.G994VendorID),
      sizeof(lineInventory.data.G994VendorID[0]),
      DSL_ARRAY_FORMAT_HEX);
   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%sG994VendorID=%s ",
      CLI_EventText, buf);

   DSL_CPE_ArraySPrintF(buf, lineInventory.data.SystemVendorID,
      sizeof(lineInventory.data.SystemVendorID),
      sizeof(lineInventory.data.SystemVendorID[0]),
      DSL_ARRAY_FORMAT_HEX);
   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%sSystemVendorID=%s ",
      CLI_EventText, buf);

   DSL_CPE_ArraySPrintF(buf, lineInventory.data.VersionNumber,
      sizeof(lineInventory.data.VersionNumber),
      sizeof(lineInventory.data.VersionNumber[0]),
      DSL_ARRAY_FORMAT_HEX);
   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%sVersionNumber=%s ",
      CLI_EventText, buf);

   DSL_CPE_ArraySPrintF(buf, lineInventory.data.SerialNumber,
      sizeof(lineInventory.data.SerialNumber),
      sizeof(lineInventory.data.SerialNumber[0]),
      DSL_ARRAY_FORMAT_HEX);
   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%sSerialNumber=%s ",
      CLI_EventText, buf);

   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%sSelfTestResult=%u ",
      CLI_EventText, lineInventory.data.SelfTestResult);

   DSL_CPE_ArraySPrintF(buf, lineInventory.data.XTSECapabilities,
      sizeof(lineInventory.data.XTSECapabilities),
      sizeof(lineInventory.data.XTSECapabilities[0]),
      DSL_ARRAY_FORMAT_HEX);
   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%sXTSECapabilities=%s ",
      CLI_EventText, buf);

   DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%s" DSL_CPE_CRLF, CLI_EventText);
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/

DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_LinePowerMgmtStateHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_char_t *pStatus = DSL_NULL;
#endif

   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL || pContext == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   switch(pEvent->data.pData->powerMgmtStatusData.nPowerManagementStatus)
   {
   case DSL_G997_PMS_L0:
      pStatus = "DSL_G997_PMS_L0";
      break;
   case DSL_G997_PMS_L1:
      pStatus = "DSL_G997_PMS_L1";
      break;
   case DSL_G997_PMS_L2:
      pStatus = "DSL_G997_PMS_L2";
      break;
   case DSL_G997_PMS_L3:
      pStatus = "DSL_G997_PMS_L3";
      break;

   default:
      pStatus = "DSL_G997_PMS_NA";
      break;
   }

   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nPowerManagementStatus=%s" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), pStatus);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d nPowerManagementStatus=%s" DSL_CPE_CRLF,
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), nDevice, pStatus);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}

#if defined(INCLUDE_DSL_CEOC)
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_S_SnmpMessageAvailableHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pContext == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)));
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s nDevice=%d",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)), nDevice);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif

#if defined(INCLUDE_DSL_PM)
#if defined(INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS)
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_I_ChannelThresholdCrossingHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL || pContext == DSL_NULL)
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->channelThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->channelThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->channelThresholdCrossing.n15Min,
         pEvent->data.pData->channelThresholdCrossing.n1Day);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nDevice=%d "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->channelThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->channelThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->channelThresholdCrossing.n15Min,
         pEvent->data.pData->channelThresholdCrossing.n1Day);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /** #if defined(INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS)*/

DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_I_LineThresholdCrossingHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->lineThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->lineThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->lineThresholdCrossing.n15Min,
         pEvent->data.pData->lineThresholdCrossing.n1Day);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nDevice=%d "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->lineThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->lineThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->lineThresholdCrossing.n15Min,
         pEvent->data.pData->lineThresholdCrossing.n1Day);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}

#if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS)
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_I_DataPathThresholdCrossingHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->dataPathThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->dataPathThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->dataPathThresholdCrossing.n15Min,
         pEvent->data.pData->dataPathThresholdCrossing.n1Day);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nDevice=%d "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->dataPathThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->dataPathThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->dataPathThresholdCrossing.n15Min,
         pEvent->data.pData->dataPathThresholdCrossing.n1Day);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /** #if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS)*/

#if defined(INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS)
DSL_CPE_STATIC  DSL_int_t DSL_CPE_Event_I_ReTxThresholdCrossingHandle(
   DSL_CPE_Control_Context_t *pContext,
   DSL_int_t nDevice,
   DSL_EventStatus_t *pEvent)
{
   if( pEvent == DSL_NULL || pEvent->data.pData == DSL_NULL )
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (pContext->bBackwardCompMode)
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->reTxThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->reTxThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->reTxThresholdCrossing.n15Min,
         pEvent->data.pData->reTxThresholdCrossing.n1Day);
   }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
   else
   {
      DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
         "sEventType=%s "
         "nDevice=%d "
         "nChannel=%d "
         "nATUDir=%s "
         "nCurr15MinTime=%d "
         "nCurr1DayTime=%d "
         "n15Min=0x%08X "
         "n1day=0x%08X ",
         DSL_CPE_Event_Type2String(&(pEvent->data.nEventType)),
         nDevice,
         pEvent->data.nChannel,
         pEvent->data.nXtuDir == DSL_NEAR_END ? "DSL_NEAR_END" : "DSL_FAR_END",
         pEvent->data.pData->reTxThresholdCrossing.nCurr15MinTime,
         pEvent->data.pData->reTxThresholdCrossing.nCurr1DayTime,
         pEvent->data.pData->reTxThresholdCrossing.n15Min,
         pEvent->data.pData->reTxThresholdCrossing.n1Day);
   }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   return 0;
}
#endif /** #if defined(INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS)*/

#endif /** #if defined(INCLUDE_DSL_PM)*/

DSL_int_t DSL_CPE_EventHandler (DSL_CPE_Thread_Params_t *param)
{
   DSL_CPE_Control_Context_t *pContext;
#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   DSL_CPE_File_t *file = DSL_NULL;
#endif /* INCLUDE_DSL_CPE_TRACE_BUFFER*/
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT */
#ifndef WIN32
#ifndef INCLUDE_DSL_EVENT_POLLING
   DSL_int_t sretval;
   DSL_CPE_fd_set_t rd_fds;               /* File descr. sets for use with select() */
#endif /* #ifndef INCLUDE_DSL_EVENT_POLLING*/
#endif /* WIN32*/
   DSL_int_t ret, nDevice = 0;
   DSL_int_t fd;
   DSL_EventStatus_t event;
#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
   DSL_ShowtimeLogging_t showtimeLogging;
   DSL_uint16_t showtimeData[120];
#endif /* INCLUDE_DSL_CPE_TRACE_BUFFER*/
#if (defined (INCLUDE_DSL_CPE_CLI_SUPPORT) || defined (INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT)) && \
    defined(INCLUDE_DSL_CPE_TRACE_BUFFER)
   DSL_int_t i;
#endif

/*   DSL_EventData_Union_t eventData;*/

   pContext = (DSL_CPE_Control_Context_t *) param->nArg1;
   if (pContext == DSL_NULL)
   {
      return DSL_ERROR;
   }

   /*
      The main fd read cycle starts here.
   */
   pContext->bEvtRun = DSL_TRUE;
   for (;;)
   {
      if (pContext->bRun == DSL_FALSE || pContext->bEvtRun == DSL_FALSE)
      {
         break;
      }

#ifdef INCLUDE_DSL_EVENT_POLLING
      DSL_CPE_MSecSleep(1000);
#else
#if !defined(RTEMS) && !defined(WIN32)
      FD_ZERO (&rd_fds);

      if (pContext->bBackwardCompMode)
      {
         DSL_CPE_FD_SET (pContext->fd[pContext->nDevNum], &rd_fds);
      }
      else
      {
         for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
         {
            DSL_CPE_FD_SET (pContext->fd[nDevice], &rd_fds);
         }
      }

      sretval = DSL_CPE_Select (DSL_FD_SETSIZE, &rd_fds, NULL, DSL_DEV_TIMEOUT_SELECT);

      /* error or timeout on select */
      if (sretval <= 0)
      {
         continue;
      }
#else
      #if defined(RTEMS)
      sretval = DSL_CPE_Select (gv_drv_dsl_pOpenCtx->eventWaitQueue, DSL_DEV_TIMEOUT_SELECT);

      if (sretval != 0)
      {
         /* Timeout (or Error) */
         continue;
      }
      #endif /* defined(RTEMS)*/
      DSL_CPE_MSecSleep(100);
      #if defined(WIN32)
      #endif /* defined(WIN32)*/
#endif /* !defined(RTEMS) && !defined(WIN32)*/
#endif /* INCLUDE_DSL_EVENT_POLLING*/

      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {

#if !defined(INCLUDE_DSL_EVENT_POLLING)
         /* handles a normal wakeup of the select because data is avialble from
            within the device driver */
         if (DSL_CPE_FD_ISSET (pContext->fd[nDevice], &rd_fds) == 0)
         {
            continue;
         }
#endif /* !defined(RTEMS) && !defined(WIN32)*/
         fd = pContext->fd[nDevice];

         DSL_CCA_DEBUG(DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX "EVENT from Device=%d..." DSL_CPE_CRLF,
            nDevice));

         /* get an event status */
         memset (&event, 0, sizeof (DSL_EventStatus_t));
         event.data.pData = (DSL_EventData_Union_t*)malloc(sizeof(DSL_EventData_Union_t));

         if( event.data.pData == DSL_NULL )
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX DSL_CPE_CRLF
               "Can't allocate memory for event data!!!"DSL_CPE_CRLF));
            continue;
         }

         /* Get Event Status*/
         ret = DSL_CPE_Ioctl (fd, DSL_FIO_EVENT_STATUS_GET, (DSL_int_t) & event);

         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
               "Could not get device driver event status, ret = %d!" DSL_CPE_CRLF, ret));
            if (event.data.pData != DSL_NULL)
            {
               free(event.data.pData);
            }
            continue;
         }

         if (event.accessCtl.nReturn != DSL_SUCCESS)
         {
            if (event.accessCtl.nReturn < DSL_SUCCESS)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
                  "Error code was returned by driver IOCTL errCode= %d!" DSL_CPE_CRLF,
                  event.accessCtl.nReturn));
            }

            /* Check for the FIFO overflow condition*/
            if (event.accessCtl.nReturn == DSL_WRN_EVENT_FIFO_OVERFLOW)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX DSL_CPE_CRLF
                  "Event FIFO overflow occured!" DSL_CPE_CRLF));
            }
            else
            {
               if (event.data.pData != DSL_NULL)
               {
                  free(event.data.pData);
               }
               continue;
            }
         }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
         CLI_EventText[0] = '\0';
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

         switch (event.data.nEventType)
         {
         case DSL_EVENT_S_INIT_READY:
            DSL_CPE_Event_S_InitReadyHandle(pContext, nDevice, &event);
            break;
#ifdef INCLUDE_DSL_CPE_TRACE_BUFFER
         case DSL_EVENT_S_SHOWTIME_LOGGING:
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
            if (pContext->bBackwardCompMode)
            {
               DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
                  "sEventType=%s",
                  DSL_CPE_Event_Type2String(&(event.data.nEventType)));
            }
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
            else
            {
               DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText),
                  "sEventType=%s nDevice=%d",
                  DSL_CPE_Event_Type2String(&(event.data.nEventType)), nDevice);
            }
#endif /* #if (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
            memset (&showtimeLogging, 0, sizeof (DSL_ShowtimeLogging_t));
            memset (&showtimeData, 0, sizeof (showtimeData));
            showtimeLogging.data.nDataSize = sizeof (showtimeData);
            showtimeLogging.data.pData = (DSL_uint8_t *) showtimeData;

            ret =
               DSL_CPE_Ioctl (fd, DSL_FIO_SHOWTIME_LOGGING_DATA_GET,
               (DSL_int_t) & showtimeLogging);
            if (ret < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
                  "Could not get showtime info, ret = %d!" DSL_CPE_CRLF, ret));
               continue;
            }

            if (showtimeLogging.accessCtl.nReturn != DSL_SUCCESS)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
                  "Error code was returned by driver IOCTL while trying to get "
                  "showtimeLogging data, errCode = %d!" DSL_CPE_CRLF,
                  showtimeLogging.accessCtl.nReturn));
               continue;
            }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
            DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%s nData=\"",
               CLI_EventText);

            for (i = 0; i < (DSL_int_t)(sizeof (showtimeData) / 2); i++)
            {
               if (i % 10 == 0)
               {
                  DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%s" DSL_CPE_CRLF,
                     CLI_EventText);
               }
               DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%s0x%04X ",
                  CLI_EventText, showtimeData[i]);
            }
            DSL_CPE_snprintf(CLI_EventText, sizeof(CLI_EventText), "%s" DSL_CPE_CRLF "\"",
               CLI_EventText);
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
            file = DSL_CPE_FOpen(DSL_CPE_SHOWTIME_EVENT_LOGGING_FILENAME,"w");
            if (file == DSL_NULL)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX DSL_CPE_CRLF
                  "Could not open file (%s) for showtime event logging data!!!"
                  DSL_CPE_CRLF, DSL_CPE_SHOWTIME_EVENT_LOGGING_FILENAME));
            }
            else
            {
               DSL_CPE_FPrintf(file, "showtime event logging data:");
               for (i = 0; i < (DSL_int_t)(sizeof (showtimeData) / 2); i++)
               {
                  if (i % 10 == 0) DSL_CPE_FPrintf (file, DSL_CPE_CRLF);
                  DSL_CPE_FPrintf (file, "0x%04X ", showtimeData[i]);
               }
               DSL_CPE_FPrintf (file, DSL_CPE_CRLF DSL_CPE_CRLF);

               DSL_CPE_FClose(file);
            }
#endif
            break;
#endif /* INCLUDE_DSL_CPE_TRACE_BUFFER*/
         case DSL_EVENT_S_FIRMWARE_REQUEST:
            DSL_CPE_Event_S_FirmwareRequestHandle(
               pContext, fd, nDevice,
               event.data.nEventType,
               event.data.pData->fwRequestData.nFirmwareRequestType);
            break;
         case DSL_EVENT_S_FIRMWARE_DOWNLOAD_STATUS:
            DSL_CPE_Event_S_FirmwareDownloadStatusHandle(
               pContext, nDevice,
               event.data.nEventType,
               event.data.pData->fwDownloadStatus);
            break;
#ifdef INCLUDE_DSL_G997_ALARM
         case DSL_EVENT_I_DATA_PATH_FAILURES:
            DSL_CPE_Event_I_DataPathFailuresHandle(pContext, nDevice, &event);
            break;
         case DSL_EVENT_I_LINE_FAILURES:
            DSL_CPE_Event_I_LineFailuresHandle(pContext, nDevice, &event);
            break;
#endif /* INCLUDE_DSL_G997_ALARM*/
         case DSL_EVENT_S_LINE_STATE:
            DSL_CPE_Event_S_LineStateHandle(pContext, nDevice, &event);
            break;
#ifdef INCLUDE_DSL_PM
#ifdef INCLUDE_DSL_CPE_PM_HISTORY
         case DSL_EVENT_S_PM_SYNC:
            DSL_CPE_Event_S_PmSyncHandle(pContext, nDevice, &event);
            break;
#endif /* INCLUDE_DSL_CPE_PM_HISTORY*/
#endif /* INCLUDE_DSL_PM*/
         case DSL_EVENT_S_AUTOBOOT_STATUS:
            DSL_CPE_Event_S_AutobootStatusHandle(pContext, fd, nDevice, &event);
            break;
#ifdef INCLUDE_DSL_SYSTEM_INTERFACE
         case DSL_EVENT_S_SYSTEM_INTERFACE_STATUS:
            DSL_CPE_Event_S_SystemInterfaceStatusHandle(pContext, nDevice, &event);
            break;
#endif /* INCLUDE_DSL_SYSTEM_INTERFACE*/
         case DSL_EVENT_S_CHANNEL_DATARATE:
            DSL_CPE_Event_S_ChannelDataRateHandle(pContext, nDevice, &event);
            break;
#ifdef INCLUDE_DSL_G997_ALARM
         case DSL_EVENT_I_CHANNEL_DATARATE_SHIFT_THRESHOLD_CROSSING:
            DSL_CPE_Event_S_ChannelDataRateShiftThresholdCrossingHandle(pContext, nDevice, &event);
            break;
#endif /* INCLUDE_DSL_G997_ALARM*/
#ifdef INCLUDE_DSL_G997_ALARM
         case DSL_EVENT_S_LINIT_FAILURE:
            DSL_CPE_Event_S_LinitFailureHandle(pContext, nDevice, &event);
            break;
#endif /* INCLUDE_DSL_G997_ALARM*/
#ifdef INCLUDE_DSL_G997_LINE_INVENTORY
         case DSL_EVENT_S_FE_INVENTORY_AVAILABLE:
            DSL_CPE_Event_S_FeInventoryAvailableHandle(pContext, fd, nDevice, &event);
            break;
#endif /* INCLUDE_DSL_G997_LINE_INVENTORY*/
         case DSL_EVENT_S_LINE_POWERMANAGEMENT_STATE:
            DSL_CPE_Event_S_LinePowerMgmtStateHandle(pContext, nDevice, &event);
            break;
#if defined(INCLUDE_DSL_CEOC)
         case DSL_EVENT_S_SNMP_MESSAGE_AVAILABLE:
            DSL_CPE_Event_S_SnmpMessageAvailableHandle(pContext, nDevice, &event);
            break;
#endif
#if defined(INCLUDE_DSL_PM)
         #if defined(INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS)
         case DSL_EVENT_I_CHANNEL_THRESHOLD_CROSSING:
            DSL_CPE_Event_I_ChannelThresholdCrossingHandle(pContext, nDevice, &event);
            break;
         #endif /** #if defined(INCLUDE_DSL_CPE_PM_CHANNEL_THRESHOLDS)*/
         case DSL_EVENT_I_LINE_THRESHOLD_CROSSING:
            DSL_CPE_Event_I_LineThresholdCrossingHandle(pContext, nDevice, &event);
            break;
         #if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS)
         case DSL_EVENT_I_DATA_PATH_THRESHOLD_CROSSING:
            DSL_CPE_Event_I_DataPathThresholdCrossingHandle(pContext, nDevice, &event);
            break;
         #endif /** #if defined(INCLUDE_DSL_CPE_PM_DATA_PATH_THRESHOLDS)*/
         #if defined(INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS)
         case DSL_EVENT_I_RETX_THRESHOLD_CROSSING:
            DSL_CPE_Event_I_ReTxThresholdCrossingHandle(pContext, nDevice, &event);
            break;
         #endif /** #if defined(INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS)*/
#endif /** #if defined(INCLUDE_DSL_PM)*/
         default:
            DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX DSL_CPE_CRLF
               "unhandled event with type: %d" DSL_CPE_CRLF, event.data.nEventType));
            break;
         }

         if (event.data.pData != DSL_NULL)
         {
            free(event.data.pData);
         }
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
         /* Do not handle if not string id given */
         if (CLI_EventText[0] != '\0')
         {
            DSL_CPE_CLI_HandleEvent(CLI_EventText);
         }
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */
      }
   }                            /* for */

   printf ("EventHandler: exit(0)\n");
   return 0;
}

/**
   Start Event handler thread
*/
DSL_Error_t DSL_CPE_EventHandlerStart (
   DSL_CPE_Control_Context_t * pContext
)
{
   if (DSL_CPE_ThreadInit (&pContext->EventControl, "evnthnd",
      DSL_CPE_EventHandler, DSL_CPE_EVENT_STACKSIZE, DSL_CPE_PRIORITY,
      (DSL_uint32_t) pContext, 0) == 0)
   {
      return DSL_SUCCESS;
   }

   printf ("Event thread start error\n");

   return DSL_ERROR;
}

DSL_Error_t DSL_CPE_WhatStringGet(
   DSL_char_t *psFirmwareName,
   const DSL_uint32_t nMaxStringLength,
   DSL_char_t *pString)
{
   DSL_Error_t errorCode = DSL_SUCCESS;
   DSL_uint32_t i = 0;
   DSL_uint32_t str_len = nMaxStringLength - 1;
   DSL_uint8_t *ptr = DSL_NULL, *pChunk = DSL_NULL;
   DSL_uint32_t rd_sz, bin_len;
   DSL_boolean_t bFound = DSL_FALSE, bEnd = DSL_FALSE;
#ifndef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
   DSL_CPE_File_t *fd_image = DSL_NULL;
   DSL_CPE_stat_t file_stat;
   DSL_uint32_t chunkSz = 0, seek = 0;
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/

   if((psFirmwareName == DSL_NULL) || (pString == DSL_NULL) || (nMaxStringLength < 2))
   {
      return DSL_ERROR;
   }

#ifdef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
   bin_len = FIRMWARE_SIZE;
   rd_sz   = bin_len - 4;

   if (bin_len < 4)
   {
      return DSL_ERROR;
   }

   ptr = cgi_pFileData_modemfw_bin;
#else
   fd_image = DSL_CPE_FOpen (psFirmwareName, "rb");
   if (fd_image == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "open %s fail" DSL_CPE_CRLF, psFirmwareName));
      return DSL_ERROR;
   }

   DSL_CPE_FStat (psFirmwareName, &file_stat);

   bin_len = file_stat.st_size;
   chunkSz = bin_len > WHAT_STRING_CHUNK_LEN ? WHAT_STRING_CHUNK_LEN : bin_len;
   rd_sz   = chunkSz - 4;

   if (bin_len < 4)
   {
      DSL_CPE_FClose (fd_image);
      return DSL_ERROR;
   }

   pChunk = DSL_CPE_Malloc((DSL_uint32_t)chunkSz);
   if (pChunk == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX "Memory allocation failed for WHAT string chunk!"
         DSL_CPE_CRLF));
      return DSL_ERROR;
   }
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/

   while (bin_len > 4)
   {
#ifndef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
      if (DSL_CPE_FRead (pChunk, sizeof (DSL_uint8_t), chunkSz, fd_image) <= 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "read %s failed!"DSL_CPE_CRLF, psFirmwareName));
         errorCode = DSL_ERROR;
         break;
      }
      ptr = pChunk;
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/

      while(rd_sz)
      {
         if(*ptr == '@' && !bFound)
         {
            if (ptr[1] == '(' && ptr[2] == '#' && ptr[3] == ')')
            {
               bFound = DSL_TRUE;
               #ifndef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
               break;
               #endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/
            }
         }

         if (bFound)
         {
            ptr   += 4;
            rd_sz -= 4;

            /* found what string */
            while(str_len && *ptr && (rd_sz))
            {
               if(*ptr != ' ')
               {
                  pString[i++] = *ptr;
                  str_len--;
               }

               ptr++;
               rd_sz--;
            }

            bEnd = DSL_TRUE;
            break;
         }

         ptr++;
         rd_sz--;
         bin_len--;
      }

      if (bEnd)
      {
         break;
      }
#ifndef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE

      seek += (chunkSz - rd_sz - 4);
      if (fseek(fd_image, seek, SEEK_SET))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "fseek %s fail" DSL_CPE_CRLF, psFirmwareName));
         errorCode = DSL_ERROR;
         break;
      }

      chunkSz = bin_len > DSL_CPE_FW_CHUNK_SIZE ? DSL_CPE_FW_CHUNK_SIZE : bin_len;

      if (chunkSz <= 4)
      {
         break;
      }

      rd_sz = chunkSz - 4;

#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/
   }

   pString[i] = 0;

   if(i)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_PRN,
         (DSL_CPE_PREFIX"ADSL Firmware WHAT String %s"DSL_CPE_CRLF, pString));
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_PRN,
         (DSL_CPE_PREFIX"ADSL Firmware WHAT String not detected"DSL_CPE_CRLF));
   }

#ifndef INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE
   /* Close FW binary*/
   if (fd_image != DSL_NULL)
   {
      DSL_CPE_FClose (fd_image);
   }

   /* Free Chunk data*/
   if (pChunk != DSL_NULL)
   {
      DSL_CPE_Free(pChunk);
   }
#endif /* INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE*/

   return errorCode;
}

DSL_Error_t DSL_CPE_FwApplicationFromWhatstringGet(
   DSL_char_t *pWhatString,
   DSL_int_t *pFwApplication)
{
   DSL_Error_t errorCode = DSL_SUCCESS;
   DSL_char_t  *pWhat = DSL_NULL;
   DSL_uint32_t i, nValue = 0;

   if (!pWhatString || !pFwApplication)
   {
      return DSL_ERROR;
   }

   /* Set undefined FW application type*/
   *pFwApplication = -1;

   pWhat = pWhatString;

   for (i=0; i<6; i++)
   {
      if (*pWhat == '\0')
      {
         break;
      }

      /* expect a decimal value */
      nValue = strtoul(pWhat, &pWhat, 10);

      /* step to the next position after a '.' */
      pWhat = strpbrk(pWhat, ".");
      pWhat++;
   }

   if (i == 6)
   {
      *pFwApplication = nValue;
   }

   return(errorCode);
}

static DSL_Error_t DSL_CPE_SwVersionFromStringGet(DSL_char_t* pVerString, DSL_SwVersion_t* pVer)
{
   DSL_Error_t errorCode = DSL_SUCCESS;
   DSL_char_t* pVerStringCopy = NULL, *pStopstr = DSL_NULL;
   DSL_char_t* token = NULL;
   DSL_uint8_t i, verPart = 0;
   DSL_int_t verPartVal = 0;

   if (pVerString == DSL_NULL || pVer == DSL_NULL)
   {
      return DSL_ERROR;
   }

   pVer->nMajor       = -1;
   pVer->nMinor       = -1;
   pVer->nDevelopment = -1;
   pVer->nMaintenance = -1;

   pVerStringCopy = (DSL_char_t*)malloc((strlen(pVerString) + 1));

   if (pVerStringCopy == DSL_NULL)
   {
      return DSL_ERROR;
   }

   strcpy(pVerStringCopy, pVerString);

   token = strtok(pVerStringCopy, ".");

   while (token != DSL_NULL)
   {
      for(i = 0; i < strlen(token); ++i)
      {
         if (! (token[i] >= '0' && token[i] <= '9') )
         {
            pVer->nMajor       = -1;
            pVer->nMinor       = -1;
            pVer->nDevelopment = -1;
            pVer->nMaintenance = -1;

            errorCode = DSL_ERROR;
            break;
         }
      }

      if (errorCode != DSL_SUCCESS)
      {
         break;
      }

      verPartVal = strtoul(token, &pStopstr, 10);

      switch (verPart)
      {
      case 0:
         pVer->nMajor = verPartVal;
         break;
      case 1:
         pVer->nMinor = verPartVal;
         break;
      case 2:
         pVer->nDevelopment = verPartVal;
         break;
      case 3:
         pVer->nMaintenance = verPartVal;
         break;
      }

      token = strtok(DSL_NULL, ".");
      ++verPart;
   }

   if (pVer->nDevelopment == -1)
   {
      pVer->nMajor       = -1;
      pVer->nMinor       = -1;
      pVer->nDevelopment = -1;
      pVer->nMaintenance = -1;

      errorCode = DSL_ERROR;
   }

   free(pVerStringCopy);

   return errorCode;
}

DSL_int32_t DSL_CPE_DeviceInit (
   DSL_CPE_Control_Context_t * pContext)
{
#ifndef INCLUDE_FW_REQUEST_SUPPORT
   DSL_Error_t nRet = DSL_SUCCESS;
   DSL_uint8_t *pFirmware1 = DSL_NULL;
   DSL_uint32_t nFirmwareSize1 = 0;
   DSL_uint8_t *pFirmware2 = DSL_NULL;
   DSL_uint32_t nFirmwareSize2 = 0;
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/
   DSL_int_t FwApplication = -1;
   DSL_int32_t ret = 0;
   DSL_Init_t init;
   DSL_G997_LineInventoryNeData_t inv;
   DSL_int_t nDevice = 0;

   memset (&init, 0x00, sizeof (DSL_Init_t));

#ifdef INCLUDE_DSL_CPE_API_VINAX
   /** Just for debug purposes. Further all initialization should be done
       in the gInitCfgData table*/
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_DOWNSTREAM].MinDataRate =
      64000;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_UPSTREAM].MinDataRate = 64000;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_DOWNSTREAM].MaxDataRate =
      140000000;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_UPSTREAM].MaxDataRate =
      140000000;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_DOWNSTREAM].MaxIntDelay = 0;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_UPSTREAM].MaxIntDelay = 0;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_DOWNSTREAM].MinINP = 0;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_UPSTREAM].MinINP = 0;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_DOWNSTREAM].MaxBER =
      DSL_G997_MAX_BER_7;
   gInitCfgData.nDeviceCfg.ChannelConfigData[DSL_UPSTREAM].MaxBER =
      DSL_G997_MAX_BER_7;
#endif

   gInitCfgData.nAutobootStartupMode = DSL_AUTOBOOT_CTRL_START;

   memcpy (&init.data, &gInitCfgData, sizeof (DSL_InitData_t));

#ifndef INCLUDE_FW_REQUEST_SUPPORT
   if (bOptimize != 1)
   {
      /* Get Firmware binary 1 */
      if ((g_bFirmware1 != -1) || (strlen(g_sFirmwareName1) > 0))
      {
         nRet = DSL_CPE_LoadFirmwareFromFile(g_sFirmwareName1, &pFirmware1,
            &nFirmwareSize1);
         if (nRet == DSL_SUCCESS)
         {
            if (pFirmware1 != DSL_NULL)
            {
               /* Assign FW binary*/
               init.data.pFirmware = pFirmware1;
               init.data.nFirmwareSize = nFirmwareSize1;
            }
         }
      }

      /* Get Firmware binary 2 */
      if ((g_bFirmware2 != -1) || (strlen(g_sFirmwareName2)))
      {
         nRet = DSL_CPE_LoadFirmwareFromFile(g_sFirmwareName2, &pFirmware2,
            &nFirmwareSize2);
         if (nRet == DSL_SUCCESS)
         {
            if (pFirmware2 != DSL_NULL)
            {
               /* Assign FW binary*/
               init.data.pFirmware2 = pFirmware2;
               init.data.nFirmwareSize2 = nFirmwareSize2;
            }
         }
      }
   }
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/

   if (bXtuOctets == 1)
   {
      /* Use given XTU octets given on init (-i) */
      memcpy (&init.data.nXtseCfg, &g_nXtseInit,
         sizeof(DSL_G997_XTUSystemEnablingData_t));
   }
   else
   {
      /* Get ADSL FW Information*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
      if ((g_bFirmware1 != -1) || (strlen(g_sFirmwareName1) > 0))
#else
      if ((g_bFirmware2 != -1) || (strlen(g_sFirmwareName2) > 0))
#endif
      {
         DSL_char_t sWhatString[MAX_WHAT_STRING_LEN] = {0};

         /* Get What String*/
         DSL_CPE_WhatStringGet(
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
            g_sFirmwareName1,
#else
            g_sFirmwareName2,
#endif
            MAX_WHAT_STRING_LEN, sWhatString);

         /* Get FW application type*/
         DSL_CPE_FwApplicationFromWhatstringGet(sWhatString, &FwApplication);
      }

      /* If no XTU octets are given on init (-i) use device specific default
         configuration */
      memset (&init.data.nXtseCfg.XTSE, 0, sizeof(init.data.nXtseCfg.XTSE));

      /* Check for Annex A (default) mode*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
      if ((FwApplication == -1) || (FwApplication == 1))
#else /* VINAX*/
      if ((FwApplication == -1) || (FwApplication == 5))
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/
      {
         init.data.nXtseCfg.XTSE[0] = 0x5;
         init.data.nXtseCfg.XTSE[2] = 0x4;
         init.data.nXtseCfg.XTSE[4] = 0xc;
         init.data.nXtseCfg.XTSE[5] = 0x1;
      }
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
      else if (FwApplication == 2)
#else /* VINAX*/
      else if (FwApplication == 6)
#endif /* defined (INCLUDE_DSL_CPE_API_DANUBE)*/
      {
         init.data.nXtseCfg.XTSE[0] = 0x10;
         init.data.nXtseCfg.XTSE[2] = 0x10;
         init.data.nXtseCfg.XTSE[5] = 0x4;
      }

#ifdef INCLUDE_DSL_CPE_API_VINAX
      /* Check if the VDSL FW was specified*/
      if ((g_bFirmware1 != -1) || (strlen(g_sFirmwareName1) > 0))
      {
         init.data.nXtseCfg.XTSE[7] = 0x7;
      }
#endif
   }

   memset(&inv,0x0, sizeof(DSL_G997_LineInventoryNeData_t ));
#ifdef INCLUDE_DSL_CPE_API_VINAX
   memcpy(&inv.Auxiliary.pData, "12344321", 8);
   inv.Auxiliary.nLength = 8;
#endif
   /* $$ ND: this is for testing only --> */
   memcpy (&inv.SerialNumber, "01234567890123456789012345678901", 32);
   memcpy (&inv.SystemVendorID, "23456789", 8);
   memcpy (&inv.VersionNumber, "0123456789012345", 16);
   init.data.pInventory = &inv;
   /* $$ <-- */

   for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
   {
      init.data.nAutobootConfig.nStateMachineOptions.bWaitBeforeLinkActivation =
         g_bWaitBeforeLinkActivation[nDevice];

      init.data.nAutobootConfig.nStateMachineOptions.bWaitBeforeConfigWrite =
         g_bWaitBeforeConfigWrite[nDevice];

      init.data.nAutobootConfig.nStateMachineOptions.bWaitBeforeRestart =
         g_bWaitBeforeRestart[nDevice];

      /* Initialize device driver*/
      ret = DSL_CPE_Ioctl (pContext->fd[nDevice], DSL_FIO_INIT, (DSL_int_t) & init);
      if (ret < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "Device %d init failed!"DSL_CPE_CRLF, nDevice));
         break;
      }
      else if (init.accessCtl.nReturn == DSL_WRN_ALREADY_INITIALIZED)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Already initialized - "
            "ignoring '-i' option!" DSL_CPE_CRLF));
         bInit = -1;
      }
#ifndef INCLUDE_FW_REQUEST_SUPPORT
      else
      {
         if (bOptimize == 1)
         {
            if ((g_bFirmware1 != -1) || (strlen(g_sFirmwareName1) > 0))
            {
               ret = DSL_CPE_DownloadFirmware(
                        pContext->fd[nDevice],
                        DSL_FW_REQUEST_ADSL, g_sFirmwareName1, DSL_NULL);
            }
         }
      }
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/
   }

#ifndef INCLUDE_FW_REQUEST_SUPPORT
   if (pFirmware1 != DSL_NULL)
   {
      DSL_CPE_Free (pFirmware1);
      pFirmware1 = DSL_NULL;
      nFirmwareSize1 = 0;
   }

   if (pFirmware2 != DSL_NULL)
   {
      DSL_CPE_Free (pFirmware2);
      pFirmware2 = DSL_NULL;
      nFirmwareSize2 = 0;
   }
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/

   return ret;
}


/**
   Termination handler. Will clean up in case of ctrl-c.

   \param sig signal number
*/
DSL_CPE_STATIC  void DSL_CPE_TerminationHandler (
   DSL_int_t sig)
{
#ifndef RTEMS
   /* ignore the signal, we'll handle by ourself */
   signal (sig, SIG_IGN);

   if (sig == SIGINT)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "terminated" DSL_CPE_CRLF));
      DSL_CPE_Termination ();
   }
#endif /* RTEMS*/
}

DSL_CPE_STATIC  DSL_void_t DSL_CPE_Termination (void)
{
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   DSL_int_t nDevice = 0;
   DSL_char_t buf[32] = "quit";
#endif

   DSL_CPE_Control_Context_t *pCtrlCtx;

   pCtrlCtx = DSL_CPE_GetGlobalContext();
   if (pCtrlCtx != DSL_NULL)
   {
      pCtrlCtx->bRun = DSL_FALSE;
   }

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
   {
      /* quit via the CLI */
      DSL_CPE_CLI_CommandExecute (DSL_CPE_GetGlobalContext()->fd[nDevice], buf, DSL_NULL,
         DSL_CPE_STDERR);
   }

   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "CLI interface of DSL CPE API terminated." DSL_CPE_CRLF));
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */
}

/**
   Control application for the DSL CPE driver.

   \param argc - number of parameters
   \param argv - array of parameter strings

   \return
   DSL_SUCCESS - success
*/
DSL_int_t dsl_cpe_daemon (
   int argc,
   DSL_char_t * argv[])
{
   DSL_int_t ret = 0;
   DSL_char_t device[128] = "";
   DSL_int_t fd = 0, i = 0, nDevice = 0;
   DSL_DBG_ModuleLevel_t nDbgModLvl;
   DSL_VersionInformation_t verInf;
#ifdef VXWORKS
   DSL_int_t stdfds[3];
#endif /* VXWORKS*/
   DSL_CPE_Control_Context_t nCtrlCtx;
   DSL_CPE_Control_Context_t *pCtrlCtx = &nCtrlCtx;
   DSL_InstanceControl_t instanceConfig;
   DSL_EventStatusMask_t statusEventMaskCfgSet;
#ifndef INCLUDE_DSL_CPE_CLI_SUPPORT
#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
   DSL_CPE_ResourceUsageStatisticsData_t pAppResStatData;
   DSL_ResourceUsageStatistics_t pDrvResStatData;
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#if defined(RTEMS) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
   void adsl_dbg_init(void);
   /* Register RTEMS Debug Module CLI.
      located in adsl_dbg.c: adsl_dbg_init()*/
   adsl_dbg_init();
#endif /* defined(RTEMS) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/

   memset (pCtrlCtx, 0x00, sizeof (DSL_CPE_Control_Context_t));
   gDSLContext = pCtrlCtx;
   g_nMsgDumpDbgLvl = DSL_DBG_MSG;

   /* Set default Device Number*/
   pCtrlCtx->nDevNum = 0;

#if (DSL_CPE_MAX_DEVICE_NUMBER == 1)
   pCtrlCtx->bBackwardCompMode = DSL_TRUE;
#else
   pCtrlCtx->bBackwardCompMode = DSL_FALSE;
#endif /* (DSL_CPE_MAX_DEVICE_NUMBER == 1)*/

   /* Initialize Wait points and Auto continue options*/
   for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
   {
      g_nPrevLineState[nDevice] = DSL_LINESTATE_NOT_INITIALIZED;
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
      g_bAutoContinueWaitBeforeLinkActivation[nDevice] = DSL_TRUE;
      g_bAutoContinueWaitBeforeConfigWrite[nDevice]    = DSL_TRUE;
      g_bAutoContinueWaitBeforeRestart[nDevice]        = DSL_TRUE;
      g_bWaitBeforeLinkActivation[nDevice] = DSL_TRUE;
      g_bWaitBeforeConfigWrite[nDevice]    = DSL_TRUE;
      g_bWaitBeforeRestart[nDevice]        = DSL_TRUE;
#else
      g_bWaitBeforeLinkActivation[nDevice] = DSL_FALSE;
      g_bWaitBeforeConfigWrite[nDevice]    = DSL_FALSE;
      g_bWaitBeforeRestart[nDevice]        = DSL_FALSE;
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT*/
   }

   /* Initialize firmware files with its default values (might be overwritten
      by user arguments in folowing DSL_CPE_ArgParse() function) */
   g_sFirmwareName1 = DSL_CPE_Malloc(strlen(sDefaultFirmwareName1) + 1);
   g_sFirmwareName2 = DSL_CPE_Malloc(strlen(sDefaultFirmwareName2) + 1);
   if(g_sFirmwareName1)
   {
      strcpy(g_sFirmwareName1, sDefaultFirmwareName1);
   }
   if(g_sFirmwareName2)
   {
      strcpy(g_sFirmwareName2, sDefaultFirmwareName2);
   }

   DSL_CPE_ArgParse (argc, argv);

   /* display version */
   if (bGetVersion != -1)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL CPE Control Application V%s" DSL_CPE_CRLF, &dsl_cpe_ctl_version[4]));
      bEventActivation = DSL_FALSE;
      ret = DSL_SUCCESS;
      goto DSL_CPE_CONTROL_EXIT;
   }

#if (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
   if (pCtrlCtx->bBackwardCompMode)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Using CLI backward compatible mode" DSL_CPE_CRLF));
   }
#endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT) */

   /* display Control Application help screen*/
   if (bHelp == 1)
   {
      DSL_CPE_Help (argv[0]);
      bEventActivation = DSL_FALSE;
      ret = DSL_SUCCESS;
      goto DSL_CPE_CONTROL_EXIT;
   }
#if defined(DSL_CPE_SIMULATOR_CONTROL)
   if (DSL_SimulatorInitialize() != DSL_SUCCESS)
   {
      ret = DSL_ERROR;
      goto DSL_CPE_CONTROL_EXIT;
   }
#endif /* defined(DSL_CPE_SIMULATOR_CONTROL)*/

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   if (g_sAdslScript != DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Using ADSL autostart script file - %s" DSL_CPE_CRLF, g_sAdslScript));
   }
#ifdef INCLUDE_DSL_CPE_API_VINAX
   if (g_sVdslScript != DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "Using VDSL autostart script file - %s" DSL_CPE_CRLF, g_sVdslScript));
   }
#endif /* INCLUDE_DSL_CPE_API_VINAX */
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT */
#ifdef INCLUDE_SCRIPT_NOTIFICATION
   if (g_sRcScript != DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "using script notification file - %s" DSL_CPE_CRLF , g_sRcScript));
   }
#endif /* #ifdef INCLUDE_SCRIPT_NOTIFICATION */

   if (bInit == 1)
   {
      /* Print currently used firmware file(s) */
      if (strlen(g_sFirmwareName1) > 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "using 1st firmware file - %s" DSL_CPE_CRLF , g_sFirmwareName1));
      }
      if (strlen(g_sFirmwareName2) > 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "using 2nd firmware file - %s" DSL_CPE_CRLF , g_sFirmwareName2));
      }
   }

#ifdef INCLUDE_DSL_CPE_API_VINAX
   if (sLowLevCfgName != DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "using low level configuration file - %s" DSL_CPE_CRLF,
         sLowLevCfgName));

      ret = DSL_CPE_GetInitialLowLevelConfig( sLowLevCfgName,
                                          &(gInitCfgData.nDeviceCfg.cfg) );
      if (ret != DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "using default low level configuration" DSL_CPE_CRLF));
      }
   }
#endif /* #ifdef INCLUDE_DSL_CPE_API_VINAX*/

#ifdef LINUX
#ifdef USE_DAEMONIZE
   if (bConsole != 1)
   {
      if (daemon (1, bNotSilent) != 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "DSL CPE Control Application cannot daemonize (err=%d)" DSL_CPE_CRLF, errno));
         ret = DSL_ERROR;
         goto DSL_CPE_CONTROL_EXIT;
      }
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL CPE Control Application running in background now!" DSL_CPE_CRLF));
   }
#endif /* USE_DAEMONIZE */
   DSL_CPE_HandlerInstall ();
#endif /* LINUX */

#ifndef RTEMS
   signal (SIGINT, DSL_CPE_TerminationHandler);
#endif /* RTEMS*/

   /* Open DSL_CPE_MAX_DEVICE_NUMBER devices*/
   for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
   {
#ifdef INCLUDE_DSL_CPE_API_VINAX
      sprintf (device, "%s/%d", DSL_CPE_DEVICE_NAME, nDevice);
#else
      sprintf (device, "%s", DSL_CPE_DEVICE_NAME);
#endif /* INCLUDE_DSL_CPE_API_VINAX*/

      fd = DSL_CPE_Open (device);
      if (fd < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX "Device %s open failed!" DSL_CPE_CRLF, device));
         ret = DSL_ERROR;
         bEventActivation = DSL_FALSE;
         goto DSL_CPE_CONTROL_EXIT;
      }

      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
         (DSL_CPE_PREFIX"Device %s opened successfully" DSL_CPE_CRLF, device));

      pCtrlCtx->fd[nDevice] = fd;
   }

   memset(&(pCtrlCtx->applicationVer), 0xFF, sizeof(DSL_SwVersion_t));
   memset(&(pCtrlCtx->driverVer), 0xFF, sizeof(DSL_SwVersion_t));

   /* Get Control Application Version*/
   if (DSL_CPE_SwVersionFromStringGet(PACKAGE_VERSION, &(pCtrlCtx->applicationVer)) != DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - DSL CPE Control Application version get failed!" DSL_CPE_CRLF));
   }

   /* Get Driver version info*/
   ret = DSL_CPE_Ioctl (pCtrlCtx->fd[0], DSL_FIO_VERSION_INFORMATION_GET, (int) &verInf);

   if ((ret < 0) && (verInf.accessCtl.nReturn < DSL_SUCCESS))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - DSL_FIO_VERSION_INFORMATION_GET ioctl failed!" DSL_CPE_CRLF));
   }
   else
   {
      ret = DSL_CPE_SwVersionFromStringGet(verInf.data.DSL_DriverVersionApi, &(pCtrlCtx->driverVer));
      if ((ret < 0) && (verInf.accessCtl.nReturn < DSL_SUCCESS))
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR - DSL CPE Driver version get failed!" DSL_CPE_CRLF));
      }
   }

   if ((pCtrlCtx->driverVer.nMajor != pCtrlCtx->applicationVer.nMajor) ||
       (pCtrlCtx->driverVer.nMinor != pCtrlCtx->applicationVer.nMinor))
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - DSL CPE Driver vs Application version mismatch!" DSL_CPE_CRLF));
   }

#ifdef INCLUDE_DSL_BONDING
   if (DSL_CPE_BND_Start(
         pCtrlCtx, pCtrlCtx->fd[0] ) < DSL_SUCCESS)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ERROR - Bonding Start failed!" DSL_CPE_CRLF));
      bEventActivation = DSL_FALSE;
      ret = DSL_ERROR;
      goto DSL_CPE_CONTROL_EXIT;
   }
#endif /* INCLUDE_DSL_BONDING*/

#ifdef DSL_CPE_SOAP_FW_UPDATE
   DSL_CPE_LockCreate(&pCtrlCtx->semFwUpdate);
   pCtrlCtx->firmware.pData = DSL_NULL;
   pCtrlCtx->firmware.nSize = 0;
   pCtrlCtx->firmware2.pData = DSL_NULL;
   pCtrlCtx->firmware2.nSize = 0;
#endif /* DSL_CPE_SOAP_FW_UPDATE */

   pCtrlCtx->bRun = DSL_TRUE;

#ifdef VXWORKS
   /* FIXME: make sure that this console runs on the serial port (fd 3) */
   for (i = 0; i < 3; ++i)
   {
      stdfds[i] = ioTaskStdGet(0,i);
      if (stdfds[i] != ERROR)
         ioTaskStdSet(0,i,3);
   }
#endif

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (DSL_CPE_CLI_Init() == DSL_ERROR)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "'DSL_CPE_CLI_Init' failed! Exit dsl_cpe_control..." DSL_CPE_CRLF));
      goto DSL_CPE_CONTROL_EXIT;
   }
#endif

#ifdef DSL_DEBUG_TOOL_INTERFACE
   if (bTcpMessageIntf == 1)
   {
      DSL_CPE_TcpDebugMessageIntfStart(pCtrlCtx);
#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
      DSL_CPE_TcpDebugCliIntfStart(pCtrlCtx);
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
   }
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   /* start soap server */
   if (DSL_CPE_SoapInit (pCtrlCtx) < 0)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "DSL: 'DSL_CPE_SoapInit' failed! Exit dsl_daemon..." DSL_CPE_CRLF));
      goto DSL_CPE_CONTROL_EXIT;
   }
#endif

   /* Configure event and resource handling*/
   for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
   {
      instanceConfig.data.bEventActivation = bEventActivation;
      instanceConfig.data.nResourceActivationMask =
         (DSL_BF_ResourceActivationType_t)nResourceActivationMask;

      ret = DSL_CPE_Ioctl (pCtrlCtx->fd[nDevice], DSL_FIO_INSTANCE_CONTROL_SET,
              (DSL_int_t) &instanceConfig);

      if (ret < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Could not configure "
            "event handling for device %d (%d)." DSL_CPE_CRLF,
            i, instanceConfig.accessCtl.nReturn));
      }
   }

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
   if (bDTI == 1)
   {
      /* start DTI agent with default values */
      if ( DSL_CPE_Dti_Start(
                              pCtrlCtx,
                              DSL_CPE_MAX_DEVICE_NUMBER,
                              1,
                              DSL_CPE_DTI_DEFAULT_TCP_PORT,
                              sDtiSocketAddr,
                              #if defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
                              DSL_TRUE,
                              #else
                              DSL_FALSE,
                              #endif
                              DSL_FALSE,
                              DSL_FALSE) < DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "ERROR - DTI Agent start failed!" DSL_CPE_CRLF));

         goto DSL_CPE_CONTROL_EXIT;
      }

      bDTI = 2;
   }
#endif /* #if defined(INCLUDE_DSL_CPE_DTI_SUPPORT) */

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
   if (bConsole == 1)
   {
      if (DSL_CPE_Console_Init (&pConsoleContext, pCtrlCtx, DSL_CPE_STDIN,
            DSL_CPE_STDOUT) == DSL_ERROR)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "'DSL_CLI_Console_Init' failed! Exit dsl_cpe_control..." DSL_CPE_CRLF));
         goto DSL_CPE_CONTROL_EXIT;
      }
   }
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

   if (bEventActivation)
   {
      /* Unmask (enable) all possible events that are masked (disabled) by
         default after instance event activation. */
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         for (i = DSL_EVENT_S_FIRST; i < DSL_EVENT_S_LAST; i++)
         {
            memset(&statusEventMaskCfgSet, 0x00, sizeof(DSL_EventStatusMask_t));
            statusEventMaskCfgSet.data.nEventType = (DSL_EventType_t)i;
            statusEventMaskCfgSet.data.bMask      = DSL_FALSE;
            ret = DSL_CPE_Ioctl(pCtrlCtx->fd[nDevice], DSL_FIO_EVENT_STATUS_MASK_CONFIG_SET,
               (DSL_int_t) &statusEventMaskCfgSet);
            if (ret < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Could not unmask "
                  "event(%d) for device %d (%d)!" DSL_CPE_CRLF, i, nDevice, instanceConfig.accessCtl.nReturn));
            }
         }
      }
   }

   if (bEventActivation == DSL_TRUE)
   {
      DSL_CPE_EventHandlerStart (pCtrlCtx);
   }

   if (bMsgDump == 1)
   {
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         nDbgModLvl.data.nDbgModule = DSL_DBG_MESSAGE_DUMP;
         nDbgModLvl.data.nDbgLevel  = (DSL_debugLevels_t)g_nMsgDumpDbgLvl;
         ret = DSL_CPE_Ioctl (pCtrlCtx->fd[nDevice], DSL_FIO_DBG_MODULE_LEVEL_SET, (int) &nDbgModLvl);
         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
               "failed to set message dump debug level for device %d!" DSL_CPE_CRLF, nDevice));
         }
      }
   }

   if (bInit == 1)
   {
      ret = DSL_CPE_DeviceInit (pCtrlCtx);
      if (ret < 0)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "Device initialization failed (ret=%d)!" DSL_CPE_CRLF, ret));
         goto DSL_CPE_CONTROL_EXIT;
      }
      else
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
            "Device initialized succeeded (ret=%d)." DSL_CPE_CRLF, ret));
      }
   }

   if (bInit != 1)
   {
#ifdef INCLUDE_FW_REQUEST_SUPPORT
#if defined(INCLUDE_DSL_CPE_API_DANUBE)
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         /* Download ADSL Firmware*/
         ret = DSL_CPE_DownloadFirmware(
            pCtrlCtx->fd[nDevice], DSL_FW_REQUEST_ADSL, DSL_NULL, DSL_NULL);
            if (ret == DSL_ERR_NOT_INITIALIZED)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
                  "Device[%d] not initialized yet, please use [-i] option" DSL_CPE_CRLF,
                  nDevice));
            }

      }
#else
      DSL_AutobootControl_t pAcs;

      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         memset(&pAcs,0x0, sizeof(DSL_AutobootControl_t));
         /* Restat Autoboot handling*/
         pAcs.data.nCommand = DSL_AUTOBOOT_CTRL_RESTART;

         ret =  DSL_CPE_Ioctl(pCtrlCtx->fd[nDevice],
            DSL_FIO_AUTOBOOT_CONTROL_SET, (DSL_int_t) &pAcs);
         if (ret < 0)
         {
            if (pAcs.accessCtl.nReturn == DSL_ERR_NOT_INITIALIZED)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
                  "Device[%d] not initialized yet, please use [-i] option" DSL_CPE_CRLF,
                  nDevice));
            }
         }
      }
#endif /* defined(INCLUDE_DSL_CPE_API_DANUBE)*/
#else
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         /* Download ADSL/VDSL Firmware if available*/
         ret = DSL_CPE_DownloadFirmware(
            pCtrlCtx->fd[nDevice], DSL_FW_REQUEST_NA, DSL_NULL, DSL_NULL);
         if (ret == DSL_ERR_NOT_INITIALIZED)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
               "Device[%d] not initialized yet, please use [-i] option" DSL_CPE_CRLF,
               nDevice));
         }
      }
#endif /* INCLUDE_FW_REQUEST_SUPPORT*/
   }

#ifndef INCLUDE_DSL_CPE_CLI_SUPPORT
#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
   /* In case of disabled CLI there is no possibility to get resource usage
      statistics manually. Therefore print resource statistics here by default */

   /* Wait for all driver modules to be initialized*/
   DSL_CPE_Sleep(3);

   memset(&pDrvResStatData, 0, sizeof(DSL_ResourceUsageStatistics_t));

   for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
   {
      ret = DSL_CPE_Ioctl (pCtrlCtx->fd[nDevice], DSL_FIO_RESOURCE_USAGE_STATISTICS_GET, (int) &pDrvResStatData);
      if ((ret < 0) && (pDrvResStatData.accessCtl.nReturn < DSL_SUCCESS))
      {
         printf(DSL_CPE_PREFIX "Failed to get device=%d resource usage statistic from driver!"
            DSL_CPE_CRLF, nDevice);
      }
      else
      {
#if (DSL_CPE_MAX_DEVICE_NUMBER > 1)
         printf(DSL_CPE_PREFIX "nDevice=%d drvStaticMemUsage=%d drvDynamicMemUsage=%d"
            DSL_CPE_CRLF,
            nDevice, pDrvResStatData.data.staticMemUsage, pDrvResStatData.data.dynamicMemUsage);
#else
         printf(DSL_CPE_PREFIX "drvStaticMemUsage=%d drvDynamicMemUsage=%d "
            DSL_CPE_CRLF,
            pDrvResStatData.data.staticMemUsage, pDrvResStatData.data.dynamicMemUsage);
#endif /* (DSL_CPE_MAX_DEVICE_NUMBER > 1)*/
      }
   }

   memset(&pAppResStatData, 0, sizeof(DSL_CPE_ResourceUsageStatisticsData_t));
   if (DSL_CPE_ResourceUsageStatisticsGet(pCtrlCtx,
      &pAppResStatData) == DSL_ERROR)
   {
      printf(DSL_CPE_PREFIX "Failed to get resource usage statistic from application!"
         DSL_CPE_CRLF);
   }

   printf(DSL_CPE_PREFIX"appStaticMemUsage=%d appDynamicMemUsage=%d" DSL_CPE_CRLF,
      pAppResStatData.staticMemUsage, pAppResStatData.dynamicMemUsage);
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
#ifndef DSL_CPE_REMOVE_PIPE_SUPPORT
   if (DSL_CPE_Pipe_Init (pCtrlCtx) == DSL_ERROR)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "'DSL_CPE_Pipe_Init' failed! Exit dsl_cpe_control..." DSL_CPE_CRLF));
      goto DSL_CPE_CONTROL_EXIT;
   }
#endif /* DSL_CPE_REMOVE_PIPE_SUPPORT */

   if (bConsole == 1)
   {
      do
      {
         ret = DSL_CPE_Handle_Console (pConsoleContext);
         if (ret < 0)
         {
            if (pCtrlCtx->bRun)
            {
               DSL_CPE_Termination ();
            }
            break;
         }
      }
      while (ret != DSL_ERROR);

      ret = DSL_SUCCESS;

      if (DSL_CPE_Console_Shutdown(pConsoleContext) == DSL_ERROR)
      {
         goto DSL_CPE_CONTROL_EXIT;
      }
      pConsoleContext = DSL_NULL;

   }
   else
   {
      dummy_console.bRun = DSL_TRUE;

      DSL_CPE_CLI_Register (&dummy_console.pCLIContext, &dummy_console,
         DSL_CPE_Control_Exit, DSL_NULL);

      while (dummy_console.bRun)
      {
         DSL_CPE_Sleep (1);
      }
   }

   DSL_CPE_CLI_Shutdown();
#else
   while(pCtrlCtx->bRun)
   {
      DSL_CPE_Sleep (1);
   }
#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT */

DSL_CPE_CONTROL_EXIT:

#ifdef INCLUDE_DSL_BONDING
   DSL_CPE_BND_Stop((DSL_CPE_BND_Context_t*)pCtrlCtx->pBnd);
#endif /* INCLUDE_DSL_BONDING*/

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
   if (bDTI == 2)
   {
      /* Stop DTI Agent*/
      if (DSL_CPE_Dti_Stop(pCtrlCtx) < DSL_SUCCESS)
      {
         DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
            (DSL_CPE_PREFIX"ERROR - DTI Agent stop failed!" DSL_CPE_CRLF));
      }
   }

   if(sDtiSocketAddr != DSL_NULL)
   {
      DSL_CPE_Free (sDtiSocketAddr);
      sDtiSocketAddr = DSL_NULL;
   }
#endif /* defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/

   if (bEventActivation)
   {
      /* Mask (disable) all possible events which were enabled at the startup.
         Workaround to solve the SMS00833504 issue.*/
      for (nDevice = 0; nDevice < DSL_CPE_MAX_DEVICE_NUMBER; nDevice++)
      {
         for (i = DSL_EVENT_S_FIRST; i < DSL_EVENT_S_LAST; i++)
         {
            if (pCtrlCtx->fd[nDevice] < 0)
            {
               continue;
            }

            memset(&statusEventMaskCfgSet, 0x00, sizeof(DSL_EventStatusMask_t));
            statusEventMaskCfgSet.data.nEventType = (DSL_EventType_t)i;
            statusEventMaskCfgSet.data.bMask      = DSL_TRUE;
            ret = DSL_CPE_Ioctl(pCtrlCtx->fd[nDevice], DSL_FIO_EVENT_STATUS_MASK_CONFIG_SET,
               (DSL_int_t) &statusEventMaskCfgSet);
            if (ret < 0)
            {
               DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX "Could not nmask "
                  "event(%d) for device %d (%d)!" DSL_CPE_CRLF, i, nDevice, instanceConfig.accessCtl.nReturn));
            }
         }
      }
   }

#ifdef VXWORKS
   /* restore stdio */
   for (i = 0; i < 3; ++i)
   {
      if (stdfds[i] != ERROR)
         ioTaskStdSet(0,i,stdfds[i]);
   }
#endif

   if (pCtrlCtx->bRun)
   {
      pCtrlCtx->bRun = DSL_FALSE;

      /* wait while CPE Event handler thread ends */
      DSL_CPE_ThreadShutdown (&pCtrlCtx->EventControl, 1000);
   }

   for (i = 0; i < DSL_CPE_MAX_DEVICE_NUMBER; i++)
   {
      if(pCtrlCtx->fd[i] >= 0)
      {
         /* close device */
         ret = (DSL_int32_t) (DSL_CPE_Close (pCtrlCtx->fd[i]));
         if (ret < 0)
         {
            DSL_CCA_DEBUG(DSL_CCA_DBG_ERR,
               (DSL_CPE_PREFIX"error on closing device %d!" DSL_CPE_CRLF, i));
         }
      }
   }
#ifdef DSL_CPE_SIMULATOR_DRIVER
   /* Delete Complex Simulator device*/
   DSL_DRV_SIM_DeviceDelete();
#endif /* DSL_CPE_SIMULATOR_DRIVER*/

#ifdef INCLUDE_SCRIPT_NOTIFICATION
   if(g_sRcScript != DSL_NULL)
   {
      DSL_CPE_Free (g_sRcScript);
      g_sRcScript = DSL_NULL;
   }
#endif

   if(g_sFirmwareName1 != DSL_NULL)
   {
      DSL_CPE_Free (g_sFirmwareName1);
      g_sFirmwareName1 = DSL_NULL;
   }

   if(g_sFirmwareName2 != DSL_NULL)
   {
      DSL_CPE_Free (g_sFirmwareName2);
      g_sFirmwareName2 = DSL_NULL;
   }

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   if (sSoapRemoteServer)
   {
      DSL_CPE_Free (sSoapRemoteServer);
      sSoapRemoteServer = DSL_NULL;
   }
#endif

#ifdef DSL_CPE_SOAP_FW_UPDATE
   if(pCtrlCtx->firmware.pData != DSL_NULL)
   {
      DSL_CPE_Free (pCtrlCtx->firmware.pData);
      pCtrlCtx->firmware.pData = DSL_NULL;
      pCtrlCtx->firmware.nSize = 0;
   }

   if(pCtrlCtx->firmware2.pData != DSL_NULL)
   {
      DSL_CPE_Free (pCtrlCtx->firmware2.pData);
      pCtrlCtx->firmware2.pData = DSL_NULL;
      pCtrlCtx->firmware2.nSize = 0;
   }
#endif

#ifdef DSL_DEBUG_TOOL_INTERFACE
   if(sTcpMessagesSocketAddr != DSL_NULL)
   {
      DSL_CPE_Free (sTcpMessagesSocketAddr);
      sTcpMessagesSocketAddr = DSL_NULL;
   }
#endif

#ifdef INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT
#ifdef INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT
   if(g_sAdslScript != DSL_NULL)
   {
      DSL_CPE_Free (g_sAdslScript);
      g_sAdslScript = DSL_NULL;
   }

   if(g_sVdslScript != DSL_NULL)
   {
      DSL_CPE_Free (g_sVdslScript);
      g_sVdslScript = DSL_NULL;
   }
#endif /* INCLUDE_DSL_CPE_CMV_SCRIPTS_SUPPORT */
#endif /* INCLUDE_DSL_CPE_FILESYSTEM_SUPPORT */

#if defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)
   if (g_sRemoteTcpServerIp)
   {
      DSL_CPE_Free (g_sRemoteTcpServerIp);
   }
#endif /* defined(INCLUDE_TCP_SIMULATOR) && defined(DSL_CPE_SIMULATOR_DRIVER) && defined(WIN32)*/

   return ret;
}

#if defined(LINUX) || defined(WIN32)
int main (int argc, char *argv[])
{

   return dsl_cpe_daemon (argc, argv);
}
#elif defined(RTEMS)
void dsl_cpe_daemon_start(void)
{
   DSL_char_t * argv[]={"dsl_cpe_control","-i","-c"};
   /* Start Daemon*/
   dsl_cpe_daemon (3,&argv);
   xt_delete(0);
}

/* RTEMS main */
void DSL_CPE_main(void)
{
   int DSL_DRV_DeviceCreate(void);
   int DSL_DRV_Open(void);

   /* level driver (MEI BSP Driver) is already initialized */

   /* load the driver module ("insmod ./drv_dsl_cpe_api" in Linux) */
   DSL_DRV_DeviceCreate();
   DSL_DRV_Open();

   memset(&DslMainControl, 0x00, sizeof(DslMainControl));

   /* Now the target system is ready to run the DSL CPE Control Application. */
   DSL_CPE_ThreadInit(&DslMainControl, "DslMain", dsl_cpe_daemon_start,
                      DSL_CPE_DEFAULT_STACK_SIZE, DSL_CPE_PRIORITY,
                      (DSL_uint32_t)0, (DSL_uint32_t)0);
}
#endif /* defined(LINUX) || defined(WIN32)*/

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT
DSL_CPE_STATIC  DSL_Error_t DSL_CPE_Control_Exit (DSL_void_t * pContext)
{
   dummy_console_t *pConsole = pContext;
   pConsole->bRun = DSL_FALSE;
   return DSL_SUCCESS;
}
#endif

/**
sscanf implementation with uint8 support.
*/
DSL_int32_t DSL_CPE_sscanf (
   DSL_char_t *buf,
   DSL_char_t const *fmt,
   ...
)
{
#ifndef _lint
   va_list marker;
#endif
   DSL_char_t const *p, *a;
   DSL_char_t *r = 0, *s = buf;
   DSL_char_t *v8 = 0;
   DSL_int_t ret = 0, mode = 32, base = 10, array = 0;
   DSL_int_t i;
   DSL_int_t *v32 = 0;
   short *v16 = 0;
   unsigned int *vu32 = 0;
   unsigned short *vu16 = 0;
   unsigned char *vu8 = 0;

   if ((buf == DSL_NULL) || (fmt == DSL_NULL))
      return ret;

#ifndef _lint
   va_start (marker, fmt);
#endif

   if (s == DSL_NULL)
      goto SF_END;

   for (p = fmt; *p; p++)
   {
      if (*p != '%')
         continue;

      if (s == DSL_NULL)
         goto SF_END;

      /* skip spaces and tabs */
      while ((*s == ' ') || (*s == '\t'))
         s++;

      if (*s == 0)
         goto SF_END;

      switch (*++p)
      {
      case 0:
         /* ret = 0; */
         goto SF_END;

         /* 8 bit */
      case 'b':
         mode = 8;
         p++;
         break;

         /* 16 bit */
      case 'h':
         mode = 16;
         p++;
         break;

         /* 32 bit */
      default:
         mode = 32;
         break;
      }

      switch (*p)
      {
      case 'd':
      case 'i':
      case 'u':
         base = 10;
         break;

      case 'x':
         base = 16;
         break;

      default:
         break;
      }

      a = p + 1;
      i = 0;

      array = 1;

      switch (*a)
      {
      case '[':
         a++;
         if (*a)
         {
            array = (DSL_char_t) strtol (a, DSL_NULL, 10);
            do
            {
               a++;
               if (*a == ']')
               {
                  a++;
                  break;
               }
            } while (*a);
         }
         break;

      default:
         break;
      }

      switch (*p)
      {
      case 0:
         /* ret = 0; */
         goto SF_END;

         /* string */
      case 's':
         {
#ifndef _lint
            r = va_arg (marker, DSL_char_t *);
#endif
            if (r != DSL_NULL)
            {
               DSL_char_t *q = s;

               if (q)
               {
                  do
                  {
                     if ((*q == ' ') || (*q == '\t'))
                        q++;
                     else
                        break;
                  } while (*q);

                  if (*q)
                  {
                     do
                     {
                        if ((*q != ' ') && (*q != '\t') && (*q != '\n') &&
                           (*q != '\r'))
                           *r++ = *q++;
                        else
                           break;
                     } while (*q);
                     s = q;
                     *r = 0;
                     ret++;
                  }
               }
            }
            break;
         }
         /* signed */
      case 'd':
      case 'i':
         {
            switch (mode)
            {
            case 8:
#ifndef _lint
               v8 = va_arg (marker, DSL_char_t *);
#endif
               if (v8 != DSL_NULL)
               {
                  for (i = 0; i < array; i++)
                  {
                     if (s && *s != 0)
                     {
                        v8[i] = (DSL_char_t) strtol (s, &s, base);
                     }
                     else
                     {
                        break;
                     }
                  }
                  if (i == array)
                     ret++;
               }
               break;

            case 16:
#ifndef _lint
               v16 = va_arg (marker, short *);
#endif
               if (v16 != DSL_NULL)
               {
                  for (i = 0; i < array; i++)
                  {
                     if (s && *s != 0)
                     {
                        v16[i] = (short) strtol (s, &s, base);
                     }
                     else
                     {
                        break;
                     }
                  }
                  if (i == array)
                     ret++;
               }
               break;

            case 32:
#ifndef _lint
               v32 = va_arg (marker, DSL_int_t *);
#endif
               if (v32 != DSL_NULL)
               {
                  for (i = 0; i < array; i++)
                  {
                     if (s && *s != 0)
                     {
                        v32[i] = (DSL_int_t) strtol (s, &s, base);
                     }
                     else
                     {
                        break;
                     }
                  }
                  if (i == array)
                     ret++;
               }
               break;

            default:
               break;
            }
            break;
         }

         /* unsigned */
      case 'u':
         /* hexadecimal */
      case 'x':
         {
            switch (mode)
            {
            case 8:
#ifndef _lint
               vu8 = va_arg (marker, unsigned char *);
#endif
               if (vu8 != DSL_NULL)
               {
                  for (i = 0; i < array; i++)
                  {
                     if (s && *s != 0)
                     {
                        vu8[i] = (unsigned char) strtoul (s, &s, base);
                     }
                     else
                     {
                        break;
                     }
                  }
                  if (i == array)
                     ret++;
               }
               break;

            case 16:
#ifndef _lint
               vu16 = va_arg (marker, unsigned short *);
#endif
               if (vu16 != DSL_NULL)
               {
                  for (i = 0; i < array; i++)
                  {
                     if (s && *s != 0)
                     {
                        vu16[i] = (unsigned short) strtoul (s, &s, base);
                     }
                     else
                     {
                        break;
                     }
                  }
                  if (i == array)
                     ret++;
               }
               break;

            case 32:
#ifndef _lint
               vu32 = va_arg (marker, unsigned int *);
#endif
               if (vu32 != DSL_NULL)
               {
                  for (i = 0; i < array; i++)
                  {
                     if (s && *s != 0)
                     {
                        vu32[i] = (unsigned int) strtoul (s, &s, base);
                     }
                     else
                     {
                        break;
                     }
                  }
                  if (i == array)
                     ret++;
               }
               break;

            default:
               break;
            }
            break;

      default:
            break;
         }
      }

      if (array > 1)
      {
         p = a;
      }
   }

 SF_END:

#ifndef _lint
   va_end (marker);
#endif

   return ret;
}
