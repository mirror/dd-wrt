/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _DT_BINDINGS_CLK_QCOM_IPQ9650_CMN_PLL_H
#define _DT_BINDINGS_CLK_QCOM_IPQ9650_CMN_PLL_H

/* Parent clock */
#define IPQ9650_CMN_PLL_CLK		0

/* Fixed-rate clocks */
#define IPQ9650_XO_24MHZ_CLK		1
#define IPQ9650_SLEEP_32KHZ_CLK		2

/* Configurable divider clocks */
#define IPQ9650_NSS_CLK			3
#define IPQ9650_PPE_CLK			4
#define IPQ9650_PCS0_CLK		5
#define IPQ9650_PCS1_CLK		6
#define IPQ9650_PCS2_CLK		7

/* Gate clocks */
#define IPQ9650_ETH0_50MHZ_CLK		8
#define IPQ9650_ETH1_50MHZ_CLK		9
#define IPQ9650_ETH2_50MHZ_CLK		10
#define IPQ9650_ETH_25MHZ_CLK		11
#define IPQ9650_ETH_PON_CLK		12

#endif
