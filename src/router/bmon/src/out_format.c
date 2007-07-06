/*
 * out_format.c		Formatted Output
 *
 * Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
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
#include <bmon/graph.h>
#include <bmon/conf.h>
#include <bmon/output.h>
#include <bmon/input.h>
#include <bmon/node.h>
#include <bmon/utils.h>

static int c_quit_after = -1;
static char *c_format;
static int c_debug = 0;
static FILE *c_fd;

enum {
	OT_STRING,
	OT_TOKEN
};

static struct out_token {
	int ot_type;
	char *ot_str;
} *out_tokens;

static int token_index;
static int out_tokens_size;

static inline char *itos(b_cnt_t n)
{
	static char str[128];
	snprintf(str, sizeof(str), "%" PRIu64, n);
	return str;
}

static char *get_token(node_t *node, item_t *it, const char *token)
{
	if (!strcasecmp(token, "node:index"))
		return itos(node->n_index);
	else if (!strcasecmp(token, "node:name"))
		return node->n_name;
	else if (!strcasecmp(token, "node:from"))
		return node->n_from;
	else if (!strcasecmp(token, "node:nitems"))
		return itos(node->n_nitems);
	else if (!strcasecmp(token, "node:rx_maj_total"))
		return itos(node->n_rx_maj_total);
	else if (!strcasecmp(token, "node:tx_maj_total"))
		return itos(node->n_tx_maj_total);
	else if (!strcasecmp(token, "node:rx_min_total"))
		return itos(node->n_rx_min_total);
	else if (!strcasecmp(token, "node:tx_min_total"))
		return itos(node->n_tx_min_total);
	else if (!strcasecmp(token, "item:name"))
		return it->i_name;
	else if (!strcasecmp(token, "item:desc"))
		return it->i_desc ? it->i_desc : "none";
	else if (!strcasecmp(token, "item:index"))
		return itos(it->i_index);
	else if (!strcasecmp(token, "item:nattrs"))
		return itos(it->i_nattrs);
	else if (!strcasecmp(token, "item:lifetime"))
		return itos(it->i_lifetime);
	else if (!strcasecmp(token, "item:level"))
		return itos(it->i_level);
	else if (!strcasecmp(token, "item:link"))
		return itos(it->i_link);
	else if (!strcasecmp(token, "item:parent"))
		return itos(it->i_parent);
	else if (!strcasecmp(token, "item:handle"))
		return itos(it->i_handle);
	else if (!strcasecmp(token, "item:rxusage"))
		return it->i_rx_usage == -1 ? "UNK" : itos(it->i_rx_usage);
	else if (!strcasecmp(token, "item:txusage"))
		return it->i_tx_usage == -1 ? "UNK" : itos(it->i_tx_usage);
	else if (!strcasecmp(token, "item:is_child"))
		return itos(!!(it->i_flags & ITEM_FLAG_IS_CHILD));
	else if (!strcasecmp(token, "item:has_childs"))
		return itos(!!(it->i_flags & ITEM_FLAG_HAS_CHILDS));
	else if (!strcasecmp(token, "read:sec"))
		return itos(rtiming.rt_last_read.tv_sec);
	else if (!strcasecmp(token, "read:usec"))
		return itos(rtiming.rt_last_read.tv_usec);
	else if (!strncasecmp(token, "attr:", 5)) {
		int atype;
		char *name = strchr(token+5, ':');
		stat_attr_t *a;
		if (name == NULL) {
			fprintf(stderr, "Invalid attribute field \"%s\"\n",
			    token);
			return "UNK";
		}
		atype = name2type(++name);

		if (atype == INT_MAX || !(a = lookup_attr(it, atype))) {
			fprintf(stderr, "Unknown attribute \"%s\"\n", name);
			return "UNK";
		}

		if (!strncasecmp(token+5, "rx:", 3))
			return itos(attr_get_rx(a));
		else if (!strncasecmp(token+5, "tx:", 3))
			return itos(attr_get_tx(a));
		else if (!strncasecmp(token+5, "rxrate:", 7))
			return itos(attr_get_rx_rate(a));
		else if (!strncasecmp(token+5, "txrate:", 7))
			return itos(attr_get_tx_rate(a));
		else if (!strncasecmp(token+5, "rx64:", 5)) {
			return a->a_rx.r_is64bit ? "1" : "0";
		} else if (!strncasecmp(token+5, "tx64:", 5)) {
			return a->a_tx.r_is64bit ? "1" : "0";
		} else if (!strncasecmp(token+5, "rxoflows:", 9)) {
			return itos(a->a_rx.r_overflows);
		} else if (!strncasecmp(token+5, "txoflows:", 9)) {
			return itos(a->a_tx.r_overflows);
		} else if (!strncasecmp(token+5, "rxenab:", 7))
			return itos(!!(a->a_flags & ATTR_FLAG_RX_ENABLED));
		else if (!strncasecmp(token+5, "txenab:", 7))
			return itos(!!(a->a_flags & ATTR_FLAG_TX_ENABLED));
	}

	fprintf(stderr, "Unknown field \"%s\"\n", token);
	return "UNK";
}

static void format_draw_item(item_t *it, void *arg)
{
	int i;
	node_t *node = (node_t *) arg;

	for (i = 0; i < token_index; i++) {
		if (out_tokens[i].ot_type == OT_STRING) {
			fprintf(c_fd, "%s", out_tokens[i].ot_str);
		} else if (out_tokens[i].ot_type == OT_TOKEN)
			fprintf(c_fd, "%s", get_token(node, it, out_tokens[i].ot_str));
	}
}

static void format_draw_node(node_t *n, void *arg)
{
	foreach_item(n, format_draw_item, n);
}

static void format_draw(void)
{
	foreach_node(format_draw_node, NULL);

	if (c_quit_after > 0)
		if (--c_quit_after == 0)
			exit(0);
}

static inline void add_token(int type, char *data)
{
	if (!out_tokens_size) {
		out_tokens_size = 32;
		out_tokens = calloc(out_tokens_size, sizeof(struct out_token));
		if (out_tokens == NULL)
			quit("Cannot allocate out token array\n");
	}

	if (out_tokens_size <= token_index) {
		out_tokens_size += 32;
		out_tokens = realloc(out_tokens, out_tokens_size * sizeof(struct out_token));
		if (out_tokens == NULL)
			quit("Cannot reallocate out token array\n");
	}
		
		
	out_tokens[token_index].ot_type = type;
	out_tokens[token_index].ot_str = data;
	token_index++;
}

static int format_probe(void)
{
	int new_one = 1;
	char *p, *e;

	for (p = c_format; *p; p++) {
		if (*p == '$') {
			char *s = p;
			s++;
			if (*s == '(') {
				s++;
				if (!*s)
					goto unexpected_end;
				e = strchr(s, ')');
				if (e == NULL)
					goto invalid;

				*p = '\0';
				*e = '\0';
				add_token(OT_TOKEN, s);
				new_one = 1;
				p = e;
				continue;
			}
		}

		if (*p == '\\') {
			char *s = p;
			s++;
			switch (*s) {
				case 'n':
					*s = '\n';
					goto finish_escape;
				case 't':
					*s = '\t';
					goto finish_escape;
				case 'r':
					*s = '\r';
					goto finish_escape;
				case 'v':
					*s = '\v';
					goto finish_escape;
				case 'b':
					*s = '\b';
					goto finish_escape;
				case 'f':
					*s = '\f';
					goto finish_escape;
				case 'a':
					*s = '\a';
					goto finish_escape;
			}

			goto out;
		
finish_escape:
			*p = '\0';
			add_token(OT_STRING, s);
			p = s;
			new_one = 0;
			continue;
		}

out:	
		if (new_one) {
			add_token(OT_STRING, p);
			new_one = 0;
		}
	}

	if (c_debug) {
		int i;
		for (i = 0; i < token_index; i++)
			printf(">>%s<\n", out_tokens[i].ot_str);
	}

	return 1;

unexpected_end:
	fprintf(stderr, "Unexpected end of format string\n");
	return 0;

invalid:
	fprintf(stderr, "Missing ')' in format string\n");
	return 0;
}

static void print_help(void)
{
	printf(
	"format - Formatable Output\n" \
	"\n" \
	"  Formatable ASCII output for scripts. Calls a drawing function for\n" \
	"  every item per node and outputs according to the specified format\n" \
	"  string. The format string consists of normal text and placeholders\n" \
	"  in the form of $(placeholder).\n" \
	"\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    fmt=FORMAT     Format string\n" \
	"    stderr         Write to stderr instead of stdout\n" \
	"    quitafter=NUM  Quit bmon NUM outputs\n" \
	"\n" \
	"  Placeholders:\n" \
	"    read:sec              time of last read (seconds)\n" \
	"    read:usec             time of last read (micro seconds)\n" \
	"    node:index            index of node\n" \
	"    node:name             name of node\n" \
	"    node:from             origin this node came from\n" \
	"    node:nitems           number of items this nodes carries\n" \
	"    node:rx_maj_total     total RX rate of major attribute\n" \
	"    node:tx_maj_total     total TX rate of major attribute\n" \
	"    node:rx_min_total     total RX rate of minor attribute\n" \
	"    node:tx_min_total     total TX rate of minor attribute\n" \
	"    item:name             name of item\n" \
	"    item:desc             description of item\n" \
	"    item:index            index of item\n" \
	"    item:nattrs           number of attributes this item carries\n" \
	"    item:lifetime         lifetime of item\n" \
	"    item:level            indentation level of item\n" \
	"    item:link             item index of root parent\n" \
	"    item:parent           item index of parent\n" \
	"    item:rxusage          RX usage of item (in percent)\n" \
	"    item:txusage          TX usage of item (in percent)\n" \
	"    item:handle           handle of item\n" \
	"    item:is_child         1 if item is a child\n" \
	"    item:has_childs       1 if item has childs\n" \
	"    attr:rx:<name>        rx counter of attribute <name>\n" \
	"    attr:tx:<name>        tx counter of attribute <name>\n" \
	"    attr:rxrate:<name>    rx rate of attribute <name>\n" \
	"    attr:txrate:<name>    tx rate of attribute <name>\n" \
	"    attr:rx64:<name>      1 if rx counter of attribute <name> is 64bit sized\n" \
	"    attr:tx64:<name>      1 if tx counter of attribute <name> is 64bit sized\n" \
	"    attr:rxoflows:<name>  number of rx counter overflows of attribute <name>\n" \
	"    attr:txoflows:<name>  number of tx counter overflows of attribute <name>\n" \
	"    attr:rxenab:<name>    1 if rx counter of attribute <name> is available\n" \
	"    attr:txenab:<name>    1 if rx counter of attribute <name> is available\n" \
	"\n" \
	"  Supported Escape Sequences: \\n, \\t, \\r, \\v, \\b, \\f, \\a\n" \
	"\n" \
	"  Examples:\n" \
	"    \"$(node:name)\\t$(item:name)\\t$(attr:rx:bytes)\\t$(attr:tx:bytes)\\n\"\n" \
	"    axs     lo      12074   12074\n" \
	"\n" \
	"    \"$(item:name) $(attr:rxrate:packets) $(attr:txrate:packets)\\n\"\n" \
	"    eth0 33 5\n" \
	"\n" \
	"    \"Node: $(node:name)\\nItem: $(item:name)\\nBytes Rate: \" \\\n" \
	"        \"$(attr:rxrate:bytes)/$(attr:txrate:bytes)\\nPackets Rate: \" \\\n" \
	"        \"$(attr:rxrate:packets)/$(attr:txrate:packets)\\n\"\n" \
	"    Node: axs\n" \
	"    Item: eth0\n" \
	"    Bytes Rate: 49130/2119\n" \
	"    Packets Rate: 40/11\n" \
	"\n");
}

static void format_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "stderr"))
			c_fd = stderr;
		else if (!strcasecmp(attrs->type, "debug"))
			c_debug = 1;
		else if (!strcasecmp(attrs->type, "fmt")) {
			if (c_format)
				free(c_format);
			c_format = strdup(attrs->value);
		} else if (!strcasecmp(attrs->type, "quitafter") &&
				       attrs->value)
			c_quit_after = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static struct output_module format_ops = {
	.om_name = "format",
	.om_draw = format_draw,
	.om_probe = format_probe,
	.om_set_opts = format_set_opts,
};

static void __init ascii_init(void)
{
	c_fd = stdout;
	c_format = strdup("$(node:name) $(item:name) $(attr:rx:bytes) $(attr:tx:bytes) " \
	    "$(attr:rx:packets) $(attr:tx:packets)\\n");
	register_output_module(&format_ops);
}
