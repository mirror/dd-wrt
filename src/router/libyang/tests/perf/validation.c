/**
 * @file validation.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief performance test - validating data.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include <libyang/libyang.h>

int main(int argc, char *argv[])
{
	struct ly_ctx *ctx;
	struct lyd_node *data;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s model.yin data.xml\n", argv[0]);
		return 1;
	}

	/* libyang context */
	ctx = ly_ctx_new(NULL, 0);
	if (!ctx) {
		fprintf(stderr, "Failed to create context.\n");
		return 1;
	}

	/* schema */
        if (!lys_parse_path(ctx, argv[1], LYS_IN_YIN)) {
		fprintf(stderr, "Failed to load data model.\n");
		goto cleanup;
	}

	/* data */
	data = lyd_parse_path(ctx, argv[2], LYD_XML, LYD_OPT_DESTRUCT | LYD_OPT_CONFIG);
	if (!data) {
		fprintf(stderr, "Failed to load data.\n");
	}
	
cleanup:
    	lyd_free_withsiblings(data);
	ly_ctx_destroy(ctx, NULL);

	return 0;
}

