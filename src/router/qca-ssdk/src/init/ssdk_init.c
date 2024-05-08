/*
 * Copyright (c) 2012, 2014-2015, The Linux Foundation. All rights reserved.
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
#ifdef IN_MISC
#include "fal_misc.h"
#endif
#ifdef IN_MIB
#include "fal_mib.h"
#endif
#ifdef IN_PORTCONTROL
#include "fal_port_ctrl.h"
#endif
#ifdef IN_PORTVLAN
#include "fal_portvlan.h"
#endif
#ifdef IN_FDB
#include "fal_fdb.h"
#endif
#ifdef IN_STP
#include "fal_stp.h"
#endif
#ifdef IN_IGMP
#include "fal_igmp.h"
#endif
#ifdef IN_QOS
#include "fal_qos.h"
#endif
#ifdef IN_LED
#include "fal_led.h"
#endif
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
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/string.h>
#if defined(ISIS) ||defined(ISISC) ||defined(GARUDA)
#include <f1_phy.h>
#endif
#if defined(ATHENA) ||defined(SHIVA) ||defined(HORUS)
#include <f2_phy.h>
#endif
#ifdef IN_MALIBU_PHY
#include <malibu_phy.h>
#endif
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#include <linux/of.h>
#include <drivers/leds/leds-ipq40xx.h>
#elif defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/switch.h>
#include <linux/of.h>
#include <linux/ar8216_platform.h>
#else
#include <net/switch.h>
#include <linux/ar8216_platform.h>
#endif
#include "ssdk_plat.h"
#include "ref_vlan.h"
#include "ref_fdb.h"
#include "ref_mib.h"
#include "ref_port_ctrl.h"
#include "ref_misc.h"
#include "ref_uci.h"
#include "shell.h"
#ifdef BOARD_AR71XX
#include "ssdk_uci.h"
#endif

#ifdef IN_IP
#if defined (CONFIG_NF_FLOW_COOKIE)
#include "fal_flowcookie.h"
#include <shortcut-fe/sfe.h>
#endif
#endif

#ifdef IN_RFS
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#include <linux/if_vlan.h>
#endif
#include <qca-rfs/rfs_dev.h>
#ifdef IN_IP
#include "fal_rfs.h"
#endif
#endif
#include <linux/proc_fs.h>

#define ISIS_CHIP_ID 0x18
#define ISIS_CHIP_REG 0
#define SHIVA_CHIP_ID 0x1f
#define SHIVA_CHIP_REG 0x10

#ifdef IN_RFS
struct rfs_device rfs_dev;
struct notifier_block ssdk_inet_notifier;
#if defined(CONFIG_RFS_ACCEL)
struct notifier_block ssdk_dev_notifier;
#endif
#endif

extern ssdk_chip_type SSDK_CURRENT_CHIP_TYPE;

//#define PSGMII_DEBUG

#define AUTO_SWITCH_RECOVERY

#define QCA_QM_WORK_DELAY	200
#define QCA_QM_ITEM_NUMBER 41
#define QCA_RGMII_WORK_DELAY	1000

int ssdk_dev_id = 0;

/*
 * Using ISIS's address as default
  */
static int switch_chip_id = ISIS_CHIP_ID;
static int switch_chip_reg = ISIS_CHIP_REG;
ssdk_dt_cfg ssdk_dt_global = {0};
u8  __iomem      *hw_addr = NULL;
u8  __iomem      *psgmii_hw_addr = NULL;

a_uint32_t ssdk_dt_global_get_mac_mode(void)
{
	return ssdk_dt_global.mac_mode;
}

//lxx
struct proc_dir_entry {
	unsigned int low_ino;
	umode_t mode;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	const struct inode_operations *proc_iops;
	const struct file_operations *proc_fops;
	struct proc_dir_entry *next, *parent, *subdir;
	void *data;
	atomic_t count;         /* use count */
	atomic_t in_use;        /* number of callers into module in progress; */
	/* negative -> it's going away RSN */
	struct completion *pde_unload_completion;
	struct list_head pde_openers;   /* who did ->open, but not ->release */
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	u8 namelen;
	char name[];
};

struct switch_dev *sw_dev_dni;
struct qca_phy_priv *priv_dni=NULL;
static struct mii_bus *miibus = NULL;
static struct mii_bus *miibus_slave = NULL;
static struct mutex switch_mdio_lock;
static struct mutex switch_mdio_slave_lock;
phy_identification_t phy_array[] =
{
	#ifdef IN_MALIBU_PHY
	{0x0, 0x004DD0B1, malibu_phy_init},
	{0x0, 0x004DD0B2, malibu_phy_init},
	#endif
	#if defined(ISIS) ||defined(ISISC) ||defined(GARUDA)
	{0x0, 0x004DD036, f1_phy_init},
	{0x0, 0x004DD033, f1_phy_init},
	#endif
	#if defined(ATHENA) ||defined(SHIVA) ||defined(HORUS)
	{0x0, 0x004DD042, f2_phy_init}
	#endif
};

static void
qca_phy_read_port_link(struct qca_phy_priv *priv, int port,
		      struct switch_port_link *port_link)
{
	a_uint32_t port_status, port_speed;

	memset(port_link, 0, sizeof(*port_link));

	port_status = priv->mii_read(AR8327_REG_PORT_STATUS(port));

    port_link->link = 1;
    port_link->aneg = !!(port_status & AR8327_PORT_STATUS_LINK_AUTO);
	if (port_link->aneg) {
		port_link->link = !!(port_status & AR8327_PORT_STATUS_LINK_UP);
		if (port_link->link == 0) {
			return;
        }
	}

	port_speed = (port_status & AR8327_PORT_STATUS_SPEED) >>
		            AR8327_PORT_STATUS_SPEED_S;
    if(port_speed == AR8327_PORT_SPEED_10M) {
        port_link->speed = SWITCH_PORT_SPEED_10;
    } else if(port_speed == AR8327_PORT_SPEED_100M) {
        port_link->speed = SWITCH_PORT_SPEED_100;
    } else if(port_speed == AR8327_PORT_SPEED_1000M) {
        port_link->speed = SWITCH_PORT_SPEED_1000;
    } else {
        port_link->speed = SWITCH_PORT_SPEED_UNKNOWN;
    }

	port_link->duplex = !!(port_status & AR8327_PORT_STATUS_DUPLEX);
	port_link->tx_flow = !!(port_status & AR8327_PORT_STATUS_TXFLOW);
	port_link->rx_flow = !!(port_status & AR8327_PORT_STATUS_RXFLOW);
}

#ifndef BOARD_AR71XX
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
static void
ssdk_phy_rgmii_set(struct qca_phy_priv *priv)
{
	struct device_node *np = NULL;
	u32 rgmii_en = 0, tx_delay = 0, rx_delay = 0;

	np = priv->phy->dev.of_node;
	if (!np)
		return;

	if (!of_property_read_u32(np, "phy_rgmii_en", &rgmii_en)) {
		a_uint16_t val = 0;
		/*enable RGMII  mode */
		qca_ar8327_phy_dbg_read(0, AR8327_PORT5_PHY_ADDR,
				AR8327_PHY_REG_MODE_SEL, &val);
		val |= AR8327_PHY_RGMII_MODE;
		qca_ar8327_phy_dbg_write(0, AR8327_PORT5_PHY_ADDR,
				AR8327_PHY_REG_MODE_SEL, val);
		if (!of_property_read_u32(np, "txclk_delay_en", &tx_delay)
				&& tx_delay == 1) {
			qca_ar8327_phy_dbg_read(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_SYS_CTRL, &val);
			val |= AR8327_PHY_RGMII_TX_DELAY;
			qca_ar8327_phy_dbg_write(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_SYS_CTRL, val);
		}
		if (!of_property_read_u32(np, "rxclk_delay_en", &rx_delay)
				&& rx_delay == 1) {
			qca_ar8327_phy_dbg_read(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_TEST_CTRL, &val);
			val |= AR8327_PHY_RGMII_RX_DELAY;
			qca_ar8327_phy_dbg_write(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_TEST_CTRL, val);
		}
	}
}
#else
static void
ssdk_phy_rgmii_set(struct qca_phy_priv *priv)
{
	struct ar8327_platform_data *plat_data;

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,8,0)
	plat_data = priv->phy->mdio.dev.platform_data;
#else
	plat_data = priv->phy->dev.platform_data;
#endif
	if (plat_data == NULL) {
		return;
	}

	if(plat_data->pad5_cfg) {
		if(plat_data->pad5_cfg->mode == AR8327_PAD_PHY_RGMII) {
			a_uint16_t val = 0;
			/*enable RGMII  mode */
			priv->phy_dbg_read(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_MODE_SEL, &val);
			val |= AR8327_PHY_RGMII_MODE;
			priv->phy_dbg_write(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_MODE_SEL, val);
			if(plat_data->pad5_cfg->txclk_delay_en) {
				priv->phy_dbg_read(0, AR8327_PORT5_PHY_ADDR,
						AR8327_PHY_REG_SYS_CTRL, &val);
				val |= AR8327_PHY_RGMII_TX_DELAY;
				priv->phy_dbg_write(0, AR8327_PORT5_PHY_ADDR,
						AR8327_PHY_REG_SYS_CTRL, val);
			}
			if(plat_data->pad5_cfg->rxclk_delay_en) {
				priv->phy_dbg_read(0, AR8327_PORT5_PHY_ADDR,
						AR8327_PHY_REG_TEST_CTRL, &val);
				val |= AR8327_PHY_RGMII_RX_DELAY;
				priv->phy_dbg_write(0, AR8327_PORT5_PHY_ADDR,
						AR8327_PHY_REG_TEST_CTRL, val);
			}
		}
	}
}
#endif
#endif


static void
qca_ar8327_phy_fixup(struct qca_phy_priv *priv, int phy)
{
	switch (priv->revision) {
	case 1:
		/* 100m waveform */
		priv->phy_dbg_write(0, phy, 0, 0x02ea);
		/* turn on giga clock */
		priv->phy_dbg_write(0, phy, 0x3d, 0x68a0);
		break;

	case 2:
		priv->phy_mmd_write(0, phy, 0x7, 0x3c);
		priv->phy_mmd_write(0, phy, 0x4007, 0x0);
		/* fallthrough */
	case 4:
		priv->phy_mmd_write(0, phy, 0x3, 0x800d);
		priv->phy_mmd_write(0, phy, 0x4003, 0x803f);

		priv->phy_dbg_write(0, phy, 0x3d, 0x6860);
		priv->phy_dbg_write(0, phy, 0x5, 0x2c46);
		priv->phy_dbg_write(0, phy, 0x3c, 0x6000);
		break;
	}
}

sw_error_t
qca_switch_init_master(a_uint32_t dev_id)
{
	fal_acl_rule_t rule;

	/*master switch, port 4 and port 6 in trunk*/
	fal_trunk_group_set(dev_id, 0, A_TRUE, 0x50);

	/*Enable trunk port atheros header*/
	fal_header_type_set(dev_id, A_TRUE, 0xffff);
	fal_port_txhdr_mode_set(dev_id, 5, FAL_ONLY_MANAGE_FRAME_EN);
	fal_port_rxhdr_mode_set(dev_id, 5, FAL_ONLY_MANAGE_FRAME_EN);
	fal_port_txhdr_mode_set(dev_id, 4, FAL_ONLY_MANAGE_FRAME_EN);
	fal_port_txhdr_mode_set(dev_id, 6, FAL_ONLY_MANAGE_FRAME_EN);

	fal_acl_port_udf_profile_set(dev_id, 5, FAL_ACL_UDF_TYPE_L2, 12, 2);
	fal_acl_port_udf_profile_set(dev_id, 4, FAL_ACL_UDF_TYPE_L2, 12, 2);
	fal_acl_port_udf_profile_set(dev_id, 6, FAL_ACL_UDF_TYPE_L2, 12, 2);

	/*list 1 rule for redirect packet from switch2 to CPU port 2 with atheros header type 0xfefe*/
	fal_acl_list_creat(dev_id, 1, 1);
	memset(&rule, 0, sizeof(fal_acl_rule_t));
	rule.rule_type = FAL_ACL_RULE_UDF;
	rule.udf_len = 2;
	rule.udf_val[0] = 0xfe;
	rule.udf_val[1] = 0xfe;
	rule.udf_mask[0] = 0xff;
	rule.udf_mask[1] = 0xff;
	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_UDF);
	rule.ports = 0x20;
	FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_REDPT);
	fal_acl_rule_add(dev_id, 1, 0, 1, &rule);
	fal_acl_list_bind(dev_id, 1, 0, 0, 4);
	fal_acl_list_bind(dev_id, 1, 0, 0, 6);

	/*list 2 rule for redirect packet from linux stack to trunk port with Atheros header type 0xfefe*/
	fal_acl_list_creat(dev_id, 2, 2);
	memset(&rule, 0, sizeof(fal_acl_rule_t));
	rule.rule_type = FAL_ACL_RULE_UDF;
	rule.udf_len = 2;
	rule.udf_val[0] = 0xfe;
	rule.udf_val[1] = 0xfe;
	rule.udf_mask[0] = 0xff;
	rule.udf_mask[1] = 0xff;
	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_UDF);
	rule.ports = 0x10;
	FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_REDPT);
	fal_acl_rule_add(dev_id, 2, 0, 1, &rule);
	fal_acl_list_bind(dev_id, 2, 0, 0, 5);

	fal_acl_status_set(dev_id, A_TRUE);

	return SW_OK;
}

sw_error_t
qca_switch_init_slave(a_uint32_t dev_id)
{
	/*slave switch, port 0 and port 5 in trunk*/
	fal_trunk_group_set(dev_id, 0, A_TRUE, 0x21);

	/*Enable trunk port atheros header*/
	fal_header_type_set(dev_id, A_TRUE, 0xfefe);
	fal_port_txhdr_mode_set(dev_id, 0, FAL_ONLY_MANAGE_FRAME_EN);
	fal_port_rxhdr_mode_set(dev_id, 0, FAL_ONLY_MANAGE_FRAME_EN);
	fal_port_txhdr_mode_set(dev_id, 6, FAL_ONLY_MANAGE_FRAME_EN);
	fal_port_rxhdr_mode_set(dev_id, 6, FAL_ONLY_MANAGE_FRAME_EN);

	return SW_OK;
}

sw_error_t
qca_switch_init(a_uint32_t dev_id)
{
	a_uint32_t nr = 0;
	int i = 0;

	/*fal_reset(dev_id);*/
	/*enable cpu and disable mirror*/
	#ifdef IN_MISC
	fal_cpu_port_status_set(dev_id, A_TRUE);
	/* setup MTU */
	fal_frame_max_size_set(dev_id, 1518);
	#endif
	#ifdef IN_MIB
	/* Enable MIB counters */
	fal_mib_status_set(dev_id, A_TRUE);
	#endif
	#ifdef IN_IGMP
	fal_igmp_mld_rp_set(dev_id, 0);
	#endif

	/*enable pppoe for dakota to support RSS*/
	if (SSDK_CURRENT_CHIP_TYPE == CHIP_DESS) {
		#ifdef DESS
		#ifdef IN_MISC
		fal_pppoe_status_set(dev_id, A_TRUE);
		#endif
		#endif
	}

	for (i = 0; i < AR8327_NUM_PORTS; i++) {
		/* forward multicast and broadcast frames to CPU */
		#ifdef IN_MISC
		fal_port_unk_uc_filter_set(dev_id, i, A_FALSE);
		fal_port_unk_mc_filter_set(dev_id, i, A_FALSE);
		fal_port_bc_filter_set(dev_id, i, A_FALSE);
		#endif
		#ifdef IN_PORTVLAN
		fal_port_default_svid_set(dev_id, i, 0);
		fal_port_default_cvid_set(dev_id, i, 0);
		fal_port_1qmode_set(dev_id, i, FAL_1Q_DISABLE);
		fal_port_egvlanmode_set(dev_id, i, FAL_EG_UNMODIFIED);
		#endif

		#ifdef IN_FDB
		fal_fdb_port_learn_set(dev_id, i, A_TRUE);
		#endif
		#ifdef IN_STP
		fal_stp_port_state_set(dev_id, 0, i, FAL_STP_FARWARDING);
		#endif
		#ifdef IN_PORTVLAN
		fal_port_vlan_propagation_set(dev_id, i, FAL_VLAN_PROPAGATION_REPLACE);
		#endif
		#ifdef IN_IGMP
		fal_port_igmps_status_set(dev_id, i, A_FALSE);
		fal_port_igmp_mld_join_set(dev_id, i, A_FALSE);
		fal_port_igmp_mld_leave_set(dev_id, i, A_FALSE);
		fal_igmp_mld_entry_creat_set(dev_id, A_FALSE);
		fal_igmp_mld_entry_v3_set(dev_id, A_FALSE);
		#endif
		if (SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA) {
			return SW_OK;
		} else if (SSDK_CURRENT_CHIP_TYPE == CHIP_DESS) {
			#ifdef DESS
			#ifdef IN_PORTCONTROL
			fal_port_flowctrl_forcemode_set(dev_id, i, A_FALSE);
			#endif
			#ifdef IN_QOS
			nr = 240; /*30*8*/
			fal_qos_port_tx_buf_nr_set(dev_id, i, &nr);
			nr = 48; /*6*8*/
			fal_qos_port_rx_buf_nr_set(dev_id, i, &nr);
			fal_qos_port_red_en_set(dev_id, i, A_TRUE);
			nr = 32;
			fal_qos_queue_tx_buf_nr_set(dev_id, i, 5, &nr);
			fal_qos_queue_tx_buf_nr_set(dev_id, i, 4, &nr);
			fal_qos_queue_tx_buf_nr_set(dev_id, i, 3, &nr);
			fal_qos_queue_tx_buf_nr_set(dev_id, i, 2, &nr);
			fal_qos_queue_tx_buf_nr_set(dev_id, i, 1, &nr);
			fal_qos_queue_tx_buf_nr_set(dev_id, i, 0, &nr);
			#endif
			#endif
		} else if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISISC ||
			SSDK_CURRENT_CHIP_TYPE == CHIP_ISIS) {
			#if defined(ISISC) || defined(ISIS)
			#ifdef IN_INTERFACECONTROL
			fal_port_3az_status_set(dev_id, i, A_FALSE);
			#endif
			#ifdef IN_PORTCONTROL
			fal_port_flowctrl_forcemode_set(dev_id, i, A_TRUE);
			fal_port_flowctrl_set(dev_id, i, A_FALSE);

			if (i != 0 && i != 6) {
				fal_port_flowctrl_set(dev_id, i, A_TRUE);
				fal_port_flowctrl_forcemode_set(dev_id, i, A_FALSE);
			}
			#endif
			if (i == 0 || i == 5 || i == 6) {
				#ifdef IN_QOS
				nr = 240; /*30*8*/
				fal_qos_port_tx_buf_nr_set(dev_id, i, &nr);
				nr = 48; /*6*8*/
				fal_qos_port_rx_buf_nr_set(dev_id, i, &nr);
				fal_qos_port_red_en_set(dev_id, i, A_TRUE);
				if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISISC) {
					nr = 64; /*8*8*/
				} else if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISIS) {
					nr = 60;
				}
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 5, &nr);
				nr = 48; /*6*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 4, &nr);
				nr = 32; /*4*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 3, &nr);
				nr = 32; /*4*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 2, &nr);
				nr = 32; /*4*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 1, &nr);
				nr = 24; /*3*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 0, &nr);
				#endif
			} else {
				#ifdef IN_QOS
				nr = 200; /*25*8*/
				fal_qos_port_tx_buf_nr_set(dev_id, i, &nr);
				nr = 48; /*6*8*/
				fal_qos_port_rx_buf_nr_set(dev_id, i, &nr);
				fal_qos_port_red_en_set(dev_id, i, A_TRUE);
				if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISISC) {
					nr = 64; /*8*8*/
				} else if (SSDK_CURRENT_CHIP_TYPE == CHIP_ISIS) {
					nr = 60;
				}
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 3, &nr);
				nr = 48; /*6*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 2, &nr);
				nr = 32; /*4*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 1, &nr);
				nr = 24; /*3*8*/
				fal_qos_queue_tx_buf_nr_set(dev_id, i, 0, &nr);
				#endif
			}
			#endif
		}
	}

	/*setup master and slave switch*/
	if (dev_id == 0)
		qca_switch_init_master(dev_id);
	if (dev_id == 1)
		qca_switch_init_slave(dev_id);

	return SW_OK;
}

void
qca_ar8327_phy_disable(void)
{
	int i = 0;
	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		/* power down all phy*/
		qca_ar8327_phy_write(0, i, MII_BMCR, BMCR_PDOWN);
	}
}

void
qca_mac_disable(void)
{
	hsl_api_t *p_api;

	p_api = hsl_api_ptr_get (0);
	if(p_api
		&& p_api->interface_mac_pad_set
		&& p_api->interface_mac_sgmii_set)
	{
		p_api->interface_mac_pad_set(0,0,0);
		p_api->interface_mac_pad_set(0,5,0);
		p_api->interface_mac_pad_set(0,6,0);
		p_api->interface_mac_sgmii_set(0,AR8327_REG_PAD_SGMII_CTRL_HW_INIT);
	}
	else
	{
		printk("API not support \n");
	}
}

void
qca_switch_mac_reset(struct qca_phy_priv *priv)
{
	a_uint32_t value, value0, i;
	if (priv == NULL || (priv->mii_read == NULL) || (priv->mii_write == NULL)) {
		printk("In qca_switch_mac_reset, private data is NULL!\r\n");
		return;
	}

	for (i=0; i < AR8327_NUM_PORTS; ++i) {
		/* b3:2=0,Tx/Rx Mac disable,
		 b9=0,LINK_EN disable */
		value0 = priv->mii_read(AR8327_REG_PORT_STATUS(i));
		value = value0 & ~(AR8327_PORT_STATUS_LINK_AUTO |
						AR8327_PORT_STATUS_TXMAC |
						AR8327_PORT_STATUS_RXMAC);
		priv->mii_write(AR8327_REG_PORT_STATUS(i), value);

		/* Force speed to 1000M Full */
		value = priv->mii_read(AR8327_REG_PORT_STATUS(i));
		value &= ~(AR8327_PORT_STATUS_DUPLEX | AR8327_PORT_STATUS_SPEED);
		value |= AR8327_PORT_SPEED_1000M | AR8327_PORT_STATUS_DUPLEX;
		priv->mii_write(AR8327_REG_PORT_STATUS(i), value);
	}
	return;
}

void
qca_ar8327_phy_enable(struct qca_phy_priv *priv)
{
	int i = 0;
	#ifndef BOARD_AR71XX
        ssdk_phy_rgmii_set(priv);
        #endif
	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		if (priv->version == QCA_VER_AR8327)
			qca_ar8327_phy_fixup(priv, i);

		/* start autoneg*/
		priv->phy_write(0, i, MII_ADVERTISE, ADVERTISE_ALL |
						     ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		priv->phy_write(0, i, MII_CTRL1000, ADVERTISE_1000FULL);
		priv->phy_write(0, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}
}

#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
static int
qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	struct device_node *np = NULL;
	const __be32 *paddr;
	a_uint32_t reg, value, i;
	a_int32_t len;

	np = priv->phy->dev.of_node;
	if(!np)
		return -EINVAL;

	/*Before switch software reset, disable PHY and clear  MAC PAD*/
	qca_ar8327_phy_disable();
	qca_mac_disable();
	msleep(1000);

	/*First software reset S17 chip*/
	value = priv->mii_read(AR8327_REG_CTRL);
	value |= 0x80000000;
	priv->mii_write(AR8327_REG_CTRL, value);

	/*After switch software reset, need disable all ports' MAC with 1000M FULL*/
	qca_switch_mac_reset(priv);
	/* Configure switch register from DT information */
	paddr = of_get_property(np, "qca,ar8327-initvals", &len);
	if (!paddr || len < (2 * sizeof(*paddr))) {
		printk("len:%d < 2 * sizeof(*paddr):%d\n", len, 2 * sizeof(*paddr));
		return -EINVAL;
	}

	len /= sizeof(*paddr);

	for (i = 0; i < len - 1; i += 2) {
		reg = be32_to_cpup(paddr + i);
		value = be32_to_cpup(paddr + i + 1);
		priv->mii_write(reg, value);
	}

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(0));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(0), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(1));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(1), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(2));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(2), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(3));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(3), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(4));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(4), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(5));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(5), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(6));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(6), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(0), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(1), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(2), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(3), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(4), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(5), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(6), value);

	qca_switch_init(0);

	qca_ar8327_phy_enable(priv);

	return 0;
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
struct qca_phy_priv *qca_phy_priv_group[AR8327_NUM_CHIPS];
const struct switch_dev_ops qca_ar8327_sw_ops;
void
qca_ar8327_cascade_phy_disable(struct qca_phy_priv *priv)
{
	int i = 0;
	/*disable master*/
	for (i = 0; i < 3; i++) {
		/* power down all phy*/
		qca_ar8327_phy_write(0, i, MII_BMCR, BMCR_PDOWN);
	}
	/*disable slave*/
	for (i = 0; i < 4; i++) {
		/* power down all phy*/
		qca_ar8327_phy_write(1, i, MII_BMCR, BMCR_PDOWN);
	}
}

void
qca_cascade_mac_disable(void)
{
	hsl_api_t *p_api;

	/*disable master*/
	p_api = hsl_api_ptr_get (0);
	if(p_api
		&& p_api->interface_mac_pad_set
		&& p_api->interface_mac_sgmii_set)
	{
		p_api->interface_mac_pad_set(0,0,0);
		p_api->interface_mac_pad_set(0,5,0);
		p_api->interface_mac_pad_set(0,6,0);
		p_api->interface_mac_sgmii_set(0,AR8327_REG_PAD_SGMII_CTRL_HW_INIT);
	}

	/*disable slave*/
	p_api = hsl_api_ptr_get (1);
	if(p_api
		&& p_api->interface_mac_pad_set
		&& p_api->interface_mac_sgmii_set)
	{
		p_api->interface_mac_pad_set(0,0,0);
		p_api->interface_mac_sgmii_set(0,AR8327_REG_PAD_SGMII_CTRL_HW_INIT);
	}

}

void
qca_cascade_switch_mac_reset(struct qca_phy_priv *priv)
{
	a_uint32_t value, value0, i;
	if (priv == NULL || (priv->mii_read == NULL) || (priv->mii_write == NULL)) {
		printk("In qca_switch_mac_reset, private data is NULL!\r\n");
		return;
	}

	for (i=0; i < AR8327_NUM_PORTS; ++i) {
		/* b3:2=0,Tx/Rx Mac disable,
		 b9=0,LINK_EN disable */
		value0 = priv->mii_read(AR8327_REG_PORT_STATUS(i));
		value = value0 & ~(AR8327_PORT_STATUS_LINK_AUTO |
						AR8327_PORT_STATUS_TXMAC |
						AR8327_PORT_STATUS_RXMAC);
		priv->mii_write(AR8327_REG_PORT_STATUS(i), value);

		/* Force speed to 1000M Full */
		value = priv->mii_read(AR8327_REG_PORT_STATUS(i));
		value &= ~(AR8327_PORT_STATUS_DUPLEX | AR8327_PORT_STATUS_SPEED);
		value |= AR8327_PORT_SPEED_1000M | AR8327_PORT_STATUS_DUPLEX;
		priv->mii_write(AR8327_REG_PORT_STATUS(i), value);
	}
	/*reset slave mac*/
	for (i=0; i < AR8327_NUM_PORTS-1; ++i) {
		/* b3:2=0,Tx/Rx Mac disable,
		 b9=0,LINK_EN disable */
		value0 = qca_ar8216_mii_slave_read(AR8327_REG_PORT_STATUS(i));
		value = value0 & ~(AR8327_PORT_STATUS_LINK_AUTO |
						AR8327_PORT_STATUS_TXMAC |
						AR8327_PORT_STATUS_RXMAC);
		qca_ar8216_mii_slave_write(AR8327_REG_PORT_STATUS(i), value);

		/* Force speed to 1000M Full */
		value = qca_ar8216_mii_slave_read(AR8327_REG_PORT_STATUS(i));
		value &= ~(AR8327_PORT_STATUS_DUPLEX | AR8327_PORT_STATUS_SPEED);
		value |= AR8327_PORT_SPEED_1000M | AR8327_PORT_STATUS_DUPLEX;
		qca_ar8216_mii_slave_write(AR8327_REG_PORT_STATUS(i), value);
	}
	return;
}

void
qca_cascade_switch_mac_config(struct qca_phy_priv *priv)
{
	hsl_api_t *p_api;
	/*pad0 pad5 pad6 sgmii control, pos config, master*/
	p_api = hsl_api_ptr_get (0);
	if(p_api
		&& p_api->interface_mac_pad_set
		&& p_api->interface_mac_sgmii_set)
	{
		p_api->interface_mac_pad_set(0,0,0x7600000);
		p_api->interface_mac_pad_set(0,5,0x7600000);
		p_api->interface_mac_pad_set(0,6,0x80);
		p_api->interface_mac_sgmii_set(0,0xc74164de);
		priv->mii_write(AR8327_REG_PAD_MAC_PWR_SEL, 0xaa545);
		priv->mii_write(AR8327_REG_POS, 0x2613a0);
	}
	/*pad0 sgmii control, pos config, slave*/
	qca_ar8216_mii_slave_write(AR8327_REG_PAD0_CTRL, 0x80080);
	qca_ar8216_mii_slave_write(AR8327_REG_PAD_SGMII_CTRL, 0xc74164de);
	qca_ar8216_mii_slave_write(AR8327_REG_PAD_MAC_PWR_SEL, 0xaa545);
	qca_ar8216_mii_slave_write(AR8327_REG_POS, 0x2613a0);

	/*mac0 and mac5 config, master*/
	priv->mii_write(0x7c, 0x4e);
	priv->mii_write(0x90, 0x4e);
	priv->mii_write(0x94, 0x4e);

	/*mac0, slave*/
	qca_ar8216_mii_slave_write(0x7c, 0x4e);


}

void
qca_ar8327_cascade_phy_enable(struct qca_phy_priv *priv)
{
	int i = 0;

	/*enable master*/
	for (i = 0; i < 3; i++) {
		if (priv->version == QCA_VER_AR8327)
			qca_ar8327_phy_fixup(priv, i);

		/* start autoneg*/
		priv->phy_write(0, i, MII_ADVERTISE, ADVERTISE_ALL |
						     ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		priv->phy_write(0, i, MII_CTRL1000, ADVERTISE_1000FULL);
		priv->phy_write(0, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}
	/*enable slave*/
	for (i = 0; i < 4; i++) {
		if (priv->version == QCA_VER_AR8327)
			qca_ar8327_phy_fixup(priv, i);

		/* start autoneg*/
		qca_ar8327_phy_write(1, i, MII_ADVERTISE, ADVERTISE_ALL |
						     ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		qca_ar8327_phy_write(1, i, MII_CTRL1000, ADVERTISE_1000FULL);
		qca_ar8327_phy_write(1, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}

}

static int
qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	a_uint32_t value, i;

	/*Before switch software reset, disable PHY and clear MAC PAD*/
	qca_ar8327_cascade_phy_disable(priv);
	qca_cascade_mac_disable();
	msleep(1000);

	/*First software reset S17 chip*/
	value = priv->mii_read(AR8327_REG_CTRL);
	value |= 0x80000000;
	priv->mii_write(AR8327_REG_CTRL, value);
	/*Need wait reset done*/
	do {
		udelay(10);
		value = priv->mii_read(AR8327_REG_CTRL);
	} while(value & AR8327_CTRL_RESET);
	do {
		udelay(10);
		value = priv->mii_read(0x20);
	} while ((value & SSDK_GLOBAL_INITIALIZED_STATUS) != SSDK_GLOBAL_INITIALIZED_STATUS);
	/*TODO, also reset slave */

	/*After switch software reset, need disable all ports' MAC with 1000M FULL*/
//	qca_cascade_switch_mac_reset(priv);

	/*TODO, wrapper below for both device*/
	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(0));
	value &= ~0x29;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(0), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(1));
	value &= ~0x2a;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(1), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(2));
	value &= ~0x2c;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(2), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(3));
	value &= ~0x5f;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(3), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(4));
	value &= ~0x38;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(4), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(5));
	value &= ~0x77;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(5), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(6));
	value &= ~0x58;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(6), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(0), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(1), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(2), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(3), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(4), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(5), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(6), value);

	for (i = 0; i < 2; i++)
		qca_switch_init(i);

	qca_cascade_switch_mac_config(priv);

	qca_ar8327_cascade_phy_enable(priv);

	return 0;

}

static int qca_switch_register_init(void)
{
	struct qca_phy_priv *priv;
	struct switch_dev *sw_dev;
	int ret;
	static int miibus_init = 0;

	priv = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
	if (priv == NULL) {
		return -ENOMEM;
	}

	mutex_init(&priv->reg_mutex);

	priv->mii_read = qca_ar8216_mii_read;
	priv->mii_write = qca_ar8216_mii_write;
	priv->phy_write = qca_ar8327_phy_write;
	priv->phy_dbg_write = qca_ar8327_phy_dbg_write;
	priv->phy_dbg_read = qca_ar8327_phy_dbg_read;
	priv->phy_mmd_write = qca_ar8327_mmd_write;
	qca_phy_priv_group[0] = priv;

	/*init the second switch*/
	qca_phy_priv_group[1] = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
	if (qca_phy_priv_group[1] == NULL) {
		return -ENOMEM;
	}
	mutex_init(&(qca_phy_priv_group[1]->reg_mutex));

	ret = qca_phy_id_chip(priv);

	sw_dev = &priv->sw_dev;
	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->name = "QCA AR8327 AR8337";
	sw_dev->alias = "QCA AR8327 AR8337";
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = AR8327_NUM_PORTS;

	ret = register_switch(&priv->sw_dev, NULL);
	if (ret != 0) {
		printk("register_switch switch0 fail %x\n", ret);
	        return ret;
	}
	printk("register_switch switch0 ok\n");
	/*register the second switch*/
	sw_dev = &(qca_phy_priv_group[1]->sw_dev);
	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->name = "QCA AR8327 AR8337";
	sw_dev->alias = "QCA AR8327 AR8337";
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = AR8327_NUM_PORTS;
	ret = register_switch(&qca_phy_priv_group[1]->sw_dev, NULL);
	if (ret != 0) {
		printk("register_switch switch1 fail %x\n", ret);
	        return ret;
	}
	printk("register_switch switch1 ok\n");

	ret = qca_ar8327_hw_init(priv);
	if (ret != 0) {
		printk("qca_ar8327_hw_init fail %x\n", ret);
		return ret;
	}

	qca_phy_mib_work_start(priv);
	qca_phy_mib_work_start(qca_phy_priv_group[1]);

	#ifdef AUTO_SWITCH_RECOVERY
	qm_err_check_work_start(priv);
	qm_err_check_work_start(qca_phy_priv_group[1]);
	#endif

	//lxx
	priv_dni = priv;

	return ret;
}

static void qca_switch_unregister()
{
	if (qca_phy_priv_group[0]  != NULL) {
		qca_phy_mib_work_stop(qca_phy_priv_group[0]);
#ifdef AUTO_SWITCH_RECOVERY
		qm_err_check_work_stop(qca_phy_priv_group[0]);
#endif
		unregister_switch(&(qca_phy_priv_group[0]->sw_dev));
		kfree(qca_phy_priv_group[0]);
	}
	if (qca_phy_priv_group[1]  != NULL) {
		qca_phy_mib_work_stop(qca_phy_priv_group[1]);
#ifdef AUTO_SWITCH_RECOVERY
		qm_err_check_work_stop(qca_phy_priv_group[1]);
#endif
		unregister_switch(&(qca_phy_priv_group[1]->sw_dev));
		kfree(qca_phy_priv_group[1]);
	}

}

#else
static a_uint32_t
qca_ar8327_get_pad_cfg(struct ar8327_pad_cfg *pad_cfg)
{
	a_uint32_t value = 0;

	if (pad_cfg == 0) {
		return 0;
    }

    if(pad_cfg->mode == AR8327_PAD_MAC2MAC_MII) {
		value = AR8327_PAD_CTRL_MAC_MII_EN;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_MAC_MII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_MAC_MII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC2MAC_GMII) {
		value = AR8327_PAD_CTRL_MAC_GMII_EN;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_MAC_GMII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_MAC_GMII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC_SGMII) {
		value = AR8327_PAD_CTRL_SGMII_EN;

		/* WAR for AP136 board. */
		value |= pad_cfg->txclk_delay_sel <<
		        AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL_S;
		value |= pad_cfg->rxclk_delay_sel <<
                AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL_S;
		if (pad_cfg->rxclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
		if (pad_cfg->txclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_EN;

    } else if (pad_cfg->mode == AR8327_PAD_MAC2PHY_MII) {
		value = AR8327_PAD_CTRL_PHY_MII_EN;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_PHY_MII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_PHY_MII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC2PHY_GMII) {
		value = AR8327_PAD_CTRL_PHY_GMII_EN;
		if (pad_cfg->pipe_rxclk_sel)
			value |= AR8327_PAD_CTRL_PHY_GMII_PIPE_RXCLK_SEL;
		if (pad_cfg->rxclk_sel)
			value |= AR8327_PAD_CTRL_PHY_GMII_RXCLK_SEL;
		if (pad_cfg->txclk_sel)
			value |= AR8327_PAD_CTRL_PHY_GMII_TXCLK_SEL;

    } else if (pad_cfg->mode == AR8327_PAD_MAC_RGMII) {
		value = AR8327_PAD_CTRL_RGMII_EN;
		value |= pad_cfg->txclk_delay_sel <<
                 AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL_S;
		value |= pad_cfg->rxclk_delay_sel <<
                 AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL_S;
		if (pad_cfg->rxclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
		if (pad_cfg->txclk_delay_en)
			value |= AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_EN;

    } else if (pad_cfg->mode == AR8327_PAD_PHY_GMII) {
		value = AR8327_PAD_CTRL_PHYX_GMII_EN;

    } else if (pad_cfg->mode == AR8327_PAD_PHY_RGMII) {
		value = AR8327_PAD_CTRL_PHYX_RGMII_EN;

    } else if (pad_cfg->mode == AR8327_PAD_PHY_MII) {
		value = AR8327_PAD_CTRL_PHYX_MII_EN;

	} else {
        value = 0;
    }

	return value;
}

#ifndef BOARD_AR71XX
static a_uint32_t
qca_ar8327_get_pwr_sel(struct qca_phy_priv *priv,
                                struct ar8327_platform_data *plat_data)
{
	struct ar8327_pad_cfg *cfg = NULL;
	a_uint32_t value;

	if (!plat_data) {
		return 0;
	}

	value = priv->mii_read(AR8327_REG_PAD_MAC_PWR_SEL);

	cfg = plat_data->pad0_cfg;

	if (cfg && (cfg->mode == AR8327_PAD_MAC_RGMII) &&
                cfg->rgmii_1_8v) {
		value |= AR8327_PAD_MAC_PWR_RGMII0_1_8V;
	}

	cfg = plat_data->pad5_cfg;
	if (cfg && (cfg->mode == AR8327_PAD_MAC_RGMII) &&
                cfg->rgmii_1_8v) {
		value |= AR8327_PAD_MAC_PWR_RGMII1_1_8V;
	}

	cfg = plat_data->pad6_cfg;
	if (cfg && (cfg->mode == AR8327_PAD_MAC_RGMII) &&
               cfg->rgmii_1_8v) {
		value |= AR8327_PAD_MAC_PWR_RGMII1_1_8V;
	}

	return value;
}
#endif

static a_uint32_t
qca_ar8327_set_led_cfg(struct qca_phy_priv *priv,
                              struct ar8327_platform_data *plat_data,
                              a_uint32_t pos)
{
	struct ar8327_led_cfg *led_cfg;
    a_uint32_t new_pos = pos;

	led_cfg = plat_data->led_cfg;
	if (led_cfg) {
		if (led_cfg->open_drain)
			new_pos |= AR8327_POS_LED_OPEN_EN;
		else
			new_pos &= ~AR8327_POS_LED_OPEN_EN;

		priv->mii_write(AR8327_REG_LED_CTRL_0, led_cfg->led_ctrl0);
		priv->mii_write(AR8327_REG_LED_CTRL_1, led_cfg->led_ctrl1);
		priv->mii_write(AR8327_REG_LED_CTRL_2, led_cfg->led_ctrl2);
		priv->mii_write(AR8327_REG_LED_CTRL_3, led_cfg->led_ctrl3);

		if (new_pos != pos) {
			new_pos |= AR8327_POS_POWER_ON_SEL;
		}
	}
    return new_pos;
}

void
qca_ar8327_port_init(struct qca_phy_priv *priv, a_uint32_t port)
{
	struct ar8327_platform_data *plat_data;
	struct ar8327_port_cfg *port_cfg;
	a_uint32_t value;

	plat_data = priv->phy->dev.platform_data;
	if (plat_data == NULL) {
		return;
    }

	if (((port == 0) && plat_data->pad0_cfg) ||
	    ((port == 5) && plat_data->pad5_cfg) ||
	    ((port == 6) && plat_data->pad6_cfg)) {
        switch (port) {
        case 0:
            port_cfg = &plat_data->cpuport_cfg;
            break;
        case 5:
            port_cfg = &plat_data->port5_cfg;
            break;
        case 6:
            port_cfg = &plat_data->port6_cfg;
            break;
        }
	} else {
        return;
	}

	if (port_cfg->force_link == 0) {
		if(port == 6) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
			printk("phy[%d], port[6]: link[%d], duplex[%d]\n",
				priv->phy->addr,
				plat_data->port6_cfg.force_link,
				plat_data->port6_cfg.duplex);
			printk("phy[%d], port[0]: link[%d], duplex[%d]\n",
                        	priv->phy->addr,
                        	plat_data->cpuport_cfg.force_link,
                        	plat_data->cpuport_cfg.duplex);
#else		
			printk("phy[%d], port[6]: link[%d], duplex[%d]\n",
				priv->phy->mdio.addr,
				plat_data->port6_cfg.force_link,
				plat_data->port6_cfg.duplex);
			printk("phy[%d], port[0]: link[%d], duplex[%d]\n",
                        	priv->phy->mdio.addr,
                        	plat_data->cpuport_cfg.force_link,
                        	plat_data->cpuport_cfg.duplex);
#endif

		}
		if(port_cfg->duplex == 0 && port_cfg->speed == 0) {
			priv->mii_write(AR8327_REG_PORT_STATUS(port),
			            AR8327_PORT_STATUS_LINK_AUTO);
			return;
		}
	}
	/*disable mac at first*/
	fal_port_rxmac_status_set(0, port, A_FALSE);
	fal_port_txmac_status_set(0, port, A_FALSE);
	value = port_cfg->duplex ? FAL_FULL_DUPLEX : FAL_HALF_DUPLEX;
	fal_port_duplex_set(0, port, value);
	value = port_cfg->txpause ? A_TRUE : A_FALSE;
	fal_port_txfc_status_set(0, port, value);
	value = port_cfg->rxpause ? A_TRUE : A_FALSE;
	fal_port_rxfc_status_set(0, port, value);
	if(port_cfg->speed == AR8327_PORT_SPEED_10) {
		value = FAL_SPEED_10;
	} else if(port_cfg->speed == AR8327_PORT_SPEED_100) {
		value = FAL_SPEED_100;
	} else if(port_cfg->speed == AR8327_PORT_SPEED_1000) {
		value = FAL_SPEED_1000;
	} else {
		value = FAL_SPEED_1000;
	}
	fal_port_speed_set(0, port, value);
	/*enable mac at last*/
	udelay(800);
	fal_port_rxmac_status_set(0, port, A_TRUE);
	fal_port_txmac_status_set(0, port, A_TRUE);
}

static int
qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	struct ar8327_platform_data *plat_data;
	a_uint32_t pos, new_pos;
	a_uint32_t value, i;

	plat_data = priv->phy->dev.platform_data;
	if (plat_data == NULL) {
		return -EINVAL;
	}

	/*Before switch software reset, disable PHY and clear MAC PAD*/
	qca_ar8327_phy_disable();
	qca_mac_disable();
	msleep(1000);

	/*First software reset S17 chip*/
	value = priv->mii_read(AR8327_REG_CTRL);
	value |= 0x80000000;
	priv->mii_write(AR8327_REG_CTRL, value);
	/*Need wait reset done*/
	do {
		udelay(10);
		value = priv->mii_read(AR8327_REG_CTRL);
	} while(value & AR8327_CTRL_RESET);
	do {
		udelay(10);
		value = priv->mii_read(0x20);
	} while ((value & SSDK_GLOBAL_INITIALIZED_STATUS) != SSDK_GLOBAL_INITIALIZED_STATUS);

	/*After switch software reset, need disable all ports' MAC with 1000M FULL*/
	qca_switch_mac_reset(priv);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(0));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(0), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(1));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(1), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(2));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(2), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(3));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(3), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(4));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(4), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(5));
	value &= ~0x5e;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(5), value);

	value = priv->mii_read(AR8327_REG_PORT_LOOKUP(6));
	value &= ~0x21;
	priv->mii_write(AR8327_REG_PORT_LOOKUP(6), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(0), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(1), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(2), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(3), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(4), value);

	value = 0x20001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(5), value);

	value = 0x10001;
	priv->mii_write(AR8327_REG_PORT_VLAN0(6), value);

	qca_switch_init(0);

	value = qca_ar8327_get_pad_cfg(plat_data->pad0_cfg);
	priv->mii_write(AR8327_REG_PAD0_CTRL, value);

	value = qca_ar8327_get_pad_cfg(plat_data->pad5_cfg);
	priv->mii_write(AR8327_REG_PAD5_CTRL, value);

	value = qca_ar8327_get_pad_cfg(plat_data->pad6_cfg);
	if(plat_data->pad5_cfg &&
		(plat_data->pad5_cfg->mode == AR8327_PAD_PHY_RGMII))
		value |= AR8327_PAD_CTRL_PHYX_RGMII_EN;
	priv->mii_write(AR8327_REG_PAD6_CTRL, value);

#ifndef BOARD_AR71XX
	value = qca_ar8327_get_pwr_sel(priv, plat_data);
	priv->mii_write(AR8327_REG_PAD_MAC_PWR_SEL, value);
#endif

	pos = priv->mii_read(AR8327_REG_POS);

    new_pos = qca_ar8327_set_led_cfg(priv, plat_data, pos);

#ifndef BOARD_AR71XX
	/*configure the SGMII*/
	if (plat_data->sgmii_cfg) {
		value = priv->mii_read(AR8327_REG_PAD_SGMII_CTRL);
		value &= ~(AR8327_PAD_SGMII_CTRL_MODE_CTRL);
		value |= ((plat_data->sgmii_cfg->sgmii_mode) <<
                  AR8327_PAD_SGMII_CTRL_MODE_CTRL_S);

		if (priv->version == QCA_VER_AR8337) {
			value |= (AR8327_PAD_SGMII_CTRL_EN_PLL |
			     AR8327_PAD_SGMII_CTRL_EN_RX |
			     AR8327_PAD_SGMII_CTRL_EN_TX);
		} else {
			value &= ~(AR8327_PAD_SGMII_CTRL_EN_PLL |
			       AR8327_PAD_SGMII_CTRL_EN_RX |
			       AR8327_PAD_SGMII_CTRL_EN_TX);
		}
		value |= AR8327_PAD_SGMII_CTRL_EN_SD;

		priv->mii_write(AR8327_REG_PAD_SGMII_CTRL, value);

		if (plat_data->sgmii_cfg->serdes_aen) {
			new_pos &= ~AR8327_POS_SERDES_AEN;
		} else {
			new_pos |= AR8327_POS_SERDES_AEN;
		}
	}
#endif

	priv->mii_write(AR8327_REG_POS, new_pos);

	if(priv->version == QCA_VER_AR8337) {
		value = priv->mii_read(AR8327_REG_PAD5_CTRL);
		value |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
		priv->mii_write(AR8327_REG_PAD5_CTRL, value);
	}

	msleep(1000);

	for (i = 0; i < AR8327_NUM_PORTS; i++) {
		qca_ar8327_port_init(priv, i);
	}

	qca_ar8327_phy_enable(priv);

	return 0;
}
#endif

static int
qca_ar8327_sw_get_reg_val(struct switch_dev *dev,
                                    int reg, int *val)
{
	return 0;
}

static int
qca_ar8327_sw_set_reg_val(struct switch_dev *dev,
                                    int reg, int val)
{
	return 0;
}

static struct switch_attr qca_ar8327_globals[] = {
	{
		.name = "enable_vlan",
		.description = "Enable 8021q VLAN",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_vlan,
		.get = qca_ar8327_sw_get_vlan,
		.max = 1
	},{
		.name = "max_frame_size",
		.description = "Set Max frame Size Of Mac",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_max_frame_size,
		.get = qca_ar8327_sw_get_max_frame_size,
		.max = 9018
	},
	{
		.name = "reset_mibs",
		.description = "Reset All MIB Counters",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_set_reset_mibs,
	},
	{
		.name = "flush_arl",
		.description = "Flush All ARL table",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_atu_flush,
	},
	{
		.name = "dump_arl",
		.description = "Dump All ARL table",
		.type = SWITCH_TYPE_STRING,
		.get = qca_ar8327_sw_atu_dump,
	},
	{
		.name = "switch_ext",
		.description = "Switch extended configuration",
		.type = SWITCH_TYPE_EXT,
		.set = qca_ar8327_sw_switch_ext,
	},
};

static struct switch_attr qca_ar8327_port[] = {
	{
		.name = "reset_mib",
		.description = "Reset Mib Counters",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_set_port_reset_mib,
	},
	{
		.name = "mib",
		.description = "Get Mib Counters",
		.type = SWITCH_TYPE_STRING,
		.set = NULL,
		.get = qca_ar8327_sw_get_port_mib,
	},
};

static struct switch_attr qca_ar8327_vlan[] = {
	{
		.name = "vid",
		.description = "Configure Vlan Id",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_vid,
		.get = qca_ar8327_sw_get_vid,
		.max = 4094,
	},
};

const struct switch_dev_ops qca_ar8327_sw_ops = {
	.attr_global = {
		.attr = qca_ar8327_globals,
		.n_attr = ARRAY_SIZE(qca_ar8327_globals),
	},
	.attr_port = {
		.attr = qca_ar8327_port,
		.n_attr = ARRAY_SIZE(qca_ar8327_port),
	},
	.attr_vlan = {
		.attr = qca_ar8327_vlan,
		.n_attr = ARRAY_SIZE(qca_ar8327_vlan),
	},
	.get_port_pvid = qca_ar8327_sw_get_pvid,
	.set_port_pvid = qca_ar8327_sw_set_pvid,
	.get_vlan_ports = qca_ar8327_sw_get_ports,
	.set_vlan_ports = qca_ar8327_sw_set_ports,
	.apply_config = qca_ar8327_sw_hw_apply,
	.reset_switch = qca_ar8327_sw_reset_switch,
	.get_port_link = qca_ar8327_sw_get_port_link,
#ifndef BOARD_AR71XX
	.get_reg_val = qca_ar8327_sw_get_reg_val,
	.set_reg_val = qca_ar8327_sw_set_reg_val,
#endif
};

#define SSDK_MIB_CHANGE_WQ

static int
qca_phy_mib_task(struct qca_phy_priv *priv)
{
	qca_ar8327_sw_mib_task(&priv->sw_dev);
	return 0;
}

static void
qca_phy_mib_work_task(struct work_struct *work)
{
	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
                                            mib_dwork.work);

	mutex_lock(&priv->mib_lock);

    qca_phy_mib_task(priv);

	mutex_unlock(&priv->mib_lock);
#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->mib_dwork,
			      msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->mib_dwork,
					msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#endif
}

int
qca_phy_mib_work_start(struct qca_phy_priv *priv)
{
	mutex_init(&priv->mib_lock);
	priv->mib_counters = kzalloc(priv->sw_dev.ports * QCA_MIB_ITEM_NUMBER *
	      sizeof(*priv->mib_counters), GFP_KERNEL);
	if (!priv->mib_counters)
		return -ENOMEM;

	INIT_DELAYED_WORK(&priv->mib_dwork, qca_phy_mib_work_task);
#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->mib_dwork,
			               msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->mib_dwork,
					msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#endif

	return 0;
}

void
qca_phy_mib_work_stop(struct qca_phy_priv *priv)
{
	if(!priv)
		return;
	if(priv->mib_counters)
		kfree(priv->mib_counters);
	cancel_delayed_work_sync(&priv->mib_dwork);
}

#define SSDK_QM_CHANGE_WQ
static void
qm_err_check_work_task(struct work_struct *work)
{
	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
                                            qm_dwork.work);

	mutex_lock(&priv->qm_lock);

	qca_ar8327_sw_mac_polling_task(&priv->sw_dev);

	mutex_unlock(&priv->qm_lock);

	#ifndef SSDK_QM_CHANGE_WQ
	schedule_delayed_work(&priv->qm_dwork,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
	#else
	queue_delayed_work_on(0, system_long_wq, &priv->qm_dwork,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
	#endif
}

int
qm_err_check_work_start(struct qca_phy_priv *priv)
{
	/*Only valid for S17c chip*/
	if (priv->version != QCA_VER_AR8337)
		return -1;

	mutex_init(&priv->qm_lock);

	INIT_DELAYED_WORK(&priv->qm_dwork, qm_err_check_work_task);

	#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->qm_dwork,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
	#else
	queue_delayed_work_on(0, system_long_wq, &priv->qm_dwork,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
	#endif

	return 0;
}

void
qm_err_check_work_stop(struct qca_phy_priv *priv)
{
	/*Only valid for S17c chip*/
	if (priv->version != QCA_VER_AR8337) return;

	cancel_delayed_work_sync(&priv->qm_dwork);
}
#ifdef DESS
static void
dess_rgmii_mac_work_task(struct work_struct *work)
{
	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
                                            rgmii_dwork.work);

	mutex_lock(&priv->rgmii_lock);

	dess_rgmii_sw_mac_polling_task(&priv->sw_dev);

	mutex_unlock(&priv->rgmii_lock);

	schedule_delayed_work(&priv->rgmii_dwork, msecs_to_jiffies(QCA_RGMII_WORK_DELAY));
}

int
dess_rgmii_mac_work_start(struct qca_phy_priv *priv)
{
	mutex_init(&priv->rgmii_lock);

	INIT_DELAYED_WORK(&priv->rgmii_dwork, dess_rgmii_mac_work_task);

	schedule_delayed_work(&priv->rgmii_dwork, msecs_to_jiffies(QCA_RGMII_WORK_DELAY));

	return 0;
}

void
dess_rgmii_mac_work_stop(struct qca_phy_priv *priv)
{
	cancel_delayed_work_sync(&priv->rgmii_dwork);
}
#endif
int
qca_phy_id_chip(struct qca_phy_priv *priv)
{
	a_uint32_t value, version;

	value = qca_ar8216_mii_read(AR8327_REG_CTRL);
	version = value & (AR8327_CTRL_REVISION |
                AR8327_CTRL_VERSION);
	priv->version = (version & AR8327_CTRL_VERSION) >>
                           AR8327_CTRL_VERSION_S;
	priv->revision = (version & AR8327_CTRL_REVISION);

    if((priv->version == QCA_VER_AR8327) ||
       (priv->version == QCA_VER_AR8337) ||
       (priv->version == QCA_VER_AR8227)) {
		return 0;

    } else {
		printk("unsupported QCA device\n");
		return -ENODEV;
	}
}


#define LED_OPTION_BLINK	33
#define LED_OPTION_ON		34
#define LED_OPTION_OFF		35
extern void (*set_lan_led)(int option);

void set_lan_led_func(int option)
{
	if (option == LED_OPTION_BLINK) {
		priv_dni->mii_write(0x0050, 0xcc35cc35);
		priv_dni->mii_write(0x0054, 0xca35ca35);
		priv_dni->mii_write(0x0058, 0xc935c935);
		qca_ar8216_mii_slave_write(0x0050, 0xcc35cc35);
		qca_ar8216_mii_slave_write(0x0054, 0xca35ca35);
		qca_ar8216_mii_slave_write(0x0058, 0xc935c935);
	}
	else if (option == LED_OPTION_ON) {
		priv_dni->mii_write(0x0050, 0xcc01cc01);
		priv_dni->mii_write(0x0054, 0xca01ca01);
		priv_dni->mii_write(0x0058, 0xc901c901);
		qca_ar8216_mii_slave_write(0x0050, 0xcc01cc01);
		qca_ar8216_mii_slave_write(0x0054, 0xca01ca01);
		qca_ar8216_mii_slave_write(0x0058, 0xc901c901);
	}
	else if (option == LED_OPTION_OFF) {
		priv_dni->mii_write(0x0050, 0x0);
		priv_dni->mii_write(0x0054, 0x0);
		priv_dni->mii_write(0x0058, 0x0);
		qca_ar8216_mii_slave_write(0x0050, 0x0);
		qca_ar8216_mii_slave_write(0x0054, 0x0);
		qca_ar8216_mii_slave_write(0x0058, 0x0);
	}
}

static int lan_led_ctrl_read( char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	return 0;
}

static int lan_led_ctrl_write( struct file *file, const char *buffer, unsigned long count, void *unused_data)
{
	char line[5]; 
	int option;
	if (copy_from_user(line, buffer, (count < sizeof(line)) ? count : sizeof(line)))
		return -EFAULT;
	option = line[0] - '0';
	printk("LAN LED state %d\n", option);
	if (option == 0) {
		/* led off */
		priv_dni->mii_write(0x0050, 0x0);
		priv_dni->mii_write(0x0054, 0x0);
		priv_dni->mii_write(0x0058, 0x0);
		priv_dni->mii_write(0x005c, 0x0);
		qca_ar8216_mii_slave_write(0x0050, 0x0);
		qca_ar8216_mii_slave_write(0x0054, 0x0);
		qca_ar8216_mii_slave_write(0x0058, 0x0);
		qca_ar8216_mii_slave_write(0x005c, 0x0);
	}else if (option == 1) {
		/* white: 1000M led on */
		priv_dni->mii_write(0x0050, 0x8000);
		priv_dni->mii_write(0x0054, 0x0);
		priv_dni->mii_write(0x0058, 0x0);
		priv_dni->mii_write(0x005c, 0x208200);
		qca_ar8216_mii_slave_write(0x0050, 0x8000);
		qca_ar8216_mii_slave_write(0x0054, 0x0);
		qca_ar8216_mii_slave_write(0x0058, 0x0);
		qca_ar8216_mii_slave_write(0x005c, 0x208200);
	}
	else if (option == 2) {
		/* amber: 100M len on */
		priv_dni->mii_write(0x0050, 0x0);
		priv_dni->mii_write(0x0054, 0x8000);
		priv_dni->mii_write(0x0058, 0x0);
		priv_dni->mii_write(0x005c, 0x820800);
		qca_ar8216_mii_slave_write(0x0050, 0x0);
		qca_ar8216_mii_slave_write(0x0054, 0x8000);
		qca_ar8216_mii_slave_write(0x0058, 0x0);
		qca_ar8216_mii_slave_write(0x005c, 0x820800);
	} else if (option == 3) {
		/* normal state */
		priv_dni->mii_write(0x0050, 0xcc35cc35);
		priv_dni->mii_write(0x0054, 0xca35ca35);
		priv_dni->mii_write(0x0058, 0xc935c935);
		priv_dni->mii_write(0x005c, 0x3ffff00);

		qca_ar8216_mii_slave_write(0x0050, 0xcc35cc35);
		qca_ar8216_mii_slave_write(0x0054, 0xca35ca35);
		qca_ar8216_mii_slave_write(0x0058, 0xc935c935);
		qca_ar8216_mii_slave_write(0x005c, 0x3ffff00);
	}
	return count;
}

static struct file_operations lan_ledctrl_operation = {
	.read = lan_led_ctrl_read,
	.write = lan_led_ctrl_write,
};

#define LAN_LED_CTRL  "lan_led_ctrl"
static void lan_led_ctrl_init(void)
{
	struct proc_dir_entry *lan_led_ctrl;
	printk("[lxx] create proc lan led \n");
	if(lan_led_ctrl = proc_create(LAN_LED_CTRL, 0666, NULL, &lan_ledctrl_operation))
	{
		lan_led_ctrl->nlink = 1 ;
	}
}

static void lan_led_ctrl_cleanup(void)
{
	remove_proc_entry(LAN_LED_CTRL, NULL);
}


#define SWITCH_PHY  "switch_phy"
#ifdef DNI_eth_phy_swap
static inline int port2phy(int port) { return (port==4) ? 0 : (port+1); }
static inline int phy2port(int phyUnit) { return (phyUnit==0) ? 4 : (phyUnit-1) ; }
#else
static inline int port2phy(int port) { return port; }
static inline int phy2port(int phyUnit) { return phyUnit; }
#endif

struct switch_phy_timer {
	struct timer_list timer;
	int loops;
	int port_mask;
	struct work_struct work;
};

#define SWITCH_INTERVAL	(HZ)

static void dni_switch_phy(int phyUnit)
{
	int dni_swid;
	if (phyUnit < 3)
	{
		dni_swid = 1;
		phyUnit = 2 - phyUnit;
	}
	else if (phyUnit < 7)
	{
		dni_swid= 2;
		phyUnit = 6 - phyUnit;
	}
	else
	{
		printk("Wrong Phy Unit\n");
		return ;
	}

	if(dni_swid !=1 && dni_swid != 2)
		return;

	if (dni_swid == 1)
	{
		mutex_lock(&miibus->mdio_lock);
		miibus->write(miibus, phyUnit, 0x04, 0x0de0);
		miibus->write(miibus, phyUnit, 0x09, 0x0200);
		miibus->write(miibus, phyUnit, 0x00, 0x1200);
		mutex_unlock(&miibus->mdio_lock);
	}
	else if (dni_swid == 2)
	{
		mutex_lock(&switch_mdio_slave_lock);
		miibus_slave->write(miibus, phyUnit, 0x04, 0x0de0);
		miibus_slave->write(miibus, phyUnit, 0x09, 0x0200);
		miibus_slave->write(miibus, phyUnit, 0x00, 0x1200);
		mutex_unlock(&switch_mdio_slave_lock);
	}
}

static void switch_phy_work(struct work_struct *work)
{
	int port;
	struct switch_phy_timer *switch_timer =
		container_of(work, struct switch_phy_timer, work);

	for(port = 0; port < 7; port++){
		if ( (switch_timer->port_mask & (1 << port)) != 0 )
			dni_switch_phy(port);
	}

	if(--(switch_timer->loops)){
		switch_timer->timer.expires += SWITCH_INTERVAL;
		add_timer(&switch_timer->timer);
	} else {
		//cancel_work_sync(&switch_timer->work);
		kfree(switch_timer);
	}
}

static void do_switch_phy_timer(unsigned long arg)
{
	struct switch_phy_timer *switch_timer = (struct switch_phy_timer *)arg;

	schedule_work(&switch_timer->work);
}

static int switch_phy_read( char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	return 0;
}

#define MAX_DOWN_TIME 30
static int switch_phy_write( struct file *file, const char *buffer, unsigned long count, void *unused_data)
{
	struct switch_phy_timer *switch_timer;
	int port;
	char line[8] = {'9', '0', '1', '1', '1', '1', '1', '1'};
	int loops, port_mask = 0;

	if (copy_from_user(line, buffer, (count < sizeof(line)) ? count : sizeof(line)))
		return -EFAULT;
	switch_timer = kmalloc(sizeof(struct switch_phy_timer), GFP_KERNEL);
	if(!switch_timer)
		return -ENOMEM;

	/* the 0th byte : the time(second) link down, should be 1-9 */
	loops = (int)(line[0] - '0');
	if (loops > MAX_DOWN_TIME)
		loops = MAX_DOWN_TIME;
	else if (loops < 1)
		loops = 1;
	loops = loops * HZ / SWITCH_INTERVAL;
	/* 1st byte : trigger port 4 or not, ... , 5th byte : trigger port 0 or not. */
	for (port = 0; port < 7; port++) {
		if (line[port + 1] != '0')
			port_mask = port_mask + (1 << port);
	}
	switch_timer->loops = loops;
	switch_timer->port_mask = port_mask;
	INIT_WORK(&switch_timer->work, switch_phy_work);

	init_timer(&switch_timer->timer);

	/* the ports 0, 1, 2, 3 are lan ports, 4 is wan port. */
	for(port = 0; port < 7; port++){
		if ( (switch_timer->port_mask & (1 << port)) != 0 )
			dni_switch_phy(port);
	}

	switch_timer->timer.data = (unsigned long)switch_timer;
	switch_timer->timer.function = do_switch_phy_timer;
	switch_timer->timer.expires = jiffies + SWITCH_INTERVAL;

	add_timer(&switch_timer->timer);

	if(signal_pending(current))
		return -ERESTARTSYS;

	return count;
}


static struct file_operations dni_switch_phy_operation = {
	.read = switch_phy_read,
	.write = switch_phy_write,
};

static void dni_switch_phy_init(void)
{
	struct proc_dir_entry *switch_phy;

	if(switch_phy = proc_create(SWITCH_PHY, 0666, NULL, &dni_switch_phy_operation))
	{
		switch_phy->nlink = 1; 
	}
}

static void dni_switch_phy_cleanup(void)
{
	remove_proc_entry(SWITCH_PHY, NULL);
}

static int blink_wan_led_read( char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	return 0;
}

extern void set_blink_wan_led(int val);
static int blink_wan_led_write( struct file *file, const char *buffer, unsigned long count, void *unused_data)
{
	char line[5] = {0};
	int option;
	if (copy_from_user(line, buffer, (count < sizeof(line)) ? count : sizeof(line)))
		return -EFAULT;
	option = line[0] - '0';
//	set_blink_wan_led(option);
	printk("[LXX]Blink wan led set [%d]\n",option);
	return count;
}

static struct file_operations blink_wan_led_operation = {
	.read = blink_wan_led_read,
	.write = blink_wan_led_write,
};

#define BLINK_WAN_LED "blink_wan_led"
static void blink_wan_led_init(void)
{
	struct proc_dir_entry *blink_wan_led;

	if (blink_wan_led = proc_create(BLINK_WAN_LED, 0666, NULL, &blink_wan_led_operation))
	{
		blink_wan_led->nlink = 1;
	}
}

static void blink_wan_led_cleanup(void)
{
	remove_proc_entry(BLINK_WAN_LED, NULL);
}

static int mirror_wan_read( char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	return 0;
}

static int mirror_wan_write( struct file *file, const char *buffer, unsigned long count, void *unused_data)
{
	char line[5]; 
	int state;
	int old, new;

	if (copy_from_user(line, buffer, (count < sizeof(line)) ? count : sizeof(line)))
		return -EFAULT;
	state = line[0] - '0';
	printk("Mirror WAN state %d\n", state);
	if (state != 0) {
		/* mirror port 0 (WAN port) to port 4 (port near WAN port) */
		/* R9000  mirror port 2 (wan port of switch A) to port 1 (port near wan port of switch A) */
		old = qca_ar8216_mii_read(0x0620);
		qca_ar8216_mii_write(0x0620, (old & (~0xf0)) | 0x20);//port 2
		new = qca_ar8216_mii_read(0x0620);
		printk("0x0620: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x0684);
		qca_ar8216_mii_write(0x0684, old | 0x2000000);
		new = qca_ar8216_mii_read(0x0684);
		printk("0x0660: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x098c); //port 3
		qca_ar8216_mii_write(0x098c, old | 0x10000);
		new = qca_ar8216_mii_read(0x098c);
		printk("0x0974: %08x => %08x\n", old, new);

#if 0
		/* don't allow CPU port (port 6) and other LAN port (port 1, 2, 3) to send packet to port 4 */
		old = qca_ar8216_mii_read(0x066c);
		qca_ar8216_mii_write(0x066c, old & (~0x10));
		new = qca_ar8216_mii_read(0x066c);
		printk("0x066c: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x0678);
		qca_ar8216_mii_write(0x0678, old & (~0x10));
		new = qca_ar8216_mii_read(0x0678);
		printk("0x0678: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x0684);
		qca_ar8216_mii_write(0x0684, old & (~0x10));
		new = qca_ar8216_mii_read(0x0684);
		printk("0x0684: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x06a8);
		qca_ar8216_mii_write(0x06a8, old & (~0x10));
		new = qca_ar8216_mii_read(0x06a8);
		printk("0x06a8: %08x => %08x\n", old, new);
#endif
	} else {
		/* disable mirror */
		old = qca_ar8216_mii_read(0x0620);
		qca_ar8216_mii_write(0x0620, old | 0xf0);
		new = qca_ar8216_mii_read(0x0620);
		printk("0x0620: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x0684);
		qca_ar8216_mii_write(0x0684, old & (~0x2000000));
		new = qca_ar8216_mii_read(0x0684);
		printk("0x0660: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x098c);
		qca_ar8216_mii_write(0x098c, old & (~0x10000));
		new = qca_ar8216_mii_read(0x098c);
		printk("0x0974: %08x => %08x\n", old, new);
#if 0
		/* allow CPU port (port 6) and other LAN port (port 1, 2, 3) to send packet to port 4 */
		old = qca_ar8216_mii_read(0x066c);
		qca_ar8216_mii_write(0x066c, old | 0x10);
		new = qca_ar8216_mii_read(0x066c);
		printk("0x066c: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x0678);
		qca_ar8216_mii_write(0x0678, old | 0x10);
		new = qca_ar8216_mii_read(0x0678);
		printk("0x0678: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x0684);
		qca_ar8216_mii_write(0x0684, old | 0x10);
		new = qca_ar8216_mii_read(0x0684);
		printk("0x0684: %08x => %08x\n", old, new);

		old = qca_ar8216_mii_read(0x06a8);
		qca_ar8216_mii_write(0x06a8, old | 0x10);
		new = qca_ar8216_mii_read(0x06a8);
		printk("0x06a8: %08x => %08x\n", old, new);
#endif
	}

	return count;
}


static struct file_operations mirror_wan_operation = {
	.read = mirror_wan_read,
	.write = mirror_wan_write,
};

#define MIRROR_WAN  "mirror_wan"
static void mirror_wan_init(void)
{
	struct proc_dir_entry *mirror_wan;

	if(mirror_wan = proc_create(MIRROR_WAN, 0666, NULL,&mirror_wan_operation))
	{
		mirror_wan->nlink = 1;
	}

}

static void mirror_wan_cleanup(void)
{
	remove_proc_entry(MIRROR_WAN, NULL);
}

static int
qca_phy_config_init(struct phy_device *pdev)
{
	struct qca_phy_priv *priv = pdev->priv;
	struct switch_dev *sw_dev;
	int ret;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
	if (pdev->addr != 0) {
#else		
	if (pdev->mdio.addr != 0) {
#endif	
        pdev->supported |= SUPPORTED_1000baseT_Full;
        pdev->advertising |= ADVERTISED_1000baseT_Full;
		#if 0 // defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
		ssdk_phy_rgmii_set(priv);
		#endif
		return 0;
	}

	if (priv == NULL)
		return -ENOMEM;

	priv->phy = pdev;
	ret = qca_phy_id_chip(priv);
	if (ret != 0) {
		return ret;
	}

	priv->mii_read = qca_ar8216_mii_read;
	priv->mii_write = qca_ar8216_mii_write;
	priv->phy_write = qca_ar8327_phy_write;
	priv->phy_dbg_write = qca_ar8327_phy_dbg_write;
	priv->phy_dbg_read = qca_ar8327_phy_dbg_read;
	priv->phy_mmd_write = qca_ar8327_mmd_write;

	pdev->priv = priv;
	pdev->supported |= SUPPORTED_1000baseT_Full;
	pdev->advertising |= ADVERTISED_1000baseT_Full;

	sw_dev = &priv->sw_dev;
	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->name = "QCA AR8327 AR8337";
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = AR8327_NUM_PORTS;

	ret = register_switch(&priv->sw_dev, pdev->attached_dev);
	if (ret != 0) {
	        return ret;
	}

	/*register the second switch*/
	sw_dev = &(qca_phy_priv_group[1]->sw_dev);
	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->name = "QCA AR8327 AR8337";
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = AR8327_NUM_PORTS;
	ret = register_switch(&priv->sw_dev, NULL);
	if (ret != 0) {
	        return ret;
	}

	ret = qca_ar8327_hw_init(priv);
	if (ret != 0) {
		return ret;
	}

	qca_phy_mib_work_start(priv);

	#ifdef AUTO_SWITCH_RECOVERY
	qm_err_check_work_start(priv);
	#endif

	return ret;
}

struct qca_phy_priv *qca_phy_priv_global;

static int ssdk_switch_register(void)
{
	struct switch_dev *sw_dev;
	struct qca_phy_priv *priv;
	int ret = 0;
	a_uint32_t chip_id = 0;

	priv = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
	if (priv == NULL) {
		return -ENOMEM;
	}
	qca_phy_priv_global = priv;

	priv->mii_read = qca_ar8216_mii_read;
	priv->mii_write = qca_ar8216_mii_write;
	priv->phy_write = qca_ar8327_phy_write;
	priv->phy_dbg_write = qca_ar8327_phy_dbg_write;
	priv->phy_dbg_read = qca_ar8327_phy_dbg_read;
	priv->phy_mmd_write = qca_ar8327_mmd_write;

	if (fal_reg_get(0, 0, (a_uint8_t *)&chip_id, 4) == SW_OK) {
		priv->version = ((chip_id >> 8) & 0xff);
		priv->revision = (chip_id & 0xff);
		printk("Dakota Chip version 0x%02x%02x\n", priv->version, priv->revision);
	}

	mutex_init(&priv->reg_mutex);

	sw_dev = &priv->sw_dev;

	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->name = "QCA AR8327 AR8337";
	sw_dev->alias = "QCA AR8327 AR8337";
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = AR8327_NUM_PORTS;

	ret = register_switch(sw_dev, NULL);
	if (ret != 0) {
			printk("register_switch failed for %s\n", sw_dev->name);
			return ret;
	}

	ret = qca_phy_mib_work_start(priv);
	if (ret != 0) {
			printk("qca_phy_mib_work_start failed for %s\n", sw_dev->name);
			return ret;
	}
	#ifdef DESS
	if ((ssdk_dt_global.mac_mode == PORT_WRAPPER_SGMII0_RGMII5)
		||(ssdk_dt_global.mac_mode == PORT_WRAPPER_SGMII1_RGMII5)
		||(ssdk_dt_global.mac_mode == PORT_WRAPPER_SGMII0_RGMII4)
		||(ssdk_dt_global.mac_mode == PORT_WRAPPER_SGMII1_RGMII4)
		||(ssdk_dt_global.mac_mode == PORT_WRAPPER_SGMII4_RGMII4)) {
		ret = dess_rgmii_mac_work_start(priv);
		if (ret != 0) {
			printk("dess_rgmii_mac_work_start failed for %s\n", sw_dev->name);
			return ret;
		}
	}
	#endif

	return 0;

}

static int ssdk_switch_unregister(void)
{
	qca_phy_mib_work_stop(qca_phy_priv_global);
	#ifdef AUTO_SWITCH_RECOVERY
	qm_err_check_work_stop(qca_phy_priv_global);
	#endif
	unregister_switch(&qca_phy_priv_global->sw_dev);
	kfree(qca_phy_priv_global);
	return 0;
}

static int
qca_phy_read_status(struct phy_device *pdev)
{
	struct qca_phy_priv *priv = pdev->priv;
	struct switch_port_link port_link;
	int ret = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
	if (pdev->addr != 0) {
#else		
	if (pdev->mdio.addr != 0) {
#endif		
		mutex_lock(&priv->reg_mutex);
		ret = genphy_read_status(pdev);
		mutex_unlock(&priv->reg_mutex);
		return ret;
	}

	mutex_lock(&priv->reg_mutex);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
	qca_phy_read_port_link(priv, pdev->addr, &port_link);
#else		
	qca_phy_read_port_link(priv, pdev->mdio.addr, &port_link);
#endif		


	mutex_unlock(&priv->reg_mutex);

	pdev->link = !!port_link.link;
	if (pdev->link == 0)
		return 0;

	if(port_link.speed == SWITCH_PORT_SPEED_10) {
		pdev->speed = SPEED_10;
	} else if (port_link.speed == SWITCH_PORT_SPEED_100) {
		pdev->speed = SPEED_100;
	} else if (port_link.speed == SWITCH_PORT_SPEED_1000) {
		pdev->speed = SPEED_1000;
	} else {
		pdev->speed = 0;
	}

	if(port_link.duplex) {
		pdev->duplex = DUPLEX_FULL;
	} else {
		pdev->duplex = DUPLEX_HALF;
	}

	pdev->state = PHY_RUNNING;
	netif_carrier_on(pdev->attached_dev);
	pdev->adjust_link(pdev->attached_dev);

	return ret;
}

static int
qca_phy_config_aneg(struct phy_device *pdev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
	if (pdev->addr != 0) {
#else		
	if (pdev->mdio.addr != 0) {
#endif	
		return genphy_config_aneg(pdev);
	}

	return 0;
}

static int
qca_phy_probe(struct phy_device *pdev)
{
	struct qca_phy_priv *priv;
	int ret;
	static int miibus_init = 0;

	priv = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
	if (priv == NULL) {
		return -ENOMEM;
	}
	if (!miibus_init) {
		miibus_init = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
		miibus = pdev->bus;
#else		
		miibus = pdev->mdio.bus;
#endif
	}

	pdev->priv = priv;
	priv->phy = pdev;
	mutex_init(&priv->reg_mutex);

	/*init the second switch*/
	qca_phy_priv_group[0] = priv;
	qca_phy_priv_group[1] = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
	if (qca_phy_priv_group[1] == NULL) {
		return -ENOMEM;
	}
	qca_phy_priv_group[1]->phy = pdev;
	mutex_init(&(qca_phy_priv_group[1]->reg_mutex));

	ret = qca_phy_id_chip(priv);
	return ret;
}

static void
qca_phy_remove(struct phy_device *pdev)
{
	struct qca_phy_priv *priv = pdev->priv;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
	if ((pdev->addr == 0) && priv && (priv->sw_dev.name != NULL)) {
#else		
	if ((pdev->mdio.addr == 0) && priv && (priv->sw_dev.name != NULL)) {
#endif	

		qca_phy_mib_work_stop(priv);
		if (qca_phy_priv_group[1]  != NULL)
			qca_phy_mib_work_stop(qca_phy_priv_group[1]);
#ifdef AUTO_SWITCH_RECOVERY
		qm_err_check_work_stop(priv);
		if (qca_phy_priv_group[1]  != NULL)
			qm_err_check_work_stop(qca_phy_priv_group[1]);
#endif
		unregister_switch(&priv->sw_dev);
		if (qca_phy_priv_group[1]  != NULL)
			unregister_switch(&(qca_phy_priv_group[1]->sw_dev));
	}

	if (priv) {
		kfree(priv);
    	}
	if (qca_phy_priv_group[1]  != NULL) {
		kfree(qca_phy_priv_group[1]);
	}

}

static struct phy_driver qca_phy_driver = {
    .name		= "QCA AR8216 AR8236 AR8316 AR8327 AR8337",
	.phy_id		= 0x004d0000,
	.phy_id_mask= 0xffff0000,
	.probe		= qca_phy_probe,
	.remove		= qca_phy_remove,
	.config_init= &qca_phy_config_init,
	.config_aneg= &qca_phy_config_aneg,
	.read_status= &qca_phy_read_status,
	.features	= PHY_BASIC_FEATURES,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)	
	.driver = {.owner = THIS_MODULE},
#endif
};

struct ag71xx_mdio {
	struct mii_bus		*mii_bus;
	int			mii_irq[PHY_MAX_ADDR];
	void __iomem		*mdio_base;
};


static int phy_address[5] = {0,1,2,3,4};

#ifdef BOARD_IPQ806X
#define IPQ806X_MDIO_BUS_NAME			"mdio-gpio"
#endif

#ifdef BOARD_AR71XX
#define IPQ806X_MDIO_BUS_NAME			"ag71xx-mdio"
#endif
#define MDIO_BUS_0						0
#define MDIO_BUS_1						1
#define IPQ806X_MDIO_BUS_NUM			MDIO_BUS_0

static inline void
split_addr(uint32_t regaddr, uint16_t *r1, uint16_t *r2, uint16_t *page)
{
	regaddr >>= 1;
	*r1 = regaddr & 0x1e;

	regaddr >>= 5;
	*r2 = regaddr & 0x7;

	regaddr >>= 3;
	*page = regaddr & 0x3ff;
}

uint32_t
qca_ar8216_mii_read(int reg)
{
        struct mii_bus *bus = miibus;
        uint16_t r1, r2, page;
        uint16_t lo, hi;

        split_addr((uint32_t) reg, &r1, &r2, &page);
        mutex_lock(&switch_mdio_lock);
        mdiobus_write(bus, switch_chip_id, switch_chip_reg, page);
	udelay(100);
        lo = mdiobus_read(bus, 0x10 | r2, r1);
        hi = mdiobus_read(bus, 0x10 | r2, r1 + 1);
        mutex_unlock(&switch_mdio_lock);
        return (hi << 16) | lo;
}

uint32_t
qca_ar8216_mii_slave_read(int reg)
{
        struct mii_bus *bus = miibus_slave;
        uint16_t r1, r2, page;
        uint16_t lo, hi;

        split_addr((uint32_t) reg, &r1, &r2, &page);
        mutex_lock(&switch_mdio_slave_lock);
        mdiobus_write(bus, switch_chip_id, switch_chip_reg, page);
	udelay(100);
        lo = mdiobus_read(bus, 0x10 | r2, r1);
        hi = mdiobus_read(bus, 0x10 | r2, r1 + 1);
        mutex_unlock(&switch_mdio_slave_lock);
        return (hi << 16) | lo;
}

void
qca_ar8216_mii_write(int reg, uint32_t val)
{
        struct mii_bus *bus = miibus;
        uint16_t r1, r2, r3;
        uint16_t lo, hi;

        split_addr((a_uint32_t) reg, &r1, &r2, &r3);
        lo = val & 0xffff;
        hi = (a_uint16_t) (val >> 16);

        mutex_lock(&switch_mdio_lock);
        mdiobus_write(bus, switch_chip_id, switch_chip_reg, r3);
	udelay(100);
        if(SSDK_CURRENT_CHIP_TYPE != CHIP_SHIVA) {
            mdiobus_write(bus, 0x10 | r2, r1, lo);
            mdiobus_write(bus, 0x10 | r2, r1 + 1, hi);
        } else {
            mdiobus_write(bus, 0x10 | r2, r1 + 1, hi);
            mdiobus_write(bus, 0x10 | r2, r1, lo);
        }
        mutex_unlock(&switch_mdio_lock);
}

void
qca_ar8216_mii_slave_write(int reg, uint32_t val)
{
        struct mii_bus *bus = miibus_slave;
        uint16_t r1, r2, r3;
        uint16_t lo, hi;

        split_addr((a_uint32_t) reg, &r1, &r2, &r3);
        lo = val & 0xffff;
        hi = (a_uint16_t) (val >> 16);

        mutex_lock(&switch_mdio_slave_lock);
        mdiobus_write(bus, switch_chip_id, switch_chip_reg, r3);
	udelay(100);
        if(SSDK_CURRENT_CHIP_TYPE != CHIP_SHIVA) {
            mdiobus_write(bus, 0x10 | r2, r1, lo);
            mdiobus_write(bus, 0x10 | r2, r1 + 1, hi);
        } else {
            mdiobus_write(bus, 0x10 | r2, r1 + 1, hi);
            mdiobus_write(bus, 0x10 | r2, r1, lo);
        }
        mutex_unlock(&switch_mdio_slave_lock);
}

a_bool_t
phy_addr_validation_check(a_uint32_t phy_addr)
{

	if (phy_addr  == SSDK_PHY_BCAST_ID)
		return A_TRUE;
	else if ((phy_addr > SSDK_PSGMII_ID) || (phy_addr < SSDK_PHY_MIN_ID))
		return A_FALSE;
	else
		return A_TRUE;
}

sw_error_t
qca_ar8327_phy_read(a_uint32_t dev_id, a_uint32_t phy_addr,
                           a_uint32_t reg, a_uint16_t* data)
{
	struct mii_bus *bus = NULL;
	int phy_dest_addr;
	if (dev_id == 0)
		bus = miibus;
	else
		bus = miibus_slave;
	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}
	if (phy_addr == SSDK_PSGMII_ID)
		phy_dest_addr = phy_address[phy_addr -1] + 1;
	else if (phy_addr == SSDK_PHY_BCAST_ID)
		phy_dest_addr = SSDK_PHY_BCAST_ID;
	else
		phy_dest_addr = phy_address[phy_addr];

	*data = mdiobus_read(bus, phy_dest_addr, reg);
	return 0;
}

int r9000_trunk_ports_phy_read  (int phy_id, int location)
{
	a_uint16_t data;

	if (qca_ar8327_phy_read (0, phy_id, location, &data))
		return -1;
	return data;
}
EXPORT_SYMBOL(r9000_trunk_ports_phy_read);

sw_error_t
qca_ar8327_phy_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data)
{
	struct mii_bus *bus = NULL;
	int phy_dest_addr;
	if (dev_id == 0)
		bus = miibus;
	else
		bus = miibus_slave;
	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}
	if (phy_addr == SSDK_PSGMII_ID)
		phy_dest_addr = phy_address[phy_addr -1] + 1;
	else if (phy_addr == SSDK_PHY_BCAST_ID)
		phy_dest_addr = SSDK_PHY_BCAST_ID;
	else
		phy_dest_addr = phy_address[phy_addr];

	mdiobus_write(bus, phy_dest_addr, reg, data);
	return 0;
}
void r9000_trunk_ports_phy_write(int phy_id, int location, int val)
{
	qca_ar8327_phy_write (0, phy_id, location, val);
	return ;
}
EXPORT_SYMBOL(r9000_trunk_ports_phy_write);

void
qca_ar8327_phy_dbg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                                a_uint16_t dbg_addr, a_uint16_t dbg_data)
{
	struct mii_bus *bus = NULL;
	int phy_dest_addr;
	if (dev_id == 0)
		bus = miibus;
	else
		bus = miibus_slave;
	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}
	if (phy_addr == SSDK_PSGMII_ID)
		phy_dest_addr = phy_address[phy_addr -1] + 1;
	else if (phy_addr == SSDK_PHY_BCAST_ID)
		phy_dest_addr = SSDK_PHY_BCAST_ID;
	else
		phy_dest_addr = phy_address[phy_addr];

	mdiobus_write(bus, phy_dest_addr, QCA_MII_DBG_ADDR, dbg_addr);
	mdiobus_write(bus, phy_dest_addr, QCA_MII_DBG_DATA, dbg_data);
}

void
qca_ar8327_phy_dbg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
		                a_uint16_t dbg_addr, a_uint16_t *dbg_data)
{
	struct mii_bus *bus = NULL;
	int phy_dest_addr;
	if (dev_id == 0)
		bus = miibus;
	else
		bus = miibus_slave;
	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}
	if (phy_addr == SSDK_PSGMII_ID)
		phy_dest_addr = phy_address[phy_addr -1] + 1;
	else if (phy_addr == SSDK_PHY_BCAST_ID)
		phy_dest_addr = SSDK_PHY_BCAST_ID;
	else
		phy_dest_addr = phy_address[phy_addr];

	mdiobus_write(bus, phy_dest_addr, QCA_MII_DBG_ADDR, dbg_addr);
	*dbg_data = mdiobus_read(bus, phy_dest_addr, QCA_MII_DBG_DATA);
}


void
qca_ar8327_mmd_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                          a_uint16_t addr, a_uint16_t data)
{
	struct mii_bus *bus = NULL;
	int phy_dest_addr;
	if (dev_id == 0)
		bus = miibus;
	else
		bus = miibus_slave;
	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}
	if (phy_addr == SSDK_PSGMII_ID)
		phy_dest_addr = phy_address[phy_addr -1] + 1;
	else if (phy_addr == SSDK_PHY_BCAST_ID)
		phy_dest_addr = SSDK_PHY_BCAST_ID;
	else
		phy_dest_addr = phy_address[phy_addr];

	mdiobus_write(bus, phy_dest_addr, QCA_MII_MMD_ADDR, addr);
	mdiobus_write(bus, phy_dest_addr, QCA_MII_MMD_DATA, data);
}

void qca_phy_mmd_write(u32 dev_id, u32 phy_id,
                     u16 mmd_num, u16 reg_id, u16 reg_val)
{
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR, mmd_num);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_DATA, reg_id);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR,
			0x4000 | mmd_num);
	qca_ar8327_phy_write(dev_id, phy_id,
		QCA_MII_MMD_DATA, reg_val);
}

u16 qca_phy_mmd_read(u32 dev_id, u32 phy_id,
		u16 mmd_num, u16 reg_id)
{
	u16 value = 0;
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR, mmd_num);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_DATA, reg_id);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR,
			0x4000 | mmd_num);
	qca_ar8327_phy_read(dev_id, phy_id,
			QCA_MII_MMD_DATA, &value);
	return value;
}

uint32_t
qca_switch_reg_read(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
	uint32_t reg_val = 0;

	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if ((reg_addr%4)!= 0)
	return SW_BAD_PARAM;

	reg_val = readl(hw_addr + reg_addr);

	aos_mem_copy(reg_data, &reg_val, sizeof (a_uint32_t));
	return 0;
}

sw_error_t
qca_switch_reg_write(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
	uint32_t reg_val = 0;
	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if ((reg_addr%4)!= 0)
	return SW_BAD_PARAM;

	aos_mem_copy(&reg_val, reg_data, sizeof (a_uint32_t));
	writel(reg_val, hw_addr + reg_addr);
	return 0;
}

uint32_t
qca_psgmii_reg_read(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
	uint32_t reg_val = 0;

	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if((reg_addr%4)!=0)
	return SW_BAD_PARAM;

	if (psgmii_hw_addr == NULL)
		return SW_NOT_SUPPORTED;

	reg_val = readl(psgmii_hw_addr + reg_addr);

	aos_mem_copy(reg_data, &reg_val, sizeof (a_uint32_t));
	return 0;
}

uint32_t
qca_psgmii_reg_write(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len)
{
	uint32_t reg_val = 0;
	if (len != sizeof (a_uint32_t))
        return SW_BAD_LEN;

	if((reg_addr%4)!=0)
	return SW_BAD_PARAM;

	if (psgmii_hw_addr == NULL)
		return SW_NOT_SUPPORTED;

	aos_mem_copy(&reg_val, reg_data, sizeof (a_uint32_t));
	writel(reg_val, psgmii_hw_addr + reg_addr);
	return 0;
}

#ifdef BOARD_AR71XX
static uint32_t switch_chip_id_adjuest(void)
{
	uint32_t chip_version = 0;
	chip_version = (qca_ar8216_mii_read(0)&0xff00)>>8;
	if((chip_version !=0) && (chip_version !=0xff))
		return 0;

	switch_chip_id = SHIVA_CHIP_ID;
	switch_chip_reg = SHIVA_CHIP_REG;

	chip_version = (qca_ar8216_mii_read(0)&0xff00)>>8;
	printk("chip_version:0x%x\n", chip_version);
	return 1;
}
#endif

extern struct mii_bus *al_get_mdiobus_by_name(const char *ethname);
extern struct mii_bus * al_get_mdiobus_by_gpio(void);

static int miibus_get(void)
{
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
	struct device_node *mdio_node = NULL;
	struct platform_device *mdio_plat = NULL;
	struct ipq40xx_mdio_data *mdio_data = NULL;

	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_LOCAL_BUS)
		mdio_node = of_find_compatible_node(NULL, NULL, "qcom,ipq40xx-mdio");
	else
		mdio_node = of_find_compatible_node(NULL, NULL, "virtual,mdio-gpio");

	if (!mdio_node) {
		printk("No MDIO node found in DTS!\n");
		return 1;
	}

	mdio_plat = of_find_device_by_node(mdio_node);
	if (!mdio_plat) {
		printk("cannot find platform device from mdio node\n");
		return 1;
	}

	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_LOCAL_BUS)
	{
		mdio_data = dev_get_drvdata(&mdio_plat->dev);
		if (!mdio_data) {
                	printk("cannot get mdio_data reference from device data\n");
                	return 1;
        	}
		miibus = mdio_data->mii_bus;
	}
	else
		miibus = dev_get_drvdata(&mdio_plat->dev);

	if (!miibus) {
		printk("cannot get mii bus reference from device data\n");
		return 1;
	}

#else
#ifdef BOARD_AR71XX
	struct ag71xx_mdio *am;
#endif
	struct device *miidev;
	char busid[MII_BUS_ID_SIZE];
#if 0
	snprintf(busid, MII_BUS_ID_SIZE, "%s.%d",
		IPQ806X_MDIO_BUS_NAME, IPQ806X_MDIO_BUS_NUM);

	miidev = bus_find_device_by_name(&platform_bus_type, NULL, busid);
	if (!miidev) {
		printk("cannot get mii bus\n");
		return 1;
	}

#ifdef BOARD_AR71XX
	am = dev_get_drvdata(miidev);
	miibus = am->mii_bus;
#else
	miibus = dev_get_drvdata(miidev);
#endif

	/*TODO: Try to get miibus_slave*/
	snprintf(busid, MII_BUS_ID_SIZE, "%s.%d",
		"mdio", 4);
	miidev = bus_find_device_by_name(&platform_bus_type, NULL, busid);
	if (!miidev) {
		printk("cannot get mii bus\n");
		return 1;
	}
	printk("get the miidev ok!!!\n");
	miibus_slave = dev_get_drvdata(miidev);
	miibus = miibus_slave;
#endif

	miibus = al_get_mdiobus_by_name("eth1");
	printk("miibus=%p\n", miibus);

	miibus_slave = al_get_mdiobus_by_gpio();
        printk("miibusslave=%p\n", miibus_slave);

#ifdef BOARD_AR71XX
	if(switch_chip_id_adjuest()) {

		snprintf(busid, MII_BUS_ID_SIZE, "%s.%d",
		IPQ806X_MDIO_BUS_NAME, MDIO_BUS_1);

		miidev = bus_find_device_by_name(&platform_bus_type, NULL, busid);
		if (!miidev) {
			printk("cannot get mii bus\n");
			return 1;
		}

		am = dev_get_drvdata(miidev);
		miibus = am->mii_bus;
		printk("chip_version:0x%x\n", (qca_ar8216_mii_read(0)&0xff00)>>8);
	}
#endif

	if((!miibus)||(!miibus_slave)){
		printk("mdio bus '%s' get FAIL\n", busid);
		return 1;
	}
#endif

	return 0;
}


static int ssdk_phy_id_get(ssdk_init_cfg *cfg)
{
	a_uint32_t phy_id = 0;
	a_uint16_t org_id = 0, rev_id = 0;

	cfg->reg_func.mdio_get(0, 0, 2, &org_id);
	cfg->reg_func.mdio_get(0, 0, 3, &rev_id);
	phy_id = (org_id<<16) | rev_id;
	cfg->phy_id = phy_id;
	printk("PHY ID is 0x%x\n",cfg->phy_id);

	return SW_OK;;
}
int ssdk_phy_init(a_uint32_t dev_id, ssdk_init_cfg *cfg)
{

	int size = sizeof(phy_array)/sizeof(phy_identification_t);
	int i = 0;

	for(i=0;i<size;i++)
	{
		if(phy_array[i].phy_id == cfg->phy_id)
			return phy_array[i].init(dev_id);
	}

	return SW_FAIL;
}

#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
struct reset_control *ess_rst = NULL;
void ssdk_ess_reset(void)
{
	if (!ess_rst)
		return;
	reset_control_assert(ess_rst);
	mdelay(10);
	reset_control_deassert(ess_rst);
	mdelay(100);
}

char ssdk_driver_name[] = "ess_ssdk";

static int ssdk_probe(struct platform_device *pdev)
{
	ess_rst = devm_reset_control_get(&pdev->dev, "ess_rst");
	if (!ess_rst) {
		printk("ess rst fail!\n");
		return -1;
	}
	reset_control_assert(ess_rst);
	mdelay(10);
	reset_control_deassert(ess_rst);
	mdelay(100);
	printk("reset ok in probe!\n");
	return 0;
}

static const struct of_device_id ssdk_of_mtable[] = {
        {.compatible = "qcom,ess-switch" },
        {}
};

static struct platform_driver ssdk_driver = {
        .driver = {
                .name    = ssdk_driver_name,
		.owner   = THIS_MODULE,
                .of_match_table = ssdk_of_mtable,
        },
        .probe    = ssdk_probe,
};
#endif
#ifdef DESS
static u32 phy_t_status = 0;
void ssdk_malibu_psgmii_and_dakota_dess_reset()
{
	int m = 0, n = 0;

	/*reset Malibu PSGMII and Dakota ESS start*/
	qca_ar8327_phy_write(0, 5, 0x0, 0x005b);/*fix phy psgmii RX 20bit*/
	qca_ar8327_phy_write(0, 5, 0x0, 0x001b);/*reset phy psgmii*/
	qca_ar8327_phy_write(0, 5, 0x0, 0x005b);/*release reset phy psgmii*/
	/* mdelay(100); this 100ms be replaced with below malibu psgmii calibration process*/
	/*check malibu psgmii calibration done start*/
	n = 0;
	while (n < 100) {
		u16 status;
		status = qca_phy_mmd_read(0, 5, 1, 0x28);
		if (status & BIT(0))
			break;
		mdelay(10);
		n++;
	}
#ifdef PSGMII_DEBUG
	if (n >= 100)
		printk("MALIBU PSGMII PLL_VCO_CALIB NOT READY\n");
#endif
	mdelay(50);
	/*check malibu psgmii calibration done end..*/
	qca_ar8327_phy_write(0, 5, 0x1a, 0x2230);/*freeze phy psgmii RX CDR*/

#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	ssdk_ess_reset();
	/*check dakota psgmii calibration done start*/
	m = 0;
	while (m < 100) {
		u32 status;
		qca_psgmii_reg_read(0, 0xa0, (a_uint8_t *)&status, 4);
		if (status & BIT(0))
			break;
		mdelay(10);
		m++;
	}
#ifdef PSGMII_DEBUG
	if (m >= 100)
		printk("DAKOTA PSGMII PLL_VCO_CALIB NOT READY\n");
#endif
	mdelay(50);
	/*check dakota psgmii calibration done end..*/
#endif
	qca_ar8327_phy_write(0, 5, 0x1a, 0x3230);/*relesae phy psgmii RX CDR*/
	qca_ar8327_phy_write(0, 5, 0x0, 0x005f);/*release phy psgmii RX 20bit*/
	mdelay(200);
	/*reset Malibu PSGMII and Dakota ESS end*/
}

void ssdk_psgmii_single_phy_testing(int phy)
{
	int j = 0;

	u32 tx_counter_ok, tx_counter_error;
	u32 rx_counter_ok, rx_counter_error;
	u32 tx_counter_ok_high16;
	u32 rx_counter_ok_high16;
	u32 tx_ok, rx_ok;
	qca_ar8327_phy_write(0, phy, 0x0, 0x9000);
	qca_ar8327_phy_write(0, phy, 0x0, 0x4140);
	j = 0;
	while (j < 100) {
		u16 status;
		qca_ar8327_phy_read(0, phy, 0x11, &status);
		if (status & (1 << 10))
			break;
		mdelay(10);
		j++;
	}

	/*enable check*/
	qca_phy_mmd_write(0, phy, 7, 0x8029, 0x0000);
	qca_phy_mmd_write(0, phy, 7, 0x8029, 0x0003);

	/*start traffic*/
	qca_phy_mmd_write(0, phy, 7, 0x8020, 0xa000);
	mdelay(200);

	/*check counter*/
	tx_counter_ok = qca_phy_mmd_read(0, phy, 7, 0x802e);
	tx_counter_ok_high16 = qca_phy_mmd_read(0, phy, 7, 0x802d);
	tx_counter_error = qca_phy_mmd_read(0, phy, 7, 0x802f);
	rx_counter_ok = qca_phy_mmd_read(0, phy, 7, 0x802b);
	rx_counter_ok_high16 = qca_phy_mmd_read(0, phy, 7, 0x802a);
	rx_counter_error = qca_phy_mmd_read(0, phy, 7, 0x802c);
	tx_ok = tx_counter_ok + (tx_counter_ok_high16<<16);
	rx_ok = rx_counter_ok + (rx_counter_ok_high16<<16);
	if (tx_ok== 0x3000 && tx_counter_error == 0) {
		/*success*/
		phy_t_status &= (~(1<<phy));
	} else {
#ifdef PSGMII_DEBUG
				printk("tx_ok = 0x%x, rx_ok = 0x%x, tx_counter_error = 0x%x, rx_counter_error = 0x%x\n",
						tx_ok, rx_ok, tx_counter_error, rx_counter_error);
				printk("PHY %d single test PSGMII issue happen \n", phy);
#endif
				phy_t_status |= (1<<phy);
			}

			qca_ar8327_phy_write(0, phy, 0x0, 0x1840);
		}

void ssdk_psgmii_all_phy_testing()
{
	int phy = 0, j = 0;

	qca_ar8327_phy_write(0, 0x1f, 0x0, 0x9000);
	qca_ar8327_phy_write(0, 0x1f, 0x0, 0x4140);
	j = 0;
	while (j < 100) {
		for (phy = 0; phy < 5; phy++) {
			u16 status;
			qca_ar8327_phy_read(0, phy, 0x11, &status);
			if (!(status & (1 << 10)))
				break;
		}

		if (phy >= 5)
			break;
		mdelay(10);
		j++;
	}
	/*enable check*/
	qca_phy_mmd_write(0, 0x1f, 7, 0x8029, 0x0000);
	qca_phy_mmd_write(0, 0x1f, 7, 0x8029, 0x0003);

	/*start traffic*/
	qca_phy_mmd_write(0, 0x1f, 7, 0x8020, 0xa000);
	mdelay(200);
	for (phy = 0; phy < 5; phy++) {
		u32 tx_counter_ok, tx_counter_error;
		u32 rx_counter_ok, rx_counter_error;
		u32 tx_counter_ok_high16;
		u32 rx_counter_ok_high16;
		u32 tx_ok, rx_ok;
		/*check counter*/
		tx_counter_ok = qca_phy_mmd_read(0, phy, 7, 0x802e);
		tx_counter_ok_high16 = qca_phy_mmd_read(0, phy, 7, 0x802d);
		tx_counter_error = qca_phy_mmd_read(0, phy, 7, 0x802f);
		rx_counter_ok = qca_phy_mmd_read(0, phy, 7, 0x802b);
		rx_counter_ok_high16 = qca_phy_mmd_read(0, phy, 7, 0x802a);
		rx_counter_error = qca_phy_mmd_read(0, phy, 7, 0x802c);
		tx_ok = tx_counter_ok + (tx_counter_ok_high16<<16);
		rx_ok = rx_counter_ok + (rx_counter_ok_high16<<16);
		if (tx_ok== 0x3000 && tx_counter_error == 0) {
			/*success*/
			phy_t_status &= (~(1<<(phy+8)));
		} else {
#ifdef PSGMII_DEBUG
				printk("tx_ok = 0x%x, rx_ok = 0x%x, tx_counter_error = 0x%x, rx_counter_error = 0x%x\n",
						tx_ok, rx_ok, tx_counter_error, rx_counter_error);
				printk("PHY %d PSGMII issue happen,Reset PSGMII!!!!!!\n", phy);
#endif
				phy_t_status |= (1<<(phy+8));
			}
		}
#ifdef PSGMII_DEBUG
		printk("PHY all test 0x%x \r\n",phy_t_status);
#endif
}
void ssdk_psgmii_self_test()
{
	int i = 0, phy = 0,j = 0;
	u32 value = 0;

	ssdk_malibu_psgmii_and_dakota_dess_reset();

	qca_ar8327_phy_write(0, 4, 0x1f, 0x8500);/*switch to access MII reg for copper*/
	for(phy = 0; phy < 5; phy++) {
		/*enable phy mdio broadcast write*/
		qca_phy_mmd_write(0, phy, 7, 0x8028, 0x801f);
	}

	/* force no link by power down */
	qca_ar8327_phy_write(0, 0x1f, 0x0, 0x1840);

	/*packet number*/
	qca_phy_mmd_write(0, 0x1f, 7, 0x8021, 0x3000);
	qca_phy_mmd_write(0, 0x1f, 7, 0x8062, 0x05e0);

	/*fix mdi status */
	qca_ar8327_phy_write(0, 0x1f, 0x10, 0x6800);

	for(i = 0; i < 100; i++) {
		phy_t_status = 0;

		for(phy = 0; phy < 5; phy++) {
			value = readl(hw_addr + 0x66c + phy * 0xc);
			writel((value|(1<<21)), (hw_addr + 0x66c + phy * 0xc));
		}

		for (phy = 0; phy < 5; phy++) {
			ssdk_psgmii_single_phy_testing(phy);
		}

		ssdk_psgmii_all_phy_testing();

		if (phy_t_status) {
			ssdk_malibu_psgmii_and_dakota_dess_reset();
		}
		else
		{
		                break;
		}
	}

#ifdef PSGMII_DEBUG
	if (i>=100)
		printk("PSGMII cannot recover\n");
	else
		printk("PSGMII recovered after %d times reset\n",i);
#endif
	/*configuration recover*/
	/*packet number*/
	qca_phy_mmd_write(0, 0x1f, 7, 0x8021, 0x0);
	/*disable check*/
	qca_phy_mmd_write(0, 0x1f, 7, 0x8029, 0x0);
	/*disable traffic*/
	qca_phy_mmd_write(0, 0x1f, 7, 0x8020, 0x0);
}


void clear_self_test_config()
{
	int phy = 0;
	u32 value = 0;
	/* disable EEE */
/*	qca_phy_mmd_write(0, 0x1f, 0x7,  0x3c, 0x0); */

	/*disable phy internal loopback*/
	qca_ar8327_phy_write(0, 0x1f, 0x10, 0x6860);
	qca_ar8327_phy_write(0, 0x1f, 0x0, 0x9040);

	for(phy = 0; phy < 5; phy++)
	{
		/*disable mac loop back*/
		value = readl(hw_addr+0x66c+phy*0xc);
		writel((value&(~(1<<21))), (hw_addr+0x66c+phy*0xc));
		/*diable phy mdio broadcast write*/
		qca_phy_mmd_write(0, phy, 7, 0x8028, 0x001f);
	}

	/* clear fdb entry */
	fal_fdb_del_all(0,1);
}
#endif

static ssize_t ssdk_dev_id_get(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	ssize_t count;
	a_uint32_t num;

	num = (a_uint32_t)ssdk_dev_id;

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%u", num);
	return count;
}

static ssize_t ssdk_dev_id_set(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	char num_buf[12];
	a_uint32_t num;

	if (count >= sizeof(num_buf)) return 0;
	memcpy(num_buf, buf, count);
	num_buf[count] = '\0';
	sscanf(num_buf, "%u", &num);

	ssdk_dev_id = num;

	return count;
}

static const struct device_attribute ssdk_dev_id_attr =
	__ATTR(dev_id, 0660, ssdk_dev_id_get, ssdk_dev_id_set);
struct kobject *ssdk_sys = NULL;

int ssdk_dev_id_procfs_init (void)
{
	int ret = 0;

	/* create /sys/ssdk/ dir */
	ssdk_sys = kobject_create_and_add("ssdk", NULL);
	if (!ssdk_sys) {
		printk("Failed to register SSDK sysfs\n");
		return ret;
	}

	/* create /sys/ssdk/dev_id file */
	ret = sysfs_create_file(ssdk_sys, &ssdk_dev_id_attr.attr);
	if (ret) {
		printk("Failed to register SSDK dev id SysFS file\n");
		kobject_put(ssdk_sys);
		return ret;
	}

	return 0;
}

void ssdk_dev_id_procfs_exit (void)
{
	sysfs_remove_file(ssdk_sys, &ssdk_dev_id_attr.attr);
	kobject_put(ssdk_sys);
}

int
ssdk_plat_init(ssdk_init_cfg *cfg)
{
	#ifdef BOARD_AR71XX
	int rv = 0;
	#endif
	printk("ssdk_plat_init start\n");

	ssdk_dev_id_procfs_init();

	mutex_init(&switch_mdio_lock);
	mutex_init(&switch_mdio_slave_lock);

	if(miibus_get())
		return -ENODEV;

	#ifdef DESS
	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_LOCAL_BUS) {
		/* Enable ess clock here */
		if(!IS_ERR(ssdk_dt_global.ess_clk))
		{
			printk("enable ess clk\n");
			clk_prepare_enable(ssdk_dt_global.ess_clk);
		}

		if (!request_mem_region(ssdk_dt_global.switchreg_base_addr,
				ssdk_dt_global.switchreg_size, "switch_mem")) {
			printk("%s Unable to request resource.", __func__);
			return -1;
		}

		hw_addr = ioremap_nocache(ssdk_dt_global.switchreg_base_addr,
				ssdk_dt_global.switchreg_size);
		if (!hw_addr) {
			printk("%s ioremap fail.", __func__);
			return -1;
		}
		cfg->reg_mode = HSL_HEADER;
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
		platform_driver_register(&ssdk_driver);
#endif
	}

	if(ssdk_dt_global.psgmii_reg_access_mode == HSL_REG_LOCAL_BUS) {
		if (!request_mem_region(ssdk_dt_global.psgmiireg_base_addr,
				ssdk_dt_global.psgmiireg_size, "psgmii_mem")) {
			printk("%s Unable to request psgmii resource.", __func__);
			return -1;
		}

		psgmii_hw_addr = ioremap_nocache(ssdk_dt_global.psgmiireg_base_addr,
				ssdk_dt_global.psgmiireg_size);
		if (!psgmii_hw_addr) {
			printk("%s ioremap fail.", __func__);
			cfg->reg_func.psgmii_reg_set = NULL;
			cfg->reg_func.psgmii_reg_get = NULL;
			return -1;
		}

		cfg->reg_func.psgmii_reg_set = qca_psgmii_reg_write;
		cfg->reg_func.psgmii_reg_get = qca_psgmii_reg_read;
	}
	#endif

	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_MDIO) {
		if(driver_find(qca_phy_driver.name, &mdio_bus_type)){
			printk("QCA PHY driver had been Registered\n");
			return 0;
		}
		cfg->reg_mode = HSL_MDIO;

		printk("Register QCA PHY driver\n");
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,8,0)
		#ifndef BOARD_AR71XX
		return phy_driver_register(&qca_phy_driver,THIS_MODULE);
		#else
		rv = phy_driver_register(&qca_phy_driver,THIS_MODULE);
		ssdk_uci_takeover_init();
		return rv;
		#endif
#else
		#ifndef BOARD_AR71XX
		return phy_driver_register(&qca_phy_driver);
		#else
		rv = phy_driver_register(&qca_phy_driver);
		ssdk_uci_takeover_init();
		return rv;
		#endif

#endif
	} else
		return 0;

}

void
ssdk_plat_exit(void)
{
    printk("ssdk_plat_exit\n");

	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_MDIO) {
#ifndef BOARD_AR71XX
		phy_driver_unregister(&qca_phy_driver);
#endif

	#ifdef BOARD_AR71XX
		ssdk_uci_takeover_exit();
	#endif
	}

	if (ssdk_dt_global.switch_reg_access_mode == HSL_REG_LOCAL_BUS) {
		iounmap(hw_addr);
		iounmap(psgmii_hw_addr);
		release_mem_region(ssdk_dt_global.switchreg_base_addr,
					ssdk_dt_global.switchreg_size);
		release_mem_region(ssdk_dt_global.psgmiireg_base_addr,
					ssdk_dt_global.psgmiireg_size);
#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
		platform_driver_unregister(&ssdk_driver);
#endif
	}

}

sw_error_t
ssdk_init(a_uint32_t dev_id, ssdk_init_cfg * cfg)
{
	sw_error_t rv;

	rv = fal_init(dev_id, cfg);
	if (rv != SW_OK)
		printk("ssdk fal init failed \r\n");

	ssdk_phy_init(dev_id, cfg);
	if (rv != SW_OK)
		printk("ssdk phy init failed \r\n");

	return rv;
}

sw_error_t
ssdk_cleanup(void)
{
	sw_error_t rv;

	rv = fal_cleanup();

	return rv;
}

sw_error_t
ssdk_hsl_access_mode_set(a_uint32_t dev_id, hsl_access_mode reg_mode)
{
    sw_error_t rv;

    rv = hsl_access_mode_set(dev_id, reg_mode);
    return rv;
}

void switch_cpuport_setup(void)
{
	#ifdef IN_PORTCONTROL
	//According to HW suggestion, enable CPU port flow control for Dakota
	fal_port_flowctrl_forcemode_set(0, 0, A_TRUE);
	fal_port_flowctrl_set(0, 0, A_TRUE);

	fal_port_duplex_set(0, 0, FAL_FULL_DUPLEX);
	fal_port_speed_set(0, 0, FAL_SPEED_1000);
	fal_port_txmac_status_set(0, 0, A_TRUE);
	fal_port_rxmac_status_set(0, 0, A_TRUE);
	#endif
}

#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
static int ssdk_dt_parse(ssdk_init_cfg *cfg)
{
	struct device_node *switch_node = NULL,*mdio_node = NULL;
	struct device_node *psgmii_node = NULL;
	struct device_node *child = NULL;
	a_uint32_t len = 0,i = 0,j = 0;
	const __be32 *reg_cfg, *mac_mode,*led_source,*led_mode, *led_speed, *led_freq, *phy_addr;
	a_uint8_t *led_str;


	/*
	 * Get reference to ESS SWITCH device node
	 */
	switch_node = of_find_node_by_name(NULL, "ess-switch");
	if (!switch_node) {
		printk("cannot find ess-switch node\n");
		return SW_BAD_PARAM;
	}
	printk("ess-switch DT exist!\n");

	reg_cfg = of_get_property(switch_node, "reg", &len);
	if(!reg_cfg) {
		printk("%s: error reading device node properties for reg\n", switch_node->name);
		return SW_BAD_PARAM;
	}

	ssdk_dt_global.switchreg_base_addr = be32_to_cpup(reg_cfg);
	ssdk_dt_global.switchreg_size = be32_to_cpup(reg_cfg + 1);

	if (of_property_read_string(switch_node, "switch_access_mode", &ssdk_dt_global.reg_access_mode)) {
		printk("%s: error reading device node properties for switch_access_mode\n", switch_node->name);
		return SW_BAD_PARAM;
	}

	ssdk_dt_global.ess_clk = of_clk_get_by_name(switch_node, "ess_clk");
	if (IS_ERR(ssdk_dt_global.ess_clk))
		printk("Getting ess_clk failed!\n");

	printk("switchreg_base_addr: 0x%x\n", ssdk_dt_global.switchreg_base_addr);
	printk("switchreg_size: 0x%x\n", ssdk_dt_global.switchreg_size);
	printk("switch_access_mode: %s\n", ssdk_dt_global.reg_access_mode);
	if(!strcmp(ssdk_dt_global.reg_access_mode, "local bus"))
		ssdk_dt_global.switch_reg_access_mode = HSL_REG_LOCAL_BUS;
	else if(!strcmp(ssdk_dt_global.reg_access_mode, "mdio"))
		ssdk_dt_global.switch_reg_access_mode = HSL_REG_MDIO;
	else
		ssdk_dt_global.switch_reg_access_mode = HSL_REG_MDIO;

	if (of_property_read_u32(switch_node, "switch_cpu_bmp", &cfg->port_cfg.cpu_bmp)
		|| of_property_read_u32(switch_node, "switch_lan_bmp", &cfg->port_cfg.lan_bmp)
		|| of_property_read_u32(switch_node, "switch_wan_bmp", &cfg->port_cfg.wan_bmp)) {
		printk("%s: error reading device node port properties\n", switch_node->name);
		return SW_BAD_PARAM;
	}
	printk("wan bmp:0x%x\n", cfg->port_cfg.wan_bmp);

	psgmii_node = of_find_node_by_name(NULL, "ess-psgmii");
	if (!psgmii_node) {
		return SW_BAD_PARAM;
	}
	printk("ess-psgmii DT exist!\n");
	reg_cfg = of_get_property(psgmii_node, "reg", &len);
	if(!reg_cfg) {
		printk("%s: error reading device node properties for reg\n", psgmii_node->name);
		return SW_BAD_PARAM;
	}

	ssdk_dt_global.psgmiireg_base_addr = be32_to_cpup(reg_cfg);
	ssdk_dt_global.psgmiireg_size = be32_to_cpup(reg_cfg + 1);
	if (of_property_read_string(psgmii_node, "psgmii_access_mode", &ssdk_dt_global.psgmii_reg_access_str)) {
		printk("%s: error reading device node properties for psmgii_access_mode\n", psgmii_node->name);
		return SW_BAD_PARAM;
	}
	if(!strcmp(ssdk_dt_global.psgmii_reg_access_str, "local bus"))
		ssdk_dt_global.psgmii_reg_access_mode = HSL_REG_LOCAL_BUS;

	mac_mode = of_get_property(switch_node, "switch_mac_mode", &len);
	if(!mac_mode) {
		printk("%s: error reading device node properties for mac mode\n", switch_node->name);
		return SW_BAD_PARAM;
	}
	cfg->mac_mode = be32_to_cpup(mac_mode);
	printk("mac mode=%d\n", be32_to_cpup(mac_mode));
	ssdk_dt_global.mac_mode = cfg->mac_mode;
	printk("current mac mode = %d\n", ssdk_dt_global.mac_mode);
	for_each_available_child_of_node(switch_node, child) {

		led_source = of_get_property(child, "source", &len);
		if (led_source)
			cfg->led_source_cfg[i].led_source_id = be32_to_cpup(led_source);
		if (!of_property_read_string(child, "mode", &led_str)) {
			if (!strcmp(led_str, "normal"))
			cfg->led_source_cfg[i].led_pattern.mode = LED_PATTERN_MAP_EN;
			if (!strcmp(led_str, "on"))
			cfg->led_source_cfg[i].led_pattern.mode = LED_ALWAYS_ON;
			if (!strcmp(led_str, "blink"))
			cfg->led_source_cfg[i].led_pattern.mode = LED_ALWAYS_BLINK;
			if (!strcmp(led_str, "off"))
			cfg->led_source_cfg[i].led_pattern.mode = LED_ALWAYS_OFF;
		}
		if (!of_property_read_string(child, "speed", &led_str)) {
			if (!strcmp(led_str, "10M"))
			cfg->led_source_cfg[i].led_pattern.map = LED_MAP_10M_SPEED;
			if (!strcmp(led_str, "100M"))
			cfg->led_source_cfg[i].led_pattern.map = LED_MAP_100M_SPEED;
			if (!strcmp(led_str, "100M"))
			cfg->led_source_cfg[i].led_pattern.map = LED_MAP_1000M_SPEED;
			if (!strcmp(led_str, "all"))
			cfg->led_source_cfg[i].led_pattern.map = LED_MAP_ALL_SPEED;
		}
		if (!of_property_read_string(child, "freq", &led_str)) {
			if (!strcmp(led_str, "2Hz"))
			cfg->led_source_cfg[i].led_pattern.freq = LED_BLINK_2HZ;
			if (!strcmp(led_str, "4Hz"))
			cfg->led_source_cfg[i].led_pattern.freq = LED_BLINK_4HZ;
			if (!strcmp(led_str, "8Hz"))
			cfg->led_source_cfg[i].led_pattern.freq = LED_BLINK_8HZ;
			if (!strcmp(led_str, "auto"))
			cfg->led_source_cfg[i].led_pattern.freq = LED_BLINK_TXRX;
		}
		i++;
	}
	cfg->led_source_num = i;
	printk("current dts led_source_num is %d\n",cfg->led_source_num);

	mdio_node = of_find_node_by_name(NULL, "mdio");
	if (!mdio_node) {
		printk("cannot find mdio node\n");
		return SW_BAD_PARAM;
	}
	printk("mdio DT exist!\n");

	for_each_available_child_of_node(mdio_node, child) {

		phy_addr = of_get_property(child, "reg", &len);
		if (phy_addr)
			phy_address[j] = be32_to_cpup(phy_addr);
		j++;
		if (j >= 5)
			break;
	}
	return SW_OK;
}
#endif

static int chip_ver_get(ssdk_init_cfg* cfg)
{
	int rv = 0;
	a_uint8_t chip_ver = 0;
	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_MDIO)
		chip_ver = (qca_ar8216_mii_read(0)&0xff00)>>8;
	else {
		a_uint32_t reg_val;
		qca_switch_reg_read(0,0,(a_uint8_t *)&reg_val, 4);
		chip_ver = (reg_val&0xff00)>>8;
	}

	if(chip_ver == 0x02)
		cfg->chip_type = CHIP_SHIVA;
	else if(chip_ver == 0x13)
		cfg->chip_type = CHIP_ISISC;
	else if(chip_ver == 0x12)
		cfg->chip_type = CHIP_ISIS;
	else if(chip_ver == 0x14)
		cfg->chip_type = CHIP_DESS;
	else
		rv = -ENODEV;

	return rv;
}

static int ssdk_flow_default_act_init(void)
{
	a_uint32_t vrf_id = 0;
	fal_flow_type_t type = 0;
	for(vrf_id = FAL_MIN_VRF_ID; vrf_id <= FAL_MAX_VRF_ID; vrf_id++)
	{
		for(type = FAL_FLOW_LAN_TO_LAN; type <= FAL_FLOW_WAN_TO_WAN; type++)
		{
			#ifdef IN_IP
			fal_default_flow_cmd_set(0, vrf_id, type, FAL_DEFAULT_FLOW_ADMIT_ALL);
			#endif
		}
	}

	return 0;
}
static int ssdk_portvlan_init(a_uint32_t cpu_bmp, a_uint32_t lan_bmp, a_uint32_t wan_bmp)
{
	a_uint32_t port = 0;
	for(port = 0; port < SSDK_MAX_PORT_NUM; port++)
	{
		if(cpu_bmp & (1 << port))
		{
			#ifdef IN_PORTVLAN
			fal_portvlan_member_update(0, 0, lan_bmp|wan_bmp);
			#endif
		}
		if(lan_bmp & (1 << port))
		{
			#ifdef IN_PORTVLAN
			fal_portvlan_member_update(0, port, (lan_bmp|cpu_bmp)&(~(1<<port)));
			#endif
		}
		if(wan_bmp & (1 << port))
		{
			#ifdef IN_PORTVLAN
			fal_portvlan_member_update(0, port, (wan_bmp|cpu_bmp)&(~(1<<port)));
			#endif
		}
	}
	return 0;
}
#ifdef DESS
static int ssdk_dess_led_init(ssdk_init_cfg *cfg)
{
	a_uint32_t i,led_num, led_source_id,source_id;
	led_ctrl_pattern_t  pattern;

	if(cfg->led_source_num != 0) {
		for (i = 0; i < cfg->led_source_num; i++) {

			led_source_id = cfg->led_source_cfg[i].led_source_id;
			pattern.mode = cfg->led_source_cfg[i].led_pattern.mode;
			pattern.map = cfg->led_source_cfg[i].led_pattern.map;
			pattern.freq = cfg->led_source_cfg[i].led_pattern.freq;
			#ifdef IN_LED
			fal_led_source_pattern_set(0, led_source_id,&pattern);
			#endif
			led_num = ((led_source_id-1)/3) + 3;
			source_id = led_source_id%3;
		#if 0 // defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
			if (source_id == 1) {
				if (led_source_id == 1) {
					ipq40xx_led_source_select(led_num, LAN0_1000_LNK_ACTIVITY);
				}
				if (led_source_id == 4) {
					ipq40xx_led_source_select(led_num, LAN1_1000_LNK_ACTIVITY);
				}
				if (led_source_id == 7) {
					ipq40xx_led_source_select(led_num, LAN2_1000_LNK_ACTIVITY);
				}
				if (led_source_id == 10) {
					ipq40xx_led_source_select(led_num, LAN3_1000_LNK_ACTIVITY);
				}
				if (led_source_id == 13) {
					ipq40xx_led_source_select(led_num, WAN_1000_LNK_ACTIVITY);
				}
			}
			if (source_id == 2) {
				if (led_source_id == 2) {
					ipq40xx_led_source_select(led_num, LAN0_100_LNK_ACTIVITY);
				}
				if (led_source_id == 5) {
					ipq40xx_led_source_select(led_num, LAN1_100_LNK_ACTIVITY);
				}
				if (led_source_id == 8) {
					ipq40xx_led_source_select(led_num, LAN2_100_LNK_ACTIVITY);
				}
				if (led_source_id == 11) {
					ipq40xx_led_source_select(led_num, LAN3_100_LNK_ACTIVITY);
				}
				if (led_source_id == 14) {
					ipq40xx_led_source_select(led_num, WAN_100_LNK_ACTIVITY);
				}
			}
			if (source_id == 0) {
				if (led_source_id == 3) {
					ipq40xx_led_source_select(led_num, LAN0_10_LNK_ACTIVITY);
				}
				if (led_source_id == 6) {
					ipq40xx_led_source_select(led_num, LAN1_10_LNK_ACTIVITY);
				}
				if (led_source_id == 9) {
					ipq40xx_led_source_select(led_num, LAN2_10_LNK_ACTIVITY);
				}
				if (led_source_id == 12) {
					ipq40xx_led_source_select(led_num, LAN3_10_LNK_ACTIVITY);
				}
				if (led_source_id == 15) {
					ipq40xx_led_source_select(led_num, WAN_10_LNK_ACTIVITY);
				}
			}
		#endif
		}
	}
	return 0;
}

static int ssdk_dess_mac_mode_init(a_uint32_t mac_mode)
{
	a_uint32_t reg_value;
	u8  __iomem      *gcc_addr = NULL;

	switch(mac_mode) {
		case PORT_WRAPPER_PSGMII:
			reg_value = 0x2200;
			qca_psgmii_reg_write(0, DESS_PSGMII_MODE_CONTROL,
								(a_uint8_t *)&reg_value, 4);
			reg_value = 0x8380;
			qca_psgmii_reg_write(0, DESS_PSGMIIPHY_TX_CONTROL,
								(a_uint8_t *)&reg_value, 4);
			break;
		case PORT_WRAPPER_SGMII0_RGMII5:
		case PORT_WRAPPER_SGMII1_RGMII5:
		case PORT_WRAPPER_SGMII0_RGMII4:
		case PORT_WRAPPER_SGMII1_RGMII4:
		case PORT_WRAPPER_SGMII4_RGMII4:

		/*config sgmii */
			if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII5)
				||(mac_mode == PORT_WRAPPER_SGMII0_RGMII4)) {
				/*PSGMII channnel 0 as SGMII*/
				reg_value = 0x2001;
				fal_psgmii_reg_set(0, 0x1b4,
								(a_uint8_t *)&reg_value, 4);
				udelay(1000);
			}
			if ((mac_mode == PORT_WRAPPER_SGMII1_RGMII5)
				||(mac_mode == PORT_WRAPPER_SGMII1_RGMII4)) {
				/*PSGMII channnel 1 as SGMII*/
				reg_value = 0x2003;
				fal_psgmii_reg_set(0, 0x1b4,
								(a_uint8_t *)&reg_value, 4);
				udelay(1000);
			}
			if ((mac_mode == PORT_WRAPPER_SGMII4_RGMII4)) {
				/*PSGMII channnel 4 as SGMII*/
				reg_value = 0x2005;
				fal_psgmii_reg_set(0, 0x1b4,
								(a_uint8_t *)&reg_value, 4);
				udelay(1000);
			}

			/*clock gen 1*/
			reg_value = 0xea6;
			fal_psgmii_reg_set(0, 0x13c,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);
			/*softreset psgmii, fixme*/
			gcc_addr = ioremap_nocache(0x1812000, 0x200);
			if (!gcc_addr) {
				printk("gcc map fail!\n");
				return 0;
			} else {
				printk("gcc map success!\n");
				writel(0x20, gcc_addr+0xc);
				mdelay(10);
				writel(0x0, gcc_addr+0xc);
				mdelay(10);
				iounmap(gcc_addr);
			}
			/*relock pll*/
			reg_value = 0x2803;
			fal_psgmii_reg_set(0, DESS_PSGMII_PLL_VCO_RELATED_CONTROL_1,
							(a_uint8_t *)&reg_value, 4);
			udelay(1000);
			reg_value = 0x4ADA;
			fal_psgmii_reg_set(0, DESS_PSGMII_VCO_CALIBRATION_CONTROL_1,
							(a_uint8_t *)&reg_value, 4);
			udelay(1000);
			reg_value = 0xADA;
			fal_psgmii_reg_set(0, DESS_PSGMII_VCO_CALIBRATION_CONTROL_1,
							(a_uint8_t *)&reg_value, 4);
			udelay(1000);

			/* Reconfig channel 0 as SGMII and re autoneg*/
			if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII5)
				||(mac_mode == PORT_WRAPPER_SGMII0_RGMII4)) {
			/*PSGMII channnel 0 as SGMII*/
			reg_value = 0x2001;
			fal_psgmii_reg_set(0, 0x1b4,
								(a_uint8_t *)&reg_value, 4);
			udelay(1000);
			/* restart channel 0 autoneg*/
			reg_value = 0xc4;
			fal_psgmii_reg_set(0, 0x1c8,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);
			reg_value = 0x44;
			fal_psgmii_reg_set(0, 0x1c8,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);
			}
			/* Reconfig channel 1 as SGMII and re autoneg*/
			if ((mac_mode == PORT_WRAPPER_SGMII1_RGMII5)
				||(mac_mode == PORT_WRAPPER_SGMII1_RGMII4)) {

			/*PSGMII channnel 1 as SGMII*/
			reg_value = 0x2003;
			fal_psgmii_reg_set(0, 0x1b4,
							(a_uint8_t *)&reg_value, 4);
			udelay(1000);
			/* restart channel 1 autoneg*/
			reg_value = 0xc4;
			fal_psgmii_reg_set(0, 0x1e0,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);
			reg_value = 0x44;
			fal_psgmii_reg_set(0, 0x1e0,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);

			}
			/* Reconfig channel 4 as SGMII and re autoneg*/
			if ((mac_mode == PORT_WRAPPER_SGMII4_RGMII4)) {
			/*PSGMII channnel 4 as SGMII*/
			reg_value = 0x2005;
			fal_psgmii_reg_set(0, 0x1b4,
							(a_uint8_t *)&reg_value, 4);
			udelay(1000);
			/* restart channel 4 autoneg*/
			reg_value = 0xc4;
			fal_psgmii_reg_set(0, 0x228,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);
			reg_value = 0x44;
			fal_psgmii_reg_set(0, 0x228,
							(a_uint8_t *)&reg_value, 4);
			mdelay(10);
			}

		  	/* config RGMII*/
			reg_value = 0x400;
			fal_reg_set(0, 0x4, (a_uint8_t *)&reg_value, 4);
			/* config mac5 RGMII*/
			if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII5)
				||(mac_mode == PORT_WRAPPER_SGMII1_RGMII5)) {
				qca_ar8327_phy_dbg_write(0, 4, 0x5, 0x2d47);
				qca_ar8327_phy_dbg_write(0, 4, 0xb, 0xbc40);
				qca_ar8327_phy_dbg_write(0, 4, 0x0, 0x82ee);
				reg_value = 0x7e;
				qca_switch_reg_write(0, 0x90, (a_uint8_t *)&reg_value, 4);
			}
			/* config mac4 RGMII*/
			if ((mac_mode == PORT_WRAPPER_SGMII0_RGMII4)
				||(mac_mode == PORT_WRAPPER_SGMII1_RGMII4)
				||(mac_mode == PORT_WRAPPER_SGMII4_RGMII4)) {
				qca_ar8327_phy_dbg_write(0, 4, 0x5, 0x2d47);
				qca_ar8327_phy_dbg_write(0, 4, 0xb, 0xbc40);
				qca_ar8327_phy_dbg_write(0, 4, 0x0, 0x82ee);
				reg_value = 0x7e;
				qca_switch_reg_write(0, 0x8c, (a_uint8_t *)&reg_value, 4);
			}
			break;

	}
	return 0;
}

static int
qca_dess_hw_init(ssdk_init_cfg *cfg)
{
	a_uint32_t reg_value;
	hsl_api_t *p_api;

	qca_switch_init(0);
	ssdk_portvlan_init(cfg->port_cfg.cpu_bmp, cfg->port_cfg.lan_bmp, cfg->port_cfg.wan_bmp);

	#ifdef IN_PORTVLAN
	fal_port_rxhdr_mode_set(0, 0, FAL_ALL_TYPE_FRAME_EN);
	#endif
	#ifdef IN_IP
	fal_ip_route_status_set(0, A_TRUE);
	#endif

	ssdk_flow_default_act_init();

	/*set normal hash and disable nat/napt*/
	qca_switch_reg_read(0, 0x0e38, (a_uint8_t *)&reg_value, 4);
	reg_value = (reg_value|0x1000000|0x8);
	reg_value &= ~2;
	qca_switch_reg_write(0, 0x0e38, (a_uint8_t *)&reg_value, 4);
	#ifdef IN_IP
	fal_ip_vrf_base_addr_set(0, 0, 0);
	#endif

	p_api = hsl_api_ptr_get (0);
	if (p_api && p_api->port_flowctrl_thresh_set)
		p_api->port_flowctrl_thresh_set(0, 0, SSDK_PORT0_FC_THRESH_ON_DFLT,
							SSDK_PORT0_FC_THRESH_OFF_DFLT);

	if (p_api && p_api->ip_glb_lock_time_set)
		p_api->ip_glb_lock_time_set(0, FAL_GLB_LOCK_TIME_100US);


	/*config psgmii,sgmii or rgmii mode for Dakota*/
	ssdk_dess_mac_mode_init(cfg->mac_mode);

	/*add BGA Board led contorl*/
	ssdk_dess_led_init(cfg);

	return 0;
}
#endif

static void ssdk_cfg_default_init(ssdk_init_cfg *cfg)
{
	memset(cfg, 0, sizeof(ssdk_init_cfg));
	cfg->cpu_mode = HSL_CPU_1;
	cfg->nl_prot = 30;
	cfg->reg_func.mdio_set = qca_ar8327_phy_write;
	cfg->reg_func.mdio_get = qca_ar8327_phy_read;
	cfg->reg_func.header_reg_set = qca_switch_reg_write;
	cfg->reg_func.header_reg_get = qca_switch_reg_read;
	/*will delete later, if dts merged*/
	cfg->port_cfg.cpu_bmp = 0x1;
	cfg->port_cfg.lan_bmp = 0x1e;
	cfg->port_cfg.wan_bmp = 0x20;
}

#ifdef IN_RFS
#if defined(CONFIG_RFS_ACCEL)
int ssdk_netdev_rfs_cb(
		struct net_device *dev,
		__be32 src, __be32 dst,
		__be16 sport, __be16 dport,
		u8 proto, u16 rxq_index, u32 action)
{
	return ssdk_rfs_ipct_rule_set(src, dst, sport, dport,
							proto, rxq_index, action);
}

static int ssdk_dev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);

	switch (event) {
		case NETDEV_UP:
			if (strstr(dev->name, "eth")) {
				if (dev->netdev_ops && dev->netdev_ops->ndo_register_rfs_filter) {
					dev->netdev_ops->ndo_register_rfs_filter(dev,
						ssdk_netdev_rfs_cb);
				}
			}
			break;
	}
	return NOTIFY_DONE;
}
#endif
int ssdk_intf_search(
	fal_intf_mac_entry_t *exist_entry, int num,
	fal_intf_mac_entry_t *new_entry, int *index)
{
	int i = 0;
	*index = 0xffffffff;
	for (i = 0; i < num; i++) {
		if (exist_entry[i].vid_high == 0 && exist_entry[i].vid_low == 0)
			*index = i;
		if (!memcmp(exist_entry[i].mac_addr.uc, new_entry->mac_addr.uc, 6) &&
			exist_entry[i].vid_low == new_entry->vid_low) {
			*index = i;
			return 1;
		}
	}
	return 0;
}

void ssdk_intf_set(struct net_device *dev, char op)
{
	a_uint8_t *devmac = NULL;
	fal_vlan_t entry;
	a_uint32_t tmp_vid = 0xffffffff;
	fal_intf_mac_entry_t intf_entry;
	char wan_port = 0;
	sw_error_t rv;
	static fal_intf_mac_entry_t if_mac_entry[8] = {{0}};
	int index = 0xffffffff;
	/*get mac*/
	devmac = (a_uint8_t*)(dev->dev_addr);
	/*get wan port*/
	wan_port = hsl_dev_wan_port_get(0);
	/*get vid*/
	memset(&intf_entry, 0, sizeof(intf_entry));
	intf_entry.ip4_route = 1;
	intf_entry.ip6_route = 1;
	memcpy(&intf_entry.mac_addr, devmac, 6);

	#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	tmp_vid = vlan_dev_vlan_id(dev);
	if (tmp_vid) {
		intf_entry.vid_low = tmp_vid;
		intf_entry.vid_high = intf_entry.vid_low;
		if (op) {
			if (!ssdk_intf_search(if_mac_entry, 8, &intf_entry, &index)) {
				if (index != 0xffffffff) {
					#ifdef IN_IP
					rv = fal_ip_intf_entry_add(0, &intf_entry);
					#endif
					if (SW_OK == rv) {
						if_mac_entry[index] = intf_entry;
					}
				}
			}
		}
		else {
			if (ssdk_intf_search(if_mac_entry, 8, &intf_entry, &index)) {
				intf_entry.entry_id = if_mac_entry[index].entry_id;
				#ifdef IN_IP
				fal_ip_intf_entry_del(0, 1, &intf_entry);
				#endif
				memset(&if_mac_entry[index], 0, sizeof(fal_intf_mac_entry_t));
			}
		}
		return;
	} else {
		tmp_vid = 0xffffffff;
	}
	#endif
	while(1) {
		#ifdef IN_VLAN
		if (SW_OK != fal_vlan_next(0, tmp_vid, &entry))
		#endif
			break;
		tmp_vid = entry.vid;
		if (tmp_vid != 0) {
			if(entry.mem_ports & wan_port) {
				if (!strcmp(dev->name, "eth0")) {
					intf_entry.vid_low = tmp_vid;
					intf_entry.vid_high = intf_entry.vid_low;
					if (op) {
						if (!ssdk_intf_search(if_mac_entry, 8, &intf_entry, &index)) {
							if (index != 0xffffffff) {
								#ifdef IN_IP
								rv = fal_ip_intf_entry_add(0, &intf_entry);
								#endif
								if (SW_OK == rv) {
									if_mac_entry[index] = intf_entry;
								}
							}
						}
					}
					else {
						if (ssdk_intf_search(if_mac_entry, 8, &intf_entry, &index)) {
							intf_entry.entry_id = if_mac_entry[index].entry_id;
							#ifdef IN_IP
							fal_ip_intf_entry_del(0, 1, &intf_entry);
							#endif
							memset(&if_mac_entry[index], 0, sizeof(fal_intf_mac_entry_t));
						}
					}
				}
			} else {
				if (strcmp(dev->name, "eth0")) {
					intf_entry.vid_low = tmp_vid;
					intf_entry.vid_high = intf_entry.vid_low;
					if (op) {
						if (!ssdk_intf_search(if_mac_entry, 8, &intf_entry, &index)) {
							if (index != 0xffffffff) {
								#ifdef IN_IP
								rv = fal_ip_intf_entry_add(0, &intf_entry);
								#endif
								if (SW_OK == rv) {
									if_mac_entry[index] = intf_entry;
								}
							}
						}
					}
					else {
						if (ssdk_intf_search(if_mac_entry, 8, &intf_entry, &index)) {
							intf_entry.entry_id = if_mac_entry[index].entry_id;
							#ifdef IN_IP
							fal_ip_intf_entry_del(0, 1, &intf_entry);
							#endif
							memset(&if_mac_entry[index], 0, sizeof(fal_intf_mac_entry_t));
						}
					}
				}
			}
		}
	}

}

static int ssdk_inet_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;

	if (!strstr(dev->name, "eth") && !strstr(dev->name, "br")) {
		return NOTIFY_DONE;
	}
	switch (event) {
		case NETDEV_DOWN:
			ssdk_intf_set(dev, 0);
			break;
		case NETDEV_UP:
			ssdk_intf_set(dev, 1);
			break;
	}
	return NOTIFY_DONE;
}
#endif

/* Trunk sendout workaround */
int qca_trunk_sendout_issue_workaround (char *pkt, int len, int port_id)
{
	sw_error_t rv = 0;

	rv = fal_trunk_manipulate_sa_set(0, pkt + 6);
	if(rv) {
		printk("manipulate sa set failed, rv:%d\n", rv);
		return -1;
	}

	rv = fal_trunk_manipulate_dp(0, pkt, len, 1<<port_id);
	if(rv) {
		printk("manipulate dp failed, rv:%d\n", rv);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(qca_trunk_sendout_issue_workaround);

static int __init regi_init(void)
{
	ssdk_init_cfg cfg;
	int rv = 0;
	garuda_init_spec_cfg chip_spec_cfg;
	ssdk_dt_global.switch_reg_access_mode = HSL_REG_MDIO;
	ssdk_dt_global.psgmii_reg_access_mode = HSL_REG_MDIO;

	ssdk_cfg_default_init(&cfg);

	#if 0 //defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
	ssdk_dt_parse(&cfg);
	#endif

	rv = ssdk_plat_init(&cfg);
	if(rv)
		goto out;

	rv = chip_ver_get(&cfg);
	if(rv)
		goto out;

	ssdk_phy_id_get(&cfg);

	memset(&chip_spec_cfg, 0, sizeof(garuda_init_spec_cfg));
	cfg.chip_spec_cfg = &chip_spec_cfg;

	rv = ssdk_init(0, &cfg);
	if(rv)
		goto out;
#if 1
	printk("ssdk init for 1:%x!\n", qca_ar8216_mii_slave_read(0));
	rv = ssdk_init(1, &cfg);
	if(rv)
		goto out;
	printk("ssdk init for slave !\n");
#endif

	qca_switch_register_init();

	//LXX
	printk("[lxx] qca phy switch reg  enter\n");
	//if((miibus)||(miibus_slave))
	lan_led_ctrl_init();
	dni_switch_phy_init();
	set_lan_led_func(LED_OPTION_ON);
//	set_lan_led = set_lan_led_func;
	blink_wan_led_init();
	mirror_wan_init();

	#ifdef DESS
	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_LOCAL_BUS) {
		/*Do Malibu self test to fix packet drop issue firstly*/
		if ((cfg.chip_type == CHIP_DESS) && (ssdk_dt_global.mac_mode == PORT_WRAPPER_PSGMII)) {
			ssdk_psgmii_self_test();
			clear_self_test_config();
		}

		rv = ssdk_switch_register();
		qca_dess_hw_init(&cfg);

		#if defined (CONFIG_NF_FLOW_COOKIE)
		#ifdef IN_NAT
		sfe_register_flow_cookie_cb(ssdk_flow_cookie_set);
		#endif
		#endif

		#ifdef IN_RFS
		memset(&rfs_dev, 0, sizeof(rfs_dev));
		rfs_dev.name = NULL;
		#ifdef IN_FDB
		rfs_dev.mac_rule_cb = ssdk_rfs_mac_rule_set;
		#endif
		#ifdef IN_IP
		rfs_dev.ip4_rule_cb = ssdk_rfs_ip4_rule_set;
		rfs_dev.ip6_rule_cb = ssdk_rfs_ip6_rule_set;
		#endif
		rfs_ess_device_register(&rfs_dev);
		#if defined(CONFIG_RFS_ACCEL)
		ssdk_dev_notifier.notifier_call = ssdk_dev_event;
		ssdk_dev_notifier.priority = 1;
		register_netdevice_notifier(&ssdk_dev_notifier);
		#endif
		ssdk_inet_notifier.notifier_call = ssdk_inet_event;
		ssdk_inet_notifier.priority = 1;
		register_inetaddr_notifier(&ssdk_inet_notifier);
		#endif

		/* Setup Cpu port for Dakota platform. */
		switch_cpuport_setup();
	}
	#endif

out:
	if (rv == 0)
		printk("qca-ssdk module init succeeded!\n");
	else {
		if (rv == -ENODEV) {
			rv = 0;
			printk("qca-ssdk module init, no device found!\n");
		} else
			printk("qca-ssdk module init failed! (code: %d)\n", rv);
	}

	return rv;
}

static void __exit
regi_exit(void)
{
	sw_error_t rv=ssdk_cleanup();

	blink_wan_led_cleanup();//lxx
	dni_switch_phy_cleanup();
	lan_led_ctrl_cleanup();
	mirror_wan_cleanup();
//	set_lan_led = NULL;
	
	if (rv == 0)
		printk("qca-ssdk module exit  done!\n");
	else
		printk("qca-ssdk module exit failed! (code: %d)\n", rv);

	if(ssdk_dt_global.switch_reg_access_mode == HSL_REG_LOCAL_BUS){
		#ifdef DESS
		ssdk_switch_unregister();
		#if defined (CONFIG_NF_FLOW_COOKIE)
		#ifdef IN_NAT
		sfe_unregister_flow_cookie_cb(ssdk_flow_cookie_set);
		#endif
		#endif
		#ifdef IN_RFS
		rfs_ess_device_unregister(&rfs_dev);
		unregister_inetaddr_notifier(&ssdk_inet_notifier);
		#if defined(CONFIG_RFS_ACCEL)
		unregister_netdevice_notifier(&ssdk_dev_notifier);
		#endif
		#endif
		#endif

	}

	qca_switch_unregister();
	ssdk_plat_exit();

	ssdk_dev_id_procfs_exit();
}

module_init(regi_init);
module_exit(regi_exit);

MODULE_DESCRIPTION("QCA SSDK Driver");
MODULE_AUTHOR("Qualcomm Atheros Inc");
MODULE_LICENSE("Dual BSD/GPL");

