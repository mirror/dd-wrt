/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _lint
/*
   Includes
*/
#include "dsl_cpe_control.h"
#include "dsl_cpe_os.h"
#include "dsl_cpe_cli.h"

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT

#include "stdsoap2.h"
#include "dsl_cpe_soap_Stub.h"


#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_SOAP


/* The namespace mapping table is required and associates namespace prefixes with namespace names: */
struct Namespace namespaces[] =
{
  {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/"},   /* MUST be first */
  {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/"},   /* MUST be second */
  {"xsi", "http://www.w3.org/1999/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance"},  /* MUST be third */
  {"xsd", "http://www.w3.org/1999/XMLSchema", "http://www.w3.org/*/XMLSchema"},
  {"ifx", "urn:dsl_api"},  /* Method namespace URI */
  {NULL, NULL}
};

#define DSL_SOAPCBWAIT_TIMEOUT         3000

/** port used by SOAP interface */
static const int DSL_SoapPort = 8080;
/** SOAP server thread */
static DSL_int_t DSL_CPE_SOAP(DSL_CPE_Thread_Params_t *param);

/** static buffer, used for fmemopen operation */
#ifdef WIN32
static DSL_char_t soap_result[32768] = "";
#else
static DSL_char_t soap_result[0x10000] = "";
#endif
/** shutdown handler */
static DSL_Error_t DSL_CPE_Soap_Exit(DSL_void_t *pContext);
/** callback for events */
static DSL_Error_t SOAP_EventCallback(DSL_void_t *pContext, DSL_char_t *pMessage);

typedef struct Soap_Client_s Soap_Client_t;
typedef struct Soap_Server_s Soap_Server_t;
typedef struct Soap_env_s    Soap_env_t;

/** data structure for the SOAP client */
struct Soap_Client_s
{
   DSL_boolean_t bInit;
   struct soap soap;
};

struct Soap_Server_s
{
   struct soap soap;
   int m;            /* master socket */
   int s;            /* slave socket */
};

struct Soap_env_s
{
   /** soap service running flag */
   volatile DSL_boolean_t bRun;
   /** reference to registered CLI */
   DSL_CLI_Context_t *pSOAPCLIContext;
   DSL_CPE_Lock_t callBackSem;
   Soap_Server_t server;
   Soap_Client_t client;
   DSL_CPE_Control_Context_t *pContext;
   DSL_CPE_Thread_t pid;
};

static Soap_env_t SoapEnv /*= {0}*/;
/** SOAP server ip address */
extern DSL_char_t *sSoapRemoteServer;

DSL_CPE_ThreadCtrl_t SoapControl;

/******************************************************************************/

DSL_int32_t DSL_CPE_SoapInit(DSL_CPE_Control_Context_t *pContext)
{
   DSL_int32_t ret = 0;

   memset(&SoapEnv, 0, sizeof(Soap_env_t));

   soap_init(&SoapEnv.client.soap);
   /* DO NOT INCLUDE "SOAP_IO_KEEPALIVE" FLAG FOR CLIENT BECAUSE THIS MIGHT
      RESULT IN PROBLEMS!!! */
   /*soap_init2(&SoapEnv.client.soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);*/
   SoapEnv.client.bInit = DSL_TRUE;

   /*soap_init(&SoapEnv.server.soap);*/
   soap_init2(&SoapEnv.server.soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);

   SoapEnv.server.soap.accept_timeout = 60;  /* let server time out after 60 sec of inactivity */

   SoapEnv.server.soap.send_timeout = 10;    /* 10 seconds */
   SoapEnv.server.soap.recv_timeout = 10;    /* 10 seconds */
   SoapEnv.server.soap.max_keep_alive = 100; /* max keep-alive sequence (100 packets) */

   SoapEnv.server.m = soap_bind ( &SoapEnv.server.soap, NULL, DSL_CPE_SOAP_PORT, 100 );
   if ( SoapEnv.server.m < 0 )
   {
      /*soap_print_fault ( &SoapEnv.server.soap, stderr );*/
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "binding SOAP port (%d) failed - already in use?" DSL_CPE_CRLF,
         DSL_CPE_SOAP_PORT));
      return 1;
   }

   SoapEnv.bRun     = DSL_TRUE;
   SoapEnv.pContext = pContext;

   memset(&SoapControl, 0x00, sizeof(SoapControl));

   DSL_CPE_ThreadInit(&SoapControl, "dsl_soap", DSL_CPE_SOAP, DSL_CPE_SOAP_STACK_SIZE, DSL_CPE_PRIORITY, (DSL_uint32_t)&SoapEnv, (DSL_uint32_t)DSL_NULL);

   DSL_CPE_LockCreate(&SoapEnv.callBackSem);

   ret = DSL_CPE_CLI_Register(&SoapEnv.pSOAPCLIContext, &SoapEnv, DSL_CPE_Soap_Exit,
      SOAP_EventCallback);

   return ret;
}

/**
   Exit-Handler for CLI registered for SOAP
*/
static DSL_Error_t DSL_CPE_Soap_Exit(DSL_void_t *pContext)
{
   Soap_env_t *env = pContext;
   env->bRun = DSL_FALSE;

   /* only shutdown socket if not called inside own thread,
      otherwise it will crash if the answer should send to the calling client */
   if (env->pid != DSL_CPE_ThreadIdGet())
   {
      shutdown(env->server.m, 2);
      soap_closesock(&env->server.soap); /* close master socket and detach environment */
   }

   DSL_CPE_LockDelete(&(env->callBackSem));

   return DSL_SUCCESS;
}

DSL_int32_t DSL_CPE_SoapExit(DSL_void_t)
{
   SoapEnv.bRun = 0;

   return 0;
}

/**
   Forward event callbacks to the SOAP remote server, if configured and connected.
*/
static DSL_Error_t SOAP_EventCallback(DSL_void_t *pContext, DSL_char_t *pMessage)
{
   Soap_env_t *env = pContext;
   char *pResult = NULL;

   if (sSoapRemoteServer == DSL_NULL || pMessage == DSL_NULL || env->client.bInit == DSL_FALSE)
   {
      /** Not connected, silent ignore! */
      return DSL_ERROR;
   }

   if(DSL_CPE_LockTimedGet(&env->callBackSem, DSL_SOAPCBWAIT_TIMEOUT, DSL_NULL) != DSL_SUCCESS)
   {
      return DSL_ERROR;
   }

   soap_call_ifx__DslCpeEventCallback(&env->client.soap, sSoapRemoteServer, "", pMessage , &pResult);

   /* free the result pointer */
   soap_free(&env->client.soap);
   soap_end(&env->client.soap);

   DSL_CPE_LockSet(&env->callBackSem);

   if (env->client.soap.error)
   {
      soap_print_fault(&env->client.soap, stderr);
   }

   return DSL_SUCCESS;
}

static DSL_int_t DSL_CPE_SOAP(DSL_CPE_Thread_Params_t *param)
{
   Soap_env_t *pSoapEnv = (Soap_env_t *)param->nArg1;
   int s;    /* slave socket */
   int ret = 0;
   pSoapEnv->pid = DSL_CPE_ThreadIdGet();
   pSoapEnv->server.soap.user = (DSL_void_t *)pSoapEnv;

   if ( ((Soap_env_t *)param->nArg1) == DSL_NULL)
   {
      return -1;
   }

   while (pSoapEnv->bRun)
   {
      s = soap_accept (&pSoapEnv->server.soap);
      if ( s < 0 )
      {
         if (pSoapEnv->bRun == 0)
            break;
         /* timeout -> continue */
         /*DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
            "SOAP socket time out (error %d)" DSL_CPE_CRLF, s));*/
         continue;
      }

      if (soap_serve(&pSoapEnv->server.soap) != SOAP_OK) /* process RPC request */
      {
         /*soap_print_fault(&pSoapEnv->server.soap, stderr);*/ /* print error */
      }

      soap_destroy (&pSoapEnv->server.soap); /* clean up class instances */
      soap_end (&pSoapEnv->server.soap);
   }

   soap_end(&pSoapEnv->server.soap);

   soap_done(&pSoapEnv->server.soap); /* close master socket and detach environment */

   pSoapEnv->pid = 0;

   return ret;
}

int ifx__DslCpeCliAccess (struct soap *soap_server, char *pCommand, char **pReturn )
{
   char cmd[256] = "";
   char *pcArg = DSL_NULL;
   Soap_env_t *pSoapEnv = soap_server->user;
   DSL_CPE_File_t *stream = DSL_NULL;
   struct soap_multipart *attachment;
#ifdef DSL_CPE_SOAP_FW_UPDATE
   DSL_CPE_Firmware_t nFirmware;
   DSL_CPE_Firmware_t nFirmware2;

   memset(&nFirmware, 0, sizeof(nFirmware));
   memset(&nFirmware2, 0, sizeof(nFirmware2));
#endif /* DSL_CPE_SOAP_FW_UPDATE */

   if(pCommand == DSL_NULL)
   {
      sprintf(soap_result, "nReturn=-1 nError=\"oops, CLI command received via SOAP is zero\n\r\"");
      *pReturn = soap_result;
      return SOAP_OK;
   }

   /* get command name */
   sscanf(pCommand, "%s", cmd);

   if(strlen(cmd) < strlen(pCommand))
   {
      pcArg = pCommand + strlen(cmd) + 1;
   }

   for (attachment = soap_server->dime.list; attachment; attachment = attachment->next)
   {
   #ifdef DSL_CPE_SOAP_FW_UPDATE
      if(attachment->type && (strcmp(attachment->type, "image/firmware1") == 0))
      {
         fprintf(stderr,  "DIME attachment: firmware1.bin\n\r");
         nFirmware.pData = (DSL_uint8_t *)attachment->ptr;
         nFirmware.nSize = attachment->size;
      }
      else if(attachment->type && (strcmp(attachment->type, "image/firmware2") == 0))
      {
         fprintf(stderr,  "DIME attachment: firmware2.bin\n\r");
         nFirmware2.pData = (DSL_uint8_t *)attachment->ptr;
         nFirmware2.nSize = attachment->size;
      }
      else
      {
         fprintf(stderr,  "DSL: cannot decode DIME attachment\n\r");
         fprintf(stderr,  "DIME attachment:\n\r");
         fprintf(stderr,  "Memory=%p\n\r", attachment->ptr);
         fprintf(stderr,  "Size=%u\n\r", (unsigned int)attachment->size);
         fprintf(stderr,  "Type=%s\n\r", attachment->type ? attachment->type:"null");
         fprintf(stderr,  "ID=%s\n\r", attachment->id ? attachment->id:"null");
      }
   #else
      fprintf(stderr,  "Firmware download via DIME attachment not supported.\n\r");
   #endif
   }

#ifdef DSL_CPE_SOAP_FW_UPDATE
   if((nFirmware.pData != DSL_NULL) || (nFirmware2.pData != DSL_NULL))
   {
      DSL_CPE_SoapFirmwareUpdate(&nFirmware, &nFirmware2);
   }
#endif /* DSL_CPE_SOAP_FW_UPDATE */

   memset(soap_result, 0, sizeof(soap_result));

   stream = DSL_CPE_FMemOpen(soap_result, sizeof(soap_result)-1, "w");

   if(stream != DSL_NULL)
   {
      DSL_CPE_CliDeviceCommandExecute(pSoapEnv->pContext, -1, cmd, pcArg, stream);
   }
   else
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "cannot open SOAP stream" DSL_CPE_CRLF));
   }

   DSL_CPE_FClose(stream);

   *pReturn = soap_result;

   return SOAP_OK;
}

/* termination of remote events */
int ifx__DslCpeEventCallback (struct soap *soap_server, char *pCommand, char **pReturn )
{
   return 0;
}


const DSL_char_t g_sRsss[] =
#ifndef DSL_CPE_DEBUG_DISABLE
   "Long Form: %s" DSL_CPE_CRLF
   "Short Form: %s" DSL_CPE_CRLF DSL_CPE_CRLF
   "Input Parameter" DSL_CPE_CRLF
   "- DSL_char_t <server_ip:port>" DSL_CPE_CRLF
   "   -1: clear server data" DSL_CPE_CRLF DSL_CPE_CRLF "";
#else
   "";
#endif

/**
   Change the SOAP server at runtime

   \param <server_ip:port> (optional): can be -1 to clear the remote server data

   \return
   DSL_ERROR with parameter   : could not set new server data
             without parameter: output current server and help
   DSL_SUCCESS new server data set
*/
DSL_int_t DSL_CPE_SOAP_RemoteSoapServerSet(
   DSL_int_t fd,
   DSL_char_t *command,
   DSL_CPE_File_t *out)
{
   DSL_int_t     ret = DSL_SUCCESS;

   if (strlen(command)) /* a parameter was provided */
   {
       /* clear the old data */
      if (sSoapRemoteServer)
      {
         DSL_CPE_Free (sSoapRemoteServer);
         sSoapRemoteServer = DSL_NULL;
      }

      /* don't set new SOAP remote server data */
      if (strcmp(command, "-1") == 0)
      {
         /* the data is already cleared */
         ret = 0;
      }
      else
      {
         /* set the new server data */
         sSoapRemoteServer = DSL_CPE_Malloc(strlen (command) + 1);
         if (sSoapRemoteServer)
         {
            strcpy (sSoapRemoteServer, command);
            ret = (strcmp(command, sSoapRemoteServer) == 0) ? DSL_SUCCESS : DSL_ERROR;
         }
         else
         {
            ret = 0;
         }
      }
   }
   else /* print help text */
   {
      return -1;
   }

   if (sSoapRemoteServer)
   {
      DSL_CPE_FPrintf(out, "Using remote soap server: '%s'" DSL_CPE_CRLF , sSoapRemoteServer );
   }
   else
   {
      DSL_CPE_FPrintf(out, "Using no remote soap server!" DSL_CPE_CRLF);
   }
   DSL_CPE_FPrintf(out, "nReturn=%d" DSL_CPE_CRLF , ret );

   return 0;
}


#endif /* INCLUDE_DSL_CPE_SOAP_SUPPORT */

#endif /* _lint*/
