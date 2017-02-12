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

#ifndef __SSDK_PLAT_H
#define __SSDK_PLAT_H

#ifndef BIT
#define BIT(_n)			(1UL << (_n))
#endif


#ifndef BITS
#define BITS(_s, _n)	(((1UL << (_n)) - 1) << _s)
#endif

/* Atheros specific MII registers */
#define QCA_MII_MMD_ADDR		0x0d
#define QCA_MII_MMD_DATA		0x0e
#define QCA_MII_DBG_ADDR		0x1d
#define QCA_MII_DBG_DATA		0x1e

#define AR8327_REG_CTRL			0x0000
#define   AR8327_CTRL_REVISION		BITS(0, 8)
#define   AR8327_CTRL_REVISION_S	0
#define   AR8327_CTRL_VERSION		BITS(8, 8)
#define   AR8327_CTRL_VERSION_S		8
#define   AR8327_CTRL_RESET		    BIT(31)

#define AR8327_REG_LED_CTRL_0	0x50
#define AR8327_REG_LED_CTRL_1	0x54
#define AR8327_REG_LED_CTRL_2	0x58
#define AR8327_REG_LED_CTRL_3	0x5c

#define AR8327_REG_PORT_STATUS(_i)  (0x07c + (_i) * 4)

#define   AR8327_PORT_STATUS_SPEED	    BITS(0,2)
#define   AR8327_PORT_STATUS_SPEED_S	0
#define   AR8327_PORT_STATUS_TXMAC	    BIT(2)
#define   AR8327_PORT_STATUS_RXMAC	    BIT(3)
#define   AR8327_PORT_STATUS_TXFLOW	    BIT(4)
#define   AR8327_PORT_STATUS_RXFLOW	    BIT(5)
#define   AR8327_PORT_STATUS_DUPLEX	    BIT(6)
#define   AR8327_PORT_STATUS_LINK_UP	BIT(8)
#define   AR8327_PORT_STATUS_LINK_AUTO	BIT(9)
#define   AR8327_PORT_STATUS_LINK_PAUSE	BIT(10)

#define AR8327_REG_PAD0_CTRL			0x4
#define AR8327_REG_PAD5_CTRL			0x8
#define AR8327_REG_PAD6_CTRL			0xc
#define   AR8327_PAD_CTRL_MAC_MII_RXCLK_SEL		BIT(0)
#define   AR8327_PAD_CTRL_MAC_MII_TXCLK_SEL		BIT(1)
#define   AR8327_PAD_CTRL_MAC_MII_EN			BIT(2)
#define   AR8327_PAD_CTRL_MAC_GMII_RXCLK_SEL	BIT(4)
#define   AR8327_PAD_CTRL_MAC_GMII_TXCLK_SEL	BIT(5)
#define   AR8327_PAD_CTRL_MAC_GMII_EN		    BIT(6)
#define   AR8327_PAD_CTRL_SGMII_EN			    BIT(7)
#define   AR8327_PAD_CTRL_PHY_MII_RXCLK_SEL		BIT(8)
#define   AR8327_PAD_CTRL_PHY_MII_TXCLK_SEL		BIT(9)
#define   AR8327_PAD_CTRL_PHY_MII_EN			BIT(10)
#define   AR8327_PAD_CTRL_PHY_GMII_PIPE_RXCLK_SEL	BIT(11)
#define   AR8327_PAD_CTRL_PHY_GMII_RXCLK_SEL	BIT(12)
#define   AR8327_PAD_CTRL_PHY_GMII_TXCLK_SEL	BIT(13)
#define   AR8327_PAD_CTRL_PHY_GMII_EN		    BIT(14)
#define   AR8327_PAD_CTRL_PHYX_GMII_EN		    BIT(16)
#define   AR8327_PAD_CTRL_PHYX_RGMII_EN		    BIT(17)
#define   AR8327_PAD_CTRL_PHYX_MII_EN		    BIT(18)
#define   AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL	BITS(20, 2)
#define   AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_SEL_S	20
#define   AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL	BITS(22, 2)
#define   AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_SEL_S	22
#define   AR8327_PAD_CTRL_RGMII_RXCLK_DELAY_EN	BIT(24)
#define   AR8327_PAD_CTRL_RGMII_TXCLK_DELAY_EN	BIT(25)
#define   AR8327_PAD_CTRL_RGMII_EN			    BIT(26)

#define AR8327_PORT5_PHY_ADDR    4
/*AR8327 inner phy debug register for RGMII mode*/
#define AR8327_PHY_REG_MODE_SEL  0x12
#define AR8327_PHY_RGMII_MODE    BIT(3)
#define AR8327_PHY_REG_TEST_CTRL  0x0
#define AR8327_PHY_RGMII_RX_DELAY    BIT(15)
#define AR8327_PHY_REG_SYS_CTRL  0x5
#define AR8327_PHY_RGMII_TX_DELAY    BIT(8)


#define AR8327_REG_POS		0x10
#define AR8327_REG_POS_HW_INIT 0x261320
#define   AR8327_POS_POWER_ON_SEL	BIT(31)
#define   AR8327_POS_LED_OPEN_EN	BIT(24)
#define   AR8327_POS_SERDES_AEN	    BIT(7)

#define AR8327_REG_MODULE_EN  0x30
#define   AR8327_REG_MODULE_EN_QM_ERR	BIT(8)


#define AR8327_REG_PAD_SGMII_CTRL			0xe0
#define AR8327_REG_PAD_SGMII_CTRL_HW_INIT   0xc70164c0
#define   AR8327_PAD_SGMII_CTRL_MODE_CTRL	BITS(22, 2)
#define   AR8327_PAD_SGMII_CTRL_MODE_CTRL_S		22
#define   AR8327_PAD_SGMII_CTRL_EN_SD		BIT(4)
#define   AR8327_PAD_SGMII_CTRL_EN_TX		BIT(3)
#define   AR8327_PAD_SGMII_CTRL_EN_RX		BIT(2)
#define   AR8327_PAD_SGMII_CTRL_EN_PLL		BIT(1)
#define   AR8327_PAD_SGMII_CTRL_EN_LCKDT	BIT(0)

#define AR8327_REG_PAD_MAC_PWR_SEL			0x0e4
#define   AR8327_PAD_MAC_PWR_RGMII0_1_8V		BIT(18)
#define   AR8327_PAD_MAC_PWR_RGMII1_1_8V		BIT(19)

#define AR8327_REG_PORT_LOOKUP(_i) (0x660 + (_i) * 0xc)
#define AR8327_REG_PORT_VLAN0(_i) (0x420 + (_i) * 0x8)

#define DESS_PSGMII_MODE_CONTROL	0x1b4
#define   DESS_PSGMII_ATHR_CSCO_MODE_25M	BIT(0)

#define DESS_PSGMIIPHY_TX_CONTROL	 0x288

#define DESS_PSGMII_PLL_VCO_RELATED_CONTROL_1	0x78c
#define   DESS_PSGMII_MII_REG_UPHY_PLL_LCKDT_EN	BIT(0)

#define DESS_PSGMII_VCO_CALIBRATION_CONTROL_1  0x9c

#define SSDK_PSGMII_ID 5
#define SSDK_PHY_BCAST_ID 0x1f
#define SSDK_PHY_MIN_ID 0x0

#define SSDK_PORT0_FC_THRESH_ON_DFLT	0x60
#define SSDK_PORT0_FC_THRESH_OFF_DFLT	0x90

#define AR8327_NUM_CHIPS        2
#define AR8327_NUM_PHYS		5
#define AR8327_PORT_CPU     0
#define AR8327_NUM_PORTS	7
#define AR8327_MAX_VLANS  128

enum {
    AR8327_PORT_SPEED_10M = 0,
    AR8327_PORT_SPEED_100M = 1,
    AR8327_PORT_SPEED_1000M = 2,
    AR8327_PORT_SPEED_NONE = 3,
};

enum {
	QCA_VER_AR8216 = 0x01,
	QCA_VER_AR8227 = 0x02,
	QCA_VER_AR8236 = 0x03,
	QCA_VER_AR8316 = 0x10,
	QCA_VER_AR8327 = 0x12,
	QCA_VER_AR8337 = 0x13
};

/*poll mib per 120secs*/
#define QCA_PHY_MIB_WORK_DELAY	120000
#define QCA_MIB_ITEM_NUMBER	41

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


struct qca_phy_priv {
	struct phy_device *phy;
	struct switch_dev sw_dev;
    a_uint8_t version;
	a_uint8_t revision;
	a_uint32_t (*mii_read)(a_uint32_t reg);
	void (*mii_write)(a_uint32_t reg, a_uint32_t val);
    void (*phy_dbg_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                        a_uint16_t dbg_addr, a_uint16_t dbg_data);
	void (*phy_dbg_read)(a_uint32_t dev_id, a_uint32_t phy_addr,
                        a_uint16_t dbg_addr, a_uint16_t *dbg_data);
    void (*phy_mmd_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                          a_uint16_t addr, a_uint16_t data);
    void (*phy_write)(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data);

	bool init;
	struct mutex reg_mutex;
	struct mutex mib_lock;
	struct delayed_work mib_dwork;
	/*qm_err_check*/
	struct mutex 	qm_lock;
	struct delayed_work qm_dwork;
	/*qm_err_check end*/
	/*dess_rgmii_mac*/
	struct mutex rgmii_lock;
	struct delayed_work rgmii_dwork;
	/*dess_rgmii_mac end*/
	u64 *mib_counters;
	/* dump buf */
	a_uint8_t  buf[2048];

    /* VLAN database */
    bool       vlan;  /* True: 1q vlan mode, False: port vlan mode */
    a_uint16_t vlan_id[AR8327_MAX_VLANS];
    a_uint8_t  vlan_table[AR8327_MAX_VLANS];
    a_uint8_t  vlan_tagged[AR8327_MAX_VLANS];
    a_uint16_t pvid[AR8327_NUM_PORTS];

};

struct ipq40xx_mdio_data {
        struct mii_bus          *mii_bus;
        void __iomem            *membase;
        int phy_irq[PHY_MAX_ADDR];
};

#define qca_phy_priv_get(_dev) \
		container_of(_dev, struct qca_phy_priv, sw_dev)


uint32_t
qca_ar8216_mii_read(int reg);
void
qca_ar8216_mii_write(int reg, uint32_t val);
uint32_t
qca_ar8216_mii_slave_read(int reg);
void
qca_ar8216_mii_slave_write(int reg, uint32_t val);
sw_error_t
qca_ar8327_phy_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data);
void
qca_ar8327_mmd_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                              a_uint16_t addr, a_uint16_t data);
void
qca_ar8327_phy_dbg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
		                          a_uint16_t dbg_addr, a_uint16_t dbg_data);
void
qca_ar8327_phy_dbg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
		                          a_uint16_t dbg_addr, a_uint16_t *dbg_data);

void 
qca_phy_mmd_write(u32 dev_id, u32 phy_id,
                     u16 mmd_num, u16 reg_id, u16 reg_val);

u16 
qca_phy_mmd_read(u32 dev_id, u32 phy_id,
		u16 mmd_num, u16 reg_id);

#endif
