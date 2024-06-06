/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup mht_init MHT_INIT
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_phy.h"
#include "hsl_port_prop.h"
#include "isisc_init.h"
#include "isisc_acl.h"
#include "isisc_ip.h"
#include "isisc_nat.h"
#include "isisc_reg.h"
#include "mht_reg.h"
#include "mht_init.h"
#include "mht_sec_ctrl.h"
#include "ssdk_dts.h"
#include "ssdk_mht_clk.h"

static sw_error_t
_mht_reset(a_uint32_t dev_id)
{
	sw_error_t rv;
	HSL_DEV_ID_CHECK(dev_id);

	ssdk_mht_clk_assert(dev_id, MHT_SWITCH_CORE_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC0_TX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC0_RX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC1_TX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC1_RX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC2_TX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC2_RX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC3_TX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC3_RX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC4_TX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC4_RX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC5_TX_CLK);
	ssdk_mht_clk_assert(dev_id, MHT_MAC5_RX_CLK);
	udelay(10);
	ssdk_mht_clk_deassert(dev_id, MHT_SWITCH_CORE_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC0_TX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC0_RX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC1_TX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC1_RX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC2_TX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC2_RX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC3_TX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC3_RX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC4_TX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC4_RX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC5_TX_CLK);
	ssdk_mht_clk_deassert(dev_id, MHT_MAC5_RX_CLK);
	/* Need to wait 50us to complete switch initialization. */
	udelay(50);

	ISISC_ACL_RESET(rv, dev_id);
	ISISC_IP_RESET(rv, dev_id);
	ISISC_NAT_RESET(rv, dev_id);

	return SW_OK;
}

sw_error_t
mht_reset(a_uint32_t dev_id)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_reset(dev_id);
	HSL_API_UNLOCK;

	return rv;
}

sw_error_t
mht_init(a_uint32_t dev_id, ssdk_init_cfg *cfg)
{
	sw_error_t rv;
	hsl_api_t *p_api;

	rv = isisc_init(dev_id, cfg);
	SW_RTN_ON_ERROR(rv);

	p_api = hsl_api_ptr_get(dev_id);
	SW_RTN_ON_NULL(p_api);

	p_api->dev_reset = mht_reset;

	return rv;
}

/**
 * @}
 */
