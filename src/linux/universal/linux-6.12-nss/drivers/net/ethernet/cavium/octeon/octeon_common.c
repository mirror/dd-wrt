/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2013 Cavium, Inc.
 */

#include <linux/module.h>

#include <asm/octeon/octeon.h>
#include "octeon_common.h"

#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-agl-defs.h>
#include <asm/octeon/cvmx-pip-defs.h>

struct cvm_oct_cam_state {
	u64 cam[6];
	u64 cam_mask;
	int cam_index;
};

static void cvm_oct_cam_state_add(struct cvm_oct_cam_state *cs, const unsigned char *addr)
{
	int i;

	for (i = 0; i < 6; i++)
		cs->cam[i] |= (u64)addr[i] << (8 * (cs->cam_index));
	cs->cam_mask |= (1ULL << cs->cam_index);
	cs->cam_index++;
}

/**
 * Set the multicast list
 * @dev      : Device to work on
 * @base_reg : Base address of register bank
 * @rx_lock  : For locking
 *
 */
void cvm_oct_common_set_rx_filtering(struct net_device *dev, u64 base_reg, spinlock_t *lock)
{
	u64 reg, saved_reg;
	unsigned long flags;
	unsigned int cam_mode = 1; /* 1 - Accept on CAM match */
	unsigned int multicast_mode = 1; /* 1 - Reject all multicast.  */
	struct cvm_oct_cam_state cam_state;
	struct netdev_hw_addr *ha;
	int available_cam_entries;

	if (base_reg == 0)
		return;

	memset(&cam_state, 0, sizeof(cam_state));

	if ((dev->flags & IFF_PROMISC) || dev->uc.count > 7) {
		cam_mode = 0;
		available_cam_entries = 8;
	} else {
		/* One CAM entry for the primary address, leaves seven
		 * for the secondary addresses.
		 */
		available_cam_entries = 7 - dev->uc.count;
	}

	if (dev->flags & IFF_MULTICAST) {
		if (cam_mode == 0 || (dev->flags & IFF_ALLMULTI) ||
		    netdev_mc_count(dev) > available_cam_entries)
			multicast_mode = 2; /* 2 - Accept all multicast.  */
		else
			multicast_mode = 0; /* 0 - Use CAM.  */
	}

	if (cam_mode == 1) {
		/* Add primary address. */
		cvm_oct_cam_state_add(&cam_state, dev->dev_addr);
		netdev_for_each_uc_addr(ha, dev)
			cvm_oct_cam_state_add(&cam_state, ha->addr);
	}
	if (multicast_mode == 0) {
		netdev_for_each_mc_addr(ha, dev)
			cvm_oct_cam_state_add(&cam_state, ha->addr);
	}

	/* Disable packet I/O. */
	spin_lock_irqsave(lock, flags);

	reg = cvmx_read_csr(base_reg + GMX_PRT_CFG);
	saved_reg = reg;
	cvmx_write_csr(base_reg + GMX_PRT_CFG, reg & ~1ull);

	reg = 0;
	reg |= (cam_mode << 3) & 0x8;
	reg |= (multicast_mode << 1) & 0x6;
	reg |= 1;     /* Allow broadcast */

	cvmx_write_csr(base_reg + GMX_RX_ADR_CTL, reg);

	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM0, cam_state.cam[0]);
	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM1, cam_state.cam[1]);
	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM2, cam_state.cam[2]);
	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM3, cam_state.cam[3]);
	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM4, cam_state.cam[4]);
	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM5, cam_state.cam[5]);
	cvmx_write_csr(base_reg + GMX_RX_ADR_CAM_EN, cam_state.cam_mask);

	/* Restore packet I/O. */
	cvmx_write_csr(base_reg + GMX_PRT_CFG, saved_reg);

	spin_unlock_irqrestore(lock, flags);
}
EXPORT_SYMBOL(cvm_oct_common_set_rx_filtering);

/**
 * Set the hardware MAC address for a device.
 * @dev      : Device to work on
 * @addr     : Address to change it to
 * @base_reg : Base address of register bank
 * @rx_lock  : For locking
 *
 * Returns zero on success
 */
int cvm_oct_common_set_mac_address(struct net_device *dev, void *addr,
	 u64 base_reg, spinlock_t *lock)
{
	int r = eth_mac_addr(dev, addr);

	if (r)
		return r;

	cvm_oct_common_set_rx_filtering(dev, base_reg, lock);

	return 0;
}
EXPORT_SYMBOL(cvm_oct_common_set_mac_address);

/**
 * cvm_oct_common_change_mtu - change the link MTU
 * @dev      : Device to change
 * @mtu      : The new MTU
 * @base_reg : Base address of register bank
 * @pip_reg  : Used for frame checking
 * @mtu_limit: Max allowed MTU size
 *
 * Returns Zero on success
 */
int cvm_oct_common_change_mtu(struct net_device *dev, int mtu, u64 base_reg,
		u64 pip_reg, int mtu_limit)
{
	int max_packet = mtu + OCTEON_FRAME_HEADER_LEN;

	if (max_packet < 64 || max_packet > mtu_limit) {
		netdev_err(dev, "MTU must be between %d and %d.\n",
			64 - OCTEON_FRAME_HEADER_LEN, mtu_limit - OCTEON_FRAME_HEADER_LEN);
		return -EINVAL;
	}

	dev->mtu = mtu;

	/* Set the hardware to truncate packets larger than
	 * the MTU. The jabber register must be set to a
	 * multiple of 8 bytes, so round up.
	 */
	if (base_reg) {
		if (pip_reg == 0)
			cvmx_write_csr(base_reg + GMX_RX_FRM_MAX, max_packet);
		else {
			union cvmx_pip_prt_cfgx port_cfg;

			port_cfg.u64 = cvmx_read_csr(pip_reg);
			if (port_cfg.s.maxerr_en) {
				/* Disable the PIP check as it can
				 * only be controlled over a group of
				 * ports, let the check be done in the
				 * GMX instead.
				 */
				port_cfg.s.maxerr_en = 0;
				cvmx_write_csr(pip_reg, port_cfg.u64);
			}
		}
		/* Set the hardware to truncate packets larger than
		 * the MTU. The jabber register must be set to a
		 * multiple of 8 bytes, so round up.
		 */
		cvmx_write_csr(base_reg + GMX_RX_JABBER, (max_packet + 7) & ~7u);
	}

	return 0;
}
EXPORT_SYMBOL(cvm_oct_common_change_mtu);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cavium, Inc. Common Network Driver");
