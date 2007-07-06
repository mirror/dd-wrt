/*
 * netlink-local.h           netlink local header
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef NETLINK_LOCAL_H_
#define NETLINK_LOCAL_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <linux/if_arp.h>
#include <linux/if.h>

#include <linux/pkt_sched.h>
#include <linux/pkt_cls.h>
#include <linux/gen_stats.h>

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/tc.h>

struct trans_tbl {
	int i;
	const char *a;
};

#define __ADD(id, name) { .i = id, .a = #name },

#define NL_DBG(LVL,FMT,ARG...) \
	if (LVL <= nl_debug) \
		fprintf(stderr, "DBG<" #LVL ">: " FMT, ##ARG);

#define BUG()                            \
	do {                                 \
		fprintf(stderr, "BUG: %s:%d\n",  \
			__FILE__, __LINE__);         \
	} while (0);

#define RET_ERR(R, E)                    \
    do {                                 \
		errno = E;                       \
		return -R;                       \
	} while (0);		

extern int __nl_error(int, const char *, unsigned int,
	const char *, const char *, ...);

#ifdef NL_ERROR_ASSERT
#include <assert.h>
static inline int __assert_error(const char *file, int line, char *func,
	const char *fmt, ...)
{
	va_list args;
	fprintf(stderr, "%s:%d:%s: ", file, line, func);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
	assert(0);
	return 0;
}
#define nl_error(E, FMT,ARG...) \
	__assert_error(__FILE__, __LINE__, __FUNCTION__, FMT, ##ARG)
#else
#define nl_error(E, FMT,ARG...) \
	__nl_error(E, __FILE__, __LINE__, __FUNCTION__, FMT, ##ARG);
#endif

static inline char * __type2str_r(int type, char *buf, size_t len,
				  struct trans_tbl *tbl, size_t tbl_len)
{
	int i;
	for (i = 0; i < tbl_len; i++) {
		if (tbl[i].i == type) {
			snprintf(buf, len, "%s", tbl[i].a);
			return buf;
		}
	}

	snprintf(buf, len, "0x%x", type);
	return buf;
}

static inline char * __flags2str_r(int flags, char *buf, size_t len,
				   struct trans_tbl *tbl, size_t tbl_len)
{
	int i;
	int tmp = flags;

	memset(buf, 0, len);
	
	for (i = 0; i < tbl_len; i++) {
		if (tbl[i].i & tmp) {
			tmp &= ~tbl[i].i;
			strncat(buf, tbl[i].a, len - strlen(buf) - 1);
			if ((tmp & flags))
				strncat(buf, ",", len - strlen(buf) - 1);
		}
	}

	return buf;
}


static inline int __str2type(const char *buf, struct trans_tbl *tbl,
			     size_t tbl_len)
{
	int i;
	for (i = 0; i < tbl_len; i++)
		if (!strcasecmp(tbl[i].a, buf))
			return tbl[i].i;

	return -1;
}

static inline void dp_new_line(FILE *fd, struct nl_dump_params *params,
			       int line_nr)
{
	if (params) {
		if (params->dp_prefix) {
			int i;
			for (i = 0; i < params->dp_prefix; i++)
				fprintf(fd, " ");
		}

		if (params->dp_nl_cb)
			params->dp_nl_cb(params, line_nr);
	}
}

static inline void dump_from_ops(struct nl_common *obj, FILE *fd,
				 struct nl_dump_params *params, 
				 struct nl_cache_ops *ops)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (ops->co_dump[type])
		ops->co_dump[type](NULL, obj, fd, params);
}


#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

#define __init __attribute__ ((constructor))
#define __exit __attribute__ ((destructor))

#define P_ACCEPT 0
#define P_IGNORE 0

extern int  tca_msg_parser(struct nlmsghdr *, struct rtnl_tca *);
extern void tca_free_data(struct rtnl_tca *);
extern int  tca_dump_brief(struct nl_cache *, struct rtnl_tca *,
                           const char *, FILE *, struct nl_dump_params *, int);
extern int  tca_dump_full(struct nl_cache *, struct rtnl_tca *, FILE *,
			  struct nl_dump_params *, int);
extern int  tca_dump_with_stats(struct nl_cache *, struct rtnl_tca *, FILE *,
				struct nl_dump_params *, int);
extern int  tca_filter(struct rtnl_tca *, struct rtnl_tca *);

extern void tca_set_ifindex(struct rtnl_tca *, int);
extern int  tca_set_ifindex_name(struct rtnl_tca *, struct nl_cache *,
                                 const char *);
extern void tca_set_handle(struct rtnl_tca *, uint32_t);
extern void tca_set_parent(struct rtnl_tca *, uint32_t);
extern void tca_set_kind(struct rtnl_tca *, const char *);

#endif
