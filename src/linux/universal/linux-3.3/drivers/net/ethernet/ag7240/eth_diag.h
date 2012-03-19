#define SIOCDEVPRIVATE  0x89F0  /* to 89FF */
#define S26_RD_PHY  (SIOCDEVPRIVATE | 0x1)
#define S26_WR_PHY  (SIOCDEVPRIVATE | 0x2)
#define S26_FORCE_PHY  (SIOCDEVPRIVATE | 0x3)

#ifdef CONFIG_AR7240_S26_VLAN_IGMP
// Add or remove ports to the device
// bit0--->port0;bit1--->port1.
#define  S26_VLAN_ADDPORTS  	(SIOCDEVPRIVATE | 0x4)
#define  S26_VLAN_DELPORTS  	(SIOCDEVPRIVATE | 0x5)

// Set the tag mode to the port.
#define  S26_VLAN_SETTAGMODE  	(SIOCDEVPRIVATE | 0x6)

// Set default vlan id to the port
#define  S26_VLAN_SETDEFAULTID  (SIOCDEVPRIVATE | 0x7)

// Enable or disable IGMP snooping based on a vlanid
#define  S26_IGMP_ON   		(SIOCDEVPRIVATE | 0x8)
#define  S26_IGMP_OFF  		(SIOCDEVPRIVATE | 0x9)

// Get a link status from the specified port.
#define  S26_LINK_GETSTAT  	(SIOCDEVPRIVATE | 0xA)

#define  S26_VLAN_ENABLE  	(SIOCDEVPRIVATE | 0xB)
#define  S26_VLAN_DISABLE  	(SIOCDEVPRIVATE | 0xC)

#define  S26_ARL_ADD  	        (SIOCDEVPRIVATE | 0xD)
#define  S26_ARL_DEL  	        (SIOCDEVPRIVATE | 0xE)

#define  S26_MCAST_CLR  	(SIOCDEVPRIVATE | 0xF)
#define  S26_PACKET_FLAG  	(SIOCDEVPRIVATE | 0x0)

#define  VLAN_DEV_INFO(x) ((struct eth_vlan_dev_info *)x->priv)

struct eth_vlan_dev_info {
    unsigned long inmap[8];
    char * outmap[16];
    unsigned short vlan_id;
};

typedef struct {
    u_int8_t uc[6];
} mac_addr_t;

struct arl_struct {
	mac_addr_t mac_addr;
	int port_map;
	int sa_drop; 
};

#endif

struct eth_diag {
    char    ad_name[IFNAMSIZ];      /* if name, e.g. "eth0" */
    union {
        u_int16_t portnum;           /* pack to fit, yech */
        u_int8_t duplex;
    }ed_u;
    u_int32_t phy_reg;
    u_int   val;
    caddr_t ad_in_data;
    caddr_t ad_out_data;
};

