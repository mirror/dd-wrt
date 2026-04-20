/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * @defgroup
 * @{
 */
#include <linux/switch.h>
#include "sw.h"
#include "ref_uci.h"

static const char *mapt_decapctrl[] = {
	"src_check_action",
	"dst_check_action",
	"no_tcp_udp_action",
	"udp_csum_zero_action",
	"ipv4_df_set",
};

static const char *mapt_Decapruleentry[] = {
	"rule_id",
	"ipv4_prefix",
	"ipv6_addr_type",
	"suffix_start",
	"suffix_width",
	"suffix_pos",
	"psid1_en",
	"psid1_start",
	"psid1_width",
	"proto_type",
	"psid2_en",
	"psid2_start",
	"psid2_width",
	"psid_check_en",
};

static const char *mapt_Decapentry[] = {
	"dst_is_local",
	"ipv6_addr",
	"prefix_len",
	"svlan_check",
	"svlan_fmt",
	"svlan_id",
	"cvlan_check",
	"cvlan_fmt",
	"cvlan_id",
	"tl_l3if_check",
	"tl_l3if",
	"src_info_en",
	"src_info_type",
	"src_info",
	"edit_rule_id",
	"exp_profile",
#if defined(MPPE)
	"service_code_en",
	"service_code",
#endif
};

int parse_mapt(const char *command_name, struct switch_val *val)
{
	int rv = -1;

	if (!strcmp(command_name, "Decapctrl")) {
		rv = parse_uci_option(val, mapt_decapctrl,
				sizeof(mapt_decapctrl)/sizeof(char *));
	} else if (!strcmp(command_name, "Decapruleentry")) {
		rv = parse_uci_option(val, mapt_Decapruleentry,
				sizeof(mapt_Decapruleentry)/sizeof(char *));
	} else if (!strcmp(command_name, "Decapentry")) {
		rv = parse_uci_option(val, mapt_Decapentry,
				sizeof(mapt_Decapentry)/sizeof(char *));
	}

	return rv;
}

/**
 * @}
 */
