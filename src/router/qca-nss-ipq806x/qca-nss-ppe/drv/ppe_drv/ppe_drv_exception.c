/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <fal/fal_sec.h>
#include "ppe_drv.h"

/*
 * ppe_drv_exception_list
 *        PPE exception list to be enabled.
 *
 * Note: Exception list is defined using CPU codes.
 */
struct ppe_drv_exception ppe_drv_exception_list[] = {
	{
		PPE_DRV_CC_UNKNOWN_L2_PROT,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_PPPOE_WRONG_VER_TYPE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_PPPOE_WRONG_CODE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_PPPOE_UNSUPPORTED_PPP_PROT,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_SMALL_IHL,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_WITH_OPTION,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_BAD_TOTAL_LEN,
#ifdef PPE_V4_BAD_LEN_EXCEP_DIS
		FAL_MAC_DROP,
		PPE_DRV_EXCEPTION_DEACCEL_DIS,
#else
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
#endif
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_DATA_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_FRAG,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_SMALL_TTL,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_CHECKSUM_ERR,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV4_ESP_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV6_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV6_BAD_PAYLOAD_LEN,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV6_DATA_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV6_SMALL_HOP_LIMIT,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV6_FRAG,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_IPV6_ESP_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_TCP_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_TCP_SMALL_DATA_OFFSET,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_TCP_FLAGS_0,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_TCP_FLAGS_1,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_TCP_FLAGS_2,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_UDP_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
	{
		PPE_DRV_CC_UDP_LITE_HDR_INCOMPLETE,
		FAL_MAC_RDT_TO_CPU,
		PPE_DRV_EXCEPTION_DEACCEL_EN,
		PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT
		| PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT
	},
};

/*
 * ppe_drv_exception_tcpflag_list
 *	TCP Flags to be enabled.
 */
static struct ppe_drv_exception_tcpflag ppe_drv_exception_tcpflag_list[] = {
	{
		PPE_DRV_TCP_FLAG_FIN
	},
	{
		PPE_DRV_TCP_FLAG_SYN
	},
	{
		PPE_DRV_TCP_FLAG_RST
	},
};

/*
 * ppe_drv_exception_max()
 *	Returns number of exception enabled in hardware.
 */
const uint8_t ppe_drv_exception_max()
{
	return sizeof(ppe_drv_exception_list) / sizeof(struct ppe_drv_exception);
}

/*
 * ppe_drv_exception_tcpflag_max()
 *	Returns number of tcp flag exception enabled in hardware.
 */
const uint8_t ppe_drv_exception_tcpflag_max()
{
	return sizeof(ppe_drv_exception_tcpflag_list) / sizeof(struct ppe_drv_exception_tcpflag);
}

/*
 * ppe_drv_exception_cc2exp()
 *	CPU codes to Exception codes
 */
static inline uint8_t ppe_drv_exception_cc2exp(uint8_t cpu_code)
{
	if (cpu_code <= PPE_DRV_CC_RANGE1) {
		return (cpu_code - PPE_DRV_EXP_RANGE1_BASE);
	} else if (cpu_code >= PPE_DRV_CC_RANGE2 && cpu_code <= PPE_DRV_CC_RANGE3) {
		return (cpu_code - PPE_DRV_EXP_RANGE2_BASE);
	}

	return cpu_code;
}

/*
 * ppe_drv_exception_init()
 *	Initialize PPE exceptions
 */
void ppe_drv_exception_init(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	const uint8_t exception_max = ppe_drv_exception_max();
	const uint8_t l4_except_max = ppe_drv_exception_tcpflag_max();
	fal_l4_excep_parser_ctrl tcp_except_ctrl = {0};
	struct ppe_drv_exception_tcpflag *tcpflag;
	fal_l3_excep_ctrl_t except_ctrl = {0};
	sw_error_t err;
	uint32_t i;
	uint8_t exp_code;

	/*
	 * Traverse through exception list and configure each exception
	 */
	for (i = 0; i < exception_max; i++) {
		struct ppe_drv_exception *pe = &ppe_drv_exception_list[i];

		/*
		 * Since our exception list is now defined using CPU code, we need to
		 * Convert CPU code into exception code for exception configuration.
		 * l3_excep_ctrl table only configures exceptions till number 71, hence
		 * We need to do necessary check to avoid configuring exception
		 * Table above 71.
		 */
		exp_code = ppe_drv_exception_cc2exp(pe->code);
		if (exp_code > ppe_drv_exception_cc2exp(PPE_DRV_CC_UDP_LITE_CHECKSUM_ERR) + 4) {
			ppe_drv_trace("%p: exception code greater than 71 not configured in l3 exception control table: %d\n", p, exp_code);
			continue;
		}

		ppe_drv_trace("%p: configuring exception code: %u flow_type: 0x%x",
				p, exp_code, pe->flow_type);

		/*
		 * Enable Exception
		 */
		except_ctrl.deacclr_en = pe->deaccel_en;
		except_ctrl.cmd = pe->action;

		/*
		 * L2_Only - bridge flow, with flow disable or bypassed by sc.
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_L2_ONLY)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_L2_ONLY) {
			except_ctrl.l2fwd_only_en = A_TRUE;
			except_ctrl.l2flow_type = FAL_FLOW_AWARE;
		}

		/*
		 * L3_Only - routed flow, with flow disable or bypassed by sc.
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_L3_ONLY)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_L3_ONLY) {
			except_ctrl.l3route_only_en = A_TRUE;
			except_ctrl.l3flow_type = FAL_FLOW_AWARE;
		}

		/*
		 * L2_Flow - bridge flow with flow enabled.
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW) {
			except_ctrl.l2flow_en = A_TRUE;
			except_ctrl.l2flow_type = FAL_FLOW_AWARE;
		}

		/*
		 * L3_Flow - routed flow with flow enabled.
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW) {
			except_ctrl.l3flow_en = A_TRUE;
			except_ctrl.l3flow_type = FAL_FLOW_AWARE;
		}

		/*
		 * Multicast flows
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_MULTICAST)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_MULTICAST) {
			except_ctrl.multicast_en = A_TRUE;
			except_ctrl.l2flow_type = FAL_FLOW_AWARE;
		}

		/*
		 * L2_FLOW_HIT - bridged flow with flow match.
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_L2_FLOW_HIT) {
			except_ctrl.l2flow_en = A_TRUE;
			except_ctrl.l2flow_type = FAL_FLOW_HIT;
		}

		/*
		 * L3_FLOW_HIT - routed flow with flow match.
		 */
		if ((pe->flow_type & PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT)
				== PPE_DRV_EXCEPTION_FLOW_TYPE_L3_FLOW_HIT) {
			except_ctrl.l3flow_en = A_TRUE;
			except_ctrl.l3flow_type = FAL_FLOW_HIT;
		}

		/*
		 * TODO: Initialize tunnel exception here while adding support for tunnel.
		 */

		/*
		 * Configure specific exception in PPE through SSDK.
		 */
		err = fal_sec_l3_excep_ctrl_set(PPE_DRV_SWITCH_ID, exp_code, &except_ctrl);
		if (err != SW_OK) {
			ppe_drv_warn("%p: failed to configure L3 exception: %d", p, exp_code);
		}
	}

	/*
	 * TCP_FLAG_* are special cases, need to set ctrl and mask register.
	 *
	 * Note: We use independent exceptions for each TCP flags - this
	 * allow us to get a unique CPU code for each TCP flag and help us
	 * provide an explicit reason for exception.
	 */
	for (i = 0; i < l4_except_max; i++) {
		tcpflag = &ppe_drv_exception_tcpflag_list[i];
		tcp_except_ctrl.tcp_flags[i] =  tcpflag->flags;
		tcp_except_ctrl.tcp_flags_mask[i] = tcpflag->flags;

	}

	err = fal_sec_l4_excep_parser_ctrl_set(PPE_DRV_SWITCH_ID, &tcp_except_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure L4 exception: %p", p, &tcp_except_ctrl);
	}
}
