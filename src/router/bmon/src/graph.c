/*
 * graph.c             Graph creation utility
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
#include <bmon/graph.h>
#include <bmon/input.h>
#include <bmon/conf.h>
#include <bmon/item.h>
#include <bmon/node.h>
#include <bmon/conf.h>
#include <bmon/utils.h>


static void put_col(table_t *t, int data_idx, hist_data_t *src, int hist_idx,
		    double half_step)
{
	int i;
	char *col = D_AT_COL(t->t_data, data_idx);

	rate_cnt_t tot = src->hd_data[hist_idx];

	if (tot == UNK_DATA) {
		for (i = 0; i < t->t_height; i++)
			*(D_AT_ROW(col, i)) = get_unk_char();
	} else if (tot) {
		*(D_AT_ROW(col, 0)) = ':';
		
		for (i = 0; i < t->t_height; i++)
			if (tot >= (t->t_y_scale[i] - half_step))
				*(D_AT_ROW(col, i)) = get_fg_char();
	}
}

static void create_table(table_t *t, hist_data_t *src, int index, int height,
			 int unit)
{
	int i, di;
	size_t dsize = height * (HISTORY_SIZE + 1);
	rate_cnt_t max = 0;
	double half_step, step;
	
	t->t_index = index;
	t->t_height = height;
	t->t_y_scale = xcalloc(height, sizeof(double));
	t->t_data = xcalloc(dsize, sizeof(char));

	memset(t->t_data, get_bg_char(), dsize);

	for (i = 0; i < height; i++)
		*(D_AT_COL(D_AT_ROW(t->t_data, i), HISTORY_SIZE)) = '\0';

	for (i = 0; i < HISTORY_SIZE; i++)
		if (max < src->hd_data[i] && src->hd_data[i] != UNK_DATA)
			max = src->hd_data[i];

	step = (double) max / (double) height;
	half_step = step / 2.0f;

	for (i = 0; i < height; i++)
		t->t_y_scale[i] = (double) (i + 1) * step;

	for (di = 0, i = (index - 1); i >= 0; di++, i--)
		put_col(t, di, src, i, half_step);

	for (i = (HISTORY_SIZE - 1); di < HISTORY_SIZE; di++, i--)
		put_col(t, di, src, i, half_step);

	{
		b_cnt_t div;
		int h = (height / 3) * 2;

		if (h >= height)
			h = (height - 1);

		div = get_divisor(t->t_y_scale[h], unit, &t->t_y_unit, NULL);
		
		for (i = 0; i < height; i++)
			t->t_y_scale[i] /= (double) div;
	}
}

graph_t * create_graph(hist_elem_t *src, int height, int unit)
{
	graph_t *g;

	g = xcalloc(1, sizeof(graph_t));

	create_table(&g->g_rx, &src->he_rx, src->he_index, height, unit);
	create_table(&g->g_tx, &src->he_tx, src->he_index, height, unit);

	return g;
}

graph_t * create_configued_graph(history_t *src, int height, int unit,
				 int x_unit)
{
	graph_t *g;
	hist_elem_t *e = NULL;
	char *u = "s";
	int h = 0;

	switch (x_unit) {
		case X_SEC:  u = "s"; e = &src->h_sec; break;
		case X_MIN:  u = "m"; e = &src->h_min; break;
		case X_HOUR: u = "h"; e = &src->h_hour; break;
		case X_DAY:  u = "d"; e = &src->h_day; break;
		case X_READ: {
			if (get_read_interval() != 1.0f) {
				char buf[32];
				float ri = get_read_interval();

				snprintf(buf, sizeof(buf), "(%.2fs)", ri);
				u = strdup(buf);
				h = 1;
				e = &src->h_read;
			} else {
				u = "s";
				e = &src->h_sec;
			}
		}
		break;
	}

	if (NULL == e)
		BUG();

	g = create_graph(e, height, unit);

	if (h)
		g->g_flags |= GRAPH_HAS_FREEABLE_X_UNIT;

	g->g_rx.t_x_unit = u;
	g->g_tx.t_x_unit = u;

	return g;
}

static void free_table(table_t *t)
{
	xfree(t->t_y_scale);
	xfree(t->t_data);
}

void free_graph(graph_t *g)
{
	if (g->g_flags & GRAPH_HAS_FREEABLE_X_UNIT)
		xfree(g->g_rx.t_x_unit);
		
	free_table(&g->g_rx);
	free_table(&g->g_tx);
	xfree(g);
}

void new_graph(void)
{
	if (get_ngraphs() >= (MAX_GRAPHS - 1))
		return;
	set_ngraphs(get_ngraphs() + 1);
}

void del_graph(void)
{
	if (get_ngraphs() <= 1)
		return;
	set_ngraphs(get_ngraphs() - 1);
}

int next_graph(void)
{
	item_t *it = get_current_item();
	if (it == NULL)
		return EMPTY_LIST;

	if (it->i_graph_sel >= (get_ngraphs() - 1))
		it->i_graph_sel = 0;
	else
		it->i_graph_sel++;

	return 0;
}
	
int prev_graph(void)
{
	item_t *it = get_current_item();
	if (it == NULL)
		return EMPTY_LIST;

	if (it->i_graph_sel <= 0)
		it->i_graph_sel = get_ngraphs() - 1;
	else
		it->i_graph_sel--;

	return 0;
}
