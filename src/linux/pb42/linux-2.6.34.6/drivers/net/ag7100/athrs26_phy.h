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
#define ATHR_PHY_SPEC_CONTROL            16
#define ATHR_PHY_SPEC_STATUS             17
#define ATHR_DEBUG_PORT_ADDRESS          29
#define ATHR_DEBUG_PORT_DATA             30

#define BITS(_s, _n)	(((1UL << (_n)) - 1) << _s)
#define BIT(nr)			(1UL << (nr))
#define AR8216_REG_GLOBAL_CTRL		0x0030
#define AR8216_GCTRL_MTU		BITS(0, 11)

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

#ifndef BOOL
#define BOOL    int
#endif

#define HEADER_LEN   2
#define HEADER_MAX_DATA  10
#undef S26_VER_1_0

/* before define HEADER_REG_CONF, please make sure you also enable s26's */
/* HEADER_EN on CPU port in bootloader, or the configuration will fail */
#ifndef CONFIG_AR9100
#undef HEADER_REG_CONF
#else
#define HEADER_REG_CONF 1
#endif
#ifdef HEADER_REG_CONF
#define HEADER_EN
#endif

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

typedef struct {
    uint64_t    reg_addr;	
    uint64_t    reserved0;	
    uint64_t    cmd_len;		
    uint64_t    reserved1; 	
    uint64_t    cmd; 		
    uint64_t    reserved2;	
    uint64_t    seq_num;  
}reg_cmd_t;

typedef struct {
    uint8_t data[HEADER_MAX_DATA];
    uint8_t len;
    uint32_t seq;
} cmd_resp_t;

int header_receive_skb(struct sk_buff *skb);
void athrs26_reg_dev(ag7100_mac_t **mac);

#define header_xmit(skb, dev) ag7100_hard_start(skb, dev) //dev_queue_xmit(skb)

void athrs26_reg_init(void);
int athrs26_phy_is_up(int unit);
int athrs26_phy_is_fdx(int unit);
int athrs26_phy_speed(int unit);
BOOL athrs26_phy_setup(int unit);

void athrs26_phy_off(ag7100_mac_t *mac);
void athrs26_phy_on(ag7100_mac_t *mac);
void athrs26_mac_speed_set(ag7100_mac_t *mac, ag7100_phy_speed_t speed);
void set_cpu_egress_tagged(uint8_t is_tagged);
uint16_t athrs26_defvid_get(uint32_t port_id);
int athr_ioctl(uint32_t *args, int cmd);
uint8_t is_cpu_egress_tagged(void);

#endif


