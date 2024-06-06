/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup
 * @{
 */
#include <linux/switch.h>
#include "sw.h"
#include "ref_uci.h"

static const char *tunnel_intf[] = {
	"intf_id",
	"ipv4_decap_en",
	"ipv6_decap_en",
	"dmac_check_en",
	"ttl_exceed_deacce_en",
	"ttl_exceed_action",
	"lpm_en",
	"mini_ipv6_mtu",
};

static const char *tunnel_encaprule[] = {
	"rule_id",
	"src1_sel",
	"src1_start",
	"src2_sel",
	"src2_en0",
	"src2_start0",
	"src2_width0",
	"dest2_pos0",
	"src2_en1",
	"src2_start1",
	"src2_width1",
	"dest2_pos1",
	"src3_sel",
	"src3_en0",
	"src3_start0",
	"src3_width0",
	"dest3_pos0",
	"src3_en1",
	"src3_start1",
	"src3_width1",
	"dest3_pos1",
};

static const char *tunnel_encapintftunnelid[] = {
	"intf_id",
	"tunnel_id_en",
	"tunnel_id",
};

#ifndef IN_TUNNEL_MINI
static const char *tunnel_vlanintf[] = {
	"port",
	"svlan_en",
	"svlan_fmt",
	"svlan_id",
	"cvlan_en",
	"cvlan_fmt",
	"cvlan_id",
	"pppoe_en",
	"tl_l3if_en",
	"tl_l3if",
};
#endif

static const char *tunnel_encapporttunnelid[] = {
	"port_id",
	"tunnel_id_en",
	"tunnel_id",
};

static const char *tunnel_decapentry[] = {
	"mode",
	"tunnel_type",
	"entry_id",
	"ip_ver",
	"sip_addr",
	"dip_addr",
	"l4_proto",
	"sport",
	"dport",
	"key_tlinfo_en",
	"tunnel_info",
	"key_udf0_en",
	"udf0",
	"key_udf1_en",
	"udf1",
	"decap_en",
	"fwd_cmd",
	"svlan_check",
	"svlan_fmt",
	"svlan_id",
	"cvlan_check",
	"cvlan_fmt",
	"cvlan_id",
	"tl_l3if_check",
	"tl_l3if",
	"deacce_en",
	"udp_csum_zero",
	"exp_profile_id",
	"service_code_en",
	"service_code",
	"src_info_en",
	"src_info_type",
	"src_info",
	"spcp_mode",
	"sdei_mode",
	"cpcp_mode",
	"cdei_mode",
	"ttl_mode",
	"dscp_mode",
	"ecn_mode",
};

static const char *tunnel_encapentry[] = {
	"tunnel_id",
	"encap_type",
	"ip_ver",
	"encap_target",
	"payload_inner_type",
	"edit_rule_id",
	"tunnel_len",
	"tunnel_offset",
	"vlan_offset",
	"l3_offset",
	"l4_offset",
	"svlan_fmt",
	"spcp_mode",
	"sdei_mode",
	"cvlan_fmt",
	"cpcp_mode",
	"cdei_mode",
	"dscp_mode",
	"ttl_mode",
	"ecn_mode",
	"ip_proto_update",
	"ipv4_df_mode",
	"ipv4_id_mode",
	"ipv6_flowlable_mode",
	"sport_entry_en",
	"l4_proto",
	"l4_checksum_en",
	"vni_mode",
	"pppoe_en",
	"vport_en",
	"cpu_vport",
#if defined(MPPE)
	"mapt_udp_csm0_keep",
#endif
	"eg_header_data",
};

static const char *tunnel_globalcfg[] = {
	"deacce_action",
	"src_if_check_deacce_en",
	"src_if_check_action",
	"vlan_check_deacce_en",
	"vlan_check_action",
	"udp_csum_zero_deacce_en",
	"udp_csum_zero_action",
	"pppoe_multicast_deacce_en",
	"pppoe_multicast_action",
	"hash_mode0",
	"hash_mode1",
};

static const char *tunnel_portintf[] = {
	"port_id",
	"tunnel_id_en",
	"tunnel_id",
	"dmac_addr",
	"pppoe_en",
	"pppoe_group_id",
	"vlan_group_id",
};

static const char *tunnel_encapheaderctrl[] = {
	"ipv4_id_seed",
	"ipv4_df_set",
	"udp_sport_base",
	"udp_sport_mask",
	"ipv4_addr_map_data",
	"ipv4_proto_map_data",
	"ipv6_addr_map_data",
	"ipv6_proto_map_data",
};

#ifndef IN_TUNNEL_MINI
static const char *tunnel_decapecn[] = {
	"ecn_mode",
	"outer_ecn",
	"inner_ecn",
	"ecn_value",
	"ecn_exp_en",
};

static const char *tunnel_encapecn[] = {
	"ecn_mode",
	"inner_ecn",
	"ecn_val",
};
#endif

static const char *tunnel_decapexpfmtctrl[] = {
	"port_id",
	"decap_exp_fmt_ctrl_en",
};

static const char *tunnel_decapkey[] = {
	"tunnel_type",
	"key_sip_en",
	"key_dip_en",
	"key_l4proto_en",
	"key_sport_en",
	"key_dport_en",
	"key_tlinfo_en",
	"tunnel_info_mask",
	"key_udf0_en",
	"udf0_idx",
	"udf0_mask",
	"key_udf1_en",
	"udf1_idx",
	"udf1_mask",
};

int parse_tunnel(a_uint32_t dev_id, const char *command_name, struct switch_val *val)
{
	int rv = -1;

	if (!strcmp(command_name, "Intf")) {
		rv = parse_uci_option(val, tunnel_intf,
				ARRAY_SIZE(tunnel_intf));
	} else if (!strcmp(command_name, "Encaprule")) {
		rv = parse_uci_option(val, tunnel_encaprule,
				ARRAY_SIZE(tunnel_encaprule));
	} else if (!strcmp(command_name, "Encapintftunnelid")) {
		rv = parse_uci_option(val, tunnel_encapintftunnelid,
				ARRAY_SIZE(tunnel_encapintftunnelid));
#ifndef IN_TUNNEL_MINI
	} else if (!strcmp(command_name, "Vlanintf")) {
		rv = parse_uci_option(val, tunnel_vlanintf,
				ARRAY_SIZE(tunnel_vlanintf));
#endif
	} else if (!strcmp(command_name, "UdfprofileEntry")) {
		rv = parse_tunnel_udfprofileentry(dev_id, val);
	} else if (!strcmp(command_name, "UdfprofileCfg")) {
		rv = parse_tunnel_udfprofilecfg(val);
	} else if (!strcmp(command_name, "Encapporttunnelid")) {
		rv = parse_uci_option(val, tunnel_encapporttunnelid,
				ARRAY_SIZE(tunnel_encapporttunnelid));
	} else if (!strcmp(command_name, "Decapentry")) {
		rv = parse_uci_option(val, tunnel_decapentry,
				ARRAY_SIZE(tunnel_decapentry));
	} else if (!strcmp(command_name, "Encapentry")) {
		rv = parse_uci_option(val, tunnel_encapentry,
				ARRAY_SIZE(tunnel_encapentry));
	} else if (!strcmp(command_name, "Globalcfg")) {
		rv = parse_uci_option(val, tunnel_globalcfg,
				ARRAY_SIZE(tunnel_globalcfg));
	} else if (!strcmp(command_name, "Portintf")) {
		rv = parse_uci_option(val, tunnel_portintf,
				ARRAY_SIZE(tunnel_portintf));
	} else if (!strcmp(command_name, "Encapheaderctrl")) {
		rv = parse_uci_option(val, tunnel_encapheaderctrl,
				ARRAY_SIZE(tunnel_encapheaderctrl));
#ifndef IN_TUNNEL_MINI
	} else if (!strcmp(command_name, "Decapecn")) {
		rv = parse_uci_option(val, tunnel_decapecn,
				ARRAY_SIZE(tunnel_decapecn));
	} else if (!strcmp(command_name, "Encapecn")) {
		rv = parse_uci_option(val, tunnel_encapecn,
				ARRAY_SIZE(tunnel_encapecn));
#endif
	} else if (!strcmp(command_name, "Decapexpfmtctrl")) {
		rv = parse_uci_option(val, tunnel_decapexpfmtctrl,
				ARRAY_SIZE(tunnel_decapexpfmtctrl));
	} else if (!strcmp(command_name, "Decapkey")) {
		rv = parse_uci_option(val, tunnel_decapkey,
				ARRAY_SIZE(tunnel_decapkey));
	}

	return rv;
}

/**
 * @}
 */
