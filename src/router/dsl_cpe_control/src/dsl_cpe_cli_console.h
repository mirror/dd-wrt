/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef DSL_CPE_CLI_CONSOLE_H
#define DSL_CPE_CLI_CONSOLE_H

/** \file
   DSL daemon command line interface for console (single key input)
*/

/** \defgroup LIB_DSL_CLI_CON_CONF Configuration of DSL CPE API console
  @{ */

/** maximum length of command line */
#define DSL_MAX_COMMAND_LINE_LENGTH    256
/** maximum length of command names */
#define DSL_MAX_FUNCTION_NAME_LENGTH   64
/** maximum of history elements */
#define MAX_OLD_COMMAND                16

/** @} */

struct DSL_CPE_Console_Context
{
   DSL_char_t old_command[MAX_OLD_COMMAND][DSL_MAX_COMMAND_LINE_LENGTH];
   DSL_char_t arg[DSL_MAX_COMMAND_LINE_LENGTH];
   DSL_char_t cmd[DSL_MAX_FUNCTION_NAME_LENGTH];
   DSL_CPE_Control_Context_t *pContext;
   DSL_int_t idx;
   DSL_int_t old_idx;
   DSL_int_t prev_idx;
   DSL_CPE_File_t *file_in;
   DSL_CPE_File_t *file_out;
   volatile DSL_boolean_t bRun;
   DSL_CLI_Context_t *pCLIContext;
};

/** Context for console (including simple "editor") */
typedef struct DSL_CPE_Console_Context DSL_CPE_Console_Context_t;

DSL_Error_t DSL_CPE_Console_Init (DSL_CPE_Console_Context_t ** pConsoleRefContext,
                                  DSL_CPE_Control_Context_t *pContext, DSL_CPE_File_t * in,
                                  DSL_CPE_File_t * out);
DSL_Error_t DSL_CPE_Console_Shutdown(DSL_CPE_Console_Context_t *pConsoleContext);
DSL_Error_t DSL_CPE_Handle_Console(DSL_CPE_Console_Context_t *pConsoleContext);

#endif /* DSL_CPE_CLI_CONSOLE_H */

