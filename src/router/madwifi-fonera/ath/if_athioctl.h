/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_athioctl.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */

/*
 * Ioctl-related defintions for the Atheros Wireless LAN controller driver.
 */
#ifndef _DEV_ATH_ATHIOCTL_H
#define _DEV_ATH_ATHIOCTL_H

struct ath_stats {
	u_int32_t ast_watchdog;		/* device reset by watchdog */
	u_int32_t ast_hardware;		/* fatal hardware error interrupts */
	u_int32_t ast_bmiss;		/* beacon miss interrupts */
	u_int32_t ast_rxorn;		/* rx overrun interrupts */
	u_int32_t ast_rxeol;		/* rx eol interrupts */
	u_int32_t ast_txurn;		/* tx underrun interrupts */
	u_int32_t ast_mib;		/* mib interrupts */
	u_int32_t ast_tx_packets;	/* packet sent on the interface */
	u_int32_t ast_tx_mgmt;		/* management frames transmitted */
	u_int32_t ast_tx_discard;	/* frames discarded prior to assoc */
	u_int32_t ast_tx_invalid;	/* frames discarded due to is device gone */
	u_int32_t ast_tx_qstop;		/* tx queue stopped because it's full */
	u_int32_t ast_tx_encap;		/* tx encapsulation failed */
	u_int32_t ast_tx_nonode;		/* tx failed due to of no node */
	u_int32_t ast_tx_nobuf;		/* tx failed due to of no tx buffer (data) */
	u_int32_t ast_tx_nobufmgt;	/* tx failed due to of no tx buffer (mgmt)*/
	u_int32_t ast_tx_xretries;	/* tx failed due to of too many retries */
	u_int32_t ast_tx_fifoerr;	/* tx failed due to of FIFO underrun */
	u_int32_t ast_tx_filtered;	/* tx failed due to xmit filtered */
	u_int32_t ast_tx_shortretry;	/* tx on-chip retries (short) */
	u_int32_t ast_tx_longretry;	/* tx on-chip retries (long) */
	u_int32_t ast_tx_badrate;	/* tx failed due to of bogus xmit rate */
	u_int32_t ast_tx_noack;		/* tx frames with no ack marked */
	u_int32_t ast_tx_rts;		/* tx frames with rts enabled */
	u_int32_t ast_tx_cts;		/* tx frames with cts enabled */
	u_int32_t ast_tx_shortpre;	/* tx frames with short preamble */
	u_int32_t ast_tx_altrate;	/* tx frames with alternate rate */
	u_int32_t ast_tx_protect;	/* tx frames with protection */
	u_int32_t ast_rx_orn;		/* rx failed due to of desc overrun */
	u_int32_t ast_rx_crcerr;		/* rx failed due to of bad CRC */
	u_int32_t ast_rx_fifoerr;	/* rx failed due to of FIFO overrun */
	u_int32_t ast_rx_badcrypt;	/* rx failed due to of decryption */
	u_int32_t ast_rx_badmic;		/* rx failed due to of MIC failure */
	u_int32_t ast_rx_phyerr;		/* rx PHY error summary count */
	u_int32_t ast_rx_phy[32];	/* rx PHY error per-code counts */
	u_int32_t ast_rx_tooshort;	/* rx discarded due to frame too short */
	u_int32_t ast_rx_toobig;		/* rx discarded due to frame too large */
	u_int32_t ast_rx_nobuf;		/* rx setup failed due to of no skbuff */
	u_int32_t ast_rx_packets;	/* packet recv on the interface */
	u_int32_t ast_rx_mgt;		/* management frames received */
	u_int32_t ast_rx_ctl;		/* control frames received */
	int8_t ast_tx_rssi;		/* tx rssi of last ack */
	int8_t ast_rx_rssi;		/* rx rssi from histogram */
	u_int32_t ast_be_xmit;		/* beacons transmitted */
	u_int32_t ast_be_nobuf;		/* no skbuff available for beacon */
	u_int32_t ast_per_cal;		/* periodic calibration calls */
	u_int32_t ast_per_calfail;	/* periodic calibration failed */
	u_int32_t ast_per_rfgain;	/* periodic calibration rfgain reset */
	u_int32_t ast_rate_calls;	/* rate control checks */
	u_int32_t ast_rate_raise;	/* rate control raised xmit rate */
	u_int32_t ast_rate_drop;		/* rate control dropped xmit rate */
	u_int32_t ast_ant_defswitch;	/* rx/default antenna switches */
	u_int32_t ast_ant_txswitch;	/* tx antenna switches */
	u_int32_t ast_ant_rx[8];		/* rx frames with antenna */
	u_int32_t ast_ant_tx[8];		/* tx frames with antenna */
};

struct ath_diag {
	char	ad_name[IFNAMSIZ];	/* if name, e.g. "ath0" */
	u_int16_t ad_id;
#define	ATH_DIAG_DYN	0x8000		/* allocate buffer in caller */
#define	ATH_DIAG_IN	0x4000		/* copy in parameters */
#define	ATH_DIAG_OUT	0x0000		/* copy out results (always) */
#define	ATH_DIAG_ID	0x0fff
	u_int16_t ad_in_size;		/* pack to fit, yech */
	void __user *ad_in_data;
	void __user *ad_out_data;
	u_int	ad_out_size;

};

#define	ATH_RADAR_MUTE_TIME	1	/* Set dfs mute time for dfs test mode */

#ifdef __linux__
#define	SIOCGATHSTATS		(SIOCDEVPRIVATE+0)
#define	SIOCGATHDIAG		(SIOCDEVPRIVATE+1)
#define SIOCGATHRADARSIG	(SIOCDEVPRIVATE+2)
#else
#define	SIOCGATHSTATS		_IOWR('i', 137, struct ifreq)
#define	SIOCGATHDIAG		_IOWR('i', 138, struct ath_diag)
#endif
#endif /* _DEV_ATH_ATHIOCTL_H */
