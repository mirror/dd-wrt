/*
 * bmon/node.h		Node Management
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

#ifndef __BMON_NODE_H_
#define __BMON_NODE_H_

#include <bmon/bmon.h>
#include <bmon/item.h>

typedef struct node_s
{
	int		n_index;
	char *		n_name;
	char *		n_from;
	item_t *	n_items;
	size_t		n_nitems;
	int		n_selected;
	b_cnt_t		n_rx_maj_total;
	b_cnt_t		n_tx_maj_total;
	b_cnt_t		n_rx_min_total;
	b_cnt_t		n_tx_min_total;
} node_t;

extern node_t * lookup_node(const char *, int);
extern node_t * get_local_node(void);
extern int get_nnodes(void);
extern void reset_nodes(void);
extern void remove_unused_node_items(void);
extern void foreach_node(void (*cb)(node_t *, void *), void *);
extern void foreach_item(node_t *, void (*cb)(item_t *, void *), void *);
extern void foreach_node_item(void (*cb)(node_t *, item_t *, void *), void *);

extern node_t * get_current_node(void);
extern int first_node(void);
extern int last_node(void);
extern int prev_node(void);
extern int next_node(void);
extern item_t * get_current_item(void);
extern int first_item(void);
extern int last_item(void);
extern int next_item(void);
extern int prev_item(void);
extern int goto_item(int);

extern void calc_node_rates(void);
extern void calc_node_rate(node_t *);

#endif
