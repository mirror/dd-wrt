/*
 * itemtab.h		Item Tab
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

#ifndef __BMON_ITEMTAB_H_
#define __BMON_ITEMTAB_H_

#include <bmon/bmon.h>
#include <bmon/conf.h>

struct it_item
{
	char *			ii_name;
	char *			ii_parent;
	b_cnt_t			ii_rx_max;
	b_cnt_t			ii_tx_max;
	char *			ii_desc;
	struct it_item *	ii_next;
};

struct it_node
{
	char *			in_name;
	struct it_item *	in_items;
	struct it_node *	in_next;
};
	
extern void read_itemtab(void);
extern struct it_item *lookup_tab(item_t *);

#endif
