/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#include <string.h>

#include <libubox/utils.h>

#include "timeout.h"

static int usteer_timeout_cmp(const void *k1, const void *k2, void *ptr)
{
	uint32_t ref = (uint32_t) (intptr_t) ptr;
	int32_t t1 = (uint32_t) (intptr_t) k1 - ref;
	int32_t t2 = (uint32_t) (intptr_t) k2 - ref;

	if (t1 < t2)
		return -1;
	else if (t1 > t2)
		return 1;
	else
		return 0;
}

static int32_t usteer_timeout_delta(struct usteer_timeout *t, uint32_t time)
{
	uint32_t val = (uint32_t) (intptr_t) t->node.key;
	return val - time;
}

static void usteer_timeout_recalc(struct usteer_timeout_queue *q, uint32_t time)
{
	struct usteer_timeout *t;
	int32_t delta;

	if (avl_is_empty(&q->tree)) {
		uloop_timeout_cancel(&q->timeout);
		return;
	}

	t = avl_first_element(&q->tree, t, node);

	delta = usteer_timeout_delta(t, time);
	if (delta < 1)
		delta = 1;

	uloop_timeout_set(&q->timeout, delta);
}

static uint32_t ampgr_timeout_current_time(void)
{
	struct timespec ts;
	uint32_t val;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	val = ts.tv_sec * 1000;
	val += ts.tv_nsec / 1000000;

	return val;
}

static void usteer_timeout_cb(struct uloop_timeout *timeout)
{
	struct usteer_timeout_queue *q;
	struct usteer_timeout *t, *tmp;
	bool found;
	uint32_t time;

	q = container_of(timeout, struct usteer_timeout_queue, timeout);
	do {
		found = false;
		time = ampgr_timeout_current_time();

		avl_for_each_element_safe(&q->tree, t, node, tmp) {
			if (usteer_timeout_delta(t, time) > 0)
				break;

			usteer_timeout_cancel(q, t);
			if (q->cb)
				q->cb(q, t);
			found = true;
		}
	} while (found);

	usteer_timeout_recalc(q, time);
}


void usteer_timeout_init(struct usteer_timeout_queue *q)
{
	avl_init(&q->tree, usteer_timeout_cmp, true, NULL);
	q->timeout.cb = usteer_timeout_cb;
}

static void __usteer_timeout_cancel(struct usteer_timeout_queue *q,
				   struct usteer_timeout *t)
{
	avl_delete(&q->tree, &t->node);
}

void usteer_timeout_set(struct usteer_timeout_queue *q, struct usteer_timeout *t,
		       int msecs)
{
	uint32_t time = ampgr_timeout_current_time();
	uint32_t val = time + msecs;
	bool recalc = false;

	q->tree.cmp_ptr = (void *) (intptr_t) time;
	if (usteer_timeout_isset(t)) {
		if (avl_is_first(&q->tree, &t->node))
			recalc = true;

		__usteer_timeout_cancel(q, t);
	}

	t->node.key = (void *) (intptr_t) val;
	avl_insert(&q->tree, &t->node);
	if (avl_is_first(&q->tree, &t->node))
		recalc = true;

	if (recalc)
		usteer_timeout_recalc(q, time);
}

void usteer_timeout_cancel(struct usteer_timeout_queue *q,
			  struct usteer_timeout *t)
{
	if (!usteer_timeout_isset(t))
		return;

	__usteer_timeout_cancel(q, t);
	memset(&t->node.list, 0, sizeof(t->node.list));
}

void usteer_timeout_flush(struct usteer_timeout_queue *q)
{
	struct usteer_timeout *t, *tmp;

	uloop_timeout_cancel(&q->timeout);
	avl_remove_all_elements(&q->tree, t, node, tmp) {
		memset(&t->node.list, 0, sizeof(t->node.list));
		if (q->cb)
			q->cb(q, t);
	}
}
