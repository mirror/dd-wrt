/*
 * hostmon.h - definitions used by the Ethernet station monitor
 */

struct ethtabent {
    int type;
    union {
        struct {
            unsigned long long inpcount;
            unsigned long long inbcount;
            unsigned long long inippcount;
            unsigned long inspanbr;
            unsigned int inpktact;
            unsigned long long outpcount;
            unsigned long long outbcount;
            unsigned long long outippcount;
            unsigned long outspanbr;
            unsigned int outpktact;
            float inrate;
            float outrate;
            short past5;
        } figs;

        struct {
            char eth_addr[ETH_ALEN];
            char ascaddr[15];
            char desc[65];
            char ifname[10];
            int withdesc;
            int printed;
            unsigned int linktype;
        } desc;
    } un;

    unsigned int index;
    struct ethtabent *prev_entry;
    struct ethtabent *next_entry;
};

struct ethtab {
    struct ethtabent *head;
    struct ethtabent *tail;
    struct ethtabent *firstvisible;
    struct ethtabent *lastvisible;
    unsigned long count;
    unsigned long entcount;
    WINDOW *borderwin;
    PANEL *borderpanel;
    WINDOW *tabwin;
    PANEL *tabpanel;
};
