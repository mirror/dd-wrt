/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _CLI_H
#define _CLI_H

#include <sys_base.h>
#include "vty.h"
#include "array.h"

#define __CLI_DEBUG
#ifdef __CLI_DEBUG
#define CLI_DEBUG(fmt, ...)                                                 \
    do                                                                      \
    {                                                                       \
        osal_print("%s[%d]:"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);    \
    } while (0)
#else
#define CLI_DEBUG(fmt, arg...) do{}while(0)
#endif

#define PROMPT_BASE         "macsec"

#define osal_malloc  malloc
#define osal_free    free
#define osal_print   printf

typedef enum {
	CLI_MODE_ENABLE,	/* first and default mode */
} CLI_MODE_ID_T;

typedef int (*CLI_MODE_FUN_T) (VTY_T *);

typedef struct {
	int id;
	CLI_MODE_FUN_T exitFunc;
	ARRAY_T *pCmdArray;
} CLI_MODE_T;

typedef int (*CLI_CMD_FUN_T) (VTY_T *, int, const char *[]);

typedef struct cli_cmd {
	const char *cmdStr;	/* command specification string */
	const char *descStr;	/* desctription string for every sub command, seperate by '\n' */
	CLI_CMD_FUN_T func;	/* executive function */
	ARRAY_T *pSubCmdArray;	/* pointer to sub command array */
	int cmdSize;		/* sub command count */
	sa_bool_t invisible;	/* True: invisible, False: normal and visible command */
	sa_bool_t isAlloc;	/* True: should free command when done, False: static nodes */
} CLI_CMD_T;

typedef enum {
	CLI_OK = 0,		/* Okay, or
				   find only matched command, and excute success */
	CLI_FAIL,		/* fail, or
				   fail when find command, or
				   find matched command but excute fail */
	CLI_NO_MATCH,		/* cannot find matched command */
	CLI_UNRECOGNIZED,	/* unrecognized command */
	CLI_AMBIGUOUS,		/* find multiple matched commands, don't know which one to execute */
	CLI_WARNING,		/*  */
	CLI_INCOMPLETE,		/* incomplete matched command, no execute */
	CLI_EXEED_ARGC_MAX,	/* too many command words */
	CLI_EMPTY_COMMAND,	/* empty command */
	CLI_COMPLETE_FULL_MATCH,	/* only one full match command */
	CLI_COMPLETE_PARTLY_MATCH,	/* multiple match command, can complete more characters */
	CLI_COMPLETE_MULTI_MATCH,	/* multiple match command */
	CLI_EXEC_FAIL,		/* sdk api execute fail */
	CLI_INVALID_PARAMETER,	/* arguments invalid */
	CLI_NO_MEMORY,		/* not enough memory */
} CLI_RET_T;

#define DEFCMD_FUNC_DECL(funcName) \
  static int funcName(VTY_T *, int, const char *[]);

/* helper defines for end-user DEFUN* macros */
#define DEFCMD_ELEMENT(funcName, cmdName, cmdStr, descStr)  \
  CLI_CMD_T cmdName =                                       \
  {                                                         \
    cmdStr,                                                 \
    descStr,                                                \
    funcName                                                \
  };

#define DEFCMD_FUNC(funcName)                                           \
  static int funcName(struct vty *pVty, const int argc, const char *argv[])

/* define a command */
#define DEFCMD(funcName, cmdName, cmdStr, descStr)      \
  DEFCMD_FUNC_DECL(funcName)                            \
  DEFCMD_ELEMENT(funcName, cmdName, cmdStr, descStr)    \
  DEFCMD_FUNC(funcName)

#define DEFALIAS(funcName, cmdName, cmdStr, descStr)    \
  DEFCMD_ELEMENT(funcName, cmdName, cmdStr, descStr)

#define DESC_NO     "Negate a command or set its defaults\n"
#define DESC_SHOW   "Show running system information\n"
#define DESC_IP     "IP information\n"
#define DESC_IPV6   "IPv6 information\n"

/******************************************************************************
 *
 * description:
 *   install mode to root
 *
 * input:
 *   pMode - pointer to mode struct
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_install_mode(CLI_MODE_T *pMode);

/******************************************************************************
 *
 * description:
 *   init cli module
 *
 * input:
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_init(void);

/******************************************************************************
 *
 * description:
 *   exit cli module
 *
 * input:
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_exit(void);

/******************************************************************************
 *
 * description:
 *   install a command to target mode, this command is supported by engine.
 *   Do not use this directly, use cli_install_cmd instead.
 *
 * input:
 *   modeId - mode ID
 *   pCmd   - pointer to command
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_install_simple_cmd(int modeId, CLI_CMD_T *pCmd);

/******************************************************************************
 *
 * description:
 *   install a command to target mode.
 *   CLI engine support command format:
 *   item 1. show aa
 *   item 2. show aa <1-1024>
 *   item 3. show aa { op1 | op2 }   // option only allow one word
 *
 *   Using this function preprocess for command format:
 *   item 4. show [aa] [bb]
 *   item 5. show {aa a1 | aa a2}   // option allow multiple words
 *
 *   - Mix use of Item 4 and item 5 is not support.
 *     For example, show {aa a1 | aa a2} [cc]  is considered to be forbidden,
 *     While show aa <1-1024> [cc]  and show aa <1-1024> {cc c1 | dd d1} is allowed.
 *
 *   - Item 4 support very complex command.
 *     For example,  [no] workaround [key1 value1] key2 [{key3 | key4}] [key5] is supported.
 *
 * input:
 *   modeId - mode ID
 *   pCmd   - pointer to command
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_install_cmd(int modeId, CLI_CMD_T *pCmd);

/******************************************************************************
 *
 * description:
 *   install a invisible command to target mode.
 *
 * input:
 *   modeId - mode ID
 *   pCmd   - pointer to command
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_install_invisible_cmd(int modeId, CLI_CMD_T *pCmd);

CLI_MODE_FUN_T cli_mode_exit_func(int modeId);

/******************************************************************************
 *
 * description:
 *   execute command according to line buffer
 *
 * input:
 *   pVty - pointer to VTY
 *
 * output:
 *
 * return:
 *   one of CMD_EXECUTE_T
 *
 ******************************************************************************/
int cli_execute_cmd(VTY_T *pVty);

/******************************************************************************
 *
 * description:
 *   find matched command according to line buffer, and display description of
 *   sub commands.
 *
 * input:
 *   pVty - pointer to VTY
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_describe_cmd(VTY_T *pVty);

/******************************************************************************
 *
 * description:
 *   find matched command according to line buffer, if:
 *    * there is no matched command, then do nothing
 *    * there is only one matched command, complete the sub command
 *    * there are some matched command, display there as reference
 *
 * input:
 *   pVty - pointer to VTY
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_complete_cmd(VTY_T *pVty);

/******************************************************************************
 *
 * description:
 *   display all arguments, it is a debug function used in command function
 *
 * input:
 *   pVty - pointer to VTY
 *   argc - count of arguments
 *   argv - value of arguments
 *
 * output:
 *
 * return:
 *   always 0
 *
 ******************************************************************************/
int cli_display_arguments(VTY_T *pVty, const int argc, const char **argv);

#endif /* _CLI_H */
