/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal_misc.h"
#include "fal_mib.h"
#include "fal_port_ctrl.h"
#include "fal_portvlan.h"
#include "fal_fdb.h"
#include "fal_stp.h"
#include "fal_igmp.h"
#include "fal_qos.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//#include <asm/mach-types.h>
#include <generated/autoconf.h>
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#elif defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/switch.h>
#else
#include <net/switch.h>
#include <linux/ar8216_platform.h>
#endif
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include "ssdk_plat.h"
#include "ref_vlan.h"
#include <linux/time.h>
#include "f1_phy.h"

int
qca_ar8327_sw_get_port_link(struct switch_dev *dev, int port,
			                        struct switch_port_link *link)
{
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);

	fal_port_speed_t speed;
	fal_port_duplex_t duplex;
	a_bool_t status;
	a_bool_t tx_fc;
	a_bool_t rx_fc;
	a_uint32_t ret;
	a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

	mutex_lock(&priv->reg_mutex);
	ret = fal_port_link_status_get(dev_id, port, &status);
	if (ret){
		mutex_unlock(&priv->reg_mutex);
		return -1;
	}

	ret = fal_port_speed_get(dev_id, port, &speed);
	if (ret){
		mutex_unlock(&priv->reg_mutex);
		return -1;
	}

	ret = fal_port_duplex_get(dev_id, port, &duplex);
	if (ret){
		mutex_unlock(&priv->reg_mutex);
		return -1;
	}

	ret = fal_port_rxfc_status_get(dev_id, port, &rx_fc);
	if (ret){
		mutex_unlock(&priv->reg_mutex);
		return -1;
	}

	ret = fal_port_txfc_status_get(dev_id, port, &tx_fc);
	if (ret){
		mutex_unlock(&priv->reg_mutex);
		return -1;
	}
	mutex_unlock(&priv->reg_mutex);

	link->link = status;
	if (speed == FAL_SPEED_10){
		link->speed = SWITCH_PORT_SPEED_10;
	} else if (speed == FAL_SPEED_100){
		link->speed = SWITCH_PORT_SPEED_100;
	} else if (speed == FAL_SPEED_1000){
		link->speed = SWITCH_PORT_SPEED_1000;
	} else {
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
	}

	if (duplex == FAL_HALF_DUPLEX){
		link->duplex = 0; /* HALF */
	} else if (duplex == FAL_FULL_DUPLEX){
		link->duplex = 1; /* FULL */
	}

	link->rx_flow = rx_fc;
	link->tx_flow = tx_fc;

	return 0;
}

void qca_phy_mutex_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                           a_uint32_t reg, a_uint16_t data_mask, a_uint16_t *data)
{
	a_uint16_t phy_val;

	qca_ar8327_phy_read(dev_id, phy_addr, reg, &phy_val);

	phy_val &= ~data_mask;
	phy_val |= (*data & data_mask);

	qca_ar8327_phy_write(dev_id, phy_addr, reg, phy_val );

	*data = phy_val;
}

static int qca_phy_mdi_update(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t *mdi, ulong from_status)
{
	ushort val;

	if(from_status){
		qca_ar8327_phy_read(dev_id, phy_addr, F1_PHY_SPEC_STATUS, &val);
		*mdi = (val & BIT(6)) >> 6;
	}

	qca_ar8327_phy_read(dev_id, phy_addr, F1_PHY_SPEC_CONTROL, &val);

	val = (val & ~(BITS(5,2))) | ((*mdi & 0x3) << 5);

	qca_ar8327_phy_write(dev_id, phy_addr, F1_PHY_SPEC_CONTROL, val);
	*mdi = val;

	return 0;
}

static int qca_switch_get_qm_status(struct switch_dev *dev, a_uint32_t port_id, a_uint32_t *qm_buffer_err)
{
	a_uint32_t reg = 0;
	a_uint32_t qm_val = 0;
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);

	if (port_id < 0 || port_id > 6) {
		*qm_buffer_err = 0;
		return -1;
	}
	if (port_id < 4) {
		reg = 0x1D;
		priv->mii_write(0x820, reg);
		qm_val = priv->mii_read(0x824);
		*qm_buffer_err = (qm_val >> (port_id * 8)) & 0xFF;
	} else {
		reg = 0x1E;
		priv->mii_write(0x820, reg);
		qm_val = priv->mii_read(0x824);
		*qm_buffer_err = (qm_val >> ((port_id-4) * 8)) & 0xFF;
	}

	return 0;
}

static int qca_switch_force_mac_1000M_full(struct switch_dev *dev, a_uint32_t port_id)
{
	a_uint32_t reg, value;
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);

	if (port_id < 0 || port_id > 6)
		return -1;

	reg = AR8327_REG_PORT_STATUS(port_id);
	value = priv->mii_read(reg);
	value &= ~(BIT(6) | BITS(0,2));
	value |= AR8327_PORT_SPEED_1000M | BIT(6);
	priv->mii_write(reg, value);
	value = priv->mii_read(reg);

	return 0;
}

static int qca_switch_force_mac_status(struct switch_dev *dev, a_uint32_t port_id,a_uint32_t speed,a_uint32_t duplex)
{
	a_uint32_t reg, value;
	a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

	if (port_id < 0 || port_id > 6)
		return -1;

	reg = AR8327_REG_PORT_STATUS(port_id);
	qca_switch_reg_read(dev_id,reg,(a_uint8_t*)&value,4);
	value &= ~(BIT(6) | BITS(0,2));
	value |= speed | (duplex?BIT(6):0);
	qca_switch_reg_write(dev_id,reg,(a_uint8_t*)&value,4);
	return 0;
}


#define MDI_FROM_PHY_STATUS 1
#define MDI_FROM_MANUAL 0
#define PORT_LINK_UP 1
#define PORT_LINK_DOWN 0

#define QM_NOT_EMPTY  1
#define QM_EMPTY  0

static a_uint32_t port_old_link[AR8327_NUM_PORTS] = {0, 0 ,0 ,0 , 0, 0, 0};
static a_uint32_t port_old_speed[AR8327_NUM_PORTS] = {AR8327_PORT_SPEED_10M,
	AR8327_PORT_SPEED_10M,
	AR8327_PORT_SPEED_10M,
	AR8327_PORT_SPEED_10M,
	AR8327_PORT_SPEED_10M,
	AR8327_PORT_SPEED_10M,
	AR8327_PORT_SPEED_10M};

static a_uint32_t port_old_duplex[AR8327_NUM_PORTS] = {0, 0 ,0 ,0 , 0, 0, 0};
static a_uint32_t port_old_phy_status[AR8327_NUM_PORTS] = {0, 0 ,0 ,0 , 0, 0, 0};


static a_uint32_t port_qm_buf[AR8327_NUM_PORTS] = {0, 0 ,0 ,0 , 0, 0, 0};

static char speed_str[4][10] = {
	"10M", "100M", "1000M", "Reserved" };

static a_uint32_t phy_current_speed = 2;
static a_uint32_t phy_current_duplex = 1;

void
qca_ar8327_sw_mac_polling_task(struct switch_dev *dev)
{
	static int task_count = 0;
	a_uint32_t i;
	a_uint32_t reg, value;
	a_uint32_t link, speed, duplex, mdi;
	a_uint32_t phy_addr;
	a_uint32_t phy_reg;
	a_uint16_t phy_val;
	a_uint32_t qm_buffer_err = 0;
	a_uint16_t port_phy_status[AR8327_NUM_PORTS];
	static a_uint32_t mac_err_flag[AR8327_NUM_PORTS] = {0,0,0,0,0,0,0};
	static a_uint32_t qm_err_cnt[AR8327_NUM_PORTS] = {0,0,0,0,0,0,0};

	static a_uint32_t link_cnt[AR8327_NUM_PORTS] = {0,0,0,0,0,0,0};

	a_uint16_t data_mask;
	struct qca_phy_priv *priv = qca_phy_priv_get(dev);
	a_uint32_t dev_id = qca_devname_2_devid(dev->devname);

	/*Only valid for S17c chip*/
	if (priv->version != QCA_VER_AR8337) return;

	++task_count;

	for (i = 1; i < AR8327_NUM_PORTS-1; ++i) {
		qca_ar8327_phy_read(dev_id, i-1, F1_PHY_SPEC_STATUS, &port_phy_status[i]);
		speed = (a_uint32_t)((port_phy_status[i] >> 14) & 0x03);
		link = (a_uint32_t)((port_phy_status[i] & BIT(10)) >> 10);
		duplex = (a_uint32_t)((port_phy_status[i] & BIT(13)) >> 13);

		if (link != port_old_link[i]) {
			++link_cnt[i];
			/* Up --> Down */
			if ((port_old_link[i] == PORT_LINK_UP) && (link == PORT_LINK_DOWN)) {
				/* MAC Tx/Rx disable */
				fal_port_txmac_status_set(dev_id, i, A_FALSE);
				fal_port_rxmac_status_set(dev_id, i, A_FALSE);

				/* Check queue buffer */
				qm_err_cnt[i] = 0;
				qca_switch_get_qm_status(dev, i, &qm_buffer_err);

				if (qm_buffer_err) {
					port_qm_buf[i] = QM_NOT_EMPTY;
				}
				else {
					port_qm_buf[i] = QM_EMPTY;
					if ((port_old_speed[i] == AR8327_PORT_SPEED_100M)
							|| (port_old_speed[i] == AR8327_PORT_SPEED_10M)) {
						a_uint16_t phy_ctrl_reg[AR8327_NUM_PORTS];

						phy_addr = i - 1;

						qca_ar8327_phy_read(dev_id, phy_addr, F1_PHY_CONTROL, &phy_ctrl_reg[i]);

						/* Force MDI/MDIX  */
						mdi = 0x0;
						qca_phy_mdi_update(dev_id, phy_addr, &mdi, MDI_FROM_PHY_STATUS);

						/* Disable phy auto-neg, force 100M, duplex, */
						phy_reg = F1_PHY_CONTROL;
						data_mask = F1_CTRL_AUTONEGOTIATION_ENABLE | F1_CTRL_SPEED_MASK
								| F1_CTRL_FULL_DUPLEX | F1_CTRL_SOFTWARE_RESET;

						if (port_old_speed[i] == AR8327_PORT_SPEED_100M)
							phy_val = F1_CTRL_SPEED_100;
						else
							phy_val = F1_CTRL_SPEED_10;
						phy_val |= F1_CTRL_FULL_DUPLEX | F1_CTRL_SOFTWARE_RESET ;
						qca_phy_mutex_write(dev_id, phy_addr, phy_reg, data_mask, &phy_val);
						udelay(1000);

						/* Force MAC 1000M Full before auto negotiation */
						qca_switch_force_mac_1000M_full(dev, i);
						/* Force MDI/MDIX auto-crossover */
						mdi = F1_CTL_AUTO_X_MODE >> 5;
						qca_phy_mdi_update(dev_id, phy_addr, &mdi, MDI_FROM_MANUAL);

						/* Enable phy auto-neg, do soft reset */
						phy_reg = F1_PHY_CONTROL;
						data_mask = F1_CTRL_FULL_DUPLEX | F1_CTRL_SPEED_MASK
								| F1_CTRL_AUTONEGOTIATION_ENABLE | F1_CTRL_SOFTWARE_RESET;
						if(phy_ctrl_reg[i] & F1_CTRL_AUTONEGOTIATION_ENABLE)
							phy_val = F1_CTRL_AUTONEGOTIATION_ENABLE | F1_CTRL_SOFTWARE_RESET;
						else
							phy_val = phy_ctrl_reg[i] | F1_CTRL_SOFTWARE_RESET;
						qca_phy_mutex_write(dev_id, phy_addr, phy_reg, data_mask, &phy_val);
					}
					else {
						/* Force MAC 1000M Full before auto negotiation */
						qca_switch_force_mac_1000M_full(dev, i);
					}
				}
			}
			/* Down --> Up */
			else if ((port_old_link[i] == PORT_LINK_DOWN) && (link == PORT_LINK_UP)) {
				/* Check MAC state machine */
				if (!mac_err_flag[i]) {
					value  = 0x00010045 | (i << 8);
					priv->mii_write(0xc0, value);
					value = priv->mii_read(0xc4);
					if (value & 0xf) {
						mac_err_flag[i] = 1;
					}
					else
						mac_err_flag[i] = 0;
				}
#if 0
				/* Check Queue Buffer */
				qca_switch_get_qm_status(dev, i, &qm_buffer_err);
				if (qm_buffer_err) {
					qm_err_flag = qm_val;
				}
#endif
				/* Change port status */
				reg = AR8327_REG_PORT_STATUS(i);
				value = priv->mii_read(reg);
				/* Speed change */
				value &= ~(BIT(6) | BITS(0,2));
				value |= speed | (duplex << 6);
				priv->mii_write(reg, value);
				udelay(200);

				/* Tx/Rx mac enable */
				fal_port_txmac_status_set(dev_id, i, A_TRUE);
				fal_port_rxmac_status_set(dev_id, i, A_TRUE);
			}

			/* Save the current status */
			port_old_speed[i] = speed;
			port_old_link[i] = link;
			port_old_duplex[i] = duplex;
			port_old_phy_status[i] = port_phy_status[i];
		}

		if (port_qm_buf[i] == QM_NOT_EMPTY) {
			/* Check QM */
			qca_switch_get_qm_status(dev, i, &qm_buffer_err);
			if (qm_buffer_err) {
				port_qm_buf[i] = QM_NOT_EMPTY;
				++qm_err_cnt[i];
			}
			else {
				port_qm_buf[i] = QM_EMPTY;
				qm_err_cnt[i] = 0;

				if ((port_old_speed[i] == AR8327_PORT_SPEED_100M)
					|| (port_old_speed[i] == AR8327_PORT_SPEED_10M)) {
					phy_addr = i - 1;
					/* Force MDI/MDIX  */
					mdi = 0x0;
					qca_phy_mdi_update(dev_id, phy_addr, &mdi, MDI_FROM_PHY_STATUS);

					/* Disable phy auto-neg, force 100M, duplex, */
					phy_reg = F1_PHY_CONTROL;
					data_mask = F1_CTRL_AUTONEGOTIATION_ENABLE | F1_CTRL_SPEED_MASK
							| F1_CTRL_FULL_DUPLEX | F1_CTRL_SOFTWARE_RESET;

					if (port_old_speed[i] == AR8327_PORT_SPEED_100M)
						phy_val = F1_CTRL_SPEED_100;
					else
						phy_val = F1_CTRL_SPEED_10;
					phy_val |= F1_CTRL_FULL_DUPLEX | F1_CTRL_SOFTWARE_RESET;
					qca_phy_mutex_write(dev_id, phy_addr, phy_reg, data_mask, &phy_val);
					udelay(1000);

					/* Force MAC 1000M Full before auto negotiation */
					qca_switch_force_mac_1000M_full(dev, i);

					/* Force MDI/MDIX auto-crossover   */
					mdi = F1_CTL_AUTO_X_MODE >> 5;
					qca_phy_mdi_update(dev_id, phy_addr, &mdi, MDI_FROM_MANUAL);

					/* Enable phy auto-neg, do soft reset */
					phy_reg = F1_PHY_CONTROL;
					data_mask = F1_CTRL_FULL_DUPLEX | F1_CTRL_SPEED_MASK
							| F1_CTRL_AUTONEGOTIATION_ENABLE | F1_CTRL_SOFTWARE_RESET;

					phy_val = F1_CTRL_AUTONEGOTIATION_ENABLE | F1_CTRL_SOFTWARE_RESET;
					qca_phy_mutex_write(dev_id, phy_addr, phy_reg, data_mask, &phy_val);
				}
				else {
					/* Force MAC 1000M Full before auto negotiation */
					qca_switch_force_mac_1000M_full(dev, i);
				}
			}
		}
	}

	return ;
}

void
dess_rgmii_sw_mac_polling_task(struct switch_dev *dev)
{
	a_uint32_t mac_mode;
	a_uint32_t phy_spec_status, phy_link_status;
	a_uint32_t speed, duplex;

	mac_mode = ssdk_dt_global_get_mac_mode();

	if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII5)
		||(mac_mode == PORT_WRAPPER_SGMII1_RGMII5)
		||(mac_mode == PORT_WRAPPER_SGMII0_RGMII4)
		||(mac_mode == PORT_WRAPPER_SGMII1_RGMII4)
		||(mac_mode == PORT_WRAPPER_SGMII4_RGMII4)) {
		qca_ar8327_phy_read(0, 4, 0x11, &phy_spec_status);
		phy_link_status = (a_uint32_t)((phy_spec_status & BIT(10)) >> 10);
		if (phy_link_status == 1) {
			speed = (a_uint32_t)((phy_spec_status >> 14) & 0x03);
			duplex = (a_uint32_t)((phy_spec_status & BIT(13)) >> 13);
			if ((speed != phy_current_speed) || (duplex != phy_current_duplex)) {
				if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII5)
				||(mac_mode == PORT_WRAPPER_SGMII1_RGMII5))
				qca_switch_force_mac_status(dev, 5,speed,duplex);

				if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII4)
				||(mac_mode == PORT_WRAPPER_SGMII1_RGMII4)
				||(mac_mode == PORT_WRAPPER_SGMII4_RGMII4))
				qca_switch_force_mac_status(dev, 4,speed,duplex);
			}
			phy_current_speed = speed;
			phy_current_duplex = duplex;
		}
	}

	return;
}

