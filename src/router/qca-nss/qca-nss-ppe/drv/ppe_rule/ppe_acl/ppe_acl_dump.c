/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/debugfs.h>
#include "ppe_acl.h"
#include "ppe_acl_dump.h"

/*
 * ppe_acl_dump_ipo_type_str
 * 	ipo type string array
 */
static const char *ppe_acl_dump_ipo_type_str[] = {
	"IPO",			/* IPO rule. */
	"PRE_IPO",		/* PRE_IPO rule. */
};

/*
 * ppe_acl_dump_src_type_str
 * 	source type string array
 */
static const char *ppe_acl_dump_src_type_str[] = {
	"SRC_TYPE_INVALID",		/* Source type invalid. */
	"SRC_TYPE_DEV",			/* Source type device. */
	"SRC_TYPE_SC",			/* Source type service code. */
	"SRC_TYPE_FLOW",		/* Source type flow. */
};

/*
 * ppe_acl_dump_slice_type_str
 * 	Slice type string array
 */
static const char *ppe_acl_dump_slice_type_str[] = {
        "SLICE_DST_MAC",         /* ACL destination MAC slice type. */
        "SLICE_SRC_MAC",         /* ACL source MAC slice type. */
        "SLICE_VLAN",            /* ACL VLAN slice type. */
        "SLICE_L2_MISC",         /* ACL L2 miscellaneous slice type. */
        "SLICE_DST_IPV4",        /* ACL destination IPv4 slice type. */
        "SLICE_SRC_IPV4",        /* ACL source IPv4 slice type. */
        "SLICE_DST_IPV6_0",      /* ACL destination IPv6 0 slice type. */
        "SLICE_DST_IPV6_1",      /* ACL destination IPv6 1 slice type. */
        "SLICE_DST_IPV6_2",      /* ACL destination IPv6 2 slice type. */
        "SLICE_SRC_IPV6_0",      /* ACL source IPv6 0 slice type. */
        "SLICE_SRC_IPV6_1",      /* ACL source IPv6 1 slice type. */
        "SLICE_SRC_IPV6_2",      /* ACL source IPv6 2 slice type. */
        "SLICE_IP_MISC",         /* ACL IP miscellaneous slice type. */
        "SLICE_UDF_012",         /* ACL UDF 012 slice type. */
        "SLICE_UDF_123",         /* ACL UDF 123 slice type. */
};

/*
 * ppe_acl_dump_write_reset()
 *	Reset the msg buffer, specifying a new initial prefix
 */
int ppe_acl_dump_write_reset(struct ppe_acl_dump_instance *adi, char *base_prefix)
{
	int result;

	adi->msgp = adi->msg;
	adi->msg_len = 0;

	result = snprintf(adi->prefix, PPE_ACL_DUMP_PREFIX_SIZE, "%s", base_prefix);
	if ((result < 0) || (result >= PPE_ACL_DUMP_PREFIX_SIZE)) {
		return -1;
	}

	adi->prefix_level = 0;
	adi->prefix_levels[adi->prefix_level] = result;

	return 0;
}

/*
 * ppe_acl_dump_prefix_add()
 *	Add another level to the prefix
 */
int ppe_acl_dump_prefix_add(struct ppe_acl_dump_instance *adi, char *prefix)
{
	int pxsz;
	int pxremain;
	int result;

	pxsz = adi->prefix_levels[adi->prefix_level];
	pxremain = PPE_ACL_DUMP_PREFIX_SIZE - pxsz;

	result = snprintf(adi->prefix + pxsz, pxremain, ".%s", prefix);
	if ((result < 0) || (result >= pxremain)) {
		return -1;
	}

	adi->prefix_level++;
	adi->prefix_levels[adi->prefix_level] = pxsz + result;

	return 0;
}

/*
 * ppe_acl_dump_prefix_index_add()
 *	Add another level (numeric) to the prefix
 */
int ppe_acl_dump_prefix_index_add(struct ppe_acl_dump_instance *adi, uint32_t index)
{
	int pxsz;
	int pxremain;
	int result;

	pxsz = adi->prefix_levels[adi->prefix_level];
	pxremain = PPE_ACL_DUMP_PREFIX_SIZE - pxsz;
	result = snprintf(adi->prefix + pxsz, pxremain, ".%u", index);
	if ((result < 0) || (result >= pxremain)) {
		return -1;
	}

	adi->prefix_level++;
	adi->prefix_levels[adi->prefix_level] = pxsz + result;

	return 0;
}

/*
 * ppe_acl_dump_prefix_remove()
 *	Remove level from the prefix
 */
int ppe_acl_dump_prefix_remove(struct ppe_acl_dump_instance *adi)
{
	int pxsz;

	adi->prefix_level--;
	pxsz = adi->prefix_levels[adi->prefix_level];
	adi->prefix[pxsz] = 0;

	return 0;
}

/*
 * ppe_acl_dump_write()
 *	Write out to the message buffer, prefix is added automatically.
 */
int ppe_acl_dump_write(struct ppe_acl_dump_instance *adi, char *name, char *fmt, ...)
{
	int remain;
	char *ptr;
	int result;
	va_list args;

	remain = PPE_ACL_DUMP_BUFFER_SIZE - adi->msg_len;
	ptr = adi->msg + adi->msg_len;
	result = snprintf(ptr, remain, "%s.%s=", adi->prefix, name);
	if ((result < 0) || (result >= remain)) {
		return -1;
	}

	adi->msg_len += result;
	remain -= result;
	ptr += result;

	va_start(args, fmt);
	result = vsnprintf(ptr, remain, fmt, args);
	va_end(args);
	if ((result < 0) || (result >= remain)) {
		return -2;
	}

	adi->msg_len += result;
	remain -= result;
	ptr += result;

	result = snprintf(ptr, remain, "\n");
	if ((result < 0) || (result >= remain)) {
		return -3;
	}

	adi->msg_len += result;
	return 0;
}

/*
 * ppe_acl_dump_one()
 *	Fill ACL dump information for one ACL rule.
 */
int ppe_acl_dump_one(struct ppe_acl_dump_instance *adi, struct ppe_acl *acl)
{
	int result;
	uint64_t pkts, bytes;
	ppe_acl_rule_src_type_t stype;
	struct ppe_acl_rule_match_one *r;
	ppe_acl_rule_match_type_t rule_t;
	enum ppe_drv_acl_slice_type slice_t;
	struct ppe_acl_rule_action *r_action;
	struct ppe_drv_acl_hw_info hw_info = {0};

	/*
	 * Current ACL count.
	 */
	if ((result = ppe_acl_dump_prefix_index_add(adi, adi->acl_cnt))) {
		goto error;
	}

	/*
	 * Rule information
	 */
	if ((result = ppe_acl_dump_prefix_add(adi, "rule"))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "user_id", "%d", acl->rule_id))) {
		goto error;
	}

	ppe_drv_acl_hw_info_get(acl->ctx, &hw_info);
	if ((result = ppe_acl_dump_write(adi, "hw_rule_id", "%d", hw_info.hw_rule_id))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "hw_list_id", "%d", hw_info.hw_list_id))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "hw_num_slices", "%d", hw_info.hw_num_slices))) {
		goto error;
	}

	if (acl->rule.cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_FLOW_QOS_OVERRIDE) {
		if ((result = ppe_acl_dump_write(adi, "flow_qos_override", "%s", "true"))) {
			goto error;
		}
	}

	if ((result = ppe_acl_dump_write(adi, "priority", "%d", acl->pri))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "ipo_type", "%s", ppe_acl_dump_ipo_type_str[acl->ipo]))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "src_type", "%s", ppe_acl_dump_src_type_str[acl->rule.stype]))) {
		goto error;
	}

	stype = acl->rule.stype;
	switch (stype) {
	case PPE_ACL_RULE_SRC_TYPE_DEV:
		if ((result = ppe_acl_dump_write(adi, "src_dev", "%s", acl->rule.src.dev_name))) {
			goto error;
		}

		break;

	case PPE_ACL_RULE_SRC_TYPE_SC:
		if ((result = ppe_acl_dump_write(adi, "src_sc", "%d", acl->rule.src.sc))) {
			goto error;
		}

		break;

	case PPE_ACL_RULE_SRC_TYPE_FLOW:
		if ((result = ppe_acl_dump_write(adi, "src_flow", "%d", "true"))) {
			goto error;
		}

		break;
	}

	if ((result = ppe_acl_dump_write(adi, "group_id", "%d", acl->info.cmn.res_chain))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "slice_cnt", "%d", acl->slice_cnt))) {
		goto error;
	}

	for (slice_t = 0; slice_t < PPE_DRV_ACL_SLICE_TYPE_MAX; slice_t++) {
		if (!acl->slice_type[slice_t]) {
			continue;
		}

		if ((result = ppe_acl_dump_write(adi, "slice_type", "%s",
						ppe_acl_dump_slice_type_str[slice_t]))) {
			goto error;
		}
	}

	for (rule_t = 0; rule_t < PPE_ACL_RULE_MATCH_TYPE_MAX; rule_t++) {
		if (!(acl->rule.valid_flags & (1 << rule_t))) {
			continue;
		}

		r = &acl->rule.rules[rule_t];

		switch (rule_t) {
		case PPE_ACL_RULE_MATCH_TYPE_SMAC:
			if ((result = ppe_acl_dump_prefix_add(adi, "smac"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "addr", "%pM", r->rule.smac.mac))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_MAC_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%pM",
								r->rule.smac.mac_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_DMAC:
			if ((result = ppe_acl_dump_prefix_add(adi, "dmac"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "addr", "%pM",
							r->rule.dmac.mac))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_MAC_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%pM",
							r->rule.dmac.mac_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_SVID:
			if ((result = ppe_acl_dump_prefix_add(adi, "svid"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "vid", "%d",
							r->rule.svid.vid_min))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_VID_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask_max", "%d",
							r->rule.svid.vid_mask_max))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_FLAG_SVID_RANGE)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_CVID:
			if ((result = ppe_acl_dump_prefix_add(adi, "cvid"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "vid", "%d",
							r->rule.cvid.vid_min))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_VID_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask_max", "%d",
							r->rule.cvid.vid_mask_max))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_FLAG_VID_RANGE)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_SPCP:
			if ((result = ppe_acl_dump_prefix_add(adi, "spcp"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "pcp", "%d",
							r->rule.spcp.pcp_mask))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_PCP_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.spcp.pcp_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_CPCP:
			if ((result = ppe_acl_dump_prefix_add(adi, "cpcp"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "pcp", "%d",
							r->rule.cpcp.pcp_mask))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_PCP_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.cpcp.pcp_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS:
			if ((result = ppe_acl_dump_prefix_add(adi, "pppoe"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "ssesion_id", "%d",
							r->rule.pppoe_sess.pppoe_session_id))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_PPPOE_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.pppoe_sess.pppoe_session_id_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_ETHER_TYPE:
			if ((result = ppe_acl_dump_prefix_add(adi, "ether_type"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "proto", "%d",
							r->rule.ether_type.l2_proto))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_ETHTYPE_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.ether_type.l2_proto_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_L3_1ST_FRAG:
			if ((result = ppe_acl_dump_prefix_add(adi, "1st_frag"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "1st_frag", "%s", "true"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_IP_LEN:
			if ((result = ppe_acl_dump_prefix_add(adi, "l3_len"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "len", "%d",
							r->rule.l3_len.l3_length_min))) {
				goto error;
			}

			if ((r->rule_flags & PPE_ACL_RULE_FLAG_IPLEN_MASK)
					|| (r->rule_flags & PPE_ACL_RULE_FLAG_IPLEN_RANGE)) {
				if ((result = ppe_acl_dump_write(adi, "mask_max", "%d",
								r->rule.l3_len.l3_length_mask_max))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_FLAG_IPLEN_RANGE)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_TTL_HOPLIMIT:
			if ((result = ppe_acl_dump_prefix_add(adi, "ttl_hl"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "val", "%d",
							r->rule.ttl_hop.hop_limit))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_TTL_HOPLIMIT_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
								r->rule.ttl_hop.hop_limit_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_DSCP_TC:
			if ((result = ppe_acl_dump_prefix_add(adi, "dscp_tc"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "val", "%d",
							r->rule.dscp_tc.l3_dscp_tc))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_DSCP_TC_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.dscp_tc.l3_dscp_tc_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR:
			if ((result = ppe_acl_dump_prefix_add(adi, "l4_proto"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "proto_nh", "%d",
							r->rule.proto_nexthdr.l3_v4proto_v6nexthdr))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_PROTO_NEXTHDR_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.proto_nexthdr.l3_v4proto_v6nexthdr_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_IP_GEN:
			if ((result = ppe_acl_dump_prefix_add(adi, "ip_misc"))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_L3_FRAG) {
				if ((result = ppe_acl_dump_write(adi, "l3_frag", "%s", "true"))) {
					goto error;
				}
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_ESP_HDR) {
				if ((result = ppe_acl_dump_write(adi, "esp_hdr", "%s", "true"))) {
					goto error;
				}
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_AH_HDR) {
				if ((result = ppe_acl_dump_write(adi, "ah_hdr", "%s", "true"))) {
					goto error;
				}
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_MOBILITY_HDR) {
				if ((result = ppe_acl_dump_write(adi, "mobility_hdr", "%s", "true"))) {
					goto error;
				}
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_OTHER_EXT_HDR) {
				if ((result = ppe_acl_dump_write(adi, "other_ext_hdr", "%s", "true"))) {
					goto error;
				}
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_FRAG_HDR) {
				if ((result = ppe_acl_dump_write(adi, "frag_hdr", "%s", "true"))) {
					goto error;
				}
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_IPV4_OPTION) {
				if ((result = ppe_acl_dump_write(adi, "ip_option", "%s", "true"))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_TCP_FLAG:
			if ((result = ppe_acl_dump_prefix_add(adi, "tcp_flag"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "flags", "%d",
							r->rule.tcp_flag.tcp_flags))) {
				goto error;
			}

			if (r->rule_flags & PPE_ACL_RULE_FLAG_TCP_FLG_MASK) {
				if ((result = ppe_acl_dump_write(adi, "mask", "%d",
							r->rule.tcp_flag.tcp_flags_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
						(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
						? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_SIP:
			if ((result = ppe_acl_dump_prefix_add(adi, "sip"))) {
				goto error;
			}

			if (r->rule.sip.ip_type == PPE_ACL_IP_TYPE_V4) {
				if ((result = ppe_acl_dump_write(adi, "v4", "%pI4",
								&r->rule.sip.ip[0]))) {
					goto error;
				}

				if (r->rule_flags & PPE_ACL_RULE_FLAG_SIP_MASK) {
					if ((result = ppe_acl_dump_write(adi, "mask", "%pI4",
								&r->rule.sip.ip_mask[0]))) {
						goto error;
					}
				}

				if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
								(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
								? "true": "false"))) {
					goto error;
				}
			} else if (r->rule.sip.ip_type == PPE_ACL_IP_TYPE_V6) {
				if ((result = ppe_acl_dump_write(adi, "v6", "%pI6",
								&r->rule.sip.ip[0]))) {
					goto error;
				}

				if (r->rule_flags & PPE_ACL_RULE_FLAG_SIP_MASK) {
					if ((result = ppe_acl_dump_write(adi, "mask", "%pI6",
								&r->rule.sip.ip_mask[0]))) {
						goto error;
					}
				}

				if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
								(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
								? "true": "false"))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_DIP:
			if ((result = ppe_acl_dump_prefix_add(adi, "dip"))) {
				goto error;
			}

			if (r->rule.dip.ip_type == PPE_ACL_IP_TYPE_V4) {
				if ((result = ppe_acl_dump_write(adi, "v4", "%pI4",
								&r->rule.dip.ip[0]))) {
					goto error;
				}

				if (r->rule_flags & PPE_ACL_RULE_FLAG_DIP_MASK) {
					if ((result = ppe_acl_dump_write(adi, "mask", "%pI4",
								&r->rule.dip.ip_mask[0]))) {
						goto error;
					}
				}

				if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
								(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
								? "true": "false"))) {
					goto error;
				}
			} else if (r->rule.dip.ip_type == PPE_ACL_IP_TYPE_V6) {
				if ((result = ppe_acl_dump_write(adi, "v6", "%pI6",
								&r->rule.dip.ip[0]))) {
					goto error;
				}

				if (r->rule_flags & PPE_ACL_RULE_FLAG_DIP_MASK) {
					if ((result = ppe_acl_dump_write(adi, "mask", "%pI6",
								&r->rule.dip.ip_mask[0]))) {
						goto error;
					}
				}

				if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
								(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
								? "true": "false"))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_SPORT:
			if ((result = ppe_acl_dump_prefix_add(adi, "sport"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "port", "%d",
							r->rule.sport.l4_port_min))) {
				goto error;
			}

			if (r->rule_flags & (PPE_ACL_RULE_FLAG_SPORT_MASK | PPE_ACL_RULE_FLAG_SPORT_RANGE)) {
				if ((result = ppe_acl_dump_write(adi, "mask_max", "%d",
								r->rule.sport.l4_port_max_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
							(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
							? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
							(r->rule_flags & PPE_ACL_RULE_FLAG_SPORT_RANGE)
							? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_DPORT:
			if ((result = ppe_acl_dump_prefix_add(adi, "dport"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "port", "%d",
							r->rule.dport.l4_port_min))) {
				goto error;
			}

			if (r->rule_flags & (PPE_ACL_RULE_FLAG_DPORT_MASK | PPE_ACL_RULE_FLAG_DPORT_RANGE)) {
				if ((result = ppe_acl_dump_write(adi, "mask_max", "%d",
								r->rule.dport.l4_port_max_mask))) {
					goto error;
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
							(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
							? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
							(r->rule_flags & PPE_ACL_RULE_FLAG_DPORT_RANGE)
							? "true": "false"))) {
				goto error;
			}

			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_UDF:
			if ((result = ppe_acl_dump_prefix_add(adi, "udf"))) {
				goto error;
			}

			if (r->rule.udf.udf_a_valid) {
				if ((result = ppe_acl_dump_write(adi, "udf_a", "%d",
								r->rule.udf.udf_a_min))) {
					goto error;
				}

				if (r->rule_flags & (PPE_ACL_RULE_FLAG_UDFA_MASK | PPE_ACL_RULE_FLAG_UDFA_RANGE)) {
					if ((result = ppe_acl_dump_write(adi, "udf_a_mask_max", "%d",
									r->rule.udf.udf_a_mask_max))) {
						goto error;
					}
				}

				if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
								(r->rule_flags & PPE_ACL_RULE_FLAG_UDFA_RANGE)
								? "true": "false"))) {
					goto error;
				}
			}

			if (r->rule.udf.udf_b_valid) {
				if ((result = ppe_acl_dump_write(adi, "udf_b", "%d",
								r->rule.udf.udf_b_min))) {
					goto error;
				}

				if (r->rule_flags & (PPE_ACL_RULE_FLAG_UDFB_MASK | PPE_ACL_RULE_FLAG_UDFB_RANGE)) {
					if ((result = ppe_acl_dump_write(adi, "udf_b_mask_max", "%d",
									r->rule.udf.udf_b_mask_max))) {
						goto error;
					}
				}

				if ((result = ppe_acl_dump_write(adi, "range_en", "%s",
								(r->rule_flags & PPE_ACL_RULE_FLAG_UDFB_RANGE)
								? "true": "false"))) {
					goto error;
				}
			}

			if (r->rule.udf.udf_c_valid) {
				if ((result = ppe_acl_dump_write(adi, "udf_c", "%d",
								r->rule.udf.udf_c))) {
					goto error;
				}

				if (r->rule_flags & PPE_ACL_RULE_FLAG_UDFC_MASK) {
					if ((result = ppe_acl_dump_write(adi, "udf_c_mask", "%d",
									r->rule.udf.udf_c_mask))) {
						goto error;
					}
				}
			}

			if (r->rule.udf.udf_d_valid) {
				if ((result = ppe_acl_dump_write(adi, "udf_d", "%d",
								r->rule.udf.udf_d))) {
					goto error;
				}

				if (r->rule_flags & PPE_ACL_RULE_FLAG_UDFD_MASK) {
					if ((result = ppe_acl_dump_write(adi, "udf_d_mask", "%d",
									r->rule.udf.udf_d_mask))) {
						goto error;
					}
				}
			}

			if ((result = ppe_acl_dump_write(adi, "inverse_en", "%s",
							(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN)
							? "true": "false"))) {
				goto error;
			}


			if ((result = ppe_acl_dump_prefix_remove(adi))) {
				goto error;
			}

			break;

		case PPE_ACL_RULE_MATCH_TYPE_DEFAULT:
			if ((result = ppe_acl_dump_write(adi, "default_rule", "%s", "true"))) {
				goto error;
			}

			break;

		default:
			ppe_acl_warn("%p: invalid rule type: %d", r, rule_t);
			break;
		}
	}

	/*
	 * Remove 'rule' from action.
	 */
	if ((result = ppe_acl_dump_prefix_remove(adi))) {
		goto error;
	}

	/*
	 * Action information.
	 */
	r_action = &acl->rule.action;
	if ((result = ppe_acl_dump_prefix_add(adi, "action"))) {
		goto error;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_SERVICE_CODE_EN) {
		if ((result = ppe_acl_dump_write(adi, "service_code", "%d", r_action->service_code))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_QID_EN) {
		if ((result = ppe_acl_dump_write(adi, "qid", "%d", r_action->qid))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_ENQUEUE_PRI_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "enqueue_pri", "%d", r_action->enqueue_pri))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CTAG_DEI_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "ctag_dei", "%s", "true"))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CTAG_PCP_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "ctag_pcp", "%d", r_action->ctag_pcp))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_STAG_DEI_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "stag_dei", "%s", "true"))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_STAG_PCP_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "stag_pcp", "%d", r_action->stag_pcp))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_DSCP_TC_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "dscp_tc", "%d", r_action->dscp_tc))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CVID_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "cvid", "%d", r_action->cvid))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_SVID_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "svid", "%d", r_action->svid))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_DEST_INFO_CHANGE_EN) {
		if ((result = ppe_acl_dump_write(adi, "dest_dev", "%s", r_action->dst.dev_name))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_MIRROR_EN) {
		if ((result = ppe_acl_dump_write(adi, "mirror", "%s", "true"))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CTAG_FMT_TAGGED) {
		if ((result = ppe_acl_dump_write(adi, "ctag_fmt", "%s", "true"))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_STAG_FMT_TAGGED) {
		if ((result = ppe_acl_dump_write(adi, "stag_fmt", "%s", "true"))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_REDIR_TO_CORE_EN) {
		if ((result = ppe_acl_dump_write(adi, "redir_core", "%d", r_action->redir_core))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_POLICER_EN) {
		if ((result = ppe_acl_dump_write(adi, "policer_id", "%d", r_action->policer_id))) {
			goto error;
		}
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_FW_CMD) {
		if ((result = ppe_acl_dump_write(adi, "fwd_cmd", "%d", r_action->fwd_cmd))) {
			goto error;
		}
	}

	/*
	 * Remove the 'action' prefix for next interation
	 */
	if ((result = ppe_acl_dump_prefix_remove(adi))) {
		goto error;
	}

	/*
	 * Match counters
	 */
	ppe_drv_acl_get_hw_stats(acl->ctx, &pkts, &bytes);

	if ((result = ppe_acl_dump_prefix_add(adi, "stats"))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "pkts", "%llu", pkts))) {
		goto error;
	}

	if ((result = ppe_acl_dump_write(adi, "bytes", "%llu", bytes))) {
		goto error;
	}

	/*
	 * Remove the 'stats' prefix for next interation
	 */
	if ((result = ppe_acl_dump_prefix_remove(adi))) {
		goto error;
	}

	/*
	 * Remove the index prefix for next interation
	 */
	if ((result = ppe_acl_dump_prefix_remove(adi))) {
		goto error;
	}

error:
	return result;
}

/*
 * ppe_acl_dump_all()
 *	Prepare acl dump information for all the active ACL rules.
 */
static bool ppe_acl_dump_all(struct ppe_acl_dump_instance *adi)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *acl;
	int result;

	if ((result = ppe_acl_dump_write_reset(adi, "acl"))) {
		return result;
	}

	/*
	 * Get the first available acl rule ID.
	 */
	spin_lock_bh(&acl_g->lock);
	if (!list_empty(&acl_g->active_rules)) {
		adi->acl_cnt = 0;
		list_for_each_entry(acl, &acl_g->active_rules, list) {
			result = ppe_acl_dump_one(adi, acl);
			if (result < 0) {
				ppe_acl_warn("%p: failed to collect dump for acl: %p", acl_g, acl);
				spin_unlock_bh(&acl_g->lock);
				return result;
			}

			adi->acl_cnt++;
		}
	}

	spin_unlock_bh(&acl_g->lock);

	return 0;
}

/*
 * ppe_acl_dump_dev_open()
 *	Open the character device file for ACL dump.
 */
static int ppe_acl_dump_dev_open(struct inode *inode, struct file *file)
{
	struct ppe_acl_dump_instance *adi;
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	/*
	 * Allocate state information for the reading
	 */
	ppe_acl_assert(file->private_data == NULL, "unexpected double open: %px?\n", file->private_data);

	adi = (struct ppe_acl_dump_instance *)kzalloc(sizeof(struct ppe_acl_dump_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!adi) {
		ppe_acl_warn("%p: unable to allocate memory for dump instance", acl_g);
		return -ENOMEM;
	}

	adi->dump_en = true;
	file->private_data = adi;

	return 0;
}

/*
 * ppe_acl_dump_dev_release()
 *	Close the character device file for ACL dump.
 */
static int ppe_acl_dump_dev_release(struct inode *inode, struct file *file)
{
	struct ppe_acl_dump_instance *adi = (struct ppe_acl_dump_instance *)file->private_data;
	if (adi) {
		kfree(adi);
	}

	return 0;
}

/*
 * ppe_acl_dump_dev_read()
 *	Read file operation handler
 */
static ssize_t ppe_acl_dump_dev_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	int bytes_read = 0;
	struct ppe_acl_dump_instance *adi;
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	adi = (struct ppe_acl_dump_instance *)file->private_data;
	if (!adi) {
		ppe_acl_warn("%p: unable to find dump instance", acl_g);
		return -ENOMEM;
	}

	/*
	 * If there is still some message remaining to be output then complete that first
	 */
	if (adi->msg_len) {
		goto read_output;
	}

	if (adi->dump_en) {
		if (ppe_acl_dump_all(adi)) {
			ppe_acl_warn("Failed to create acl dump\n");
			return -EIO;
		}

		adi->dump_en = false;
		goto read_output;
	}

	return 0;

read_output:
	/*
	 * If supplied buffer is small we limit what we output
	 */
	bytes_read = adi->msg_len;
	if (bytes_read > length) {
		bytes_read = length;
	}

	if (copy_to_user(buffer, adi->msgp, bytes_read)) {
		return -EIO;
	}

	adi->msg_len -= bytes_read;
	adi->msgp += bytes_read;

	ppe_acl_trace("%p: state read done, bytes_read: %d bytes, remaining msg_len: %d\n",
			adi, bytes_read, adi->msg_len);

	/*
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*
 * ppe_acl_dump_dev_write()
 *	Write file operation handler
 */
static ssize_t ppe_acl_dump_dev_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	/*
	 * Not supported.
	 */
	return -EINVAL;
}

/*
 * File operations associated with acl dump character device
 */
static struct file_operations ppe_acl_dump_fops = {
	.read = ppe_acl_dump_dev_read,
	.write = ppe_acl_dump_dev_write,
	.open = ppe_acl_dump_dev_open,
	.release = ppe_acl_dump_dev_release
};

/*
 * ppe_acl_dump_exit()
 *	Unregister character device for ACL dump
 */
void ppe_acl_dump_exit(void)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	unregister_chrdev(acl_g->acl_dump_major_id, "ppe_acl_dump_dev");
}

/*
 * ppe_acl_dump_init()
 *	Register a character device for ACL dump
 */
bool ppe_acl_dump_init(struct dentry *dentry)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	int dev_id = -1;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	if (!debugfs_create_u32("ppe_acl_dump", S_IRUGO, acl_g->dentry, (u32 *)&acl_g->acl_dump_major_id)) {
		ppe_acl_warn("%p: Failed to create ppe state dev major file in debugfs\n", acl_g);
		return false;
	}
#else
	debugfs_create_u32("ppe_acl_dump", S_IRUGO, acl_g->dentry, (u32 *)&acl_g->acl_dump_major_id);
#endif

	/*
	 * Register a character device to dump the output
	 */
	dev_id = register_chrdev(0, "ppe_acl_dump_dev", &ppe_acl_dump_fops);
	if (dev_id < 0) {
		ppe_acl_warn("%p: Failed to register chrdev %d\n", acl_g, dev_id);
		return false;
	}

	acl_g->acl_dump_major_id = dev_id;
	ppe_acl_trace("%p: acl dump dev major id %d\n", acl_g, acl_g->acl_dump_major_id);

	return true;
}
