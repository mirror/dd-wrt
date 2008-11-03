#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ethtool.h"

#define RAETHER_DRIVER_NAME		"raether"
#define RA_NUM_STATS 			4


static struct {
    const char str[ETH_GSTRING_LEN];
} ethtool_stats_keys[] = {
    { "statistic1" },
    { "statistic2" },
    { "statistic3" },
    { "statistic4" },
};
/*
unsigned char get_current_phy_address(void)
{
	struct net_device *cur_dev_p;
	END_DEVICE *ei_local;
	for(cur_dev_p=dev_base; cur_dev_p!=NULL; cur_dev_p=cur_dev_p->next){
		if (strncmp(cur_dev_p->name, DEV_NAME , 4) == 0)
			break;
	}
	if(!cur_dev_p)
		return 0;
	ei_local = cur_dev_p->priv;
	return ei_local->mii_info.phy_id;
}
*/
static u32 et_get_tx_csum(struct net_device *dev)
{
	return (sysRegRead(GDMA1_FWD_CFG) & RT2880_GDM1_DISCRC) ? 0 : 1;	// a pitfall here, "0" means to enable.
}

static u32 et_get_rx_csum(struct net_device *dev)
{
	return (sysRegRead(GDMA1_FWD_CFG) & RT2880_GDM1_STRPCRC) ? 1 : 0;
}

static int et_set_tx_csum(struct net_device *dev, u32 data)
{
	int value;
	//printk("et_set_tx_csum(): data = %d\n", data);

	value = sysRegRead(GDMA1_FWD_CFG);
	if(data)
		value |= RT2880_GDM1_DISCRC;
	else
		value &= ~RT2880_GDM1_DISCRC;

	sysRegWrite(GDMA1_FWD_CFG, value);
    return 0;
}

static int et_set_rx_csum(struct net_device *dev, u32 data)
{
	int value;
	//printk("et_set_rx_csum(): data = %d\n", data);

	value = sysRegRead(GDMA1_FWD_CFG);
	if(data)
		value |= RT2880_GDM1_STRPCRC;
	else
		value &= ~RT2880_GDM1_STRPCRC;

	sysRegWrite(GDMA1_FWD_CFG, value);
    return 0;
}


#define MII_CR_ADDR			0x00
#define MII_CR_MR_AUTONEG_ENABLE	(1 << 12)
#define MII_CR_MR_RESTART_NEGOTIATION	(1 << 9)

#define AUTO_NEGOTIATION_ADVERTISEMENT	0x04
#define AN_PAUSE			(1 << 10)
static void et_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	int mii_an_reg;
	int mdio_cfg_reg;
	END_DEVICE *ei_local = dev->priv;

	// get mii auto-negotiation register
	mii_mgr_read(ei_local->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, &mii_an_reg);
	epause->autoneg = (mii_an_reg & AN_PAUSE) ? 1 : 0; //get autonet_enable flag bit
	
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	epause->tx_pause = (mdio_cfg_reg & RT2880_MDIO_CFG_GP1_FC_TX) ? 1 : 0;
	epause->rx_pause = (mdio_cfg_reg & RT2880_MDIO_CFG_GP1_FC_RX) ? 1 : 0;

	//printk("et_get_pauseparam(): autoneg=%d, tx_pause=%d, rx_pause=%d\n", epause->autoneg, epause->tx_pause, epause->rx_pause);
}

static int et_set_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
	int mdio_cfg_reg;
	int mii_an_reg;
	END_DEVICE *ei_local = dev->priv;

	//printk("et_set_pauseparam(): autoneg=%d, tx_pause=%d, rx_pause=%d\n", epause->autoneg, epause->tx_pause, epause->rx_pause);

	// auto-neg pause
	mii_mgr_read(ei_local->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, &mii_an_reg);
	if(epause->autoneg)
		mii_an_reg |= AN_PAUSE;
	else
		mii_an_reg &= ~AN_PAUSE;
	mii_mgr_write(ei_local->mii_info.phy_id, AUTO_NEGOTIATION_ADVERTISEMENT, mii_an_reg);

	// tx/rx pause
	mdio_cfg_reg = sysRegRead(MDIO_CFG);
	if(epause->tx_pause)
		mdio_cfg_reg |= RT2880_MDIO_CFG_GP1_FC_TX;
	else
		mdio_cfg_reg &= ~RT2880_MDIO_CFG_GP1_FC_TX;
	if(epause->rx_pause)
		mdio_cfg_reg |= RT2880_MDIO_CFG_GP1_FC_RX;
	else
		mdio_cfg_reg &= ~RT2880_MDIO_CFG_GP1_FC_RX;
	sysRegWrite(MDIO_CFG, mdio_cfg_reg);

	return 0;
}

static int et_nway_reset(struct net_device *dev)
{
	END_DEVICE *ei_local = dev->priv;
	return mii_nway_restart(&ei_local->mii_info);
}

static u32 et_get_link(struct net_device *dev)
{
	END_DEVICE *ei_local = dev->priv;
	return mii_link_ok(&ei_local->mii_info);
}

static int et_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	END_DEVICE *ei_local = dev->priv;
	int rc;
	rc = mii_ethtool_sset(&ei_local->mii_info, cmd);
	return rc;
}

static int et_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	END_DEVICE *ei_local = dev->priv;
	mii_ethtool_gset(&ei_local->mii_info, cmd);
	return 0;
}

static u32 et_get_msglevel(struct net_device *dev)
{
	return 0;
}

static void et_set_msglevel(struct net_device *dev, u32 datum)
{
	return;
}

static void et_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	//END_DEVICE *ei_local = dev->priv;
	strcpy(info->driver, RAETHER_DRIVER_NAME);
	strcpy(info->version, RAETH_VERSION);
	strcpy(info->bus_info, "n/a");
	info->n_stats = RA_NUM_STATS;
	info->eedump_len = 0;
	info->regdump_len = 0;
}

static int et_get_stats_count(struct net_device *dev)
{
	return RA_NUM_STATS;
}

static void et_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
//	END_DEVICE *ei_local = dev->priv;
	data[0] = 0;//np->xstats.early_rx;
	data[1] = 0;//np->xstats.tx_buf_mapped;
	data[2] = 0;//np->xstats.tx_timeouts;
	data[3] = 0;//np->xstats.rx_lost_in_ring;
}

static void et_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	memcpy(data, ethtool_stats_keys, sizeof(ethtool_stats_keys));
}

struct ethtool_ops ra_ethtool_ops = {
	.get_drvinfo		= et_get_drvinfo,
	.get_settings		= et_get_settings,
	.set_settings		= et_set_settings,
	.get_pauseparam		= et_get_pauseparam,
	.set_pauseparam		= et_set_pauseparam,
	.get_rx_csum		= et_get_rx_csum,
	.set_rx_csum		= et_set_rx_csum,
	.get_tx_csum		= et_get_tx_csum,
	.set_tx_csum		= et_set_tx_csum,
	.nway_reset		= et_nway_reset,
	.get_link		= et_get_link,
	.get_msglevel		= et_get_msglevel,
	.set_msglevel		= et_set_msglevel,
	.get_strings		= et_get_strings,
	.get_stats_count	= et_get_stats_count,
	.get_ethtool_stats	= et_get_ethtool_stats,
/*	.get_regs_len		= et_get_regs_len,
	.get_regs		= et_get_regs,
*/
};
