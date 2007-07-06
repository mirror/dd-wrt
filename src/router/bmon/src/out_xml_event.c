/*
 * out_xml_event.c	Event based XML output
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
static FILE *outfd;

static void write_attr(stat_attr_t *a, void *arg)
{
	fprintf(outfd,
	    "<attr name=\"%s\" rx=\"%" PRIu64 "\" tx=\"%" PRIu64 "\" />\n",
	    type2name(a->a_type), attr_get_rx(a), attr_get_tx(a));
}

static void write_per_item(item_t *it, void *arg)
{
	fprintf(outfd, "<item index=\"%d\" name=\"%s\" handle=\"%d\" " \
		       "level=\"%d\" ischild=\"%d\" parent=\"%d\" >\n",
		       it->i_index, it->i_name, it->i_handle, it->i_level,
		       !!(it->i_flags & ITEM_FLAG_IS_CHILD), it->i_parent);
	foreach_attr(it, &write_attr, NULL);
	fprintf(outfd, "</item>\n");
}

static void write_per_node(node_t *node, void *arg)
{
	fprintf(outfd, "<node index=\"%d\" name=\"%s\" from=\"%s\">\n",
			node->n_index, node->n_name, node->n_from ? : "local");
	foreach_item(node, write_per_item, (void *) node);
	fprintf(outfd, "</node>\n");
}

void xml_event_draw(void)
{
	if (outfd == NULL) {
		if (c_file) {
			umask(0133);
			outfd = fopen(c_file, "w");
			if (outfd == NULL)
				quit("fopen(%s) failed: %s\n",
				    c_file, strerror(errno));
		} else
			outfd = stdout;
		
		fprintf(outfd,
		    "<?xml version=\"1.0\"?>\n" \
		    "<eventfile start=\"%llu\">\n",
		    (unsigned long long) time(0));
	}

	fprintf(outfd,
	    "<update ts_sec=\"%" PRId64 "\" ts_usec=\"%" PRId64 "\">\n",
	    rtiming.rt_last_read.tv_sec, rtiming.rt_last_read.tv_usec);
	foreach_node(write_per_node, NULL);
	fprintf(outfd, "</update>\n");
}

static void print_module_help(void)
{
	printf(
	"xml_event - Event Based XML Output\n" \
	"\n" \
	"  XML based event output, outputs counter as-is in a flow\n" \
	"  of XML objects.\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    path=FILE        Output file (default: stdout)\n");
}

static void xml_event_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "path") && attrs->value)
			c_file = attrs->value;
		else if (!strcasecmp(attrs->type, "help")) {
			print_module_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static void xml_event_shutdown(void)
{
	if (outfd) {
		fprintf(outfd, "</eventfile>");
		fclose(outfd);
	}
}

static int xml_event_probe(void)
{
	return 1;
}

static struct output_module xml_event_ops = {
	.om_name = "xml_event",
	.om_draw = xml_event_draw,
	.om_set_opts = xml_event_set_opts,
	.om_probe = xml_event_probe,
	.om_shutdown = xml_event_shutdown,
};

static void __init xml_event_init(void)
{
	register_secondary_output_module(&xml_event_ops);
}
