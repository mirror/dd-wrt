#ifndef _MORSE_IPMON_H_
#define _MORSE_IPMON_H_

/*
 * IP monitor
 *
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/ieee80211.h>
#include <net/tcp.h>
#include <linux/ktime.h>

#define IPMON_CHECK 0xaabbccdd
/* Allow ipmon to be used with iperf without intefering with iperf's own data fields. */
#define IPMON_PAYLOAD_OFFSET (32)

struct ipmon_hdr {
	u32 check;
	u32 pktnum;
	/* Times in milliseconds since first record received by this module */
	u64 time_client;
	/*
	 * The following names are intentionally vague so the call points can be moved around
	 * without requiring changes to all of the ipmon components.
	 */
	u64 time_client_drv1;	/* Early in the driver Tx path, but may be moved */
	u64 time_client_drv2;	/* Late in the driver Tx path, but may be moved */
	u64 time_client_fw;
	u64 time_server_fw;
	u64 time_server_drv;
	u64 time_server;
	u32 queue_stop;
};

/* Location of caller to IPMON */
enum ipmon_loc {
	IPMON_LOC_CLIENT_DRV1,
	IPMON_LOC_CLIENT_DRV2,
	IPMON_LOC_SERVER_DRV
};

#define QOS_HDR_SIZE	32	/* sizeof(struct ieee80211_qos_hdr) */
#define LLC_HDR_SIZE	8	/* sizeof(struct llc_snap_hdr) */
#define IP_HDR_SIZE	20	/* sizeof(struct iphdr) */
#define IPMON_HDRS_LEN (QOS_HDR_SIZE + LLC_HDR_SIZE + IP_HDR_SIZE)

void morse_ipmon(u64 *time_start, struct sk_buff *skb, char *data, int len,
		 enum ipmon_loc loc, int queue_stop);

#endif /* ! _MORSE_IPMON_H_ */
