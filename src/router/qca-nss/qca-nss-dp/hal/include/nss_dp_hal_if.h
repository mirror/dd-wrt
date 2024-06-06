/*
 **************************************************************************
 * Copyright (c) 2016-2017,2020-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF0
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef __NSS_DP_HAL_IF_H__
#define __NSS_DP_HAL_IF_H__

#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <uapi/linux/if_link.h>

enum nss_gmac_hal_device_type {
	GMAC_HAL_TYPE_QCOM = 0,	/* 1G GMAC type */
	GMAC_HAL_TYPE_SYN_XGMAC,/* Synopsys XGMAC type */
	GMAC_HAL_TYPE_SYN_GMAC,	/* Synopsys 1G GMAC type */
	GMAC_HAL_TYPE_MAX
};

/*
 * nss_gmac_hal_platform_data
 */
struct nss_gmac_hal_platform_data {
	struct net_device *netdev; /* Net device */
	uint32_t reg_len;	/* Register space length */
	uint32_t mactype;	/* MAC chip type */
	uint32_t macid;		/* MAC sequence id on the Chip */
};

/*
 * NSS GMAC HAL device data
 */
struct nss_gmac_hal_dev {
	void __iomem *mac_base;	/* Base address of MAC registers	*/
	uint32_t version;	/* GMAC Revision version		*/
	uint32_t drv_flags;	/* Driver specific feature flags	*/

	/*
	 * Phy related stuff
	 */
	uint32_t link_state;	/* Link status as reported by the Phy	*/
	uint32_t duplex_mode;	/* Duplex mode of the Phy		*/
	uint32_t speed;		/* Speed of the Phy			*/
	uint32_t loop_back_mode;/* Loopback status of the Phy		*/
	uint32_t phy_mii_type;	/* RGMII/SGMII/XSGMII			*/

	struct net_device *netdev;
	struct resource *memres;
	uint32_t mac_reg_len;	/* MAC Register block length		*/
	uint32_t mac_id;	/* MAC sequence id on the Chip */
	spinlock_t slock;	/* lock to protect concurrent reg access */
};

/*
 * nss_gmac_hal_ops
 */
struct nss_gmac_hal_ops {
	void* (*init)(struct nss_gmac_hal_platform_data *);
	void (*exit)(struct nss_gmac_hal_dev *);
	int32_t (*start)(struct nss_gmac_hal_dev *);
	int32_t (*stop)(struct nss_gmac_hal_dev *);
	void (*setmacaddr)(struct nss_gmac_hal_dev *, uint8_t *);
	void (*getmacaddr)(struct nss_gmac_hal_dev *, uint8_t *);
	void (*promisc)(struct nss_gmac_hal_dev *, bool enabled);
	void (*multicast)(struct nss_gmac_hal_dev *, bool enabled);
	void (*broadcast)(struct nss_gmac_hal_dev *, bool enabled);
	void (*rxcsumoffload)(struct nss_gmac_hal_dev *, bool enabled);
	void (*txcsumoffload)(struct nss_gmac_hal_dev *, bool enabled);
	void (*rxflowcontrol)(struct nss_gmac_hal_dev *, bool enabled);
	void (*txflowcontrol)(struct nss_gmac_hal_dev *, bool enabled);
	int32_t (*getstats)(struct nss_gmac_hal_dev *);
	int32_t (*setmaxframe)(struct nss_gmac_hal_dev *, uint32_t);
	int32_t (*getmaxframe)(struct nss_gmac_hal_dev *);
	int32_t (*getndostats)(struct nss_gmac_hal_dev *,
			    struct rtnl_link_stats64 *);
	void (*sendpause)(struct nss_gmac_hal_dev *);
	void (*stoppause)(struct nss_gmac_hal_dev *);
	int32_t (*getssetcount)(struct nss_gmac_hal_dev *, int32_t);
	int32_t (*getstrings)(struct nss_gmac_hal_dev *, int32_t, uint8_t *);
	int32_t (*getethtoolstats)(struct nss_gmac_hal_dev *, uint64_t *, struct nss_dp_gmac_stats *);
};

extern struct nss_gmac_hal_ops qcom_gmac_ops;
extern struct nss_gmac_hal_ops syn_gmac_ops;

/**********************************************************
 * Common functions
 **********************************************************/
/*
 * hal_read_reg()
 */
static inline uint32_t hal_read_reg(void __iomem *regbase, uint32_t regoffset)
{
	return (uint32_t)readl(regbase + regoffset);
}

/*
 * hal_write_reg()
 */
static inline void hal_write_reg(void __iomem *regbase, uint32_t regoffset,
								uint32_t val)
{
	writel(val, regbase + regoffset);
}

/*
 * hal_read_relaxed_reg()
 */
static inline uint32_t hal_read_relaxed_reg(void __iomem *regbase,
							uint32_t regoffset)
{
	return readl_relaxed(regbase + regoffset);
}

/*
 * hal_write_relaxed_reg()
 */
static inline void hal_write_relaxed_reg(void __iomem *regbase,
						uint32_t regoffset,
						uint32_t regdata)
{
	writel_relaxed(regdata, regbase + regoffset);
}

/*
 * hal_set_reg_bits()
 */
static inline void hal_set_reg_bits(void __iomem *regbase,
				    uint32_t regoffset,
				    uint32_t bitpos)
{
	uint32_t data;

	data = bitpos | hal_read_relaxed_reg(regbase, regoffset);
	hal_write_relaxed_reg(regbase, regoffset, data);
}

/*
 * hal_clear_reg_bits()
 */
static inline void hal_clear_reg_bits(void __iomem *regbase,
				      uint32_t regoffset,
				      uint32_t bitpos)
{
	uint32_t data;

	data = ~bitpos & hal_read_relaxed_reg(regbase, regoffset);
	hal_write_relaxed_reg(regbase, regoffset, data);
}

/*
 * hal_check_reg_bits()
 */
static inline bool hal_check_reg_bits(void __iomem *regbase,
				      uint32_t regoffset,
				      uint32_t bitpos)
{
	return (bitpos & hal_read_relaxed_reg(regbase, regoffset)) != 0;
}

#endif /* __NSS_DP_HAL_IF_H__ */
