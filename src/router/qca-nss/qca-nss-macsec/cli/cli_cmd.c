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

//#include "osal_common.h"
#include "cli.h"
#include "cli_cmd.h"

#define CLI_UNIT_ID_MIN 0
#define CLI_UNIT_ID_MAX 1

typedef sa_u32_t(*CLI_VALUE_FUN_T) (void);

typedef struct {
	char *funcName;
	CLI_VALUE_FUN_T func;
} CLI_VALUE_STR_FUN_T;


extern int cli_cmd_fal_init(void);

/* unit */
static sa_u32_t min_unit_id(void)
{
	return CLI_UNIT_ID_MIN;
}

static sa_u32_t max_unit_id(void)
{
	return CLI_UNIT_ID_MAX;
}

static CLI_VALUE_STR_FUN_T gCliValueStrFunArray[] = {
	/* unit  */
	{"min_unit_id", min_unit_id},
	{"max_unit_id", max_unit_id},
};

int cli_get_value_by_fun_name(const char *funcName, sa_u32_t *pValue)
{
	int i;
	int arraySize =
	    (sizeof(gCliValueStrFunArray) / sizeof(CLI_VALUE_STR_FUN_T));

	for (i = 0; i < arraySize; i++) {
		if (strcmp(funcName, gCliValueStrFunArray[i].funcName) == 0) {
			*pValue = gCliValueStrFunArray[i].func();
			return CLI_OK;
		}
	}

	CLI_DEBUG("Cannot find \"%s\" matched function\r\n", funcName);
	return CLI_FAIL;
}

char *cli_emsg_get(int rc)
{
	char *emsg = NULL;

	switch (rc) {
	case 0:
		break;
	case CLI_ERROR_RESOURCE:
		emsg = "ERROR_RESOURCE";
		break;
	case CLI_ERROR_PARAM:
		emsg = "ERROR_PARAM";
		break;
	case CLI_ERROR_NOT_FOUND:
		emsg = "ERROR_NOT_FOUND";
		break;
	case CLI_ERROR_CONFLICT:
		emsg = "ERROR_CONFLICT";
		break;
	case CLI_ERROR_TIMEOUT:
		emsg = "ERROR_TIMEOUT";
		break;
	case CLI_ERROR_NOT_SUPPORT:
		emsg = "ERROR_NOT_SUPPORT";
		break;
	case CLI_ERROR_ERROR:
		emsg = "ERROR_ERROR";
		break;
	default:
		break;
	}

	return emsg;
}

void cli_cmd_init(void)
{
	cli_cmd_basic_mode_init();

	cli_cmd_basic_init();

	cli_cmd_fal_init();

	return;
}
