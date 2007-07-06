/*
 * out_rrd.c		RRD Output
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/node.h>
#include <bmon/output.h>
#include <bmon/graph.h>
#include <bmon/input.h>
#include <bmon/utils.h>
#include <inttypes.h>

#define MAX_RRA 128

static const char *c_path = "./";
static char *c_step = "1";
static char *c_heartbeat = "2";
static char *c_rra[MAX_RRA];
static int c_rra_index;
static int c_update_interval = 1;

static void create_rrd(char *file, item_t *it)
{
	char *argv[256];
	char nows[32];
	int i, m, ul;

	memset(argv, 0, sizeof(argv));

	snprintf(nows, sizeof(nows), "%lld", (unsigned long long) (time(0) - 1));

	argv[0] = "create";
	argv[1] = file;
	argv[2] = "--start";
	argv[3] = nows;
	argv[4] = "--step";
	argv[5] = c_step;

	i = 6;

	for (m = 0; m < ATTR_HASH_MAX;  m++) {
		stat_attr_t *a;
		for (a = it->i_attrs[m]; a; a = a->a_next) {
			char ds[128];
			if (i >= (253 - c_rra_index))
				goto overflow;
			snprintf(ds, sizeof(ds), "DS:%s_rx:%s:%s:U:U",
			    type2name(a->a_type),
			    a->a_flags & ATTR_FLAG_IS_RATE ? "GAUGE" : "COUNTER",
			    c_heartbeat);
			argv[i++] = strdup(ds);

			if (i >= (253 - c_rra_index))
				goto overflow;
			snprintf(ds, sizeof(ds), "DS:%s_tx:%s:%s:U:U",
			    type2name(a->a_type),
			    a->a_flags & ATTR_FLAG_IS_RATE ? "GAUGE" : "COUNTER",
			    c_heartbeat);
			argv[i++] = strdup(ds);
		}
	}
	
	ul = i;

	if (i + c_rra_index >= 255)
		goto overflow;

	for (m = 0; m < c_rra_index; m++)
		argv[i++] = c_rra[m];

	optind = 0; /* What a nice undocumented precondition */
	opterr = 0;

	if (rrd_create(i, argv) < 0)
		fprintf(stderr, "rrd_create failed: %s\n", rrd_get_error());

	for (i = 6; i < ul && argv[i]; i++)
		free(argv[i]);

	return;

overflow:
	quit("Argument overflow, blame the RRD API\n");
}

static void update_rrd(char *file, item_t *it)
{
	char *argv[6];
	char template[256];
	char data[1024];
	int m;

	memset(argv, 0, sizeof(argv));

	argv[0] = "update";
	argv[1] = file;
	argv[2] = "--template";

	memset(template, 0, sizeof(template));

	for (m = 0; m < ATTR_HASH_MAX; m++) {
		stat_attr_t *a;
		for (a = it->i_attrs[m]; a; a = a->a_next) {
			if (template[0])
				strncat(template, ":",
				    sizeof(template) - strlen(template) - 1);
			strncat(template, type2name(a->a_type),
			    sizeof(template) - strlen(template) - 1);
			strncat(template, "_rx",
			    sizeof(template) - strlen(template) - 1);
			strncat(template, ":",
			    sizeof(template) - strlen(template) - 1);
			strncat(template, type2name(a->a_type),
			    sizeof(template) - strlen(template) - 1);
			strncat(template, "_tx",
			    sizeof(template) - strlen(template) - 1);
		}
	}

	argv[3] = template;

	snprintf(data, sizeof(data), "%lld",
	    rtiming.rt_last_read.tv_sec);

	for (m = 0; m < ATTR_HASH_MAX; m++) {
		stat_attr_t *a;
		for (a = it->i_attrs[m]; a; a = a->a_next) {
			char valuepair[64];
			snprintf(valuepair, sizeof(valuepair),
			    "%s%lld:%lld",
			    data[0] ? ":" : "", attr_get_rx(a),
			    attr_get_tx(a));

			strncat(data, valuepair,
			    sizeof(data) - strlen(data) - 1);
		}
	}

	argv[4] = data;

	for (m = 0; m < 5; m++)
		printf("%s ", argv[m]);
	printf("\n");

	optind = 0; /* What a nice undocumented precondition */
	opterr = 0;

	rrd_update(5, argv);
}

static void write_per_item(item_t *it, void *arg)
{
	node_t *node = (node_t *) arg;
	char file[FILENAME_MAX];

	snprintf(file, sizeof(file), "%s/%s_%d.rrd", c_path, node->n_name, it->i_index);

	if (access(file, W_OK) != 0)
		create_rrd(file, it);

	if (access(file, W_OK) == 0)
		update_rrd(file, it);
}

static void write_per_node(node_t *node, void *arg)
{
	foreach_item(node, write_per_item, (void *) node);
}

void rrd_draw(void)
{
	static int rem = 1;

	if (--rem)
		return;
	else
		rem = c_update_interval;

	foreach_node(write_per_node, NULL);
}

static void print_module_help(void)
{
	printf(
	"RRD - RRD Output\n" \
	"\n" \
	"  Writes updated to RRD databases. Databases are created if needed\n" \
	"  and non-existent\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    path=PATH        Output directory\n" \
	"    step=SECS        Interval RRD expects updates (default: 1 second)\n" \
	"    heartbeat=SECS   Maximum interval until RRD throws away an\n" \
	"                     update (default: 2 seconds)\n" \
	"    interval=NUM     Update interval in read interval cycles\n" \
	"    rra=RRA_DEF      RRA definition (default: RRA:AVERAGE:0.5:1:86400)\n");
}

static void rrd_set_opts(tv_t *attrs)
{
	static int first_rra = 1;

	while (attrs) {
		if (!strcasecmp(attrs->type, "path") && attrs->value)
			c_path = attrs->value;
		else if (!strcasecmp(attrs->type, "step") && attrs->value)
			c_step = attrs->value;
		else if (!strcasecmp(attrs->type, "heartbeat") && attrs->value)
			c_heartbeat = attrs->value;
		else if (!strcasecmp(attrs->type, "interval") && attrs->value)
			c_update_interval = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "rra") && attrs->value) {
			if (first_rra) {
				c_rra_index = 0;
				first_rra = 0;
			}
			if (c_rra_index < MAX_RRA)
				c_rra[c_rra_index++] = attrs->value;
		} else if (!strcasecmp(attrs->type, "help")) {
			print_module_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static int rrd_probe(void)
{
	return 1;
}

static struct output_module rrd_ops = {
	.om_name = "rrd",
	.om_draw = rrd_draw,
	.om_set_opts = rrd_set_opts,
	.om_probe = rrd_probe,
};

static void __init save_init(void)
{
	c_rra[0] = "RRA:AVERAGE:0.5:1:86400";
	c_rra_index = 1;
	register_secondary_output_module(&rrd_ops);
}
