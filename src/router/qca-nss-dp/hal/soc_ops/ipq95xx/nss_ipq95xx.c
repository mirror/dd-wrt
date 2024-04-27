/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/of.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <nss_dp_arch.h>
#include "nss_dp_hal.h"

/*
 * nss_dp_hal_nsm_sawf_sc_stats_read()
 *	Send nsm stats for the given service-class.
 */
bool nss_dp_hal_nsm_sawf_sc_stats_read(struct nss_dp_hal_nsm_sawf_sc_stats *nsm_stats, uint8_t service_class)
{
	return edma_nsm_sawf_sc_stats_read(nsm_stats, service_class);
}

/*
 * nss_dp_hal_get_data_plane_ops()
 *	Return the data plane ops for registered data plane.
 */
struct nss_dp_data_plane_ops *nss_dp_hal_get_data_plane_ops(void)
{
	return &nss_dp_edma_ops;
}

/*
 * nss_dp_hal_clock_set_and_enable()
 *	API to set and enable the EDMA common clocks
 */
int32_t nss_dp_hal_clock_set_and_enable(struct device *dev, const char *id, unsigned long rate)
{
	struct clk *clk = NULL;
	int err;

	clk = devm_clk_get(dev, id);
	if (IS_ERR(clk)) {
		return -1;
	}

	if (rate) {
		err = clk_set_rate(clk, rate);
		if (err) {
			return -1;
		}
	}

	err = clk_prepare_enable(clk);
	if (err) {
		return -1;
	}

	return 0;
}

/*
 * nss_dp_hal_configure_clocks()
 *	configure the EDMA clock's.
 */
int32_t nss_dp_hal_configure_clocks(void *ctx)
{
	struct platform_device *pdev = (struct platform_device *)ctx;
	int32_t err;

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_CSR_CLK, NSS_DP_EDMA_CSR_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_CSR_CLK, NSS_DP_EDMA_NSSNOC_CSR_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_TS_CLK, NSS_DP_EDMA_TS_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSCC_CLK, NSS_DP_EDMA_NSSCC_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSCFG_CLK, NSS_DP_EDMA_NSSCFG_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSCNOC_ATB_CLK,
					NSS_DP_EDMA_NSSCNOC_ATB_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_NSSCC_CLK,
					NSS_DP_EDMA_NSSNOC_NSSCC_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_PCNOC_1_CLK,
					NSS_DP_EDMA_NSSNOC_PCNOC_1_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_QOSGEN_REF_CLK,
					NSS_DP_EDMA_NSSNOC_QOSGEN_REF_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_SNOC_1_CLK,
					NSS_DP_EDMA_NSSNOC_SNOC_1_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_SNOC_CLK,
					NSS_DP_EDMA_NSSNOC_SNOC_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_TIMEOUT_REF_CLK,
					NSS_DP_EDMA_NSSNOC_TIMEOUT_REF_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_XO_DCD_CLK,
					NSS_DP_EDMA_NSSNOC_XO_DCD_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_IMEM_QSB_CLK,
					NSS_DP_EDMA_IMEM_QSB_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_IMEM_QSB_CLK,
					NSS_DP_EDMA_NSSNOC_IMEM_QSB_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_IMEM_AHB_CLK,
					NSS_DP_EDMA_IMEM_AHB_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_IMEM_AHB_CLK,
					NSS_DP_EDMA_NSSNOC_IMEM_AHB_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_MEM_NOC_NSSNOC_CLK,
					NSS_DP_EDMA_MEM_NOC_NSSNOC_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_TBU_CLK,
					NSS_DP_EDMA_TBU_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_MEM_NOC_1_CLK,
					NSS_DP_EDMA_NSSNOC_MEM_NOC_1_CLK_FREQ);
	if (err) {
		return -1;
	}

	err = nss_dp_hal_clock_set_and_enable(&pdev->dev, NSS_DP_EDMA_NSSNOC_MEMNOC_CLK,
					NSS_DP_EDMA_NSSNOC_MEMNOC_CLK_FREQ);
	if (err) {
		return -1;
	}

	return 0;
}

/*
 * nss_dp_hal_hw_reset()
 *	Reset EDMA hardware.
 */
int32_t nss_dp_hal_hw_reset(void *ctx)
{
	struct reset_control *edma_hw_rst;
	struct platform_device *pdev = (struct platform_device *)ctx;

	edma_hw_rst = devm_reset_control_get(&pdev->dev, EDMA_HW_RESET_ID);
	if (IS_ERR(edma_hw_rst)) {
		return -EINVAL;
	}

	reset_control_assert(edma_hw_rst);
	udelay(100);

	reset_control_deassert(edma_hw_rst);
	udelay(100);

	return 0;
}

/*
 * nss_dp_hal_init()
 *	Initialize EDMA and set gmac ops.
 */
bool nss_dp_hal_init(void)
{
	/*
	 * Bail out on not supported platform
	 */
	if (!of_machine_is_compatible("qcom,ipq9574")) {
		return false;
	}

	if (edma_init()) {
		return false;
	}

	nss_dp_hal_set_gmac_ops(&qcom_gmac_ops, GMAC_HAL_TYPE_QCOM);
	nss_dp_hal_set_gmac_ops(&syn_gmac_ops, GMAC_HAL_TYPE_SYN_XGMAC);

	return true;
}

/*
 * nss_dp_hal_cleanup()
 *	Cleanup EDMA and set gmac ops to NULL.
 */
void nss_dp_hal_cleanup(void)
{
	nss_dp_hal_set_gmac_ops(NULL, GMAC_HAL_TYPE_QCOM);
	nss_dp_hal_set_gmac_ops(NULL, GMAC_HAL_TYPE_SYN_XGMAC);
	edma_cleanup(false);
}

/*
 * nss_dp_ppeds_ops_get()
 *	API to get PPE-DS operations()
 */
struct nss_dp_ppeds_ops *nss_dp_ppeds_ops_get(void)
{
#ifdef NSS_DP_PPEDS_SUPPORT
	return &edma_ppeds_ops;
#else
	return NULL;
#endif
}
