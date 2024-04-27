/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/bitops.h>
#include <nss_dp_dev.h>
#include <ppe_drv_public.h>

/*
 * __nss_dp_get_priv_flags()
 *	get nss-dp private flags
 */
u32 __nss_dp_get_priv_flags(struct net_device *dev)
{
	struct nss_dp_dev *dp_priv = (struct nss_dp_dev *)netdev_priv(dev);
	return dp_priv->ethtool_priv_flags;
}

/*
 * nss_dp_reset_mrr_analysis_port_cfg()
 *	reset previously set mirror analysis port
 */
static int nss_dp_reset_mrr_analysis_port_cfg(uint8_t prv_analysis_port,
		uint32_t mrr_analysis_flag)
{
	struct net_device *dev;
	struct nss_dp_dev *dp_priv;
	dev = ppe_drv_port_num_to_dev(prv_analysis_port);
	if (!dev) {
		return -EIO;
	}

	dp_priv = (struct nss_dp_dev *)netdev_priv(dev);
	change_bit(mrr_analysis_flag,
			(unsigned long *)&(dp_priv->ethtool_priv_flags));

	return 0;
}

/*
 * nss_dp_set_analysis_port()
 *	Set mirror analysis port
 */
static int nss_dp_set_analysis_port(struct net_device *dev, u32 flags,
		ppe_drv_dp_mirror_direction_t mirr_analysis_dir)
{
	ppe_drv_ret_t err;
	uint8_t prv_analysis_port;
	bool is_prv_port_need_to_reset = false;
	struct nss_dp_dev *dp_priv = (struct nss_dp_dev *)netdev_priv(dev);
	struct ppe_drv_iface *iface = ppe_drv_iface_get_by_dev(dev);

	if (dp_priv->ethtool_priv_flags & NSS_DP_MIRR_IN_ENABLE ||
			dp_priv->ethtool_priv_flags & NSS_DP_MIRR_EG_ENABLE) {
		/*
		 * If current port is already set for ingress/egress mirror then user
		 * can not set current port to mirror analysis port
		 */
		netdev_info(dev, "Port %s is set for %s mirror port, So can not set port to %s\n",
			   dev->name,
			   (dp_priv->ethtool_priv_flags & NSS_DP_MIRR_IN_ENABLE ?
				"Ingress" : "Egress"),
			   (mirr_analysis_dir == PPE_DRV_DP_MIRR_DI_IN ?
				"ingress mirror analysis" : "egress mirror analysis"));
		return -EINVAL;
	}

	switch (mirr_analysis_dir) {
	/*
	 * Set ingress mirror analysis port
	 */
	case PPE_DRV_DP_MIRR_DI_IN:
		if (flags & NSS_DP_MIRR_ANALYSIS_IN_ENABLE) {
			/*
			 * If command is to set mirror egress analysis port then check
			 * if previously any port was set for egress mirror analysis
			 * if yes then get the port no and on same port reset the
			 * egress mirror analysis bit.
			 */
			err = ppe_drv_dp_get_mirr_analysis_port(PPE_DRV_DP_MIRR_DI_IN,
					&prv_analysis_port);
			/*
			 * if above api returns success mean other port is already set
			 * need to reset the local ethtool private flag.
			 */
			if (err == PPE_DRV_RET_SUCCESS) {
				is_prv_port_need_to_reset = true;
			}
		}

		err = ppe_drv_dp_set_mirr_analysis_port(iface, mirr_analysis_dir,
				(flags & NSS_DP_MIRR_ANALYSIS_IN_ENABLE),
				PPE_DRV_MIRR_ANALYSIS_PRI);
		if (err != PPE_DRV_RET_SUCCESS) {
			netdev_info(dev, "Failed to set %s mirror port analysis for %s\n",
					(flags & NSS_DP_MIRR_ANALYSIS_IN_ENABLE ?
					 "enable" : "disable"),
					dev->name);
			return -EIO;
		}

		change_bit(NSS_DP_MIRR_ANALYSIS_IN_FLG_BIT,
				(unsigned long *)&(dp_priv->ethtool_priv_flags));

		/*
		 * Reset previous mirror analysis port
		 */
		if ((flags & NSS_DP_MIRR_ANALYSIS_IN_ENABLE) &&
				is_prv_port_need_to_reset) {
			return nss_dp_reset_mrr_analysis_port_cfg(prv_analysis_port,
					NSS_DP_MIRR_ANALYSIS_IN_FLG_BIT);
		}
		break;

	case PPE_DRV_DP_MIRR_DI_EG:
		if (flags & NSS_DP_MIRR_ANALYSIS_EG_ENABLE) {
			/*
			 * If command is to set mirror egress analysis port then check
			 * if previously any port was set for egress mirror analysis
			 * if yes then get the port no and on same port reset the
			 * egress mirror analysis bit.
			 */
			err = ppe_drv_dp_get_mirr_analysis_port(PPE_DRV_DP_MIRR_DI_EG,
					&prv_analysis_port);
			/*
			 * if above api returns success mean other port is already set
			 * need to reset the local ethtool private flag.
			 */
			if (err == PPE_DRV_RET_SUCCESS) {
				is_prv_port_need_to_reset = true;
			}
		}

		err = ppe_drv_dp_set_mirr_analysis_port(iface, mirr_analysis_dir,
				(flags & NSS_DP_MIRR_ANALYSIS_EG_ENABLE),
				PPE_DRV_MIRR_ANALYSIS_PRI);
		if (err != PPE_DRV_RET_SUCCESS) {
			netdev_info(dev, "Failed to set %s mirror port analysis for %s\n",
					(flags & NSS_DP_MIRR_ANALYSIS_EG_ENABLE ?
					 "enable" : "disable"),
					dev->name);
			return -EIO;
		}

		change_bit(NSS_DP_MIRR_ANALYSIS_EG_FLG_BIT,
				(unsigned long *)&(dp_priv->ethtool_priv_flags));

		/*
		 * Reset previous mirror analysis port
		 */
		if ((flags & NSS_DP_MIRR_ANALYSIS_EG_ENABLE) &&
				is_prv_port_need_to_reset) {
			return nss_dp_reset_mrr_analysis_port_cfg(prv_analysis_port,
					NSS_DP_MIRR_ANALYSIS_IN_FLG_BIT);
		}
	}
	return 0;
}

/*
 * __nss_dp_set_priv_flags()
 *	set nss-dp private flags
 */
int __nss_dp_set_priv_flags(struct net_device *dev, u32 flags)
{
	struct ppe_drv_iface *iface;
	ppe_drv_ret_t err;
	struct nss_dp_dev *dp_priv = (struct nss_dp_dev *)netdev_priv(dev);
	uint32_t loc_flag = dp_priv->ethtool_priv_flags ^ flags;

	iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		netdev_info(dev, "Failed to get iface for interface %s\n", dev->name);
		return -EIO;
	}

	switch (loc_flag) {
	case NSS_DP_NO_PRIV_FLAG_CHANGE:
		/*
		 * No change of flags therefore, return 0
		 */
		break;

	case NSS_DP_MIRR_IN_ENABLE:
		/*
		 * Set ingress mirror interface
		 */
		if (dp_priv->ethtool_priv_flags & NSS_DP_MIRR_ANALYSIS_IN_ENABLE ||
				dp_priv->ethtool_priv_flags & NSS_DP_MIRR_ANALYSIS_EG_ENABLE) {
			/*
			 * If current port is already set for ingress/egress mirror analysis
			 * then user can not set current port to mirror port
			 */
			netdev_info(dev, "Port %s is set for imirror analysis port, can not set ingress mirror port\n",
					dev->name);
			return -EINVAL;
		}
		err = ppe_drv_dp_set_mirror_if(iface, PPE_DRV_DP_MIRR_DI_IN,
				(NSS_DP_MIRR_IN_ENABLE & flags));
		if (err != PPE_DRV_RET_SUCCESS) {
			netdev_info(dev, "Failed to set ingress mirror to %s\n",
					(NSS_DP_MIRR_IN_ENABLE & flags ? "enable" : "disable"));
			return -EIO;
		}

		/*
		 * toggle mirror ingress bit in private flag
		 */
		change_bit(NSS_DP_MIRR_IN_FLG_BIT,
				(unsigned long *)&(dp_priv->ethtool_priv_flags));
		break;

	case NSS_DP_MIRR_EG_ENABLE:
		/*
		 * Set egress mirror interface
		 */
		if (dp_priv->ethtool_priv_flags & NSS_DP_MIRR_ANALYSIS_IN_ENABLE ||
				dp_priv->ethtool_priv_flags & NSS_DP_MIRR_ANALYSIS_EG_ENABLE) {
			/*
			 * If current port is already set for ingress/egress mirror analysis
			 * then user can not set current port to mirror port
			 */
			netdev_info(dev, "Port %s is set for imirror analysis port, can not set egress mirror port\n",
					dev->name);
			return -EINVAL;
		}

		err = ppe_drv_dp_set_mirror_if(iface, PPE_DRV_DP_MIRR_DI_EG,
				(NSS_DP_MIRR_EG_ENABLE & flags));
		if (err != PPE_DRV_RET_SUCCESS) {
			netdev_info(dev, "Failed to set egress mirror to %s\n",
					(NSS_DP_MIRR_EG_ENABLE & flags ? "enable" : "disable"));
			return -EIO;
		}

		/*
		 * toggle mirror egress bit in private flag
		 */
		change_bit(NSS_DP_MIRR_EG_FLG_BIT,
				(unsigned long *)&(dp_priv->ethtool_priv_flags));
		break;

	case NSS_DP_MIRR_ANALYSIS_IN_ENABLE:
		/*
		 * Set ingress mirror analysis port
		 */
		return nss_dp_set_analysis_port(dev, flags, PPE_DRV_DP_MIRR_DI_IN);

	case NSS_DP_MIRR_ANALYSIS_EG_ENABLE:
		/*
		 * Set egress mirror analysis port
		 */
		return nss_dp_set_analysis_port(dev, flags, PPE_DRV_DP_MIRR_DI_EG);

	default:
		return -EOPNOTSUPP;
	}

	return 0;
}
