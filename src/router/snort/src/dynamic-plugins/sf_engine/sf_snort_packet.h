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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

#define VLAN_HDR_LEN  4

// for vrt backwards compatibility
#define pcap_header pkt_header

typedef DAQ_PktHdr_t SFDAQ_PktHdr_t;

typedef struct _VlanHeader
{
    u_int16_t vth_pri_cfi_vlan;
    u_int16_t vth_proto;  /* protocol field... */

} VlanHeader;

//#define NO_NON_ETHER_DECODER
#define ETHER_HDR_LEN  14
#define ETHERNET_TYPE_IP    0x0800
#define ETHERNET_TYPE_IPV6  0x86dd
#define ETHERNET_TYPE_8021Q 0x8100

typedef struct _EtherHeader
{
    u_int8_t ether_destination[6];
    u_int8_t ether_source[6];
    u_int16_t ethernet_type;

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

typedef struct _IPV4Header
{
    u_int8_t version_headerlength;
    u_int8_t type_service;
    u_int16_t data_length;
    u_int16_t identifier;
    u_int16_t offset;
    u_int8_t time_to_live;
    u_int8_t proto;
    u_int16_t checksum;
    struct in_addr source;
    struct in_addr destination;
} IPV4Header;

#define MAX_IP_OPTIONS 40
#define MAX_IP6_EXTENSIONS 40
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
    u_int8_t option_code;
    u_int8_t length;
    u_int8_t *option_data;
} IPOptions;


#define TCP_HDR_LEN  20

typedef struct _TCPHeader
{
    u_int16_t source_port;
    u_int16_t destination_port;
    u_int32_t sequence;
    u_int32_t acknowledgement;
    u_int8_t offset_reserved;
    u_int8_t flags;
    u_int16_t window;
    u_int16_t checksum;
    u_int16_t urgent_pointer;
} TCPHeader;

#define TCPHEADER_FIN  0x01
#define TCPHEADER_SYN  0x02
#define TCPHEADER_RST  0x04
#define TCPHEADER_PUSH 0x08
#define TCPHEADER_ACK  0x10
#define TCPHEADER_URG  0x20
#define TCPHEADER_RES2 0x40
#define TCPHEADER_RES1 0x80
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
    u_int16_t source_port;
    u_int16_t destination_port;
    u_int16_t data_length;
    u_int16_t checksum;
} UDPHeader;

typedef struct _ICMPSequenceID
{
    u_int16_t id;
    u_int16_t seq;
} ICMPSequenceID;

typedef struct _ICMPHeader
{
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;

    union
    {
        /* type 12 */
        u_int8_t parameter_problem_ptr; 

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
            u_int16_t voidInfo;
            u_int16_t next_mtu;
        } path_mtu;

        /* type 9 */
        struct router_advertisement 
        {
            u_int8_t number_addrs;
            u_int8_t entry_size;
            u_int16_t lifetime;
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
            u_int32_t orig;
            u_int32_t receive;
            u_int32_t transmit;
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
            u_int32_t addr;
            u_int32_t preference;
        } router_address;

        /* type 17, 18 */
        u_int32_t mask;

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
    u_int8_t option_type;
    const u_int8_t *option_data;
} IP6Extension;

typedef struct _IPv4Hdr
{
    u_int8_t ip_verhl;      /* version & header length */
    u_int8_t ip_tos;        /* type of service */
    u_int16_t ip_len;       /* datagram length */
    u_int16_t ip_id;        /* identification  */
    u_int16_t ip_off;       /* fragment offset */
    u_int8_t ip_ttl;        /* time to live field */
    u_int8_t ip_proto;      /* datagram protocol */ 
    u_int16_t ip_csum;      /* checksum */
    sfip_t ip_src;          /* source IP */
    sfip_t ip_dst;          /* dest IP */
} IP4Hdr;

typedef struct _IP6RawHdr
{
    u_int32_t vcl;          // version, class, and label */
    u_int16_t payload_len;  // length of the payload */
    u_int8_t  next_header;  // same values as ip4 protocol field + new ip6 values
    u_int8_t  hop_limit;    // same usage as ip4 ttl

    struct in6_addr src_addr;
    struct in6_addr dst_addr;
} IP6RawHdr;

typedef struct _IPv6Hdr
{ 
    u_int32_t vcl;      /* version, class, and label */
    u_int16_t len;      /* length of the payload */
    u_int8_t  next;     /* next header
                         * Uses the same flags as
                         * the IPv4 protocol field */
    u_int8_t  hop_lmt;  /* hop limit */ 
    sfip_t ip_src;
    sfip_t ip_dst;
} IP6Hdr; 

typedef struct _IP6FragHdr 
{
    u_int8_t   ip6f_nxt;     /* next header */
    u_int8_t   ip6f_reserved;    /* reserved field */
    u_int16_t  ip6f_offlg;   /* offset, reserved, and flag */
    u_int32_t  ip6f_ident;   /* identification */
} IP6FragHdr;

typedef struct _ICMP6
{
    u_int8_t type;
    u_int8_t code;
    u_int16_t csum;

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


/* IPHeader access calls */
sfip_t *    ip4_ret_src(struct _SFSnortPacket *);
sfip_t *    ip4_ret_dst(struct _SFSnortPacket *);
u_int16_t   ip4_ret_tos(struct _SFSnortPacket *);
u_int8_t    ip4_ret_ttl(struct _SFSnortPacket *);
u_int16_t   ip4_ret_len(struct _SFSnortPacket *);
u_int32_t   ip4_ret_id(struct _SFSnortPacket *);
u_int8_t    ip4_ret_proto(struct _SFSnortPacket *);
u_int16_t   ip4_ret_off(struct _SFSnortPacket *);
u_int8_t    ip4_ret_ver(struct _SFSnortPacket *);
u_int8_t    ip4_ret_hlen(struct _SFSnortPacket *);

sfip_t *    orig_ip4_ret_src(struct _SFSnortPacket *);
sfip_t *    orig_ip4_ret_dst(struct _SFSnortPacket *);
u_int16_t   orig_ip4_ret_tos(struct _SFSnortPacket *);
u_int8_t    orig_ip4_ret_ttl(struct _SFSnortPacket *);
u_int16_t   orig_ip4_ret_len(struct _SFSnortPacket *);
u_int32_t   orig_ip4_ret_id(struct _SFSnortPacket *);
u_int8_t    orig_ip4_ret_proto(struct _SFSnortPacket *);
u_int16_t   orig_ip4_ret_off(struct _SFSnortPacket *);
u_int8_t    orig_ip4_ret_ver(struct _SFSnortPacket *);
u_int8_t    orig_ip4_ret_hlen(struct _SFSnortPacket *);

sfip_t *    ip6_ret_src(struct _SFSnortPacket *);
sfip_t *    ip6_ret_dst(struct _SFSnortPacket *);
u_int16_t   ip6_ret_toc(struct _SFSnortPacket *);
u_int8_t    ip6_ret_hops(struct _SFSnortPacket *);
u_int16_t   ip6_ret_len(struct _SFSnortPacket *);
u_int32_t   ip6_ret_id(struct _SFSnortPacket *);
u_int8_t    ip6_ret_next(struct _SFSnortPacket *);
u_int16_t   ip6_ret_off(struct _SFSnortPacket *);
u_int8_t    ip6_ret_ver(struct _SFSnortPacket *);
u_int8_t    ip6_ret_hlen(struct _SFSnortPacket *);

sfip_t *    orig_ip6_ret_src(struct _SFSnortPacket *);
sfip_t *    orig_ip6_ret_dst(struct _SFSnortPacket *);
u_int16_t   orig_ip6_ret_toc(struct _SFSnortPacket *);
u_int8_t    orig_ip6_ret_hops(struct _SFSnortPacket *);
u_int16_t   orig_ip6_ret_len(struct _SFSnortPacket *);
u_int32_t   orig_ip6_ret_id(struct _SFSnortPacket *);
u_int8_t    orig_ip6_ret_next(struct _SFSnortPacket *);
u_int16_t   orig_ip6_ret_off(struct _SFSnortPacket *);
u_int8_t    orig_ip6_ret_ver(struct _SFSnortPacket *);
u_int8_t    orig_ip6_ret_hlen(struct _SFSnortPacket *);

typedef struct _IPH_API 
{
    sfip_t *    (*iph_ret_src)(struct _SFSnortPacket *);
    sfip_t *    (*iph_ret_dst)(struct _SFSnortPacket *);
    u_int16_t   (*iph_ret_tos)(struct _SFSnortPacket *);
    u_int8_t    (*iph_ret_ttl)(struct _SFSnortPacket *);
    u_int16_t   (*iph_ret_len)(struct _SFSnortPacket *);
    u_int32_t   (*iph_ret_id)(struct _SFSnortPacket *);
    u_int8_t    (*iph_ret_proto)(struct _SFSnortPacket *);
    u_int16_t   (*iph_ret_off)(struct _SFSnortPacket *);
    u_int8_t    (*iph_ret_ver)(struct _SFSnortPacket *);
    u_int8_t    (*iph_ret_hlen)(struct _SFSnortPacket *);

    sfip_t *    (*orig_iph_ret_src)(struct _SFSnortPacket *);
    sfip_t *    (*orig_iph_ret_dst)(struct _SFSnortPacket *);
    u_int16_t   (*orig_iph_ret_tos)(struct _SFSnortPacket *);
    u_int8_t    (*orig_iph_ret_ttl)(struct _SFSnortPacket *);
    u_int16_t   (*orig_iph_ret_len)(struct _SFSnortPacket *);
    u_int16_t   (*orig_iph_ret_id)(struct _SFSnortPacket *);
    u_int8_t    (*orig_iph_ret_proto)(struct _SFSnortPacket *);
    u_int16_t   (*orig_iph_ret_off)(struct _SFSnortPacket *);
    u_int8_t    (*orig_iph_ret_ver)(struct _SFSnortPacket *);
    u_int8_t    (*orig_iph_ret_hlen)(struct _SFSnortPacket *);
    char version;
} IPH_API;

#ifdef SUP_IP6

#include "ipv6_port.h"

#define IP6_HEADER_LEN  40

#define IPH_API_V4 4
#define IPH_API_V6 6

extern IPH_API ip4;
extern IPH_API ip6;

#define iph_is_valid(p) (p->family != NO_IP)

#define NO_IP 0

#define IP6_HDR_LEN     40
#endif

typedef struct _MplsHdr
{
    u_int32_t label;
    u_int8_t  exp;
    u_int8_t  bos; 
    u_int8_t  ttl;
} MplsHdr;

#define MAX_PROTO_LAYERS 32

typedef enum {
    PROTOCOL_ETH,
    PROTOCOL_IP4,
    PROTOCOL_ICMP4,
    PROTOCOL_UDP,
    PROTOCOL_TCP,
    PROTOCOL_MAX
} PROTOCOL_ID;

typedef struct {
    PROTOCOL_ID proto_id;
    uint16_t proto_length;
    uint8_t* proto_start;
} ProtoLayer;

typedef struct _SFSnortPacket
{
    const SFDAQ_PktHdr_t *pkt_header; /* Is this GPF'd? */
    const u_int8_t *pkt_data;

    void *ether_arp_header;
    const EtherHeader *ether_header;
    const void *vlan_tag_header;
    void *ether_header_llc;
    void *ether_header_other;
    const void *gre_header;
    u_int32_t *mpls;

    const IPV4Header *ip4_header, *orig_ip4_header;
    const IPV4Header *inner_ip4_header;
    const IPV4Header *outer_ip4_header;
    const TCPHeader *tcp_header, *orig_tcp_header;
    const UDPHeader *udp_header, *orig_udp_header;
    const UDPHeader *inner_udph;   /* if Teredo + UDP, this will be the inner UDP header */
    const UDPHeader *outer_udph;   /* if Teredo + UDP, this will be the outer UDP header */
    const ICMPHeader *icmp_header, *orig_icmp_header;

    const u_int8_t *payload;
    const u_int8_t *ip_payload;
    const u_int8_t *outer_ip_payload;
    const u_int8_t *ip_frag_start;
    const u_int8_t *ip4_options_data;
    const u_int8_t *tcp_options_data;

    void *stream_session_ptr;
    void *fragmentation_tracking_ptr;
    void *flow_ptr;
    void *stream_ptr;

    IP4Hdr *ip4h, *orig_ip4h;
    IP6Hdr *ip6h, *orig_ip6h;
    ICMP6Hdr *icmp6h, *orig_icmp6h;

    IPH_API* iph_api;
    IPH_API* orig_iph_api;
    IPH_API* outer_iph_api;
    IPH_API* outer_orig_iph_api;

    IP4Hdr inner_ip4h, inner_orig_ip4h;
    IP6Hdr inner_ip6h, inner_orig_ip6h;
    IP4Hdr outer_ip4h, outer_orig_ip4h;
    IP6Hdr outer_ip6h, outer_orig_ip6h;

    MplsHdr   mplsHdr;

    int family;
    int orig_family;
    int outer_family;
    int number_bytes_to_check;

    //int ip_payload_length;
    //int ip_payload_offset;

    u_int32_t preprocessor_bit_mask;
    u_int32_t preproc_reassembly_pkt_bit_mask;

    u_int32_t pcap_cap_len;
    u_int32_t http_pipeline_count;
    u_int32_t flags;
    u_int16_t proto_bits;
    u_int16_t data_flags;

    u_int16_t payload_size;
    u_int16_t ip_payload_size;
    u_int16_t normalized_payload_size;
    u_int16_t actual_ip_length;
    u_int16_t outer_ip_payload_size;

    u_int16_t ip_fragment_offset;
    u_int16_t ip_frag_length;
    u_int16_t ip4_options_length;
    u_int16_t tcp_options_length;

    u_int16_t src_port;
    u_int16_t dst_port;
    u_int16_t orig_src_port;
    u_int16_t orig_dst_port;

    int16_t application_protocol_ordinal;

    u_int8_t ip_fragmented;
    u_int8_t ip_more_fragments;
    u_int8_t ip_dont_fragment;
    u_int8_t ip_reserved;

    u_int8_t num_uris;
    u_int8_t invalid_flags;
    u_int8_t encapsulated;

    u_int8_t num_ip_options;
    u_int8_t num_tcp_options;
    u_int8_t num_ip6_extensions;
    u_int8_t ip6_frag_extension;

    u_char ip_last_option_invalid_flag;
    u_char tcp_last_option_invalid_flag;

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

#ifdef DLT_LINUX_SLL
    const void *sll_header;
#endif
#ifdef DLT_IEEE802_11
    const void *wifi_header;
#endif
    const void *ppp_over_ether_header;

    const void *ether_eapol_header;
    const void *eapol_headear;
    const u_int8_t *eapol_type;
    void *eapol_key;
#endif

    IPOptions ip_options[MAX_IP_OPTIONS];
    TCPOptions tcp_options[MAX_TCP_OPTIONS];
    IP6Extension ip6_extensions[MAX_IP6_EXTENSIONS];

    const IP6RawHdr* raw_ip6_header;
    ProtoLayer proto_layers[MAX_PROTO_LAYERS];
    uint16_t max_payload;

    /**policyId provided in configuration file. Used for correlating configuration 
     * with event output
     */
    uint16_t configPolicyId;

} SFSnortPacket;

#define PKT_ZERO_LEN offsetof(SFSnortPacket, ip_options)

#define PROTO_BIT__IP       0x0001
#define PROTO_BIT__ARP      0x0002
#define PROTO_BIT__TCP      0x0004
#define PROTO_BIT__UDP      0x0008
#define PROTO_BIT__ICMP     0x0010
#define PROTO_BIT__TEREDO   0x0020
#define PROTO_BIT__ALL      0xffff

#define DATA_FLAGS_TRUE_IP  0x0001
#define DATA_FLAGS_GZIP     0x0002
#define DATA_FLAGS_RESP_BODY 0x0004

#define IsIP(p) (IPH_IS_VALID(p))
#define IsTCP(p) (IsIP(p) && (GET_IPH_PROTO(p) == IPPROTO_TCP))
#define IsUDP(p) (IsIP(p) && (GET_IPH_PROTO(p) == IPPROTO_UDP))
#define IsICMP(p) (IsIP(p) && (GET_IPH_PROTO(p) == IPPROTO_ICMP))

#define SET_IP4_VER(ip_header, value) \
    ((ip_header)->version_headerlength = \
     (unsigned char)(((ip_header)->version_headerlength & 0x0f) | (value << 4)))
#define SET_IP4_HLEN(ip_header, value) \
    ((ip_header)->version_headerlength = \
     (unsigned char)(((ip_header)->version_headerlength & 0xf0) | (value & 0x0f)))

#define SET_TCP_HDR_OFFSET(tcp_header, value) \
    ((tcp_header)->offset_reserved = \
     (unsigned char)(((tcp_header)->offset_reserved & 0x0f) | (value << 4)))

#define FLAG_REBUILT_FRAG     0x00000001
#define FLAG_REBUILT_STREAM   0x00000002
#define FLAG_STREAM_UNEST_UNI 0x00000004
#define FLAG_PSEUDO           0x00000008

//--------------------------------------
// beware:  these are redefined in dynamic-plugins/sf_dynamic_define.h!
#define FLAG_STREAM_EST       0x00000010
#define FLAG_STATELESS        0x00000020  /* packet has matched a stateless rule */
#define FLAG_FROM_SERVER      0x00000040	
#define FLAG_FROM_CLIENT      0x00000080
//--------------------------------------

#define FLAG_HTTP_DECODE      0x00000100
#define FLAG_OBFUSCATED       0x00000200
#define FLAG_STREAM_INSERT    0x00000400
#define FLAG_ALT_DECODE       0x00000800

#define FLAG_STREAM_TWH       0x00001000
#define FLAG_IGNORE_PORT      0x00002000  /* this packet should be ignored, based on port */
#define FLAG_PASS_RULE        0x00004000  /* this packet has matched a pass rule */
#define FLAG_NO_DETECT        0x00008000  /* this packet should not be preprocessed */

#define FLAG_PREPROC_RPKT     0x00010000  /* set in original packet to indicate a pp
                                           * has a reassembled packet */
#define FLAG_DCE_RPKT         0x00020000  /* this is a DCE/RPC reassembled packet */
#define FLAG_IP_RULE          0x00040000  /* this packet being evaluated against an ip rule */
#define FLAG_IP_RULE_2ND      0x00080000  /* this packet is being evaluated against an IP rule */

#define FLAG_SMB_SEG          0x00100000  /* this is an SMB desegmented packet */
#define FLAG_DCE_SEG          0x00200000  /* this is a DCE/RPC desegmented packet */
#define FLAG_DCE_FRAG         0x00400000  /* this is a DCE/RPC defragmented packet */
#define FLAG_SMB_TRANS        0x00800000  /* this is an SMB Transact reassembled packet */

#define FLAG_DCE_PKT          0x01000000  /* a DCE packet processed by DCE/RPC pp */
#define FLAG_RPC_PKT          0x02000000  /* an ONC RPC packet processed by rpc decode pp */
#define FLAG_LOGGED           0x04000000  /* this packet has been logged */
#ifdef NORMALIZER
#define FLAG_RESIZED          0x08000000  /* packet has new size; must set modified too */
#endif
#define FLAG_MODIFIED         0x10000000  /* packet had normalizations, etc. */

#define FLAG_HTTP_RESP_BODY   0x20000000  /* packet contains non-zipped HTTP response Body */
#define FLAG_ALLOW_MULTIPLE_DETECT 0x40000000

#define SFTARGET_UNKNOWN_PROTOCOL -1

static INLINE int PacketWasCooked(SFSnortPacket* p)
{
    return ( p->flags &
        ( FLAG_REBUILT_STREAM | FLAG_REBUILT_FRAG |
          FLAG_DCE_RPKT | FLAG_DCE_SEG | FLAG_DCE_FRAG |
          FLAG_SMB_SEG | FLAG_SMB_TRANS | FLAG_PSEUDO) ) != 0;
}

/* Only include application layer reassembled data
 * flags here - no PKT_REBUILT_FRAG */
#define REASSEMBLED_PACKET_FLAGS \
    (FLAG_REBUILT_STREAM|FLAG_SMB_SEG|FLAG_DCE_SEG|FLAG_DCE_FRAG|FLAG_SMB_TRANS)

#endif /* _SF_SNORT_PACKET_H_ */

