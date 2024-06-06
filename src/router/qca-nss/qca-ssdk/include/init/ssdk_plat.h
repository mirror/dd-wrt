/*
 * Copyright (c) 2012, 2014-2015, 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef __SSDK_PLAT_H
#define __SSDK_PLAT_H

#include "sw.h"
/*qca808x_end*/
#include <linux/kconfig.h>
/*qca808x_start*/
#include <linux/version.h>
/*qca808x_end*/
#include <linux/kernel.h>
#include <linux/module.h>
#if defined(IN_SWCONFIG)
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/switch.h>
#else
#include <net/switch.h>
#endif
#endif
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
#include <dt-bindings/arm/qcom,ids.h>
#include <linux/soc/qcom/smem.h>
#else
#include <soc/qcom/socinfo.h>
#endif

/*qca808x_start*/
#include <linux/phy.h>
#include <linux/bitfield.h>

#ifndef BIT
#define BIT(_n)                                      (1UL << (_n))
#endif


#ifndef BITS
#define BITS(_s, _n)                                 (((1UL << (_n)) - 1) << _s)
#endif

/* Atheros specific MII registers */
#define QCA_MII_MMD_ADDR                             0x0d
#define QCA_MII_MMD_DATA                             0x0e
#define QCA_MII_DBG_ADDR                             0x1d
#define QCA_MII_DBG_DATA                             0x1e
/*qca808x_end*/
#define AR8327_REG_CTRL                              0x0000
#define   AR8327_CTRL_REVISION                       BITS(0, 8)
#define   AR8327_CTRL_REVISION_S                     0
#define   AR8327_CTRL_VERSION                        BITS(8, 8)
#define   AR8327_CTRL_VERSION_S                      8
#define   AR8327_CTRL_RESET                          BIT(31)

#define AR8327_REG_LED_CTRL_0                        0x50
#define AR8327_REG_LED_CTRL_1                        0x54
#define AR8327_REG_LED_CTRL_2                        0x58
#define AR8327_REG_LED_CTRL_3                        0x5c

#define AR8327_REG_PORT_STATUS(_i)                   (0x07c + (_i) * 4)

#define   AR8327_PORT_STATUS_SPEED                   BITS(0,2)
#define   AR8327_PORT_STATUS_SPEED_S                 0
#define   AR8327_PORT_STATUS_TXMAC                   BIT(2)
#define   AR8327_PORT_STATUS_RXMAC                   BIT(3)
#define   AR8327_PORT_STATUS_TXFLOW                  BIT(4)
#define   AR8327_PORT_STATUS_RXFLOW                  BIT(5)
#define   AR8327_PORT_STATUS_DUPLEX                  BIT(6)
#define   AR8327_PORT_STATUS_LINK_UP                 BIT(8)
#define   AR8327_PORT_STATUS_LINK_AUTO               BIT(9)
#define   AR8327_PORT_STATUS_LINK_PAUSE              BIT(10)

#define AR8327_REG_PAD0_CTRL                         0x4
#define AR8327_REG_PAD5_CTRL                         0x8
#define AR8327_REG_PAD6_CTRL                         0xc
#define   AR8327_PAD_CTRL_MAC_MII_RXCLK_SEL          BIT(0)
#define   AR8327_PAD_CTRL_MAC_MII_TXCLK_SEL          BIT(1)
#define   AR8327_PAD_CTRL_MAC_MII_EN                 BIT(2)
#define   AR8327_PAD_CTRL_MAC_GMII_RXCLK_SEL         BIT(4)
#define   AR8327_PAD_CTRL_MAC_GMII_TXCLK_SEL         BIT(5)
#define   AR8327_PAD_CTRL_MAC_GMII_EN                BIT(6)
#define   AR8327_PAD_CTRL_SGMII_EN                   BIT(7)
#define   AR8327_PAD_CTRL_PHY_MII_RXCLK_SEL          BIT(8)
#define   AR8327_PAD_CTRL_PHY_MII_TXCLK_SEL          BIT(9)
#define   AR8327_PAD_CTRL_PHY_MII_EN                 BIT(10)
#define   AR8327_PAD_CTRL_PHY_GMII_PIPE_RXCLK_SEL    BIT(11)
#define   AR8327_PAD_CTRL_PHY_GMII_RXCLK_SEL         BIT(12)
#define   AR8327_PAD_CTRL_PHY_GMII_TXCLK_SEL         BIT(13)
#define   AR8327_PAD_CTRL_PHY_GMII_EN                BIT(14)
#define   AR8327_PAD_CTRL_PHYX_GMII_EN               BIT(16)
#define   AR8327_PAD_CTRL_PHYX_RGMII_EN              BIT(17)
#define   AR8327_PAD_CTRL_PHYX_MII_EN                BIT(18)
#define   AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL      BITS(20, 2)
#define   AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL_S    20
#define   AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL      BITS(22, 2)
#define   AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL_S    22
#define   AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN       BIT(24)
#define   AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_EN       BIT(25)
#define   AR8327_PAD_CTRL_RGMII_EN                   BIT(26)

#define AR8327_PORT5_PHY_ADDR                        4
/*AR8327 inner phy debug register for RGMII mode*/
#define AR8327_PHY_REG_MODE_SEL                      0x12
#define AR8327_PHY_RGMII_MODE                        BIT(3)
#define AR8327_PHY_REG_TEST_CTRL                     0x0
#define AR8327_PHY_RGMII_RX_DELAY                    BIT(15)
#define AR8327_PHY_REG_SYS_CTRL                      0x5
#define AR8327_PHY_RGMII_TX_DELAY                    BIT(8)


#define AR8327_REG_POS                               0x10
#define AR8327_REG_POS_HW_INIT                       0x261320
#define   AR8327_POS_POWER_ON_SEL                    BIT(31)
#define   AR8327_POS_LED_OPEN_EN                     BIT(24)
#define   AR8327_POS_SERDES_AEN                      BIT(7)

#define AR8327_REG_MODULE_EN                         0x30
#define   AR8327_REG_MODULE_EN_QM_ERR                BIT(8)
#define   AR8327_REG_MODULE_EN_LOOKUP_ERR            BIT(9)

#define AR8327_REG_MAC_SFT_RST                       0x68

#define AR8327_REG_PAD_SGMII_CTRL                    0xe0
#define AR8327_REG_PAD_SGMII_CTRL_HW_INIT            0xc70164c0
#define   AR8327_PAD_SGMII_CTRL_MODE_CTRL            BITS(22, 2)
#define   AR8327_PAD_SGMII_CTRL_MODE_CTRL_S          22
#define   AR8327_PAD_SGMII_CTRL_EN_SD                BIT(4)
#define   AR8327_PAD_SGMII_CTRL_EN_TX                BIT(3)
#define   AR8327_PAD_SGMII_CTRL_EN_RX                BIT(2)
#define   AR8327_PAD_SGMII_CTRL_EN_PLL               BIT(1)
#define   AR8327_PAD_SGMII_CTRL_EN_LCKDT             BIT(0)

#define AR8327_REG_PAD_MAC_PWR_SEL                   0x0e4
#define   AR8327_PAD_MAC_PWR_RGMII0_1_8V             BIT(18)
#define   AR8327_PAD_MAC_PWR_RGMII1_1_8V             BIT(19)

#define AR8327_REG_PORT_LOOKUP(_i)                   (0x660 + (_i) * 0xc)
#define AR8327_REG_PORT_VLAN0(_i)                    (0x420 + (_i) * 0x8)

#define DESS_PSGMII_MODE_CONTROL                     0x1b4
#define   DESS_PSGMII_ATHR_CSCO_MODE_25M             BIT(0)

#define DESS_PSGMIIPHY_TX_CONTROL                    0x288

#define DESS_PSGMII_PLL_VCO_RELATED_CONTROL_1        0x78c
#define   DESS_PSGMII_MII_REG_UPHY_PLL_LCKDT_EN      BIT(0)

#define DESS_PSGMII_VCO_CALIBRATION_CONTROL_1        0x9c

#define SSDK_PSGMII_ID                               5
/*qca808x_start*/
#define SSDK_PHY_BCAST_ID                            0x1f
#define SSDK_PHY_MIN_ID                              0x0
#define SSDK_PORT_CPU                                0
/*qca808x_end*/
#define SSDK_PORT0_FC_THRESH_ON_DFLT                 0x60
#define SSDK_PORT0_FC_THRESH_OFF_DFLT                0x90

#define AR8327_NUM_PHYS                              5
#define AR8327_PORT_CPU                              0
#define AR8327_NUM_PORTS                             7
#define AR8327_MAX_VLANS                             128

#define MII_PHYADDR_C45                              (1<<30)

#define SSDK_GPIO_RESET                              0
#define SSDK_GPIO_RELEASE                            1
#define SSDK_MAX_GPIO                                0xffff
#define SSDK_INVALID_GPIO                            (SSDK_MAX_GPIO+1)

enum {
    AR8327_PORT_SPEED_10M = 0,
    AR8327_PORT_SPEED_100M = 1,
    AR8327_PORT_SPEED_1000M = 2,
    AR8327_PORT_SPEED_NONE = 3,
};
/*qca808x_start*/
enum {
	QCA_VER_AR8216 = 0x01,
	QCA_VER_AR8227 = 0x02,
	QCA_VER_AR8236 = 0x03,
	QCA_VER_AR8316 = 0x10,
	QCA_VER_AR8327 = 0x12,
	QCA_VER_AR8337 = 0x13,
	QCA_VER_DESS = 0x14,
	QCA_VER_HPPE = 0x15,
	QCA_VER_MHT = 0x17,
	QCA_VER_APPE = 0x20,
	QCA_VER_SCOMPHY = 0xEE
};
/*qca808x_end*/
/*poll mib per 120secs*/
#define QCA_PHY_MIB_WORK_DELAY	120000
#define QCA_MIB_ITEM_NUMBER \
	(sizeof(fal_mib_counter_t)/sizeof(a_uint64_t))

#define SSDK_MAX_UNIPHY_INSTANCE        3
#define SSDK_UNIPHY_INSTANCE0           0
#define SSDK_UNIPHY_INSTANCE1           1
#define SSDK_UNIPHY_INSTANCE2           2
#define SSDK_UNIPHY_CHANNEL0            0
#define SSDK_UNIPHY_CHANNEL1            1
#define SSDK_UNIPHY_CHANNEL2            2
#define SSDK_UNIPHY_CHANNEL3            3
#define SSDK_UNIPHY_CHANNEL4            4

/*qca808x_start*/
#define SSDK_PHYSICAL_PORT0             0
#define SSDK_PHYSICAL_PORT1             1
#define SSDK_PHYSICAL_PORT2             2
#define SSDK_PHYSICAL_PORT3             3
#define SSDK_PHYSICAL_PORT4             4
#define SSDK_PHYSICAL_PORT5             5
#define SSDK_PHYSICAL_PORT6             6
#define SSDK_PHYSICAL_PORT7             7
/*qca808x_end*/
#define SSDK_GLOBAL_INT0_ACL_INI_INT        (1<<29)
#define SSDK_GLOBAL_INT0_LOOKUP_INI_INT     (1<<28)
#define SSDK_GLOBAL_INT0_QM_INI_INT         (1<<27)
#define SSDK_GLOBAL_INT0_MIB_INI_INT        (1<<26)
#define SSDK_GLOBAL_INT0_OFFLOAD_INI_INT    (1<<25)
#define SSDK_GLOBAL_INT0_HARDWARE_INI_DONE  (1<<24)

#define SSDK_GLOBAL_INITIALIZED_STATUS 				\
			(					\
			SSDK_GLOBAL_INT0_ACL_INI_INT | 		\
			SSDK_GLOBAL_INT0_LOOKUP_INI_INT | 	\
			SSDK_GLOBAL_INT0_QM_INI_INT | 		\
			SSDK_GLOBAL_INT0_MIB_INI_INT | 		\
			SSDK_GLOBAL_INT0_OFFLOAD_INI_INT |	\
			SSDK_GLOBAL_INT0_HARDWARE_INI_DONE	\
			)
/*qca808x_start*/
#define SSDK_LOG_LEVEL_ERROR    0
#define SSDK_LOG_LEVEL_WARN     1
#define SSDK_LOG_LEVEL_INFO     2
#define SSDK_LOG_LEVEL_DEBUG    3
#define SSDK_LOG_LEVEL_DEFAULT  SSDK_LOG_LEVEL_INFO
#define SSDK_MII_BUS_MAX           2
#define SSDK_MII_INVALID_BUS_ID    SSDK_MII_BUS_MAX
#define SSDK_MII_DEFAULT_BUS_ID    0

#define SSDK_ADDR_C45            (1<<30)
#define SSDK_DEVADDR_C45_MASK    GENMASK(20, 16)
#define SSDK_REGADDR_C45_MASK    GENMASK(15, 0)

extern a_uint32_t ssdk_log_level;

#define __SSDK_LOG_FUN(lev, fmt, ...) \
	do { \
		if (SSDK_LOG_LEVEL_##lev <= ssdk_log_level) { \
			printk("%s[%u]:"#lev":"fmt, \
				 __FUNCTION__, __LINE__, ##__VA_ARGS__); \
		} \
	} while(0)
#define SSDK_DUMP_BUF(lev, buf, len) \
	do {\
		if (SSDK_LOG_LEVEL_##lev <= ssdk_log_level) {\
			a_uint32_t i_buf = 0;\
			for(i_buf=0; i_buf<(len); i_buf++) {\
				printk(KERN_CONT "%08lx ", *((buf)+i_buf));\
			}\
			printk(KERN_CONT "\n");\
		}\
	} while(0)

#define SSDK_ERROR(fmt, ...) __SSDK_LOG_FUN(ERROR, fmt, ##__VA_ARGS__)
#define SSDK_WARN(fmt, ...)  __SSDK_LOG_FUN(WARN, fmt, ##__VA_ARGS__)
#define SSDK_INFO(fmt, ...)  __SSDK_LOG_FUN(INFO, fmt, ##__VA_ARGS__)
#define SSDK_DEBUG(fmt, ...) __SSDK_LOG_FUN(DEBUG, fmt, ##__VA_ARGS__)

typedef enum {
	FDB_SYNC_DIS = 0,
	FDB_SYNC_INTR,
	FDB_SYNC_POLLING,
} fdb_sync_t;

struct qca_phy_priv {
	struct phy_device *phy;
#if defined(IN_SWCONFIG)
	struct switch_dev sw_dev;
#endif
    a_uint8_t version;
	a_uint8_t revision;
	a_uint32_t (*mii_read)(a_uint32_t dev_id, a_uint32_t reg);
	void (*mii_write)(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t val);
    void (*phy_dbg_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                        a_uint16_t dbg_addr, a_uint16_t dbg_data);
	void (*phy_dbg_read)(a_uint32_t dev_id, a_uint32_t phy_addr,
                        a_uint16_t dbg_addr, a_uint16_t *dbg_data);
    void (*phy_mmd_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                          a_uint16_t addr, a_uint16_t data);
    sw_error_t (*phy_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data);
    sw_error_t (*phy_read)(a_uint32_t dev_id, a_uint32_t phy_addr,
                           a_uint32_t reg, a_uint16_t* data);

	bool init;
/*qca808x_end*/
	a_bool_t qca_ssdk_sw_dev_registered;
	a_bool_t ess_switch_flag;
	struct mutex reg_mutex;
	struct mutex mib_lock;
	struct delayed_work mib_dwork;
	/*qm_err_check*/
	struct mutex 	qm_lock;
	a_uint32_t port_link_down[SW_MAX_NR_PORT];
	a_uint32_t port_link_up[SW_MAX_NR_PORT];
	a_uint32_t port_old_link[SW_MAX_NR_PORT];
	a_uint32_t port_old_speed[SW_MAX_NR_PORT];
	a_uint32_t port_old_duplex[SW_MAX_NR_PORT];
	a_uint32_t port_old_phy_status[SW_MAX_NR_PORT];
	a_uint32_t port_qm_buf[SW_MAX_NR_PORT];
	a_bool_t port_old_tx_flowctrl[SW_MAX_NR_PORT];
	a_bool_t port_old_rx_flowctrl[SW_MAX_NR_PORT];
	a_bool_t port_tx_flowctrl_forcemode[SW_MAX_NR_PORT];
	a_bool_t port_rx_flowctrl_forcemode[SW_MAX_NR_PORT];
	struct delayed_work qm_dwork_polling;
	struct work_struct	 intr_workqueue;
	/*qm_err_check end*/
/*qca808x_start*/
	a_uint8_t device_id;
	struct device_node *of_node;
/*qca808x_end*/
	/*dess_rgmii_mac*/
	struct mutex rgmii_lock;
	struct delayed_work rgmii_dwork;
	/*dess_rgmii_mac end*/
	/*hppe_mac_sw_sync*/
	struct mutex mac_sw_sync_lock;
	struct delayed_work mac_sw_sync_dwork;
	/*hppe_mac_sw_sync end*/
	/*hppe_fdb_sw_sync*/
	aos_lock_t fdb_sw_sync_lock;
	struct delayed_work fdb_sw_sync_dwork;
	fal_pbmp_t fdb_sw_sync_port_map;
	struct list_head sw_fdb_tbl;
	a_bool_t fdb_polling_started;
	/*hppe_fdb_sw_sync end*/
/*qca808x_start*/
	struct mii_bus *miibus[SSDK_MII_BUS_MAX];
/*qca808x_end*/
	u64 *mib_counters;
	a_uint32_t mib_loop_cnt;
	/* dump buf */
	a_uint8_t  buf[2048];
	a_uint32_t link_polling_required;
	fdb_sync_t fdb_sync;
	a_uint32_t interrupt_no;
	a_uint32_t interrupt_flag;
	char intr_name[IFNAMSIZ];
	/* VLAN database */
	bool       vlan;  /* True: 1q vlan mode, False: port vlan mode */
	a_uint16_t vlan_id[AR8327_MAX_VLANS];
	a_uint8_t  vlan_table[AR8327_MAX_VLANS];
	a_uint8_t  vlan_tagged[AR8327_MAX_VLANS];
	a_uint16_t pvid[SSDK_MAX_PORT_NUM];
	a_uint32_t ports;
	u8 __iomem *hw_addr;
	u8 __iomem *psgmii_hw_addr;
	u8 __iomem *uniphy_hw_addr;
	/*rx_los_pin, tx_dis_pin and mod_present_pin are for SFP port*/
	a_uint32_t sfp_rx_los_pin[SW_MAX_NR_PORT];
	a_uint32_t sfp_tx_dis_pin[SW_MAX_NR_PORT];
	a_uint32_t sfp_mod_present_pin[SW_MAX_NR_PORT];
	/*sfp_medium_pin, use to select sfp medium or not in combo mode*/
	a_uint32_t sfp_medium_pin[SW_MAX_NR_PORT];
	a_uint32_t uniphy_clk_output[SSDK_UNIPHY_INSTANCE2];
/*qca808x_start*/
};

#define SSDK_SWITCH_REG_TYPE_MASK		GENMASK(31, 28)
#define SSDK_SWITCH_REG_TYPE_QCA8337		FIELD_PREP(SSDK_SWITCH_REG_TYPE_MASK, 1)
#define SSDK_SWITCH_REG_TYPE_QCA8386		FIELD_PREP(SSDK_SWITCH_REG_TYPE_MASK, 0)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
#define ETH_LDO_RDY_CNT		3
struct qca_mdio_data{
	void __iomem	*membase[2];
	void __iomem *eth_ldo_rdy[ETH_LDO_RDY_CNT];
	int clk_div;
	bool force_c22;
	struct gpio_descs *reset_gpios;
	void (*preinit)(struct mii_bus *bus);
	u32 (*sw_read)(struct mii_bus *bus, u32 reg);
	void (*sw_write)(struct mii_bus *bus, u32 reg, u32 val);
	struct clk *clk[];
};
#else
struct qca_mdio_data {
	struct mii_bus *mii_bus;
	struct clk *mdio_clk;
	void __iomem *membase;
	int phy_irq[PHY_MAX_ADDR];
	int clk_div;
	bool force_c22;
	void (*preinit)(struct mii_bus *bus);
	u32 (*sw_read)(struct mii_bus *bus, u32 reg);
	void (*sw_write)(struct mii_bus *bus, u32 reg, u32 val);
};
#endif

#if defined(IN_SWCONFIG)
#define qca_phy_priv_get(_dev) \
		container_of(_dev, struct qca_phy_priv, sw_dev)
#endif

/*qca808x_end*/

sw_error_t
qca_mii_bus_lock(a_uint32_t dev_id, a_bool_t enable);

sw_error_t
__qca_mii_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint8_t value[], a_uint32_t value_len);
sw_error_t
__qca_mii_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
                   a_uint32_t value_len);
sw_error_t
__qca_mii_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                    a_uint32_t bit_offset, a_uint32_t field_len,
                    a_uint8_t value[], a_uint32_t value_len);
sw_error_t
__qca_mii_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   const a_uint8_t value[], a_uint32_t value_len);
sw_error_t
qca_mii_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint8_t value[], a_uint32_t value_len);
sw_error_t
qca_mii_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
                   a_uint32_t value_len);
sw_error_t
qca_mii_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   a_uint8_t value[], a_uint32_t value_len);
sw_error_t
qca_mii_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   const a_uint8_t value[], a_uint32_t value_len);

#define MHT_REG_FIELD_GET(rv, dev, reg, index, field, value, val_len) \
	rv = qca_mii_field_get(dev, reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
	reg##_##field##_BOFFSET, \
	reg##_##field##_BLEN, value, val_len);

#define MHT_REG_FIELD_SET(rv, dev, reg, index, field, value, val_len) \
		rv = qca_mii_field_set(dev, reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
		reg##_##field##_BOFFSET, \
		reg##_##field##_BLEN, value, val_len);
sw_error_t qca_mii_raw_read(struct mii_bus *bus, a_uint32_t reg, a_uint32_t *val);
sw_error_t qca_mii_raw_write(struct mii_bus *bus, a_uint32_t reg, a_uint32_t val);
sw_error_t qca_mii_raw_update(struct mii_bus *bus, a_uint32_t reg,
		a_uint32_t clear, a_uint32_t set);
a_uint32_t qca_mii_read(a_uint32_t dev_id, a_uint32_t reg);
void qca_mii_write(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t val);
int qca_mii_update(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t mask, a_uint32_t val);

a_uint32_t __qca_mii_read(a_uint32_t dev_id, a_uint32_t reg);
void __qca_mii_write(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t val);
int __qca_mii_update(a_uint32_t dev_id, a_uint32_t reg, a_uint32_t mask, a_uint32_t val);

sw_error_t
qca_switch_reg_read(a_uint32_t dev_id, a_uint32_t reg_addr,
			a_uint8_t * reg_data, a_uint32_t len);

sw_error_t
qca_switch_reg_write(a_uint32_t dev_id, a_uint32_t reg_addr,
			a_uint8_t * reg_data, a_uint32_t len);

sw_error_t
qca_psgmii_reg_read(a_uint32_t dev_id, a_uint32_t reg_addr,
			a_uint8_t * reg_data, a_uint32_t len);

sw_error_t
qca_psgmii_reg_write(a_uint32_t dev_id, a_uint32_t reg_addr,
			a_uint8_t * reg_data, a_uint32_t len);

sw_error_t
qca_uniphy_reg_write(a_uint32_t dev_id, a_uint32_t uniphy_index,
				a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len);

sw_error_t
qca_uniphy_reg_read(a_uint32_t dev_id, a_uint32_t uniphy_index,
				a_uint32_t reg_addr, a_uint8_t * reg_data, a_uint32_t len);
/*qca808x_start*/
struct mii_bus *ssdk_miibus_get(a_uint32_t dev_id, a_uint32_t index);
struct mii_bus *ssdk_port_miibus_get(a_uint32_t dev_id, a_uint32_t port_id);
struct mii_bus *ssdk_phy_miibus_get(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t ssdk_miibus_add(a_uint32_t dev_id, struct mii_bus *miibus, a_uint32_t *index);
a_uint32_t ssdk_miibus_index_get(a_uint32_t dev_id, struct mii_bus *miibus);
/*qca808x_end*/
sw_error_t ssdk_miibus_freq_set(a_uint32_t dev_id, a_uint32_t index, a_uint32_t freq);
sw_error_t ssdk_miibus_freq_get(a_uint32_t dev_id, a_uint32_t index, a_uint32_t *freq);

int ssdk_sysfs_init (void);
void ssdk_sysfs_exit (void);
int ssdk_uniphy_valid_check(a_uint32_t dev_id, a_uint32_t index, a_uint32_t mode);
/*qca808x_start*/
int ssdk_plat_init(ssdk_init_cfg *cfg, a_uint32_t dev_id);
void ssdk_plat_exit(a_uint32_t dev_id);
#define qca_mht_mii_read qca_mii_read
#define qca_mht_mii_write qca_mii_write
#define qca_mht_mii_update qca_mii_update
#endif
/*qca808x_end*/
