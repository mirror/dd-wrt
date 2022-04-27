/*-
 * Copyright (c) 2007 Pavel Roskin
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * Atheros module glue for rate control algorithms.
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif

#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <net80211/if_media.h>
#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_rate.h>

static const char *module_names[] = {
	[IEEE80211_RATE_AMRR] = "ath_rate_amrr",
	[IEEE80211_RATE_MINSTREL] = "ath_rate_minstrel",
	[IEEE80211_RATE_ONOE] = "ath_rate_onoe",
	[IEEE80211_RATE_SAMPLE] = "ath_rate_sample"
};

/*
 * Table of registered rate controllers.
 */
static struct ieee80211_rate_ops ratectls[IEEE80211_RATE_MAX];

int ieee80211_rate_register(struct ieee80211_rate_ops *ops)
{
	int id = ops->ratectl_id;

	if ((0 <= id) && (id < IEEE80211_RATE_MAX)) {
		memcpy(&ratectls[id], ops, sizeof(*ops));
		return 0;
	}
	return -EINVAL;
}

EXPORT_SYMBOL(ieee80211_rate_register);

void ieee80211_rate_unregister(struct ieee80211_rate_ops *ops)
{
	int id = ops->ratectl_id;

	if ((0 <= id) && (id < IEEE80211_RATE_MAX))
		memset(&ratectls[id], 0, sizeof(ratectls[0]));
}

EXPORT_SYMBOL(ieee80211_rate_unregister);

struct ath_ratectrl *ieee80211_rate_attach(struct ath_softc *sc, const char *name)
{
	int id;
	char buf[64];
	struct ath_ratectrl *ctl;

	snprintf(buf, sizeof(buf), "ath_rate_%s", name);
	for (id = 0; id < IEEE80211_RATE_MAX; id++) {
		if (strcmp(buf, module_names[id]) == 0)
			break;
	}

	if (id >= IEEE80211_RATE_MAX) {
		printk(KERN_ERR "Module \"%s\" is not known\n", buf);
		return NULL;
	}

	if (!ratectls[id].attach)
		ieee80211_load_module(buf);

	if (!ratectls[id].attach) {
		/* pick the first available rate control module */
		printk(KERN_INFO "Rate control module \"%s\" not available\n", buf);
		for (id = 0; id < IEEE80211_RATE_MAX - 1; id++) {
			if (ratectls[id].attach)
				break;
		}
		if (!ratectls[id].attach) {
			printk(KERN_ERR "No rate control module available");
			return NULL;
		} else {
			printk(KERN_INFO "Using \"%s\" instead.\n", module_names[id]);
		}
	}

	ctl = ratectls[id].attach(sc);
	if (!ctl) {
		printk(KERN_ERR "Module \"%s\" failed to initialize\n", buf);
		return NULL;
	}

	ctl->ops = &ratectls[id];
	return ctl;
}

EXPORT_SYMBOL(ieee80211_rate_attach);

void ieee80211_rate_detach(struct ath_ratectrl *ctl)
{
	ctl->ops->detach(ctl);
}

EXPORT_SYMBOL(ieee80211_rate_detach);
