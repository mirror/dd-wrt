#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "libyang.h"

LY_ERR buf_add_char(struct ly_ctx *ctx, const char **input, size_t len, char **buf, size_t *buf_len, size_t *buf_used);

int LLVMFuzzerTestOneInput(uint8_t const *buf, size_t len)
{
	struct ly_ctx *ctx = NULL;
	LY_ERR err;
	uint8_t *data = NULL;
	uint8_t *old_data = NULL;
	uint8_t *dest = NULL;
	size_t dest_len = 0;
	size_t used = 0;
	static bool log = false;

	if (!log) {
		ly_log_options(0);
		log = true;
	}

	err = ly_ctx_new(NULL, 0, &ctx);
	if (err != LY_SUCCESS) {
		fprintf(stderr, "Failed to create context\n");
		return 0;
	}

	data = malloc(len);
	if (data == NULL) {
		return 0;
	}

	dest = malloc(len);
	if (dest == NULL) {
		return 0;
	}
	dest_len = len;

	memcpy(data, buf, len);
	old_data = data;
	err = buf_add_char(ctx, (const char **) &data, len, (char **) &dest, &dest_len, &used);

	free(old_data);
	free(dest);
	ly_ctx_destroy(ctx);

	return 0;
}
