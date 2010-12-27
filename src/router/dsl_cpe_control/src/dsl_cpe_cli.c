/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL daemon command line interface
*/

/*#define DSL_INTERN*/

#include "dsl_cpe_os.h"
#include "dsl_cpe_cli.h"
#include "dsl_cpe_control.h"

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_CLI


static DSL_void_t DSL_CPE_CLI_Cleanup(void);

static DSL_int_t DSL_CPE_CLI_CheckHelp
(
   const DSL_char_t *pCommands
);

static DSL_Error_t DSL_CPE_CLI_PrintHelp
(
   const DSL_char_t *psHelp,
   DSL_char_t *psCmdLong,
   DSL_char_t *psCmdShort,
   DSL_uint32_t nCmdMask,
   DSL_CPE_File_t *out
);

/** 'less then' defintion for binary tree, (a < b)*/
#define compLT(a,b) (strcmp(a,b) < 0)
/** 'equal' defintion for binary tree, (a == b)*/
#define compEQ(a,b) (strcmp(a,b) == 0)

#define DSL_CLI_HELP_NOT_AVAILABLE "nReturn=-1 (wrong number of parameters/help not available) \n"
#define DSL_CLI_HELP_MEMORY_ERROR  "nReturn=-1 (help string memory allocation error) \n"

#define DSL_CLI_HELP_NOTE_FUNCTION_DEPRECATED  "Note: This function is deprecated! \n"

/** implementation dependend declarations */
typedef enum
{
   DSL_CPE_STATUS_OK,
   DSL_CPE_STATUS_MEM_EXHAUSTED,
   DSL_CPE_STATUS_DUPLICATE_KEY,
   DSL_CPE_STATUS_KEY_NOT_FOUND,
   DSL_CPE_STATUS_KEY_INVALID
} DSL_CPE_statusEnum;

/** type of key */
typedef char* DSL_CPE_keyType;

/** user data stored in tree */
typedef struct
{
   DSL_char_t *sCmdShort;
   DSL_char_t *sCmdLong;
   const DSL_char_t *psHelp;
   unsigned int mask;
   DSL_int_t (*func)(DSL_int_t, DSL_char_t*, DSL_CPE_File_t*);
} DSL_CPE_recType;

typedef struct DSL_CPE_nodeTag
{
   /* left child */
   struct DSL_CPE_nodeTag *left;
   /** right child */
   struct DSL_CPE_nodeTag *right;
   /** parent */
   struct DSL_CPE_nodeTag *parent;
   /** key used for searching */
   DSL_CPE_keyType key;
   /** user data */
   DSL_CPE_recType rec;
} DSL_CPE_nodeType;

static DSL_CPE_statusEnum DSL_CPE_keyInsert
(
   DSL_CPE_keyType key,
   DSL_CPE_recType *rec
);

static void DSL_CPE_treeResourceUsageGet
(
   DSL_CPE_nodeType *node,
   unsigned int *pResStatic,
   unsigned int *pResDynamic
);

static DSL_CPE_statusEnum DSL_CPE_keyDelete
(
   DSL_CPE_keyType key
);

DSL_CPE_statusEnum DSL_CPE_keyFind
(
   DSL_CPE_keyType key,
   DSL_CPE_recType *rec
);

static void DSL_CPE_treePrint
(
   DSL_CPE_nodeType *node,
   unsigned int mask,
   DSL_CPE_File_t *
);

static void DSL_CPE_treeDelete
(
   DSL_CPE_nodeType *node
);

static void DSL_CPE_nodeDelete
(
   DSL_CPE_nodeType *node
);

/** root of binary tree */
DSL_CPE_nodeType *root_node = DSL_NULL;

struct DSL_CLI_Context
{
   /**
   pointer for list */
   DSL_CLI_Context_t *next;
   /**
   Context for CLI Callbacks */
   DSL_void_t *pCBContext;
   /**
   Callback for CLI Shutdown */
   DSL_CPE_Exit_Callback_t pExitCallback;
   /**
   DSL_CPE_API Event Callback, may be DSL_NULL */
   DSL_CLI_Event_Callback_t pEventCallback;
};

DSL_char_t CLI_EventText[16000];

static DSL_CLI_Context_t *CLI_List_head = DSL_NULL;

/** indicates if the CLI already initialized */
static DSL_boolean_t bCLI_Init = DSL_FALSE;

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
   extern const DSL_char_t g_sRsss[];
   extern DSL_int_t DSL_CPE_SOAP_RemoteSoapServerSet(DSL_int_t fd,
      DSL_char_t *command, DSL_CPE_File_t *out);
#endif


/******************************************************************************/

DSL_Error_t DSL_CPE_CLI_Init(DSL_void_t)
{
   if(bCLI_Init == DSL_FALSE)
   {
      DSL_CPE_CLI_CMD_ADD_COMM("help", "Help", DSL_CPE_CLI_HelpPrint, DSL_NULL);
      DSL_CPE_CLI_CMD_ADD_COMM("quit", "Quit", DSL_NULL, DSL_NULL);
#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
      DSL_CPE_CLI_CMD_ADD_COMM("rsss", "RemoteSoapServerSet", DSL_CPE_SOAP_RemoteSoapServerSet, g_sRsss);
#endif
      DSL_CPE_CLI_AccessCommandsRegister();

      bCLI_Init = DSL_TRUE;
   }
   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_CLI_Shutdown(DSL_void_t)
{
   DSL_CPE_CLI_CommandClear();

   DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "Goodbye from DSL CPE API CLI "
      "interface" DSL_CPE_CRLF );

   bCLI_Init = DSL_FALSE;

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_CLI_HandleEvent(DSL_char_t *pMsg)
{
   DSL_CLI_Context_t *pCLIList = CLI_List_head;

   while (pCLIList != DSL_NULL)
   {
      if (pCLIList->pEventCallback != DSL_NULL)
      {
         (void)pCLIList->pEventCallback(pCLIList->pCBContext, CLI_EventText);
      }
      pCLIList = pCLIList->next;
   };

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_CLI_Register(
   DSL_CLI_Context_t **pNewCLIContext,
   DSL_void_t *pCBContext,
   DSL_CPE_Exit_Callback_t pExitCallback,
   DSL_CLI_Event_Callback_t pEventCallback)
{
   DSL_CLI_Context_t *pCLIContext;

   if(*pNewCLIContext != DSL_NULL)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "expecting zero context pointer "
         "'*pNewCLIContext'" DSL_CPE_CRLF);
      return DSL_ERROR;
   }

   *pNewCLIContext = malloc(sizeof(DSL_CLI_Context_t));
   if(*pNewCLIContext == DSL_NULL)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "no memory for '*pNewCLIContext'"
         DSL_CPE_CRLF);
      return DSL_ERROR;
   }
   pCLIContext = *pNewCLIContext;

   memset(pCLIContext, 0x00, sizeof(*pCLIContext));

   pCLIContext->next = DSL_NULL;
   pCLIContext->pCBContext = pCBContext;
   pCLIContext->pExitCallback = pExitCallback;
   pCLIContext->pEventCallback = pEventCallback;

   if (CLI_List_head == DSL_NULL)
      CLI_List_head = pCLIContext;
   else
   {
      DSL_CLI_Context_t *pCLI_List = CLI_List_head;
      while (pCLI_List->next != DSL_NULL)
      {
         pCLI_List = pCLI_List->next;
      }
      pCLI_List->next = pCLIContext;
   }

   return DSL_SUCCESS;
}

#if defined(INCLUDE_DSL_API_CONSOLE_EXTRA) || defined(INCLUDE_DSL_CPE_DTI_SUPPORT)
DSL_Error_t DSL_CPE_CLI_Unregister(DSL_CLI_Context_t *pCLIContext)
{
   DSL_CLI_Context_t *pCLIList=DSL_NULL;

   if(pCLIContext == DSL_NULL)
   {
      return DSL_SUCCESS;
   }

   if (CLI_List_head == pCLIContext)
   {
      CLI_List_head = pCLIContext->next;
      return DSL_SUCCESS;
   }
   else
   {
      pCLIList = CLI_List_head;
      while (pCLIList != DSL_NULL)
      {
         if (pCLIList->next == pCLIContext)
         {
            /* entry found, remove from list */
            pCLIList->next = pCLIContext->next;
            return DSL_SUCCESS;
         }
         pCLIList = pCLIList->next;
      }
   }

   return DSL_ERROR;
}
#endif /* #if defined(INCLUDE_DSL_API_CONSOLE_EXTRA) || defined(INCLUDE_DSL_CPE_DTI_SUPPORT)*/

/**
   Cleanup the CLI (and stop the DSL CPE API)
*/
static DSL_void_t DSL_CPE_CLI_Cleanup(void)
{
   DSL_CLI_Context_t *pCLIList = CLI_List_head;
   DSL_CLI_Context_t *pCLI_del;

   CLI_List_head = DSL_NULL;
   while (pCLIList != DSL_NULL)
   {
      if (pCLIList->pExitCallback != DSL_NULL)
      {
         /*DSL_CPE_FPrintf(DSL_CPE_STDERR, "CLI: calling pExitCallback()" DSL_CPE_CRLF );*/
         (void)pCLIList->pExitCallback(pCLIList->pCBContext);
      }
      pCLI_del = pCLIList;
      pCLIList = pCLIList->next;
      free(pCLI_del);
   };
}

/**
   Checks if the command string includes option to display help text.

   \param pCommands
      user command line arguments

   \return
      returns 0 in case of no help request or any other value if help is
      requested from user.
*/
static DSL_int_t DSL_CPE_CLI_CheckHelp (
   const DSL_char_t * pCommands)
{
   if (pCommands && (strstr (pCommands, "-h") || strstr (pCommands, "--help") ||
         strstr (pCommands, "/h") || strstr (pCommands, "-?")))
   {
      return -1;
   }
   return 0;
}

static DSL_Error_t DSL_CPE_CLI_PrintHelp(
   const DSL_char_t *psHelp,
   DSL_char_t *psCmdLong,
   DSL_char_t *psCmdShort,
   DSL_uint32_t nCmdMask,
   DSL_CPE_File_t *out)
{
#ifndef DSL_CPE_DEBUG_DISABLE
   DSL_char_t *sHelp = DSL_NULL;

   if (psHelp == DSL_NULL)
   {
      return DSL_ERROR;
   }

   sHelp = (DSL_char_t*)DSL_CPE_Malloc(4096);
   if (sHelp == DSL_NULL)
   {
      DSL_CPE_FPrintf(out, DSL_CLI_HELP_MEMORY_ERROR);
      return DSL_ERROR;
   }

   if (nCmdMask & DSL_CPE_MASK_DEPRECATED)
   {
      DSL_CPE_FPrintf (out, DSL_CLI_HELP_NOTE_FUNCTION_DEPRECATED);
   }

   sprintf(sHelp, psHelp, psCmdLong, psCmdShort);
   
   DSL_CPE_FPrintf (out, "%s", sHelp);

   DSL_CPE_Free(sHelp);
#else
   DSL_CPE_FPrintf (out, DSL_CLI_HELP_NOT_AVAILABLE);
#endif /* DSL_CPE_DEBUG_DISABLE */

   return DSL_SUCCESS;
}

/**
   Checks a given command list (string) according to the included number of
   parameter.

   \param pCommands
      specifies a pointer to a list of parameters
   \param nParams
      specifies the expected number of parameters that has to be included
      within the command list

   \return
      Returns DSL_TRUE if the number of scanned parameters equals to the
      given value of nParams otherwise returns DSL_FALSE
*/
DSL_boolean_t DSL_CPE_CLI_CheckParamNumber(
   DSL_char_t *pCommands,
   DSL_int_t nParams,
   DSL_CLI_ParamCheckType_t nCheckType)
{
   DSL_char_t string[256] = { 0 };
   DSL_char_t seps[] = " ";
   DSL_boolean_t bRet = DSL_FALSE;
   DSL_char_t *token;
   DSL_int_t i = 0;

   strncpy (string, pCommands, sizeof(string)-1);
   string[sizeof(string)-1]=0;

   /* Get first token */
   token = strtok (string, seps);
   if (token != DSL_NULL)
   {
      for (i = 1; ; i++)
      {
         /* Get next token */
         token = strtok(DSL_NULL, seps);

         /* Exit scanning if no further information is included */
         if (token == DSL_NULL)
         {
            break;
         }
      }
   }

   bRet = DSL_FALSE;
   switch (nCheckType)
   {
   case DSL_CLI_EQUALS:
      if (i == nParams) bRet =DSL_TRUE;
      break;
   case DSL_CLI_MIN:
      if (i >= nParams) bRet = DSL_TRUE;
      break;
   case DSL_CLI_MAX:
      if (i <= nParams) bRet = DSL_TRUE;
      break;
   default:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX "DSL_CPE_CLI_CheckParamNumber: "
         "unknown check type" DSL_CPE_CRLF);
      break;
   }

   return bRet;
}

/**
   Inform the user about the built in commands.

   \param command not used
*/
DSL_int_t DSL_CPE_CLI_HelpPrint(
   DSL_int_t fd,
   DSL_char_t *command,
   DSL_CPE_File_t *out)
{
   unsigned int mask = 0;
   DSL_CPE_File_t *target = out;
   #ifdef WIN32
   DSL_CPE_File_t *file = DSL_NULL;
   #endif

   if( (command != DSL_NULL) && ( strstr(command,"--help") ||
       strstr(command,"-h")) )
   {
      DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "help [-h | --help"
   #ifdef DSL_CPE_MASK_LONG
      " | -l | --long |"
   #endif
   #ifdef WIN32
      " | file |"
   #endif
      " all | device | g997 | pm |"
   #ifdef INCLUDE_DEPRECATED
      " deprecated |"
   #endif
      " detailed]" DSL_CPE_CRLF );

      return 0;
   }

   if(command)
   {
   #ifdef WIN32
      if(strstr(command, "file") != 0)
      {
         file = DSL_CPE_FOpen("cli_help.txt","a+");
         target = file;
      }
   #endif

   #ifdef DSL_CPE_MASK_LONG
      if((strstr(command, "-l") != 0) || (strstr(command, "--long") != 0) )
         mask = DSL_CPE_MASK_LONG;
   #endif
      if(strstr(command, "detailed") != 0)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "detailed information" DSL_CPE_CRLF );
         mask |= DSL_CPE_MASK_DETAILED;
      }
      #ifdef INCLUDE_DEPRECATED
      if(strstr(command, "deprecated") != 0)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "deprecated functions" DSL_CPE_CRLF );
         mask |= DSL_CPE_MASK_DEPRECATED;
      }
      #endif
      if(strstr(command, "device") != 0)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "device related functions" DSL_CPE_CRLF );
         DSL_CPE_treePrint(root_node, mask | DSL_CPE_MASK_DEVICE, target);
      }
      if(strstr(command, "g997") != 0)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "G997 related functions" DSL_CPE_CRLF );
         DSL_CPE_treePrint(root_node, mask | DSL_CPE_MASK_G997, target);
      }
      if(strstr(command, "pm") != 0)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "performance related functions" DSL_CPE_CRLF );
         DSL_CPE_treePrint(root_node, mask | DSL_CPE_MASK_PM, target);
      }
      if(strstr(command, "sar") != 0)
      {
         DSL_CPE_FPrintf(out, DSL_CPE_PREFIX "SAR related functions" DSL_CPE_CRLF );
         DSL_CPE_treePrint(root_node, mask | DSL_CPE_MASK_SAR, target);
      }
      if(strlen(command)==0 || strstr(command, "all") != 0)
      {
         DSL_CPE_treePrint(root_node, mask | DSL_CPE_MASK_ALL, target);
      }
   }
   else
   {
      DSL_CPE_treePrint(root_node, DSL_CPE_MASK_ALL | mask, target);
   }

   #ifdef WIN32
   if(file != DSL_NULL)
      DSL_CPE_FClose(file);
   #endif

   return 0;
}

/**
   Execute command.

   \param name Command name
   \param command Command line (optional parameters)
*/
DSL_int_t DSL_CPE_CLI_CommandExecute(
   DSL_int_t fd,
   DSL_char_t *cmd,
   DSL_char_t *arg,
   DSL_CPE_File_t *out)
{
   DSL_CPE_recType  rec = {DSL_NULL, DSL_NULL, DSL_NULL, 0, DSL_NULL};
   DSL_char_t dummy_arg[10] = "";

   if(cmd == DSL_NULL)
   {
      DSL_CPE_CLI_HelpPrint (fd, "all", out);
      return -1;
   }

   cmd = DSL_CPE_CLI_WhitespaceRemove(cmd);
   if(arg != DSL_NULL)
   {
      arg = DSL_CPE_CLI_WhitespaceRemove(arg);
   }
   else
      arg = dummy_arg;

   switch(DSL_CPE_keyFind(cmd, &rec))
   {
      case DSL_CPE_STATUS_OK:
      if(rec.func == DSL_NULL)
      {
         /* this is an internal function */
         if (strcmp(cmd, "quit") == 0)
         {
            DSL_CPE_CLI_Cleanup();
            /* main() will execute exit */
            return 1;
         }
         DSL_CPE_FPrintf(out, DSL_CPE_CRLF "Error: command \"%s\" without callback" DSL_CPE_CRLF ,cmd);
      }
      else
      {
         if ((rec.psHelp != DSL_NULL) && (DSL_CPE_CLI_CheckHelp(arg) != 0))
         {
            DSL_CPE_CLI_PrintHelp(rec.psHelp, rec.sCmdLong, rec.sCmdShort, rec.mask, out);
         }
         else
         {
            if (rec.func(fd, arg, out) != 0)
            {
               DSL_CPE_CLI_PrintHelp(rec.psHelp, rec.sCmdLong, rec.sCmdShort, rec.mask, out);
            }
         }
      }
      break;

      default:
#if 0 /* #ifdef LINUX */
/* FIXME: This leads into a crash on the RefBoard */
      {
         DSL_int_t   k=0;
         DSL_char_t  *argv[DSL_MAX_ARGS];
         DSL_char_t  *ptr;
         DSL_char_t  file_name[64], *brkt;
         DSL_int_t   status;

         /* try to call external utility */
         argv[k++] = cmd;

         ptr = strtok_r(arg, " ", &brkt);

         while(ptr && (k<(DSL_MAX_ARGS-1)))
         {
            argv[k++] = ptr;
            ptr = strtok_r(DSL_NULL, " ", &brkt);
         }

         argv[k] = DSL_NULL;

         switch (fork())
         {
         case -1:
            DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX "fork failed" DSL_CPE_CRLF );
            break;

         case 0:
            sprintf(file_name,"/bin/%s",cmd);
            execv(file_name,argv);

            sprintf(file_name,"/usr/bin/%s",cmd);
            execv(file_name,argv);

            sprintf(file_name,"/opt/ifx/%s",cmd);
            execv(file_name,argv);

            DSL_CPE_FPrintf(DSL_CPE_STDERR, "%s: command not found" DSL_CPE_CRLF,cmd);
            exit(1);
            break;

         default:
            wait(&status);
            break;
         }
      }
      return 0;
#else
      DSL_CPE_FPrintf(out, "%s: command not found" DSL_CPE_CRLF, cmd);
      DSL_CPE_CLI_HelpPrint (fd, "all", out);
      return -1;
#endif /* LINUX */
   }
   return 0;
}

/**
   Add a command to the static list.

   \param name Command name
   \param help Help string
   \param func Command entry point

   \return
   DSL_ERROR no more space left in command table
   DSL_SUCCESS command added to the command table
*/
DSL_Error_t DSL_CPE_CLI_CommandAdd(
   DSL_char_t *name,
   DSL_char_t *long_name,
   DSL_int_t (*func)(DSL_int_t, DSL_char_t*, DSL_CPE_File_t*),
   const DSL_char_t *psHelp,
   DSL_uint32_t nCmdSortMask)
{
   DSL_CPE_recType rec;
   DSL_char_t buf[64]="";
   DSL_uint8_t i=0,k=0;

   rec.mask = 0;

   if(name == DSL_NULL)
   {
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " name pointer is invalid" DSL_CPE_CRLF );
      return DSL_ERROR;
   }

   if(long_name == DSL_NULL)
   {
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " long_name pointer is invalid" DSL_CPE_CRLF );
      return DSL_ERROR;
   }

   for(i = 0; i < strlen(long_name); i++)
   {
      if(long_name[i] >= 'A' && long_name[i] <= 'Z')
      {
         buf[k++] = 'a' + (long_name[i] - 'A');
      }
      else if(long_name[i] >= '0' && long_name[i] <= '9')
      {
         buf[k++] = long_name[i];
      }
   }
   buf[k] = 0;

   if(strcmp(buf, name) != 0)
   {
      if( ! ((strcmp(long_name,"Help") == 0) || (strcmp(long_name,"Quit") == 0)) )
      {
         DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " CLI short name mismatch %s / %s / %s " DSL_CPE_CRLF, name, buf, long_name );
      }
   }

   rec.func = func;

   if(strstr(name, "g997") != 0)
   {
      rec.mask |= DSL_CPE_MASK_G997;
   }
   else if(long_name[0] == 'P' && long_name[1] == 'M')
   {
      rec.mask |= DSL_CPE_MASK_PM;
   }
   else if(strstr(name, "sar") != 0)
   {
      rec.mask |= DSL_CPE_MASK_SAR;
   }
   else
   {
      rec.mask |= DSL_CPE_MASK_DEVICE;
   }

   rec.mask |= nCmdSortMask;

   rec.sCmdShort = malloc(strlen(name)+1);
   if(rec.sCmdShort)
   {
      strcpy(rec.sCmdShort, name);
      rec.sCmdShort[strlen(name)] = 0;
   }

   rec.sCmdLong = malloc(strlen(long_name)+1);
   if(rec.sCmdLong)
   {
      strcpy(rec.sCmdLong, long_name);
      rec.sCmdLong[strlen(long_name)] = 0;
   }

   if(rec.sCmdShort == DSL_NULL || rec.sCmdLong == DSL_NULL)
   {
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;
   }

   rec.psHelp = psHelp;

   switch(DSL_CPE_keyInsert(rec.sCmdShort, &rec))
   {
      case DSL_CPE_STATUS_KEY_INVALID:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " invalid key %s for %s" DSL_CPE_CRLF , rec.sCmdShort, rec.sCmdLong);
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;

      case DSL_CPE_STATUS_DUPLICATE_KEY:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " duplicate key %s for %s" DSL_CPE_CRLF , rec.sCmdShort, rec.sCmdLong);
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;

      case DSL_CPE_STATUS_MEM_EXHAUSTED:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " memory error" DSL_CPE_CRLF );
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;

      case DSL_CPE_STATUS_OK:
      break;

      default:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " insert key aborted" DSL_CPE_CRLF );
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;
   }

#ifdef DSL_CPE_MASK_LONG
   rec.mask |= DSL_CPE_MASK_LONG;

   switch(DSL_CPE_keyInsert(rec.sCmdLong, &rec))
   {
      case DSL_CPE_STATUS_KEY_INVALID:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " invalid key %s for %s" DSL_CPE_CRLF , rec.sCmdShort, rec.sCmdLong);
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;

      case DSL_CPE_STATUS_DUPLICATE_KEY:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " duplicate key %s for %s" DSL_CPE_CRLF , rec.sCmdShort, rec.sCmdLong);
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;

      case DSL_CPE_STATUS_MEM_EXHAUSTED:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " memory error" DSL_CPE_CRLF );
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;

      case DSL_CPE_STATUS_OK:
      break;

      default:
      DSL_CPE_FPrintf(DSL_CPE_STDERR, DSL_CPE_PREFIX " insert key aborted" DSL_CPE_CRLF );
      free(rec.sCmdShort);
      free(rec.sCmdLong);
      return DSL_ERROR;
   }
#endif

   return DSL_SUCCESS;
}


/**
   Clean command  list.

   \return
   DSL_SUCCESS successfull operation
*/
DSL_Error_t DSL_CPE_CLI_CommandClear(DSL_void_t)
{
   DSL_CPE_nodeType *tmp_root = root_node;

   /*root_node = DSL_NULL;*/
   DSL_CPE_treeDelete(tmp_root);

   return DSL_SUCCESS;
}

#ifdef INCLUDE_DSL_RESOURCE_STATISTICS
DSL_Error_t DSL_CPE_CLI_ResourceUsageGet(
   DSL_CLI_ResourceUsageStatisticsData_t *pData)
{
   DSL_Error_t ret = DSL_SUCCESS;
   unsigned int ResStatic = 0, ResDynamic = 0;
   
   if (pData == DSL_NULL)
   {
      return DSL_ERROR;
   }

   pData->staticMemUsage  = 0;
   pData->dynamicMemUsage = 0;

   pData->staticMemUsage += sizeof(CLI_EventText) ;

   DSL_CPE_treeResourceUsageGet(root_node, &ResStatic, &ResDynamic);

   pData->staticMemUsage  += ResStatic;
   pData->dynamicMemUsage += ResDynamic;
   
   return ret;
}
#endif /* INCLUDE_DSL_RESOURCE_STATISTICS*/

static void DSL_CPE_treeResourceUsageGet(
   DSL_CPE_nodeType *node,
   unsigned int *pResStatic,
   unsigned int *pResDynamic)
{
   if(node == DSL_NULL)
      return;

   DSL_CPE_treeResourceUsageGet(node->left, pResStatic, pResDynamic);

   if (node->rec.psHelp)
   {
      *pResStatic  += strlen(node->rec.psHelp);
   }

   *pResDynamic += sizeof(DSL_CPE_nodeType);   
   if (node->key)
   {
      *pResDynamic += strlen(node->key);
   }

   if (node->rec.sCmdShort)
   {
      *pResDynamic += strlen(node->rec.sCmdShort);
   }

   if (node->rec.sCmdLong)
   {
      *pResDynamic += strlen(node->rec.sCmdLong);
   }

   DSL_CPE_treeResourceUsageGet(node->right, pResStatic, pResDynamic);

   return;
}


/**
   allocate node for data and insert in tree
*/
static DSL_CPE_statusEnum DSL_CPE_keyInsert(DSL_CPE_keyType key, DSL_CPE_recType *rec)
{
    DSL_CPE_nodeType *x, *current, *parent;

    if(key == DSL_NULL)
      return DSL_CPE_STATUS_KEY_INVALID;

    /* find future parent */
    current = root_node;
    parent = 0;
    while (current) {
        if (compEQ(key, current->key))
            return DSL_CPE_STATUS_DUPLICATE_KEY;
        parent = current;
        current = compLT(key, current->key) ?
            current->left : current->right;
    }

    /* setup new node */
    if ((x = malloc(sizeof(*x))) == 0) {
        return DSL_CPE_STATUS_MEM_EXHAUSTED;
    }
    x->parent = parent;
    x->left = DSL_NULL;
    x->right = DSL_NULL;
    x->key = key;
    memcpy ((void *)&x->rec, (void *)rec,  sizeof(DSL_CPE_recType));

    /* insert x in tree */
    if(parent)
        if(compLT(x->key, parent->key))
            parent->left = x;
        else
            parent->right = x;
    else
        root_node = x;

    return DSL_CPE_STATUS_OK;
}

/**
   delete node from tree
*/
static DSL_CPE_statusEnum DSL_CPE_keyDelete(DSL_CPE_keyType key)
{
    DSL_CPE_nodeType *x, *y, *z;

    /* find node in tree */
    z = root_node;

    while(z != DSL_NULL) {
        if(compEQ(key, z->key))
            break;
        else
            z = compLT(key, z->key) ? z->left : z->right;
    }

    if (!z) return DSL_CPE_STATUS_KEY_NOT_FOUND;

    /* find tree successor */
    if (z->left == DSL_NULL || z->right == DSL_NULL)
        y = z;
    else {
        y = z->right;
        while (y->left != DSL_NULL) y = y->left;
    }

    /* x is y's only child */
    if (y->left != DSL_NULL)
        x = y->left;
    else
        x = y->right;

    /* remove y from the parent chain */
    if (x) x->parent = y->parent;
    if (y->parent)
        if (y == y->parent->left)
            y->parent->left = x;
        else
            y->parent->right = x;
    else
        root_node = x;

    /* if z and y are not the same, replace z with y. */
    if (y != z) {
        y->left = z->left;
        if (y->left) y->left->parent = y;
        y->right = z->right;
        if (y->right) y->right->parent = y;
        y->parent = z->parent;
        if (z->parent)
            if (z == z->parent->left)
                z->parent->left = y;
            else
                z->parent->right = y;
        else
            root_node = y;
        DSL_CPE_nodeDelete(z);
    } else {
        DSL_CPE_nodeDelete(y);
    }

    return DSL_CPE_STATUS_OK;
}

/**
   find node containing data
*/
DSL_CPE_statusEnum DSL_CPE_keyFind(DSL_CPE_keyType key, DSL_CPE_recType *rec)
{
    DSL_CPE_nodeType *current = root_node;
    while(current != DSL_NULL)
    {
        if(compEQ(key, current->key))
        {
            memcpy ((void *)rec, (void *)&current->rec,  sizeof(DSL_CPE_recType));

            return DSL_CPE_STATUS_OK;
        }
        else
        {
            current = compLT(key, current->key) ?
                current->left : current->right;
        }
    }
    return DSL_CPE_STATUS_KEY_NOT_FOUND;
}


/**
   print binary tree
*/
static void DSL_CPE_treePrint(
   DSL_CPE_nodeType *node,
   unsigned int mask,
   DSL_CPE_File_t *out)
{
   DSL_int_t j = 0;
   DSL_int_t nChar = 0, nFillChar = 0;
   DSL_int_t nHelpClm = 18;

   if(node == DSL_NULL)
      return;

   DSL_CPE_treePrint(node->left, mask, out);

   while(1)
   {
#ifdef INCLUDE_DEPRECATED
      if ( (mask & DSL_CPE_MASK_DEPRECATED) && !(node->rec.mask & DSL_CPE_MASK_DEPRECATED) )
         break;
#endif /* INCLUDE_DEPRECATED*/

      if(node->rec.mask & (mask & (~DSL_CPE_MASK_DEPRECATED)))
      {
         if((mask & DSL_CPE_MASK_DETAILED) == DSL_CPE_MASK_DETAILED)
         {
            if(node->rec.func)
            {
            #ifndef DSL_CPE_DEBUG_DISABLE

            #ifdef DSL_CPE_MASK_LONG
               if((node->rec.mask & DSL_CPE_MASK_LONG) == DSL_CPE_MASK_LONG)
            #endif
                  DSL_CPE_CLI_PrintHelp(node->rec.psHelp, node->rec.sCmdLong,
                     node->rec.sCmdShort, node->rec.mask, out);
            #else
               DSL_CPE_FPrintf (out, DSL_CLI_HELP_NOT_AVAILABLE);
            #endif /* DSL_CPE_DEBUG_DISABLE */
            }
         }
         else
         {
            #ifdef DSL_CPE_MASK_LONG
            /*
            if(node->rec.mask & DSL_CPE_MASK_LONG)
               nHelpClm *= 3;
            */

            /*if((node->rec.mask & DSL_CPE_MASK_LONG) == (mask & DSL_CPE_MASK_LONG))*/
            if((node->rec.mask & DSL_CPE_MASK_LONG) == DSL_CPE_MASK_LONG)
            #endif
            {
               /*nChar = DSL_CPE_FPrintf(out, "   %s,", node->key);*/
               nChar = DSL_CPE_FPrintf(out, "   %s,", node->rec.sCmdShort);
               nFillChar = nHelpClm - nChar;
               if (nFillChar > 0)
               {
                  for (j = 0; j < nFillChar; j++) DSL_CPE_FPrintf(out, " ");
               }
               DSL_CPE_FPrintf(out, "%s" DSL_CPE_CRLF , node->rec.sCmdLong);
            }
         }
      }

      break;   
   } /* while(1)*/

   DSL_CPE_treePrint(node->right, mask, out);
      return;
}

/**
   delete complete binary tree
*/
static void DSL_CPE_treeDelete(DSL_CPE_nodeType *node)
{
   if(node != DSL_NULL)
   {
      DSL_CPE_treeDelete(node->left);
      DSL_CPE_treeDelete(node->right);
      DSL_CPE_keyDelete(node->key);
   }
}

/**
   delete node of binary tree
*/
static void DSL_CPE_nodeDelete(DSL_CPE_nodeType *node)
{
   if(node != DSL_NULL)
   {
      if (node->key == node->rec.sCmdLong)
      {
         free(node->rec.sCmdLong);
      }
      else if (node->key == node->rec.sCmdShort)
      {
         free(node->rec.sCmdShort);      
      }
     
      free(node);
   }
}


/**
   Remove leading, tailing and double whitespaces
   Arguments has to have only one space in between to get sure that the CLI
   comments are working correctly.
*/
char *DSL_CPE_CLI_WhitespaceRemove(char *str)
{
   char *buf;
   char *pRead, *pWrite;
   int i = 0;

   /* remove leading whitespaces */
   for (buf = str; buf && *buf && isspace((int)(*buf)); ++buf)
   {
      ;
   }

   /* remove double spaces in between and at the end */
   pRead = buf;
   pWrite = buf;
   for( i = 0; pWrite && pRead && *pRead != '\0'; ++pRead )
   {
      if (isspace((int)(*pRead)))
      {
         if ( (i == 0) && (*(pRead + 1) != '\0') )
         {
            *pWrite = *pRead;
            pWrite++;
            i++;
         }
      }
      else
      {
         i = 0;
         *pWrite = *pRead;
         pWrite++;
      }
   }

   /* Write string termination */
   if (pWrite && (pWrite != pRead))
      *pWrite = '\0';

   return buf;
}

#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
