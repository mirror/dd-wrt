/*
 * u32.c              rtnetlink u32 classifier
 *
 * $Id: u32.c 119 2005-01-21 20:29:58Z tgr $
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


#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/tca.h>
#include <netlink/filter.h>
#include <netlink/u32.h>
#include <netlink/helpers.h>
#include <netlink/route/rtattr.h>

/**************************************************************************
 ** 
 ** FILTER OPS
 **
 **************************************************************************/

static int u32_msg_parser(struct rtnl_filter *f)
{
	int err;
	struct rtattr *tb[TCA_U32_MAX + 1] = {0};
	struct rtnl_u32 *u = NULL;

	memset(tb, 0, (TCA_U32_MAX + 1) * sizeof(struct rtattr *));
	
	if ((err = nl_parse_rtattr(tb, TCA_U32_MAX, (void *) f->tc_opts.d_data,
		f->tc_opts.d_size)) < 0)
			return err;

	u = f->tc_subdata = calloc(1, sizeof(*u));

	if (tb[TCA_U32_DIVISOR]) {
		if ((err = NL_COPY_DATA(u->divisor, tb[TCA_U32_DIVISOR])) < 0)
			goto err_out;
		u->mask |= U32_HAS_DIVISOR;
	}

	if (tb[TCA_U32_SEL]) {
		if ((err = nl_alloc_data_from_rtattr(&u->selector, tb[TCA_U32_SEL])) < 0)
			goto err_out;
		u->mask |= U32_HAS_SELECTOR;
	}

	if (tb[TCA_U32_HASH]) {
		if ((err = NL_COPY_DATA(u->hash, tb[TCA_U32_HASH])) < 0)
			goto err_out;
		u->mask |= U32_HAS_HASH;
	}

	if (tb[TCA_U32_CLASSID]) {
		if ((err = NL_COPY_DATA(u->classid, tb[TCA_U32_CLASSID])) < 0)
			goto err_out;
		u->mask |= U32_HAS_CLASSID;
	}

	if (tb[TCA_U32_LINK]) {
		if ((err = NL_COPY_DATA(u->link, tb[TCA_U32_LINK])) < 0)
			goto err_out;
		u->mask |= U32_HAS_LINK;
	}

	if (tb[TCA_U32_ACT]) {
		if ((err = nl_alloc_data_from_rtattr(&u->act, tb[TCA_U32_ACT])) < 0)
			goto err_out;
		u->mask |= U32_HAS_ACTION;
	}

	if (tb[TCA_U32_POLICE]) {
		if ((err = nl_alloc_data_from_rtattr(&u->police, tb[TCA_U32_POLICE])) < 0)
			goto err_out;
		u->mask |= U32_HAS_POLICE;
	}

	if (tb[TCA_U32_PCNT] && tb[TCA_U32_SEL]) {
		struct tc_u32_sel *s = u->selector.d_data;
		if (RTA_PAYLOAD(tb[TCA_U32_PCNT]) < (sizeof(struct tc_u32_pcnt) +
			(s->nkeys * sizeof(uint64_t)))) {
				err = -1;
				goto err_out;
		}
		
		if ((err = nl_alloc_data_from_rtattr(&u->pcnt, tb[TCA_U32_PCNT])) < 0)
			goto err_out;
		u->mask |= U32_HAS_PCNT;
	}

	if (tb[TCA_U32_INDEV]) {
		err = nl_copy_data(u->indev, sizeof(u->indev), tb[TCA_U32_INDEV]);

		if (err < 0)
			goto err_out;
		u->mask |= U32_HAS_INDEV;
	}

	return 0;

err_out:
	if (u) {
		nl_free_data(&u->selector);
		nl_free_data(&u->act);
		nl_free_data(&u->police);
		nl_free_data(&u->pcnt);
		free(u);
	}
	
	return err;
}



static void u32_free_data(struct rtnl_filter *f)
{
	struct rtnl_u32 *u = f->tc_subdata;

	if (NULL == u)
		return;

	nl_free_data(&u->selector);
	nl_free_data(&u->act);
	nl_free_data(&u->police);
	nl_free_data(&u->pcnt);

	free(f->tc_subdata);
	f->tc_subdata = NULL;
}

static void u32_dump_brief(struct nl_cache *c, struct rtnl_filter *f, FILE *fd)
{
	struct rtnl_u32 *u = f->tc_subdata;
	char h[32];

	if (u->mask & U32_HAS_DIVISOR)
		fprintf(fd, " divisor %u", u->divisor);
	else  if (u->mask & U32_HAS_CLASSID)
		fprintf(fd, " target %s",
			nl_handle2str_r(u->classid, h, sizeof(h)));

}

static void print_selector(FILE *fd, struct tc_u32_sel *sel,
			   struct rtnl_filter *f, struct rtnl_u32 *u)
{
	int i;
	struct tc_u32_key *key;

	if (sel->hmask || sel->hoff) {
		/* I guess this will never be used since the kernel only
		 * exports the selector if no divisor is set but hash offset
		 * and hash mask make only sense in hash filters with divisor
		 * set */
		fprintf(fd, " hash at %u & 0x%x", sel->hoff, sel->hmask);
	}

	if (sel->flags & (TC_U32_OFFSET | TC_U32_VAROFFSET)) {
		fprintf(fd, " offset at %u", sel->off);

		if (sel->flags & TC_U32_VAROFFSET)
			fprintf(fd, " variable (at %u & 0x%x) >> %u",
				sel->offoff, ntohs(sel->offmask), sel->offshift);
	}

	if (sel->flags) {
		int flags = sel->flags;
		fprintf(fd, " <");

#define PRINT_FLAG(f) if (flags & TC_U32_##f) { \
	flags &= ~TC_U32_##f; fprintf(fd, #f "%s", flags ? "," : ""); }

		PRINT_FLAG(TERMINAL);
		PRINT_FLAG(OFFSET);
		PRINT_FLAG(VAROFFSET);
		PRINT_FLAG(EAT);
#undef PRINT_FLAG

		fprintf(fd, ">");
	}
		
	
	for (i = 0; i < sel->nkeys; i++) {
		key = (struct tc_u32_key *) ((char *) sel + sizeof(*sel));

		fprintf(fd, "\n%s        match key at %s%u ",
		nl_get_dump_prefix(), key->offmask ? "nexthdr+" : "", key->off);

		if (key->offmask)
			fprintf(fd, "[0x%u] ", key->offmask);

		fprintf(fd, "& 0x%08x == 0x%08x", ntohl(key->mask), ntohl(key->val));

		if (f->with_stats && u->mask & U32_HAS_PCNT) {
			struct tc_u32_pcnt *p = u->pcnt.d_data;
			fprintf(fd, " successful %llu", p->kcnts[i]);
		}
	}
}


static void u32_dump_full(struct nl_cache *c, struct rtnl_filter *f, FILE *fd)
{
	struct rtnl_u32 *u = f->tc_subdata;
	struct tc_u32_sel *s;

	if (!(u->mask & U32_HAS_SELECTOR))
		return;
	
	s = u->selector.d_data;

	fprintf(fd, "%s    nkeys %u ", nl_get_dump_prefix(), s->nkeys);

	if (u->mask & U32_HAS_HASH)
		fprintf(fd, "ht key 0x%x hash 0x%u",
			TC_U32_USERHTID(u->hash), TC_U32_HASH(u->hash));

	if (u->mask & U32_HAS_LINK)
		fprintf(fd, "link %u ", u->link);

	if (u->mask & U32_HAS_INDEV)
		fprintf(fd, "indev %s ", u->indev);

	print_selector(fd, s, f, u);
	fprintf(fd, "\n");

#if 0	
#define U32_HAS_ACTION       0x040
#define U32_HAS_POLICE       0x080

	struct nl_data   act;
	struct nl_data   police;
#endif
}

static void u32_dump_with_stats(struct nl_cache *c, struct rtnl_filter *f,
				FILE *fd)
{
	struct rtnl_u32 *u = f->tc_subdata;

	if (u->mask & U32_HAS_PCNT) {
		struct tc_u32_pcnt *pc = u->pcnt.d_data;
		fprintf(fd, "\n%s         successful       hits",
			nl_get_dump_prefix());
		fprintf(fd, "\n%s           %8llu   %8llu\n",
			nl_get_dump_prefix(), pc->rhit, pc->rcnt);
	}
}

/**************************************************************************
 ** 
 ** MODULE HOOKUP
 **
 **************************************************************************/

static struct rtnl_filter_ops u32_ops = {
	.kind			= "u32",
	.msg_parser		= &u32_msg_parser,
	.free_data		= &u32_free_data,
	.dump[NL_DUMP_BRIEF]	= &u32_dump_brief,
	.dump[NL_DUMP_FULL]	= &u32_dump_full,
	.dump[NL_DUMP_STATS]	= &u32_dump_with_stats,
};

void __init u32_init(void)
{
	rtnl_filter_register(&u32_ops);
}

void __exit u32_exit(void)
{
	rtnl_filter_unregister(&u32_ops);
}
