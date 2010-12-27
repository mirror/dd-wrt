/******************************************************************************

                              Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL daemon command line interface for the Debug & Trace Interface
*/

/*#define DEBUG_CONSOLE*/

/* ============================================================================
   inlcudes
   ========================================================================= */
#include "dsl_cpe_control.h"

#if defined(INCLUDE_DSL_CPE_DTI_SUPPORT)

#include "dsl_cpe_os.h"
#include "dsl_cpe_cli.h"
#include "dsl_cpe_dti.h"

/* get the DTI interface defines */
#include "dti_agent_interface.h"
#include "dti_cli_interface.h"

#ifdef DTI_STATIC
#undef DTI_STATIC
#endif

#if (defined(DTI_DEBUG) || 1)
#define DTI_STATIC
#else
#define DTI_STATIC   static
#endif


/* ============================================================================
   Local defines
   ========================================================================= */

struct DSL_CPE_Dti_Context_s
{
   /* points to the DTI Agent context pointer */
   DTI_AgentCtx_t            *pDtiAgent;

   /* enable / disbale DTI interface */
   volatile DSL_boolean_t    bEnabled;
   /* if number */
   volatile DSL_int_t        ifNum;
   /** dsl cpe api context pointer */
   DSL_CPE_Control_Context_t *pDSLContext;
   /** console pointer (event handling) */
   DSL_CLI_Context_t         *pCLIContext;
};

typedef struct DSL_CPE_Dti_Context_s DSL_CPE_Dti_Context_t;

/* ============================================================================
   Local Function declarations
   ========================================================================= */
DTI_STATIC DSL_int_t DSL_CPE_CLI_Dti_Command_Exec(
                        DSL_CPE_Dti_Context_t     *pDtiContext,
                        const DSL_char_t          *pCliDtiCommand,
                        DSL_CPE_File_t                *pOutStream);

DTI_STATIC DSL_Error_t DSL_CPE_CLI_Dti_Event(
                        DSL_void_t *pContext,
                        DSL_char_t *pMessage);

DTI_STATIC DSL_Error_t DSL_CPE_CLI_Dti_Exit(
                        DSL_void_t *pContext);

DTI_STATIC DSL_int_t DSL_CPE_CLI_Dti_Exec(
                        DSL_void_t        *pCliDtiDescriptor,
                        const DSL_char_t  *pCmdIn,
                        DSL_char_t        *pResultOut,
                        DSL_int_t         *pResultBufSize_byte,
                        DSL_int_t         *pResultCode);

/* ============================================================================
   Local Variables
   ========================================================================= */

/* DTI interface - context */
DTI_STATIC DSL_CPE_Dti_Context_t DSL_CPE_GlobalDtiContext = {DSL_NULL, DSL_FALSE, -1, DSL_NULL, DSL_NULL};


/* ============================================================================
   Local Function definitons
   ========================================================================= */

#if defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
/**
   Handle a DTI command form the Debug & Trace interface
*/
DTI_STATIC DSL_int_t DSL_CPE_CLI_Dti_Command_Exec(
                        DSL_CPE_Dti_Context_t   *pDtiContext,
                        const DSL_char_t        *pCliDtiCommand,
                        DSL_CPE_File_t              *pOutStream)
{
   DSL_int_t   retVal = 0;
   DSL_char_t  cmdBuf[256];
   DSL_char_t  *pArgBuf = DSL_NULL;

   /* get command name */
   sscanf(pCliDtiCommand, "%s", cmdBuf);

   if(strlen(cmdBuf) < strlen(pCliDtiCommand))
   {
      pArgBuf = (DSL_char_t *)pCliDtiCommand + strlen(cmdBuf) + 1;
   }

   retVal = DSL_CPE_CliDeviceCommandExecute(pDtiContext->pDSLContext, -1, cmdBuf, pArgBuf, pOutStream);

   return retVal;
}

/**
   Exit the CLI DTI
*/
DTI_STATIC DSL_Error_t DSL_CPE_CLI_Dti_Exit(
                        DSL_void_t *pContext)
{
   DSL_Error_t             ret = DSL_SUCCESS;
   DSL_CPE_Dti_Context_t   *pDtiContext = (DSL_CPE_Dti_Context_t*)pContext;

   if (pDtiContext == DSL_NULL)
   {
      return DSL_ERROR;
   }

   /* unregister the exec within the DTI module */
#ifdef INCLUDE_DSL_API_CONSOLE_EXTRA
   if (pDtiContext->bEnabled == DSL_TRUE)
   {
#if defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
      if (DSL_CPE_CLI_Unregister(pDtiContext->pCLIContext) == DSL_SUCCESS)
      {
         DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"oops, CLI DTI Exit - unregister failed" DSL_CPECRLF);
         DSL_CPE_Free(pDtiContext->pCLIContext);
         pDtiContext->pCLIContext = DSL_NULL;
      }
#endif /* #if defined(INCLUDE_DSL_CPE_CLI_SUPPORT) */
   }
#endif

   return ret;
}

/**
   CLI DTI Event
*/
DTI_STATIC DSL_Error_t DSL_CPE_CLI_Dti_Event(
                        DSL_void_t *pContext,
                        DSL_char_t *pMessage)
{
   DSL_uint_t              eventOutSize_byte = 0;
   DSL_CPE_Dti_Context_t   *pDtiContext = (DSL_CPE_Dti_Context_t *)pContext;

   eventOutSize_byte = strlen(pMessage) + 1;

   if (DTI_CLI_InterfaceEventSend(
                     pDtiContext->pDtiAgent, pDtiContext->ifNum,
                     pMessage, eventOutSize_byte) != IFX_SUCCESS)
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"CLI DTI Event - event send failed" DSL_CPE_CRLF
                              "Evt: <%s>" DSL_CPE_CRLF, pMessage);

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

/**
   Handle a incoming command form the Debug & Trace interface
*/
DTI_STATIC DSL_int_t DSL_CPE_CLI_Dti_Exec(
                        DSL_void_t        *pCliDtiDescriptor,
                        const DSL_char_t  *pCmdIn,
                        DSL_char_t        *pResultOut,
                        DSL_int_t         *pResultBufSize_byte,
                        DSL_int_t         *pResultCode)
{
   DSL_int_t retVal = 0, writtenBytes = 0;
   DSL_CPE_Dti_Context_t *pCliDtiContext = (DSL_CPE_Dti_Context_t *)pCliDtiDescriptor;
   DSL_CPE_File_t *pOutStream = DSL_CPE_STDOUT;

   if (pResultOut && pResultBufSize_byte)
   {
      memset(pResultOut, 0, *pResultBufSize_byte);
   }
   else
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"oops, missing DTI result buffer" DSL_CPE_CRLF);
      return DSL_ERROR;
   }

   if (!pCliDtiDescriptor)
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"oops, missing CLI DTI handle" DSL_CPE_CRLF);
      writtenBytes = DSL_CPE_snprintf(pResultOut, *pResultBufSize_byte,
                        "nReturn=-1 nError=\"oops, missing CLI DTI handle\n\r\"");

      *pResultBufSize_byte = writtenBytes;
      return DSL_ERROR;
   }

   if (!pCmdIn)
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"oops, missing DTI command" DSL_CPE_CRLF);
      writtenBytes = DSL_CPE_snprintf(pResultOut, *pResultBufSize_byte,
                        "nReturn=-1 nError=\"oops, missing DTI command\n\r\"");

      *pResultBufSize_byte = writtenBytes;
      return DSL_ERROR;
   }

#if defined(IFXOS_HAVE_MEMORY_FILE) && (IFXOS_HAVE_MEMORY_FILE == 1)
   pOutStream = DSL_CPE_FMemOpen(pResultOut, *pResultBufSize_byte - 1, "w");
   if(pOutStream == NULL)
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"oops, cannot open DTI stream" DSL_CPE_CRLF);
      writtenBytes = DSL_CPE_snprintf(pResultOut, *pResultBufSize_byte,
                        "nReturn=-1 nError=\"oops, cannot open DTI stream\n\r\"");

      *pResultBufSize_byte = writtenBytes;
      return DSL_ERROR;
   }
#endif

   /*
      execute the given DTI command.
   */
   retVal = DSL_CPE_CLI_Dti_Command_Exec(pCliDtiContext, pCmdIn, pOutStream);

#if defined(IFXOS_HAVE_MEMORY_FILE) && (IFXOS_HAVE_MEMORY_FILE == 1)
   DSL_CPE_FClose(pOutStream);

   writtenBytes = strlen(pResultOut);
   pResultOut[writtenBytes] = '\0';
#else
   writtenBytes = DSL_CPE_snprintf(pResultOut, *pResultBufSize_byte,
                     "nReturn=%d nWarning=\"result written to stdout\n\r\"", retVal);
#endif

   *pResultBufSize_byte = writtenBytes;

   return DSL_SUCCESS;
}
#endif /* #if defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/

/* ============================================================================
   Global Function definitons
   ========================================================================= */
/**
   Initialize the DTI
*/
DSL_Error_t DSL_CPE_Dti_Start(
                        DSL_CPE_Control_Context_t *pContext,
                        DSL_int_t      numOfPhyDevices,
                        DSL_int_t      numOfLinesPerPhyDevice,
                        DSL_uint16_t   dtiListenPort,
                        DSL_char_t     *pDtiServerIp,
                        DSL_boolean_t  bEnableCliAutoMsg,
                        DSL_boolean_t  bEnableDevAutoMsg,
                        DSL_boolean_t  bEnableSingleThreadMode)
{
   DSL_int_t                  ret = DSL_SUCCESS;
   DTI_AgentCtx_t             *pDtiAgent = DSL_NULL;
   DTI_AgentStartupSettings_t dtiStartup;

   if (DSL_CPE_GlobalDtiContext.pDtiAgent == DSL_NULL)
   {
      /* init the DTI control struct */
      memset(&DSL_CPE_GlobalDtiContext, 0, sizeof(DSL_CPE_Dti_Context_t));
      DSL_CPE_GlobalDtiContext.ifNum = -1;


      memset(&dtiStartup, 0x00, sizeof(DTI_AgentStartupSettings_t));

      /* physical device setup */
      dtiStartup.numOfDevices   = (IFX_int_t)numOfPhyDevices;
      dtiStartup.linesPerDevice = (IFX_int_t)numOfLinesPerPhyDevice;


      /* DTI IP Setup setup */
      strncpy(dtiStartup.serverIpAddr, pDtiServerIp, 16);
      dtiStartup.listenPort = (dtiListenPort == 0) ? 9000 : (IFX_uint16_t)dtiListenPort;

      /* DTI Agent configuration */
      dtiStartup.bStartupAutoCliMsgSupport = (bEnableCliAutoMsg == DSL_TRUE) ? 1 : 0;
      dtiStartup.bStartupAutoDevMsgSupport = (bEnableDevAutoMsg == DSL_TRUE) ? 1 : 0;

      dtiStartup.debugLevel = 3;

#if defined(DTI_SUPPORT_SINGLE_THREADED_MODE) && (DTI_SUPPORT_SINGLE_THREADED_MODE == 1)
      /*
         Out from the DTI Agent interface:
            This version ot the DTI Agent supports "single threaded mode"
      */
      if (bEnableSingleThreadMode == DSL_TRUE)
      {
         dtiStartup.bSingleThreadedMode = 1;
         dtiStartup.numOfUsedWorker     = 1;
      }
      else
      {
         dtiStartup.bSingleThreadedMode = 0;
         dtiStartup.numOfUsedWorker     = 4;
      }
#endif

      /*
         start the DTI Agent
      */
      if ( DTI_AgentStart(&pDtiAgent, &dtiStartup) != DTI_SUCCESS)
      {
         DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"ERROR - start DTI agent" DSL_CPE_CRLF);

         return DSL_ERROR;
      }
      DSL_CPE_GlobalDtiContext.pDtiAgent = pDtiAgent;
   }
   else
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"WARNING - DTI agent already running" DSL_CPE_CRLF);
   }

#if defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
   if (DSL_CPE_GlobalDtiContext.bEnabled != DSL_TRUE)
   {
      DSL_CPE_GlobalDtiContext.pDSLContext = pContext;

      if (dtiStartup.bStartupAutoCliMsgSupport == 1)
      {
         ret = DSL_CPE_CLI_Register(
                  &DSL_CPE_GlobalDtiContext.pCLIContext,
                  &DSL_CPE_GlobalDtiContext,
                  DSL_CPE_CLI_Dti_Exit,
                  DSL_CPE_CLI_Dti_Event);
      }
      else
      {
         ret = DSL_CPE_CLI_Register(
                  &DSL_CPE_GlobalDtiContext.pCLIContext,
                  &DSL_CPE_GlobalDtiContext,
                  DSL_CPE_CLI_Dti_Exit,
                  IFX_NULL);
      }


      /* register the exec within the DTI module */
      DSL_CPE_GlobalDtiContext.ifNum =  DTI_CLI_SendFunctionRegister(
                                            (IFX_void_t *)DSL_CPE_GlobalDtiContext.pDtiAgent,
                                            (IFX_void_t *)&DSL_CPE_GlobalDtiContext,
                                            "DSL_API",
                                            DSL_CPE_CLI_Dti_Exec,
                                            0x10000 /* size of response buffer */);
      if (DSL_CPE_GlobalDtiContext.ifNum < 0)
      {
         DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"ERROR - start DTI agent, send fct register" DSL_CPE_CRLF);

         if (DSL_CPE_CLI_Unregister(DSL_CPE_GlobalDtiContext.pCLIContext) == DSL_SUCCESS)
         {
            DSL_CPE_Free(DSL_CPE_GlobalDtiContext.pCLIContext);
            DSL_CPE_GlobalDtiContext.pCLIContext = DSL_NULL;
         }

         return DSL_ERROR;
      }
   }
   else
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"WARNING - DTI agent already registered" DSL_CPE_CRLF);
   }
#endif /* #if defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/

   DSL_CPE_GlobalDtiContext.bEnabled = DSL_TRUE;

   return ret;
}

/**
   Initialise the CLI DTI
*/
DSL_Error_t DSL_CPE_Dti_Stop(
                        DSL_CPE_Control_Context_t *pContext)
{
   DTI_AgentCtx_t             *pDtiAgent = DSL_NULL;

   if (DSL_CPE_GlobalDtiContext.bEnabled == DSL_TRUE)
   {
      pDtiAgent = DSL_CPE_GlobalDtiContext.pDtiAgent;
      DSL_CPE_GlobalDtiContext.pDtiAgent = DSL_NULL;

      if ( DTI_AgentStop(&pDtiAgent) != DTI_SUCCESS)
      {
         DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"ERROR - Stop DTI agent" DSL_CPE_CRLF);

         return DSL_ERROR;
      }

      if (DSL_CPE_CLI_Unregister(DSL_CPE_GlobalDtiContext.pCLIContext) == DSL_SUCCESS)
      {
         DSL_CPE_Free(DSL_CPE_GlobalDtiContext.pCLIContext);
         DSL_CPE_GlobalDtiContext.pCLIContext = DSL_NULL;
      }
   }
   else
   {
      DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_PREFIX"ERROR - Stop DTI agent, not running" DSL_CPE_CRLF);

      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

#endif   /* #if defined(INCLUDE_DSL_CPE_DTI_SUPPORT) */
