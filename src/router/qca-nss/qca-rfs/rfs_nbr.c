/*
 * Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * rfs_nbr.c
 *	Receiving Flow Streering - Neighbor Manager
 */

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <linux/jhash.h>
#include <linux/proc_fs.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netevent.h>
#include <net/route.h>

#include "rfs.h"
#include "rfs_nbr.h"
#include "rfs_rule.h"


/*
 * Per-module structure.
 */
struct rfs_nbr {
	int is_running;
};


static struct rfs_nbr __nbr;

/*
 * nbr_netevent_callback
 */
static int nbr_netevent_callback(struct notifier_block *notifier, unsigned long event, void *ctx)
{
	struct neigh_table *tbl;
	struct neighbour *neigh;
	int family;
	int key_len;

	if (event != NETEVENT_NEIGH_UPDATE)
		return NOTIFY_DONE;

	neigh = ctx;
	tbl   = neigh->tbl;
	key_len = tbl->key_len;
	family  = tbl->family;

	if (family != AF_INET && family != AF_INET6)
		return NOTIFY_DONE;

	if (neigh->nud_state & NUD_VALID) {
		rfs_rule_create_ip_rule(family, neigh->primary_key, neigh->ha, RPS_NO_CPU, 0);

	} else {
		rfs_rule_destroy_ip_rule(family, neigh->primary_key, 0);
	}

	return NOTIFY_DONE;
}

static struct notifier_block nbr_notifier = {
	.notifier_call = nbr_netevent_callback,
};


/*
 * rfs_nbr_start
 */
int rfs_nbr_start(void)
{
	struct rfs_nbr *nbr = &__nbr;

	if (nbr->is_running)
		return 0;

	RFS_DEBUG("RFS nbr start\n");
	register_netevent_notifier(&nbr_notifier);
	nbr->is_running = 1;
	return 0;
}


/*
 * rfs_nbr_stop
 */
int rfs_nbr_stop(void)
{
	struct rfs_nbr *nbr = &__nbr;

	if (!nbr->is_running)
		return 0;

	RFS_DEBUG("RFS nbr stop\n");
	unregister_netevent_notifier(&nbr_notifier);
	nbr->is_running = 0;
	return 0;
}


/*
 * rfs_nbr_init
 */
int rfs_nbr_init(void)
{
	struct rfs_nbr *nbr = &__nbr;

	RFS_DEBUG("RFS nbr init\n");
	nbr->is_running = 0;
	return 0;
}


/*
 * rfs_nbr_exit
 */
void rfs_nbr_exit(void)
{
	RFS_DEBUG("RFS nbr exit\n");
	rfs_nbr_stop();
}

