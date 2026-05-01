/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __LINUX_PCS_QCOM_IPQ9574_H
#define __LINUX_PCS_QCOM_IPQ9574_H

struct device_node;
struct phylink_pcs;

struct phylink_pcs *ipq_pcs_get(struct device_node *np);
void ipq_pcs_put(struct phylink_pcs *pcs);

#endif /* __LINUX_PCS_QCOM_IPQ9574_H */
