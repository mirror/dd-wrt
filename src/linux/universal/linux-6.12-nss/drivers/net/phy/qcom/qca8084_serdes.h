// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for QCA8084 SerDes
 *
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _QCA8084_SERDES_H_
#define _QCA8084_SERDES_H_

#include <linux/mdio.h>
#include <linux/of.h>

struct mdio_device *qca8084_package_pcs_probe(struct device_node *pcs_np);
struct mdio_device *qca8084_package_xpcs_probe(struct device_node *xpcs_np);
void qca8084_package_xpcs_and_pcs_remove(struct mdio_device *xpcs_mdiodev,
					 struct mdio_device *pcs_mdiodev);
int qca8084_qxgmii_set_mode(struct mdio_device *xpcs_mdiodev,
			    struct mdio_device *pcs_mdiodev);
void qca8084_qxgmii_set_speed(struct mdio_device *xpcs_mdiodev,
			      struct mdio_device *pcs_mdiodev,
			      int channel, int speed);
#endif /* _QCA8084_SERDES_H_ */
