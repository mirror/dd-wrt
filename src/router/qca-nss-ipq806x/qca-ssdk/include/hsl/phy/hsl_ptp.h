/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup hsl_ptp
 * @{
 */
#ifndef _HSL_PTP_H_
#define _HSL_PTP_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum {
	PTP_PKT_SEQID_CHECKED = 0,
	PTP_PKT_SEQID_UNMATCHED,
	PTP_PKT_SEQID_MATCHED,
	PTP_PKT_SEQID_MATCH_MAX
};

enum {
	PTP_MSG_SYNC = 0,
	PTP_MSG_DREQ,
	PTP_MSG_PREQ,
	PTP_MSG_PRESP,
	PTP_MSG_MAX
};

/* statistics for the event packet*/
typedef struct {
	/* the counter saves the packet with sequence id
	 * matched and unmatched */
	a_uint64_t sync_cnt[PTP_PKT_SEQID_MATCH_MAX];
	a_uint64_t delay_req_cnt[PTP_PKT_SEQID_MATCH_MAX];
	a_uint64_t pdelay_req_cnt[PTP_PKT_SEQID_MATCH_MAX];
	a_uint64_t pdelay_resp_cnt[PTP_PKT_SEQID_MATCH_MAX];
} hsl_ptp_event_pkt_stat_t;

int hsl_ptp_event_stat_operation(char *phy_driver_name, char *buf);
void hsl_ptp_event_stat_update(hsl_ptp_event_pkt_stat_t *ptp_event_stat,
		a_int32_t msg_type, a_int32_t seqid_matched);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HSL_PTP_H_ */
/**
 * @}
 */
