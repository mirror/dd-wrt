/* frame_defs.h - definitions for LANE control frames, TLVs etc. */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef FRAMES_DEFS_H
#define FRAMES_DEFS_H

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

/* Try to squeeze out all the padding from the structs so that we
 * can use them as templates for filling in and examining frames.
 * From the gcc documentation:
 *    This attribute, attached to an `enum', `struct', or `union' type
 *    definition, specified that the minimum required memory be used to
 *    represent the type.
 */
#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/* READY_QUERY and READY_IND frame format
 */
struct ready_frame {
        uint16_t marker;
        uint8_t  protocol;
        uint8_t  version;
        uint16_t opcode;
};

/* Fields common to all control frames,
 * not including READY_*
 */
struct frame_hdr {
        uint16_t marker;
        uint8_t  protocol;
        uint8_t  version;
        uint16_t opcode;
        uint16_t status;
        uint32_t tran_id;
        uint16_t lec_id;
        uint16_t flags;
} PACKED;

/* values for lan_dst.tag
 */
#define LAN_DST_NOT_PRESENT 0x0000
#define LAN_DST_MAC_ADDR    0x0001
#define LAN_DST_ROUTE_DESC  0x0002

struct lan_dst { /* Token Ring route descriptors omitted */
        uint16_t tag;
        uint8_t  mac[ETH_ALEN];
} PACKED;

/* All frames except STATUS_INQ and STATUS_REPLY look like this
 */
struct ctrl_frame {
        struct frame_hdr header;
        struct lan_dst src_lan_dst;
        struct lan_dst target_lan_dst;
        uint8_t src_atm_addr[ATM_ESA_LEN];
        uint8_t lan_type;
        uint8_t max_frame_size;
        uint8_t num_tlvs;
        uint8_t elan_name_size;
        uint8_t target_atm_addr[ATM_ESA_LEN];
        uint8_t elan_name[32];
        /* TLVs if any follow elan_name */
} PACKED;

/* Frame types
 */
#define LE_CONFIG_REQ 0x0001
#define LE_CONFIG_RSP 0x0101
#define LE_JOIN_REQ   0x0002
#define LE_JOIN_RSP   0x0102
#define LE_REG_REQ    0x0004
#define LE_REG_RSP    0x0104
#define LE_ARP_REQ    0x0006
#define LE_ARP_RSP    0x0106
#define LE_FLUSH_REQ  0x0007
#define LE_FLUSH_RSP  0x0107
#define LE_NARP_REQ   0x0008
#define LE_TOPO_REQ   0x0009
#define READY_QUERY   0x0003 /* READY_* are not in ctrl_frame format */
#define READY_IND     0x0103

/* Flags for LE Control Frames
 */
#define REMOTE_ADDRESS  0x0001
#define V2_CAPABLE      0x0002
#define V2_REQUIRED     0x0008
#define PROXY_FLAG      0x0080
#define TOPO_CHANGE     0x0100

/* TLV types defined in LANEv1 and v2 + one Fore specific TLV
 */
#define MAX_CUM_CTRL_TIMEOUT   0x00A03E01
#define MAX_UNKNOWN_FRAME_CNT  0x00A03E02
#define MAX_UNKNOWN_FRAME_TIME 0x00A03E03
#define VCC_TIMEOUT_PERIOD     0x00A03E04
#define MAX_RETRY_COUNT        0x00A03E05
#define AGING_TIME             0x00A03E06
#define FORWARD_DELAY_TIME     0x00A03E07
#define EXPECTED_LE_ARP_TIME   0x00A03E08
#define FLUSH_TIMEOUT          0x00A03E09
#define PATH_SWITCHING_DELAY   0x00A03E0A
#define LOCAL_SEGMENT_ID       0x00A03E0B
#define DEF_MCAST_SND_VCC_TYPE 0x00A03E0C
#define DEF_MCAST_SND_VCC_AVG  0x00A03E0D
#define DEF_MCAST_SEND_PEAK_RT 0x00A03E0E
#define CONN_COMPLETION_TIMER  0x00A03E0F
#define CONFIG_FRAG_INFO       0x00A03E10 /* This and the rest are LANEv2 only */
#define LAYER3_ADDRESS         0x00A03E11
#define ELAN_ID                0x00A03E12
#define SERVICE_CATEGORY       0x00A03E13
#define LLC_MUXED_ATM_ADDR     0x00A03E2B
#define X5_ADJUSTMENT          0x00A03E2C /* length 0 */
#define PREFERRED_LES          0x00A03E2D

#define FORE_NAME              0x00204808 /* check zeppelin(8), -f option */

#endif /* FRAMES_DEFS_H */
