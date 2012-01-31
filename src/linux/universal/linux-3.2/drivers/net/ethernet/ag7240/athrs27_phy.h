/*
 * Copyright (c) 2008, Atheros Communications Inc.
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

#ifndef _ATHRS27_PHY_H
#define _ATHRS27_PHY_H


/*****************/
/* PHY Registers */
/*****************/
#define S27_PHY_CONTROL                            0
#define S27_PHY_STATUS                             1
#define S27_PHY_ID1                                2
#define S27_PHY_ID2                                3
#define S27_AUTONEG_ADVERT                         4
#define S27_LINK_PARTNER_ABILITY                   5
#define S27_AUTONEG_EXPANSION                      6
#define S27_NEXT_PAGE_TRANSMIT                     7
#define S27_LINK_PARTNER_NEXT_PAGE                 8
#define S27_1000BASET_CONTROL                      9
#define S27_1000BASET_STATUS                       10
#define S27_MMD_CTRL_REG			   13
#define S27_MMD_DATA_REG			   14
#define S27_PHY_FUNC_CONTROL                       16
#define S27_PHY_SPEC_STATUS                        17
#define S27_PHY_INTR_ENABLE                        18
#define S27_PHY_INTR_STATUS                        19
#define S27_DEBUG_PORT_ADDRESS                     29
#define S27_DEBUG_PORT_DATA                        30

/* ATHR_PHY_CONTROL fields */
#define S27_CTRL_SOFTWARE_RESET                    0x8000
#define S27_CTRL_SPEED_LSB                         0x2000
#define S27_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define S27_CTRL_RESTART_AUTONEGOTIATION           0x0200
#define S27_CTRL_SPEED_FULL_DUPLEX                 0x0100
#define S27_CTRL_SPEED_MSB                         0x0040

#define S27_RESET_DONE(phy_control)                   \
    (((phy_control) & (S27_CTRL_SOFTWARE_RESET)) == 0)
    
/* Phy status fields */
#define S27_STATUS_AUTO_NEG_DONE                   0x0020

#define S27_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (S27_STATUS_AUTO_NEG_DONE)) ==                    \
        (S27_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define S27_LINK_100BASETX_FULL_DUPLEX             0x0100
#define S27_LINK_100BASETX                         0x0080
#define S27_LINK_10BASETX_FULL_DUPLEX              0x0040
#define S27_LINK_10BASETX                          0x0020

/* Advertisement register. */
#define S27_ADVERTISE_NEXT_PAGE                    0x8000
#define S27_ADVERTISE_ASYM_PAUSE                   0x0800
#define S27_ADVERTISE_PAUSE                        0x0400
#define S27_ADVERTISE_100FULL                      0x0100
#define S27_ADVERTISE_100HALF                      0x0080  
#define S27_ADVERTISE_10FULL                       0x0040  
#define S27_ADVERTISE_10HALF                       0x0020  

#define S27_ADVERTISE_ALL (S27_ADVERTISE_ASYM_PAUSE | S27_ADVERTISE_PAUSE | \
                            S27_ADVERTISE_10HALF | S27_ADVERTISE_10FULL | \
                            S27_ADVERTISE_100HALF | S27_ADVERTISE_100FULL)
                       
/* 1000BASET_CONTROL */
#define S27_ADVERTISE_1000FULL                   0x0200
#define S27_ADVERTISE_1000HALF		         0x0100

/* Phy Specific status fields */
#define S27_STATUS_LINK_MASK                     0xC000
#define S27_STATUS_LINK_SHIFT                    14
#define S27_STATUS_FULL_DUPLEX                   (1 << 13)
#define S27_STATUS_LINK_PASS                     (1 << 10)
#define S27_STATUS_RESOVLED                      (1 << 11)
#define S27_STATUS_LINK_10M			 0
#define S27_STATUS_LINK_100M			 1
#define S27_STATUS_LINK_1000M			 2


/* S27 CSR Registers */
#define S27_MASK_CTL_REG		         0x0000
#define S27_OPMODE_REG0                          0x0004
#define S27_OPMODE_REG1                          0x0008
#define S27_OPMODE_REG2                          0x000C
#define S27_PWRSTRAP_REG                         0x0010
#define S27_GLOBAL_INTR_REG                      0x0014
#define S27_GLOBAL_INTR_MASK_REG                 0x0018
#define S27_FLD_MASK_REG                         0x002c
#define S27_FLCTL_REG0				 0x0034
#define S27_FLCTL_REG1				 0x0038
#define S27_ARL_TBL_FUNC_REG0                    0x0050
#define S27_ARL_TBL_FUNC_REG1                    0x0054
#define S27_ARL_TBL_FUNC_REG2                    0x0058
#define S27_ARL_TBL_CTRL_REG                     0x005c
#define S27_TAGPRI_REG                           0x0070
#define S27_CPU_PORT_REGISTER                    0x0078
#define S27_MDIO_CTRL_REGISTER                   0x0098

#define S27_PORT_STATUS_REGISTER0                0x0100 
#define S27_PORT_STATUS_REGISTER1                0x0200
#define S27_PORT_STATUS_REGISTER2                0x0300
#define S27_PORT_STATUS_REGISTER3                0x0400
#define S27_PORT_STATUS_REGISTER4                0x0500
#define S27_PORT_STATUS_REGISTER5                0x0600

#define S27_PORT_CONTROL_REGISTER0               0x0104
#define S27_PORT_CONTROL_REGISTER1               0x0204
#define S27_PORT_CONTROL_REGISTER2               0x0304
#define S27_PORT_CONTROL_REGISTER3               0x0404
#define S27_PORT_CONTROL_REGISTER4               0x0504
#define S27_PORT_CONTROL_REGISTER5               0x0604

#define S27_PORT_BASE_VLAN_REGISTER0             0x0108 
#define S27_PORT_BASE_VLAN_REGISTER1             0x0208
#define S27_PORT_BASE_VLAN_REGISTER2             0x0308
#define S27_PORT_BASE_VLAN_REGISTER3             0x0408
#define S27_PORT_BASE_VLAN_REGISTER4             0x0508
#define S27_PORT_BASE_VLAN_REGISTER5             0x0608

#define S27_RATE_LIMIT_REGISTER0                 0x010C
#define S27_RATE_LIMIT_REGISTER1                 0x020C
#define S27_RATE_LIMIT_REGISTER2                 0x030C
#define S27_RATE_LIMIT_REGISTER3                 0x040C
#define S27_RATE_LIMIT_REGISTER4                 0x050C
#define S27_RATE_LIMIT_REGISTER5                 0x060C

#define S27_RATE_LIMIT1_REGISTER0                0x011c
#define S27_RATE_LIMIT2_REGISTER0                0x0120

/* SWITCH QOS REGISTERS */
#define S27_CPU_PORT                		 0x0078
#define S27_IP_PRI_MAP0             		 0x0060
#define S27_IP_PRI_MAP1             		 0x0064
#define S27_IP_PRI_MAP2             		 0x0068

#define S27_PRI_CTRL_PORT_0             	 0x110 /* CPU PORT */
#define S27_PRI_CTRL_PORT_1             	 0x210
#define S27_PRI_CTRL_PORT_2             	 0x310
#define S27_PRI_CTRL_PORT_3             	 0x410
#define S27_PRI_CTRL_PORT_4             	 0x510
#define S27_PRI_CTRL_PORT_5             	 0x610

#define S27_QOS_PORT_0				 0x110 /* CPU PORT */
#define S27_QOS_PORT_1				 0x210
#define S27_QOS_PORT_2				 0x310
#define S27_QOS_PORT_3				 0x410
#define S27_QOS_PORT_4				 0x510

#define S27_LPI_CTRL_PORT_1			 0x230 /* phy 0 */
#define S27_LPI_CTRL_PORT_2			 0x330 /* phy 1 */
#define S27_LPI_CTRL_PORT_3			 0x430 /* phy 2 */
#define S27_LPI_CTRL_PORT_4			 0x530 /* phy 3 */

/* enable broadcast on CPU port (port 0) */
#define S27_ENABLE_CPU_BCAST_FWD                 (1 << 25)

/* For GLOBAL INTs */
#define S27_LINK_CHANGE_REG 		         0x4
#define S27_LINK_UP 		                 0x400
#define S27_LINK_DOWN 		                 0x800
#define S27_LINK_DUPLEX_CHANGE 		         0x2000
#define S27_LINK_SPEED_CHANGE		         0x4000
#define S27_LINK_INTRS			         (S27_LINK_UP | S27_LINK_DOWN | \
					          S27_LINK_DUPLEX_CHANGE | S27_LINK_SPEED_CHANGE)

/* For OPMODE_REGs */
/* in OPMODE_REG0 */
#define S27_MAC0_MAC_GMII_EN                     (1 << 6) 
/* in OPMODE_REG1 */
#define S27_PHY4_RMII_EN                         (1 << 29)
#define S27_PHY4_MII_EN                          (1 << 28)
#define S27_MAC5_RGMII_EN                        (1 << 26) 

/* For PORT_STATUS_REGISTERs */
#define S27_FLOW_LINK_EN                         (1 << 12)
#define S27_LINK_EN                              (1 << 9)
#define S27_TXH_FLOW_EN                          (1 << 7)
#define S27_PORT_STATUS_DEFAULT                  (S27_FLOW_LINK_EN | S27_LINK_EN | S27_TXH_FLOW_EN)

/* For PORT_CONTROL_REGISTERs */
#define S27_EAPOL_EN                             (1 << 23)
#define S27_ARP_LEAKY_EN                         (1 << 22)
#define S27_IGMP_LEAVE_EN                        (1 << 21)
#define S27_IGMP_JOIN_EN                         (1 << 20)
#define S27_DHCP_EN                              (1 << 19)
#define S27_IPG_DEC_EN                           (1 << 18)
#define S27_ING_MIRROR_EN                        (1 << 17)
#define S27_EG_MIRROT_EN                         (1 << 16)
#define S27_LEARN_EN                             (1 << 14)
#define S27_MAC_LOOP_BACK                        (1 << 12)
#define S27_HEADER_EN              	         (1 << 11)
#define S27_IGMP_MLD_EN                          (1 << 10)
#define S27_LEARN_ONE_LOCK             	         (1 << 7)
#define S27_PORT_LOCK_EN                         (1 << 6)
#define S27_PORT_DROP_EN                         (1 << 5)
#define S27_PORT_MODE_FWD                        0x4

/* For MDIO_CONTROL */
#define S27_MDIO_BUSY                            (1 << 31)
#define S27_MDIO_MASTER                          (1 << 30)
#define S27_MDIO_CMD_RD                          (1 << 27)
#define S27_MDIO_CMD_WR                          (0 << 27)
#define S27_MDIO_SUP_PRE                         (1 << 26)
#define S27_MDIO_PHY_ADDR                        21
#define S27_MDIO_REG_ADDR                        16

/* For MMD register control */
#define S27_MMD_FUNC_ADDR			(0 << 14)
#define S27_MMD_FUNC_DATA			(1 << 14)
#define S27_MMD_FUNC_DATA_2			(2 << 14)
#define S27_MMD_FUNC_DATA_3			(3 << 14)

/* For 802.3az (LPI) control */
#define S27_LPI_ENABLED			        (1 << 31)
#define S27_LPI_WAKEUP_TIMER			0x20

/* For phyInfo_t azFeature */
#define S27_8023AZ_PHY_ENABLED			(1 << 0)
#define S27_8023AZ_PHY_LINKED                   (1 << 1)

/* For Tag priority */
#define S27_TAGPRI_DEFAULT                       0xFA50

#define S27_QOS_MODE_REGISTER                     0x030
#define S27_QOS_FIXED_PRIORITY                   ((0 << 31) | (0 << 28))
#define S27_QOS_WEIGHTED                         ((1 << 31) | (0 << 28)) /* Fixed weight 8,4,2,1 */
#define S27_QOS_MIXED                            ((1 << 31) | (1 << 28)) /* Q3 for managment; Q2,Q1,Q0 - 4,2,1 */

#ifndef BOOL
#define BOOL    int
#endif
/* enabled the 802.3az feature */
#define S27_8023AZ_FEATURE

int athrs27_reg_init(int arg);
int athrs27_reg_init_lan(int arg);
int athrs27_phy_is_up(int unit);
int athrs27_phy_is_fdx(int unit,int phyUnit);
int athrs27_phy_speed(int unit,int phyUnit);
void athrs27_phy_stab_wr(int phy_id, BOOL phy_up, int phy_speed);
int athrs27_phy_setup(int arg);
int athrs27_mdc_check(void);
void athrs27_phy_off(int arg);
void athrs27_phy_on(int arg);
void athrs27_mac_speed_set(int arg, int phy_speed);
int athrs27_register_ops(int arg);
void athrs27_enable_linkintrs(int arg);
void athrs27_disable_linkintrs(int arg);
int athrs27_phy_is_link_alive(int phyUnit);
void athrs27_phy_stab_wr(int phy_id, int phy_up, int phy_speed);
unsigned int athrs27_reg_read(unsigned int s27_addr);
void athrs27_reg_write(unsigned int s27_addr, unsigned int s27_write_data);
void s27_wr_phy(unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data);
unsigned int s27_rd_phy(unsigned int phy_addr, unsigned int reg_addr);
int athrs27_ioctl(struct net_device *dev,void *args, int cmd);
void athrs27_reg_rmw(unsigned int s27_addr, unsigned int s27_write_data); 
void athrs27_reg_dev(ag7240_mac_t **mac);

/*
 *  Atheros header defines
 */
#ifndef _ATH_HEADER_CONF
#define _ATH_HEADER_CONF

typedef enum {
    NORMAL_PACKET,
    RESERVED0,
    MIB_1ST,
    RESERVED1,
    RESERVED2,
    READ_WRITE_REG,
    READ_WRITE_REG_ACK,
    RESERVED3
} AT_HEADER_TYPE;

typedef struct {
    uint16_t    reserved0  :2;
    uint16_t    priority   :2;
    uint16_t    type       :4;
    uint16_t    broadcast  :1;
    uint16_t    from_cpu   :1;
    uint16_t    reserved1  :2;
    uint16_t    port_num   :4;
}at_header_t;

#define ATHR_HEADER_LEN 2
#endif // _ATH_HEADER_CONF

#endif
