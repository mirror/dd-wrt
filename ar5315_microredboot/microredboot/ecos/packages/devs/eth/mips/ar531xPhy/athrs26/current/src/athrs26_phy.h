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

#define ATHR_ADVERTISE_ALL (ATHR_ADVERTISE_10HALF | ATHR_ADVERTISE_10FULL | \
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


#define BOOL   UINT32 
#define HEADER_LEN   2
#define HEADER_MAX_DATA  10
#undef S26_VER_1_0
/* Ken: for debug output */
//#define DEBUGOUT(S) printf(S "\n")
#define DEBUGOUT(S) 


/* before define HEADER_REG_CONF, please make sure you also enable s26's */
/* HEADER_EN on CPU port in bootloader, or the configuration will fail */
#define HEADER_REG_CONF
#ifdef HEADER_REG_CONF
#define HEADER_EN
#endif

#ifdef HEADER_EN
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
    UINT16    reserved0; 
    UINT16    priority;  
    UINT16    type ;     
    UINT16    broadcast; 
    UINT16    from_cpu;  
    UINT16    reserved1; 
    UINT16    port_num;  
}at_header_t;

typedef struct {
    unsigned long long    reg_addr;	
    unsigned long long    reserved0;	
    unsigned long long    cmd_len;		
    unsigned long long    reserved1; 	
    unsigned long long    cmd; 		
    unsigned long long    reserved2;	
    unsigned long long    seq_num;  
}reg_cmd_t;

//int header_receive_skb(struct sk_buff *skb);
void athrs26_reg_dev(ae531x_MAC_t*mac);

#ifdef HEADER_REG_CONF
#define header_xmit(skb, dev) ag7100_hard_start(skb, dev) //dev_queue_xmit(skb)
#endif

#endif

#endif


