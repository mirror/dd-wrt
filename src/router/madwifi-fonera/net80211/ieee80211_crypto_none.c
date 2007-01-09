/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
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
 *
 * $Id: ieee80211_crypto_none.c 1426 2006-02-01 20:07:11Z mrenzmann $
 */

/*
 * IEEE 802.11 NULL crypto support.
 */
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>

static void *none_attach(struct ieee80211vap *, struct ieee80211_key *);
static void none_detach(struct ieee80211_key *);
static int none_setkey(struct ieee80211_key *);
static int none_encap(struct ieee80211_key *, struct sk_buff *, u_int8_t);
static int none_decap(struct ieee80211_key *, struct sk_buff *, int);
static int none_enmic(struct ieee80211_key *, struct sk_buff *, int);
static int none_demic(struct ieee80211_key *, struct sk_buff *, int);

const struct ieee80211_cipher ieee80211_cipher_none = {
	.ic_name	= "NONE",
	.ic_cipher	= IEEE80211_CIPHER_NONE,
	.ic_header	= 0,
	.ic_trailer	= 0,
	.ic_miclen	= 0,
	.ic_attach	= none_attach,
	.ic_detach	= none_detach,
	.ic_setkey	= none_setkey,
	.ic_encap	= none_encap,
	.ic_decap	= none_decap,
	.ic_enmic	= none_enmic,
	.ic_demic	= none_demic,
};
EXPORT_SYMBOL(ieee80211_cipher_none);

static void *
none_attach(struct ieee80211vap *vap, struct ieee80211_key *k)
{
	return vap;		/* for diagnostics+stats */
}

static void
none_detach(struct ieee80211_key *k)
{
	(void) k;
}

static int
none_setkey(struct ieee80211_key *k)
{
	(void) k;
	return 1;
}

static int
none_encap(struct ieee80211_key *k, struct sk_buff *skb, u_int8_t keyid)
{
	struct ieee80211vap *vap = k->wk_private;
#ifdef IEEE80211_DEBUG
	struct ieee80211_frame *wh = (struct ieee80211_frame *)skb->data;
#endif

	/*
	 * The specified key is not setup; this can
	 * happen, at least, when changing keys.
	 */
	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr1,
		"key id %u is not set (encap)", keyid>>6);
	vap->iv_stats.is_tx_badcipher++;
	return 0;
}

static int
none_decap(struct ieee80211_key *k, struct sk_buff *skb, int hdrlen)
{
	struct ieee80211vap *vap = k->wk_private;
#ifdef IEEE80211_DEBUG
	struct ieee80211_frame *wh = (struct ieee80211_frame *)skb->data;
	const u_int8_t *ivp = (const u_int8_t *)&wh[1];
#endif

	/*
	 * The specified key is not setup; this can
	 * happen, at least, when changing keys.
	 */
	/* XXX useful to know dst too */
	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr2,
		"key id %u is not set (decap)", ivp[IEEE80211_WEP_IVLEN] >> 6);
	vap->iv_stats.is_rx_badkeyid++;
	return 0;
}

static int
none_enmic(struct ieee80211_key *k, struct sk_buff *skb, int force)
{
	struct ieee80211vap *vap = k->wk_private;

	vap->iv_stats.is_tx_badcipher++;
	return 0;
}

static int
none_demic(struct ieee80211_key *k, struct sk_buff *skb, int hdrlen)
{
	struct ieee80211vap *vap = k->wk_private;

	vap->iv_stats.is_rx_badkeyid++;
	return 0;
}
