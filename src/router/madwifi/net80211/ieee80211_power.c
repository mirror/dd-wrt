/*-
 * Copyright (c) 2001 Atsushi Onoe
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
 * $Id: ieee80211_power.c 1520 2006-04-21 16:57:59Z dyqith $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 power save support.
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_proto.h>

static void ieee80211_set_tim(struct ieee80211_node *ni, int set);

void
ieee80211_power_attach(struct ieee80211com *ic)
{
}

void
ieee80211_power_detach(struct ieee80211com *ic)
{
}

void
ieee80211_power_vattach(struct ieee80211vap *vap)
{
	if (vap->iv_opmode == IEEE80211_M_HOSTAP ||
	    vap->iv_opmode == IEEE80211_M_IBSS) {
		/* NB: driver should override */
		vap->iv_set_tim = ieee80211_set_tim;
	}
}

void
ieee80211_power_latevattach(struct ieee80211vap *vap)
{
	/*
	 * Allocate these only if needed.  Beware that we
	 * know adhoc mode doesn't support ATIM yet...
	 */
	if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
		vap->iv_tim_len = howmany(vap->iv_max_aid,8) * sizeof(u_int8_t);
		MALLOC(vap->iv_tim_bitmap, u_int8_t *, vap->iv_tim_len,
			M_DEVBUF, M_NOWAIT | M_ZERO);
		if (vap->iv_tim_bitmap == NULL) {
			printf("%s: no memory for TIM bitmap!\n", __func__);
			/* XXX good enough to keep from crashing? */
			vap->iv_tim_len = 0;
		}
	}
}

void
ieee80211_power_vdetach(struct ieee80211vap *vap)
{
	if (vap->iv_tim_bitmap != NULL) {
		FREE(vap->iv_tim_bitmap, M_DEVBUF);
		vap->iv_tim_bitmap = NULL;
	}
}

/*
 * Clear any frames queued on a node's power save queue.
 * The number of frames that were present is returned.
 */
int
ieee80211_node_saveq_drain(struct ieee80211_node *ni)
{
	struct sk_buff *skb;
	int qlen;

	IEEE80211_NODE_SAVEQ_LOCK(ni);
	qlen = skb_queue_len(&ni->ni_savedq);
	while ((skb = __skb_dequeue(&ni->ni_savedq)) != NULL) {
		ieee80211_free_node(ni);
		dev_kfree_skb_any(skb);
	}
	IEEE80211_NODE_SAVEQ_UNLOCK(ni);

	return qlen;
}

/*
 * Age frames on the power save queue. The aging interval is
 * 4 times the listen interval specified by the station.  This
 * number is factored into the age calculations when the frame
 * is placed on the queue.  We store ages as time differences
 * so we can check and/or adjust only the head of the list.
 * If a frame's age exceeds the threshold then discard it.
 * The number of frames discarded is returned so the caller
 * can check if it needs to adjust the tim.
 */
int
ieee80211_node_saveq_age(struct ieee80211_node *ni)
{
	int discard = 0;

	/* XXX racey but good 'nuf? */
	if (IEEE80211_NODE_SAVEQ_QLEN(ni) != 0) {
#ifdef IEEE80211_DEBUG
		struct ieee80211vap *vap = ni->ni_vap;
#endif
		struct sk_buff *skb;

		IEEE80211_NODE_SAVEQ_LOCK(ni);
		while ((skb = skb_peek(&ni->ni_savedq)) != NULL &&
		     M_AGE_GET(skb) < IEEE80211_INACT_WAIT) {
			IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
				"discard frame, age %u", M_AGE_GET(skb));

			skb = __skb_dequeue(&ni->ni_savedq);
			dev_kfree_skb_any(skb);
			discard++;
		}
		if (skb != NULL)
			M_AGE_SUB(skb, IEEE80211_INACT_WAIT);
		IEEE80211_NODE_SAVEQ_UNLOCK(ni);

		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
			"discard %u frames for age", discard);
		IEEE80211_NODE_STAT_ADD(ni, ps_discard, discard);
	}
	return discard;
}

/*
 * Indicate whether there are frames queued for a station in power-save mode.
 */
static void
ieee80211_set_tim(struct ieee80211_node *ni, int set)
{
	struct ieee80211vap *vap = ni->ni_vap;
	u_int16_t aid;

	KASSERT(vap->iv_opmode == IEEE80211_M_HOSTAP ||
		vap->iv_opmode == IEEE80211_M_IBSS,
		("operating mode %u", vap->iv_opmode));

	aid = IEEE80211_AID(ni->ni_associd);
	KASSERT(aid < vap->iv_max_aid,
		("bogus aid %u, max %u", aid, vap->iv_max_aid));

	IEEE80211_LOCK(ni->ni_ic);
	if (set != (isset(vap->iv_tim_bitmap, aid) != 0)) {
		if (set) {
			setbit(vap->iv_tim_bitmap, aid);
			vap->iv_ps_pending++;
		} else {
			clrbit(vap->iv_tim_bitmap, aid);
			vap->iv_ps_pending--;
		}
		vap->iv_flags |= IEEE80211_F_TIMUPDATE;
	}
	IEEE80211_UNLOCK(ni->ni_ic);
}

/*
 * Save an outbound packet for a node in power-save sleep state.
 * The new packet is placed on the node's saved queue, and the TIM
 * is changed, if necessary.
 */
void
ieee80211_pwrsave(struct ieee80211_node *ni, struct sk_buff *skb)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	unsigned long flags;
	struct sk_buff *tail;
	int qlen, age;

	spin_lock_irqsave(&ni->ni_savedq.lock, flags);
	if (skb_queue_len(&ni->ni_savedq) >= IEEE80211_PS_MAX_QUEUE) {
		IEEE80211_NODE_STAT(ni, psq_drops);
		spin_unlock_irqrestore(&ni->ni_savedq.lock, flags);
		IEEE80211_NOTE(vap, IEEE80211_MSG_ANY, ni,
			"pwr save q overflow, drops %d (size %d)",
			ni->ni_stats.ns_psq_drops, IEEE80211_PS_MAX_QUEUE);
#ifdef IEEE80211_DEBUG
		if (ieee80211_msg_dumppkts(vap))
			ieee80211_dump_pkt(ni->ni_ic, skb->data, skb->len, -1, -1);
#endif
		dev_kfree_skb(skb);
		return;
	}
	/*
	 * Tag the frame with it's expiry time and insert
	 * it in the queue.  The aging interval is 4 times
	 * the listen interval specified by the station.
	 * Frames that sit around too long are reclaimed
	 * using this information.
	 */
	/* XXX handle overflow? */
	age = ((ni->ni_intval * ic->ic_lintval) << 2) / 1024; /* TU -> secs */
	tail = skb_peek_tail(&ni->ni_savedq);
	if (tail != NULL) {
		age -= M_AGE_GET(tail);
		__skb_append(tail, skb, &ni->ni_savedq);
	} else
		__skb_queue_head(&ni->ni_savedq, skb);
	M_AGE_SET(skb, age);
	qlen = skb_queue_len(&ni->ni_savedq);
	spin_unlock_irqrestore(&ni->ni_savedq.lock, flags);

	IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
		"save frame, %u now queued", qlen);

	if (qlen == 1 && vap->iv_set_tim != NULL)
		vap->iv_set_tim(ni, 1);
}

/*
 * Handle power-save state change in ap/ibss mode.
 */
void
ieee80211_node_pwrsave(struct ieee80211_node *ni, int enable)
{
	struct ieee80211vap *vap = ni->ni_vap;

	KASSERT(vap->iv_opmode == IEEE80211_M_HOSTAP ||
		vap->iv_opmode == IEEE80211_M_IBSS,
		("unexpected operating mode %u", vap->iv_opmode));

	if (enable) {
		if ((ni->ni_flags & IEEE80211_NODE_PWR_MGT) == 0)
			vap->iv_ps_sta++;
		ni->ni_flags |= IEEE80211_NODE_PWR_MGT;
		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
			"power save mode on, %u sta's in ps mode",
			vap->iv_ps_sta);
		return;
	}

	if ((ni->ni_flags & IEEE80211_NODE_PWR_MGT))
		vap->iv_ps_sta--;
	ni->ni_flags &= ~IEEE80211_NODE_PWR_MGT;
	IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
		"power save mode off, %u sta's in ps mode", vap->iv_ps_sta);
	/* XXX if no stations in ps mode, flush mc frames */

	/*
	 * Flush queued unicast frames.
	 */
	if (IEEE80211_NODE_SAVEQ_QLEN(ni) == 0) {
		if (vap->iv_set_tim != NULL)
			vap->iv_set_tim(ni, 0);		/* just in case */
		return;
	}
	IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
		"flush ps queue, %u packets queued",
		IEEE80211_NODE_SAVEQ_QLEN(ni));
	for (;;) {
		struct sk_buff *skb;
		int qlen;

		IEEE80211_NODE_SAVEQ_LOCK(ni);
		IEEE80211_NODE_SAVEQ_DEQUEUE(ni, skb, qlen);
		IEEE80211_NODE_SAVEQ_UNLOCK(ni);
		if (skb == NULL)
			break;
		/* 
		 * If this is the last packet, turn off the TIM bit.
		 *
		 * Set the M_PWR_SAV bit on skb to allow encap to test for
		 * adding MORE_DATA bit to wh.
		 *
		 * The 802.11 MAC Spec says we should only set MORE_DATA for 
		 * unicast packets when the STA is in PS mode (7.1.3.1.8);
		 * which it isn't.
		 */
		// M_PWR_SAV_SET(skb);

#ifdef ATH_SUPERG_XR
		/*
		 * if it is a XR vap, send the data to associated normal net
		 * device. XR vap has a net device which is not registered with
		 * OS. 
		 */
		if (vap->iv_xrvap && vap->iv_flags & IEEE80211_F_XR)
			skb->dev = vap->iv_xrvap->iv_dev;
		else
			skb->dev = vap->iv_dev;		/* XXX? unnecessary */
#endif
		
		ieee80211_parent_queue_xmit(skb);
	}
	vap->iv_set_tim(ni, 0);
}

/*
 * Handle power-save state change in station mode.
 */
void
ieee80211_sta_pwrsave(struct ieee80211vap *vap, int enable)
{
	struct ieee80211_node *ni = vap->iv_bss;
	int qlen;

	if (!((enable != 0) ^ ((ni->ni_flags & IEEE80211_NODE_PWR_MGT) != 0)))
		return;

	IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
		"sta power save mode %s", enable ? "on" : "off");
	if (!enable) {
		ni->ni_flags &= ~IEEE80211_NODE_PWR_MGT;
		ieee80211_send_nulldata(ieee80211_ref_node(ni));
		/*
		 * Flush any queued frames; we can do this immediately
		 * because we know they'll be queued behind the null
		 * data frame we send the ap.
		 * XXX can we use a data frame to take us out of ps?
		 */
		qlen = IEEE80211_NODE_SAVEQ_QLEN(ni);
		if (qlen != 0) {
			IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
				"flush ps queue, %u packets queued", qlen);
			for (;;) {
				struct sk_buff *skb;

				IEEE80211_NODE_SAVEQ_LOCK(ni);
				skb = __skb_dequeue(&ni->ni_savedq);
				IEEE80211_NODE_SAVEQ_UNLOCK(ni);
				if (skb == NULL)
					break;
				ieee80211_parent_queue_xmit(skb);
			}
		}
	} else {
		ni->ni_flags |= IEEE80211_NODE_PWR_MGT;
		ieee80211_send_nulldata(ieee80211_ref_node(ni));
	}
}
