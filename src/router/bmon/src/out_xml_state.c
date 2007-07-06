/*
 * out_xml_state.c	State based XML output
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

static const char *c_file;
static int c_nolock = 0;

static void write_attr(stat_attr_t *a, void *arg)
{
	FILE *fd = (FILE *) arg;
	fprintf(fd,
	    "<attr name=\"%s\" rx=\"%" PRIu64 "\" tx=\"%" PRIu64 "\" />\n",
	    type2name(a->a_type), attr_get_rx(a), attr_get_tx(a));
}

static void write_per_item(item_t *it, void *arg)
{
	FILE *fd = (FILE *) arg;
	fprintf(fd, "<item index=\"%d\" name=\"%s\" handle=\"%d\" " \
		    "level=\"%d\" ischild=\"%d\" parent=\"%d\" >\n",
		    it->i_index, it->i_name, it->i_handle, it->i_level,
		    !!(it->i_flags & ITEM_FLAG_IS_CHILD), it->i_parent);
	foreach_attr(it, &write_attr, fd);
	fprintf(fd, "</item>\n");
}

static void write_per_node(node_t *node, void *arg)
{
	FILE *fd = (FILE *) arg;
	fprintf(fd, "<node index=\"%d\" name=\"%s\" from=\"%s\">\n",
		    node->n_index, node->n_name, node->n_from ? : "local");
	foreach_item(node, write_per_item, fd);
	fprintf(fd, "</node>\n");
}

void xml_state_draw(void)
{
	FILE *fd;

	fd = fopen(c_file, "w");
	if (fd == NULL)
		return;

	/* We ignore a locking failure and simply try again next time */
	if (!c_nolock && flock(fileno(fd), LOCK_EX | LOCK_NB) < 0)
		return;
	
	fprintf(fd,
	    "<?xml version=\"1.0\"?>\n" \
	    "<statefile ts_sec=\"%" PRId64 "\" ts_usec=\"%" PRId64 "\">\n",
	    rtiming.rt_last_read.tv_sec, rtiming.rt_last_read.tv_usec);

	foreach_node(write_per_node, fd);
	fprintf(fd, "</statefile>\n");

	if (!c_nolock)
		flock(fileno(fd), LOCK_UN | LOCK_NB);
	fclose(fd);
}

static void print_module_help(void)
{
	printf(
	"xml_state - State Based XML Output\n" \
	"\n" \
	"  XML based state output, outputs counters as-is as XML\n" \
	"  objects into a file. The file is overwritten in each\n" \
	"  cycle and locked during this period if nolock is not\n" \
	"  specified.\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    path=FILE        Output file (default: stdout)\n" \
	"    nolock           Do not lock the file\n");
}

static void xml_state_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "path") && attrs->value)
			c_file = attrs->value;
		else if (!strcasecmp(attrs->type, "nolock"))
			c_nolock = 1;
		else if (!strcasecmp(attrs->type, "help")) {
			print_module_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static int xml_state_probe(void)
{
	FILE *t;

	if (c_file == NULL) {
		fprintf(stderr, "xml_state: You must specify a output file\n");
		return 0;
	}

	t = fopen(c_file, "w");
	if (t == NULL) {
		fprintf(stderr, "xml_state: Unable to open output file %s:%s\n",
		    c_file, strerror(errno));
		return 0;
	}

	return 1;
}

static struct output_module xml_state_ops = {
	.om_name = "xml_state",
	.om_draw = xml_state_draw,
	.om_set_opts = xml_state_set_opts,
	.om_probe = xml_state_probe
};

static void __init xml_state_init(void)
{
	register_secondary_output_module(&xml_state_ops);
}
