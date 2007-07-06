/*
 * bmon/graph.h            Graph creation utility
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

#ifndef __BMON_GRAPH_H_
#define __BMON_GRAPH_H_

#include <bmon/bmon.h>
#include <bmon/item.h>

#define D_ROW_SIZE (HISTORY_SIZE + 1)
#define D_AT_ROW(P,R) ((P) + ((R) * D_ROW_SIZE))
#define D_AT_COL(P, R) ((P) + (R))

typedef struct table_s {
	int               t_index;
	int               t_height;
	char *            t_data;
	char *            t_x_unit;
	char *            t_y_unit;
	double *          t_y_scale;
} table_t;

#define GRAPH_HAS_FREEABLE_X_UNIT 1

typedef struct graph_s {
	int               g_flags;
	table_t           g_rx;
	table_t           g_tx;
} graph_t;
	
extern void free_graph(graph_t *);
extern graph_t * create_graph(hist_elem_t *, int, int);
extern graph_t * create_configued_graph(history_t *, int, int, int);

extern void new_graph(void);
extern void del_graph(void);
extern int next_graph(void);
extern int prev_graph(void);

#endif
