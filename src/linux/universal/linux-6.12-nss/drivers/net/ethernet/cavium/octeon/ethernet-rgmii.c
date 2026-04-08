/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/phy.h>
#include <linux/ratelimit.h>
#include <net/dst.h>

#include <asm/octeon/octeon-hw-status.h>
#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

#include <asm/octeon/cvmx-helper.h>

#include <asm/octeon/cvmx-ipd-defs.h>
#include <asm/octeon/cvmx-npi-defs.h>
#include <asm/octeon/cvmx-gmxx-defs.h>

#define INT_BIT_PHY_LINK 16
#define INT_BIT_PHY_SPD 17
#define INT_BIT_PHY_DUPX 18

DEFINE_SPINLOCK(global_register_lock);

static void cvm_oct_rgmii_poll(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	cvmx_helper_link_info_t link_info;

	if (priv->phydev) {
		link_info.u64 = 0;
		link_info.s.link_up = priv->last_link ? 1 : 0;
		link_info.s.full_duplex = priv->phydev->duplex ? 1 : 0;
		link_info.s.speed = priv->phydev->speed;
	} else {
		link_info = cvmx_helper_link_get(priv->ipd_port);
	}
	if (link_info.u64 == priv->link_info) {
		/* If the 10Mbps preamble workaround is supported and we're
		 * at 10Mbps we may need to do some special checking.
		 */
		if (USE_10MBPS_PREAMBLE_WORKAROUND && (link_info.s.speed == 10)) {
			/* Read the GMXX_RXX_INT_REG[PCTERR] bit and
			 * see if we are getting preamble errors.
			 */
			union cvmx_gmxx_rxx_int_reg gmxx_rxx_int_reg;
			gmxx_rxx_int_reg.u64 =
				cvmx_read_csr(CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface));
			if (gmxx_rxx_int_reg.s.pcterr) {
				/* We are getting preamble errors at
				 * 10Mbps.  Most likely the PHY is
				 * giving us packets with mis aligned
				 * preambles. In order to get these
				 * packets we need to disable preamble
				 * checking and do it in software.
				 */
				union cvmx_gmxx_rxx_frm_ctl gmxx_rxx_frm_ctl;

				/* Disable preamble checking */
				gmxx_rxx_frm_ctl.u64 =
					cvmx_read_csr(CVMX_GMXX_RXX_FRM_CTL(priv->interface_port, priv->interface));
				gmxx_rxx_frm_ctl.s.pre_chk = 0;
				cvmx_write_csr(CVMX_GMXX_RXX_FRM_CTL(priv->interface_port, priv->interface),
					       gmxx_rxx_frm_ctl.u64);

				/* Clear any error bits */
				cvmx_write_csr(CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface),
					       gmxx_rxx_int_reg.u64);
				printk_ratelimited("%s: Using 10Mbps with software preamble removal\n",
						   dev->name);
			}
		}
		return;
	}

	/* If the 10Mbps preamble workaround is allowed we need to on
	 * preamble checking, FCS stripping, and clear error bits on
	 * every speed change. If errors occur during 10Mbps operation
	 * the above code will change this stuff.
	 */
	if (USE_10MBPS_PREAMBLE_WORKAROUND) {
		union cvmx_gmxx_rxx_frm_ctl gmxx_rxx_frm_ctl;
		union cvmx_gmxx_rxx_int_reg gmxx_rxx_int_reg;

		/* Enable preamble checking */
		gmxx_rxx_frm_ctl.u64 =
		    cvmx_read_csr(CVMX_GMXX_RXX_FRM_CTL(priv->interface_port, priv->interface));
		gmxx_rxx_frm_ctl.s.pre_chk = 1;
		cvmx_write_csr(CVMX_GMXX_RXX_FRM_CTL(priv->interface_port, priv->interface),
			       gmxx_rxx_frm_ctl.u64);
		/* Clear any error bits */
		gmxx_rxx_int_reg.u64 =
			cvmx_read_csr(CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface));
		cvmx_write_csr(CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface),
			       gmxx_rxx_int_reg.u64);
	}
	if (priv->phydev == NULL) {
		link_info = cvmx_helper_link_autoconf(priv->ipd_port);
		priv->link_info = link_info.u64;
	}

	if (priv->phydev == NULL)
		cvm_oct_set_carrier(priv, link_info);
}

static int cvm_oct_rgmii_hw_status(struct notifier_block *nb, unsigned long val, void *v)
{
	struct octeon_ethernet *priv = container_of(nb, struct octeon_ethernet, hw_status_notifier);

	if (val == OCTEON_HW_STATUS_SOURCE_ASSERTED) {
		struct octeon_hw_status_data *d = v;
		if (d->reg == CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface) &&
		    (d->bit == INT_BIT_PHY_LINK ||
		     d->bit == INT_BIT_PHY_SPD ||
		     d->bit == INT_BIT_PHY_DUPX)) {
			if (!atomic_read(&cvm_oct_poll_queue_stopping))
				queue_work(cvm_oct_poll_queue, &priv->port_work);
			return NOTIFY_STOP;
		}
	}
	return NOTIFY_DONE;
}

static void cvm_oct_rgmii_immediate_poll(struct work_struct *work)
{
	struct octeon_ethernet *priv = container_of(work, struct octeon_ethernet, port_work);
	cvm_oct_rgmii_poll(priv->netdev);
}

int cvm_oct_rgmii_open(struct net_device *dev)
{
	union cvmx_ipd_sub_port_fcs ipd_sub_port_fcs;
	struct octeon_hw_status_reg sr[3];
	u64 en_mask;
	union cvmx_gmxx_prtx_cfg gmx_cfg;
	struct octeon_ethernet *priv = netdev_priv(dev);
	cvmx_helper_link_info_t link_info;
	int rv;

	rv = cvm_oct_phy_setup_device(dev);
	if (rv)
		return rv;


	gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface));
	gmx_cfg.s.en = 1;
	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface), gmx_cfg.u64);

	if (!octeon_is_simulation()) {
		if (priv->phydev) {
			int r = phy_read_status(priv->phydev);
			if (r == 0 && priv->phydev->link == 0)
				netif_carrier_off(dev);
			cvm_oct_adjust_link(dev);
		} else {
			link_info = cvmx_helper_link_get(priv->ipd_port);
			if (!link_info.s.link_up)
				netif_carrier_off(dev);
			priv->poll = cvm_oct_rgmii_poll;
		}
	}

	INIT_WORK(&priv->port_work, cvm_oct_rgmii_immediate_poll);


	/* Only true RGMII ports need to be polled. In GMII mode, port
	 * 0 is really a RGMII port.
	 */
	if ((priv->imode == CVMX_HELPER_INTERFACE_MODE_GMII && priv->ipd_port != 0) ||
	    octeon_is_simulation())
		return 0;

	/* Due to GMX errata in CN3XXX series chips, it is necessary
	 * to take the link down immediately when the PHY changes
	 * state. In order to do this we call the poll function every
	 * time the RGMII inband status changes.  This may cause
	 * problems if the PHY doesn't implement inband status
	 * properly.
	 */
	priv->hw_status_notifier.priority = 10;
	priv->hw_status_notifier.notifier_call = cvm_oct_rgmii_hw_status;
	octeon_hw_status_notifier_register(&priv->hw_status_notifier);

	en_mask = 0;
	memset(sr, 0, sizeof(sr));
	sr[0].reg = 46; /* RML */
	sr[0].reg_is_hwint = 1;
	sr[0].has_child = 1;
	sr[1].reg = CVMX_NPI_RSL_INT_BLOCKS;
	sr[1].bit = priv->interface + 1;
	sr[1].has_child = 1;
	sr[2].reg = CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface);
	sr[2].mask_reg = CVMX_GMXX_RXX_INT_EN(priv->interface_port, priv->interface);
	sr[2].ack_w1c = 1;

	sr[2].bit = INT_BIT_PHY_LINK;
	en_mask |= 1ull << sr[2].bit;
	octeon_hw_status_add_source(sr);

	sr[2].bit = INT_BIT_PHY_SPD;
	en_mask |= 1ull << sr[2].bit;
	octeon_hw_status_add_source(sr);

	sr[2].bit = INT_BIT_PHY_DUPX;
	en_mask |= 1ull << sr[2].bit;
	octeon_hw_status_add_source(sr);

	octeon_hw_status_enable(sr[2].reg, en_mask);


	/* Disable FCS stripping for PKI-602*/
	ipd_sub_port_fcs.u64 = cvmx_read_csr(CVMX_IPD_SUB_PORT_FCS);
	ipd_sub_port_fcs.s.port_bit &= 0xffffffffull ^ (1ull << priv->ipd_port);
	cvmx_write_csr(CVMX_IPD_SUB_PORT_FCS, ipd_sub_port_fcs.u64);
	priv->rx_strip_fcs = 1;

	return 0;
}

int cvm_oct_rgmii_stop(struct net_device *dev)
{
	union cvmx_ipd_sub_port_fcs ipd_sub_port_fcs;
	union cvmx_gmxx_prtx_cfg gmx_cfg;
	struct octeon_ethernet *priv = netdev_priv(dev);

	gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface));
	gmx_cfg.s.en = 0;
	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface), gmx_cfg.u64);


	if (priv->hw_status_notifier.notifier_call) {
		struct octeon_hw_status_reg sr;
		memset(&sr, 0, sizeof(sr));

		sr.reg = CVMX_GMXX_RXX_INT_REG(priv->interface_port, priv->interface);
		sr.mask_reg = CVMX_GMXX_RXX_INT_EN(priv->interface_port, priv->interface);
		sr.ack_w1c = 1;
		sr.bit = INT_BIT_PHY_LINK;
		octeon_hw_status_remove_source(&sr);
		sr.bit = INT_BIT_PHY_SPD;
		octeon_hw_status_remove_source(&sr);
		sr.bit = INT_BIT_PHY_DUPX;
		octeon_hw_status_remove_source(&sr);
		octeon_hw_status_notifier_unregister(&priv->hw_status_notifier);
		priv->hw_status_notifier.notifier_call = NULL;
	}

	cancel_work_sync(&priv->port_work);

	/* re-Enable FCS stripping */
	ipd_sub_port_fcs.u64 = cvmx_read_csr(CVMX_IPD_SUB_PORT_FCS);
	ipd_sub_port_fcs.s.port_bit |= 1ull << priv->ipd_port;
	cvmx_write_csr(CVMX_IPD_SUB_PORT_FCS, ipd_sub_port_fcs.u64);
	priv->rx_strip_fcs = 0;

	return cvm_oct_common_stop(dev);
}

int cvm_oct_rgmii_init(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	union cvmx_gmxx_prtx_cfg gmx_cfg;

	gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface));
	gmx_cfg.s.en = 0;
	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface), gmx_cfg.u64);

	cvm_oct_common_init(dev);

	return 0;
}
