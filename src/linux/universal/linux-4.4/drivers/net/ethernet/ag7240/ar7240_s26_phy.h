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

#ifndef _ATHRS26_PHY_H
#define _ATHRS26_PHY_H


/*****************/
/* PHY Registers */
/*****************/
#define ATHR_PHY_CONTROL                 0
#define ATHR_PHY_STATUS                  1
#define ATHR_PHY_ID1                     2
#define ATHR_PHY_ID2                     3
#define ATHR_AUTONEG_ADVERT              4
#define ATHR_LINK_PARTNER_ABILITY        5
#define ATHR_AUTONEG_EXPANSION           6
#define ATHR_NEXT_PAGE_TRANSMIT          7
#define ATHR_LINK_PARTNER_NEXT_PAGE      8
#define ATHR_1000BASET_CONTROL           9
#define ATHR_1000BASET_STATUS            10
#define ATHR_PHY_FUNC_CONTROL            16
#define ATHR_PHY_SPEC_STATUS             17
#define ATHR_DEBUG_PORT_ADDRESS          29
#define ATHR_DEBUG_PORT_DATA             30
#define ATHR_PHY_INTR_ENABLE             0x12
#define ATHR_PHY_INTR_STATUS             0x13

/* ATHR_PHY_CONTROL fields */
#define ATHR_CTRL_SOFTWARE_RESET                    0x8000
#define ATHR_CTRL_SPEED_LSB                         0x2000
#define ATHR_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define ATHR_CTRL_RESTART_AUTONEGOTIATION           0x0200
#define ATHR_CTRL_SPEED_FULL_DUPLEX                 0x0100
#define ATHR_CTRL_SPEED_MSB                         0x0040

#define BITS(_s, _n)	(((1UL << (_n)) - 1) << _s)
#define BIT(nr)			(1UL << (nr))
#define AR8216_REG_GLOBAL_CTRL		0x0030
#define AR8216_GCTRL_MTU		BITS(0, 11)


#define ATHR_RESET_DONE(phy_control)                   \
    (((phy_control) & (ATHR_CTRL_SOFTWARE_RESET)) == 0)
    
/* Phy status fields */
#define ATHR_STATUS_AUTO_NEG_DONE                   0x0020

#define ATHR_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (ATHR_STATUS_AUTO_NEG_DONE)) ==                    \
        (ATHR_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define ATHR_LINK_100BASETX_FULL_DUPLEX       0x0100
#define ATHR_LINK_100BASETX                   0x0080
#define ATHR_LINK_10BASETX_FULL_DUPLEX        0x0040
#define ATHR_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define ATHR_ADVERTISE_NEXT_PAGE              0x8000
#define ATHR_ADVERTISE_ASYM_PAUSE             0x0800
#define ATHR_ADVERTISE_PAUSE                  0x0400
#define ATHR_ADVERTISE_100FULL                0x0100
#define ATHR_ADVERTISE_100HALF                0x0080  
#define ATHR_ADVERTISE_10FULL                 0x0040  
#define ATHR_ADVERTISE_10HALF                 0x0020  

#define ATHR_ADVERTISE_ALL (ATHR_ADVERTISE_ASYM_PAUSE | ATHR_ADVERTISE_PAUSE | \
                            ATHR_ADVERTISE_10HALF | ATHR_ADVERTISE_10FULL | \
                            ATHR_ADVERTISE_100HALF | ATHR_ADVERTISE_100FULL)
                       
/* 1000BASET_CONTROL */
#define ATHR_ADVERTISE_1000FULL               0x0200

/* Phy Specific status fields */
#define ATHER_STATUS_LINK_MASK                0xC000
#define ATHER_STATUS_LINK_SHIFT               14
#define ATHER_STATUS_FULL_DEPLEX              0x2000
#define ATHR_STATUS_LINK_PASS                 0x0400 
#define ATHR_LATCH_LINK_PASS                  0x0004 
#define ATHR_STATUS_RESOVLED                  0x0800

/*phy debug port  register */
#define ATHER_DEBUG_SERDES_REG                5

/* Serdes debug fields */
#define ATHER_SERDES_BEACON                   0x0100

/* S26 CSR Registers */

#define PORT_STATUS_REGISTER0                0x0100 
#define PORT_STATUS_REGISTER1                0x0200
#define PORT_STATUS_REGISTER2                0x0300
#define PORT_STATUS_REGISTER3                0x0400
#define PORT_STATUS_REGISTER4                0x0500
#define PORT_STATUS_REGISTER5                0x0600

#define RATE_LIMIT_REGISTER0                 0x010C
#define RATE_LIMIT_REGISTER1                 0x020C
#define RATE_LIMIT_REGISTER2                 0x030C
#define RATE_LIMIT_REGISTER3                 0x040C
#define RATE_LIMIT_REGISTER4                 0x050C
#define RATE_LIMIT_REGISTER5                 0x060C

#define PORT_CONTROL_REGISTER0               0x0104
#define PORT_CONTROL_REGISTER1               0x0204
#define PORT_CONTROL_REGISTER2               0x0204
#define PORT_CONTROL_REGISTER3               0x0204
#define PORT_CONTROL_REGISTER4               0x0204
#define PORT_CONTROL_REGISTER5               0x0204

#define CPU_PORT_REGISTER                    0x0078
#define MDIO_CTRL_REGISTER                   0x0098

#define S26_ARL_TBL_FUNC_REG0                0x0050
#define S26_ARL_TBL_FUNC_REG1                0x0054
#define S26_ARL_TBL_FUNC_REG2                0x0058
#define S26_FLD_MASK_REG                     0x002c
#define S26_ARL_TBL_CTRL_REG                 0x005c
#define S26_GLOBAL_INTR_REG                  0x10
#define S26_GLOBAL_INTR_MASK_REG             0x14


#define S26_ENABLE_CPU_BROADCAST             (1 << 26)

#define PHY_LINK_CHANGE_REG 		     0x4
#define PHY_LINK_UP 		             0x400
#define PHY_LINK_DOWN 		             0x800
#define PHY_LINK_DUPLEX_CHANGE 		     0x2000
#define PHY_LINK_SPEED_CHANGE		     0x4000
#define PHY_LINK_INTRS			     (PHY_LINK_UP | PHY_LINK_DOWN | PHY_LINK_DUPLEX_CHANGE | PHY_LINK_SPEED_CHANGE)

/* SWITCH QOS REGISTERS */

#define ATHR_PRI_CTRL_PORT_0             0x110 /* CPU PORT */
#define ATHR_PRI_CTRL_PORT_1             0x210
#define ATHR_PRI_CTRL_PORT_2             0x310
#define ATHR_PRI_CTRL_PORT_3             0x410
#define ATHR_PRI_CTRL_PORT_4             0x510
#define ATHR_PRI_CTRL_PORT_5             0x610

#define ATHR_TOS_PRI_EN                      (1 << 16)
#define ATHR_VLAN_PRI_EN                     (1 << 17)
#define ATHR_DA_PRI_EN                       (1 << 18)
#define ATHR_PORT_PRI_EN                     (1 << 19)
#define ATHR_HDR_EN              (1 << 11)
#define ATHR_CPU_EN              (1 << 8 )


#define ATHR_QOS_MODE_REGISTER          0x030
#define ATHR_QOS_FIXED_PRIORITY        ((0 << 31) | (0 << 28))
#define ATHR_QOS_WEIGHTED              ((1 << 31) | (0 << 28)) /* Fixed weight 8,4,2,1 */
#define ATHR_QOS_MIXED                 ((1 << 31) | (1 << 28)) /* Q3 for managment; Q2,Q1,Q0 - 4,2,1 */


#ifndef BOOL
#define BOOL    int
#endif

#undef S26_VER_1_0

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

int ag7240_hard_start(struct sk_buff *skb, struct net_device *dev);

void athrs26_reg_init(int ethUnit);
void athrs26_reg_init_lan(int ethUnit);
int athrs26_phy_is_up(int unit);
int athrs26_phy_is_fdx(int unit,int phyUnit);
int athrs26_phy_speed(int unit,int phyUnit);
void athrs26_phy_stab_wr(int phy_id, BOOL phy_up, int phy_speed);
BOOL athrs26_phy_setup(int unit);
int athrs26_mdc_check(void);

void athrs26_phy_off(ag7240_mac_t *mac);
void athrs26_phy_on(ag7240_mac_t *mac);
void athrs26_mac_speed_set(ag7240_mac_t *mac, ag7240_phy_speed_t speed);
void athrs26_reg_dev(ag7240_mac_t **mac);
int athrs26_ioctl(struct net_device *dev,void *args, int cmd);
uint32_t athrs26_reg_read(unsigned int s26_addr);
void athrs26_reg_write(unsigned int s26_addr, unsigned int s26_write_data);

#define S26_FORCE_100M 1
#endif
