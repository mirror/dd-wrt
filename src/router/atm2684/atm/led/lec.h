/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef LEC_H
#define LEC_H

#include "conn.h"

/*
 * LANE client configuration and operation values.
 * These are in host byte order since there are
 * some values coming from network and some values
 * which are used by the host only
 */
struct lec_params {
        unsigned char c1n_my_atm_addr[ATM_ESA_LEN];
        uint8_t  c2_lan_type;
        uint8_t  c3_max_frame_size;
        int      c4_proxy_flag;
        char     c5_elan_name[32 + 1];
        char     c6_mac_addr[ETH_ALEN];
        int      c7_ctrl_timeout;
        int      c7i_initial_ctrl_timeout;
        int      c7x_timeout_multiplier;
        int      c7c_current_timeout; /* sum of c7i and c7x, LANEv2 5.3.1.7 */
        unsigned char c9_les_atm_addr[ATM_ESA_LEN];
        int      c10_max_unknown_frames;
        int      c11_max_unknown_frame_time;
        int      c12_vcc_timeout;
        int      c13_max_retry_count;
        uint16_t c14_lec_id;
        int      c17_aging_time;
        int      c18_forward_delay_time;
        int      c19_topology_change;
        int      c20_le_arp_response_time;
        int      c21_flush_timeout;
        int      c22_path_switching_delay;
        /* LANE2 variables follow */
        int      c29_v2_capable;
        uint32_t c31_elan_id;
        unsigned char c35_preferred_les[ATM_ESA_LEN];
        int      c35_contains_address;
        int      c37_min_reconfig_delay; /* milliseconds */
        int      c38_max_reconfig_delay; /* milliseconds */

        /* other stuff */
        int  itf_num;    /* 1 for lec1 and so forth */
        int  sizeoftlvs; /* total size of TLVs associated with this LEC */
        int  num_tlvs;   /* number of the TLVs */
        unsigned char *tlvs; /* the TLVs */
        char foreId[255];

        /* connections to and from LES/BUS plus listen sockets */
        Conn_t *kernel;
        Conn_t *ctrl_direct;
        Conn_t *ctrl_listen; /* Closed when join phase is over */
        Conn_t *ctrl_dist;
        Conn_t *mcast_send;  /* LANEv2 calls this Default Mcast Send VCC */
        Conn_t *mcast_listen;
        Conn_t *data_listen;
};

extern struct lec_params lec_params;

#define LAN_TYPE_UNSPEC 0x00 /* Implies Ethernet/IEEE 802.3 */
#define LAN_TYPE_8023   0x01 /* IEEE 802.3 */
#define LAN_TYPE_8025   0x02 /* IEEE 802.5 */

#endif /* LEC_H */
