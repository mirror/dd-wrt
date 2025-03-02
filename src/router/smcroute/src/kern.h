/* Kernel API for join/leave multicast groups and add/del routes
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SMCROUTE_KERN_H_
#define SMCROUTE_KERN_H_

#include "mcgroup.h"
#include "mroute.h"

struct mroute_stats {
	unsigned long ms_pktcnt;
	unsigned long ms_bytecnt;
	unsigned long ms_wrong_if;
};

int kern_join_leave  (int sd, int cmd, struct mcgroup *mcg);

int kern_mroute_init (int table_id, void (*cb)(int, void *), void *arg);
int kern_mroute_exit (void);

int kern_mroute6_init(int table_id, void (*cb)(int, void *), void *arg);
int kern_mroute6_exit(void);

int kern_vif_add     (struct iface *iface);
int kern_vif_del     (struct iface *iface);

int kern_mif_add     (struct iface *iface);
int kern_mif_del     (struct iface *iface);

int kern_mroute_add  (struct mroute *route);
int kern_mroute_del  (struct mroute *route);

int kern_stats       (struct mroute *route, struct mroute_stats *ms);

#endif /* SMCROUTE_KERN_H_ */
