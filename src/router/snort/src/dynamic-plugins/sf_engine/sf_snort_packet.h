/*
 * sf_snort_packet.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steve Sturges
 *         Andy Mullican
 *
 * Date: 5/2005
 *
 * Sourcefire Black-box Plugin API for rules
 *
 */

#ifndef _SF_SNORT_PACKET_H_
#define _SF_SNORT_PACKET_H_

#ifndef WIN32
#include <sys/types.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include <daq.h>
#include <sfbpf_dlt.h>

#include "sf_ip.h"
#include "sf_protocols.h"
#include "preprocids.h"

#define VLAN_HDR_LEN  4

/* for vrt backwards compatibility */
#define pcap_header pkt_header

typedef int (*LogFunction)(void *ssnptr, uint8_t **buf, uint32_t *len, uint32_t *type);

typedef DAQ_PktHdr_t SFDAQ_PktHdr_t;

#define VTH_PRIORITY(vh)  ((ntohs((vh)->vth_pri_cfi_vlan) & 0xe000) >> 13)
#define VTH_CFI(vh)       ((ntohs((vh)->vth_pri_cfi_vlan) & 0x1000) >> 12)
#define VTH_VLAN(vh)      ((uint16_t)(ntohs((vh)->vth_pri_cfi_vlan) & 0x0FFF))

typedef struct _VlanHeader
{
    uint16_t vth_pri_cfi_vlan;
    uint16_t vth_proto;  /* protocol field... */

} VlanHeader;

/*#define NO_NON_ETHER_DECODER */
#define ETHER_HDR_LEN  14
#define ETHERNET_TYPE_IP    0x0800
#define ETHERNET_TYPE_IPV6  0x86dd
#define ETHERNET_TYPE_8021Q 0x8100
/*
 * Cisco MetaData header
 */

typedef struct _CiscoMetaHdr
{
    uint8_t version; // This must be 1
    uint8_t length; //This is the header size in bytes / 8
} CiscoMetaHdr;

/*
 * Cisco MetaData header options
 */

typedef struct _CiscoMetaOpt
{
    uint16_t opt_len_type;  /* 3-bit length + 13-bit type. Length of 0 = 4. Type must be 1. */
    uint16_t sgt;           /* Can be any value except 0xFFFF */
} CiscoMetaOpt;


typedef struct _EtherHeader
{
    uint8_t ether_destination[6];
    uint8_t ether_source[6];
    uint16_t ethernet_type;

} EtherHeader;

/* We must twiddle to align the offset the ethernet header and align
 * the IP header on solaris -- maybe this will work on HPUX too.
 */
#if defined (SOLARIS) || defined (SUNOS) || defined (__sparc__) || defined(__sparc64__) || defined (HPUX)
#define SUN_SPARC_TWIDDLE       2
#else
#define SUN_SPARC_TWIDDLE       0
#endif

#define IP_RESBIT       0x8000
#ifdef IP_DONTFRAG
#undef IP_DONTFRAG
#endif
#define IP_DONTFRAG     0x4000
#define IP_MOREFRAGS    0x2000

#ifndef IP_MAXPKT
#define IP_MAXPKT    65535        /* maximum packet size */
#endif /* IP_MAXPACKET */

#define IP_HDR_LEN  20

#if !defined(SFLINUX) && defined(DAQ_CAPA_CARRIER_ID)
#if defined(DAQ_VERSION) && DAQ_VERSION > 10
#define GET_SFOUTER_IPH_PROTOID(p, pkt_header) ((uint32_t)(p->pkt_header->carrier_id) ? p->pkt_header->carrier_id : 0 )
#else
#define GET_SFOUTER_IPH_PROTOID(p, pkt_header) ((uint32_t)((p)->outer_ip4_header ? (IS_IP6(p) ? ((p)->outer_ip6h.next) : ((p)->outer_ip4h.ip_proto)):0))
#endif
#endif

typedef struct _IPV4Header
{
    uint8_t version_headerlength;
    uint8_t type_service;
    uint16_t data_length;
    uint16_t identifier;
    uint16_t offset;
    uint8_t time_to_live;
    uint8_t proto;
    uint16_t checksum;
    struct in_addr source;
    struct in_addr destination;
} IPV4Header;

#define MAX_LOG_FUNC   32
#define MAX_IP_OPTIONS 40

/* ip option codes */
#define IPOPTION_EOL            0x00
#define IPOPTION_NOP            0x01
#define IPOPTION_RR             0x07
#define IPOPTION_RTRALT         0x94
#define IPOPTION_TS             0x44
#define IPOPTION_SECURITY       0x82
#define IPOPTION_LSRR           0x83
#define IPOPTION_LSRR_E         0x84
#define IPOPTION_SATID          0x88
#define IPOPTION_SSRR           0x89

typedef struct _IPOptions
{
    uint8_t option_code;
    uint8_t length;
    uint8_t *option_data;
} IPOptions;


#define TCP_HDR_LEN  20

typedef struct _TCPHeader
{
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence;
    uint32_t acknowledgement;
    uint8_t offset_reserved;
    uint8_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_pointer;
} TCPHeader;

#define TCPHEADER_FIN  0x01
#define TCPHEADER_SYN  0x02
#define TCPHEADER_RST  0x04
#define TCPHEADER_PUSH 0x08
#define TCPHEADER_ACK  0x10
#define TCPHEADER_URG  0x20
#define TCPHEADER_ECE  0x40
#define TCPHEADER_CWR  0x80
#define TCPHEADER_NORESERVED (TCPHEADER_FIN|TCPHEADER_SYN|TCPHEADER_RST \
                            |TCPHEADER_PUSH|TCPHEADER_ACK|TCPHEADER_URG)

#define MAX_TCP_OPTIONS 40
/* tcp option codes */
#define TCPOPT_EOL              0x00
#define TCPOPT_NOP              0x01
#define TCPOPT_MSS              0x02
#define TCPOPT_WSCALE           0x03     /* window scale factor (rfc1072) */
#define TCPOPT_SACKOK           0x04     /* selective ack ok (rfc1072) */
#define TCPOPT_SACK             0x05     /* selective ack (rfc1072) */
#define TCPOPT_ECHO             0x06     /* echo (rfc1072) */
#define TCPOPT_ECHOREPLY        0x07     /* echo (rfc1072) */
#define TCPOPT_TIMESTAMP        0x08     /* timestamps (rfc1323) */
#define TCPOPT_CC               0x11     /* T/TCP CC options (rfc1644) */
#define TCPOPT_CCNEW            0x12     /* T/TCP CC options (rfc1644) */
#define TCPOPT_CCECHO           0x13     /* T/TCP CC options (rfc1644) */

typedef IPOptions TCPOptions;

#define UDP_HDR_LEN  8

typedef struct _UDPHeader
{
    uint16_t source_port;
    uint16_t destination_port;
    uint16_t data_length;
    uint16_t checksum;
} UDPHeader;

typedef struct _ICMPSequenceID
{
    uint16_t id;
    uint16_t seq;
} ICMPSequenceID;

typedef struct _ICMPHeader
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;

    union
    {
        /* type 12 */
        uint8_t parameter_problem_ptr;

        /* type 5 */
        struct in_addr gateway_addr;

        /* type 8, 0 */
        ICMPSequenceID echo;

        /* type 13, 14 */
        ICMPSequenceID timestamp;

        /* type 15, 16 */
        ICMPSequenceID info;

        int voidInfo;

        /* type 3/code=4 (Path MTU, RFC 1191) */
        struct path_mtu
        {
            uint16_t voidInfo;
            uint16_t next_mtu;
        } path_mtu;

        /* type 9 */
        struct router_advertisement
        {
            uint8_t number_addrs;
            uint8_t entry_size;
            uint16_t lifetime;
        } router_advertisement;
    } icmp_header_union;

#define icmp_parameter_ptr  icmp_header_union.parameter_problem_ptr
#define icmp_gateway_addr   icmp_header_union.gateway_waddr
#define icmp_echo_id        icmp_header_union.echo.id
#define icmp_echo_seq       icmp_header_union.echo.seq
#define icmp_timestamp_id   icmp_header_union.timestamp.id
#define icmp_timestamp_seq  icmp_header_union.timestamp.seq
#define icmp_info_id        icmp_header_union.info.id
#define icmp_info_seq       icmp_header_union.info.seq
#define icmp_void           icmp_header_union.void
#define icmp_nextmtu        icmp_header_union.path_mtu.nextmtu
#define icmp_ra_num_addrs   icmp_header_union.router_advertisement.number_addrs
#define icmp_ra_entry_size  icmp_header_union.router_advertisement.entry_size
#define icmp_ra_lifetime    icmp_header_union.router_advertisement.lifetime

    union
    {
        /* timestamp */
        struct timestamp
        {
            uint32_t orig;
            uint32_t receive;
            uint32_t transmit;
        } timestamp;

        /* IP header for unreach */
        struct ipv4_header
        {
            IPV4Header *ip;
            /* options and then 64 bits of data */
        } ipv4_header;

        /* Router Advertisement */
        struct router_address
        {
            uint32_t addr;
            uint32_t preference;
        } router_address;

        /* type 17, 18 */
        uint32_t mask;

        char    data[1];

    } icmp_data_union;
#define icmp_orig_timestamp     icmp_data_union.timestamp.orig
#define icmp_recv_timestamp     icmp_data_union.timestamp.receive
#define icmp_xmit_timestamp     icmp_data_union.timestamp.transmit
#define icmp_ipheader           icmp_data_union.ip_header
#define icmp_ra_addr0           icmp_data_union.router_address
#define icmp_mask               icmp_data_union.mask
#define icmp_data               icmp_data_union.data
} ICMPHeader;

#define ICMP_ECHO_REPLY             0    /* Echo Reply                   */
#define ICMP_DEST_UNREACHABLE       3    /* Destination Unreachable      */
#define ICMP_SOURCE_QUENCH          4    /* Source Quench                */
#define ICMP_REDIRECT               5    /* Redirect (change route)      */
#define ICMP_ECHO_REQUEST           8    /* Echo Request                 */
#define ICMP_ROUTER_ADVERTISEMENT   9    /* Router Advertisement         */
#define ICMP_ROUTER_SOLICITATION    10    /* Router Solicitation          */
#define ICMP_TIME_EXCEEDED          11    /* Time Exceeded                */
#define ICMP_PARAMETER_PROBLEM      12    /* Parameter Problem            */
#define ICMP_TIMESTAMP_REQUEST      13    /* Timestamp Request            */
#define ICMP_TIMESTAMP_REPLY        14    /* Timestamp Reply              */
#define ICMP_INFO_REQUEST           15    /* Information Request          */
#define ICMP_INFO_REPLY             16    /* Information Reply            */
#define ICMP_ADDRESS_REQUEST        17    /* Address Mask Request         */
#define ICMP_ADDRESS_REPLY          18    /* Address Mask Reply           */

#define INVALID_CHECKSUM_IP   0x01
#define INVALID_CHECKSUM_TCP  0x02
#define INVALID_CHECKSUM_UDP  0x04
#define INVALID_CHECKSUM_ICMP 0x08
#define INVALID_CHECKSUM_IGMP 0x10
#define INVALID_CHECKSUM_ALL  0x1F
#define INVALID_TTL           0x20

typedef struct _IPv6Extension
{
    uint8_t option_type;
    const uint8_t *option_data;
} IP6Extension;

typedef struct _IPAddresses
{
    sfaddr_t ip_src;       /* source IP */
    sfaddr_t ip_dst;       /* dest IP */
} IPAddresses;

typedef struct _IPv4Hdr
{
    uint8_t ip_verhl;      /* version & header length */
    uint8_t ip_tos;        /* type of service */
    uint16_t ip_len;       /* datagram length */
    uint16_t ip_id;        /* identification  */
    uint16_t ip_off;       /* fragment offset */
    uint8_t ip_ttl;        /* time to live field */
    uint8_t ip_proto;      /* datagram protocol */
    uint16_t ip_csum;      /* checksum */
    IPAddresses* ip_addrs; /* IP addresses*/
} IP4Hdr;

typedef struct _IP6RawHdr
{
    uint32_t vcl;          /* version, class, and label */
    uint16_t payload_len;  /* length of the payload */
    uint8_t  next_header;  /* same values as ip4 protocol field + new ip6 values */
    uint8_t  hop_limit;    /* same usage as ip4 ttl */

    struct in6_addr src_addr;
    struct in6_addr dst_addr;
} IP6RawHdr;

#define ip6_vcl          vcl
#define ip6_payload_len  payload_len
#define ip6_next_header  next_header
#define ip6_hop_limit    hop_limit
#define ip6_hops         hop_limit

typedef struct _IPv6Hdr
{
    uint32_t vcl;      /* version, class, and label */
    uint16_t len;      /* length of the payload */
    uint8_t  next;     /* next header
                         * Uses the same flags as
                         * the IPv4 protocol field */
    uint8_t  hop_lmt;  /* hop limit */
    IPAddresses* ip_addrs; /* IP addresses*/
} IP6Hdr;

typedef struct _IP6FragHdr
{
    uint8_t   ip6f_nxt;     /* next header */
    uint8_t   ip6f_reserved;    /* reserved field */
    uint16_t  ip6f_offlg;   /* offset, reserved, and flag */
    uint32_t  ip6f_ident;   /* identification */
} IP6FragHdr;

typedef struct _ICMP6
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;

} ICMP6Hdr;

#define ICMP6_UNREACH 1
#define ICMP6_BIG    2
#define ICMP6_TIME   3
#define ICMP6_PARAMS 4
#define ICMP6_ECHO   128
#define ICMP6_REPLY  129

/* Minus 1 due to the 'body' field  */
#define ICMP6_MIN_HEADER_LEN (sizeof(ICMP6Hdr) )

struct _SFSnortPacket;

typedef struct _IPH_API
{
    sfaddr_t * (*iph_ret_src)(const struct _SFSnortPacket *);
    sfaddr_t * (*iph_ret_dst)(const struct _SFSnortPacket *);
    uint16_t   (*iph_ret_tos)(const struct _SFSnortPacket *);
    uint8_t    (*iph_ret_ttl)(const struct _SFSnortPacket *);
    uint16_t   (*iph_ret_len)(const struct _SFSnortPacket *);
    uint32_t   (*iph_ret_id)(const struct _SFSnortPacket *);
    uint8_t    (*iph_ret_proto)(const struct _SFSnortPacket *);
    uint16_t   (*iph_ret_off)(const struct _SFSnortPacket *);
    uint8_t    (*iph_ret_ver)(const struct _SFSnortPacket *);
    uint8_t    (*iph_ret_hlen)(const struct _SFSnortPacket *);

    sfaddr_t * (*orig_iph_ret_src)(const struct _SFSnortPacket *);
    sfaddr_t * (*orig_iph_ret_dst)(const struct _SFSnortPacket *);
    uint16_t   (*orig_iph_ret_tos)(const struct _SFSnortPacket *);
    uint8_t    (*orig_iph_ret_ttl)(const struct _SFSnortPacket *);
    uint16_t   (*orig_iph_ret_len)(const struct _SFSnortPacket *);
    uint32_t   (*orig_iph_ret_id)(const struct _SFSnortPacket *);
    uint8_t    (*orig_iph_ret_proto)(const struct _SFSnortPacket *);
    uint16_t   (*orig_iph_ret_off)(const struct _SFSnortPacket *);
    uint8_t    (*orig_iph_ret_ver)(const struct _SFSnortPacket *);
    uint8_t    (*orig_iph_ret_hlen)(const struct _SFSnortPacket *);
    char version;
} IPH_API;

typedef enum {
    PSEUDO_PKT_IP,
    PSEUDO_PKT_TCP,
    PSEUDO_PKT_DCE_RPKT,
    PSEUDO_PKT_SMB_SEG,
    PSEUDO_PKT_DCE_SEG,
    PSEUDO_PKT_DCE_FRAG,
    PSEUDO_PKT_SMB_TRANS,
    PSEUDO_PKT_PS,
    PSEUDO_PKT_SDF,
    PSEUDO_PKT_MAX
} PseudoPacketType;

#include "ipv6_port.h"

#define IP6_HEADER_LEN  40

#define IPH_API_V4 4
#define IPH_API_V6 6

extern IPH_API ip4;
extern IPH_API ip6;

#define iph_is_valid(p) ((p)->family != NO_IP)

#define NO_IP 0

#define IP6_HDR_LEN     40

typedef struct _MplsHdr
{
    uint32_t label;
    uint8_t  exp;
    uint8_t  bos;
    uint8_t  ttl;
} MplsHdr;

typedef struct _H2PriSpec
{
    uint32_t stream_id;
    uint32_t weight;
    uint8_t  exclusive;
} H2PriSpec;

typedef struct _H2Hdr
{
    uint32_t length;
    uint32_t stream_id;
    uint8_t  type;
    uint8_t  flags;
    uint8_t  reserved;
    H2PriSpec pri;
} H2Hdr;

#define MAX_PROTO_LAYERS 32

typedef struct {
    PROTO_ID proto_id;
    uint16_t proto_length;
    uint8_t* proto_start;
} ProtoLayer;

// for backwards compatibility with VRT .so rules
#define stream_session_ptr stream_session

// forward declaration for snort list management type
struct sfSDList;

// forward declaration for snort expected session created due to this packet.
struct _ExpectNode;

// NOTE: Any modifcation to _SFSnortPacket, please bump up REQ_ENGINE_LIB_MINOR 
typedef struct _SFSnortPacket
{
    const SFDAQ_PktHdr_t *pkt_header; /* Is this GPF'd? */
    const uint8_t *pkt_data;

    void *ether_arp_header;
    const EtherHeader *ether_header;
    const VlanHeader *vlan_tag_header;
    void *ether_header_llc;
    void *ether_header_other;
    const void *ppp_over_ether_header;
    const void *gre_header;
    uint32_t *mpls;
    const CiscoMetaHdr *cmdh;                /* Cisco Metadata Header */

    const IPV4Header *ip4_header, *orig_ip4_header;
    const IPV4Header *inner_ip4_header;
    const IPV4Header *outer_ip4_header;
    const TCPHeader *tcp_header, *orig_tcp_header;
    const UDPHeader *udp_header, *orig_udp_header;
    const UDPHeader *inner_udph;   /* if Teredo + UDP, this will be the inner UDP header */
    const UDPHeader *outer_udph;   /* if Teredo + UDP, this will be the outer UDP header */
    const ICMPHeader *icmp_header, *orig_icmp_header;

    const uint8_t *payload;
    const uint8_t *ip_payload;
    const uint8_t *outer_ip_payload;

    void *stream_session;
    void *fragmentation_tracking_ptr;

    IP4Hdr *ip4h, *orig_ip4h;
    IP6Hdr *ip6h, *orig_ip6h;
    ICMP6Hdr *icmp6h, *orig_icmp6h;

    IPH_API* iph_api;
    IPH_API* orig_iph_api;
    IPH_API* outer_iph_api;
    IPH_API* outer_orig_iph_api;

    int family;
    int orig_family;
    int outer_family;

    PreprocEnableMask preprocessor_bit_mask;

    uint64_t flags;

    uint32_t xtradata_mask;

    uint16_t proto_bits;

    uint16_t payload_size;
    uint16_t ip_payload_size;
    uint16_t normalized_payload_size;
    uint16_t actual_ip_length;
    uint16_t outer_ip_payload_size;

    uint16_t ip_fragment_offset;
    uint16_t ip_frag_length;
    uint16_t ip4_options_length;
    uint16_t tcp_options_length;

    uint16_t src_port;
    uint16_t dst_port;
    uint16_t orig_src_port;
    uint16_t orig_dst_port;

    int16_t application_protocol_ordinal;

    uint8_t ip_fragmented;
    uint8_t ip_more_fragments;
    uint8_t ip_dont_fragment;
    uint8_t ip_reserved;

    uint8_t num_ip_options;
    uint8_t num_tcp_options;
    uint8_t num_ip6_extensions;
    uint8_t ip6_frag_extension;

    uint8_t invalid_flags;
    uint8_t encapsulated;
    uint8_t GTPencapsulated;
    uint8_t GREencapsulated;
    uint8_t IPnIPencapsulated;
    uint8_t non_ip_pkt;
    uint8_t next_layer_index;

#ifndef NO_NON_ETHER_DECODER
    const void *fddi_header;
    void *fddi_saps;
    void *fddi_sna;
    void *fddi_iparp;
    void *fddi_other;

    const void *tokenring_header;
    void *tokenring_header_llc;
    void *tokenring_header_mr;

    void *pflog1_header;
    void *pflog2_header;
    void *pflog3_header;
    void *pflog4_header;

#ifdef DLT_LINUX_SLL
    const void *sll_header;
#endif
#ifdef DLT_IEEE802_11
    const void *wifi_header;
#endif
    const void *ether_eapol_header;
    const void *eapol_headear;
    const uint8_t *eapol_type;
    void *eapol_key;
#endif

    IPOptions ip_options[MAX_IP_OPTIONS];
    TCPOptions tcp_options[MAX_TCP_OPTIONS];
    IP6Extension *ip6_extensions;
    CiscoMetaOpt *cmd_options;    /* Cisco Metadata header options */

    const uint8_t *ip_frag_start;
    const uint8_t *ip4_options_data;
    const uint8_t *tcp_options_data;

    const IP6RawHdr* raw_ip6_header;
    ProtoLayer proto_layers[MAX_PROTO_LAYERS];

    IPAddresses inner_ips, inner_orig_ips;
    IP4Hdr inner_ip4h, inner_orig_ip4h;
    IP6Hdr inner_ip6h, inner_orig_ip6h;
    IPAddresses outer_ips, outer_orig_ips;
    IP4Hdr outer_ip4h, outer_orig_ip4h;
    IP6Hdr outer_ip6h, outer_orig_ip6h;

    MplsHdr mplsHdr;
    H2Hdr   *h2Hdr;

    PseudoPacketType pseudo_type;
    uint16_t max_payload;

    /**policyId provided in configuration file. Used for correlating configuration
     * with event output
     */
    uint16_t configPolicyId;

    uint32_t iplist_id;
    unsigned char iprep_layer;

    uint8_t ps_proto;  /* Used for portscan and unified2 logging */

    uint8_t ips_os_selected;
    void    *cur_pp;

    // Expected session created due to this packet.
    struct _ExpectNode* expectedSession;
} SFSnortPacket;

#define IP_INNER_LAYER   1
#define IP_OUTTER_LAYER  0

#define PKT_ZERO_LEN offsetof(SFSnortPacket, ip_options)

#define PROTO_BIT__IP       0x0001
#define PROTO_BIT__ARP      0x0002
#define PROTO_BIT__TCP      0x0004
#define PROTO_BIT__UDP      0x0008
#define PROTO_BIT__ICMP     0x0010
#define PROTO_BIT__TEREDO   0x0020
#define PROTO_BIT__ALL      0xffff

#define IsIP(p) (IPH_IS_VALID(p))
#define IsTCP(p) (IsIP(p) && p->tcp_header)
#define IsUDP(p) (IsIP(p) && p->udp_header)
#define IsICMP(p) (IsIP(p) && p->icmp_header)

#define SET_IP4_VER(ip_header, value) \
    ((ip_header)->version_headerlength = \
     (unsigned char)(((ip_header)->version_headerlength & 0x0f) | (value << 4)))
#define SET_IP4_HLEN(ip_header, value) \
    ((ip_header)->version_headerlength = \
     (unsigned char)(((ip_header)->version_headerlength & 0xf0) | (value & 0x0f)))

#define SET_TCP_HDR_OFFSET(tcp_header, value) \
    ((tcp_header)->offset_reserved = \
     (unsigned char)(((tcp_header)->offset_reserved & 0x0f) | (value << 4)))

#define BIT(i) (0x1 << (i-1))


/* beware:  some flags are redefined in dynamic-plugins/sf_dynamic_define.h! */
#define FLAG_REBUILT_FRAG     0x00000001  /* is a rebuilt fragment */
#define FLAG_REBUILT_STREAM   0x00000002  /* is a rebuilt stream */
#define FLAG_STREAM_UNEST_UNI 0x00000004  /* is from an unestablished stream and
                                           * we've only seen traffic in one direction */
#define FLAG_STREAM_EST       0x00000008  /* is from an established stream */

#define FLAG_STREAM_INSERT    0x00000010  /* this packet has been queued for stream reassembly */
#define FLAG_STREAM_TWH       0x00000020  /* packet completes the 3-way handshake */
#define FLAG_FROM_SERVER      0x00000040  /* this packet came from the server
                                             side of a connection (TCP) */
#define FLAG_FROM_CLIENT      0x00000080  /* this packet came from the client
                                             side of a connection (TCP) */

#define FLAG_PDU_HEAD         0x00000100  /* start of PDU */
#define FLAG_PDU_TAIL         0x00000200  /* end of PDU */
#define FLAG_UNSURE_ENCAP     0x00000400  /* packet may have incorrect encapsulation layer. */
                                          /* don't alert if "next layer" is invalid. */
#define FLAG_HTTP_DECODE      0x00000800  /* this packet has normalized http */

#define FLAG_IGNORE_PORT           0x00001000  /* this packet should be ignored, based on port */
#define FLAG_NO_DETECT             0x00002000  /* this packet should not be preprocessed */
#define FLAG_ALLOW_MULTIPLE_DETECT 0x00004000  /* packet has either pipelined mime attachements */
                                               /* or pipeline http requests */
#define FLAG_PAYLOAD_OBFUSCATE     0x00008000

#define FLAG_STATELESS        0x00010000  /* Packet has matched a stateless rule */
#define FLAG_PASS_RULE        0x00020000  /* this packet has matched a pass rule */
#define FLAG_IP_RULE          0x00040000  /* this packet is being evaluated against an IP rule */
#define FLAG_IP_RULE_2ND      0x00080000  /* this packet is being evaluated against an IP rule */

#define FLAG_LOGGED           0x00100000  /* this packet has been logged */
#define FLAG_PSEUDO           0x00200000  /* is a pseudo packet */
#define FLAG_MODIFIED         0x00400000  /* packet had normalizations, etc. */
#ifdef NORMALIZER
#define FLAG_RESIZED          0x00800000  /* packet has new size; must set modified too */
#endif

/* neither of these flags will be set for (full) retransmissions or non-data segments */
/* a partial overlap results in out of sequence condition */
/* out of sequence condition is sticky */
#define FLAG_STREAM_ORDER_OK  0x01000000  /* this segment is in order, w/o gaps */
#define FLAG_STREAM_ORDER_BAD 0x02000000  /* this stream had at least one gap */
#define FLAG_REASSEMBLED_OLD  0x04000000  /* for backwards compat with so rules */

#define FLAG_IPREP_SOURCE_TRIGGERED  0x08000000
#define FLAG_IPREP_DATA_SET          0x10000000
#define FLAG_FILE_EVENT_SET          0x20000000
#define FLAG_EARLY_REASSEMBLY 0x40000000  /* this packet. part of the expected stream, should have stream reassembly set */
#define FLAG_RETRANSMIT       0x80000000  /* this packet is identified as re-transmitted one */
#define FLAG_PURGE            0x0100000000 /* Stream will not flush the data */
#define FLAG_H1_ABORT         0x0200000000  /* Used by H1 and H2 paf */
#define FLAG_UPGRADE_PROTO    0x0400000000  /* Used by H1 paf */
#define FLAG_PSEUDO_FLUSH     0x0800000000
#define FLAG_FAST_BLOCK       0x1000000000
#define FLAG_EVAL_DROP        0x2000000000  /* Packet with FLAG_EVAL_DROP is evaluated if it is needed to dropped */


#define FLAG_PDU_FULL (FLAG_PDU_HEAD | FLAG_PDU_TAIL)

#define REASSEMBLED_PACKET_FLAGS (FLAG_REBUILT_STREAM|FLAG_REASSEMBLED_OLD)

#define SFTARGET_UNKNOWN_PROTOCOL -1

static inline int PacketWasCooked(const SFSnortPacket* p)
{
    return ( p->flags & FLAG_PSEUDO ) != 0;
}

static inline int IsPortscanPacket(const SFSnortPacket *p)
{
    return ((p->flags & FLAG_PSEUDO) && (p->pseudo_type == PSEUDO_PKT_PS));
}

static inline uint8_t GetEventProto(const SFSnortPacket *p)
{
    if (IsPortscanPacket(p))
        return p->ps_proto;
    return IPH_IS_VALID(p) ? GET_IPH_PROTO(p) : 0;
}

static inline int PacketHasFullPDU (const SFSnortPacket* p)
{
    return ( (p->flags & FLAG_PDU_FULL) == FLAG_PDU_FULL );
}

static inline int PacketHasStartOfPDU (const SFSnortPacket* p)
{
    return ( (p->flags & FLAG_PDU_HEAD) != 0 );
}

static inline int PacketHasPAFPayload (const SFSnortPacket* p)
{
    return ( (p->flags & FLAG_REBUILT_STREAM) || (p->flags & FLAG_PDU_TAIL) );
}

static inline void SetExtraData (SFSnortPacket* p, uint32_t xid)
{
    p->xtradata_mask |= BIT(xid);
}

#endif /* _SF_SNORT_PACKET_H_ */

