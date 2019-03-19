/**
 * @file addloop.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief performance test - adding data.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <libyang/libyang.h>

int main(int argc, char *argv[])
{
	int fd, i;
	struct ly_ctx *ctx = NULL;
	char buf[30];
	struct lyd_node *data = NULL, *next;
	const struct lys_module *mod;

	/* libyang context */
        ctx = ly_ctx_new(NULL, 0);
        if (!ctx) {
                fprintf(stderr, "Failed to create context.\n");
                return 1;
        }

        /* schema */
        if (!(mod = lys_parse_path(ctx, argv[1], LYS_IN_YIN))) {
                fprintf(stderr, "Failed to load data model.\n");
                goto cleanup;
        }

	/* data */
	data = NULL;
	fd = open("./addloop_result.xml", O_WRONLY | O_CREAT, 0666);
	data = NULL;
	for(i = 1; i <= 5000; i++) {
		next = lyd_new(NULL, mod, "ptest1");
		// if (i == 2091) {sprintf(buf, "%d", 1);} else {
		sprintf(buf, "%d", i);//}
		lyd_new_leaf(next, mod, "index", buf);
		lyd_new_leaf(next, mod, "p1", buf);
		if (!data) {
			data = next;
		} else {
			lyd_insert_after(data->prev, next);
		}
		if (lyd_validate(&data, LYD_OPT_CONFIG, NULL)) {
			goto cleanup;
		}
		//lyd_print_fd(fd, data, LYD_XML);
	}
	lyd_print_fd(fd, data, LYD_XML, LYP_WITHSIBLINGS | LYP_FORMAT);
	close(fd);

cleanup:
	lyd_free_withsiblings(data);
	ly_ctx_destroy(ctx, NULL);

	return 0;
}
