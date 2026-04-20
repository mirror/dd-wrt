/*
 * Copyright (c) 2012, 2014-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*qca808x_start*/
#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_phy.h"
/*qca808x_end*/
#include "ssdk_dts.h"
#include "ssdk_interrupt.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
/*qca808x_start*/
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
/*qca808x_end*/
//#include <asm/mach-types.h>
#include <generated/autoconf.h>
#include <linux/if_arp.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
/*qca808x_start*/
#include <linux/phy.h>
#include <linux/mdio.h>
/*qca808x_end*/
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#if defined(IN_SWCONFIG)
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#else
#include <net/switch.h>
#endif
#endif

#if defined(ISIS) ||defined(ISISC) ||defined(GARUDA)
#include <f1_phy.h>
#endif
#if defined(ATHENA) ||defined(SHIVA) ||defined(HORUS)
#include <f2_phy.h>
#endif
#ifdef IN_MALIBU_PHY
#include <malibu_phy.h>
#endif
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
/*qca808x_start*/
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/reset.h>
/*qca808x_end*/
#elif defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/reset.h>
#else
#include <linux/ar8216_platform.h>
#include <drivers/net/phy/ar8216.h>
#include <drivers/net/ethernet/atheros/ag71xx/ag71xx.h>
#endif
/*qca808x_start*/
#include "ssdk_plat.h"
/*qca808x_end*/
#include "ssdk_clk.h"
#include "ssdk_led.h"
#include "ref_vlan.h"
#include "ref_fdb.h"
#include "ref_mib.h"
#include "ref_port_ctrl.h"
#include "ref_misc.h"
#include "ref_uci.h"
#include "ref_vsi.h"
#include "shell.h"
/*qca808x_start*/
#if defined(IN_PHY_I2C_MODE)
#include "ssdk_phy_i2c.h"
#endif
/*qca808x_end*/
#ifdef IN_IP
#if defined (CONFIG_NF_FLOW_COOKIE)
#include "fal_flowcookie.h"
#ifdef IN_SFE
#include <shortcut-fe/sfe.h>
#endif
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
#include "adpt.h"
#ifdef HPPE
#include "ssdk_hppe.h"
#include "adpt_hppe.h"
#endif
#ifdef APPE
#include "ssdk_appe.h"
#endif
#ifdef SCOMPHY
#include "ssdk_scomphy.h"
#endif
#ifdef IN_NETLINK
#include "ssdk_netlink.h"
#endif

#if defined(MHT)
#include "ssdk_mht.h"
#include "ssdk_mht_clk.h"
#endif
#ifdef IN_LED
#include "ssdk_led.h"
#endif

#ifdef IN_RFS
struct rfs_device rfs_dev;
struct notifier_block ssdk_inet_notifier;
ssdk_rfs_intf_t rfs_intf_tbl[SSDK_RFS_INTF_MAX] = {{0}};
#endif

//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
struct notifier_block ssdk_dev_notifier;
//#endif


extern a_uint32_t hsl_dev_wan_port_get(a_uint32_t dev_id);
extern void dess_rgmii_sw_mac_polling_task(struct qca_phy_priv *priv);
extern void qca_ar8327_sw_mac_polling_task(struct qca_phy_priv *priv);
extern void qca_ar8327_sw_mib_task(struct qca_phy_priv *priv);

//#define PSGMII_DEBUG

#define QCA_QM_WORK_DELAY	100
#define QCA_QM_ITEM_NUMBER 41
#define QCA_RGMII_WORK_DELAY	1000
#define QCA_MAC_SW_SYNC_WORK_DELAY	1000
#define QCA_FDB_SW_SYNC_WORK_DELAY	1000
/*qca808x_start*/
struct qca_phy_priv **qca_phy_priv_global;

struct qca_phy_priv* ssdk_phy_priv_data_get(a_uint32_t dev_id)
{
	if (dev_id >= SW_MAX_NR_DEV || !qca_phy_priv_global)
		return NULL;

	return qca_phy_priv_global[dev_id];
}
/*qca808x_end*/

a_uint32_t hppe_port_type[6] = {0,0,0,0,0,0}; // this variable should be init by ssdk_init

a_uint32_t
qca_hppe_port_mac_type_get(a_uint32_t dev_id, a_uint32_t port_id)
{
	if ((port_id < 1) || (port_id > 6))
		return 0;
	return hppe_port_type[port_id - 1];
}

a_uint32_t
qca_hppe_port_mac_type_set(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t port_type)
{
	 if ((port_id < 1) || (port_id > 6))
		 return 0;
	hppe_port_type[port_id - 1] = port_type;

	return 0;
}

a_uint32_t
ssdk_ifname_to_port(a_uint32_t dev_id, const char *ifname)
{
	struct net_device *eth_dev = NULL;
	eth_dev = dev_get_by_name(&init_net, ifname);
	if (!eth_dev || !eth_dev->phydev)
	{
		return 0;
	}
	dev_put(eth_dev);
	return qca_ssdk_phydev_to_port(dev_id, eth_dev->phydev);
}

char *
ssdk_port_to_ifname(a_uint32_t dev_id, a_uint32_t port_id)
{
	struct phy_device *phydev = NULL;

	hsl_port_phydev_get(dev_id, port_id, &phydev);
	if (phydev && phydev->attached_dev)
	{
		return phydev->attached_dev->name;
	}
	else
	{
		return NULL;
	}
}

#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
static void
ssdk_phy_rgmii_set(struct qca_phy_priv *priv)
{
	struct device_node *np = NULL;
	u32 rgmii_en = 0, tx_delay = 0, rx_delay = 0;

	if (priv->ess_switch_flag == A_TRUE)
		np = priv->of_node;
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
		np = priv->phy->mdio.dev.of_node;
#else
		np = priv->phy->dev.of_node;
#endif

	if (!np)
		return;

	if (!of_property_read_u32(np, "phy_rgmii_en", &rgmii_en)) {
		/*enable RGMII  mode */
		hsl_phy_modify_debug(0, AR8327_PORT5_PHY_ADDR,
			AR8327_PHY_REG_MODE_SEL, AR8327_PHY_RGMII_MODE,
			AR8327_PHY_RGMII_MODE);
		if (!of_property_read_u32(np, "txclk_delay_en", &tx_delay)
				&& tx_delay == 1) {
			hsl_phy_modify_debug(0, AR8327_PORT5_PHY_ADDR,
				AR8327_PHY_REG_SYS_CTRL, AR8327_PHY_RGMII_TX_DELAY,
				AR8327_PHY_RGMII_TX_DELAY);
		}
		if (!of_property_read_u32(np, "rxclk_delay_en", &rx_delay)
				&& rx_delay == 1) {
			hsl_phy_modify_debug(0, AR8327_PORT5_PHY_ADDR,
				AR8327_PHY_REG_TEST_CTRL, AR8327_PHY_RGMII_RX_DELAY,
				AR8327_PHY_RGMII_RX_DELAY);
		}
	}
}
#else
static void
ssdk_phy_rgmii_set(struct qca_phy_priv *priv)
{
	struct ar8327_platform_data *plat_data;

	plat_data = priv->phy->dev.platform_data;
	if (plat_data == NULL) {
		return;
	}

	if(plat_data->pad5_cfg) {
		if(plat_data->pad5_cfg->mode == AR8327_PAD_PHY_RGMII) {
			a_uint16_t val = 0;
			/*enable RGMII  mode */
			hsl_phy_modify_debug(0, AR8327_PORT5_PHY_ADDR,
				AR8327_PHY_REG_MODE_SEL, AR8327_PHY_RGMII_MODE,
				AR8327_PHY_RGMII_MODE);
			if(plat_data->pad5_cfg->txclk_delay_en) {
				hsl_phy_modify_debug(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_SYS_CTRL, AR8327_PHY_RGMII_TX_DELAY,
					AR8327_PHY_RGMII_TX_DELAY);
			}
			if(plat_data->pad5_cfg->rxclk_delay_en) {
				hsl_phy_modify_debug(0, AR8327_PORT5_PHY_ADDR,
					AR8327_PHY_REG_TEST_CTRL, AR8327_PHY_RGMII_RX_DELAY,
					AR8327_PHY_RGMII_RX_DELAY);
			}
		}
	}
}
#endif


static void
qca_ar8327_phy_fixup(struct qca_phy_priv *priv, int phy)
{
	switch (priv->revision) {
	case 1:
		/* 100m waveform */
		hsl_phy_debug_reg_write(priv->device_id, phy, 0, 0x02ea);
		/* turn on giga clock */
		hsl_phy_debug_reg_write(priv->device_id, phy, 0x3d, 0x68a0);
		break;

	case 2:
		hsl_phy_mmd_reg_write(priv->device_id, phy, A_FALSE,
			0x7, 0x3c, 0);
#if defined(FALLTHROUGH)
		fallthrough;
#else
		/* fall through */
#endif
	case 4:
		if(priv->version == QCA_VER_AR8327)
		{
			hsl_phy_mmd_reg_write(priv->device_id, phy, A_FALSE,
				0x3, 0x800d, 0x803f);
			hsl_phy_debug_reg_write(priv->device_id, phy, 0x3d, 0x6860);
			hsl_phy_debug_reg_write(priv->device_id, phy, 0x5, 0x2c46);
			hsl_phy_debug_reg_write(priv->device_id, phy, 0x3c, 0x6000);
		}
		break;
	}
}

#ifdef IN_PORTVLAN
static void qca_port_isolate(a_uint32_t dev_id)
{
	a_uint32_t port_id, mem_port_id, mem_port_map[AR8327_NUM_PORTS]={0};

	for(port_id = 0; port_id < AR8327_NUM_PORTS; port_id++)
	{
		if(port_id == 6)
			for(mem_port_id = 1; mem_port_id<= 4; mem_port_id++)
				mem_port_map[port_id]  |= (1 << mem_port_id);
		else if (port_id == 0)
			mem_port_map[port_id]  |= (1 << 5);
		else if (port_id >= 1 && port_id <= 4)
			mem_port_map[port_id]  |= (1 << 6);
		else
			mem_port_map[port_id]  |= 1;
	}

	for(port_id = 0; port_id < AR8327_NUM_PORTS; port_id++)

		 fal_portvlan_member_update(dev_id, port_id, mem_port_map[port_id]);

}

void ssdk_portvlan_init(a_uint32_t dev_id)
{
	a_uint32_t port = 0;
	a_uint32_t cpu_bmp, lan_bmp, wan_bmp;

	cpu_bmp = ssdk_cpu_bmp_get(dev_id);
	lan_bmp = ssdk_lan_bmp_get(dev_id);
	wan_bmp = ssdk_wan_bmp_get(dev_id);

	if (!(cpu_bmp | lan_bmp | wan_bmp)) {
		qca_port_isolate(dev_id);
		return;
	}

	for(port = 0; port < SSDK_MAX_PORT_NUM; port++)
	{
		if(cpu_bmp & (1 << port))
		{
			fal_portvlan_member_update(dev_id, port, lan_bmp|wan_bmp);
		}
		if(lan_bmp & (1 << port))
		{
			fal_portvlan_member_update(dev_id, port, (lan_bmp|cpu_bmp)&(~(1<<port)));
		}
		if(wan_bmp & (1 << port))
		{
			fal_portvlan_member_update(dev_id, port, (wan_bmp|cpu_bmp)&(~(1<<port)));
		}
	}
}
#endif

sw_error_t
qca_switch_init(a_uint32_t dev_id)
{
#if (defined(DESS) || defined(ISISC) || defined(ISIS)) && defined(IN_QOS)
	a_uint32_t nr = 0, react_nr = 0;
#endif
#if defined(MHT)
	a_uint32_t port_hol_ctrl[2] = {0}, queue_hol_ctrl[6] = {0};
	a_uint32_t j = 0, cpu_bmp = 0;
#endif
	int i = 0;
	a_uint32_t port_bmp = 0;
	hsl_reg_mode reg_mode = HSL_REG_MDIO;
	a_bool_t flag = A_FALSE;
	fal_port_eee_cfg_t port_eee_cfg = {0};
	ssdk_chip_type chip_type = hsl_get_current_chip_type(dev_id);

	/*fal_reset(dev_id);*/
	/*enable cpu and disable mirror*/
#ifdef IN_MISC
	fal_cpu_port_status_set(dev_id, A_TRUE);
	/* setup MTU */
	fal_frame_max_size_set(dev_id, 9216);
#endif
#ifdef IN_MIB
	/* Enable MIB counters */
	fal_mib_status_set(dev_id, A_TRUE);
	fal_mib_cpukeep_set(dev_id, A_FALSE);
#endif
	/*set mirror analysis port as 0xf in default*/
#ifdef IN_MIRROR
	fal_mirr_analysis_port_set(dev_id, 0xf);
#endif
#ifdef IN_IGMP
	fal_igmp_mld_rp_set(dev_id, 0);
#endif
	reg_mode = ssdk_switch_reg_access_mode_get(dev_id);
	flag = ssdk_ess_switch_flag_get(dev_id);

	if (reg_mode == HSL_REG_MDIO && flag == A_FALSE) {
		/* For legacy S17C without defining port bmp in dts */
		port_bmp = 0x7f;
	} else {
		port_bmp = qca_ssdk_port_bmp_get(dev_id);
		/* Including CPU port */
		port_bmp |= ssdk_cpu_bmp_get(dev_id);
	}

	i = 0;
	while (port_bmp) {
		if (port_bmp & 1) {
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
			fal_fdb_port_learn_set(dev_id, i, A_FALSE);
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
#ifdef IN_PORTCONTROL
			fal_port_interface_eee_cfg_get(dev_id, i, &port_eee_cfg);
			port_eee_cfg.enable = A_FALSE;
			port_eee_cfg.lpi_tx_enable = A_FALSE;
			fal_port_interface_eee_cfg_set(dev_id, i, &port_eee_cfg);
#endif

			switch (chip_type) {
				case CHIP_SHIVA:
					return SW_OK;
				case CHIP_ISISC:
				case CHIP_ISIS:
#if defined(ISISC) || defined(ISIS)
#ifdef IN_PORTCONTROL
					fal_port_flowctrl_forcemode_set(dev_id, i, A_TRUE);
					fal_port_flowctrl_set(dev_id, i, A_FALSE);

					if (i != 0 && i != 6) {
						fal_port_flowctrl_forcemode_set(dev_id, i,
								A_FALSE);
					}
#endif
					if (i == 0 || i == 5 || i == 6) {
#ifdef IN_QOS
						nr = 240; /*30*8*/
						fal_qos_port_tx_buf_nr_set(dev_id, i, &nr);
						fal_qos_port_rx_buf_nr_get(dev_id, i, &nr,
							&react_nr);
						nr = 48; /*6*8*/
						fal_qos_port_rx_buf_nr_set(dev_id, i, &nr,
							&react_nr);
						fal_qos_port_red_en_set(dev_id, i, A_TRUE);
						if (chip_type == CHIP_ISISC) {
							nr = 64; /*8*8*/
						} else if (chip_type == CHIP_ISIS) {
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
						fal_qos_port_rx_buf_nr_get(dev_id, i, &nr,
							&react_nr);
						nr = 48; /*6*8*/
						fal_qos_port_rx_buf_nr_set(dev_id, i, &nr,
							&react_nr);
						fal_qos_port_red_en_set(dev_id, i, A_TRUE);
						if (chip_type == CHIP_ISISC) {
							nr = 64; /*8*8*/
						} else if (chip_type == CHIP_ISIS) {
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
					break;
				case CHIP_MHT:
#if defined(MHT)
					aos_mem_zero(&port_hol_ctrl, sizeof(port_hol_ctrl));
					aos_mem_zero(&queue_hol_ctrl, sizeof(queue_hol_ctrl));
					cpu_bmp = ssdk_cpu_bmp_get(dev_id);
#if defined(IN_PORTCONTROL)
					fal_port_txmac_status_set(dev_id, i, A_FALSE);
					fal_port_rxmac_status_set(dev_id, i, A_FALSE);
					fal_port_flowctrl_set(dev_id, i, A_TRUE);
#endif
					if (cpu_bmp & BIT(i)) {
#if defined(IN_PORTCONTROL)
						fal_port_flowctrl_forcemode_set(dev_id, i, A_TRUE);
						fal_port_flow_ctrl_thres_set(dev_id, i,
								MHT_PORT0_XON_THRES, MHT_PORT0_XOFF_THRES);
						fal_header_type_set(dev_id,
								A_TRUE, MHT_HEADER_TYPE_VAL);
						fal_port_rxhdr_mode_set(dev_id,
								i, FAL_ONLY_MANAGE_FRAME_EN);
						fal_port_txhdr_mode_set(dev_id,
								i, FAL_NO_HEADER_EN);
#endif
						/* port tx buf number */
						port_hol_ctrl[0] = 600;
						/* port rx buf number */
						port_hol_ctrl[1] = 48;
						/* queue0 tx buf number */
						queue_hol_ctrl[0] = 24;
						/* queue1 tx buf number */
						queue_hol_ctrl[1] = 32;
						/* queue2 tx buf number */
						queue_hol_ctrl[2] = 32;
						/* queue3 tx buf number */
						queue_hol_ctrl[3] = 32;
						/* queue4 tx buf number */
						queue_hol_ctrl[4] = 48;
						/* queue5 tx buf number */
						queue_hol_ctrl[5] = 64;
					} else {
#if defined(IN_PORTCONTROL)
						fal_port_flowctrl_forcemode_set(dev_id, i,
								A_FALSE);
#endif
						/* port tx buf number */
						port_hol_ctrl[0] = 500;
						/* port rx buf number */
						port_hol_ctrl[1] = 48;
						/* queue0 tx buf number */
						queue_hol_ctrl[0] = 24;
						/* queue1 tx buf number */
						queue_hol_ctrl[1] = 32;
						/* queue2 tx buf number */
						queue_hol_ctrl[2] = 48;
						/* queue3 tx buf number */
						queue_hol_ctrl[3] = 64;
					}
#if defined(IN_QOS)
					fal_qos_port_red_en_set(dev_id, i, A_TRUE);
					fal_qos_port_tx_buf_nr_set(dev_id, i, &port_hol_ctrl[0]);
					fal_qos_port_rx_buf_nr_get(dev_id, i, &nr, &react_nr);
					nr = port_hol_ctrl[1];
					react_nr = 60;
					fal_qos_port_rx_buf_nr_set(dev_id, i, &nr, &react_nr);

					for (j = 0; j < ARRAY_SIZE(queue_hol_ctrl)
							&& queue_hol_ctrl[j] != 0; j++)
						fal_qos_queue_tx_buf_nr_set(dev_id, i, j,
								&queue_hol_ctrl[j]);
#endif
#endif
					break;
				default:
					break;
			}
		}
		port_bmp >>=1;
		i++;
	}

	return SW_OK;
}

void qca_ar8327_phy_linkdown(a_uint32_t dev_id)
{
	int i;

	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		hsl_phy_mii_reg_write(dev_id, i, 0x0, 0x0800);	// phy powerdown

		hsl_phy_modify_debug(dev_id, i, 0x3d, 0x0040, 0);
		/*PHY will stop the tx clock for a while when link is down
			1. en_anychange  debug port 0xb bit13 = 0  //speed up link down tx_clk
			2. sel_rst_80us  debug port 0xb bit10 = 0  //speed up speed mode change to 2'b10 tx_clk
		*/
		hsl_phy_modify_debug(dev_id, i, 0xb, 0x2400, 0);
	}
}

void
qca_mac_disable(a_uint32_t device_id)
{
	hsl_api_t *p_api;

	p_api = hsl_api_ptr_get (device_id);
	if(p_api
		&& p_api->interface_mac_pad_set
		&& p_api->interface_mac_sgmii_set)
	{
		p_api->interface_mac_pad_set(device_id,0,0);
		p_api->interface_mac_pad_set(device_id,5,0);
		p_api->interface_mac_pad_set(device_id,6,0);
		p_api->interface_mac_sgmii_set(device_id,AR8327_REG_PAD_SGMII_CTRL_HW_INIT);
	}
	else
	{
		SSDK_ERROR("API not support \n");
	}
}

static void qca_switch_set_mac_force(struct qca_phy_priv *priv)
{
	a_uint32_t value, value0, i;
	if (priv == NULL || (priv->mii_read == NULL) || (priv->mii_write == NULL)) {
		SSDK_ERROR("In qca_switch_set_mac_force, private data is NULL!\r\n");
		return;
	}

	for (i=0; i < AR8327_NUM_PORTS; ++i) {
		/* b3:2=0,Tx/Rx Mac disable,
		 b9=0,LINK_EN disable */
		value0 = priv->mii_read(priv->device_id, AR8327_REG_PORT_STATUS(i));
		value = value0 & ~(AR8327_PORT_STATUS_LINK_AUTO |
						AR8327_PORT_STATUS_TXMAC |
						AR8327_PORT_STATUS_RXMAC);
		priv->mii_write(priv->device_id, AR8327_REG_PORT_STATUS(i), value);

		/* Force speed to 1000M Full */
		value = priv->mii_read(priv->device_id, AR8327_REG_PORT_STATUS(i));
		value &= ~(AR8327_PORT_STATUS_DUPLEX | AR8327_PORT_STATUS_SPEED);
		value |= AR8327_PORT_SPEED_1000M | AR8327_PORT_STATUS_DUPLEX;
		priv->mii_write(priv->device_id, AR8327_REG_PORT_STATUS(i), value);
	}
	return;
}

void
qca_ar8327_phy_enable(struct qca_phy_priv *priv)
{
	int i = 0;

        ssdk_phy_rgmii_set(priv);
	for (i = 0; i < AR8327_NUM_PHYS; i++) {
		if (priv->version == QCA_VER_AR8327 || priv->version == QCA_VER_AR8337)
			qca_ar8327_phy_fixup(priv, i);

		/* start autoneg*/
		hsl_phy_mii_reg_write(priv->device_id, i, MII_ADVERTISE, ADVERTISE_ALL |
						     ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		//phy reg 0x9, b10,1 = Prefer multi-port device (master)
		hsl_phy_mii_reg_write(priv->device_id, i, MII_CTRL1000, (0x0400|ADVERTISE_1000FULL));

		hsl_phy_mii_reg_write(priv->device_id, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);

		hsl_phy_modify_debug(priv->device_id, i, 0, BIT(12), 0);
		msleep(100);
	}
}
void qca_ar8327_sw_soft_reset(struct qca_phy_priv *priv)
{
	a_uint32_t value = 0;

	value = priv->mii_read(priv->device_id, AR8327_REG_CTRL);
	value |= 0x80000000;
	priv->mii_write(priv->device_id, AR8327_REG_CTRL, value);
	/*Need wait reset done*/
	do {
		udelay(10);
		value = priv->mii_read(priv->device_id, AR8327_REG_CTRL);
	} while(value & AR8327_CTRL_RESET);
	do {
		udelay(10);
		value = priv->mii_read(priv->device_id, 0x20);
	} while ((value & SSDK_GLOBAL_INITIALIZED_STATUS) !=
			SSDK_GLOBAL_INITIALIZED_STATUS);

	return;
}

#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
int qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	struct device_node *np = NULL;
	const __be32 *paddr;
	a_uint32_t reg, value, i;
	a_int32_t len;

	if (priv->ess_switch_flag == A_TRUE)
		np = priv->of_node;
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
		np = priv->phy->mdio.dev.of_node;
#else
		np = priv->phy->dev.of_node;
#endif

	if(!np)
		return -EINVAL;

	/*Before switch software reset, disable PHY and clear  MAC PAD*/
	qca_ar8327_phy_linkdown(priv->device_id);
	qca_mac_disable(priv->device_id);
	msleep(1000);

	/*First software reset S17 chip*/
	qca_ar8327_sw_soft_reset(priv);

	/*After switch software reset, need disable all ports' MAC with 1000M FULL*/
	qca_switch_set_mac_force(priv);

	/* Configure switch register from DT information */
	paddr = of_get_property(np, "qca,ar8327-initvals", &len);
	if (paddr) {
		if (len < (2 * sizeof(*paddr))) {
			SSDK_ERROR("len:%d < 2 * sizeof(*paddr):%zu\n", len, 2 * sizeof(*paddr));
			return -EINVAL;
		}

		len /= sizeof(*paddr);

		for (i = 0; i < len - 1; i += 2) {
			reg = be32_to_cpup(paddr + i);
			value = be32_to_cpup(paddr + i + 1);
			priv->mii_write(priv->device_id, reg, value);
		}
	}

	value = priv->mii_read(priv->device_id, AR8327_REG_MODULE_EN);
	value &= ~AR8327_REG_MODULE_EN_QM_ERR;
	value &= ~AR8327_REG_MODULE_EN_LOOKUP_ERR;
	priv->mii_write(priv->device_id, AR8327_REG_MODULE_EN, value);

	qca_switch_init(priv->device_id);
#ifdef IN_PORTVLAN
	ssdk_portvlan_init(priv->device_id);
#endif
	qca_switch_enable_intr(priv, FAL_SWITCH_INTR_LINK_STATUS);
	qca_ar8327_phy_enable(priv);

	return 0;
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

static a_uint32_t
qca_ar8327_get_pwr_sel(struct qca_phy_priv *priv,
                                struct ar8327_platform_data *plat_data)
{
	struct ar8327_pad_cfg *cfg = NULL;
	a_uint32_t value;

	if (!plat_data) {
		return 0;
	}

	value = priv->mii_read(priv->device_id, AR8327_REG_PAD_MAC_PWR_SEL);

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

		priv->mii_write(priv->device_id, AR8327_REG_LED_CTRL_0, led_cfg->led_ctrl0);
		priv->mii_write(priv->device_id, AR8327_REG_LED_CTRL_1, led_cfg->led_ctrl1);
		priv->mii_write(priv->device_id, AR8327_REG_LED_CTRL_2, led_cfg->led_ctrl2);
		priv->mii_write(priv->device_id, AR8327_REG_LED_CTRL_3, led_cfg->led_ctrl3);

		if (new_pos != pos) {
			new_pos |= AR8327_POS_POWER_ON_SEL;
		}
	}
	return new_pos;
}

static int
qca_ar8327_set_sgmii_cfg(struct qca_phy_priv *priv,
                              struct ar8327_platform_data *plat_data,
                              a_uint32_t* new_pos)
{
	a_uint32_t value = 0;

	/*configure the SGMII*/
	value = priv->mii_read(priv->device_id, AR8327_REG_PAD_SGMII_CTRL);
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

	priv->mii_write(priv->device_id, AR8327_REG_PAD_SGMII_CTRL, value);

	if (plat_data->sgmii_cfg->serdes_aen) {
		*new_pos &= ~AR8327_POS_SERDES_AEN;
	} else {
		*new_pos |= AR8327_POS_SERDES_AEN;
	}
	return 0;
}

static int
qca_ar8327_set_plat_data_cfg(struct qca_phy_priv *priv,
                              struct ar8327_platform_data *plat_data)
{
	a_uint32_t pos, new_pos;

	pos = priv->mii_read(priv->device_id, AR8327_REG_POS);

	new_pos = qca_ar8327_set_led_cfg(priv, plat_data, pos);

	/*configure the SGMII*/
	if (plat_data->sgmii_cfg) {
		qca_ar8327_set_sgmii_cfg(priv, plat_data, &new_pos);
	}

	priv->mii_write(priv->device_id, AR8327_REG_POS, new_pos);

	return 0;
}

static int
qca_ar8327_set_pad_cfg(struct qca_phy_priv *priv,
                              struct ar8327_platform_data *plat_data)
{
	a_uint32_t pad0 = 0, pad5 = 0, pad6 = 0;

	pad0 = qca_ar8327_get_pad_cfg(plat_data->pad0_cfg);
	priv->mii_write(priv->device_id, AR8327_REG_PAD0_CTRL, pad0);

	pad5 = qca_ar8327_get_pad_cfg(plat_data->pad5_cfg);
	if(priv->version == QCA_VER_AR8337) {
	        pad5 |= AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN;
	}
	priv->mii_write(priv->device_id, AR8327_REG_PAD5_CTRL, pad5);

	pad6 = qca_ar8327_get_pad_cfg(plat_data->pad6_cfg);
	if(plat_data->pad5_cfg &&
		(plat_data->pad5_cfg->mode == AR8327_PAD_PHY_RGMII))
		pad6 |= AR8327_PAD_CTRL_PHYX_RGMII_EN;
	priv->mii_write(priv->device_id, AR8327_REG_PAD6_CTRL, pad6);

	return 0;
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

	/*disable mac at first*/
	fal_port_rxmac_status_set(priv->device_id, port, A_FALSE);
	fal_port_txmac_status_set(priv->device_id, port, A_FALSE);
	value = port_cfg->duplex ? FAL_FULL_DUPLEX : FAL_HALF_DUPLEX;
	fal_port_duplex_set(priv->device_id, port, value);
	value = port_cfg->txpause ? A_TRUE : A_FALSE;
	fal_port_txfc_status_set(priv->device_id, port, value);
	value = port_cfg->rxpause ? A_TRUE : A_FALSE;
	fal_port_rxfc_status_set(priv->device_id, port, value);
	if(port_cfg->speed == AR8327_PORT_SPEED_10) {
		value = FAL_SPEED_10;
	} else if(port_cfg->speed == AR8327_PORT_SPEED_100) {
		value = FAL_SPEED_100;
	} else if(port_cfg->speed == AR8327_PORT_SPEED_1000) {
		value = FAL_SPEED_1000;
	} else {
		value = FAL_SPEED_1000;
	}
	fal_port_speed_set(priv->device_id, port, value);
	/*enable mac at last*/
	udelay(800);
	fal_port_rxmac_status_set(priv->device_id, port, A_TRUE);
	fal_port_txmac_status_set(priv->device_id, port, A_TRUE);
}

int
qca_ar8327_hw_init(struct qca_phy_priv *priv)
{
	struct ar8327_platform_data *plat_data;
	a_uint32_t i = 0;
	a_uint32_t value = 0;

	plat_data = priv->phy->dev.platform_data;
	if (plat_data == NULL) {
		return -EINVAL;
	}

	/*Before switch software reset, disable PHY and clear MAC PAD*/
	qca_ar8327_phy_linkdown(priv->device_id);
	qca_mac_disable(priv->device_id);
	udelay(10);

	qca_ar8327_set_plat_data_cfg(priv, plat_data);

	/*mac reset*/
	priv->mii_write(priv->device_id, AR8327_REG_MAC_SFT_RST, 0x3fff);

	msleep(100);

	/*First software reset S17 chip*/
	qca_ar8327_sw_soft_reset(priv);
	udelay(1000);

	/*After switch software reset, need disable all ports' MAC with 1000M FULL*/
	qca_switch_set_mac_force(priv);

	qca_ar8327_set_pad_cfg(priv, plat_data);

	value = priv->mii_read(priv->device_id, AR8327_REG_MODULE_EN);
	value &= ~AR8327_REG_MODULE_EN_QM_ERR;
	value &= ~AR8327_REG_MODULE_EN_LOOKUP_ERR;
	priv->mii_write(priv->device_id, AR8327_REG_MODULE_EN, value);

	qca_switch_init(priv->device_id);

	value = qca_ar8327_get_pwr_sel(priv, plat_data);
	priv->mii_write(priv->device_id, AR8327_REG_PAD_MAC_PWR_SEL, value);

	msleep(1000);

	for (i = 0; i < AR8327_NUM_PORTS; i++) {
		qca_ar8327_port_init(priv, i);
	}

	qca_ar8327_phy_enable(priv);

	return 0;
}
#endif

#if defined(IN_SWCONFIG)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0))
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
#endif

static struct switch_attr qca_ar8327_globals[] = {
#if defined(IN_VLAN)
	{
		.name = "enable_vlan",
		.description = "Enable 8021q VLAN",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_vlan,
		.get = qca_ar8327_sw_get_vlan,
		.max = 1
	},
#endif
#if defined(IN_MISC)
	{
		.name = "max_frame_size",
		.description = "Set Max frame Size Of Mac",
		.type = SWITCH_TYPE_INT,
		.set = qca_ar8327_sw_set_max_frame_size,
		.get = qca_ar8327_sw_get_max_frame_size,
		.max = 9018
	},
#endif
#if defined(IN_MIB)
	{
		.name = "reset_mibs",
		.description = "Reset All MIB Counters",
		.type = SWITCH_TYPE_NOVAL,
		.set = qca_ar8327_sw_set_reset_mibs,
	},
#endif
#ifdef IN_FDB
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
#endif
	{
		.name = "switch_ext",
		.description = "Switch extended configuration",
		.type = SWITCH_TYPE_EXT,
		.set = qca_ar8327_sw_switch_ext,
	},
};

static struct switch_attr qca_ar8327_port[] = {
#if defined(IN_MIB)
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
#endif
#if defined(IN_PORTCONTROL)
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_eee",
		.description = "Enable EEE",
		.set = qca_ar8327_sw_set_eee,
		.get = qca_ar8327_sw_get_eee,
		.max = 1,
	},
#endif
};

#if defined(IN_VLAN)
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
#endif

const struct switch_dev_ops qca_ar8327_sw_ops = {
	.attr_global = {
		.attr = qca_ar8327_globals,
		.n_attr = ARRAY_SIZE(qca_ar8327_globals),
	},
	.attr_port = {
		.attr = qca_ar8327_port,
		.n_attr = ARRAY_SIZE(qca_ar8327_port),
	},
#if defined(IN_VLAN)
	.attr_vlan = {
		.attr = qca_ar8327_vlan,
		.n_attr = ARRAY_SIZE(qca_ar8327_vlan),
	},
	.get_port_pvid = qca_ar8327_sw_get_pvid,
	.set_port_pvid = qca_ar8327_sw_set_pvid,
	.get_vlan_ports = qca_ar8327_sw_get_ports,
	.set_vlan_ports = qca_ar8327_sw_set_ports,
	.apply_config = qca_ar8327_sw_hw_apply,
#endif
#if defined(IN_MISC)
	.reset_switch = qca_ar8327_sw_reset_switch,
#endif
#if defined(IN_PORTCONTROL)
	.get_port_link = qca_ar8327_sw_get_port_link,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0))
	.get_reg_val = qca_ar8327_sw_get_reg_val,
	.set_reg_val = qca_ar8327_sw_set_reg_val,
#endif
};
#endif

#define SSDK_MIB_CHANGE_WQ

static int
qca_phy_mib_task(struct qca_phy_priv *priv)
{
	qca_ar8327_sw_mib_task(priv);
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
	if(SW_OK != fal_mib_counter_alloc(priv->device_id, &priv->mib_counters)){
		SSDK_ERROR("Memory allocation fail\n");
		return -ENOMEM;
	}

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

void
qca_phy_mib_work_pause(struct qca_phy_priv *priv)
{
	if(!priv)
		return;
	cancel_delayed_work_sync(&priv->mib_dwork);
}

int
qca_phy_mib_work_resume(struct qca_phy_priv *priv)
{
#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->mib_dwork,
			               msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->mib_dwork,
					msecs_to_jiffies(QCA_PHY_MIB_WORK_DELAY));
#endif
	return 0;
}

#define SSDK_QM_CHANGE_WQ

static void
qm_err_check_work_task_polling(struct work_struct *work)
{
	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
                                            qm_dwork_polling.work);

	mutex_lock(&priv->qm_lock);

	if(priv->version == QCA_VER_MHT) {
#if defined(MHT)
		qca_mht_sw_mac_polling_task(priv);
#endif
	} else {
		qca_ar8327_sw_mac_polling_task(priv);
	}

	mutex_unlock(&priv->qm_lock);

#ifndef SSDK_QM_CHANGE_WQ
	schedule_delayed_work(&priv->qm_dwork,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->qm_dwork_polling,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
#endif
}

int
qm_err_check_work_start(struct qca_phy_priv *priv)
{
	/*Only valid for S17c chip*/
	if (priv->version != QCA_VER_AR8337 &&
		priv->version != QCA_VER_AR8327 &&
		priv->version != QCA_VER_DESS &&
		priv->version != QCA_VER_MHT)
	{
		return 0;
	}

	mutex_init(&priv->qm_lock);
	INIT_DELAYED_WORK(&priv->qm_dwork_polling, qm_err_check_work_task_polling);
#ifndef SSDK_MIB_CHANGE_WQ
	schedule_delayed_work(&priv->qm_dwork_polling,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
#else
	queue_delayed_work_on(0, system_long_wq, &priv->qm_dwork_polling,
							msecs_to_jiffies(QCA_QM_WORK_DELAY));
#endif

	return 0;
}

void
qm_err_check_work_stop(struct qca_phy_priv *priv)
{
	/*Only valid for S17c chip*/
	if (priv->version != QCA_VER_AR8337 &&
		priv->version != QCA_VER_AR8327 &&
		priv->version != QCA_VER_DESS) return;

		cancel_delayed_work_sync(&priv->qm_dwork_polling);

}

void
qca_mac_port_status_init(a_uint32_t dev_id, a_uint32_t port_id)
{
	if(port_id < SSDK_PHYSICAL_PORT1 || port_id >= SW_MAX_NR_PORT)
	{
		SSDK_ERROR("port %d does not support status init\n", port_id);
		return;
	}
	qca_phy_priv_global[dev_id]->port_old_link[port_id - 1] = 0;
	qca_phy_priv_global[dev_id]->port_old_speed[port_id - 1] = FAL_SPEED_BUTT;
	qca_phy_priv_global[dev_id]->port_old_duplex[port_id - 1] = FAL_DUPLEX_BUTT;
	if(hsl_port_feature_get(dev_id, port_id, PHY_F_FORCE) || hsl_port_feature_get
		(dev_id, port_id, PHY_F_SFP))
	{
		qca_phy_priv_global[dev_id]->port_tx_flowctrl_forcemode[port_id - 1] = A_TRUE;
		qca_phy_priv_global[dev_id]->port_rx_flowctrl_forcemode[port_id - 1] = A_TRUE;
	}

	return;
}

void
qca_mac_sw_sync_work_task(struct work_struct *work)
{
	adpt_api_t *p_adpt_api;

	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
					mac_sw_sync_dwork.work);

	mutex_lock(&priv->mac_sw_sync_lock);

	 if((p_adpt_api = adpt_api_ptr_get(priv->device_id)) != NULL) {
		if (NULL == p_adpt_api->adpt_port_polling_sw_sync_set)
			return;
		p_adpt_api->adpt_port_polling_sw_sync_set(priv);
	}

	mutex_unlock(&priv->mac_sw_sync_lock);

	schedule_delayed_work(&priv->mac_sw_sync_dwork,
					msecs_to_jiffies(QCA_MAC_SW_SYNC_WORK_DELAY));
}

int
qca_mac_sw_sync_work_init(struct qca_phy_priv *priv)
{
	mutex_init(&priv->mac_sw_sync_lock);

	INIT_DELAYED_WORK(&priv->mac_sw_sync_dwork,
			qca_mac_sw_sync_work_task);

	return 0;
}

static sw_error_t
_ssdk_mac_sw_sync_chip_check(struct qca_phy_priv *priv)
{
	sw_error_t rv = SW_OK;

	switch (priv->version) {
		case QCA_VER_HPPE:
		case QCA_VER_SCOMPHY:
		case QCA_VER_APPE:
			break;
		default:
			SSDK_DEBUG("Unsupported chip version %d\n", priv->version);
			rv = SW_NOT_SUPPORTED;
	}

	return rv;
}

sw_error_t
ssdk_mac_sw_sync_work_stop(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);
	SW_RTN_ON_NULL(priv);

	if (ssdk_is_emulation(priv->device_id))
	{
		return SW_NOT_SUPPORTED;
	}
	rv = _ssdk_mac_sw_sync_chip_check(priv);
	SW_RTN_ON_ERROR(rv);

	cancel_delayed_work_sync(&priv->mac_sw_sync_dwork);

	return rv;
}
EXPORT_SYMBOL(ssdk_mac_sw_sync_work_stop);

sw_error_t
ssdk_mac_sw_sync_work_start(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t port_id = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);
	SW_RTN_ON_NULL(priv);

	if (ssdk_is_emulation(priv->device_id))
	{
		for (port_id = SSDK_PHYSICAL_PORT1; port_id < SSDK_PHYSICAL_PORT7; port_id++)
		{ /* enable mac for rumi ports */
			if (SW_IS_PBMP_MEMBER(qca_ssdk_port_bmp_get(dev_id), port_id))
			{
				fal_port_txmac_status_set(dev_id, port_id, A_TRUE);
				fal_port_rxmac_status_set(dev_id, port_id, A_TRUE);
			}
		}
		return SW_NOT_SUPPORTED;
	}

	rv = _ssdk_mac_sw_sync_chip_check(priv);
	SW_RTN_ON_ERROR(rv);

	schedule_delayed_work(&priv->mac_sw_sync_dwork,
			msecs_to_jiffies(QCA_MAC_SW_SYNC_WORK_DELAY));
	return rv;
}
EXPORT_SYMBOL(ssdk_mac_sw_sync_work_start);

void
qca_fdb_sw_sync_work_task(struct work_struct *work)
{
	struct qca_phy_priv *priv = container_of(work, struct qca_phy_priv,
		fdb_sw_sync_dwork.work);

#ifdef IN_FDB
	ref_fdb_sw_sync_task(priv);
#endif
	schedule_delayed_work(&priv->fdb_sw_sync_dwork,
		msecs_to_jiffies(QCA_FDB_SW_SYNC_WORK_DELAY));
}

int
qca_fdb_sw_sync_work_init(struct qca_phy_priv *priv)
{
	if ((priv->version != QCA_VER_HPPE) && (priv->version != QCA_VER_APPE) &&
		(priv->version != QCA_VER_MHT))
	{
		return 0;
	}

	aos_lock_init(&priv->fdb_sw_sync_lock);

	return 0;
}

/**
 * add fdb polling on port_map
 */
int
qca_fdb_sw_sync_work_start(struct qca_phy_priv *priv, fal_pbmp_t port_map)
{
	if ((priv->version != QCA_VER_HPPE) && (priv->version != QCA_VER_APPE) &&
		(priv->version != QCA_VER_MHT))
	{
		return 0;
	}

	aos_lock_bh(&priv->fdb_sw_sync_lock);
	SSDK_DEBUG("fdb_sw_sync_port_map 0x%x\n", priv->fdb_sw_sync_port_map);
	if (priv->fdb_sw_sync_port_map == 0 && priv->fdb_polling_started == A_FALSE)
	{
		INIT_DELAYED_WORK(&priv->fdb_sw_sync_dwork,
						qca_fdb_sw_sync_work_task);
		schedule_delayed_work(&priv->fdb_sw_sync_dwork,
						msecs_to_jiffies(QCA_FDB_SW_SYNC_WORK_DELAY));
		priv->fdb_polling_started = A_TRUE;
	}
	SW_PBMP_OR(priv->fdb_sw_sync_port_map, port_map);
	SSDK_DEBUG("fdb_sw_sync_port_map 0x%x\n", priv->fdb_sw_sync_port_map);
	aos_unlock_bh(&priv->fdb_sw_sync_lock);

	return 0;
}

/**
 * stop fdb polling on port_map
 */
void
qca_fdb_sw_sync_work_stop(struct qca_phy_priv *priv, fal_pbmp_t port_map)
{
	if ((priv->version != QCA_VER_HPPE) && (priv->version != QCA_VER_APPE))
	{
		return;
	}

	aos_lock_bh(&priv->fdb_sw_sync_lock);
	SSDK_DEBUG("fdb_sw_sync_port_map 0x%x\n", priv->fdb_sw_sync_port_map);
	SW_PBMP_AND(priv->fdb_sw_sync_port_map, ~port_map);
	if (priv->fdb_sw_sync_port_map == 0 && priv->fdb_polling_started == A_TRUE)
	{
		aos_unlock_bh(&priv->fdb_sw_sync_lock);
		cancel_delayed_work_sync(&priv->fdb_sw_sync_dwork);
		priv->fdb_polling_started = A_FALSE;
	}
	else
		aos_unlock_bh(&priv->fdb_sw_sync_lock);
#ifdef IN_FDB
	ref_fdb_sw_sync_reset(priv, port_map);
#endif
	SSDK_DEBUG("fdb_sw_sync_port_map 0x%x\n", priv->fdb_sw_sync_port_map);
}

#if defined(APPE)
static sw_error_t
qca_ppe_port_reset(a_uint32_t dev_id)
{
#if 0
	a_uint32_t i = 0, port_max = SSDK_PHYSICAL_PORT7;

#if defined(MPPE)
	if (adpt_chip_revision_get(dev_id) == MPPE_REVISION)
		port_max = SSDK_PHYSICAL_PORT3;
#endif
	for(i = SSDK_PHYSICAL_PORT1; i < port_max; i++) {
#ifdef IN_PORTCONTROL
		fal_port_rxmac_status_set(dev_id, i, A_FALSE);
#endif
		ssdk_port_mac_clock_reset(dev_id, i);
	}
#endif
	return SW_OK;
}

sw_error_t ssdk_ppe_hw_recover(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;

	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		rv = qca_ppe_port_reset(dev_id);
		SW_RTN_ON_ERROR(rv);
		rv = qca_appe_hw_init(dev_id);
		SW_RTN_ON_ERROR(rv);
	}
	SSDK_INFO("ssdk ppe hw recover successfully!\n");

	return rv;
}
EXPORT_SYMBOL(ssdk_ppe_hw_recover);
#endif

int
qca_phy_id_chip(struct qca_phy_priv *priv)
{
	a_uint32_t value, version;

	value = qca_mii_read(priv->device_id, AR8327_REG_CTRL);
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
		SSDK_ERROR("unsupported QCA device\n");
		return -ENODEV;
	}
}

#if defined(IN_SWCONFIG)
static int qca_switchdev_register(struct qca_phy_priv *priv)
{
	struct switch_dev *sw_dev;
	int ret = SW_OK;
	sw_dev = &priv->sw_dev;

	switch (priv->version) {
		case QCA_VER_AR8227:
			sw_dev->name = "QCA AR8227";
			sw_dev->alias = "QCA AR8227";
			break;
		case QCA_VER_AR8327:
			sw_dev->name = "QCA AR8327";
			sw_dev->alias = "QCA AR8327";
			break;
		case QCA_VER_AR8337:
			sw_dev->name = "QCA AR8337";
			sw_dev->alias = "QCA AR8337";
			break;
		case QCA_VER_DESS:
			sw_dev->name = "QCA DESS";
			sw_dev->alias = "QCA DESS";
			break;
		case QCA_VER_APPE:
		case QCA_VER_HPPE:
#ifdef HPPE
			sw_dev->name = "QCA "PPE_STR;
			sw_dev->alias = "QCA "PPE_STR;
#endif
			break;
		case QCA_VER_MHT:
			sw_dev->name = "QCA MHT";
			sw_dev->alias = "QCA MHT";
			break;
		case QCA_VER_SCOMPHY:
#ifdef MP
			if(adapt_scomphy_revision_get(priv->device_id)
				== MP_GEPHY)
			{
				sw_dev->name = "QCA MP";
				sw_dev->alias = "QCA MP";
			}
#endif
			break;
		default:
			sw_dev->name = "unknown switch";
			sw_dev->alias = "unknown switch";
			break;
	}

	sw_dev->ops = &qca_ar8327_sw_ops;
	sw_dev->vlans = AR8327_MAX_VLANS;
	sw_dev->ports = priv->ports;

	ret = register_switch(sw_dev, NULL);
	if (ret != SW_OK) {
		SSDK_ERROR("register_switch failed for %s\n", sw_dev->name);
		return ret;
	}

	return ret;
}
#endif

#if defined(DESS) || defined(HPPE) || defined (ISISC) || defined (ISIS) || defined(MP) || defined(MHT)
static int ssdk_switch_register(a_uint32_t dev_id, ssdk_chip_type  chip_type)
{
	struct qca_phy_priv *priv;
	int ret = 0;
	a_uint32_t chip_id = 0;
	priv = qca_phy_priv_global[dev_id];

	priv->mii_read = qca_mii_read;
	priv->mii_write = qca_mii_write;
#if 0
	if (chip_type == CHIP_DESS || chip_type == CHIP_MHT) {
		priv->ports = 6;
	} else if ((chip_type == CHIP_ISIS) || (chip_type == CHIP_ISISC)) {
		priv->ports = 7;
	} else if (chip_type == CHIP_SCOMPHY) {
#ifdef MP
		if(adapt_scomphy_revision_get(priv->device_id) == MP_GEPHY) {
			/*for ipq50xx, port id is 1 and 2, port 0 is not available*/
			priv->ports = 3;
		}
#endif
	} else {
#ifdef MPPE
		if (chip_type == CHIP_APPE &&
			adpt_chip_revision_get(priv->device_id) == MPPE_REVISION) {
			priv->ports = 3;
		} else
#endif
			priv->ports = SSDK_MAX_PORT_NUM;
	}
#endif
#ifdef MP
	if(chip_type == CHIP_SCOMPHY)
	{
		priv->version = QCA_VER_SCOMPHY;
	}
	else
#endif
	{
		if (fal_reg_get(dev_id, 0, (a_uint8_t *)&chip_id, 4) == SW_OK) {
			priv->version = ((chip_id >> 8) & 0xff);
			priv->revision = (chip_id & 0xff);
		}
	}

	mutex_init(&priv->reg_mutex);

#if defined(IN_SWCONFIG)
	ret = qca_switchdev_register(priv);
	if (ret != SW_OK) {
		return ret;
	}
#endif
	priv->qca_ssdk_sw_dev_registered = A_TRUE;
	ret = qca_phy_mib_work_start(qca_phy_priv_global[dev_id]);
	if (ret != 0) {
			SSDK_ERROR("qca_phy_mib_work_start failed for chip 0x%02x%02x\n", priv->version, priv->revision);
			return ret;
	}

	if(priv->link_polling_required)
	{
		ret = qm_err_check_work_start(priv);
		if (ret != 0)
		{
			SSDK_ERROR("qm_err_check_work_start failed for chip 0x%02x%02x\n", priv->version, priv->revision);
			return ret;
		}
#ifdef HPPE
		if (_ssdk_mac_sw_sync_chip_check(priv) != SW_OK) {
			SSDK_INFO("mac_sw_sync is not enabled on chip 0x%02x%02x\n",
				priv->version, priv->revision);
			return 0;
		}

		if (!ssdk_is_emulation(dev_id)) {
			ret = qca_mac_sw_sync_work_init(priv);
			if (ret != 0) {
				SSDK_ERROR("qca_mac_sw_sync_work_init failed on chip 0x%02x%02x\n",
						priv->version, priv->revision);
				return ret;
			}
		}
#endif
	}
#if defined(HPPE) || defined(MHT)
	ret = qca_fdb_sw_sync_work_init(priv);
	if (ret != 0) {
		SSDK_ERROR("qca_fdb_sw_sync_work_init failed on chip 0x%02x%02x\n",
				priv->version, priv->revision);
		return ret;
	}
#endif
	if(priv->interrupt_no > 0)
	{
		snprintf(priv->intr_name, IFNAMSIZ, "switch%d", dev_id);
		switch(priv->version)
		{
#ifdef MHT
			case QCA_VER_MHT:
				priv->interrupt_flag = IRQF_TRIGGER_HIGH;
				break;
#endif
#ifdef ISISC
			case QCA_VER_AR8337:
				priv->interrupt_flag = IRQF_TRIGGER_LOW;
				break;
#endif
			default:
				priv->interrupt_flag = IRQF_TRIGGER_NONE;
				break;
		}
		SSDK_INFO("intr_number:%d, intr_name:%s, intr_flag:%d\n",
			priv->interrupt_no, priv->intr_name, priv->interrupt_flag);
		ret = qca_intr_init(priv);
		if(ret)
		{
			SSDK_ERROR("the interrupt init faild !\n");
			return ret;
		}
	}

	return 0;
}

static int ssdk_switch_unregister(a_uint32_t dev_id)
{
	qca_phy_mib_work_stop(qca_phy_priv_global[dev_id]);
	qm_err_check_work_stop(qca_phy_priv_global[dev_id]);
#ifdef HPPE
	if(!ssdk_is_emulation(dev_id)) {
		ssdk_mac_sw_sync_work_stop(dev_id);
	}
#endif
#if defined(IN_SWCONFIG)
	unregister_switch(&qca_phy_priv_global[dev_id]->sw_dev);
#endif
	return 0;
}
#endif

#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
char ssdk_driver_name[] = "ess_ssdk";

static int ssdk_probe(struct platform_device *pdev)
{
	struct device_node *np;

	np = of_node_get(pdev->dev.of_node);
	if (of_device_is_compatible(np, "qcom,ess-instance"))
		return of_platform_populate(np, NULL, NULL, &pdev->dev);

	return 0;
}

static const struct of_device_id ssdk_of_mtable[] = {
	{.compatible = "qcom,ess-switch" },
	{.compatible = "qcom,ess-switch-ipq60xx" },
	{.compatible = "qcom,ess-switch-ipq807x" },
	{.compatible = "qcom,ess-switch-ipq95xx" },
	{.compatible = "qcom,ess-switch-ipq53xx" },
	{.compatible = "qcom,ess-instance" },
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
/*qca808x_start*/
sw_error_t
ssdk_init(a_uint32_t dev_id, ssdk_init_cfg * cfg)
{
	sw_error_t rv;

	rv = fal_init(dev_id, cfg);
//	if (rv != SW_OK)
//		SSDK_ERROR("ssdk fal init failed: %d. \r\n", rv);

/*qca808x_end*/
	if(!ssdk_is_emulation(dev_id))
/*qca808x_start*/
	{
		rv = ssdk_phy_driver_init(dev_id);
		if (rv != SW_OK)
			SSDK_ERROR("ssdk phy init failed: %d. \r\n", rv);
	}

	return rv;
}

sw_error_t
ssdk_cleanup(a_uint32_t dev_id)
{
	sw_error_t rv;

	rv = fal_cleanup(dev_id);
/*qca808x_end*/
	if(!ssdk_is_emulation(dev_id))
/*qca808x_start*/
	{
		rv = ssdk_phy_driver_cleanup(dev_id);
	}

	return rv;
}
/*qca808x_end*/

sw_error_t
ssdk_hsl_access_mode_set(a_uint32_t dev_id, hsl_access_mode reg_mode)
{
    sw_error_t rv;

    rv = hsl_access_mode_set(dev_id, reg_mode);
    return rv;
}

void switch_cpuport_setup(a_uint32_t dev_id)
{
#ifdef IN_PORTCONTROL
	//According to HW suggestion, enable CPU port flow control for Dakota
	fal_port_flowctrl_forcemode_set(dev_id, 0, A_TRUE);
	fal_port_flowctrl_set(dev_id, 0, A_TRUE);
	fal_port_duplex_set(dev_id, 0, FAL_FULL_DUPLEX);
	fal_port_speed_set(dev_id, 0, FAL_SPEED_1000);
	udelay(10);
	fal_port_txmac_status_set(dev_id, 0, A_TRUE);
	fal_port_rxmac_status_set(dev_id, 0, A_TRUE);
#endif
}
#ifdef IN_AQUANTIA_PHY
#if 1 //def CONFIG_MDIO
static struct mdio_if_info ssdk_mdio_ctl;
#endif
static struct net_device *ssdk_miireg_netdev = NULL;

static int ssdk_miireg_open(struct net_device *netdev)
{
	return 0;
}
static int ssdk_miireg_close(struct net_device *netdev)
{
	return 0;
}

static int ssdk_miireg_do_ioctl(struct net_device *netdev,
			struct ifreq *ifr, int32_t cmd)
{
	int ret = -EINVAL;
#if 0 //def CONFIG_MDIO
	struct mii_ioctl_data *mii_data = if_mii(ifr);
	ret = mdio_mii_ioctl(&ssdk_mdio_ctl, mii_data, cmd);
#endif
	return ret;
}

static const struct net_device_ops ssdk_netdev_ops = {
	.ndo_open = &ssdk_miireg_open,
	.ndo_stop = &ssdk_miireg_close,
	.ndo_do_ioctl = &ssdk_miireg_do_ioctl,
};

#if 1 //def CONFIG_MDIO
static int ssdk_miireg_ioctl_read(struct net_device *netdev, int phy_addr, int mmd, uint16_t addr)
{
	a_uint16_t val = 0;

	if (MDIO_DEVAD_NONE == mmd) {
		val = hsl_phy_mii_reg_read(0, phy_addr, addr);
		return (int)val;
	}

	val = hsl_phy_mmd_reg_read(0, phy_addr, A_TRUE, mmd, addr);

	return (int)val;
}

static int ssdk_miireg_ioctl_write(struct net_device *netdev, int phy_addr, int mmd,
				uint16_t addr, uint16_t value)
{
	if (MDIO_DEVAD_NONE == mmd) {
		hsl_phy_mii_reg_write(0, phy_addr, addr, value);
		return 0;
	}

	hsl_phy_mmd_reg_write(0, phy_addr, A_TRUE, mmd, addr, value);

	return 0;
}
#endif

static void ssdk_netdev_setup(struct net_device *dev)
{
	dev->netdev_ops = &ssdk_netdev_ops;
}
static void ssdk_miireg_ioctrl_register(void)
{
	if (ssdk_miireg_netdev)
		return;
#if 1 //def CONFIG_MDIO
	ssdk_mdio_ctl.mdio_read = ssdk_miireg_ioctl_read;
	ssdk_mdio_ctl.mdio_write = ssdk_miireg_ioctl_write;
	ssdk_mdio_ctl.mode_support = MDIO_SUPPORTS_C45;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))
	ssdk_miireg_netdev = alloc_netdev(100, "miireg", 0, ssdk_netdev_setup);
#else
	ssdk_miireg_netdev = alloc_netdev(100, "miireg", ssdk_netdev_setup);
#endif
	if (ssdk_miireg_netdev)
		register_netdev(ssdk_miireg_netdev);
}

static void ssdk_miireg_ioctrl_unregister(void)
{
	if (ssdk_miireg_netdev) {
		unregister_netdev(ssdk_miireg_netdev);
		kfree(ssdk_miireg_netdev);
		ssdk_miireg_netdev = NULL;
	}
}
#endif
static void ssdk_driver_register(a_uint32_t dev_id)
{
	hsl_reg_mode reg_mode;

	reg_mode = ssdk_switch_reg_access_mode_get(dev_id);
	if(reg_mode == HSL_REG_LOCAL_BUS) {
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
		platform_driver_register(&ssdk_driver);
#endif
	}
}

static void ssdk_driver_unregister(a_uint32_t dev_id)
{
	hsl_reg_mode reg_mode;

	reg_mode= ssdk_switch_reg_access_mode_get(dev_id);
	if (reg_mode == HSL_REG_LOCAL_BUS) {
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
		platform_driver_unregister(&ssdk_driver);
#endif
	}
}
/*qca808x_start*/
static inline a_uint32_t qca_detect_phyid(a_uint32_t dev_id)
{
	a_uint32_t phy_id = 0, port_id = 0;
	a_uint32_t port_bmp = qca_ssdk_port_bmp_get(dev_id);
	while (port_bmp) {
		if (port_bmp & 0x1) {
			phy_id = hsl_phyid_get(dev_id, port_id);
			if (INVALID_PHY_ID != phy_id)
				break;
		}
		port_bmp >>= 1;
		port_id++;
	}
	return phy_id;
}

static int chip_is_scomphy(a_uint32_t dev_id, ssdk_init_cfg* cfg)
{
	int rv = -ENODEV;
	a_uint32_t phy_id = qca_detect_phyid(dev_id);

	switch (phy_id) {
		/*qca808x_end*/
		case QCA8030_PHY:
		case QCA8033_PHY:
		case QCA8035_PHY:
		case MP_GEPHY:
			/*qca808x_start*/
		case QCA8081_PHY_V1_1:
			cfg->chip_type = CHIP_SCOMPHY;
			/*MP GEPHY is always the first port*/
			if(cfg->phy_id == 0) {
				cfg->phy_id = phy_id;
			}
			rv = SW_OK;
			break;
		default:
			break;
	}

	return rv;
}

static int chip_ver_get(a_uint32_t dev_id, ssdk_init_cfg* cfg)
{
	int rv = SW_OK;
	a_uint8_t chip_ver = 0;
	a_uint8_t chip_revision = 0;
/*qca808x_end*/
	hsl_reg_mode reg_mode;
	a_uint32_t reg_val = 0;

	reg_mode= ssdk_switch_reg_access_mode_get(dev_id);
	if(reg_mode == HSL_REG_MDIO) {
		a_uint32_t phy_id = qca_detect_phyid(dev_id);
		switch (phy_id) {
			case QCA8084_PHY:
				chip_ver = QCA_VER_MHT;
				break;
			case F1V4_PHY:
				chip_ver = QCA_VER_AR8337;
			default:
				break;
		}
	} else {
		qca_switch_reg_read(dev_id,0,(a_uint8_t *)&reg_val, 4);
		chip_ver = (reg_val&0xff00)>>8;
		chip_revision = reg_val&0xff;
	}
/*qca808x_start*/
	switch (chip_ver) {
		case QCA_VER_AR8227:
			cfg->chip_type = CHIP_SHIVA;
			break;
		case QCA_VER_AR8337:
			cfg->chip_type = CHIP_ISISC;
			break;
		case QCA_VER_AR8327:
			cfg->chip_type = CHIP_ISIS;
			break;
		case QCA_VER_DESS:
			cfg->chip_type = CHIP_DESS;
			break;
		case QCA_VER_HPPE:
			cfg->chip_type = CHIP_HPPE;
			cfg->chip_revision = chip_revision;
			break;
		case QCA_VER_APPE:
			cfg->chip_type = CHIP_APPE;
			cfg->chip_revision = chip_revision;
			break;
		case QCA_VER_MHT:
			cfg->chip_type = CHIP_MHT;
			break;
		default:
			/* try single phy without switch connected */
			rv = chip_is_scomphy(dev_id, cfg);
	}

	return rv;
}
/*qca808x_end*/

/*qca808x_start*/
void ssdk_cfg_default_init(ssdk_init_cfg *cfg)
{
	memset(cfg, 0, sizeof(ssdk_init_cfg));
	cfg->cpu_mode = HSL_CPU_1;
	cfg->nl_prot = 30;
/*qca808x_end*/

	cfg->reg_func.header_reg_set = qca_switch_reg_write;
	cfg->reg_func.header_reg_get = qca_switch_reg_read;
	cfg->reg_func.mii_reg_set = qca_mii_write;
	cfg->reg_func.mii_reg_get = qca_mii_read;
/*qca808x_start*/
}
/*qca808x_end*/

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
#endif
#endif

//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
static int ssdk_dev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	int rv = 0;
	ssdk_init_cfg cfg;
#ifdef MP
	a_uint32_t port_id = 0, dev_id = 0;
	struct qca_phy_priv *priv = ssdk_phy_priv_data_get(dev_id);
	adpt_api_t *p_api = adpt_api_ptr_get(dev_id);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
#else
	struct net_device *dev = (struct net_device *)ptr;
#endif

	ssdk_cfg_default_init(&cfg);
	rv = chip_ver_get(0, &cfg);
	if (rv) {
		SSDK_ERROR("chip verfion get failed\n");
		return NOTIFY_DONE;
	}
	switch (event) {
#ifdef IN_RFS
#if defined(CONFIG_RFS_ACCEL)
		case NETDEV_UP:
			if (strstr(dev->name, "eth")) {
				if (dev->netdev_ops && dev->netdev_ops->ndo_register_rfs_filter) {
					dev->netdev_ops->ndo_register_rfs_filter(dev,
						ssdk_netdev_rfs_cb);
				}
			}
			break;
#endif
#endif
		case NETDEV_CHANGEMTU:
			if(dev->type == ARPHRD_ETHER) {
				if (cfg.chip_type == CHIP_DESS ||
					cfg.chip_type == CHIP_ISIS ||
					cfg.chip_type == CHIP_ISISC) {
					struct net_device *eth_dev = NULL;
					unsigned int mtu= 0;

					if(!strcmp(dev->name, "eth0")) {
						eth_dev = dev_get_by_name(&init_net, "eth1");
					} else if (!strcmp(dev->name, "eth1")) {
						eth_dev = dev_get_by_name(&init_net, "eth0");
					} else {
						return NOTIFY_DONE;
					}
					if (!eth_dev) {
						return NOTIFY_DONE;
					}
					mtu = dev->mtu > eth_dev->mtu ? dev->mtu : eth_dev->mtu;
#ifdef IN_MISC
					fal_frame_max_size_set(0, mtu + 18);
#endif
					dev_put(eth_dev);
				}
			}
			break;
#ifdef MP
		case NETDEV_CHANGE:
			if ((cfg.chip_type == CHIP_SCOMPHY) &&
				(cfg.phy_id == MP_GEPHY)) {
				if ((p_api == NULL) || (p_api->adpt_port_netdev_notify_set == NULL)
					|| (priv == NULL)) {
					SSDK_ERROR("Failed to get pointer\n");
					return NOTIFY_DONE;
				}
				if (dev->phydev != NULL) {
					port_id = qca_ssdk_phydev_to_port(priv->device_id,
						dev->phydev);
					rv = p_api->adpt_port_netdev_notify_set(priv, port_id);
					if (rv) {
						SSDK_ERROR("netdev change notify failed\n");
						return NOTIFY_DONE;
					}
				}
			}
			break;
#endif
	}

	return NOTIFY_DONE;
}

/*qca808x_start*/
static void ssdk_free_priv(void)
{
	a_uint32_t dev_id, dev_num = 1;

	if(!qca_phy_priv_global) {
		return;
	}
/*qca808x_end*/
	dev_num = ssdk_switch_device_num_get();
/*qca808x_start*/
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (qca_phy_priv_global[dev_id]) {
			kfree(qca_phy_priv_global[dev_id]);
		}

		qca_phy_priv_global[dev_id] = NULL;
	}

	kfree(qca_phy_priv_global);

	qca_phy_priv_global = NULL;
/*qca808x_end*/
	ssdk_switch_device_num_exit();
/*qca808x_start*/
}

static int ssdk_alloc_priv(a_uint32_t dev_num)
{
	int rev = SW_OK;
	a_uint32_t dev_id = 0;

	qca_phy_priv_global = kzalloc(dev_num * sizeof(struct qca_phy_priv *), GFP_KERNEL);
	if (qca_phy_priv_global == NULL) {
		return -ENOMEM;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		qca_phy_priv_global[dev_id] = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
		if (qca_phy_priv_global[dev_id] == NULL) {
			return -ENOMEM;
		}
/*qca808x_end*/
		qca_phy_priv_global[dev_id]->qca_ssdk_sw_dev_registered = A_FALSE;
		qca_phy_priv_global[dev_id]->ess_switch_flag = A_FALSE;
/*qca808x_start*/
		qca_ssdk_port_bmp_init(dev_id);
		qca_ssdk_phy_info_init(dev_id);
	}

	return rev;
}

#ifndef SSDK_STR
#define SSDK_STR "ssdk"
#endif
#if defined (ISISC) || defined (ISIS)
static void qca_ar8327_gpio_reset(struct qca_phy_priv *priv)
{
	struct device_node *np = NULL;
	const __be32 *reset_gpio;
	a_int32_t len;
	int gpio_num = 0, ret = 0;

	if (priv->ess_switch_flag == A_TRUE)
		np = priv->of_node;
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
		np = priv->phy->mdio.dev.of_node;
#else
		np = priv->phy->dev.of_node;
#endif
	if(!np)
		return;
	gpio_num = of_get_named_gpio(np, "reset_gpio", 0);
	if(gpio_num < 0)
	{
		reset_gpio = of_get_property(np, "reset_gpio", &len);
		if (!reset_gpio )
		{
			SSDK_INFO("reset_gpio node does not exist\n");
			return;
		}
		gpio_num = be32_to_cpup(reset_gpio);
		if(gpio_num <= 0)
		{
			SSDK_INFO("reset gpio doesn't exist\n ");
			return;
		}
	}
	ret = gpio_request(gpio_num, "reset_gpio");
	if(ret)
	{
		SSDK_ERROR("gpio%d request failed, ret:%d\n", gpio_num, ret);
		return;
	}
	gpio_direction_output(gpio_num, SSDK_GPIO_RESET);
	gpio_set_value(gpio_num, SSDK_GPIO_RESET);
	msleep(200);
	gpio_set_value(gpio_num, SSDK_GPIO_RELEASE);
	msleep(10);
	SSDK_INFO("GPIO%d reset switch done\n", gpio_num);

	gpio_free(gpio_num);

	return;
}
#endif
static int __init regi_init(void)
{
	a_uint32_t num = 0, dev_id = 0, dev_num = 1;
	ssdk_init_cfg cfg;
/*qca808x_end*/
	garuda_init_spec_cfg chip_spec_cfg;
/*qca808x_start*/
	int rv = 0;
/*qca808x_end*/
	/*init switch device num firstly*/
	ssdk_switch_device_num_init();

	dev_num = ssdk_switch_device_num_get();
/*qca808x_start*/
	rv = ssdk_alloc_priv(dev_num);
	if (rv)
		goto out;

	for (num = 0; num < dev_num; num++) {
		ssdk_cfg_default_init(&cfg);
/*qca808x_end*/
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
		if(SW_DISABLE == ssdk_dt_parse(&cfg, num, &dev_id)) {
			SSDK_INFO("ess-switch node is unavalilable\n");
			continue;
		}
#endif

		/* device id is the array index */
		qca_phy_priv_global[dev_id]->device_id = ssdk_device_id_get(dev_id);
		qca_phy_priv_global[dev_id]->ess_switch_flag = ssdk_ess_switch_flag_get(dev_id);
		qca_phy_priv_global[dev_id]->of_node = ssdk_dts_node_get(dev_id);
		INIT_LIST_HEAD(&(qca_phy_priv_global[dev_id]->sw_fdb_tbl));
		qca_phy_priv_global[dev_id]->ports = SSDK_PHYSICAL_PORT7;
/*qca808x_start*/
		rv = ssdk_plat_init(&cfg, dev_id);
		SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
/*qca808x_end*/
		ssdk_driver_register(dev_id);
/*qca808x_start*/
		rv = chip_ver_get(dev_id, &cfg);
		SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
/*qca808x_end*/
#ifdef IN_AQUANTIA_PHY
		ssdk_miireg_ioctrl_register();
#endif
		memset(&chip_spec_cfg, 0, sizeof(garuda_init_spec_cfg));
		cfg.chip_spec_cfg = &chip_spec_cfg;
/*qca808x_start*/
		rv = ssdk_init(dev_id, &cfg);
		SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
/*qca808x_end*/


		switch (cfg.chip_type)
		{
			case CHIP_ISIS:
			case CHIP_ISISC:
#if defined (ISISC) || defined (ISIS)
				if (qca_phy_priv_global[dev_id]->ess_switch_flag == A_TRUE) {
					qca_ar8327_gpio_reset(qca_phy_priv_global[dev_id]);
					rv = ssdk_switch_register(dev_id, cfg.chip_type);
					SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
					rv = qca_ar8327_hw_init(qca_phy_priv_global[dev_id]);
					SSDK_INFO("Initializing ISISC Done!!\n");
				}
#endif
				break;
			case CHIP_MHT:
#if defined(MHT)
				qca_phy_priv_global[dev_id]->ports = SSDK_PHYSICAL_PORT6;
				rv = qca_mht_hw_init(&cfg, dev_id);
				SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
				rv = ssdk_switch_register(dev_id, cfg.chip_type);
				SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
				SSDK_INFO("Initializing MHT Done!!\n");
#endif
				break;
			case CHIP_APPE:
#if defined(APPE)
				if(adpt_ppe_type_get(dev_id) == MPPE_TYPE)
					qca_phy_priv_global[dev_id]->ports = SSDK_PHYSICAL_PORT3;
				qca_appe_hw_init(dev_id);
				rv = ssdk_switch_register(dev_id, cfg.chip_type);
				SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
				SSDK_INFO("Initializing %s Done!!\n", PPE_STR);
#endif
				break;
			case CHIP_HPPE:
#if defined(HPPE)
				qca_hppe_hw_init(dev_id);
				rv = ssdk_switch_register(dev_id, cfg.chip_type);
				SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
				SSDK_INFO("Initializing %s Done!!\n", PPE_STR);
#endif
				break;

			case CHIP_SHIVA:
			case CHIP_ATHENA:
			case CHIP_GARUDA:
			case CHIP_HORUS:
			case CHIP_UNSPECIFIED:
				break;
			case CHIP_SCOMPHY:
#if defined(SCOMPHY)
					rv = qca_scomphy_hw_init(&cfg, dev_id);
					SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
#if defined(MP)
					if(cfg.phy_id == MP_GEPHY)
					{
						qca_phy_priv_global[dev_id]->ports =
							SSDK_PHYSICAL_PORT3;
						rv = ssdk_switch_register(dev_id, cfg.chip_type);
						SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
					}
#endif
					SSDK_INFO("Initializing SCOMPHY Done!!\n");
#endif
				break;
			default:
				break;
		}
/*qca808x_start*/

	}
/*qca808x_end*/

	ssdk_sysfs_init();
#ifdef IN_NETLINK
	ssdk_genl_init();
#endif

	if (rv == 0){
		/* register the notifier later should be ok */
		ssdk_dev_notifier.notifier_call = ssdk_dev_event;
		ssdk_dev_notifier.priority = 1;
		register_netdevice_notifier(&ssdk_dev_notifier);
	}
/*qca808x_start*/

out:
	if (rv == 0)
		SSDK_INFO("qca-%s module init succeeded!\n", SSDK_STR);
	else {
		if (rv == -ENODEV) {
			rv = 0;
			SSDK_INFO("qca-%s module init, no device found!\n", SSDK_STR);
		} else {
			SSDK_INFO("qca-%s module init failed! (code: %d)\n", SSDK_STR, rv);
			ssdk_free_priv();
		}
	}

	return rv;
}

static void __exit
regi_exit(void)
{
	a_uint32_t dev_id, dev_num = 1;
	sw_error_t rv = SW_OK;
/*qca808x_end*/
	unregister_netdevice_notifier(&ssdk_dev_notifier);

	dev_num = ssdk_switch_device_num_get();
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ssdk_driver_unregister(dev_id);
#if defined(DESS) || defined(HPPE) || defined(ISISC) || defined(ISIS) || defined(MP)
		if (qca_phy_priv_global[dev_id]->qca_ssdk_sw_dev_registered == A_TRUE)
			ssdk_switch_unregister(dev_id);
#endif
	}
/*qca808x_start*/
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		rv = ssdk_cleanup(dev_id);
	}

	if (rv == SW_OK)
		SSDK_INFO("qca-%s module exit  done!\n", SSDK_STR);
	else
		SSDK_ERROR("qca-%s module exit failed! (code: %d)\n", SSDK_STR, rv);
/*qca808x_end*/

	ssdk_sysfs_exit();
#ifdef IN_AQUANTIA_PHY
	ssdk_miireg_ioctrl_unregister();
#endif
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ssdk_plat_exit(dev_id);
	}
#ifdef IN_NETLINK
	ssdk_genl_deinit();
#endif

/*qca808x_start*/
	ssdk_free_priv();
}

module_init(regi_init);
module_exit(regi_exit);

MODULE_DESCRIPTION("QCA SSDK Driver");
MODULE_LICENSE("Dual BSD/GPL");
/*qca808x_end*/
