/*
 * itemtab.c		Item Tab file 
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
#include <bmon/conf.h>
#include <bmon/item.h>
#include <bmon/node.h>
#include <bmon/itemtab.h>
#include <bmon/utils.h>

static struct it_item *items;

static inline struct it_item *find_item(const char *name, const char *parent)
{
	struct it_item *i;

	for (i = items; i; i = i->ii_next)
		if (!strcmp(name, i->ii_name) &&
		    !strcmp(parent, i->ii_parent))
		    return i;

	return NULL;
}

static void parse_option(struct it_item *item, char *opt)
{
	char *value = strchr(opt, '=');

	if (value) {
		*value = '\0';
		value++;
	}

	if (!strcasecmp(opt, "rxmax") && value)
		item->ii_rx_max = parse_size(value);
	else if (!strcasecmp(opt, "txmax") && value)
		item->ii_tx_max = parse_size(value);
	else if (!strcasecmp(opt, "max") && value) {
		item->ii_rx_max = parse_size(value);
		item->ii_tx_max = item->ii_rx_max;
	} else if (!strcasecmp(opt, "desc") && value)
		item->ii_desc = strdup(value);
}

static void parse_itemtab(struct it_item *item, char *opts)
{
	char *next, *current = opts;
	
	do {
		next = strchr(current, ',');
		if (next) {
			*next = '\0';
			++next;
		}

		if (*current)
			parse_option(item, current);
		current = next;
	} while (next);
}


static inline void parse_itemtab_line(char *data)
{
	char *buf = strdup(data);
	char *p, *item_name = buf, *parent_name;
	struct it_item *item;

	if (buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1] = '\0';

	/* jump over item */
	for (p = buf; *p != ' ' && *p != '\t' && *p != '\0'; p++);
	if (*p == '\0')
		goto skip;
	*p = '\0';

	/* jump over gap to parent */
	for (++p; *p == ' ' || *p == '\t'; p++);
	if (*p == '\0')
		goto skip;
	parent_name = p;

	/* jump over parent */
	for (; *p != ' ' && *p != '\t' && *p != '\0'; p++);
	if (*p == '\0')
		goto skip;
	*p = '\0';

	/* jump over gap to options */
	for (++p; *p == ' ' || *p == '\t'; p++);
	if (*p == '\0')
		goto skip;

	item = find_item(item_name, parent_name);
	if (item != NULL)
		goto skip;

	item = xcalloc(1, sizeof(*item));
	item->ii_name = strdup(item_name);
	item->ii_parent = strdup(parent_name);
	item->ii_next = items;
	items = item;

	parse_itemtab(item, p);

skip:
	free(buf);
}

static void itemtab_read(const char *path)
{
	FILE *f = fopen(path, "r");
	char *p, buf[1024];
	
	if (f == NULL)
		return;

	while (fgets(buf, sizeof(buf), f)) {
		if ('#' == *buf)
			continue;

		if ((p = strchr(buf, '\r')) || (p = strchr(buf, '\n')))
			*p = '\0';

		if (*buf)
			parse_itemtab_line(buf);
	}

	fclose(f);
}

void read_itemtab(void)
{
	itemtab_read(get_itemtab());
}

struct it_item *lookup_tab(item_t *it)
{
	item_t *parent = NULL;

	if (!(it->i_flags & ITEM_FLAG_LOCAL))
		return NULL;

	if (it->i_flags & ITEM_FLAG_IS_CHILD) {
		parent = get_item(it->i_node, it->i_parent);
		if (parent == NULL)
			return NULL;
	}
	
	return find_item(it->i_name, parent ? parent->i_name : "<none>");
}
