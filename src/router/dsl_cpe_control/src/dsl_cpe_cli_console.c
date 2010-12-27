/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL daemon command line interface for console (single key input)
*/

/*#define DEBUG_CONSOLE*/

/*#define DSL_INTERN*/

#include "dsl_cpe_os.h"
#include "dsl_cpe_cli.h"
#include "dsl_cpe_control.h"
#include "dsl_cpe_cli_console.h"
#include "drv_dsl_cpe_api_error.h"

#ifdef INCLUDE_DSL_CPE_CLI_SUPPORT

#undef DSL_CCA_DBG_BLOCK
#define DSL_CCA_DBG_BLOCK DSL_CCA_DBG_CONSOLE

#define KEY_INPUT_NONE  0
#define KEY_INPUT_ENTER 1
#define KEY_INPUT_BS    2
#define KEY_INPUT_UP    3
#define KEY_INPUT_DOWN  4
#define KEY_INPUT_CHAR  5

#define DSL_CPE_MAX_CLI_LEN 1024

#ifndef WIN32
   #define BOLD "\033[1;30m"
   #define NORMAL "\033[0m"
   #define CLREOL "\033[K"
#else
   #define BOLD   ""
   #define NORMAL ""
   #define CLREOL ""
#endif

static DSL_Error_t DSL_CPE_Console_Exit
(
   DSL_void_t *pContext
);

static DSL_Error_t DSL_CPE_Console_EventCallback
(
   DSL_void_t *pContext,
   DSL_char_t *pMsg
);

static DSL_int_t DSL_CPE_CommandLineRead
(
   DSL_char_t *str,
   DSL_uint32_t len,
   DSL_CPE_File_t *in,
   DSL_CPE_File_t * out
);

/******************************************************************************/

#ifdef INCLUDE_DSL_API_CONSOLE_EXTRA
DSL_Error_t run_dsl_cpe_console(int fd)
{
   DSL_CPE_Console_Context_t *pConsoleContext = DSL_NULL;
   DSL_Error_t ret = DSL_SUCCESS;
   DSL_CPE_Control_Context_t *pContext = DSL_CPE_GetGlobalContext ( );
#ifdef VXWORKS
   DSL_int_t i;
   DSL_int_t stdfds[3];
#endif

   if (pContext == DSL_NULL)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "'dsl_cpe_control' not running! Exit..." DSL_CPE_CRLF );
      return DSL_ERROR;
   }

#ifdef VXWORKS
   /* FIXME: make sure that this console runs on the serial port (fd 3) */
   for (i = 0; i < 3; ++i)
   {
      stdfds[i] = ioTaskStdGet(0,i);
      if (stdfds[i] != ERROR)
         ioTaskStdSet(0,i,3);
   }
#endif

   if (DSL_CPE_CLI_Init () == DSL_ERROR)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "'DSL_CPE_CLI_Init' failed" DSL_CPE_CRLF));
      goto console_exit;
   }

   if (DSL_CPE_Console_Init (&pConsoleContext, pContext, DSL_CPE_STDIN, DSL_CPE_STDOUT) ==
       DSL_ERROR)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "'DSL_CPE_Console_Init' failed! Exit..." DSL_CPE_CRLF));
      ret = DSL_ERROR;
      goto console_exit;
   }

   do
   {
      ret = DSL_CPE_Handle_Console (pConsoleContext);
   }
   while (ret != DSL_ERROR);

   if (DSL_CPE_Console_Shutdown (pConsoleContext) == DSL_ERROR)
   {
      ret = DSL_ERROR;
      goto console_exit;
   }

console_exit:

#ifdef VXWORKS
   /* restore stdio */
   for (i = 0; i < 3; ++i)
   {
      if (stdfds[i] != ERROR)
         ioTaskStdSet(0,i,stdfds[i]);
   }
#endif

   return ret;
}
#endif

DSL_Error_t DSL_CPE_Console_Init (DSL_CPE_Console_Context_t ** pConsoleRefContext,
                                  DSL_CPE_Control_Context_t *pContext, DSL_CPE_File_t * in,
                                  DSL_CPE_File_t * out)
{
   DSL_CPE_Console_Context_t *pConsoleContext;

   if (*pConsoleRefContext != DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "expecting zero context pointer '*pConsoleRefContext'" DSL_CPE_CRLF));
      return DSL_ERROR;
   }

   *pConsoleRefContext = malloc (sizeof (DSL_CPE_Console_Context_t));
   if (*pConsoleRefContext == DSL_NULL)
   {
      DSL_CCA_DEBUG(DSL_CCA_DBG_WRN, (DSL_CPE_PREFIX
         "no memory for '*pConsoleRefContext'" DSL_CPE_CRLF));
      return DSL_ERROR;
   }
   pConsoleContext = *pConsoleRefContext;

   memset (pConsoleContext, 0x00, sizeof (*pConsoleContext));
   pConsoleContext->pContext  = pContext;
   pConsoleContext->file_in  = in;
   pConsoleContext->file_out = out;
   pConsoleContext->bRun     = DSL_TRUE;

   if (out == DSL_CPE_STDOUT)
      DSL_CPE_EchoOff ();
   if (in == DSL_CPE_STDIN)
      DSL_CPE_KeypressSet ();

   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "Welcome to the CLI interface of DSL CPE API" DSL_CPE_CRLF));
#ifdef INCLUDE_DSL_API_CONSOLE_EXTRA
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "Enter 'bye' to close console only." DSL_CPE_CRLF));
#endif
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "Enter 'help all' for a list of built-in commands." DSL_CPE_CRLF));

   /*DSL_CPE_CLI_HelpPrint (fd, "-h", out);*/

   DSL_CPE_CLI_Register(&pConsoleContext->pCLIContext, pConsoleContext,
      DSL_CPE_Console_Exit, DSL_CPE_Console_EventCallback);

#ifdef RTEMS
   gv_pConsoleContext = pConsoleContext;
#endif /* RTEMS*/

   return DSL_SUCCESS;
}

DSL_Error_t DSL_CPE_Console_Shutdown (DSL_CPE_Console_Context_t * pConsoleContext)
{
#ifdef INCLUDE_DSL_API_CONSOLE_EXTRA
   if (DSL_CPE_CLI_Unregister(pConsoleContext->pCLIContext) == DSL_SUCCESS)
   {
      free(pConsoleContext->pCLIContext);
      pConsoleContext->pCLIContext = DSL_NULL;
   }
#endif
   if (pConsoleContext->file_in == DSL_CPE_STDIN)
      DSL_CPE_KeypressReset ();
   if (pConsoleContext->file_out == DSL_CPE_STDOUT)
      DSL_CPE_EchoOn ();

   free(pConsoleContext);
   pConsoleContext = DSL_NULL;

   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_Console_Exit (DSL_void_t * pContext)
{
   DSL_CPE_Console_Context_t *pConsoleContext = pContext;
#ifdef DEBUG_CONSOLE
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
      "DSL_CPE_Console_Exit() called" DSL_CPE_CRLF));
#endif
   pConsoleContext->pCLIContext = DSL_NULL;
   pConsoleContext->bRun = DSL_FALSE;
   return DSL_SUCCESS;
}

static DSL_Error_t DSL_CPE_Console_EventCallback(
   DSL_void_t *pContext,
   DSL_char_t *pMsg)
{
   DSL_CCA_DEBUG(DSL_CCA_DBG_ERR, ("%s" DSL_CPE_CRLF, pMsg));

   return DSL_SUCCESS;
}

/**
   Read command line.

   \param str Destination pointer for command line
   \param len Maximum length of command line
   \param in File for key input (e.g. DSL_CPE_STDIN)
   \param out File for output (e.g. DSL_CPE_STDOUT)

   \return
   Length of command string
*/
static DSL_int_t DSL_CPE_CommandLineRead(
   DSL_char_t *str,
   DSL_uint32_t len,
   DSL_CPE_File_t *in,
   DSL_CPE_File_t *out)
{
   char c, echo;
   int read_len;
   int blank_cnt = 0;
   int first = 1;
   int ret = KEY_INPUT_CHAR;

   if (strlen (str))
      first = 0;

   len -= strlen (str);
   str += strlen (str);

   while (len--)
   {
      echo = 0;
      read_len = DSL_CPE_FRead (&c, 1, 1, in);

      if (read_len == 0)
      {
         if (DSL_CPE_Feof (in))
            ret = KEY_INPUT_NONE;
         goto KEY_FINISH;
      }
#ifdef DEBUG_CONSOLE
      else
         DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_CRLF " no EOF from getc()" DSL_CPE_CRLF );
#endif

      if (first && (c == ' '))
      {
         len++;
         continue;
      }

      first = 0;

      if (c != ' ')
      {
         blank_cnt = 0;
      }

      switch (c)
      {
         default:
            echo = c;
            break;

         case ' ':
            if (blank_cnt)
            {
               len++;
               str--;
            }
            else
            {
               echo = ' ';
            }
            blank_cnt++;
            break;

            /* backspace */
         case 127:             /* BS in Linux Terminal */
         case 8:               /* BS in TTY */
#ifdef DEBUG_CONSOLE
            DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_CRLF" DEL" DSL_CPE_CRLF );
#endif
            len++;
            str--;
            ret = KEY_INPUT_BS;
            goto KEY_FINISH;

            /* tab */
         case 9:
#ifdef DEBUG_CONSOLE
            DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_CRLF " TAB" DSL_CPE_CRLF );
#endif
            break;

         case '\n':
            DSL_CPE_PutChar ('\n', out);
            DSL_CPE_FFlush (out);
            ret = KEY_INPUT_ENTER;
            goto KEY_FINISH;

         case 033:
            read_len = DSL_CPE_FRead (&c, 1, 1, in);
            if (read_len == 0)
               break;
            switch (c)
            {
               case 27:
#ifdef DEBUG_CONSOLE
                  DSL_CPE_FPrintf(DSL_CPE_STDOUT, DSL_CPE_CRLF " ESC" DSL_CPE_CRLF );
#endif
                  /* escape */
                  break;

               case '[':
                  read_len = DSL_CPE_FRead (&c, 1, 1, in);
                  if (read_len == 0)
                     break;
                  switch (c)
                  {
                     case 'D':
                        /* left */
                        break;
                     case 'C':
                        /* right */
                        break;
                     case 'A':
                        /* up */
                        ret = KEY_INPUT_UP;
                        break;
                     case 'B':
                        /* down */
                        ret = KEY_INPUT_DOWN;
                        break;
                     default:
                        break;
                  }
                  break;

               default:
                  break;
            }
            goto KEY_FINISH;
      }

      *str++ = c;

      if (echo)
         DSL_CPE_PutChar (echo, out);

      DSL_CPE_FFlush (out);
   }

 KEY_FINISH:

   *str++ = 0;

   return ret;
}


/**
   This function collects single key input and handles execution of commands.

   \param pConsoleContext Pointer to console context structure, [I]

   \return
   Return values are defined within the DSL_Error_t definition
   - DSL_SUCCESS if a command was completely entered and sucessful executed
   - DSL_ERROR if an error occured or the command "quit" was entered
*/
DSL_Error_t DSL_CPE_Handle_Console (DSL_CPE_Console_Context_t * pConsoleContext)
{
   DSL_int_t result = 0, len = 0, i = 0;
   DSL_CPE_File_t *in = pConsoleContext->file_in, *out = pConsoleContext->file_out;

   pConsoleContext->cmd[0] = 0;
   pConsoleContext->arg[0] = 0;
   memset (pConsoleContext->arg, 0x00, sizeof (pConsoleContext->arg));

   DSL_CPE_FPrintf(out, "" DSL_CPE_CRLF );

 ARROW_KEY:

   DSL_CPE_FPrintf(out, "\r" CLREOL BOLD "DSL_CPE#>" NORMAL "%s", pConsoleContext->arg);
   DSL_CPE_FFlush (out);

   i = pConsoleContext->old_idx;

 NEW_LOOP:

#ifndef RTEMS
   /* read command line */
   switch (DSL_CPE_CommandLineRead
           (pConsoleContext->arg, DSL_MAX_COMMAND_LINE_LENGTH, in, out))
   {
      case KEY_INPUT_UP:
         if (pConsoleContext->old_idx)
            pConsoleContext->old_idx--;
         else
            pConsoleContext->old_idx = MAX_OLD_COMMAND - 1;

         if (strlen (pConsoleContext->old_command[pConsoleContext->old_idx]))
         {
            strcpy (pConsoleContext->arg,
                    pConsoleContext->old_command[pConsoleContext->old_idx]);

            goto ARROW_KEY;
         }
         else
         {
            pConsoleContext->old_idx = i;
            goto NEW_LOOP;
         }

      case KEY_INPUT_DOWN:

         if (++pConsoleContext->old_idx >= MAX_OLD_COMMAND)
            pConsoleContext->old_idx = 0;

         if (strlen (pConsoleContext->old_command[pConsoleContext->old_idx]))
         {

            if (pConsoleContext->old_idx >= MAX_OLD_COMMAND)
               pConsoleContext->old_idx = 0;

            strcpy (pConsoleContext->arg,
                    pConsoleContext->old_command[pConsoleContext->old_idx]);


            goto ARROW_KEY;
         }
         else
         {
            pConsoleContext->old_idx = i;
            goto NEW_LOOP;
         }

      case KEY_INPUT_BS:
         /* backspace */
         goto ARROW_KEY;

      case KEY_INPUT_NONE:
         if (pConsoleContext->bRun == DSL_FALSE)
         {
            /* indicate command "quit" */
            return DSL_ERROR;
            /* DSL_CPE_FPrintf(DSL_CPE_STDOUT, "KEY_INPUT_NONE" DSL_CPE_CRLF ); */
            /* DSL_CPE_Sleep(1); */
         }
         goto NEW_LOOP;

      default:
         break;
   }

   if (pConsoleContext->bRun == DSL_FALSE)
      /* indicate command "quit" */
      return DSL_ERROR;
#else
   while(gv_dsl_cli_wait==0)
   {
      DSL_CPE_Sleep(1);
   }
   gv_dsl_cli_wait = 0;
#endif /* RTEMS*/

   /* get command name */
   sscanf (pConsoleContext->arg, "%s", pConsoleContext->cmd);

   len = (DSL_int_t) strlen (pConsoleContext->cmd);

   if (len)
   {
#ifdef INCLUDE_DSL_API_CONSOLE_EXTRA
      if (strcmp(pConsoleContext->cmd, "bye") == 0)
      {
         /* indicate command "bye" */
         return DSL_ERROR;
      }
#endif /* INCLUDE_DSL_API_CONSOLE_EXTRA */

      if (strcmp
          (pConsoleContext->old_command[pConsoleContext->prev_idx],
           pConsoleContext->arg) != 0)
      {
         strcpy (pConsoleContext->old_command[pConsoleContext->idx],
                 pConsoleContext->arg);
         pConsoleContext->prev_idx = pConsoleContext->idx;
         if (++pConsoleContext->idx >= MAX_OLD_COMMAND)
            pConsoleContext->idx = 0;
      }

      pConsoleContext->old_idx = pConsoleContext->idx;

      /* find function by name and execute it */

      result =
         DSL_CPE_CliDeviceCommandExecute (pConsoleContext->pContext, -1,
               pConsoleContext->cmd, pConsoleContext->arg + len + 1, out);
   }

   if (result == 1)
   {
      /* indicate command "quit" */
      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

#if defined(RTEMS) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)
DSL_CPE_Console_Context_t *gv_pConsoleContext = DSL_NULL;
DSL_uint32_t gv_dsl_cli_wait=0;

/******************************************************************************/
// call from cpe_test.c

/*
   KAv: Should be renamed to DSL_CPE_CommandHandle
*/
DSL_Error_t DSL_CPE_Handle_Command (char *cmd)
{
   DSL_CPE_Console_Context_t * pConsoleContext = gv_pConsoleContext;
   DSL_int_t result, len;
   DSL_int32_t fd = DSL_NULL;
   DSL_CPE_File_t *out = pConsoleContext->file_out;

   if (gv_pConsoleContext == DSL_NULL)
   {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "'DSL_CPE_Handle_Command' Console not initilized..." DSL_CPE_CRLF );
      return DSL_ERROR;
   }
   fd = pConsoleContext->fd;

   strcpy(gv_pConsoleContext->arg, cmd);

   /* get command name */
   sscanf (pConsoleContext->arg, "%s", pConsoleContext->cmd);

   len = (DSL_int_t) strlen (pConsoleContext->cmd);

   if (len)
   {
       strcpy(pConsoleContext->arg, cmd);
   }

   /* find function by name and execute it */
   result = DSL_CPE_CliDeviceCommandExecute(
               pConsoleContext->pContext, -1,
               pConsoleContext->cmd,
               pConsoleContext->arg + len + 1, out);

   if (result == 1)
   {
      /* indicate command "quit" */
      return DSL_ERROR;
   }

   return DSL_SUCCESS;
}

// call from adsl_dbg.c: adsl_dbg_cmd()
void rtems_DSL_CPE_Handle_Console(char *cmd)
{

    while(gv_dsl_cli_wait==1)
    {
       DSL_CPE_Sleep (1);
    }

    if (gv_pConsoleContext == DSL_NULL)
    {
      DSL_CPE_FPrintf (DSL_CPE_STDOUT, DSL_CPE_PREFIX "'rtems_DSL_CPE_Handle_Console' Console not initilized..." DSL_CPE_CRLF );
      return DSL_ERROR;
    }

    strcpy(gv_pConsoleContext->arg, cmd);
    // DSL_CPE_Handle_Console (gv_pConsoleContext);
    gv_dsl_cli_wait=1;
}

/*
   Implementation below moved from the adsl_dbg.c module
*/

#undef  DBG_TARGET
#define DBG_TARGET   DIO_NONE

/* adsl_dbg_help -- ATM debug help module callback. */
static void adsl_dbg_help (void)
{
   tty_print ("Following ADSL commands are available:\r\n");
   tty_print ("dpt : Run DSL production test (with 100 Ohm resistor)\r\n");
   tty_print ("other commands : Controlled by DSL CPE Control API:\r\n");

   /* call DSL-API CLI handler */
   rtems_DSL_CPE_Handle_Console("help");
}

/* adsl_dbg_cmd -- ADSL debug command module callback. */
unsigned long adsl_dbg_cmd (char *str, int *idx, char *cmd)
{
#ifdef INCLUDE_DSL_CPE_PRODUCTION_TEST_SUPPORT
   if (!astrncmp (str, "dpt", 3))
   {
      /* DSL Production test */
      dsl_cpe_test();
   }
   else
#endif /* INCLUDE_DSL_CPE_PRODUCTION_TEST_SUPPORT*/
   {
      /* call DSL-API CLI handler */
      rtems_DSL_CPE_Handle_Console(str);
   }
   return (1); // 0= command not defined
}

void adsl_dbg_init (void)
{
   dbg_link ("ADSL", adsl_dbg_help, adsl_dbg_cmd);
}
#endif /* defined(RTEMS) && defined(INCLUDE_DSL_CPE_CLI_SUPPORT)*/

#endif /* INCLUDE_DSL_CPE_CLI_SUPPORT*/
