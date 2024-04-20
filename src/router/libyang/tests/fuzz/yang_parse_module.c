#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../src/common.h"
#include "../../src/tree_schema_internal.h"

LY_ERR yang_parse_module(struct lys_yang_parser_ctx **context, const char *data, struct lys_module *mod);

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
	struct lys_module *mod = NULL;
	struct lys_yang_parser_ctx *context = NULL;
	uint8_t *data = NULL;
	struct ly_ctx *ctx = NULL;
	static bool log = false; 
	LY_ERR err;
	
	if (!log) {
		ly_log_options(0);
		log = true;
	}

	err = ly_ctx_new(NULL, 0, &ctx);
	if (err != LY_SUCCESS) {
		fprintf(stderr, "Failed to create new context\n");
		return 0;
	}

	data = malloc(len + 1);
	if (data == NULL) {
		fprintf(stderr, "Out of memory\n");
		return 0;
	}
	memcpy(data, buf, len);
	data[len] = 0;

	mod = calloc(1, sizeof *mod);
	if (mod == NULL) {
		fprintf(stderr, "Out of memory\n");
		return 0;
	}
	mod->ctx = ctx;

	yang_parse_module(&context, (const char *) data, mod);

	free(data);
	free(mod);
	ly_ctx_destroy(ctx);
	return 0;
}
