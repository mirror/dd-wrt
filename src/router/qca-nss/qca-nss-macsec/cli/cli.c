/*
 * Copyright (c) 2014, 2019 The Linux Foundation. All rights reserved.
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

//#include "osal_common.h"
#include "array.h"
#include "cli.h"
#include "cli_cmd.h"
#include "cli_lib.h"

static ARRAY_T *gpModeArray = NULL;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define CLI_COMMAND_WORDS_MAX    32

/* the result of token match subcmd */
typedef enum {
	SUBCMD_MATCH_NO = 0,	/* not match */
	SUBCMD_MATCH_TOKEN_EMPTY,	/* token is empty, can match anything */
	SUBCMD_MATCH_IPV4,	/* match IPv4 address, like 192.168.1.1 */
	SUBCMD_MATCH_IPV4_PREFIX,	/* match IPv4 address and prefix, like 192.168.1.1/24 */
	SUBCMD_MATCH_IPV6,	/* match IPv6 address */
	SUBCMD_MATCH_IPV6_PREFIX,	/* match IPv6 address and prefix */
	SUBCMD_MATCH_MAC,	/* match MAC address */
	SUBCMD_MATCH_LIST,	/* match list */
	SUBCMD_MATCH_HEX,	/* match HEX, like 0x12ab */
	SUBCMD_MATCH_RANGE,	/* match range, like <1-10> */
	SUBCMD_MATCH_WORD,	/* match word */
	SUBCMD_MATCH_PARTLY,	/* partly match key */
	SUBCMD_MATCH_EXACT	/* exact match key */
} SUBCMD_MATCH_T;

/* subcmd type */
typedef enum {
	SUBCMD_TYPE_NONE = 0,
	SUBCMD_TYPE_KEY,	/* fixed key word */
	SUBCMD_TYPE_VAR_IPV4,	/* A.B.C.D */
	SUBCMD_TYPE_VAR_IPV4_PREFIX,	/* A.B.C.D/M */
	SUBCMD_TYPE_VAR_IPV6,	/* X:X::X:X */
	SUBCMD_TYPE_VAR_IPV6_PREFIX,	/* X:X::X:X/M */
	SUBCMD_TYPE_VAR_MAC,	/* H.H.H */
	SUBCMD_TYPE_VAR_LIST,	/* <min_func...max_func> or <1...30>, like "1,3,7-18,22" */
	SUBCMD_TYPE_VAR_HEX,	/* HEX, like "0xab345678" */
	SUBCMD_TYPE_VAR_RANGE,	/* <min_func-max_func> or <1-10>, like "8" */
	SUBCMD_TYPE_VAR_WORD,	/* WORD, any word */
	SUBCMD_TYPE_SELECT,	/* {aa|bb|cc} */
	SUBCMD_TYPE_CR		/* <cr> */
} SUBCMD_TYPE_T;

/* ambiguous type */
typedef enum {
	SUBCMD_AMBIGUOUS_NO = 0,
	SUBCMD_AMBIGUOUS_YES
} SUBCMD_AMBIGUOUS_T;

/*
 * struct for sub command or parameter
 * if the type is SUBCMD_TYPE_SELECT, pSubCmdArray contain select list
 */
typedef struct {
	int type;
	char *key;
	char *desc;
	union {
		struct {
			sa_u32_t min;
			sa_u32_t max;
		};
		ARRAY_T *pSubCmdArray;
	};
} CLI_SUBCMD_T;

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
int cli_install_mode(CLI_MODE_T *pMode)
{
	if (NULL == pMode) {
		return CLI_FAIL;
	}

	/* alloc new command array for the mode */
	pMode->pCmdArray = array_new(ARRAY_SLOT_MIN);
	if (NULL == pMode->pCmdArray) {
		return CLI_FAIL;
	}

	return array_set_slot(gpModeArray, pMode, pMode->id);
}

#if 0
static int _cli_compare_cmd(const void *p1, const void *p2)
{
	const CLI_CMD_T *pCmd1 = *((CLI_CMD_T **) p1);
	const CLI_CMD_T *pCmd2 = *((CLI_CMD_T **) p2);

	if ((NULL == pCmd1)
	    || (NULL == pCmd2)) {
		return 0;
	}

	return strcmp(pCmd1->cmdStr, pCmd2->cmdStr);
}
#endif

#if 0
static int _cli_compare_subcmd(const void *p1, const void *p2)
{
	const CLI_SUBCMD_T *pSubCmd1 = *((CLI_SUBCMD_T **) p1);
	const CLI_SUBCMD_T *pSubCmd2 = *((CLI_SUBCMD_T **) p2);

	if ((NULL == pSubCmd1)
	    || (NULL == pSubCmd1)) {
		return 0;
	}

	return strcmp(pSubCmd1->key, pSubCmd2->key);
}

static int _cli_compare_desc(const void *p1, const void *p2)
{
	const CLI_SUBCMD_T *pSubCmd1 = *((CLI_SUBCMD_T **) p1);
	const CLI_SUBCMD_T *pSubCmd2 = *((CLI_SUBCMD_T **) p2);

	if ((NULL == pSubCmd1)
	    || (NULL == pSubCmd1)) {
		return 0;
	}

	if (strcmp(pSubCmd1->key, "<cr>") == 0) {
		return 1;
	} else if (strcmp(pSubCmd2->key, "<cr>") == 0) {
		return -1;
	} else {
		return strcmp(pSubCmd1->key, pSubCmd2->key);
	}
}
#endif

/******************************************************************************
 *
 * description:
 *   sort commands tree
 *
 * input:
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
static int _cli_sort_cmd(void)
{
	int i, j, k;
	CLI_MODE_T *pMode;
	CLI_CMD_T *pCmd;
	CLI_SUBCMD_T *pSubCmd;
	ARRAY_T *pCmdArray;
	ARRAY_T *pSubCmdArray;
//    ARRAY_T        *pInnerSubCmdArray;

	if (NULL == gpModeArray) {
		return CLI_FAIL;
	}

	/* sort commands in every mode */
	for (i = 0; i < array_index(gpModeArray); i++) {
		/* get mode */
		pMode = array_slot(gpModeArray, i);
		if (NULL == pMode) {
			continue;
		}

		/* get commands array in the mode */
		pCmdArray = pMode->pCmdArray;
		if (NULL == pCmdArray) {
			continue;
		}
#if 0
		/* sort commands in pMode */
		osal_qsort(pCmdArray->slot,
			   array_index(pCmdArray),
			   sizeof(void *), _cli_compare_cmd);
#endif

		for (j = 0; j < array_index(pCmdArray); j++) {
			/* get a command */
			pCmd = array_slot(pCmdArray, j);
			if (NULL == pCmd) {
				continue;
			}

			pSubCmdArray = pCmd->pSubCmdArray;
			if (NULL == pSubCmdArray) {
				continue;
			}

			for (k = 0; k < array_index(pSubCmdArray); k++) {
				pSubCmd = array_slot(pSubCmdArray, k);
				if (NULL == pSubCmd) {
					continue;
				}
#if 0
				/* sort sub commands if it is select type like {aa|cc|bb} */
				if ((SUBCMD_TYPE_SELECT == pSubCmd->type)
				    && (pInnerSubCmdArray =
					pSubCmd->pSubCmdArray)) {
					osal_qsort(pInnerSubCmdArray->slot,
						   array_index
						   (pInnerSubCmdArray),
						   sizeof(void *),
						   _cli_compare_subcmd);

				}
#endif
			}
		}
	}

	return CLI_OK;
}

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
int cli_init(void)
{
	gpModeArray = array_new(ARRAY_SLOT_MIN);

	if (NULL == gpModeArray) {
		CLI_DEBUG("Cannot alloc command tree\r\n");
		return CLI_FAIL;
	}

	cli_cmd_init();
	_cli_sort_cmd();

	return CLI_OK;
}

int _cli_free_subcmd(CLI_SUBCMD_T *pSubCmd)
{
	int i;
	CLI_SUBCMD_T *pInnerSubcmd;

	if (SUBCMD_TYPE_SELECT == pSubCmd->type) {
		for (i = 0; i < array_index(pSubCmd->pSubCmdArray); i++) {
			pInnerSubcmd =
			    (CLI_SUBCMD_T *) array_slot(pSubCmd->pSubCmdArray,
							i);
			if (pInnerSubcmd) {
				if (pInnerSubcmd->key) {
					osal_free(pInnerSubcmd->key);
					pInnerSubcmd->key = NULL;
				}

				if (pInnerSubcmd->desc) {
					osal_free(pInnerSubcmd->desc);
					pInnerSubcmd->desc = NULL;
				}

				osal_free(pInnerSubcmd);
			}

		}

		array_free(pSubCmd->pSubCmdArray);
	} else {
		if (pSubCmd->key) {
			osal_free(pSubCmd->key);
			pSubCmd->key = NULL;
		}

		if (pSubCmd->desc) {
			osal_free(pSubCmd->desc);
			pSubCmd->desc = NULL;
		}
	}

	osal_free(pSubCmd);

	return 0;
}

int _cli_free_cmd(CLI_CMD_T *pCmd)
{
	int i;
	CLI_SUBCMD_T *pSubCmd;

	if (pCmd->pSubCmdArray != NULL) {
		for (i = 0; i < array_index(pCmd->pSubCmdArray); i++) {
			pSubCmd =
			    (CLI_SUBCMD_T *) array_slot(pCmd->pSubCmdArray, i);
			if (pSubCmd) {
				_cli_free_subcmd(pSubCmd);
			}
		}

		array_free(pCmd->pSubCmdArray);
		pCmd->pSubCmdArray = NULL;
	}

	/* dynamic cmd, igonre un-initialization problem */
	if (pCmd->isAlloc == SA_TRUE) {
		if (pCmd->cmdStr) {
			osal_free((char *)pCmd->cmdStr);
		}

		if (pCmd->descStr) {
			osal_free((char *)pCmd->descStr);
		}

		osal_free(pCmd);
	}

	return 0;
}

int _cli_free_mode(CLI_MODE_T *pMode)
{
	int i;
	CLI_CMD_T *pCmd;

	for (i = 0; i < array_index(pMode->pCmdArray); i++) {
		pCmd = (CLI_CMD_T *) array_slot(pMode->pCmdArray, i);
		if (pCmd) {
			_cli_free_cmd(pCmd);
		}
	}

	array_free(pMode->pCmdArray);

	return 0;
}

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
int cli_exit(void)
{
	int i;
	CLI_MODE_T *pMode;

	/* free all commands and modes in gModeArray */
	for (i = 0; i < array_index(gpModeArray); i++) {
		pMode = (CLI_MODE_T *) array_slot(gpModeArray, i);
		if (pMode) {
			_cli_free_mode(pMode);
		}
	}

	array_free(gpModeArray);

	return 0;
}

/******************************************************************************
 *
 * description:
 *   trim unused character in the left of string
 *
 * input:
 *   str - source string
 *
 * output:
 *
 * return:
 *   NULL     - str is NULL
 *   not NULL - pointer to first valid character
 *
 ******************************************************************************/
static const char *_cli_trim_left(const char *str)
{
	const char *cp;

	if (NULL == str) {
		return NULL;
	}

	cp = str;

	while ((*cp != '\0')
	       && (isspace((int)(*cp)) || ('|' == *cp))) {
		cp++;
	}

	return cp;
}

/******************************************************************************
 *
 * description:
 *   get a token from string, alloc memory for new token and return
 *
 * input:
 *   pStr - pointer to source string
 *
 * output:
 *   pStr - pointer to dst string which strip the new token
 *
 * return:
 *   NULL     - get token error
 *   not NULL - pointer to token
 *
 ******************************************************************************/
static char *_cli_token(const char **pStr)
{
	const char *tokenBegin = NULL;
	const char *tokenEnd = NULL;
	char *token = NULL;
	int tokenLen = 0;

	/* parameter check */
	if ((NULL == pStr) || (NULL == *pStr)) {
		return NULL;
	}

	/* strip unused characters and find begin */
	tokenBegin = _cli_trim_left(*pStr);

	/* find end */
	tokenEnd = tokenBegin;
	while (*tokenEnd != '\0') {
		if (!isspace((int)(*tokenEnd))
		    && (*tokenEnd != '|')
		    && (*tokenEnd != '}')) {
			tokenEnd++;
		} else {
			break;
		}
	}

	*pStr = tokenEnd;
	tokenLen = tokenEnd - tokenBegin;
	if (0 == tokenLen) {
		return NULL;
	}

	/* alloc memory for new token */
	token = osal_malloc(tokenLen + 1);
	if (NULL == token) {
		return NULL;
	}

	osal_memcpy(token, (char *)tokenBegin, tokenLen);
	token[tokenLen] = 0;

	return token;
}

/******************************************************************************
 *
 * description:
 *   get a subcmd description from string, alloc memory for new desc and return
 *
 * input:
 *   pStr - pointer to source string, the description is split by \n
 *
 * output:
 *   pStr - pointer to dst string which strip the new description
 *
 * return:
 *   NULL     - get description error
 *   not NULL - pointer to description
 *
 ******************************************************************************/
static char *_cli_desc(const char **pStr)
{
	const char *descBegin;
	const char *descEnd;
	const char *nextBegin;
	char *desc;
	int descLen;

	/* parameter check */
	if ((NULL == pStr) || (NULL == *pStr)) {
		return NULL;
	}

	/* strip unused characters and find begin */
	descBegin = _cli_trim_left(*pStr);

	/* find end */
	descEnd = descBegin;
	while ((*descEnd != '\0') && (*descEnd != '\r') && (*descEnd != '\n')) {
		descEnd++;
	}

	/* strip unused character after end */
	nextBegin = descEnd;
	while ((*nextBegin != '\0')
	       && !isalpha((int)(*nextBegin))
	       && !isdigit((int)(*nextBegin))) {
		nextBegin++;
	}
	*pStr = nextBegin;

	descLen = descEnd - descBegin;
	if (0 == descLen) {
		return NULL;
	}

	/* alloc memory for new description */
	desc = osal_malloc(descLen + 1);
	if (NULL == desc) {
		return NULL;
	}

	osal_memcpy(desc, (char *)descBegin, descLen);
	desc[descLen] = 0;

	return desc;
}

/******************************************************************************
 *
 * description:
 *   alloc memory for new sub command, and set to 0
 *
 * input:
 *
 * output:
 *
 * return:
 *   NULL     - new subcmd error
 *   not NULL - pointer to new subcmd
 *
 ******************************************************************************/
static CLI_SUBCMD_T *_cli_subcmd_new(void)
{
	CLI_SUBCMD_T *pSubCmd = NULL;

	pSubCmd = osal_malloc(sizeof(CLI_SUBCMD_T));
	if (NULL != pSubCmd) {
		osal_memset(pSubCmd, 0, sizeof(CLI_SUBCMD_T));
	}

	return pSubCmd;
}

/******************************************************************************
 *
 * description:
 *   check command string format
 *
 * input:
 *   cmdStr - command string
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
static int _cli_cmd_str_verify(const char *cmdStr)
{
	return CLI_OK;
}

static sa_bool_t _cli_is_key(char *key)
{
	return SA_TRUE;
}

/******************************************************************************
 *
 * description:
 *   check whether sub command string is a LIST or RANGE
 *   the LIST parameter is like <min_func...max_func> or <1...30>
 *   the RANGE parameter is like <min_func-max_func> or <1-10>
 *   min_func is function name which return min value of list,
 *   max_func is function name which return max value of list
 *
 * input:
 *   str - sub command string
 *
 * output:
 *   pMin - pointer to min value
 *   pMax - pointer to max value
 *
 * return:
 *   SUBCMD_TYPE_NONE
 *   SUBCMD_TYPE_VAR_RANGE
 *   SUBCMD_TYPE_VAR_LIST
 *
 ******************************************************************************/
static int _cli_subcmd_list_range_check(const char *str, sa_u32_t *pMin,
					sa_u32_t *pMax)
{
#define CLI_RANGE_LIST_STR_CHARS "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_<>"
#define CLI_MIN_MAX_FUNCTION_NAME_LEN_MAX   31

	int type;
	int strLen;
	char *pDelimiter;
	int delimiterLen;
	const char *funcNameStart;
	int funcNameLen;
	char funcNameStr[CLI_MIN_MAX_FUNCTION_NAME_LEN_MAX + 1];
	char *endPtr;

	/* parameter check */
	if (NULL == str) {
		type = SUBCMD_TYPE_NONE;
		goto EXIT;
	}

	strLen = strlen(str);
	if ((str[0] != '<') || (str[strLen - 1] != '>')) {
		type = SUBCMD_TYPE_NONE;
		goto EXIT;
	}

	/* character check */
	if (strspn(str, CLI_RANGE_LIST_STR_CHARS) != strLen) {
		type = SUBCMD_TYPE_NONE;
		goto EXIT;
	}

	/* search delimiter, if the delimiter is '-' then it may be range
	 * if the delimiter is "..." then it may be list
	 */
	if ((pDelimiter = strstr(str, "-")) != NULL) {
		type = SUBCMD_TYPE_VAR_RANGE;
		delimiterLen = strlen("-");
	} else if ((pDelimiter = strstr(str, "...")) != NULL) {
		type = SUBCMD_TYPE_VAR_LIST;
		delimiterLen = strlen("...");
	} else {
		type = SUBCMD_TYPE_NONE;
		goto EXIT;
	}

	/* handle min vlaue, it may be a number or a function name
	 */
	funcNameStart = str + 1;
	funcNameLen = pDelimiter - funcNameStart;

	if ((funcNameLen <= 0)
	    || (funcNameLen > CLI_MIN_MAX_FUNCTION_NAME_LEN_MAX)) {
		type = SUBCMD_TYPE_NONE;
		goto EXIT;
	}

	memcpy(funcNameStr, funcNameStart, funcNameLen);
	funcNameStr[funcNameLen] = '\0';

	*pMin = strtoul(funcNameStr, &endPtr, 10);
	if (*endPtr != '\0') {
		/* it is not a number, try treat it as funciton name */
		if (cli_get_value_by_fun_name(funcNameStr, pMin) != CLI_OK) {
			type = SUBCMD_TYPE_NONE;
			goto EXIT;
		}
	}

	/* handle max vlaue, it may be a number or a function name
	 */
	funcNameStart = pDelimiter + delimiterLen;
	funcNameLen = str + strLen - 1 - funcNameStart;

	if ((funcNameLen <= 0)
	    || (funcNameLen > CLI_MIN_MAX_FUNCTION_NAME_LEN_MAX)) {
		type = SUBCMD_TYPE_NONE;
		goto EXIT;
	}

	memcpy(funcNameStr, funcNameStart, funcNameLen);
	funcNameStr[funcNameLen] = '\0';

	*pMax = strtoul(funcNameStr, &endPtr, 10);
	if (*endPtr != '\0') {
		/* it is not a number, try treat it as funciton name */
		if (cli_get_value_by_fun_name(funcNameStr, pMax) != CLI_OK) {
			type = SUBCMD_TYPE_NONE;
			goto EXIT;
		}
	}

      EXIT:
	return type;
}

/******************************************************************************
 *
 * description:
 *   check key string type
 *
 * input:
 *   pKey - pointer to key string
 *
 * output:
 *   pKey - pointer to key string, if the key type is LIST or RANGE, the key
 *          will be re-malloced and modified
 *
 * return:
 *   one of SUBCMD_TYPE_T
 *
 * note:
 *   the caller must make sure pSubCmd != NULL
 *
 ******************************************************************************/
static int _cli_subcmd_type(CLI_SUBCMD_T *pSubCmd)
{
	int type = SUBCMD_TYPE_NONE;
	char *key = pSubCmd->key;
	char newKey[32];	/* max new key is <4294967294...4294967295> */
	int keyLen, newKeyLen;

	if ((NULL == key)
	    || ((keyLen = strlen(key)) == 0)) {
		type = SUBCMD_TYPE_NONE;
	} else if ((key[0] >= 'A') && (key[0] <= 'Z')) {
		if (strcmp(key, "A.B.C.D") == 0) {
			type = SUBCMD_TYPE_VAR_IPV4;
		} else if (strcmp(key, "A.B.C.D/M") == 0) {
			type = SUBCMD_TYPE_VAR_IPV4_PREFIX;
		} else if (strcmp(key, "X:X::X:X") == 0) {
			type = SUBCMD_TYPE_VAR_IPV6;
		} else if (strcmp(key, "X:X::X:X/M") == 0) {
			type = SUBCMD_TYPE_VAR_IPV6_PREFIX;
		} else if (strcmp(key, "H.H.H") == 0) {
			type = SUBCMD_TYPE_VAR_MAC;
		} else if (strcmp(key, "HEX") == 0) {
			type = SUBCMD_TYPE_VAR_HEX;
		} else if (strcmp(key, "WORD") == 0) {
			type = SUBCMD_TYPE_VAR_WORD;
		} else {
			type = SUBCMD_TYPE_NONE;
		}
	} else if (key[0] == '<') {
		type =
		    _cli_subcmd_list_range_check(key, &(pSubCmd->min),
						 &(pSubCmd->max));

		if (SUBCMD_TYPE_NONE == type) {
			goto EXIT;
		} else if (SUBCMD_TYPE_VAR_RANGE == type) {
			osal_free(pSubCmd->key);
			newKeyLen =
			    sprintf(newKey, "<%u-%u>", pSubCmd->min,
				    pSubCmd->max);
			pSubCmd->key = osal_malloc(newKeyLen + 1);
			if (pSubCmd->key != NULL) {
				strcpy(pSubCmd->key, newKey);
			}
		} else if (SUBCMD_TYPE_VAR_LIST == type) {
			osal_free(pSubCmd->key);
			newKeyLen =
			    sprintf(newKey, "<%d...%d>", pSubCmd->min,
				    pSubCmd->max);
			pSubCmd->key = osal_malloc(newKeyLen + 1);
			if (pSubCmd->key != NULL) {
				strcpy(pSubCmd->key, newKey);
			}
		}
	} else if (_cli_is_key(key)) {
		type = SUBCMD_TYPE_KEY;
	} else {
		type = SUBCMD_TYPE_NONE;
	}

      EXIT:
	pSubCmd->type = type;

	if (SUBCMD_TYPE_NONE == type && pSubCmd->key != NULL) {
		CLI_DEBUG("Unknown type of sub command \"%s\"\r\n",
			  pSubCmd->key);
	}

	return CLI_OK;
}

/******************************************************************************
 *
 * description:
 *   analysis command string and description to make sub command arrays
 *
 * input:
 *   pCmd - pointer to command struct
 *
 * output:
 *   pCmd - sub command array may be modified
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
static int _cli_cmd_analysis(CLI_CMD_T *pCmd)
{
	const char *cp;		/* floating pointer in command string */
	const char *dp;		/* floating pointer in description string */
	int inBraces = 0;
	CLI_SUBCMD_T *pSubCmd = NULL;
	ARRAY_T *pInnerSubCmdArray = NULL;

	/* parameter check */
	if ((NULL == pCmd)
	    || ((cp = pCmd->cmdStr) == NULL)
	    || ((dp = pCmd->descStr) == NULL)) {
		CLI_DEBUG("Parameter error.\r\n");
		return CLI_FAIL;
	}

	/* check command string format */
	if (_cli_cmd_str_verify(pCmd->cmdStr) != CLI_OK) {
		CLI_DEBUG("Command string unavailable.\r\n");
		return CLI_FAIL;
	}

	/* create sub command array */
	pCmd->pSubCmdArray = array_new(ARRAY_SLOT_MIN);
	if (NULL == pCmd->pSubCmdArray) {
		CLI_DEBUG("New command array fail.\r\n");
		return CLI_FAIL;
	}

	while (*cp != '\0') {
		cp = _cli_trim_left(cp);

		if ('{' == *cp) {
			/* set in braces flag, and only support 1 set braces */
			if (inBraces) {
				goto fail;
			}
			inBraces = 1;
			cp++;

			/* create sub command structure */
			pSubCmd = _cli_subcmd_new();
			if (NULL == pSubCmd) {
				goto fail;
			}

			/* create inner sub command array */
			pInnerSubCmdArray = array_new(ARRAY_SLOT_MIN);
			if (NULL == pInnerSubCmdArray) {
				goto fail;
			}

			pSubCmd->type = SUBCMD_TYPE_SELECT;
			pSubCmd->pSubCmdArray = pInnerSubCmdArray;
			array_insert_slot(pCmd->pSubCmdArray, pSubCmd);
		} else if ('}' == *cp) {
			/* set in braces flag */
			if (!inBraces) {
				goto fail;
			}
			inBraces = 0;
			cp++;
		} else {
			/* create sub command structure */
			pSubCmd = _cli_subcmd_new();
			if (NULL == pSubCmd) {
				goto fail;
			}

			pSubCmd->key = _cli_token(&cp);
			pSubCmd->desc = _cli_desc(&dp);
			_cli_subcmd_type(pSubCmd);

			if (NULL == pSubCmd->key) {
				CLI_DEBUG("Get key fail.\r\n");
				goto fail;
			} else if (NULL == pSubCmd->desc) {
				CLI_DEBUG("Get description fail.\r\n");
				goto fail;
			} else if (SUBCMD_TYPE_NONE == pSubCmd->type) {
				CLI_DEBUG
				    ("Unknown sub command type for \"%s\".\r\n",
				     pSubCmd->key);
				goto fail;
			}

			if (0 == inBraces) {
				array_insert_slot(pCmd->pSubCmdArray, pSubCmd);
			} else {
				array_insert_slot(pInnerSubCmdArray, pSubCmd);
			}
		}
	}

	/* the in braces flag must be empty after analysis total command string */
	if (inBraces) {
		goto fail;
	}

	return CLI_OK;

      fail:
	CLI_DEBUG("Command or description string analysis fail \"%s\"\r\n",
		  pCmd->cmdStr);

	/* The fail handle may leak some memory for the sub command
	 * key and description. But it does not matter, because
	 * it only affect this command itself and will not leak
	 * memory any more for other commands.
	 */
	array_free(pCmd->pSubCmdArray);
	pCmd->pSubCmdArray = NULL;

	return CLI_FAIL;
}

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
int cli_install_simple_cmd(int modeId, CLI_CMD_T *pCmd)
{
	CLI_MODE_T *pMode;

	/* get pointer of mode */
	pMode = array_slot(gpModeArray, modeId);
	if (NULL == pMode) {
		CLI_DEBUG
		    ("Mode %d does not exist when install command \"%s\".\r\n",
		     modeId, pCmd->cmdStr);
		return CLI_FAIL;
	}

	/* analysis command string and description string */
	if (NULL == pCmd->pSubCmdArray) {
		if (_cli_cmd_analysis(pCmd) != CLI_OK) {
			return CLI_FAIL;
		}
	}

	pCmd->cmdSize = array_index(pCmd->pSubCmdArray);

	/* insert command to mode */
	return array_insert_slot(pMode->pCmdArray, pCmd);
}

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
int cli_install_cmd(int modeId, CLI_CMD_T *pCmd)
{
	/* the default command is normal and visible */
	pCmd->invisible = SA_FALSE;
	pCmd->isAlloc = SA_FALSE;

	return cli_install_simple_cmd(modeId, pCmd);
}

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
int cli_install_invisible_cmd(int modeId, CLI_CMD_T *pCmd)
{
	/* set command as invisible */
	pCmd->invisible = SA_TRUE;
	pCmd->isAlloc = SA_FALSE;

	return cli_install_simple_cmd(modeId, pCmd);
}

/******************************************************************************
 *
 * description:
 *   free token array
 *
 * input:
 *   pLine - pointer to line token array
 *
 * output:
 *
 * return:
 *   always CLI_OK
 *
 ******************************************************************************/
static int _cli_free_line_array(ARRAY_T *pLine)
{
	int i;
	char *token;

	for (i = 0; i < array_index(pLine); i++) {
		token = array_slot(pLine, i);
		if (NULL != token) {
			osal_free(token);
		}
	}

	array_free(pLine);

	return CLI_OK;
}

/******************************************************************************
 *
 * description:
 *   split line string to token array
 *
 * input:
 *   lineStr - line string
 *
 * output:
 *   ppArray - pointer to pointer token array
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
static int _cli_split_line(const char *lineStr, ARRAY_T **ppArray)
{
	const char *cp;
	char *token;
	ARRAY_T *pTokenArray;

	/* parameter check */
	if (NULL == lineStr) {
		return CLI_FAIL;
	}

	cp = lineStr;
	cp = _cli_trim_left(cp);

	/* check special line */
	if (('\0' == *cp)	/* empty line */
	    ||('#' == *cp)	/* comments line */
	    ||('!' == *cp)) {	/* comments line */
		*ppArray = NULL;
		return CLI_OK;
	}

	/* alloc token array */
	pTokenArray = array_new(ARRAY_SLOT_MIN);
	if (NULL == pTokenArray) {
		return CLI_FAIL;
	}

	while (*cp != '\0') {
		/* get token from rest string */
		token = _cli_token(&cp);
		if (NULL == token) {
			break;
		}

		/* insert token to array */
		if (array_insert_slot(pTokenArray, token) != CLI_OK) {
			_cli_free_line_array(pTokenArray);
			*ppArray = NULL;
			return CLI_FAIL;
		}
	}

	*ppArray = pTokenArray;
	return CLI_OK;
}

/******************************************************************************
 *
 * description:
 *   get command array of mode
 *
 * input:
 *   modeId - mode ID
 *
 * output:
 *
 * return:
 *   NULL     - get command array fail
 *   non NULL - pointer to command array
 *
 ******************************************************************************/
static ARRAY_T *_cli_mode_cmd_array(int modeId)
{
	CLI_MODE_T *pMode;

	pMode = array_slot(gpModeArray, modeId);
	if (NULL == pMode) {
		return NULL;
	}

	return pMode->pCmdArray;
}

CLI_MODE_FUN_T cli_mode_exit_func(int modeId)
{
	CLI_MODE_T *pMode;

	pMode = array_slot(gpModeArray, modeId);
	if (NULL == pMode) {
		return NULL;
	}

	return pMode->exitFunc;
}

/******************************************************************************
 *
 * description:
 *   compare token and key
 *
 * input:
 *   token - token string
 *   key   - key string
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_key(const char *token, const char *key)
{
	int tokenLen;
	int keyLen;

	if (NULL == key) {
		return SUBCMD_MATCH_NO;
	}

	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	tokenLen = strlen(token);
	keyLen = strlen(key);

	if ((tokenLen == keyLen)
	    && (strcmp(token, key) == 0)) {
		return SUBCMD_MATCH_EXACT;
	} else if ((tokenLen < keyLen)
		   && (strncmp(token, key, tokenLen) == 0)) {
		return SUBCMD_MATCH_PARTLY;
	} else {
		return SUBCMD_MATCH_NO;
	}
}

/******************************************************************************
 *
 * description:
 *   compare token and range
 *
 * input:
 *   token - token string, decimal
 *   min   - list value min
 *   max   - list value max
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_range(const char *token, const sa_u32_t min,
				  const sa_u32_t max)
{
	sa_u32_t val;
	char *endPtr;

	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	/* convert token to value */
	val = strtoul(token, &endPtr, 10);
	if (*endPtr != '\0') {
		return SUBCMD_MATCH_NO;
	}

	/* compare value with min and max */
	if ((val >= min) && (val <= max)) {
		return SUBCMD_MATCH_RANGE;
	} else {
		return SUBCMD_MATCH_NO;
	}
}

/******************************************************************************
 *
 * description:
 *   check token whether is IPv4 address
 *
 * input:
 *   token - token string
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_ipv4(const char *token)
{
	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	if (cli_str_2_ipv4(token, NULL) != 0) {
		return SUBCMD_MATCH_NO;
	}

	return SUBCMD_MATCH_IPV4;
}

/******************************************************************************
 *
 * description:
 *   check token whether is IPv6 address
 *
 * input:
 *   token - token string
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_ipv6(const char *token)
{
	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	if (cli_str_2_ipv6(token, NULL) != 0) {
		return SUBCMD_MATCH_NO;
	}

	return SUBCMD_MATCH_IPV6;
}

/******************************************************************************
 *
 * description:
 *   check token whether is MAC address
 *
 * input:
 *   token - token string
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_mac(const char *token)
{
	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	if (cli_str_2_mac(token, NULL) != 0) {
		return SUBCMD_MATCH_NO;
	} else {
		return SUBCMD_MATCH_MAC;
	}
}

/******************************************************************************
 *
 * description:
 *   check token whether is valid list
 *
 * input:
 *   token - token string
 *   min   - list value min
 *   max   - list value max
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_list(const char *token, const sa_u32_t min,
				 const sa_u32_t max)
{
	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	if (cli_str_2_list(token, min, max, NULL) != 0) {
		return SUBCMD_MATCH_NO;
	} else {
		return SUBCMD_MATCH_LIST;
	}
}

/******************************************************************************
 *
 * description:
 *   check token whether is hex
 *
 * input:
 *   token - token string
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_hex(const char *token)
{
	sa_u32_t hex;

	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	if (cli_str_2_hex(token, &hex) != 0) {
		return SUBCMD_MATCH_NO;
	} else {
		return SUBCMD_MATCH_HEX;
	}
}

/******************************************************************************
 *
 * description:
 *   check token whether is word
 *
 * input:
 *   token - token string
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_word(const char *token)
{
	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	return SUBCMD_MATCH_WORD;
}

/******************************************************************************
 *
 * description:
 *   compare token with sub commands in array
 *
 * input:
 *   token        - token string
 *   pSubCmdArray - pointer to sub commands array
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 * note:
 *   If the token match some sub commands in different match type, return the
 *   biggest match type.
 *
 ******************************************************************************/
static int _cli_token_match_subcmd_array(const char *token,
					 ARRAY_T *pSubCmdArray)
{
	CLI_SUBCMD_T *pSubCmd;
	int i;
	int match = SUBCMD_MATCH_NO;

	/* parameter check */
	if (NULL == pSubCmdArray) {
		return SUBCMD_MATCH_NO;
	}

	/* if token is empty, it can match anything */
	if ((NULL == token)
	    || ('\0' == token[0])) {
		return SUBCMD_MATCH_TOKEN_EMPTY;
	}

	/* handle each sub command in the array */
	for (i = 0; i < array_index(pSubCmdArray); i++) {
		pSubCmd = array_slot(pSubCmdArray, i);
		if (NULL == pSubCmd) {
			continue;
		}

		switch (pSubCmd->type) {
		case SUBCMD_TYPE_KEY:
			match =
			    MAX(match,
				_cli_token_match_key(token, pSubCmd->key));
			break;
		case SUBCMD_TYPE_VAR_IPV4:
			match = MAX(match, _cli_token_match_ipv4(token));
			break;
		case SUBCMD_TYPE_VAR_IPV4_PREFIX:
			match = SUBCMD_MATCH_NO;
			break;
		case SUBCMD_TYPE_VAR_IPV6:
			match = MAX(match, _cli_token_match_ipv6(token));
			break;
		case SUBCMD_TYPE_VAR_IPV6_PREFIX:
			match = SUBCMD_MATCH_NO;
			break;
		case SUBCMD_TYPE_VAR_MAC:
			match = MAX(match, _cli_token_match_mac(token));
			break;
		case SUBCMD_TYPE_VAR_LIST:
			match =
			    MAX(match,
				_cli_token_match_list(token, pSubCmd->min,
						      pSubCmd->max));
			break;
		case SUBCMD_TYPE_VAR_HEX:
			match = MAX(match, _cli_token_match_hex(token));
			break;
		case SUBCMD_TYPE_VAR_RANGE:
			match =
			    MAX(match,
				_cli_token_match_range(token, pSubCmd->min,
						       pSubCmd->max));
			break;
		case SUBCMD_TYPE_VAR_WORD:
			match = MAX(match, _cli_token_match_word(token));
			break;
		case SUBCMD_TYPE_SELECT:
		case SUBCMD_TYPE_NONE:
		default:
			match = SUBCMD_MATCH_NO;
			break;
		}
	}

	return match;
}

/******************************************************************************
 *
 * description:
 *   compare token with sub command, return match type
 *
 * input:
 *   token   - token string
 *   pSubCmd - pointer to sub command
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_match_subcmd(const char *token, CLI_SUBCMD_T *pSubCmd)
{
	int match = SUBCMD_MATCH_NO;

	switch (pSubCmd->type) {
	case SUBCMD_TYPE_KEY:
		match = _cli_token_match_key(token, pSubCmd->key);
		break;
	case SUBCMD_TYPE_VAR_IPV4:
		match = _cli_token_match_ipv4(token);
		break;
	case SUBCMD_TYPE_VAR_IPV4_PREFIX:
		match = SUBCMD_MATCH_NO;
		break;
	case SUBCMD_TYPE_VAR_IPV6:
		match = _cli_token_match_ipv6(token);
		break;
	case SUBCMD_TYPE_VAR_IPV6_PREFIX:
		match = SUBCMD_MATCH_NO;
		break;
	case SUBCMD_TYPE_VAR_MAC:
		match = _cli_token_match_mac(token);
		break;
	case SUBCMD_TYPE_VAR_LIST:
		match =
		    _cli_token_match_list(token, pSubCmd->min, pSubCmd->max);
		break;
	case SUBCMD_TYPE_VAR_HEX:
		match = _cli_token_match_hex(token);
		break;
	case SUBCMD_TYPE_VAR_RANGE:
		match =
		    _cli_token_match_range(token, pSubCmd->min, pSubCmd->max);
		break;
	case SUBCMD_TYPE_VAR_WORD:
		match = _cli_token_match_word(token);
		break;
	case SUBCMD_TYPE_SELECT:
		match =
		    _cli_token_match_subcmd_array(token, pSubCmd->pSubCmdArray);
		break;
	case SUBCMD_TYPE_NONE:
	default:
		match = SUBCMD_MATCH_NO;
	}

	return match;
}

/******************************************************************************
 *
 * description:
 *   Compare token with every sub command, which in fixed position of every
 *   command of array. If the sub command is not matched, unset the command.
 *
 * input:
 *   token     - token string
 *   pCmdArray - pointer to command array
 *   index     - the token position in a command
 *
 * output:
 *   pCmdArray - pointer to command array, commands may be unset
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 * note:
 *   If the token match some sub commands in different match type, return the
 *   biggest match type.
 *
 ******************************************************************************/
static int _cli_token_match_cmd_array(const char *token, ARRAY_T *pCmdArray,
				      int index)
{
	CLI_CMD_T *pCmd;
	CLI_SUBCMD_T *pSubCmd;
	int i;
	int totalMatch = SUBCMD_MATCH_NO;
	int match;

	/* compare token with every command in the pCmdArray */
	for (i = 0; i < array_index(pCmdArray); i++) {
		/* get a command from array */
		pCmd = array_slot(pCmdArray, i);
		if (NULL == pCmd) {
			continue;
		}

		/* unset the command if the token position extend command length
		 * the command will not be handled in next steps
		 */
		if (index >= pCmd->cmdSize) {
			array_unset_slot(pCmdArray, i);
			continue;
		}

		/* get sub command in fixed postion */
		pSubCmd = array_slot(pCmd->pSubCmdArray, index);
		if (NULL == pSubCmd) {
			array_unset_slot(pCmdArray, i);
			continue;
		}
//        match = SUBCMD_MATCH_NO;

		/* get compare result according to the sub command type */
		match = _cli_token_match_subcmd(token, pSubCmd);

		/* unset the command in array if the sub command is not matched */
		if (SUBCMD_MATCH_NO == match) {
			array_unset_slot(pCmdArray, i);
		} else {
			totalMatch = MAX(totalMatch, match);
		}
	}

	return totalMatch;
}

/******************************************************************************
 *
 * description:
 *   check whether token is ambiguous and unset not matched command
 *
 * input:
 *   token     - token string
 *   pCmdArray - pointer to command array
 *   index     - the token position in a command
 *   match     - match type of last step
 *
 * output:
 *   pCmdArray - pointer to command array, commands may be unset
 *
 * return:
 *   one of SUBCMD_AMBIGUOUS_T
 *
 ******************************************************************************/
static int _cli_token_is_ambiguous(char *token, ARRAY_T *pCmdArray, int index,
				   int match)
{
	CLI_CMD_T *pCmd;
	CLI_SUBCMD_T *pSubCmd, *pInnerSubCmd;
	int i, j;
	int tokenLen = strlen(token);
	char *matchedKey = NULL;
	ARRAY_T *pInnerSubCmdArray;
	int matched;

	switch (match) {
	case SUBCMD_MATCH_EXACT:
		/* handle every command in the array */
		for (i = 0; i < array_index(pCmdArray); i++) {
			/* get a command */
			pCmd = array_slot(pCmdArray, i);
			if (NULL == pCmd) {
				continue;
			}

			/* unset the command if the token position extend command length */
			if (index > array_index(pCmd->pSubCmdArray)) {
				array_unset_slot(pCmdArray, i);
				continue;
			}

			/* get sub command in fixed position */
			pSubCmd = array_slot(pCmd->pSubCmdArray, index);
			if (NULL == pSubCmd) {
				array_unset_slot(pCmdArray, i);
				continue;
			}

			matched = 0;

			switch (pSubCmd->type) {
			case SUBCMD_TYPE_KEY:
				/* if the token is different with key, unset the command */
				if (strcmp(pSubCmd->key, token) == 0) {
					matched++;
				}
				break;

			case SUBCMD_TYPE_SELECT:
				pInnerSubCmdArray = pSubCmd->pSubCmdArray;

				/* compare with every sub command in the inner sub command array */
				for (j = 0; j < array_index(pInnerSubCmdArray);
				     j++) {
					if (((pInnerSubCmd =
					      array_slot(pInnerSubCmdArray,
							 j)) != NULL)
					    && (pInnerSubCmd->type ==
						SUBCMD_TYPE_KEY)
					    &&
					    (strncmp
					     (token, pInnerSubCmd->key,
					      tokenLen) == 0)) {
						matched++;
					}
				}

				break;

			default:
				break;
			}

			/* if the sub command is not matched, unset the command */
			if (0 == matched) {
				array_unset_slot(pCmdArray, i);
			}
		}

		break;

	case SUBCMD_MATCH_PARTLY:
		/* handle every command in the array */
		for (i = 0; i < array_index(pCmdArray); i++) {
			/* get a command */
			pCmd = array_slot(pCmdArray, i);
			if (NULL == pCmd) {
				continue;
			}

			/* unset the command if the token position extend command length */
			if (index > array_index(pCmd->pSubCmdArray)) {
				array_unset_slot(pCmdArray, i);
				continue;
			}

			/* get sub command in fixed position */
			pSubCmd = array_slot(pCmd->pSubCmdArray, index);
			if (NULL == pSubCmd) {
				array_unset_slot(pCmdArray, i);
				continue;
			}

			matched = 0;

			switch (pSubCmd->type) {
			case SUBCMD_TYPE_KEY:
				if (strncmp(token, pSubCmd->key, tokenLen) == 0) {
					/* if matched this time and has matched before, and the key
					 * is different with the current key, it MUST be ambiguous.
					 */
					if (matchedKey
					    && (strcmp(matchedKey, pSubCmd->key)
						!= 0)) {
						return SUBCMD_AMBIGUOUS_YES;
					} else {
						matchedKey = pSubCmd->key;
					}
					matched++;
				}
				break;

			case SUBCMD_TYPE_SELECT:
				pInnerSubCmdArray = pSubCmd->pSubCmdArray;

				/* compare with every sub command in the inner sub command array */
				for (j = 0; j < array_index(pInnerSubCmdArray);
				     j++) {
					if (((pInnerSubCmd =
					      array_slot(pInnerSubCmdArray,
							 j)) != NULL)
					    && (pInnerSubCmd->type ==
						SUBCMD_TYPE_KEY)
					    &&
					    (strncmp
					     (token, pInnerSubCmd->key,
					      tokenLen) == 0)) {
						/* if matched this time and has matched before, and the key
						 * is different with the current key, it MUST be ambiguous.
						 */
						if (matchedKey
						    &&
						    (strcmp
						     (matchedKey,
						      pInnerSubCmd->key) !=
						     0)) {
							return
							    SUBCMD_AMBIGUOUS_YES;
						} else {
							matchedKey =
							    pInnerSubCmd->key;
						}
						matched++;
					}
				}
				break;

			default:
				break;
			}

			/* if the sub command is not matched, unset the command */
			if (0 == matched) {
				array_unset_slot(pCmdArray, i);
			}
		}

		break;

	case SUBCMD_MATCH_RANGE:
		break;

	case SUBCMD_MATCH_NO:
	default:
		break;
	}

	return SUBCMD_AMBIGUOUS_NO;
}

static int _cli_get_err_position(const char *lineStr, const int index)
{
	const char *cp;
	int i = 0;
	int errPosition = 0;

	/* parameter check */
	if (NULL == lineStr) {
		return 0;	/* pointer to the first character */
	}

	cp = lineStr;
	cp = _cli_trim_left(cp);

	while (*cp != '\0') {
		if (i == index) {
			errPosition = _cli_trim_left(cp) - lineStr;
			break;
		}

		/* get token from rest string */
		if (_cli_token(&cp) == NULL) {
			errPosition = _cli_trim_left(cp) - lineStr;
			break;
		}

		i++;
	}

	return errPosition;
}

static void _cli_set_err_str(VTY_T *pVty, int errId)
{
	switch (errId) {
	case CLI_FAIL:
		vty_err_str("Command execute fail.");
		break;
	case CLI_NO_MATCH:
	case CLI_UNRECOGNIZED:
		vty_err_str("Unrecognized command.");
		break;
	case CLI_AMBIGUOUS:
		vty_err_str("Ambiguous command.");
		break;
	case CLI_WARNING:
		vty_err_str("Warning.");
		break;
	case CLI_INCOMPLETE:
		vty_err_str("Incomplete Command .");
		break;
	case CLI_EXEED_ARGC_MAX:
		vty_err_str("Too much arguments.");
		break;
	case CLI_OK:
	case CLI_EMPTY_COMMAND:
	case CLI_EXEC_FAIL:
	case CLI_INVALID_PARAMETER:
	case CLI_NO_MEMORY:
	default:
		/* output nothing */
		break;
	}
}

/******************************************************************************
 *
 * description:
 *   execute command according to line buffer
 *
 * input:
 *   pVty - pointer to VTY
 *
 * output:
 *   pVty->errPosition - error position, -1 when command is Okay
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int cli_execute_cmd(VTY_T *pVty)
{
	ARRAY_T *pLine = NULL;
	ARRAY_T *pCmdArray = NULL;
	int executeRet = CLI_OK;
	int ret;
	int i;
	char *token;
	int match;
	CLI_CMD_T *pCmd;
	CLI_CMD_T *pMatchedCmd = NULL;
	int matchedCmdNum = 0;	/* number of matched commands */
	int incompleteCmdNum = 0;	/* number of incomplete commands */

	/* split line buf to token array */
	ret = _cli_split_line(pVty->line.data, &pLine);
	if (ret != CLI_OK) {
		executeRet = CLI_FAIL;
		goto EXIT;
	} else if (NULL == pLine) {
		executeRet = CLI_EMPTY_COMMAND;
		goto EXIT;
	} else if (array_num(pLine) > CLI_COMMAND_WORDS_MAX) {
		executeRet = CLI_EXEED_ARGC_MAX;
		goto EXIT;
	}

	/* get a copied command list according to current mode,
	 * the copied command list may be modified in next steps
	 */
	pCmdArray = array_copy(_cli_mode_cmd_array(pVty->cliMode));
	if (NULL == pCmdArray) {
		executeRet = CLI_FAIL;
		goto EXIT;
	}

	/* match every token of command line */
	for (i = 0; i < array_index(pLine); i++) {
		token = array_slot(pLine, i);
		if (NULL == token) {
			continue;
		}

		/* get match type after scan every command */
		match = _cli_token_match_cmd_array(token, pCmdArray, i);

		/* check whether it is unmatched command */
		if (0 == array_num(pCmdArray)) {
			executeRet = CLI_UNRECOGNIZED;
			pVty->errPosition =
			    _cli_get_err_position(pVty->line.data, i);
			goto EXIT;
		}

		/* check if the token is ambiguous */
		else if (_cli_token_is_ambiguous(token, pCmdArray, i, match)) {
			pVty->errPosition =
			    _cli_get_err_position(pVty->line.data, i);
			executeRet = CLI_AMBIGUOUS;
			goto EXIT;
		}
	}

	/* check remain commands in pCmdArray */
	for (i = 0; i < array_index(pCmdArray); i++) {
		pCmd = array_slot(pCmdArray, i);
		if (NULL == pCmd) {
			continue;
		}

		if (array_index(pLine) >= pCmd->cmdSize) {
			matchedCmdNum++;
			pMatchedCmd = pCmd;
		} else {
			incompleteCmdNum++;
		}
	}

	/* matchedCmdNum MUST be 1 */
	if (matchedCmdNum == 0) {
		if (incompleteCmdNum > 0) {
			executeRet = CLI_INCOMPLETE;
			goto EXIT;
		} else {
			executeRet = CLI_UNRECOGNIZED;
			goto EXIT;
		}
	} else if (matchedCmdNum > 1) {
		executeRet = CLI_AMBIGUOUS;
		goto EXIT;
	}

	/* execute the matched command */
	pVty->pExecCmd = (void *)pMatchedCmd;
	executeRet =
	    pMatchedCmd->func(pVty, (int)(pLine->num),
			      (const char **)(pLine->slot));

      EXIT:
	if (NULL != pLine) {
		_cli_free_line_array(pLine);
	}

	if (NULL != pCmdArray) {
		array_free(pCmdArray);
	}

	_cli_set_err_str(pVty, executeRet);

	return executeRet;
}

static CLI_SUBCMD_T gCrSubCmd = {
	SUBCMD_TYPE_CR,
	"<cr>",
	""
};

/******************************************************************************
 *
 * description:
 *   add new sub command to description array, if it is not exist.
 *
 * input:
 *   pDescArray - pointer to description array
 *   pNewSubCmd - pointer to new sub command
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
static int _cli_insert_desc_array(ARRAY_T *pDescArray,
				  CLI_SUBCMD_T *pNewSubCmd)
{
	int i;
	CLI_SUBCMD_T *pSubCmd;

	for (i = 0; i < array_index(pDescArray); i++) {
		pSubCmd = array_slot(pDescArray, i);
		if ((pSubCmd != NULL)
		    && (strcmp(pSubCmd->key, pNewSubCmd->key) == 0)) {
			return CLI_OK;
		}
	}

	return array_insert_slot(pDescArray, pNewSubCmd);
}

/******************************************************************************
 *
 * description:
 *   print sub command like "key  desctiption" in description array
 *
 * input:
 *   pDescArray - pointer to description array
 *
 * output:
 *
 * return:
 *   one of CLI_RET_T
 *
 ******************************************************************************/
static int _cli_print_desc_array(VTY_T *pVty, ARRAY_T *pDescArray)
{
	int i;
	CLI_SUBCMD_T *pSubCmd;
	int keyWidth = 0;

	/* calculate key str width */
	for (i = 0; i < array_index(pDescArray); i++) {
		pSubCmd = array_slot(pDescArray, i);
		if (NULL == pSubCmd) {
			continue;
		}

		keyWidth = MAX(strlen(pSubCmd->key), keyWidth);
	}

	for (i = 0; i < array_index(pDescArray); i++) {
		pSubCmd = array_slot(pDescArray, i);
		if (NULL == pSubCmd) {
			continue;
		}

		vty_print("  %-*s    %s\r\n", keyWidth, pSubCmd->key,
			  pSubCmd->desc);
	}

	return CLI_OK;
}

/******************************************************************************
 *
 * description:
 *   find matched command according to line buffer, and display description of
 *   sub commands. Support cases:
 *     * case "prompt> ?"
 *     * case "prompt> xxx yyy ?"
 *     * case "prompt> xxx yyy?"
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
int cli_describe_cmd(VTY_T *pVty)
{
	ARRAY_T *pLine = NULL;
	ARRAY_T *pCmdArray = NULL;
	ARRAY_T *pDescArray = NULL;
	int executeRet = CLI_OK;
	int ret;
	int i, j, lastIndex;
	char *token;
	int match;
	CLI_CMD_T *pCmd;
	CLI_SUBCMD_T *pSubCmd;

	/* split line buf to token array */
	ret = _cli_split_line(pVty->line.data, &pLine);
	if (ret != CLI_OK) {
		executeRet = CLI_FAIL;
		goto EXIT;
	} else if (NULL == pLine) {
		/* If pLine is NULL, it is case "prompt> ?".
		 * Then add a NULL token to the array.
		 */
		pLine = array_new(ARRAY_SLOT_MIN);
		array_insert_slot(pLine, NULL);
		if (pLine == NULL) {
			executeRet = CLI_FAIL;
			goto EXIT;
		}
	} else if (array_num(pLine) > CLI_COMMAND_WORDS_MAX) {
		executeRet = CLI_EXEED_ARGC_MAX;
		goto EXIT;
	} else if (isspace((int)pVty->line.data[pVty->line.len - 1])) {
		/* If last character of input buf is space, it is case "prompt> xxx yyy ?".
		 * Then add a empty token to the array.
		 */
		array_insert_slot(pLine, NULL);
	} else {
		/* it is case "prompt> xxx yyy?", do nothing */
	}

	/* init description array */
	pDescArray = array_new(16);
	if (NULL == pDescArray) {
		executeRet = CLI_FAIL;
		goto EXIT;
	}

	/* get a copied command list according to current mode,
	 * the copied command list may be modified in next steps
	 */
	pCmdArray = array_copy(_cli_mode_cmd_array(pVty->cliMode));
	if (NULL == pCmdArray) {
		executeRet = CLI_FAIL;
		goto EXIT;
	}

	/* Match every token of command line, except the last
	 * token of pLine. The last token will be handled in next step.
	 */
	for (i = 0; i < (array_index(pLine) - 1); i++) {
		token = array_slot(pLine, i);
		if (NULL == token) {
			continue;
		}

		/* get match type after scan every command */
		match = _cli_token_match_cmd_array(token, pCmdArray, i);

		/* check whether it is unmatched command */
		if (0 == array_num(pCmdArray)) {
			executeRet = CLI_UNRECOGNIZED;
			pVty->errPosition =
			    _cli_get_err_position(pVty->line.data, i);
			goto EXIT;
		}

		/* check if the token is ambiguous */
		if (_cli_token_is_ambiguous(token, pCmdArray, i, match)) {
			executeRet = CLI_AMBIGUOUS;
			pVty->errPosition =
			    _cli_get_err_position(pVty->line.data, i);
			goto EXIT;
		}
	}

	/* match last token of command line */
	lastIndex = i;
	token = array_slot(pLine, lastIndex);
	if (NULL != token) {
		/* get match type after scan every command */
		match = _cli_token_match_cmd_array(token, pCmdArray, lastIndex);
	}

	/* collect matched sub command and build description array */
	for (i = 0; i < array_index(pCmdArray); i++) {
		/* get a command from array */
		pCmd = array_slot(pCmdArray, i);
		if (NULL == pCmd) {
			continue;
		}

		/* check whether the command is invisible */
		if (pCmd->invisible) {
			continue;
		}

		/* If the last token is NULL(case "xx yy ?") and the command
		 * is completed, then add <cr> to description array.
		 */
		if ((NULL == token) && (lastIndex == pCmd->cmdSize)) {
			_cli_insert_desc_array(pDescArray, &gCrSubCmd);
		} else {
			/* get sub command in fixed postion */
			pSubCmd = array_slot(pCmd->pSubCmdArray, lastIndex);
			if (NULL == pSubCmd) {
				array_unset_slot(pCmdArray, i);
				continue;
			}

			/* compare sub command according to the type */
			if (SUBCMD_TYPE_SELECT == pSubCmd->type) {
				CLI_SUBCMD_T *pInnerSubCmd;
				ARRAY_T *pInnerSubCmdArray;

				if ((pInnerSubCmdArray =
				     pSubCmd->pSubCmdArray) == NULL) {
					continue;
				}

				/* compare with every sub command in the inner sub command array */
				for (j = 0; j < array_index(pInnerSubCmdArray);
				     j++) {
					if ((pInnerSubCmd =
					     array_slot(pInnerSubCmdArray,
							j)) != NULL) {
						if (_cli_token_match_subcmd
						    (token, pInnerSubCmd)) {
							_cli_insert_desc_array
							    (pDescArray,
							     pInnerSubCmd);
						}
					}
				}
			} else {
				if (_cli_token_match_subcmd(token, pSubCmd)) {
					_cli_insert_desc_array(pDescArray,
							       pSubCmd);
				}
			}
		}
	}

	/* if the description array is empty, then it match no command */
	if (array_num(pDescArray) == 0) {
		executeRet = CLI_UNRECOGNIZED;
		pVty->errPosition =
		    _cli_get_err_position(pVty->line.data,
					  array_index(pLine) - 1);
		goto EXIT;
	}
#if 0
	/* sort sub commands in pDescArray */
	osal_qsort(pDescArray->slot,
		   array_index(pDescArray), sizeof(void *), _cli_compare_desc);
#endif

	/* display description array */
	_cli_print_desc_array(pVty, pDescArray);
	executeRet = CLI_OK;

      EXIT:
	if (NULL != pLine) {
		_cli_free_line_array(pLine);
	}

	if (NULL != pDescArray) {
		array_free(pDescArray);
	}

	if (NULL != pCmdArray) {
		array_free(pCmdArray);
	}

	_cli_set_err_str(pVty, executeRet);

	return executeRet;
}

/******************************************************************************
 *
 * description:
 *   compare token with sub command, if it can be completed, return
 *
 * input:
 *   token   - token string
 *   pSubCmd - pointer to sub command
 *
 * output:
 *
 * return:
 *   one of SUBCMD_MATCH_T
 *
 ******************************************************************************/
static int _cli_token_complete_subcmd(const char *token, CLI_SUBCMD_T *pSubCmd)
{
	if (SUBCMD_TYPE_KEY == pSubCmd->type) {
		return _cli_token_match_key(token, pSubCmd->key);
	} else {
		return SUBCMD_MATCH_NO;
	}
}

/******************************************************************************
 *
 * description:
 *   calculate max common characters of sub command keys, make complete string
 *
 * input:
 *   pDescArray - pointer to sub command
 *
 * output:
 *   completeStr - complete string
 *
 * return:
 *   complete string len
 *
 ******************************************************************************/
static int _cli_get_complete_str(ARRAY_T *pDescArray, char *completeStr)
{
	int i, j;
	int index;
	int commonCharCount;
	char *lastKey;
	char *thisKey;
	CLI_SUBCMD_T *pSubCmd;

	completeStr[0] = '\0';

	if (array_num(pDescArray) < 2) {
		return 0;
	}

	pSubCmd = (CLI_SUBCMD_T *) array_slot(pDescArray, 0);
	if (pSubCmd == NULL)
		return 0;
	lastKey = pSubCmd->key;
	commonCharCount = strlen(lastKey);

	for (i = 1; i < array_index(pDescArray); i++) {
		thisKey = ((CLI_SUBCMD_T *) array_slot(pDescArray, i))->key;
		if (NULL == thisKey) {
			continue;
		}

		index = MIN(strlen(lastKey), strlen(thisKey));
		for (j = 0; j < index; j++) {
			if (lastKey[j] != thisKey[j]) {
				break;
			}
		}

		commonCharCount = MIN(commonCharCount, j);

		lastKey = thisKey;
	}

	strcpy(completeStr, lastKey);
	completeStr[commonCharCount] = '\0';

	return commonCharCount;
}

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
int cli_complete_cmd(VTY_T *pVty)
{
	ARRAY_T *pLine = NULL;
	ARRAY_T *pCmdArray = NULL;
	ARRAY_T *pDescArray = NULL;
	int execRet = CLI_OK;
	int ret;
	int i, j, lastIndex;
	char *token;
	char *lastToken;
//    int             lastTokenLen;
	int match;
	CLI_CMD_T *pCmd;
	CLI_SUBCMD_T *pSubCmd;
	char completeStr[64] = { 0 };
	int completeStrLen;

	/* split line buf to token array */
	ret = _cli_split_line(pVty->line.data, &pLine);
	if (ret != CLI_OK) {
		execRet = CLI_FAIL;
		goto EXIT;
	} else if (NULL == pLine) {
		/* If pLine is NULL, it is case "prompt> \t".
		 * Then add a NULL token to the array.
		 */
		execRet = CLI_FAIL;
		goto EXIT;
	} else if (array_num(pLine) > CLI_COMMAND_WORDS_MAX) {
		execRet = CLI_EXEED_ARGC_MAX;
		goto EXIT;
	} else if (isspace((int)pVty->line.data[pVty->line.len - 1])) {
		/* If last character of input buf is space, it is case "prompt> xxx yyy \t".
		 * Then add a empty token to the array.
		 */
		array_insert_slot(pLine, NULL);
	} else {
		/* it is case "prompt> xxx yyy\t", do nothing */
	}

	/* init description array */
	pDescArray = array_new(16);
	if (NULL == pDescArray) {
		execRet = CLI_FAIL;
		goto EXIT;
	}

	/* get a copied command list according to current mode,
	 * the copied command list may be modified in next steps
	 */
	pCmdArray = array_copy(_cli_mode_cmd_array(pVty->cliMode));
	if (NULL == pCmdArray) {
		execRet = CLI_FAIL;
		goto EXIT;
	}

	/* Match every token of command line, except the last
	 * token of pLine. The last token will be handled in next step.
	 */
	for (i = 0; i < (array_index(pLine) - 1); i++) {
		token = array_slot(pLine, i);
		if (NULL == token) {
			continue;
		}

		/* get match type after scan every command */
		match = _cli_token_match_cmd_array(token, pCmdArray, i);

		/* check if the token is ambiguous */
		if (_cli_token_is_ambiguous(token, pCmdArray, i, match)) {
			execRet = CLI_AMBIGUOUS;
			vty_print("%% Ambiguous command.\r\n");
			goto EXIT;
		}
	}

	/* match last token of command line */
	lastIndex = i;
	lastToken = array_slot(pLine, lastIndex);
#if 0
	if (NULL != token) {
		/* get match type after scan every command */
		match = _cli_token_match_cmd_array(token, pCmdArray, lastIndex);
	}
#endif

	/* collect matched sub command and build sub command array */
	for (i = 0; i < array_index(pCmdArray); i++) {
		/* get a command from array */
		pCmd = array_slot(pCmdArray, i);
		if (NULL == pCmd) {
			continue;
		}

		/* check whether the command is invisible */
		if (pCmd->invisible) {
			continue;
		}

		/* get sub command in fixed postion */
		pSubCmd = array_slot(pCmd->pSubCmdArray, lastIndex);
		if (NULL == pSubCmd) {
			array_unset_slot(pCmdArray, i);
			continue;
		}

		/* compare sub command according to the type */
		if (SUBCMD_TYPE_SELECT == pSubCmd->type) {
			CLI_SUBCMD_T *pInnerSubCmd;
			ARRAY_T *pInnerSubCmdArray;

			if ((pInnerSubCmdArray = pSubCmd->pSubCmdArray) == NULL) {
				continue;
			}

			/* compare with every sub command in the inner sub command array */
			for (j = 0; j < array_index(pInnerSubCmdArray); j++) {
				if ((pInnerSubCmd =
				     array_slot(pInnerSubCmdArray,
						j)) != NULL) {
					if (_cli_token_complete_subcmd
					    (lastToken, pInnerSubCmd)) {
						_cli_insert_desc_array
						    (pDescArray, pInnerSubCmd);
					}
				}
			}
		} else {
			if (_cli_token_complete_subcmd(lastToken, pSubCmd)) {
				_cli_insert_desc_array(pDescArray, pSubCmd);
			}
		}
	}

	/* no matchedcommand */
	if (array_num(pDescArray) == 0) {
		execRet = CLI_UNRECOGNIZED;
		goto EXIT;
	}

	/* only one matched command */
	else if (array_num(pDescArray) == 1) {
		execRet = CLI_COMPLETE_FULL_MATCH;
		pSubCmd = array_slot(pDescArray, 0);
		if (pSubCmd == NULL) {
			execRet = CLI_FAIL;
			goto EXIT;
		}
		vty_complete_cmd(pVty, pSubCmd->key, SA_TRUE);
		goto EXIT;
	}

	/* multiple matched command */
	else {
		/* case "xxx yy\t", try to complete last word as long as possible */
		if (lastToken != NULL) {
			completeStrLen =
			    _cli_get_complete_str(pDescArray, completeStr);

			if ((completeStrLen > 0)
			    && (strlen(lastToken) < completeStrLen)) {
				execRet = CLI_COMPLETE_PARTLY_MATCH;
				vty_complete_cmd(pVty, completeStr, SA_FALSE);
				goto EXIT;
			}
		}

		/* multiple matched commands to display */
		_cli_print_desc_array(pVty, pDescArray);
		execRet = CLI_COMPLETE_MULTI_MATCH;
		goto EXIT;
	}

      EXIT:
	if (NULL != pLine) {
		_cli_free_line_array(pLine);
	}

	if (NULL != pDescArray) {
		array_free(pDescArray);
	}

	if (NULL != pCmdArray) {
		array_free(pCmdArray);
	}

	return execRet;
}

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
int cli_display_arguments(VTY_T *pVty, const int argc, const char **argv)
{
	int i, j;
	CLI_CMD_T *pCmd = (CLI_CMD_T *) (pVty->pExecCmd);
	CLI_SUBCMD_T *pSubCmd;
	CLI_SUBCMD_T *pInnerSubCmd;
	ARRAY_T *pSubCmdArray;
	sa_bool_t matched;

	if (NULL == pCmd) {
		return 0;
	}

	vty_print("  Full command: \"");

	for (i = 0; i < argc; i++) {
		pSubCmd = array_slot(pCmd->pSubCmdArray, i);
		if (pSubCmd == NULL)
			continue;

		switch (pSubCmd->type) {
		case SUBCMD_TYPE_KEY:
			vty_print("%s ", pSubCmd->key);
			break;
		case SUBCMD_TYPE_SELECT:
			matched = SA_FALSE;
			pSubCmdArray = pSubCmd->pSubCmdArray;

			/* handle each sub command in the array */
			for (j = 0; j < array_index(pSubCmdArray); j++) {
				pInnerSubCmd = array_slot(pSubCmdArray, j);
				if (NULL == pInnerSubCmd) {
					continue;
				}

				if ((SUBCMD_TYPE_KEY == pInnerSubCmd->type)
				    &&
				    (strncmp
				     (pInnerSubCmd->key, argv[i],
				      strlen(argv[i])) == 0)) {
					vty_print("%s ", pInnerSubCmd->key);
					matched = SA_TRUE;
				}
			}

			if (!matched) {
				vty_print("%s ", argv[i]);
			}

			break;
		default:
			vty_print("%s ", argv[i]);
			break;
		}
	}

	vty_print("\"\r\n");

	return 0;
}
