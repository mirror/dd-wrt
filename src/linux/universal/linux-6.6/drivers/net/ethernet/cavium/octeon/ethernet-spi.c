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
#include <net/dst.h>

#include <asm/octeon/octeon-hw-status.h>
#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

#include <asm/octeon/cvmx-spi.h>

#include <asm/octeon/cvmx-npi-defs.h>
#include <asm/octeon/cvmx-spxx-defs.h>
#include <asm/octeon/cvmx-stxx-defs.h>

#define MASK_FOR_BIT(_X) (1ull << (_X))

#define INT_BIT_SPX_PRTNXA 0
#define INT_BIT_SPX_ABNORM 1
#define INT_BIT_SPX_SPIOVR 4
#define INT_BIT_SPX_CLSERR 5
#define INT_BIT_SPX_DRWNNG 6
#define INT_BIT_SPX_RSVERR 7
#define INT_BIT_SPX_TPAOVR 8
#define INT_BIT_SPX_DIPERR 9
#define INT_BIT_SPX_SYNCERR 10
#define INT_BIT_SPX_CALERR 11

#define SPX_MASK (				\
	MASK_FOR_BIT(INT_BIT_SPX_PRTNXA) |	\
	MASK_FOR_BIT(INT_BIT_SPX_ABNORM) |	\
	MASK_FOR_BIT(INT_BIT_SPX_SPIOVR) |	\
	MASK_FOR_BIT(INT_BIT_SPX_CLSERR) |	\
	MASK_FOR_BIT(INT_BIT_SPX_DRWNNG) |	\
	MASK_FOR_BIT(INT_BIT_SPX_RSVERR) |	\
	MASK_FOR_BIT(INT_BIT_SPX_TPAOVR) |	\
	MASK_FOR_BIT(INT_BIT_SPX_DIPERR) |	\
	MASK_FOR_BIT(INT_BIT_SPX_CALERR))

#define INT_BIT_SPX_SPF 31

#define INT_BIT_STX_CALPAR0 0
#define INT_BIT_STX_CALPAR1 1
#define INT_BIT_STX_OVRBST 2
#define INT_BIT_STX_DATOVR 3
#define INT_BIT_STX_DIPERR 4
#define INT_BIT_STX_NOSYNC 5
#define INT_BIT_STX_UNXFRM 6
#define INT_BIT_STX_FRMERR 7
#define INT_BIT_STX_SYNCERR 8
#define STX_MASK (					\
		MASK_FOR_BIT(INT_BIT_STX_CALPAR0) |	\
		MASK_FOR_BIT(INT_BIT_STX_CALPAR1) |	\
		MASK_FOR_BIT(INT_BIT_STX_OVRBST) |	\
		MASK_FOR_BIT(INT_BIT_STX_DATOVR) |	\
		MASK_FOR_BIT(INT_BIT_STX_DIPERR) |	\
		MASK_FOR_BIT(INT_BIT_STX_NOSYNC) |	\
		MASK_FOR_BIT(INT_BIT_STX_UNXFRM) |	\
		MASK_FOR_BIT(INT_BIT_STX_FRMERR) |	\
		MASK_FOR_BIT(INT_BIT_STX_SYNCERR))

static int need_retrain[2] = { 0, 0 };

static int cvm_oct_spi_hw_status(struct notifier_block *nb, unsigned long val, void *v)
{
	struct octeon_ethernet *priv = container_of(nb, struct octeon_ethernet, hw_status_notifier);

	if (val == OCTEON_HW_STATUS_SOURCE_ASSERTED) {
		struct octeon_hw_status_data *d = v;
		if (d->reg == CVMX_SPXX_INT_REG(priv->interface) ||
		    d->reg == CVMX_STXX_INT_REG(priv->interface)) {
			if (need_retrain[priv->interface])
				return NOTIFY_STOP;
			need_retrain[priv->interface] = 1;
			octeon_hw_status_disable(CVMX_SPXX_INT_REG(priv->interface), SPX_MASK);
			octeon_hw_status_disable(CVMX_STXX_INT_REG(priv->interface), STX_MASK);
		}
	}
	return NOTIFY_DONE;
}
static void cvm_oct_spi_poll(struct net_device *dev)
{
	static int spi4000_port;
	struct octeon_ethernet *priv = netdev_priv(dev);
	int interface;

	for (interface = 0; interface < 2; interface++) {
		if ((priv->ipd_port == interface * 16) && need_retrain[interface]) {
			if (cvmx_spi_restart_interface(interface, CVMX_SPI_MODE_DUPLEX, 10) == 0) {
				need_retrain[interface] = 0;
				octeon_hw_status_enable(CVMX_SPXX_INT_REG(priv->interface), SPX_MASK);
				octeon_hw_status_enable(CVMX_STXX_INT_REG(priv->interface), STX_MASK);
			}
		}

		/* The SPI4000 TWSI interface is very slow. In order
		 * not to bring the system to a crawl, we only poll a
		 * single port every second. This means negotiation
		 * speed changes take up to 10 seconds, but at least
		 * we don't waste absurd amounts of time waiting for
		 * TWSI.
		 */
		if (priv->ipd_port == spi4000_port) {
			/* This function does nothing if it is called on an
			 * interface without a SPI4000.
			 */
			cvmx_spi4000_check_speed(interface, priv->ipd_port);
			/* Normal ordering increments. By decrementing
			 * we only match once per iteration.
			 */
			spi4000_port--;
			if (spi4000_port < 0)
				spi4000_port = 10;
		}
	}
}

int cvm_oct_spi_init(struct net_device *dev)
{
	int r, i;
	struct octeon_hw_status_reg sr[3];
	struct octeon_ethernet *priv = netdev_priv(dev);

	if ((priv->ipd_port == 0) || (priv->ipd_port == 16)) {

		priv->hw_status_notifier.priority = 10;
		priv->hw_status_notifier.notifier_call = cvm_oct_spi_hw_status;
		r = octeon_hw_status_notifier_register(&priv->hw_status_notifier);
		if (r)
			return r;

		memset(sr, 0, sizeof(sr));
		sr[0].reg = 46; /* RML */
		sr[0].reg_is_hwint = 1;
		sr[0].has_child = 1;

		sr[1].reg = CVMX_NPI_RSL_INT_BLOCKS;
		sr[1].bit = priv->interface + 18;
		sr[1].has_child = 1;

		sr[2].reg = CVMX_SPXX_INT_REG(priv->interface);
		sr[2].mask_reg = CVMX_SPXX_INT_MSK(priv->interface);
		sr[2].ack_w1c = 1;

		for (i = 0; i < 32; i++) {
			if ((1ull << i) & SPX_MASK) {
				sr[2].bit = i;
				octeon_hw_status_add_source(sr);
			}
		}

		sr[2].bit = INT_BIT_SPX_SPF;
		sr[2].ack_w1c = 0;
		octeon_hw_status_add_source(sr);
		octeon_hw_status_enable(sr[2].reg, SPX_MASK);

		sr[2].reg = CVMX_STXX_INT_REG(priv->interface);
		sr[2].mask_reg = CVMX_STXX_INT_MSK(priv->interface);
		sr[2].ack_w1c = 1;

		for (i = 0; i < 32; i++) {
			if ((1ull << i) & STX_MASK) {
				sr[2].bit = i;
				octeon_hw_status_add_source(sr);
			}
		}
		octeon_hw_status_enable(sr[2].reg, STX_MASK);

		priv->poll = cvm_oct_spi_poll;
	}
	cvm_oct_common_init(dev);
	return 0;
}

void cvm_oct_spi_uninit(struct net_device *dev)
{
	int i;
	struct octeon_ethernet *priv = netdev_priv(dev);

	if (priv->hw_status_notifier.notifier_call) {
		struct octeon_hw_status_reg sr;
		memset(&sr, 0, sizeof(sr));

		sr.reg = CVMX_SPXX_INT_REG(priv->interface);
		sr.mask_reg = CVMX_SPXX_INT_MSK(priv->interface);
		sr.ack_w1c = 1;

		for (i = 0; i < 32; i++) {
			if ((1ull << i) & SPX_MASK) {
				sr.bit = i;
				octeon_hw_status_remove_source(&sr);
			}
		}
		sr.bit = INT_BIT_SPX_SPF;
		sr.ack_w1c = 0;
		octeon_hw_status_remove_source(&sr);

		sr.reg = CVMX_STXX_INT_REG(priv->interface);
		sr.mask_reg = CVMX_STXX_INT_MSK(priv->interface);
		sr.ack_w1c = 1;
		for (i = 0; i < 32; i++) {
			if ((1ull << i) & STX_MASK) {
				sr.bit = i;
				octeon_hw_status_remove_source(&sr);
			}
		}

		octeon_hw_status_notifier_unregister(&priv->hw_status_notifier);
		priv->hw_status_notifier.notifier_call = NULL;
	}
}
