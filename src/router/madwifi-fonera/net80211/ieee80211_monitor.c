/*-
 * Copyright (c) 2005 John Bicket
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
 * $Id: ieee80211_monitor.c 1520 2006-04-21 16:57:59Z dyqith $
 */

#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 monitor mode 
 */
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/sysctl.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>

#include <net/iw_handler.h>
#include <linux/wireless.h>
#include <linux/if_arp.h>		/* XXX for ARPHRD_* */

#include <asm/uaccess.h>

#include "if_media.h"
#include "if_ethersubr.h"

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_monitor.h>
#include <ath/if_athvar.h>



static int
ratecode_to_dot11(int ratecode)
{
        switch (ratecode) {
                /* a */
        case 0x0b: return 12;
        case 0x0f: return 18;
        case 0x0a: return 24;
        case 0x0e: return 36;
        case 0x09: return 48;
        case 0x0d: return 72;
        case 0x08: return 96;
        case 0x0c: return 108;

        case 0x1b: return 2;
        case 0x1a: return 4;
        case 0x1e: return 4;
        case 0x19: return 11;
        case 0x1d: return 11;
        case 0x18: return 22;
        case 0x1c: return 22;
        }
        return 0;
}

struct ar5212_openbsd_desc {
	/*
	 * tx_control_0
	 */
	u_int32_t frame_len:12;
	u_int32_t reserved_12_15:4;
	u_int32_t xmit_power:6;
	u_int32_t rts_cts_enable:1;
	u_int32_t veol:1;
	u_int32_t clear_dest_mask:1;
	u_int32_t ant_mode_xmit:4;
	u_int32_t inter_req:1;
	u_int32_t encrypt_key_valid:1;
	u_int32_t cts_enable:1;

	u_int32_t r1;

	/*
	 * tx_control_2
	 */
	u_int32_t rts_duration:15;
	u_int32_t duration_update_enable:1;
	u_int32_t xmit_tries0:4;
	u_int32_t xmit_tries1:4;
	u_int32_t xmit_tries2:4;
	u_int32_t xmit_tries3:4;

	/*
	 * tx_control_3
	 */
	u_int32_t xmit_rate0:5;
	u_int32_t xmit_rate1:5;
	u_int32_t xmit_rate2:5;
	u_int32_t xmit_rate3:5;
	u_int32_t rts_cts_rate:5;
	u_int32_t reserved_25_31:7;
};

void
ieee80211_monitor_encap(struct ieee80211vap *vap, struct sk_buff *skb) 
{
	struct ieee80211_cb *cb = (struct ieee80211_cb *) skb->cb;
	struct ieee80211_phy_params *ph =
		(struct ieee80211_phy_params *) (skb->cb + sizeof(struct ieee80211_cb));
	cb->flags = M_RAW;
	cb->ni = NULL;
	cb->next = NULL;
	memset(ph, 0, sizeof(struct ieee80211_phy_params));

	/* send at a static rate if it is configured */
	ph->rate0 = vap->iv_fixed_rate > 0 ? vap->iv_fixed_rate : 2;
	/* don't retry control packets */
	ph->try0 = 11;
	ph->power = 60;

	switch (skb->dev->type) {
	case ARPHRD_IEEE80211: {
		struct ieee80211_frame *wh = (struct ieee80211_frame *) skb->data;
		if ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL) 
			ph->try0 = 1;
		break;
	}
	case ARPHRD_IEEE80211_PRISM: {
		struct ieee80211_frame *wh = NULL;
		wlan_ng_prism2_header *p2h = 
			(wlan_ng_prism2_header *) skb->data;
		/* does it look like there is a prism header here? */
		if (skb->len > sizeof (wlan_ng_prism2_header) &&
                    p2h->msgcode == DIDmsg_lnxind_wlansniffrm &&
		    p2h->rate.did == DIDmsg_lnxind_wlansniffrm_rate) {
                        ph->rate0 = p2h->rate.data;
                        skb_pull(skb, sizeof(wlan_ng_prism2_header));
		}
		wh = (struct ieee80211_frame *) skb->data;
		if ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL) 
			ph->try0 = 1;
                break;
	}
	case ARPHRD_IEEE80211_ATHDESC: {
		if (skb->len > ATHDESC_HEADER_SIZE) {
			struct ar5212_openbsd_desc *desc =
				(struct ar5212_openbsd_desc *) (skb->data + 8);
			ph->power = desc->xmit_power;
			ph->rate0 = ratecode_to_dot11(desc->xmit_rate0);
			ph->rate1 = ratecode_to_dot11(desc->xmit_rate1);
			ph->rate2 = ratecode_to_dot11(desc->xmit_rate2);
			ph->rate3 = ratecode_to_dot11(desc->xmit_rate3);
			ph->try0 = desc->xmit_tries0;
			ph->try1 = desc->xmit_tries1;
			ph->try2 = desc->xmit_tries2;
			ph->try3 = desc->xmit_tries3;
			skb_pull(skb, ATHDESC_HEADER_SIZE);
		}
		break;
	}		
	default:
		break;
	}

	if (!ph->rate0) {
		ph->rate0 = 0;
		ph->try0 = 11;
	}
}
EXPORT_SYMBOL(ieee80211_monitor_encap);

/*
 * Context: softIRQ (tasklet)
 */
void
ieee80211_input_monitor(struct ieee80211com *ic, struct sk_buff *skb,
	struct ath_desc *ds, int tx, u_int32_t mactime, struct ath_softc *sc) 
{
	struct ieee80211vap *vap, *next;
	u_int32_t signal = 0;
	signal = tx ? ds->ds_txstat.ts_rssi : ds->ds_rxstat.rs_rssi;
	
	/* XXX locking */
	for (vap = TAILQ_FIRST(&ic->ic_vaps); vap != NULL; vap = next) {
		struct sk_buff *skb1;
		struct net_device *dev = vap->iv_dev;
		struct ieee80211_frame *wh = (struct ieee80211_frame *)skb->data;
		u_int8_t dir = wh->i_fc[1] & IEEE80211_FC1_DIR_MASK;

		next = TAILQ_NEXT(vap, iv_next);
		if (vap->iv_opmode != IEEE80211_M_MONITOR ||
		    vap->iv_state != IEEE80211_S_RUN)
			continue;
		if (vap->iv_monitor_nods_only &&
		    dir != IEEE80211_FC1_DIR_NODS) {
			/* don't rx fromds, tods, or dstods packets */
			continue;
		}		    
		skb1 = skb_copy(skb, GFP_ATOMIC);
		if (skb1 == NULL) {
			/* XXX stat+msg */
			continue;
		}		
		if (vap->iv_monitor_txf_len && tx) {
			/* truncate transmit feedback packets */
			skb_trim(skb1, vap->iv_monitor_txf_len);
			skb1->nh.raw = skb1->data;
		}
		switch (vap->iv_dev->type) {
		case ARPHRD_IEEE80211:
			break;
		case ARPHRD_IEEE80211_PRISM: {
			wlan_ng_prism2_header *ph;
			if (skb_headroom(skb1) < sizeof(wlan_ng_prism2_header)) {
				dev_kfree_skb(skb1);
				skb1 = NULL;
				break;
			}
			
			ph = (wlan_ng_prism2_header *)
				skb_push(skb1, sizeof(wlan_ng_prism2_header));
			memset(ph, 0, sizeof(wlan_ng_prism2_header));
			
			ph->msgcode = DIDmsg_lnxind_wlansniffrm;
			ph->msglen = sizeof(wlan_ng_prism2_header);
			strncpy(ph->devname, dev->name, sizeof(ph->devname));
			
			ph->hosttime.did = DIDmsg_lnxind_wlansniffrm_hosttime;
			ph->hosttime.status = 0;
			ph->hosttime.len = 4;
			ph->hosttime.data = jiffies;
			/* Pass up tsf clock in mactime */
			ph->mactime.did = DIDmsg_lnxind_wlansniffrm_mactime;
			ph->mactime.status = 0;
			ph->mactime.len = 4;
			ph->mactime.data = mactime;
			
			ph->istx.did = DIDmsg_lnxind_wlansniffrm_istx;
			ph->istx.status = 0;
			ph->istx.len = 4;
			ph->istx.data = tx ? P80211ENUM_truth_true : P80211ENUM_truth_false;
			
			ph->frmlen.did = DIDmsg_lnxind_wlansniffrm_frmlen;
			ph->frmlen.status = 0;
			ph->frmlen.len = 4;
			ph->frmlen.data = skb->len - sizeof(wlan_ng_prism2_header);
			
			ph->channel.did = DIDmsg_lnxind_wlansniffrm_channel;
			ph->channel.status = 0;
			ph->channel.len = 4;
			ph->channel.data =
				ieee80211_mhz2ieee(ic->ic_curchan->ic_freq, 
					ic->ic_curchan->ic_flags);
			
			ph->rssi.did = DIDmsg_lnxind_wlansniffrm_rssi;
			ph->rssi.status = 0;
			ph->rssi.len = 4;
			ph->rssi.data = signal;
			
			ph->noise.did = DIDmsg_lnxind_wlansniffrm_noise;
			ph->noise.status = 0;
			ph->noise.len = 4;
			if (ic->ic_getchannelnoise)
				ph->noise.data = (int32_t) ic->ic_getchannelnoise(ic,ic->ic_curchan);
			else
				ph->noise.data = -95;
			
			ph->signal.did = DIDmsg_lnxind_wlansniffrm_signal;
			ph->signal.status = 0;
			ph->signal.len = 4;
			ph->signal.data = signal;
			
			ph->rate.did = DIDmsg_lnxind_wlansniffrm_rate;
			ph->rate.status = 0;
			ph->rate.len = 4;
			ph->rate.data = sc->sc_hwmap[ds->ds_rxstat.rs_rate].ieeerate;
			break;
		}
		case ARPHRD_IEEE80211_RADIOTAP: {
			if (tx) {
				struct ath_tx_radiotap_header *th;
				if (skb_headroom(skb1) < sizeof(struct ath_tx_radiotap_header)) {
					printk("%s:%d %s\n", __FILE__, __LINE__, __func__);
					dev_kfree_skb(skb1);
					skb1 = NULL;
					break;
				}
				
				th = (struct ath_tx_radiotap_header *) skb_push(skb1, 
					sizeof(struct ath_tx_radiotap_header));
				memset(th, 0, sizeof(struct ath_tx_radiotap_header));
				th->wt_ihdr.it_version = 0;
				th->wt_ihdr.it_len = cpu_to_le16(sizeof(struct ath_tx_radiotap_header));
				th->wt_ihdr.it_present = cpu_to_le32(ATH_TX_RADIOTAP_PRESENT);
				th->wt_flags = 0;
				th->wt_rate = sc->sc_hwmap[ds->ds_rxstat.rs_rate].ieeerate;
				th->wt_txpower = 0;
				th->wt_antenna = 0;
			} else {
				struct ath_rx_radiotap_header *th;
				if (skb_headroom(skb1) < sizeof(struct ath_rx_radiotap_header)) {
					printk("%s:%d %s\n", __FILE__, __LINE__, __func__);
					dev_kfree_skb(skb1);
					skb1 = NULL;
					break;
				}
				
				th = (struct ath_rx_radiotap_header *) skb_push(skb1, 
					sizeof(struct ath_rx_radiotap_header));
				memset(th, 0, sizeof(struct ath_rx_radiotap_header));
				th->wr_ihdr.it_version = 0;
				th->wr_ihdr.it_len = cpu_to_le16(sizeof(struct ath_rx_radiotap_header));
				th->wr_ihdr.it_present = cpu_to_le32(ATH_RX_RADIOTAP_PRESENT);

				if (ic->ic_flags & IEEE80211_F_SHPREAMBLE)
					th->wr_flags |= IEEE80211_RADIOTAP_F_SHORTPRE;

				th->wr_rate = sc->sc_hwmap[ds->ds_rxstat.rs_rate].ieeerate;
				th->wr_chan_freq = cpu_to_le16(ic->ic_curchan->ic_freq);

				/* Define the channel flags for radiotap */
				switch(sc->sc_curmode) {
					case IEEE80211_MODE_11A:
						th->wr_chan_flags =
							cpu_to_le16(IEEE80211_CHAN_A);
						break;
					case IEEE80211_MODE_TURBO_A:
						th->wr_chan_flags = 
							cpu_to_le16(IEEE80211_CHAN_TA);
						break;
					case IEEE80211_MODE_11B:
						th->wr_chan_flags = 
							cpu_to_le16(IEEE80211_CHAN_B);
						break;
					case IEEE80211_MODE_11G:
						th->wr_chan_flags = 
							cpu_to_le16(IEEE80211_CHAN_G);
						break;
					case IEEE80211_MODE_TURBO_G:
						th->wr_chan_flags = 
							cpu_to_le16(IEEE80211_CHAN_TG);
						break;
					default:
						th->wr_chan_flags = 0; /* unknown */
						break;
				}

				th->wr_antenna = ds->ds_rxstat.rs_antenna;
				th->wr_antsignal = signal;
				memcpy(&th->wr_fcs, &skb1->data[skb1->len - IEEE80211_CRC_LEN],
				       IEEE80211_CRC_LEN);
				th->wr_fcs = cpu_to_le32(th->wr_fcs);
				memcpy(&th->wr_tsft, &mactime, IEEE80211_TSF_LEN);
				th->wr_tsft = cpu_to_le64(th->wr_tsft);
			}
			break;
		}
		case ARPHRD_IEEE80211_ATHDESC: {
			if (skb_headroom(skb1) < ATHDESC_HEADER_SIZE) {
				printk("%s:%d %s\n", __FILE__, __LINE__, __func__);
				dev_kfree_skb(skb1);
				skb1 = NULL;
				break;
			}
			memcpy(skb_push(skb1, ATHDESC_HEADER_SIZE), ds, ATHDESC_HEADER_SIZE);
			break;
		}
		default:
			break;
		}
		if (skb1) {
			if (!tx) {
				/* Remove FCS from end of rx frames*/
				skb_trim(skb1, skb1->len - IEEE80211_CRC_LEN);
			}
			skb1->dev = dev; /* NB: deliver to wlanX */
			skb1->mac.raw = skb1->data;
			skb1->ip_summed = CHECKSUM_NONE;
			skb1->pkt_type = PACKET_OTHERHOST;
			skb1->protocol = __constant_htons(0x0019); /* ETH_P_80211_RAW */
			
			netif_rx(skb1);
			
			vap->iv_devstats.rx_packets++;
			vap->iv_devstats.rx_bytes += skb1->len;
		}
	}
}
EXPORT_SYMBOL(ieee80211_input_monitor);
