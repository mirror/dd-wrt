/*
 * item.c		Item Management
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
#include <bmon/node.h>
#include <bmon/conf.h>
#include <bmon/item.h>
#include <bmon/itemtab.h>
#include <bmon/input.h>
#include <bmon/graph.h>
#include <bmon/utils.h>

#define MAX_POLICY             255
#define SECOND                 1.0f
#define MINUTE                60.0f
#define HOUR                3600.0f
#define DAY                86400.0f

static char * allowed_items[MAX_POLICY];
static char * denied_items[MAX_POLICY];
static char * allowed_attrs[MAX_POLICY];
static char * denied_attrs[MAX_POLICY];
static int allow_all_attrs;

static inline uint8_t attr_hash(int type)
{
	return (type & 0xFF) % ATTR_HASH_MAX;
}

stat_attr_t *lookup_attr(item_t *it, int type)
{
	stat_attr_t *a;
	uint8_t h = attr_hash(type);
	
	for (a = it->i_attrs[h]; a; a = a->a_next)
		if (a->a_type == type)
			return a;
	return NULL;
}

b_cnt_t attr_get_rx(stat_attr_t *a)
{
#ifndef DISABLE_OVERFLOW_WORKAROUND
	return (a->a_rx.r_overflows * OVERFLOW_LIMIT) + a->a_rx.r_total;
#else
	return a->a_rx.r_total;
#endif
}

rate_cnt_t attr_get_rx_rate(stat_attr_t *a)
{
	return a->a_rx.r_tps;
}

b_cnt_t attr_get_tx(stat_attr_t *a)
{
#ifndef DISABLE_OVERFLOW_WORKAROUND
	return (a->a_tx.r_overflows * OVERFLOW_LIMIT) + a->a_tx.r_total;
#else
	return a->a_tx.r_total;
#endif
}

rate_cnt_t attr_get_tx_rate(stat_attr_t *a)
{
	return a->a_tx.r_tps;
}
void foreach_attr(item_t *i, void (*cb)(stat_attr_t *, void *), void *arg)
{
	int m;

	for (m = 0; m < ATTR_HASH_MAX; m++) {
		stat_attr_t *a;
		for (a = i->i_attrs[m]; a; a = a->a_next)
			cb(a, arg);
	}
}

static struct attr_type attr_types[] = {
#define IGNORE_OVERFLOWS 1
#define IS_RATE 2
#define __A(_I, _N, _D, _U, _F) { .id = _I, .name = _N, .desc = _D, .flags = _F, .unit = _U }
	__A(BYTES, "bytes", "Bytes", U_BYTES, 0),
	__A(PACKETS, "packets", "Packets", U_NUMBER, 0),
	__A(ERRORS, "errors", "Errors", U_NUMBER, 0),
	__A(DROP, "drop", "Dropped", U_NUMBER, 0),
	__A(FIFO, "fifo", "FIFO Err", U_NUMBER, 0),
	__A(FRAME, "frame", "Frame Err", U_NUMBER, 0),
	__A(COMPRESSED, "compressed", "Compressed", U_NUMBER, 0),
	__A(MULTICAST, "multicast", "Multicast", U_NUMBER, 0),
	__A(BROADCAST, "broadcast", "Broadcast", U_NUMBER, 0),
	__A(LENGTH_ERRORS, "len_err", "Length Err", U_NUMBER, 0),
	__A(OVER_ERRORS, "over_err", "Over Err", U_NUMBER, 0),
	__A(CRC_ERRORS, "crc_err", "CRC Err", U_NUMBER, 0),
	__A(MISSED_ERRORS, "miss_err", "Missed Err", U_NUMBER, 0),
	__A(ABORTED_ERRORS, "abort_err", "Aborted Err", U_NUMBER, 0),
	__A(CARRIER_ERRORS, "carrier_err", "Carrier Err", U_NUMBER, 0),
	__A(HEARTBEAT_ERRORS, "hbeat_err", "HBeat Err", U_NUMBER, 0),
	__A(WINDOW_ERRORS, "win_err", "Window Err", U_NUMBER, 0),
	__A(COLLISIONS, "collisions", "Collisions", U_NUMBER, 0),
	__A(OVERLIMITS, "overlimits", "Overlimits", U_NUMBER, 0),
	__A(BPS, "bps", "Bits/s", U_BYTES, IGNORE_OVERFLOWS | IS_RATE),
	__A(PPS, "pps", "Packets/s", U_NUMBER, IGNORE_OVERFLOWS | IS_RATE),
	__A(QLEN, "qlen", "Queue Len", U_NUMBER, IGNORE_OVERFLOWS | IS_RATE),
	__A(BACKLOG, "backlog", "Backlog", U_NUMBER, IGNORE_OVERFLOWS | IS_RATE),
	__A(REQUEUES, "requeues", "Requeues", U_NUMBER, 0)
#undef __A
};

int foreach_attr_type(int (*cb)(struct attr_type *, void *), void *arg)
{
	int i, ret;

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++) {
		ret = cb(&attr_types[i], arg);
		if (ret < 0)
			return ret;
	}

	return 0;
}

const char * type2name(int type)
{
	int i;

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++)
		if (attr_types[i].id == type)
			return attr_types[i].name;

	return NULL;
}

int name2type(const char *name)
{
	int i;

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++)
		if (!strcasecmp(attr_types[i].name, name))
			return attr_types[i].id;

	return INT_MAX;
}

const char * type2desc(int type)
{
	int i;
	static char str[256];

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++)
		if (attr_types[i].id == type)
			return attr_types[i].desc;

	snprintf(str, sizeof(str), "unknown (%d)", type);
	return str;
}

int attr_ignore_overflows(int type)
{
	int i;

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++)
		if (attr_types[i].id == type &&
		    attr_types[i].flags & IGNORE_OVERFLOWS)
			return 1;

	return 0;
}

int attr_is_rate(int type)
{
	int i;

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++)
		if (attr_types[i].id == type &&
		    attr_types[i].flags & IS_RATE)
			return 1;

	return 0;
}

int attr_unit(int type)
{
	int i;

	for (i = 0; i < sizeof(attr_types)/sizeof(attr_types[0]); i++)
		if (attr_types[i].id == type)
			return attr_types[i].unit;

	return 0;
}

static int match_mask(const char *mask, const char *str)
{
	int i, n;
	char c;

	if (!mask || !str)
		return 0;
	
	for (i = 0, n = 0; mask[i] != '\0'; i++, n++) {
		if (mask[i] == '*') {
			c = tolower(mask[i+1]);
			
			if (c == '\0')
				return 1; /* nothing after wildcard, matches in any case */
			
			/*look for c in str, c is the character after the wildcard */
			for (; tolower(str[n]) != c; n++)
				if (str[n] == '\0')
					return 0; /* character after wildcard was not found */
			
			n--;
		} else if (tolower(mask[i]) != tolower(str[n]))
			return 0;
	}

	return str[n] == '\0' ? 1 : 0;
}

static int item_allowed(const char *name)
{
	int n;
	
	if (!allowed_items[0] && !denied_items[0])
		return 1;

	if (!allowed_items[0]) {
		for (n = 0; n < MAX_POLICY && denied_items[n]; n++)
			if (match_mask(denied_items[n], name))
				return 0;
	
		return 1;
	}

	for (n = 0; n < MAX_POLICY && denied_items[n]; n++)
		if (match_mask(denied_items[n], name))
			return 0;
	
	for (n=0; n < MAX_POLICY && allowed_items[n]; n++)
		if (match_mask(allowed_items[n], name))
			return 1;

	return 0;
}

void item_parse_policy(const char *policy)
{
	static int set = 0;
	int i, a = 0, d = 0, f = 0;
	char *p, *s;

	if (set)
		return;
	set = 1;
	
	s = strdup(policy);
	
	for (i = 0, p = s; ; i++) {
		if (s[i] == ',' || s[i] == '\0') {

			f = s[i] == '\0' ? 1 : 0;
			s[i] = '\0';
			
			if ('!' == *p) {
				if (d > (MAX_POLICY - 1))
					break;
				denied_items[d++] = strdup(++p);
			} else {
				if(a > (MAX_POLICY - 1))
					break;
				allowed_items[a++] = strdup(p);
			}
			
			if (f)
				break;
			
			p = &s[i+1];
		}
	}
	
	xfree(s);
}

static inline int collect_history(int type)
{
	int n;
	const char *name = type2name(type);

	if (allow_all_attrs)
		return 1;

	if (!allowed_attrs[0] && !denied_attrs[0]) {
		if (!strcmp(name, "bytes") || !strcmp(name, "packets"))
			return 1;
		else
			return 0;
	}

	if (!allowed_attrs[0]) {
		for (n = 0; n < MAX_POLICY && denied_attrs[n]; n++)
			if (!strcasecmp(denied_attrs[n], name))
				return 0;
		return 1;
	}

	for (n = 0; n < MAX_POLICY && denied_attrs[n]; n++)
		if (!strcasecmp(denied_attrs[n], name))
			return 0;
	
	for (n=0; n < MAX_POLICY && allowed_attrs[n]; n++)
		if (!strcasecmp(allowed_attrs[n], name))
			return 1;

	return 0;
}

void parse_attr_policy(const char *policy)
{
	static int set = 0;
	int i, a = 0, d = 0, f = 0;
	char *p, *s;

	if (set)
		return;
	set = 1;

	if (!strcasecmp(policy, "all")) {
		allow_all_attrs = 1;
		return ;
	}

	s = strdup(policy);
	
	for (i = 0, p = s; ; i++) {
		if (s[i] == ',' || s[i] == '\0') {

			f = s[i] == '\0' ? 1 : 0;
			s[i] = '\0';
			
			if ('!' == *p) {
				if (d > (MAX_POLICY - 1))
					break;
				denied_attrs[d++] = strdup(++p);
			} else {
				if(a > (MAX_POLICY - 1))
					break;
				allowed_attrs[a++] = strdup(p);
			}
			
			if (f)
				break;
			
			p = &s[i+1];
		}
	}
	
	xfree(s);
}

item_t * lookup_item(node_t *node, const char *name, uint32_t handle,
		     item_t *parent)
{
	int i;
	
	if (NULL == node)
		BUG();
	
	if (NULL == node->n_items) {
		node->n_nitems = 32;
		node->n_items = xcalloc(node->n_nitems, sizeof(item_t));
	}
	
	for (i = 0; i < node->n_nitems; i++)
		if (!strcmp(name, node->n_items[i].i_name) &&
			node->n_items[i].i_handle == handle &&
			node->n_items[i].i_parent == (parent ? parent->i_index : 0))
			return !(node->n_items[i].i_flags & ITEM_FLAG_UPDATED) ?
				&node->n_items[i] : NULL;
	
	if (!handle && !item_allowed(name))
		return NULL;
	
	for (i = 0; i < node->n_nitems; i++)
		if (node->n_items[i].i_name[0] == '\0')
			break;
	
	if (i >= node->n_nitems) {
		int oldsize = node->n_nitems;
		node->n_nitems += 32;
		node->n_items = xrealloc(node->n_items, node->n_nitems * sizeof(item_t));
		memset(node->n_items + oldsize, 0, (node->n_nitems - oldsize) * sizeof(item_t));
	}
	
	memset(&node->n_items[i], 0, sizeof(node->n_items[i]));
	strncpy(node->n_items[i].i_name, name, sizeof(node->n_items[i].i_name) - 1);
	node->n_items[i].i_handle = handle;
	node->n_items[i].i_parent = parent ? parent->i_index : 0;
	node->n_items[i].i_index = i;
	node->n_items[i].i_node = node;
	node->n_items[i].i_lifetime = get_lifetime();

	if (!node->n_from)
		node->n_items[i].i_flags |= ITEM_FLAG_LOCAL;

	{
		struct it_item *tab = lookup_tab(&node->n_items[i]);

		if (tab && tab->ii_desc)
			node->n_items[i].i_desc = tab->ii_desc;
	}

	if (parent)
		parent->i_flags |= ITEM_FLAG_HAS_CHILDS;

	return &node->n_items[i];
}

void foreach_child(node_t *node, item_t *parent, void (*cb)(item_t *, void *),
		   void *arg)
{
	int i;

	for (i = 0; i < node->n_nitems; i++)
		if (node->n_items[i].i_parent == parent->i_index &&
		    node->n_items[i].i_flags & ITEM_FLAG_IS_CHILD)
			cb(&node->n_items[i], arg);
}

void reset_item(item_t *i)
{
	i->i_flags &= ~ITEM_FLAG_UPDATED;
}

void remove_unused_items(item_t *i)
{
	if (--(i->i_lifetime) <= 0) {
		int m;
		for (m = 0; m < ATTR_HASH_MAX; m++) {
			stat_attr_t *a, *next;
			for (a = i->i_attrs[m]; a; a = next) {
				next = a->a_next;
				free(a);
			}
		}
		if (!(i->i_flags & ITEM_FLAG_LOCAL)) {
			if (i->i_desc)
				free(i->i_desc);
		}
		memset(i, 0, sizeof(item_t));
	}
}

static void calc_rate(rate_t *rate, timestamp_t *ts, int ignore_overflows)
{
	float diff;

	if (!rate->r_prev_total) {
		rate->r_prev_total = rate->r_total;
		COPY_TS(&rate->r_last_update, ts);
		return;
	}
	
	diff = time_diff(&rate->r_last_update, ts);
	
	if (diff >= get_rate_interval()) {
		if (rate->r_total) {
			rate_cnt_t old_rate;

			b_cnt_t t = (rate->r_total - rate->r_prev_total);
			old_rate = rate->r_tps;

#ifndef DISABLE_OVERFLOW_WORKAROUND
			/* HACK:
			 *
			 * Workaround for counter overflow, all input methods
			 * except kstat in 64bit mode use a 32bit counter which
			 * tends to overflow. We can work around the problem
			 * when assuming that there will be no  scenario with
			 * 4GiB/s and no more than 1 overflow per second.
			 */
			if (t >= OVERFLOW_LIMIT &&
			    !rate->r_is64bit &&
			    !ignore_overflows) {
				rate->r_tps = OVERFLOW_LIMIT -
					(rate->r_prev_total - rate->r_total);
				rate->r_overflows++;
			} else
#endif
				rate->r_tps  = (rate_cnt_t) t;

			rate->r_tps /= diff;
			rate->r_tps = ((rate->r_tps * 3) + old_rate) / 4;
			rate->r_prev_total = rate->r_total;
		}

		COPY_TS(&rate->r_last_update, ts);
	}
}

static inline b_cnt_t get_real_total(rate_t *r, unsigned int prev_overflows,
				     b_cnt_t prev_total)
{
	b_cnt_t res;
	unsigned int new_overflows = (r->r_overflows - prev_overflows);

#ifndef DISABLE_OVERFLOW_WORKAROUND
	if (new_overflows)
		res = (new_overflows * OVERFLOW_LIMIT) + r->r_total - prev_total;
	else
#endif
		res = r->r_total - prev_total;

	return res;
}

static inline void update_history_data(hist_data_t *hd, rate_t *r,
				       int index, double diff)
{
	double t;
	
	t = (double) get_real_total(r, hd->hd_overflows,
	    hd->hd_prev_total) / diff;
	hd->hd_data[index] = (rate_cnt_t) t;
	hd->hd_prev_total = r->r_total;
	hd->hd_overflows = r->r_overflows;
}

static void update_history_element(hist_elem_t *he, rate_t *rx, rate_t *tx,
				   timestamp_t *ts, float unit)
{
	double diff = time_diff(&he->he_last_update, ts);
	
	if (!he->he_last_update.tv_sec)
		diff = 0.0f;

	/* Do not updated histor for graphs with intervals
	 * less than the reading interval. */
	if (get_read_interval() > unit)
		goto discard;

	/* Discard outdated updates */
	if (diff >= (unit + (get_hb_factor() * unit)))
		goto discard;

	if (!he->he_last_update.tv_sec) {
		he->he_rx.hd_prev_total = rx->r_total;
		he->he_tx.hd_prev_total = tx->r_total;
	} else if (get_read_interval() == unit) {
		update_history_data(&he->he_rx, rx, he->he_index, 1.0f);
		update_history_data(&he->he_tx, tx, he->he_index, 1.0f);
	/* The timing code might do shorter intervals than requested to
	 * adjust previous intervals being too long */
	} else if (diff >= ((1.0f - get_hb_factor()) * unit)) {
		update_history_data(&he->he_rx, rx, he->he_index, diff);
		update_history_data(&he->he_tx, tx, he->he_index, diff);
	} else {
		/* Silently discard and give it a chance to come back
		 * in time. */
		return;
	}

	if (he->he_index >= (HISTORY_SIZE - 1))
		he->he_index = 0;
	else
		he->he_index++;

	COPY_TS(&he->he_last_update, ts);

	return;
discard:
	while(diff > 0.0f) {
		he->he_rx.hd_data[he->he_index] = UNK_DATA;
		he->he_tx.hd_data[he->he_index] = UNK_DATA;

		if (he->he_index >= (HISTORY_SIZE - 1))
			he->he_index = 0;
		else
			he->he_index++;

		diff -= unit;
	}

	he->he_rx.hd_prev_total = rx->r_total;
	he->he_tx.hd_prev_total = tx->r_total;

	he->he_rx.hd_overflows = rx->r_overflows;
	he->he_tx.hd_overflows = tx->r_overflows;
	COPY_TS(&he->he_last_update, ts);

}

static inline void update_history(history_t *hist, rate_t *rx, rate_t *tx,
				  timestamp_t *ts)
{
	if (get_read_interval() != 1.0f)
		update_history_element(&hist->h_read, rx, tx, ts,
		    get_read_interval());
	update_history_element(&hist->h_sec, rx, tx, ts, SECOND);
	update_history_element(&hist->h_min, rx, tx, ts, MINUTE);
	update_history_element(&hist->h_hour, rx, tx, ts, HOUR);
	update_history_element(&hist->h_day, rx, tx, ts, DAY);
}

static void update_attr_hist(stat_attr_t *a, void *arg)
{
	timestamp_t *ts = (timestamp_t *) arg;
	int ign_of = a->a_flags & ATTR_FLAG_IGNORE_OVERFLOWS;

	if (ts == NULL)
		ts = &rtiming.rt_last_read;

	if (a->a_flags & ATTR_FLAG_IS_RATE) {
		a->a_rx.r_tps = a->a_rx.r_total;
		a->a_tx.r_tps = a->a_tx.r_total;
	} else {
		calc_rate(&a->a_rx, ts, ign_of);
		calc_rate(&a->a_tx, ts, ign_of);
	}

	if (a->a_flags & ATTR_FLAG_HISTORY) {
		stat_attr_hist_t *ah = (stat_attr_hist_t *) a;
		update_history(&ah->a_hist, &ah->a_rx, &ah->a_tx, ts);
	}
}

void notify_update(item_t *i, timestamp_t *ts)
{
	struct it_item *tab;
	stat_attr_t *a;

	i->i_flags |= ITEM_FLAG_UPDATED;
	foreach_attr(i, &update_attr_hist, ts);
	
	a = lookup_attr(i, i->i_major_attr);
	tab = lookup_tab(i);

	if (tab && a) {
		rate_cnt_t rx = attr_get_rx_rate(a);
		rate_cnt_t tx = attr_get_tx_rate(a);

		if (tab->ii_rx_max) {
			if (rx)
				i->i_rx_usage = (100.0f / (tab->ii_rx_max / rx));
			else
				i->i_rx_usage = 0;
		} else
			i->i_rx_usage = -1;

		if (tab->ii_tx_max) {
			if (tx)
				i->i_tx_usage = (100.0f / (tab->ii_tx_max / tx));
			else
				i->i_tx_usage = 0;
		} else
			i->i_tx_usage = -1;
	} else if (!i->i_node || !i->i_node->n_from) {
		i->i_rx_usage = -1;
		i->i_tx_usage = -1;
	}
}

void increase_lifetime(item_t *i, int l)
{
	i->i_lifetime = l*get_lifetime();
}

void update_attr(item_t *i, int type, b_cnt_t rx, b_cnt_t tx, int flags)
{
	stat_attr_t *a;
	uint8_t h = attr_hash(type);
	
	for (a = i->i_attrs[h]; a; a = a->a_next)
		if (a->a_type == type)
			goto found;
	
	if (collect_history(type)) {
		a = xcalloc(1, sizeof(stat_attr_hist_t));
		a->a_flags |= ATTR_FLAG_HISTORY;
	} else
		a = xcalloc(1, sizeof(stat_attr_t));

	a->a_type = type;
	a->a_unit = attr_unit(a->a_type);
	if (attr_ignore_overflows(a->a_type))
		a->a_flags |= ATTR_FLAG_IGNORE_OVERFLOWS;
	if (attr_is_rate(a->a_type))
		a->a_flags |= ATTR_FLAG_IS_RATE;

	a->a_next = i->i_attrs[h];
	i->i_attrs[h] = a;
	i->i_nattrs++;

found:
	if (flags & IS64BIT)
		a->a_rx.r_is64bit = a->a_tx.r_is64bit = 1;

	if (flags & RX_PROVIDED) {
		a->a_rx.r_total = rx;
		a->a_flags |= ATTR_FLAG_RX_ENABLED;
		update_ts(&a->a_updated); /* XXX: use read ts */
	}

	if (flags & TX_PROVIDED) {
		a->a_tx.r_total = tx;
		a->a_flags |= ATTR_FLAG_TX_ENABLED;
		update_ts(&a->a_updated);
	}
}


item_t * get_item(node_t *node, int index)
{
	int i;

	for (i = 0; i < node->n_nitems; i++)
		if (node->n_items[i].i_index == index)
			return &node->n_items[i];
	return NULL;
}

stat_attr_t *current_attr(int graph)
{
	int i, c;
	item_t *it = get_current_item();
	stat_attr_t *a;

	if (it == NULL)
		return NULL;

	for (i = 0, c = 0; i < ATTR_HASH_MAX; i++)
		for (a = it->i_attrs[i]; a; a = a->a_next, c++)
			if (c == it->i_attr_sel[graph])
				return a;

	return NULL;
}

int first_attr(void)
{
	int i, c;
	stat_attr_t *a;
	item_t *it = get_current_item();
	if (it == NULL)
		return EMPTY_LIST;

	for (i = 0, c = 0; i < ATTR_HASH_MAX; i++) {
		for (a = it->i_attrs[i]; a; a = a->a_next) {
			if (a->a_flags & ATTR_FLAG_HISTORY) {
				it->i_attr_sel[it->i_graph_sel] = c;
				return 0;
			} else
				c++;
		}
	}

	return EMPTY_LIST;
}

int next_attr(void)
{
	item_t *it = get_current_item();
	if (it == NULL)
		return EMPTY_LIST;

	if (it->i_attr_sel[it->i_graph_sel] >= (it->i_nattrs - 1))
		return first_attr();
	else {
		stat_attr_t *a = current_attr(it->i_graph_sel);
		int i, n = it->i_attr_sel[it->i_graph_sel] + 1;

		if (a) {
			stat_attr_t *b = a->a_next;
			i = attr_hash(a->a_type);
			if (!b)
				i++;
			for (; i < ATTR_HASH_MAX; i++) {
				for (a = b ? : it->i_attrs[i]; a; a = a->a_next) {
					if (!(a->a_flags & ATTR_FLAG_HISTORY)) {
						n++;
						continue;
					}
					b = NULL;
					it->i_attr_sel[it->i_graph_sel] = n;
					return 0;
				}
			}
		}
	
		return first_attr();
	}

	return 0;
}


int set_graph_unit(int unit)
{
	item_t *it = get_current_item();
	if (it == NULL)
		return EMPTY_LIST;

	it->i_unit[it->i_graph_sel] = unit;

	return 0;
}
