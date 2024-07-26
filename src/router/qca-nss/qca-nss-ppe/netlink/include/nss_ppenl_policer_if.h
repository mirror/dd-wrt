/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * @file nss_ppenl_policer_if.h
 *	NSS PPE Netlink POLICER
 */
#ifndef __NSS_PPENL_POLICER_IF_H
#define __NSS_PPENL_POLICER_IF_H

/*
 * POLICER Configure Family
 */
#define NSS_PPENL_POLICER_FAMILY "nss_ppenl_poli"

/*
 * Policer action info
 */
struct nss_ppenl_policer_action_info {
	uint8_t yellow_dp;	/* yellow drop priority */
	uint8_t yellow_int_pri;	/* yellow priority */
	uint8_t yellow_pcp;	/* yellow pcp */
	uint8_t yellow_dei;	/* yellow dei */
	uint8_t yellow_dscp;	/* yellow dscp */
};

/*
 * @brief Policer config
 */
struct nss_ppenl_policer_config {
	int policer_id;	/* user given policer id */
	bool is_port_policer;	/* true if it is port policer */
	bool meter_mode;	/* 0 for RFC 2698, 1 for RFC 2697, 4115 */
	bool meter_unit;	/* 0 for byte based, 1 for frame based */
	uint32_t committed_rate;	/* committed information rate */
	uint32_t committed_burst_size;	/* CBS, committed burst size */
	uint32_t peak_rate;	/* EIR, expected information rate */
	uint32_t peak_burst_size;	/* EBS, expected burst size */
	char dev[IFNAMSIZ];	/* dev name for port policer */
	uint8_t meter_enable;	/* meter enable */
	uint8_t couple_enable;	/* couple enable */
	uint8_t colour_aware;	/* colour aware */
	struct nss_ppenl_policer_action_info action_info;	/* policer action info */
	int ret;	/* return type from ppe */
};


/*
 * @brief POLICER rule
 */
struct nss_ppenl_policer_rule {
	struct nss_ppenl_cmn cm;	/*< common message header */
	struct nss_ppenl_policer_config config;	/* ppe policer config */
};

/*
 * @brief Message types
 */
enum nss_ppe_policer_message_types {
	NSS_PPE_POLICER_CREATE_RULE_MSG,	/* Policer rule create message */
	NSS_PPE_POLICER_DESTROY_RULE_MSG,	/* Policer rule delete message */
	NSS_PPE_POLICER_MAX_MSG_TYPES		/* Maximum message type */
};

/**
 * @brief NETLINK POLICER message init
 *
 * @param rule[IN] NSS NETLINK POLICER rule
 * @param type[IN] POLICER message type
 */
static inline void nss_ppenl_policer_rule_init(struct nss_ppenl_policer_rule *rule, enum nss_ppe_policer_message_types type)
{
	nss_ppenl_cmn_set_ver(&rule->cm, NSS_PPENL_VER);
	nss_ppenl_cmn_init_cmd(&rule->cm, sizeof(struct nss_ppenl_policer_rule), type);
}

#endif /* __NSS_PPENL_POLICER_IF_H */
