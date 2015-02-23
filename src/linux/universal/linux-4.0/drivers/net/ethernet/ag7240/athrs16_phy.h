#ifndef _ATHRS16_PHY_H
#define _ATHRS16_PHY_H

#define BITS(_s, _n)	(((1UL << (_n)) - 1) << _s)

#define AR8216_REG_GLOBAL_CTRL		0x0030
#define   AR8316_GCTRL_MTU		BITS(0, 14)

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
#define ATHR_PHY_SPEC_CONTROL            16
#define ATHR_PHY_SPEC_STATUS             17
#define ATHR_DEBUG_PORT_ADDRESS          29
#define ATHR_DEBUG_PORT_DATA             30

/* ATHR_PHY_CONTROL fields */
#define ATHR_CTRL_SOFTWARE_RESET                    0x8000
#define ATHR_CTRL_SPEED_LSB                         0x2000
#define ATHR_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define ATHR_CTRL_RESTART_AUTONEGOTIATION           0x0200
#define ATHR_CTRL_SPEED_FULL_DUPLEX                 0x0100
#define ATHR_CTRL_SPEED_MSB                         0x0040

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
#define ATHR_STATUS_RESOVLED                  0x0800

/*phy debug port  register */
#define ATHER_DEBUG_SERDES_REG                5

/* Serdes debug fields */
#define ATHER_SERDES_BEACON                   0x0100

/* S16 CSR Registers */

#define S16_PORT_STATUS_REGISTER0            0x0100 
#define S16_PORT_STATUS_REGISTER1            0x0200
#define S16_PORT_STATUS_REGISTER2            0x0300
#define S16_PORT_STATUS_REGISTER3            0x0400
#define S16_PORT_STATUS_REGISTER4            0x0500
#define S16_PORT_STATUS_REGISTER5            0x0600


/* bit definition for port status register
 * 31:13    : RO :       : Reserved
 *    12    : RW :      1: Reserved
 *    11    : RW :      0: Reserved
 *    10    : RW :      0: Reserved
 *     9    : RW :      1: LIN_EN 
 *     8    : RO :      1: 0 - phy link down. 1 - phy link up
 *     7    : RW :      1; TX_HALF_FLOW_EN
 *     6    : RW :      0: DUPLEX_MODE, 0-half, 1-full
 *     5    : RW :      0: RxFlow control Enable
 *     4    : RW :      0: TxFlow control Enable
 *     3    : RW :      0: Rx Mac Enable
 *     2    : RW :      0: Tx Mac Enable
 *    1:0   : RW :      00: Speed, 00-10, 01-100, 10-1000, 11 -err mbps. 
*/
#define S16_PORT_STATUS_LINK_EN              (1<<9)
#define S16_PORT_STATUS_LINK_STATUS          (1<<8)
#define S16_PORT_STATUS_TXHALF_FLOW_EN       (1<<7)
#define S16_PORT_STATUS_DUPLEX_HALF          (0<<6)
#define S16_PORT_STATUS_DUPLEX_FULL          (1<<6)
#define S16_PORT_STATUS_RXFLOW_EN            (1<<5)
#define S16_PORT_STATUS_TXFLOW_EN            (1<<4)
#define S16_PORT_STATUS_RXMAC_EN             (1<<3)
#define S16_PORT_STATUS_TXMAC_EN             (1<<2)
#define S16_PORT_STATUS_SPEED_10MBPS         0x0
#define S16_PORT_STATUS_SPEED_100MBPS        0x1
#define S16_PORT_STATUS_SPEED_1000MBPS       0x2
#define S16_PORT_STATUS_SPEED_ERROR          0x3
 


#define S16_PORT_BASE_VLAN_REGISTER0         0x0108 
#define S16_PORT_BASE_VLAN_REGISTER1         0x0208
#define S16_PORT_BASE_VLAN_REGISTER2         0x0308
#define S16_PORT_BASE_VLAN_REGISTER3         0x0408
#define S16_PORT_BASE_VLAN_REGISTER4         0x0508
#define S16_PORT_BASE_VLAN_REGISTER5         0x0608

#define S16_VLAN_FUNC_REGISTER0               0x0040
#define S16_VLAN_FUNC_REGISTER1               0x0044


#define S16_RATE_LIMIT_REGISTER0             0x010C
#define S16_RATE_LIMIT_REGISTER1             0x020C
#define S16_RATE_LIMIT_REGISTER2             0x030C
#define S16_RATE_LIMIT_REGISTER3             0x040C
#define S16_RATE_LIMIT_REGISTER4             0x050C
#define S16_RATE_LIMIT_REGISTER5             0x060C

#define S16_PORT_CONTROL_REGISTER0           0x0104
#define S16_PORT_CONTROL_REGISTER1           0x0204
#define S16_PORT_CONTROL_REGISTER2           0x0304
#define S16_PORT_CONTROL_REGISTER3           0x0404
#define S16_PORT_CONTROL_REGISTER4           0x0504
#define S16_PORT_CONTROL_REGISTER5           0x0604

#define S16_CPU_PORT_REGISTER                0x0078
#define S16_MDIO_CTRL_REGISTER               0x0098

#define S16_ARL_TBL_FUNC_REG0                0x0050
#define S16_ARL_TBL_FUNC_REG1                0x0054
#define S16_ARL_TBL_FUNC_REG2                0x0058
#define S16_FLD_MASK_REG                     0x002c
#define S16_ARL_TBL_CTRL_REG                 0x005c
#define S16_GLOBAL_INTR_REG                  0x10
#define S16_GLOBAL_INTR_MASK_REG             0x14
#define S16_PWR_ON_STRAP_REG                 0x8


#define S16_ENABLE_CPU_BROADCAST             (1 << 26)

#define S16_PHY_LINK_CHANGE_REG               0x4
#define S16_PHY_LINK_UP               0x400
#define S16_PHY_LINK_DOWN             0x800
#define S16_PHY_LINK_DUPLEX_CHANGE        0x2000
#define S16_PHY_LINK_SPEED_CHANGE         0x4000
#define S16_PHY_LINK_INTRS            (PHY_LINK_UP | PHY_LINK_DOWN | PHY_LINK_DUPLEX_CHANGE | PHY_LINK_SPEED_CHANGE)

/* SWITCH QOS REGISTERS */

#define ATHR_PRI_CTRL_PORT_0              0x110 /* CPU PORT */
#define ATHR_PRI_CTRL_PORT_1              0x210
#define ATHR_PRI_CTRL_PORT_2              0x310
#define ATHR_PRI_CTRL_PORT_3              0x410
#define ATHR_PRI_CTRL_PORT_4              0x510
#define ATHR_PRI_CTRL_PORT_5              0x610

#define ATHR_TOS_PRI_EN                       (1 << 16)
#define ATHR_VLAN_PRI_EN                      (1 << 17)
#define ATHR_DA_PRI_EN                        (1 << 18)
#define ATHR_PORT_PRI_EN                      (1 << 19)

#define ATHR_QOS_MODE_REGISTER                0x030
#define ATHR_QOS_FIXED_PRIORITY               ((0 << 31) | (0 << 28))
#define ATHR_QOS_WEIGHTED                     ((1 << 31) | (0 << 28)) /* Fixed weight 8,4,2,1 */
#define ATHR_QOS_MIXED                        ((1 << 31) | (1 << 28)) /* Q3 for managment; Q2,Q1,Q0 - 4,2,1 */

#ifndef BOOL
#define BOOL    int
#endif

/*add feature define here*/
//#define FULL_FEATURE

#ifdef CONFIG_AR7242_S16_PHY
#undef HEADER_REG_CONF
#undef HEADER_EN
#endif

void athrs16_reg_init(int ethUnit);
int athrs16_phy_is_up(int unit);
int athrs16_phy_is_fdx(int unit);
int athrs16_phy_speed(int unit);
BOOL athrs16_phy_setup(int unit);
int athrs16_phy_is_link_alive(int phyUnit);
void athrs16_reg_dev(ag7240_mac_t **mac);
uint32_t athrs16_reg_read(uint32_t reg_addr);
void athrs16_reg_write(uint32_t reg_addr, uint32_t reg_val);
int athrs16_ioctl(struct eth_cfg_params *ethcfg, int cmd);

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
    uint16_t    reserved0;
    uint16_t    priority;
    uint16_t    type ;
    uint16_t    broadcast;
    uint16_t    from_cpu;
    uint16_t    reserved1;
    uint16_t    port_num;
}at_header_t;

#define ATHR_HEADER_LEN 2

#endif // _ATH_HEADER_CONF

#endif


