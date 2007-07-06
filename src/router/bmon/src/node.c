/*
 * node.c            node handling
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
#include <bmon/item.h>
#include <bmon/utils.h>

static node_t *nodes;
static size_t nodes_size;
static size_t nnodes;
static const char * node_name;
static node_t *local_node;
static node_t *current_node;


node_t * lookup_node(const char *name, int creat)
{
	int i;

	if (NULL == nodes) {
		nodes_size = 32;
		nodes = xcalloc(nodes_size, sizeof(node_t));
	}

	for (i = 0; i < nodes_size; i++)
		if (nodes[i].n_name && !strcmp(name, nodes[i].n_name))
			return &nodes[i];

	if (creat) {
		for (i = 0; i < nodes_size; i++)
			if (NULL == nodes[i].n_name)
				break;

		while (i >= nodes_size) {
			int oldsize = nodes_size;
			nodes_size += 32;
			nodes = xrealloc(nodes, nodes_size * sizeof(node_t));
			memset((uint8_t *) nodes + oldsize, 0, (nodes_size - oldsize) * sizeof(node_t));
		}

		nodes[i].n_name = strdup(name);
		nodes[i].n_index = i;
		nnodes++;
		return &nodes[i];
	}

	return NULL;
}

void foreach_node(void (*cb)(node_t *, void *), void *arg)
{
	int i;

	for (i = 0; i < nnodes; i++)
		cb(&nodes[i], arg);
}

void foreach_item(node_t *n, void (*cb)(item_t *, void *), void *arg)
{
	int i;

	for (i = 0; i < n->n_nitems; i++)
		if (n->n_items[i].i_name[0])
			cb(&n->n_items[i], arg);
}


void foreach_node_item(void (*cb)(node_t *, item_t *, void *), void *arg)
{
	int i, m;

	for (i = 0; i < nnodes; i++) {
		node_t *n = &nodes[i];
		
		for (m = 0; m < n->n_nitems; m++)
			if (n->n_items[m].i_name[0])
				cb(n, &n->n_items[m], arg);
	}
}


static void __reset_item(node_t *n, item_t *i, void *arg)
{
	reset_item(i);
}

void reset_nodes(void)
{
	foreach_node_item(__reset_item, NULL);
}

static void __remove_unused_item(node_t *n, item_t *i, void *arg)
{
	remove_unused_items(i);
}

void remove_unused_node_items(void)
{
	foreach_node_item(__remove_unused_item, NULL);
}



node_t * get_local_node(void)
{
	if (NULL == local_node)
		local_node = lookup_node(node_name, 1);

	return local_node;
}

int get_nnodes(void)
{
	return nnodes;
}

static void get_node_info(void)
{
	struct utsname uts;
	
	if (uname(&uts) < 0)
		quit("utsname failed: %s\n", strerror(errno));
	
	node_name = strdup(uts.nodename);
}

void calc_node_rate(node_t *node)
{
	int i;

	node->n_rx_maj_total = 0;
	node->n_tx_maj_total = 0;
	node->n_rx_min_total = 0;
	node->n_tx_min_total = 0;

	for (i = 0; i < node->n_nitems; i++) {
		item_t *it = &node->n_items[i];
		if (it->i_name[0] &&
		    it->i_level == 0 &&
		    !(it->i_flags & ITEM_FLAG_EXCLUDE)) {
			stat_attr_t *maj = lookup_attr(it, it->i_major_attr);
			stat_attr_t *min = lookup_attr(it, it->i_minor_attr);

			if (maj) {
				node->n_rx_maj_total += attr_get_rx_rate(maj);
				node->n_tx_maj_total += attr_get_tx_rate(maj);
			}

			if (min) {
				node->n_rx_min_total += attr_get_rx_rate(min);
				node->n_tx_min_total += attr_get_tx_rate(min);
			}
			
		}
	}
}

void calc_node_rates(void)
{
	int i;

	for (i = 0; i < nnodes; i++)
		calc_node_rate(&nodes[i]);
}

node_t * get_current_node(void)
{
	return current_node;
}

int first_node(void)
{
	int i;

	if (nnodes <= 0)
		return EMPTY_LIST;
	
	for (i = 0; i < nnodes; i++) {
		if (nodes[i].n_name) {
			current_node = &nodes[i];
			return 0;
		}
	}
	
	return EMPTY_LIST;
}

int last_node(void)
{
	int i;
	
	if (nnodes <= 0)
		return EMPTY_LIST;
	
	for (i = (nnodes - 1); i >= 0; i--) {
		if (nodes[i].n_name) {
			current_node = &nodes[i];
			return 0;
		}
	}
	
	return EMPTY_LIST;
}
	

int prev_node(void)
{
	if (nnodes <= 0)
		return EMPTY_LIST;
	
	if (current_node == NULL)
		return first_node();
	else {
		int i;
		for (i = (current_node->n_index - 1); i >= 0; i--) {
			if (nodes[i].n_name) {
				current_node = &nodes[i];
				return 0;
			}
		}
		return END_OF_LIST;
	}
	
	return EMPTY_LIST;
}

int next_node(void)
{
	if (nnodes <= 0)
		return EMPTY_LIST;
	
	if (current_node == NULL)
		return first_node();
	else {
		int i;
		for (i = (current_node->n_index + 1); i < nnodes; i++) {
			if (nodes[i].n_name) {
				current_node = &nodes[i];
				return 0;
			}
		}
		return END_OF_LIST;
	}
	
	return EMPTY_LIST;
}

item_t * get_current_item(void)
{
	if (current_node) {
		if (current_node->n_selected < current_node->n_nitems &&
		    current_node->n_selected >= 0) {
			item_t *i = &(current_node->n_items[current_node->n_selected]);

			if (i->i_name[0])
				return i;
		}
	}
	return NULL;
}

int first_item(void)
{
	int i;
	
	if (current_node == NULL)
		return EMPTY_LIST;
	
	if (current_node->n_nitems <= 0)
		return EMPTY_LIST;
	
	for (i = 0; i < current_node->n_nitems; i++) {
		if (current_node->n_items[i].i_name[0]) {
			current_node->n_selected = i;
			return 0;
		}
	}
	
	return EMPTY_LIST;
}

int last_item(void)
{
	int i;
	
	if (current_node == NULL)
		return EMPTY_LIST;

	if (current_node->n_nitems <= 0)
		return EMPTY_LIST;

	for (i = (current_node->n_nitems - 1); i >= 0; i--) {
		if (current_node->n_items[i].i_name[0]) {
			current_node->n_selected = i;
			return 0;
		}
	}

	return EMPTY_LIST;
}

int goto_item(int num)
{
	if (current_node == NULL)
		return EMPTY_LIST;

	if (current_node->n_nitems <= 0)
		return EMPTY_LIST;

	if (num < current_node->n_nitems && num >= 0)
		if (current_node->n_items[num].i_name[0])
			current_node->n_selected = num;

	return 0;
}

int next_item(void)
{
	if (current_node == NULL)
		return EMPTY_LIST;

	if (current_node->n_nitems <= 0)
		return EMPTY_LIST;

	if (current_node->n_selected < 0 ||
		current_node->n_selected >= current_node->n_nitems)
		return last_item();
	else {
		int i;
		node_t *cn = current_node;

		for (i = (cn->n_selected + 1); i < cn->n_nitems; i++) {
			if (!cn->n_items[i].i_name[0])
				continue;

			if (cn->n_items[i].i_flags & ITEM_FLAG_IS_CHILD) {
				item_t *fi = get_item(cn, cn->n_items[i].i_link);

				if (fi->i_flags & ITEM_FLAG_FOLDED)
					continue;
			}

			cn->n_selected = i;
			return 0;
		}

		return END_OF_LIST;
	}

	return EMPTY_LIST;
}

int prev_item(void)
{
	if (current_node == NULL)
		return EMPTY_LIST;

	if (current_node->n_nitems <= 0)
		return EMPTY_LIST;

	if (current_node->n_selected < 0 ||
		current_node->n_selected >= current_node->n_nitems)
		return first_item();
	else {
		int i;
		node_t *cn = current_node;

		for (i = (cn->n_selected - 1); i >= 0; i--) {
			if (!cn->n_items[i].i_name[0])
				continue;

			if (cn->n_items[i].i_flags & ITEM_FLAG_IS_CHILD) {
				item_t *fi = get_item(cn, cn->n_items[i].i_link);

				if (fi->i_flags & ITEM_FLAG_FOLDED)
					continue;
			}
			
			cn->n_selected = i;
			return 0;
		}

		return END_OF_LIST;
	}

	return EMPTY_LIST;
}

static void __init node_init(void)
{
	get_node_info();
}
