struct user_net_device_stats {
    unsigned long long rx_packets;	/* total packets received       */
    unsigned long long tx_packets;	/* total packets transmitted    */
    unsigned long long rx_bytes;	/* total bytes received         */
    unsigned long long tx_bytes;	/* total bytes transmitted      */
    unsigned long rx_errors;	/* bad packets received         */
    unsigned long tx_errors;	/* packet transmit problems     */
    unsigned long rx_dropped;	/* no space in linux buffers    */
    unsigned long tx_dropped;	/* no space available in linux  */
    unsigned long rx_multicast;	/* multicast packets received   */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_length_errors;
    unsigned long rx_over_errors;	/* receiver ring buff overflow  */
    unsigned long rx_crc_errors;	/* recved pkt with crc error    */
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long rx_missed_errors;	/* receiver missed packet     */
    /* detailed tx_errors */
    unsigned long tx_aborted_errors;
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
    unsigned long tx_heartbeat_errors;
    unsigned long tx_window_errors;
};

struct interface {
    struct interface *next, *prev; 
    char name[IFNAMSIZ];	/* interface name        */
    short type;			/* if type               */
    short flags;		/* various flags         */
    int metric;			/* routing metric        */
    int mtu;			/* MTU value             */
    int tx_queue_len;		/* transmit queue length */
    struct ifmap map;		/* hardware setup        */
    struct sockaddr addr;	/* IP address            */
    struct sockaddr dstaddr;	/* P-P IP address        */
    struct sockaddr broadaddr;	/* IP broadcast address  */
    struct sockaddr netmask;	/* IP network mask       */
    struct sockaddr ipxaddr_bb;	/* IPX network address   */
    struct sockaddr ipxaddr_sn;	/* IPX network address   */
    struct sockaddr ipxaddr_e3;	/* IPX network address   */
    struct sockaddr ipxaddr_e2;	/* IPX network address   */
    struct sockaddr ddpaddr;	/* Appletalk DDP address */
    struct sockaddr ecaddr;	/* Econet address        */
    int has_ip;
    int has_ipx_bb;
    int has_ipx_sn;
    int has_ipx_e3;
    int has_ipx_e2;
    int has_ax25;
    int has_ddp;
    int has_econet;
    char hwaddr[32];		/* HW address            */
    int statistics_valid;
    struct user_net_device_stats stats;		/* statistics            */
    int keepalive;		/* keepalive value for SLIP */
    int outfill;		/* outfill value for SLIP */
};

extern int if_fetch(struct interface *ife);

extern int for_all_interfaces(int (*)(struct interface *, void *), void *);
extern int free_interface_list(void);
extern struct interface *lookup_interface(char *name);
extern int if_readlist(void);

extern int do_if_fetch(struct interface *ife);
extern int do_if_print(struct interface *ife, void *cookie);

extern void ife_print(struct interface *ptr);

extern int ife_short;

extern const char *if_port_text[][4];

/* Defines for poor glibc2.0 users, the feature check is done at runtime */
#if !defined(SIOCSIFTXQLEN)
#define SIOCSIFTXQLEN      0x8943
#define SIOCGIFTXQLEN      0x8942
#endif

#if !defined(ifr_qlen)
/* Actually it is ifru_ivalue, but that is not present in 2.0 kernel headers */   
#define ifr_qlen        ifr_ifru.ifru_mtu
#endif

#define HAVE_TXQUEUELEN

#define HAVE_DYNAMIC
#ifndef IFF_DYNAMIC
#define IFF_DYNAMIC	0x8000	/* dialup device with changing addresses */
#endif
