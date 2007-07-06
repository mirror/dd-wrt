/*
 * bmon/item.h		Item Management
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

#ifndef __BMON_ITEM_H_
#define __BMON_ITEM_H_

#include <bmon/bmon.h>

#define OVERFLOW_LIMIT  4294967296ULL
#define MAX_GRAPHS 32
#define HISTORY_SIZE 60
#define IFNAME_MAX 32
#define ATTR_HASH_MAX 32
#define UNK_DATA UINT_MAX

typedef struct hist_data_s
{
	b_cnt_t         hd_prev_total;
	rate_cnt_t      hd_data[HISTORY_SIZE];
	unsigned int    hd_overflows;
} hist_data_t;

typedef struct hist_elem_s
{
	hist_data_t     he_rx;
	hist_data_t     he_tx;
	timestamp_t     he_last_update;
	int             he_index;
} hist_elem_t;

typedef struct history_s
{
	hist_elem_t     h_read;
	hist_elem_t     h_sec;
	hist_elem_t     h_min;
	hist_elem_t     h_hour;
	hist_elem_t     h_day;
} history_t;

typedef struct rate_s
{
	b_cnt_t         r_total;
	b_cnt_t         r_prev_total;
	timestamp_t     r_last_update;
	rate_cnt_t      r_tps; 
	int             r_overflows;
	int             r_is64bit;
} rate_t;

#define RX_PROVIDED			0x01
#define TX_PROVIDED			0x02
#define IS64BIT				0x04

#define ATTR_FLAG_RX_ENABLED		0x01
#define ATTR_FLAG_TX_ENABLED		0x02
#define ATTR_FLAG_HISTORY		0x04
#define ATTR_FLAG_IGNORE_OVERFLOWS	0x08
#define ATTR_FLAG_IS_RATE		0x10

struct stat_attr_s;

#define STAT_ATTR_COMMON				\
	uint16_t		a_type;			\
	uint8_t			a_flags;		\
	uint8_t			a_unit;			\
	struct stat_attr_s *	a_next;			\
	timestamp_t		a_last_distribution;	\
	timestamp_t		a_updated;		\
	rate_t			a_rx;			\
	rate_t			a_tx

typedef struct stat_attr_s
{
	STAT_ATTR_COMMON;
} stat_attr_t;

typedef struct stat_attr_hist_s
{
	STAT_ATTR_COMMON;
	history_t	a_hist;
} stat_attr_hist_t;

struct node_s;

typedef struct item_s
{
	char            i_name[IFNAME_MAX];
	char *		i_desc;
	uint32_t        i_handle;
	struct node_s * i_node;
	int             i_index;
	int             i_parent;
	int             i_link;
	uint16_t        i_level;
	uint16_t        i_nattrs; 
	uint16_t        i_lifetime;
	uint16_t	i_attr_sel[MAX_GRAPHS];
	stat_attr_t *   i_attrs[ATTR_HASH_MAX];
	uint8_t		i_unit[MAX_GRAPHS];
	uint16_t	i_graph_sel;
	uint32_t	i_flags;
	int		i_rx_usage;
	int		i_tx_usage;
	int		i_major_attr;
	int		i_minor_attr;

} item_t;

#define ITEM_FLAG_IS_CHILD	0x01
#define ITEM_FLAG_FOLDED	0x02
#define ITEM_FLAG_UPDATED	0x04
#define ITEM_FLAG_HAS_CHILDS	0x08
#define ITEM_FLAG_EXCLUDE	0x10
#define ITEM_FLAG_LOCAL		0x20

enum {
	BYTES,
	PACKETS,
	ERRORS,
	DROP,
	FIFO,
	FRAME,
	COMPRESSED,
	MULTICAST,
	BROADCAST,
	LENGTH_ERRORS,
	OVER_ERRORS,
	CRC_ERRORS,
	MISSED_ERRORS,
	ABORTED_ERRORS,
	CARRIER_ERRORS,
	HEARTBEAT_ERRORS,
	WINDOW_ERRORS,
	COLLISIONS,
	OVERLIMITS,
	BPS,
	PPS,
	QLEN,
	BACKLOG,
	REQUEUES,
	__ATTR_MAX
};
#define ATTR_MAX (__ATTR_MAX - 1)

struct attr_type {
	int id;
	char *name;
	char *desc;
	int flags;
	int unit;
};
	
extern item_t * lookup_item(struct node_s *, const char *, uint32_t, item_t *);
extern void foreach_child(struct node_s *, item_t *,
			  void (*cb)(item_t *, void *), void *);
extern void notify_update(item_t *, timestamp_t *);
extern void increase_lifetime(item_t *, int);
extern void reset_item(item_t *);
extern void remove_unused_items(item_t *);

extern void update_attr(item_t *, int, b_cnt_t, b_cnt_t, int);
extern void item_parse_policy(const char *);
extern void parse_attr_policy(const char *);
extern void foreach_attr(item_t *, void (*cb)(stat_attr_t *, void *), void *);
extern const char * type2name(int);
extern int name2type(const char *);
extern const char * type2desc(int);
extern int attr_ignore_overflows(int);
extern item_t * get_item(struct node_s *, int);
extern int foreach_attr_type(int (*cb)(struct attr_type *, void *), void *);

extern stat_attr_t *lookup_attr(item_t *, int);
extern b_cnt_t attr_get_rx(stat_attr_t *);
extern rate_cnt_t attr_get_rx_rate(stat_attr_t *);
extern b_cnt_t attr_get_tx(stat_attr_t *);
extern rate_cnt_t attr_get_tx_rate(stat_attr_t *);

extern stat_attr_t *current_attr(int);
extern int next_attr(void);
extern int set_graph_unit(int unit);

#endif
