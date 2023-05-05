/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __GENERATORS_H__
#define __GENERATORS_H__

#define GENERATOR_SNORT_ENGINE        1

#define GENERATOR_TAG                 2
#define    TAG_LOG_PKT                1

#define GENERATOR_SPP_BO            105
#define     BO_TRAFFIC_DETECT           1
#define     BO_CLIENT_TRAFFIC_DETECT    2
#define     BO_SERVER_TRAFFIC_DETECT    3
#define     BO_SNORT_BUFFER_ATTACK      4

#define GENERATOR_SPP_RPC_DECODE    106
#define     RPC_FRAG_TRAFFIC                1
#define     RPC_MULTIPLE_RECORD             2
#define     RPC_LARGE_FRAGSIZE              3
#define     RPC_INCOMPLETE_SEGMENT          4
#define     RPC_ZERO_LENGTH_FRAGMENT        5

#define GENERATOR_SPP_ARPSPOOF      112
#define     ARPSPOOF_UNICAST_ARP_REQUEST         1
#define     ARPSPOOF_ETHERFRAME_ARP_MISMATCH_SRC  2
#define     ARPSPOOF_ETHERFRAME_ARP_MISMATCH_DST  3
#define     ARPSPOOF_ARP_CACHE_OVERWRITE_ATTACK   4

#define GENERATOR_SNORT_DECODE      116
#define     DECODE_NOT_IPV4_DGRAM                 1
#define     DECODE_IPV4_INVALID_HEADER_LEN        2
#define     DECODE_IPV4_DGRAM_LT_IPHDR            3
#define     DECODE_IPV4OPT_BADLEN                 4
#define     DECODE_IPV4OPT_TRUNCATED              5
#define     DECODE_IPV4_DGRAM_GT_CAPLEN           6

#define     DECODE_TCP_DGRAM_LT_TCPHDR            45
#define     DECODE_TCP_INVALID_OFFSET             46
#define     DECODE_TCP_LARGE_OFFSET               47

#define     DECODE_TCPOPT_BADLEN                  54
#define     DECODE_TCPOPT_TRUNCATED               55
#define     DECODE_TCPOPT_TTCP                    56
#define     DECODE_TCPOPT_OBSOLETE                57
#define     DECODE_TCPOPT_EXPERIMENT              58
#define     DECODE_TCPOPT_WSCALE_INVALID          59

#define     DECODE_UDP_DGRAM_LT_UDPHDR            95
#define     DECODE_UDP_DGRAM_INVALID_LENGTH       96
#define     DECODE_UDP_DGRAM_SHORT_PACKET         97
#define     DECODE_UDP_DGRAM_LONG_PACKET          98

#define     DECODE_ICMP_DGRAM_LT_ICMPHDR          105
#define     DECODE_ICMP_DGRAM_LT_TIMESTAMPHDR     106
#define     DECODE_ICMP_DGRAM_LT_ADDRHDR          107

#define     DECODE_ARP_TRUNCATED                  109
#define     DECODE_EAPOL_TRUNCATED                110
#define     DECODE_EAPKEY_TRUNCATED               111
#define     DECODE_EAP_TRUNCATED                  112

#define     DECODE_BAD_PPPOE                      120
#define     DECODE_BAD_VLAN                       130
#define     DECODE_BAD_VLAN_ETHLLC                131
#define     DECODE_BAD_VLAN_OTHER                 132
#define     DECODE_BAD_80211_ETHLLC               133
#define     DECODE_BAD_80211_OTHER                134

#define     DECODE_BAD_TRH                        140
#define     DECODE_BAD_TR_ETHLLC                  141
#define     DECODE_BAD_TR_MR_LEN                  142
#define     DECODE_BAD_TRHMR                      143

#define     DECODE_BAD_TRAFFIC_LOOPBACK           150
#define     DECODE_BAD_TRAFFIC_SAME_SRCDST        151

#ifdef GRE
#define     DECODE_GRE_DGRAM_LT_GREHDR            160
#define     DECODE_GRE_MULTIPLE_ENCAPSULATION     161
#define     DECODE_GRE_INVALID_VERSION            162
#define     DECODE_GRE_INVALID_HEADER             163
#define     DECODE_GRE_V1_INVALID_HEADER          164
#define     DECODE_GRE_TRANS_DGRAM_LT_TRANSHDR    165
#endif  /* GRE */

/** MPLS takes 170 block **/
#define     DECODE_BAD_MPLS                       170
#define     DECODE_BAD_MPLS_LABEL0                171
#define     DECODE_BAD_MPLS_LABEL1                172
#define     DECODE_BAD_MPLS_LABEL2                173
#define     DECODE_BAD_MPLS_LABEL3                174
#define     DECODE_MPLS_RESERVED_LABEL            175
#define     DECODE_MPLS_LABEL_STACK               176

#define     DECODE_ICMP_ORIG_IP_TRUNCATED         250
#define     DECODE_ICMP_ORIG_IP_VER_MISMATCH      251
#define     DECODE_ICMP_ORIG_DGRAM_LT_ORIG_IP     252
#define     DECODE_ICMP_ORIG_PAYLOAD_LT_64        253
#define     DECODE_ICMP_ORIG_PAYLOAD_GT_576       254
#define     DECODE_ICMP_ORIG_IP_WITH_FRAGOFFSET   255

#define     DECODE_IPV6_MIN_TTL                   270
#define     DECODE_IPV6_IS_NOT                    271
#define     DECODE_IPV6_TRUNCATED_EXT             272
#define     DECODE_IPV6_TRUNCATED                 273
#define     DECODE_IPV6_DGRAM_LT_IPHDR            274
#define     DECODE_IPV6_DGRAM_GT_CAPLEN           275
#define     DECODE_IPV6_DST_ZERO                  276
#define     DECODE_IPV6_SRC_MULTICAST             277
#define     DECODE_IPV6_DST_RESERVED_MULTICAST    278
#define     DECODE_IPV6_BAD_OPT_TYPE              279
#define     DECODE_IPV6_BAD_MULTICAST_SCOPE       280
#define     DECODE_IPV6_BAD_NEXT_HEADER           281
#define     DECODE_IPV6_ROUTE_AND_HOPBYHOP        282
#define     DECODE_IPV6_TWO_ROUTE_HEADERS         283

#define     DECODE_ICMPV6_TOO_BIG_BAD_MTU         285
#define     DECODE_ICMPV6_UNREACHABLE_NON_RFC_2463_CODE    286
#define     DECODE_ICMPV6_SOLICITATION_BAD_CODE   287
#define     DECODE_ICMPV6_ADVERT_BAD_CODE         288
#define     DECODE_ICMPV6_SOLICITATION_BAD_RESERVED     289
#define     DECODE_ICMPV6_ADVERT_BAD_REACHABLE    290

#define     DECODE_IPV6_TUNNELED_IPV4_TRUNCATED   291
#define     DECODE_IPV6_DSTOPTS_WITH_ROUTING      292
#define     DECODE_IP_MULTIPLE_ENCAPSULATION      293

#define     DECODE_ESP_HEADER_TRUNC               294
#define     DECODE_IPV6_BAD_OPT_LEN               295
#define     DECODE_IPV6_UNORDERED_EXTENSIONS      296

#define     DECODE_GTP_MULTIPLE_ENCAPSULATION     297
#define     DECODE_GTP_BAD_LEN                    298

#define     DECODE_DECODING_DEPTH_EXCEEDED        300

//-----------------------------------------------------
// remember to add rules to preproc_rules/decoder.rules
// add the new decoder rules to the following enum.

#define     DECODE_START_INDEX                    400

enum {
    DECODE_TCP_XMAS = DECODE_START_INDEX,
    DECODE_TCP_NMAP_XMAS,
    DECODE_DOS_NAPTHA,
    DECODE_SYN_TO_MULTICAST,
    DECODE_ZERO_TTL,
    DECODE_BAD_FRAGBITS,
    DECODE_UDP_IPV6_ZERO_CHECKSUM,
    DECODE_IP4_LEN_OFFSET,
    DECODE_IP4_SRC_THIS_NET,
    DECODE_IP4_DST_THIS_NET,
    DECODE_IP4_SRC_MULTICAST,
    DECODE_IP4_SRC_RESERVED,
    DECODE_IP4_DST_RESERVED,
    DECODE_IP4_SRC_BROADCAST,
    DECODE_IP4_DST_BROADCAST,
    DECODE_ICMP4_DST_MULTICAST,
    DECODE_ICMP4_DST_BROADCAST,
    DECODE_ICMP4_TYPE_OTHER = 418,
    DECODE_TCP_BAD_URP,
    DECODE_TCP_SYN_FIN,
    DECODE_TCP_SYN_RST,
    DECODE_TCP_MUST_ACK,
    DECODE_TCP_NO_SYN_ACK_RST,
    DECODE_ETH_HDR_TRUNC,
    DECODE_IP4_HDR_TRUNC,
    DECODE_ICMP4_HDR_TRUNC,
    DECODE_ICMP6_HDR_TRUNC,
    DECODE_IP4_MIN_TTL,
    DECODE_IP6_ZERO_HOP_LIMIT,
    DECODE_IP4_DF_OFFSET,
    DECODE_ICMP6_TYPE_OTHER,
    DECODE_ICMP6_DST_MULTICAST,
    DECODE_TCP_SHAFT_SYNFLOOD,
    DECODE_ICMP_PING_NMAP,
    DECODE_ICMP_ICMPENUM,
    DECODE_ICMP_REDIRECT_HOST,
    DECODE_ICMP_REDIRECT_NET,
    DECODE_ICMP_TRACEROUTE_IPOPTS,
    DECODE_ICMP_SOURCE_QUENCH,
    DECODE_ICMP_BROADSCAN_SMURF_SCANNER,
    DECODE_ICMP_DST_UNREACH_ADMIN_PROHIBITED,
    DECODE_ICMP_DST_UNREACH_DST_HOST_PROHIBITED,
    DECODE_ICMP_DST_UNREACH_DST_NET_PROHIBITED,
    DECODE_IP_OPTION_SET,
    DECODE_UDP_LARGE_PACKET,
    DECODE_TCP_PORT_ZERO,
    DECODE_UDP_PORT_ZERO,
    DECODE_IP_RESERVED_FRAG_BIT,
    DECODE_IP_UNASSIGNED_PROTO,
    DECODE_IP_BAD_PROTO,
    DECODE_ICMP_PATH_MTU_DOS,
    DECODE_ICMP_DOS_ATTEMPT,
    DECODE_IPV6_ISATAP_SPOOF,
    DECODE_PGM_NAK_OVERFLOW,
    DECODE_IGMP_OPTIONS_DOS,
    DECODE_IP6_EXCESS_EXT_HDR,
    DECODE_ICMPV6_UNREACHABLE_NON_RFC_4443_CODE,
    DECODE_IPV6_BAD_FRAG_PKT,
    DECODE_ZERO_LENGTH_FRAG,
    DECODE_ICMPV6_NODE_INFO_BAD_CODE,
    DECODE_IPV6_ROUTE_ZERO,
    DECODE_ERSPAN_HDR_VERSION_MISMATCH,
    DECODE_ERSPAN2_DGRAM_LT_HDR,
    DECODE_ERSPAN3_DGRAM_LT_HDR,
    DECODE_AUTH_HDR_TRUNC,
    DECODE_AUTH_HDR_BAD_LEN,
    DECODE_FPATH_HDR_TRUNC,
    DECODE_CISCO_META_HDR_TRUNC,
    DECODE_CISCO_META_HDR_OPT_LEN,
    DECODE_CISCO_META_HDR_OPT_TYPE,
    DECODE_CISCO_META_HDR_SGT,
    DECODE_INDEX_MAX
};


//-----------------------------------------------------
/*
**  HttpInspect Generator IDs
**
**  IMPORTANT::
**    Whenever events are added to the internal HttpInspect
**    event queue, you must also add the event here.  The
**    trick is that whatever the number is in HttpInspect,
**    it must be +1 when you define it here.
*/
// these are client specific events
#define GENERATOR_SPP_HTTP_INSPECT_CLIENT           119
#define     HI_CLIENT_ASCII                         1   /* done */
#define     HI_CLIENT_DOUBLE_DECODE                 2   /* done */
#define     HI_CLIENT_U_ENCODE                      3   /* done */
#define     HI_CLIENT_BARE_BYTE                     4   /* done */
/* Base 36 is deprecated and essentially a noop
 * Leaving here in case anyone out there has historical data with
 * alerts of this type */
#define     HI_CLIENT_BASE36                        5   /* done */
#define     HI_CLIENT_UTF_8                         6   /* done */
#define     HI_CLIENT_IIS_UNICODE                   7   /* done */
#define     HI_CLIENT_MULTI_SLASH                   8   /* done */
#define     HI_CLIENT_IIS_BACKSLASH                 9   /* done */
#define     HI_CLIENT_SELF_DIR_TRAV                 10  /* done */
#define     HI_CLIENT_DIR_TRAV                      11  /* done */
#define     HI_CLIENT_APACHE_WS                     12  /* done */
#define     HI_CLIENT_IIS_DELIMITER                 13  /* done */
#define     HI_CLIENT_NON_RFC_CHAR                  14  /* done */
#define     HI_CLIENT_OVERSIZE_DIR                  15  /* done */
#define     HI_CLIENT_LARGE_CHUNK                   16  /* done */
#define     HI_CLIENT_PROXY_USE                     17  /* done */
#define     HI_CLIENT_WEBROOT_DIR                   18  /* done */
#define     HI_CLIENT_LONG_HDR                      19  /* done */
#define     HI_CLIENT_MAX_HEADERS                   20  /* done */
#define     HI_CLIENT_MULTIPLE_CONTLEN              21
#define     HI_CLIENT_CHUNK_SIZE_MISMATCH           22
#define     HI_CLIENT_INVALID_TRUEIP                23
#define     HI_CLIENT_MULTIPLE_HOST_HDRS            24
#define     HI_CLIENT_LONG_HOSTNAME                 25
#define     HI_CLIENT_EXCEEDS_SPACES                26
#define     HI_CLIENT_CONSECUTIVE_SMALL_CHUNK_SIZES 27
#define     HI_CLIENT_UNBOUNDED_POST                28
#define     HI_CLIENT_MULTIPLE_TRUEIP_IN_SESSION    29
#define     HI_CLIENT_BOTH_TRUEIP_XFF_HDRS          30 
#define     HI_CLIENT_UNKNOWN_METHOD                31
#define     HI_CLIENT_SIMPLE_REQUEST                32
#define     HI_CLIENT_UNESCAPED_SPACE_URI           33
#define     HI_CLIENT_PIPELINE_MAX                  34

#define     HI_CLIENT_INVALID_RANGE_UNIT_FMT        36
#define     HI_CLIENT_RANGE_NON_GET_METHOD          37
#define     HI_CLIENT_RANGE_FIELD_ERROR             38

// these are either server specific or both client / server
#define GENERATOR_SPP_HTTP_INSPECT                 120 
#define     HI_ANOM_SERVER_ALERT                    1   /* done */
#define     HI_SERVER_INVALID_STATCODE              2
#define     HI_SERVER_NO_CONTLEN                    3
#define     HI_SERVER_UTF_NORM_FAIL                 4
#define     HI_SERVER_UTF7                          5
#define     HI_SERVER_DECOMPR_FAILED                6
#define     HI_SERVER_CONSECUTIVE_SMALL_CHUNK_SIZES 7 
#define     HI_CLISRV_MSG_SIZE_EXCEPTION            8 
#define     HI_SERVER_JS_OBFUSCATION_EXCD           9 
#define     HI_SERVER_JS_EXCESS_WS                  10 
#define     HI_SERVER_MIXED_ENCODINGS               11
#define     HI_SERVER_SWF_ZLIB_FAILURE              12
#define     HI_SERVER_SWF_LZMA_FAILURE              13
#define     HI_SERVER_PDF_DEFLATE_FAILURE           14
#define     HI_SERVER_PDF_UNSUP_COMP_TYPE           15
#define     HI_SERVER_PDF_CASC_COMP                 16
#define     HI_SERVER_PDF_PARSE_FAILURE             17

#define     HI_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT 30
#define     HI_SERVER_RANGE_FIELD_ERROR             31
#define     HI_SERVER_NON_RANGE_GET_PARTIAL_METHOD  32


#define GENERATOR_PSNG                             122
#define     PSNG_TCP_PORTSCAN                      1
#define     PSNG_TCP_DECOY_PORTSCAN                2
#define     PSNG_TCP_PORTSWEEP                     3
#define     PSNG_TCP_DISTRIBUTED_PORTSCAN          4
#define     PSNG_TCP_FILTERED_PORTSCAN             5
#define     PSNG_TCP_FILTERED_DECOY_PORTSCAN       6
#define     PSNG_TCP_PORTSWEEP_FILTERED            7
#define     PSNG_TCP_FILTERED_DISTRIBUTED_PORTSCAN 8

#define     PSNG_IP_PORTSCAN                       9
#define     PSNG_IP_DECOY_PORTSCAN                 10
#define     PSNG_IP_PORTSWEEP                      11
#define     PSNG_IP_DISTRIBUTED_PORTSCAN           12
#define     PSNG_IP_FILTERED_PORTSCAN              13
#define     PSNG_IP_FILTERED_DECOY_PORTSCAN        14
#define     PSNG_IP_PORTSWEEP_FILTERED             15
#define     PSNG_IP_FILTERED_DISTRIBUTED_PORTSCAN  16

#define     PSNG_UDP_PORTSCAN                      17
#define     PSNG_UDP_DECOY_PORTSCAN                18
#define     PSNG_UDP_PORTSWEEP                     19
#define     PSNG_UDP_DISTRIBUTED_PORTSCAN          20
#define     PSNG_UDP_FILTERED_PORTSCAN             21
#define     PSNG_UDP_FILTERED_DECOY_PORTSCAN       22
#define     PSNG_UDP_PORTSWEEP_FILTERED            23
#define     PSNG_UDP_FILTERED_DISTRIBUTED_PORTSCAN 24

#define     PSNG_ICMP_PORTSWEEP                    25
#define     PSNG_ICMP_PORTSWEEP_FILTERED           26

#define     PSNG_OPEN_PORT                         27

#define GENERATOR_SPP_FRAG3                       123
#define     FRAG3_IPOPTIONS                         1
#define     FRAG3_TEARDROP                          2
#define     FRAG3_SHORT_FRAG                        3
#define     FRAG3_ANOMALY_OVERSIZE                  4
#define     FRAG3_ANOMALY_ZERO                      5
#define     FRAG3_ANOMALY_BADSIZE_SM                6
#define     FRAG3_ANOMALY_BADSIZE_LG                7
#define     FRAG3_ANOMALY_OVLP                      8
/* 123:9, 123:10 are OBE w/ addition of 116:458
 * (aka DECODE_IPV6_BAD_FRAG_PKT).
 * Leave these here so they are not reused.
 * ------
#define     FRAG3_IPV6_BSD_ICMP_FRAG                9
#define     FRAG3_IPV6_BAD_FRAG_PKT                10
 * ------
*/
#define     FRAG3_MIN_TTL_EVASION                  11
#define     FRAG3_EXCESSIVE_OVERLAP                12
#define     FRAG3_TINY_FRAGMENT                    13

#define GENERATOR_SMTP                             124
#define     SMTP_COMMAND_OVERFLOW                  1
#define     SMTP_DATA_HDR_OVERFLOW                 2
#define     SMTP_RESPONSE_OVERFLOW                 3
#define     SMTP_SPECIFIC_CMD_OVERFLOW             4
#define     SMTP_UNKNOWN_CMD                       5
#define     SMTP_ILLEGAL_CMD                       6
#define     SMTP_HEADER_NAME_OVERFLOW              7
#define     SMTP_XLINK2STATE_OVERFLOW              8
/* This alert is obsolete. *
* #define     SMTP_DECODE_MEMCAP_EXCEEDED            9*/
#define     SMTP_B64_DECODING_FAILED               10 
#define     SMTP_QP_DECODING_FAILED                11
/* Do not delete or reuse this SID. Commenting this SID as this alert is no longer valid.*
* #define     SMTP_BITENC_DECODING_FAILED            12
*/
#define     SMTP_UU_DECODING_FAILED                13
    
/*
**  FTPTelnet Generator IDs
**
**  IMPORTANT::
**    Whenever events are added to the internal FTP or Telnet
**    event queues, you must also add the event here.  The
**    trick is that whatever the number is in FTPTelnet,
**    it must be +1 when you define it here.
*/
#define GENERATOR_SPP_FTPP_FTP                     125
#define FTPP_FTP_TELNET_CMD                   1
#define FTPP_FTP_INVALID_CMD                  2
#define FTPP_FTP_PARAMETER_LENGTH_OVERFLOW    3
#define FTPP_FTP_MALFORMED_PARAMETER          4
#define FTPP_FTP_PARAMETER_STR_FORMAT         5
#define FTPP_FTP_RESPONSE_LENGTH_OVERFLOW     6
#define FTPP_FTP_ENCRYPTED                    7
#define FTPP_FTP_BOUNCE                       8
#define GENERATOR_SPP_FTPP_TELNET                  126
#define FTPP_TELNET_AYT_OVERFLOW              1
#define FTPP_TELNET_ENCRYPTED                 2
#define FTPP_TELNET_SUBNEG_BEGIN_NO_END       3

#define GENERATOR_SPP_ISAKMP                 127

#define GENERATOR_SPP_SSH                    128
#define     SSH_EVENT_RESPOVERFLOW             1
#define     SSH_EVENT_CRC32                    2
#define     SSH_EVENT_SECURECRT                3
#define     SSH_EVENT_PROTOMISMATCH            4
#define     SSH_EVENT_WRONGDIR                 5
#define     SSH_EVENT_PAYLOAD_SIZE             6
#define     SSH_EVENT_VERSION                  7

#define GENERATOR_SPP_STREAM                     129
#define     STREAM_SYN_ON_EST                      1
#define     STREAM_DATA_ON_SYN                     2
#define     STREAM_DATA_ON_CLOSED                  3
#define     STREAM_BAD_TIMESTAMP                   4
#define     STREAM_BAD_SEGMENT                     5
#define     STREAM_WINDOW_TOO_LARGE                6
#define     STREAM_EXCESSIVE_TCP_OVERLAPS          7
#define     STREAM_DATA_AFTER_RESET                8
#define     STREAM_SESSION_HIJACKED_CLIENT         9
#define     STREAM_SESSION_HIJACKED_SERVER        10
#define     STREAM_DATA_WITHOUT_FLAGS             11
#define     STREAM_SMALL_SEGMENT                  12
#define     STREAM_4WAY_HANDSHAKE                 13
#define     STREAM_NO_TIMESTAMP                   14
#define     STREAM_BAD_RST                        15
#define     STREAM_BAD_FIN                        16
#define     STREAM_BAD_ACK                        17
#define     STREAM_DATA_AFTER_RST_RCVD            18
#define     STREAM_WINDOW_SLAM                    19
#define     STREAM_NO_3WHS                        20
#define     STREAM_WIN_SZ_0_TCP_FIN_WAIT_1        21

#define GENERATOR_DNS                             131
#define     DNS_EVENT_OBSOLETE_TYPES                1
#define     DNS_EVENT_EXPERIMENTAL_TYPES            2
#define     DNS_EVENT_RDATA_OVERFLOW                3

#define GENERATOR_SKYPE                           132

#define GENERATOR_DCE2                              133
#define     DCE2_EVENT__MEMCAP                        1
#define     DCE2_EVENT__SMB_BAD_NBSS_TYPE             2
#define     DCE2_EVENT__SMB_BAD_TYPE                  3
#define     DCE2_EVENT__SMB_BAD_ID                    4
#define     DCE2_EVENT__SMB_BAD_WCT                   5
#define     DCE2_EVENT__SMB_BAD_BCC                   6
#define     DCE2_EVENT__SMB_BAD_FORMAT                7
#define     DCE2_EVENT__SMB_BAD_OFF                   8
#define     DCE2_EVENT__SMB_TDCNT_ZERO                9
#define     DCE2_EVENT__SMB_NB_LT_SMBHDR             10
#define     DCE2_EVENT__SMB_NB_LT_COM                11
#define     DCE2_EVENT__SMB_NB_LT_BCC                12
#define     DCE2_EVENT__SMB_NB_LT_DSIZE              13
#define     DCE2_EVENT__SMB_TDCNT_LT_DSIZE           14
#define     DCE2_EVENT__SMB_DSENT_GT_TDCNT           15
#define     DCE2_EVENT__SMB_BCC_LT_DSIZE             16
#define     DCE2_EVENT__SMB_INVALID_DSIZE            17
#define     DCE2_EVENT__SMB_EXCESSIVE_TREE_CONNECTS  18
#define     DCE2_EVENT__SMB_EXCESSIVE_READS          19
#define     DCE2_EVENT__SMB_EXCESSIVE_CHAINING       20
#define     DCE2_EVENT__SMB_MULT_CHAIN_SS            21
#define     DCE2_EVENT__SMB_MULT_CHAIN_TC            22
#define     DCE2_EVENT__SMB_CHAIN_SS_LOGOFF          23
#define     DCE2_EVENT__SMB_CHAIN_TC_TDIS            24
#define     DCE2_EVENT__SMB_CHAIN_OPEN_CLOSE         25
#define     DCE2_EVENT__SMB_INVALID_SHARE            26
#define     DCE2_EVENT__CO_BAD_MAJ_VERSION           27
#define     DCE2_EVENT__CO_BAD_MIN_VERSION           28
#define     DCE2_EVENT__CO_BAD_PDU_TYPE              29
#define     DCE2_EVENT__CO_FLEN_LT_HDR               30
#define     DCE2_EVENT__CO_FLEN_LT_SIZE              31
#define     DCE2_EVENT__CO_ZERO_CTX_ITEMS            32
#define     DCE2_EVENT__CO_ZERO_TSYNS                33
#define     DCE2_EVENT__CO_FRAG_LT_MAX_XMIT_FRAG     34
#define     DCE2_EVENT__CO_FRAG_GT_MAX_XMIT_FRAG     35
#define     DCE2_EVENT__CO_ALTER_CHANGE_BYTE_ORDER   36
#define     DCE2_EVENT__CO_FRAG_DIFF_CALL_ID         37
#define     DCE2_EVENT__CO_FRAG_DIFF_OPNUM           38
#define     DCE2_EVENT__CO_FRAG_DIFF_CTX_ID          39
#define     DCE2_EVENT__CL_BAD_MAJ_VERSION           40
#define     DCE2_EVENT__CL_BAD_PDU_TYPE              41
#define     DCE2_EVENT__CL_DATA_LT_HDR               42
#define     DCE2_EVENT__CL_BAD_SEQ_NUM               43
#define     DCE2_EVENT__SMB_V1                       44
#define     DCE2_EVENT__SMB_V2                       45
#define     DCE2_EVENT__SMB_INVALID_BINDING          46
#define     DCE2_EVENT__SMB2_EXCESSIVE_COMPOUNDING   47
#define     DCE2_EVENT__SMB_DCNT_ZERO                48
#define     DCE2_EVENT__SMB_DCNT_MISMATCH            49
#define     DCE2_EVENT__SMB_MAX_REQS_EXCEEDED        50
#define     DCE2_EVENT__SMB_REQS_SAME_MID            51
#define     DCE2_EVENT__SMB_DEPR_DIALECT_NEGOTIATED  52
#define     DCE2_EVENT__SMB_DEPR_COMMAND_USED        53
#define     DCE2_EVENT__SMB_UNUSUAL_COMMAND_USED     54
#define     DCE2_EVENT__SMB_INVALID_SETUP_COUNT      55
#define     DCE2_EVENT__SMB_MULTIPLE_NEGOTIATIONS    56
#define     DCE2_EVENT__SMB_EVASIVE_FILE_ATTRS       57
#define     DCE2_EVENT__SMB_INVALID_FILE_OFFSET      58
#define     DCE2_EVENT__SMB_BAD_NEXT_COMMAND_OFFSET  59

#define GENERATOR_PPM                               134
#define     PPM_EVENT_RULE_TREE_DISABLED              1
#define     PPM_EVENT_RULE_TREE_ENABLED               2
#define     PPM_EVENT_PACKET_ABORTED                  3

#define GENERATOR_INTERNAL                          135
#define     INTERNAL_EVENT_SYN_RECEIVED               1
#define     INTERNAL_EVENT_SESSION_ADD                2
#define     INTERNAL_EVENT_SESSION_DEL                3

#define GENERATOR_SPP_REPUTATION	                136

#define GENERATOR_SPP_SSLPP                         137

#define GENERATOR_SPP_SDF_RULES                     138
#define GENERATOR_SPP_SDF_PREPROC                   139
// #define GENERATOR_SPP_SIP	                    140 // Defined in spp_sip.h file, not here.
// #define GENERATOR_SPP_IMAP                       141 // Defined in imap_log.h file
// #define GENERATOR_SPP_POP                        142 // Defined in pop_log.h file.
#define     SDF_COMBO_ALERT                           1


#define GENERATOR_SPP_GTP                           143

#define GENERATOR_SPP_MODBUS                        144

#define GENERATOR_SPP_DNP3                          145

// #define GENERATOR_FILE_TYPE                      146 //Defined in file_service.h
// #define GENERATOR_FILE_SIGNATURE                 147 //Defined in file_service.h

#define GENERATOR_SPP_CIP                           148
#define GENERATOR_SPP_S7COMMPLUS                    149


/*  This is where all the alert messages will be archived for each
    internal alerts */

#define ARPSPOOF_UNICAST_ARP_REQUEST_STR "(spp_arpspoof) Unicast ARP request"
#define ARPSPOOF_ETHERFRAME_ARP_MISMATCH_SRC_STR \
"(spp_arpspoof) Ethernet/ARP Mismatch request for Source"
#define ARPSPOOF_ETHERFRAME_ARP_MISMATCH_DST_STR \
"(spp_arpspoof) Ethernet/ARP Mismatch request for Destination"
#define ARPSPOOF_ARP_CACHE_OVERWRITE_ATTACK_STR \
"(spp_arpspoof) Attempted ARP cache overwrite attack"

#define BO_TRAFFIC_DETECT_STR "(spo_bo) Back Orifice Traffic detected"
#define BO_CLIENT_TRAFFIC_DETECT_STR "(spo_bo) Back Orifice Client Traffic detected"
#define BO_SERVER_TRAFFIC_DETECT_STR "(spo_bo) Back Orifice Server Traffic detected"
#define BO_SNORT_BUFFER_ATTACK_STR "(spo_bo) Back Orifice Snort buffer attack"

/*   FRAG3 strings */
#define FRAG3_IPOPTIONS_STR "(spp_frag3) Inconsistent IP Options on Fragmented Packets"
#define FRAG3_TEARDROP_STR "(spp_frag3) Teardrop attack"
#define FRAG3_SHORT_FRAG_STR "(spp_frag3) Short fragment, possible DoS attempt"
#define FRAG3_ANOM_OVERSIZE_STR "(spp_frag3) Fragment packet ends after defragmented packet"
#define FRAG3_ANOM_ZERO_STR "(spp_frag3) Zero-byte fragment packet"
#define FRAG3_ANOM_BADSIZE_SM_STR "(spp_frag3) Bad fragment size, packet size is negative"
#define FRAG3_ANOM_BADSIZE_LG_STR "(spp_frag3) Bad fragment size, packet size is greater than 65536"
#define FRAG3_ANOM_OVLP_STR "(spp_frag3) Fragmentation overlap"
/* 123:9, 123:10 are OBE w/ addition of 116:458
 * (aka DECODE_IPV6_BAD_FRAG_PKT).
 * Leave these here so they are not reused.
 * ------
#define FRAG3_IPV6_BSD_ICMP_FRAG_STR "(spp_frag3) IPv6 BSD mbufs remote kernel buffer overflow"
#define FRAG3_IPV6_BAD_FRAG_PKT_STR "(spp_frag3) Bogus fragmentation packet. Possible BSD attack"
 * ------
 */
#define FRAG3_MIN_TTL_EVASION_STR "(spp_frag3) TTL value less than configured minimum, not using for reassembly"
#define FRAG3_EXCESSIVE_OVERLAP_STR "(spp_frag3) Excessive fragment overlap"
#define FRAG3_TINY_FRAGMENT_STR "(spp_frag3) Tiny fragment"

/*   Stream strings */
#define STREAM_SYN_ON_EST_STR "Syn on established session"
#define STREAM_DATA_ON_SYN_STR "Data on SYN packet"
#define STREAM_DATA_ON_CLOSED_STR "Data sent on stream not accepting data"
#define STREAM_BAD_TIMESTAMP_STR "TCP Timestamp is outside of PAWS window"
#define STREAM_BAD_SEGMENT_STR "Bad segment, adjusted size <= 0"
#define STREAM_WINDOW_TOO_LARGE_STR "Window size (after scaling) larger than policy allows"
#define STREAM_EXCESSIVE_TCP_OVERLAPS_STR "Limit on number of overlapping TCP packets reached"
#define STREAM_DATA_AFTER_RESET_STR "Data sent on stream after TCP Reset sent"
#define STREAM_SESSION_HIJACKED_CLIENT_STR "TCP Client possibly hijacked, different Ethernet Address"
#define STREAM_SESSION_HIJACKED_SERVER_STR "TCP Server possibly hijacked, different Ethernet Address"
#define STREAM_DATA_WITHOUT_FLAGS_STR "TCP Data with no TCP Flags set"
#define STREAM_SMALL_SEGMENT_STR "Consecutive TCP small segments exceeding threshold"
#define STREAM_4WAY_HANDSHAKE_STR "4-way handshake detected"
#define STREAM_NO_TIMESTAMP_STR "TCP Timestamp is missing"
#define STREAM_BAD_RST_STR "Reset outside window"
#define STREAM_BAD_FIN_STR "FIN number is greater than prior FIN"
#define STREAM_BAD_ACK_STR "ACK number is greater than prior FIN"
#define STREAM_DATA_AFTER_RST_RCVD_STR "Data sent on stream after TCP Reset received"
#define STREAM_WINDOW_SLAM_STR "TCP window closed before receiving data"
#define STREAM_NO_3WHS_STR "TCP session without 3-way handshake"
#define STREAM_WIN_SZ_0_TCP_FIN_WAIT_1_STR "TCP window size is zero for ACK packet received after FIN,ACK"

#define STREAM_INTERNAL_EVENT_STR ""

/* PPM strings */
#define PPM_EVENT_RULE_TREE_DISABLED_STR "PPM Rule Options Disabled by Rule Latency"
#define PPM_EVENT_RULE_TREE_ENABLED_STR  "PPM Rule Options Re-enabled by Rule Latency"
#define PPM_EVENT_PACKET_ABORTED_STR     "PPM Packet Aborted due to Latency"

/*   Snort decoder strings */
#define DECODE_NOT_IPV4_DGRAM_STR "(snort_decoder) WARNING: Not IPv4 datagram"
#define DECODE_IPV4_INVALID_HEADER_LEN_STR "(snort_decoder) WARNING: hlen < IP_HEADER_LEN"
#define DECODE_IPV4_DGRAM_LT_IPHDR_STR "(snort_decoder) WARNING: IP dgm len < IP Hdr len"
#define DECODE_IPV4OPT_BADLEN_STR      "(snort_decoder) WARNING: Ipv4 Options found with bad lengths"
#define DECODE_IPV4OPT_TRUNCATED_STR   "(snort_decoder) WARNING: Truncated Ipv4 Options"
#define DECODE_IPV4_DGRAM_GT_CAPLEN_STR "(snort_decoder) WARNING: IP dgm len > captured len"
#define DECODE_NOT_IPV6_DGRAM_STR      "(snort_decoder) WARNING: Not an IPv6 datagram"

#define DECODE_TCP_DGRAM_LT_TCPHDR_STR "(snort_decoder) WARNING: TCP packet len is smaller than 20 bytes"
#define DECODE_TCP_INVALID_OFFSET_STR "(snort_decoder) WARNING: TCP Data Offset is less than 5"
#define DECODE_TCP_LARGE_OFFSET_STR "(snort_decoder) WARNING: TCP Header length exceeds packet length"

#define DECODE_TCPOPT_BADLEN_STR      "(snort_decoder) WARNING: Tcp Options found with bad lengths"
#define DECODE_TCPOPT_TRUNCATED_STR   "(snort_decoder) WARNING: Truncated Tcp Options"
#define DECODE_TCPOPT_TTCP_STR        "(snort_decoder) WARNING: T/TCP Detected"
#define DECODE_TCPOPT_OBSOLETE_STR    "(snort_decoder) WARNING: Obsolete TCP Options found"
#define DECODE_TCPOPT_EXPERIMENT_STR  "(snort_decoder) WARNING: Experimental Tcp Options found"
#define DECODE_TCPOPT_WSCALE_INVALID_STR "(snort_decoder) WARNING: Tcp Window Scale Option found with length > 14"

#define DECODE_UDP_DGRAM_LT_UDPHDR_STR "(snort_decoder) WARNING: Truncated UDP Header"
#define DECODE_UDP_DGRAM_INVALID_LENGTH_STR "(snort_decoder) WARNING: Invalid UDP header, length field < 8"
#define DECODE_UDP_DGRAM_SHORT_PACKET_STR "(snort_decoder) WARNING: Short UDP packet, length field > payload length"
#define DECODE_UDP_DGRAM_LONG_PACKET_STR "(snort_decoder) WARNING: Long UDP packet, length field < payload length"

#define DECODE_ICMP_DGRAM_LT_ICMPHDR_STR "(snort_decoder) WARNING: ICMP Header Truncated"
#define DECODE_ICMP_DGRAM_LT_TIMESTAMPHDR_STR "(snort_decoder) WARNING: ICMP Timestamp Header Truncated"
#define DECODE_ICMP_DGRAM_LT_ADDRHDR_STR "(snort_decoder) WARNING: ICMP Address Header Truncated"
#define DECODE_IPV4_DGRAM_UNKNOWN_STR "(snort_decoder) WARNING: Unknown Datagram decoding problem"
#define DECODE_ARP_TRUNCATED_STR "(snort_decoder) WARNING: Truncated ARP"
#define DECODE_EAPOL_TRUNCATED_STR "(snort_decoder) WARNING: Truncated EAP Header"
#define DECODE_EAPKEY_TRUNCATED_STR "(snort_decoder) WARNING: EAP Key Truncated"
#define DECODE_EAP_TRUNCATED_STR "(snort_decoder) WARNING: EAP Header Truncated"
#define DECODE_BAD_PPPOE_STR "(snort_decoder) WARNING: Bad PPPOE frame detected"
#define DECODE_BAD_VLAN_STR "(snort_decoder) WARNING: Bad VLAN Frame"
#define DECODE_BAD_VLAN_ETHLLC_STR "(snort_decoder) WARNING: Bad LLC header"
#define DECODE_BAD_VLAN_OTHER_STR "(snort_decoder) WARNING: Bad Extra LLC Info"
#define DECODE_BAD_80211_ETHLLC_STR "(snort_decoder) WARNING: Bad 802.11 LLC header"
#define DECODE_BAD_80211_OTHER_STR "(snort_decoder) WARNING: Bad 802.11 Extra LLC Info"

#define DECODE_BAD_TRH_STR "(snort_decoder) WARNING: Bad Token Ring Header"
#define DECODE_BAD_TR_ETHLLC_STR "(snort_decoder) WARNING: Bad Token Ring ETHLLC Header"
#define DECODE_BAD_TR_MR_LEN_STR "(snort_decoder) WARNING: Bad Token Ring MRLENHeader"
#define DECODE_BAD_TRHMR_STR "(snort_decoder) WARNING: Bad Token Ring MR Header"

#define     DECODE_BAD_TRAFFIC_LOOPBACK_STR     "(snort decoder) WARNING: Bad Traffic Loopback IP"
#define     DECODE_BAD_TRAFFIC_SAME_SRCDST_STR  "(snort decoder) WARNING: Bad Traffic Same Src/Dst IP"

#ifdef GRE
#define DECODE_GRE_DGRAM_LT_GREHDR_STR "(snort decoder) WARNING: GRE header length > payload length"
#define DECODE_GRE_MULTIPLE_ENCAPSULATION_STR "(snort decoder) WARNING: Multiple encapsulations in packet"
#define DECODE_GRE_INVALID_VERSION_STR "(snort decoder) WARNING: Invalid GRE version"
#define DECODE_GRE_INVALID_HEADER_STR "(snort decoder) WARNING: Invalid GRE header"
#define DECODE_GRE_V1_INVALID_HEADER_STR "(snort decoder) WARNING: Invalid GRE v.1 PPTP header"
#define DECODE_GRE_TRANS_DGRAM_LT_TRANSHDR_STR "(snort decoder) WARNING: GRE Trans header length > payload length"
#endif  /* GRE */

#define DECODE_ICMP_ORIG_IP_TRUNCATED_STR "(snort_decoder) WARNING: ICMP Original IP Header Truncated"
#define DECODE_ICMP_ORIG_IP_VER_MISMATCH_STR "(snort_decoder) WARNING: ICMP version and Original IP Header versions differ"
#define DECODE_ICMP_ORIG_DGRAM_LT_ORIG_IP_STR "(snort_decoder) WARNING: ICMP Original Datagram Length < Original IP Header Length"
#define DECODE_ICMP_ORIG_PAYLOAD_LT_64_STR "(snort_decoder) WARNING: ICMP Original IP Payload < 64 bits"
#define DECODE_ICMP_ORIG_PAYLOAD_GT_576_STR "(snort_decoder) WARNING: ICMP Origianl IP Payload > 576 bytes"
#define DECODE_ICMP_ORIG_IP_WITH_FRAGOFFSET_STR "(snort_decoder) WARNING: ICMP Original IP Fragmented and Offset Not 0"

#define DECODE_IPV6_MIN_TTL_STR "(snort decoder) WARNING: IPv6 packet below TTL limit"
#define DECODE_IPV6_IS_NOT_STR "(snort decoder) WARNING: IPv6 header claims to not be IPv6"
#define DECODE_IPV6_TRUNCATED_EXT_STR "(snort decoder) WARNING: IPV6 truncated extension header"
#define DECODE_IPV6_TRUNCATED_STR "(snort decoder) WARNING: IPV6 truncated header"
#define DECODE_IPV6_DGRAM_LT_IPHDR_STR "(snort_decoder) WARNING: IP dgm len < IP Hdr len"
#define DECODE_IPV6_DGRAM_GT_CAPLEN_STR "(snort_decoder) WARNING: IP dgm len > captured len"

#define DECODE_IPV6_DST_ZERO_STR "(snort_decoder) WARNING: IPv6 packet with destination address ::0"
#define DECODE_IPV6_SRC_MULTICAST_STR "(snort_decoder) WARNING: IPv6 packet with multicast source address"
#define DECODE_IPV6_DST_RESERVED_MULTICAST_STR "(snort_decoder) WARNING: IPv6 packet with reserved multicast destination address"
#define DECODE_IPV6_BAD_OPT_TYPE_STR "(snort_decoder) WARNING: IPv6 header includes an undefined option type"
#define DECODE_IPV6_BAD_MULTICAST_SCOPE_STR "(snort_decoder) WARNING: IPv6 address includes an unassigned multicast scope value"
#define DECODE_IPV6_BAD_NEXT_HEADER_STR "(snort_decoder) WARNING: IPv6 header includes an invalid value for the \"next header\" field"
#define DECODE_IPV6_ROUTE_AND_HOPBYHOP_STR "(snort_decoder) WARNING: IPv6 header includes a routing extension header followed by a hop-by-hop header"
#define DECODE_IPV6_TWO_ROUTE_HEADERS_STR "(snort_decoder) WARNING: IPv6 header includes two routing extension headers"
#define DECODE_IPV6_DSTOPTS_WITH_ROUTING_STR "(snort_decoder) WARNING: IPv6 header has destination options followed by a routing header"
#define DECODE_ICMPV6_TOO_BIG_BAD_MTU_STR "(snort_decoder) WARNING: ICMPv6 packet of type 2 (message too big) with MTU field < 1280"
#define DECODE_ICMPV6_UNREACHABLE_NON_RFC_2463_CODE_STR "(snort_decoder) WARNING: ICMPv6 packet of type 1 (destination unreachable) with non-RFC 2463 code"
#define DECODE_ICMPV6_SOLICITATION_BAD_CODE_STR "(snort_decoder) WARNING: ICMPv6 router solicitation packet with a code not equal to 0"
#define DECODE_ICMPV6_ADVERT_BAD_CODE_STR "(snort_decoder) WARNING: ICMPv6 router advertisement packet with a code not equal to 0"
#define DECODE_ICMPV6_SOLICITATION_BAD_RESERVED_STR "(snort_decoder) WARNING: ICMPv6 router solicitation packet with the reserved field not equal to 0"
#define DECODE_ICMPV6_ADVERT_BAD_REACHABLE_STR "(snort_decoder) WARNING: ICMPv6 router advertisement packet with the reachable time field set > 1 hour"

#define DECODE_IPV6_TUNNELED_IPV4_TRUNCATED_STR "(snort_decoder) WARNING: IPV6 tunneled over IPv4, IPv6 header truncated, possible Linux Kernel attack"

#define DECODE_IP_MULTIPLE_ENCAPSULATION_STR "(snort_decoder) WARNING: Two or more IP (v4 and/or v6) encapsulation layers present"

#define DECODE_ESP_HEADER_TRUNC_STR "(snort_decoder) WARNING: truncated Encapsulated Security Payload (ESP) header"

#define DECODE_IPV6_BAD_OPT_LEN_STR "(snort_decoder) WARNING: IPv6 header includes an option which is too big for the containing header"

#define DECODE_IPV6_UNORDERED_EXTENSIONS_STR "(snort_decoder) WARNING: IPv6 packet includes out-of-order extension headers"
#define DECODE_GTP_MULTIPLE_ENCAPSULATION_STR "(snort_decoder) WARNING: Two or more GTP encapsulation layers present"
#define DECODE_GTP_BAD_LEN_STR "(snort_decoder) WARNING: GTP header length is invalid"
#define DECODE_TCP_XMAS_STR "(snort_decoder) WARNING: XMAS Attack Detected"
#define DECODE_TCP_NMAP_XMAS_STR "(snort_decoder) WARNING: Nmap XMAS Attack Detected"

#define DECODE_DOS_NAPTHA_STR "(snort_decoder) WARNING: DOS NAPTHA Vulnerability Detected"
#define DECODE_SYN_TO_MULTICAST_STR "(snort_decoder) WARNING: Bad Traffic SYN to multicast address"
#define DECODE_ZERO_TTL_STR "(snort_decoder) WARNING: IPV4 packet with zero TTL"
#define DECODE_BAD_FRAGBITS_STR "(snort_decoder) WARNING: IPV4 packet with bad frag bits (Both MF and DF set)"
#define DECODE_UDP_IPV6_ZERO_CHECKSUM_STR "(snort_decoder) WARNING: Invalid IPv6 UDP packet, checksum zero"
#define DECODE_IP4_LEN_OFFSET_STR "(snort_decoder) WARNING: IPV4 packet frag offset + length exceed maximum"
#define DECODE_IP4_SRC_THIS_NET_STR "(snort_decoder) WARNING: IPV4 packet from 'current net' source address"
#define DECODE_IP4_DST_THIS_NET_STR "(snort_decoder) WARNING: IPV4 packet to 'current net' dest address"
#define DECODE_IP4_SRC_MULTICAST_STR "(snort_decoder) WARNING: IPV4 packet from multicast source address"
#define DECODE_IP4_SRC_RESERVED_STR "(snort_decoder) WARNING: IPV4 packet from reserved source address"
#define DECODE_IP4_DST_RESERVED_STR "(snort_decoder) WARNING: IPV4 packet to reserved dest address"
#define DECODE_IP4_SRC_BROADCAST_STR "(snort_decoder) WARNING: IPV4 packet from broadcast source address"
#define DECODE_IP4_DST_BROADCAST_STR "(snort_decoder) WARNING: IPV4 packet to broadcast dest address"
#define DECODE_ICMP4_DST_MULTICAST_STR "(snort_decoder) WARNING: ICMP4 packet to multicast dest address"
#define DECODE_ICMP4_DST_BROADCAST_STR "(snort_decoder) WARNING: ICMP4 packet to broadcast dest address"
#define DECODE_ICMP4_TYPE_OTHER_STR "(snort_decoder) WARNING: ICMP4 type other"
#define DECODE_TCP_BAD_URP_STR "(snort_decoder) WARNING: TCP urgent pointer exceeds payload length or no payload"
#define DECODE_TCP_SYN_FIN_STR "(snort_decoder) WARNING: TCP SYN with FIN"
#define DECODE_TCP_SYN_RST_STR "(snort_decoder) WARNING: TCP SYN with RST"
#define DECODE_TCP_MUST_ACK_STR "(snort_decoder) WARNING: TCP PDU missing ack for established session"
#define DECODE_TCP_NO_SYN_ACK_RST_STR "(snort_decoder) WARNING: TCP has no SYN, ACK, or RST"
#define DECODE_ETH_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated eth header"
#define DECODE_IP4_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated IP4 header"
#define DECODE_ICMP4_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated ICMP4 header"
#define DECODE_ICMP6_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated ICMP6 header"
#define DECODE_IP4_MIN_TTL_STR "(snort decoder) WARNING: IPV4 packet below TTL limit"
#define DECODE_IP6_ZERO_HOP_LIMIT_STR "(snort decoder) WARNING: IPV6 packet has zero hop limit"
#define DECODE_IP4_DF_OFFSET_STR "(snort_decoder) WARNING: IPV4 packet both DF and offset set"
#define DECODE_ICMP6_TYPE_OTHER_STR "(snort_decoder) WARNING: ICMP6 type not decoded"
#define DECODE_ICMP6_DST_MULTICAST_STR "(snort_decoder) WARNING: ICMP6 packet to multicast address"
#define DECODE_TCP_SHAFT_SYNFLOOD_STR "(snort_decoder) WARNING: DDOS shaft synflood"
#define DECODE_ICMP_PING_NMAP_STR "(snort_decoder) WARNING: ICMP PING NMAP"
#define DECODE_ICMP_ICMPENUM_STR "(snort_decoder) WARNING: ICMP icmpenum v1.1.1"
#define DECODE_ICMP_REDIRECT_HOST_STR "(snort_decoder) WARNING: ICMP redirect host"
#define DECODE_ICMP_REDIRECT_NET_STR "(snort_decoder) WARNING: ICMP redirect net"
#define DECODE_ICMP_TRACEROUTE_IPOPTS_STR "(snort_decoder) WARNING: ICMP traceroute ipopts"
#define DECODE_ICMP_SOURCE_QUENCH_STR "(snort_decoder) WARNING: ICMP Source Quench"
#define DECODE_ICMP_BROADSCAN_SMURF_SCANNER_STR "(snort_decoder) WARNING: Broadscan Smurf Scanner"
#define DECODE_ICMP_DST_UNREACH_ADMIN_PROHIBITED_STR "(snort_decoder) WARNING: ICMP Destination Unreachable Communication Administratively Prohibited"
#define DECODE_ICMP_DST_UNREACH_DST_HOST_PROHIBITED_STR "(snort_decoder) WARNING: ICMP Destination Unreachable Communication with Destination Host is Administratively Prohibited"
#define DECODE_ICMP_DST_UNREACH_DST_NET_PROHIBITED_STR "(snort_decoder) WARNING: ICMP Destination Unreachable Communication with Destination Network is Administratively Prohibited"
#define DECODE_IP_OPTION_SET_STR "(snort_decoder) WARNING: MISC IP option set"
#define DECODE_UDP_LARGE_PACKET_STR "(snort_decoder) WARNING: MISC Large UDP Packet"
#define DECODE_TCP_PORT_ZERO_STR "(snort_decoder) WARNING: BAD-TRAFFIC TCP port 0 traffic"
#define DECODE_UDP_PORT_ZERO_STR "(snort_decoder) WARNING: BAD-TRAFFIC UDP port 0 traffic"
#define DECODE_IP_RESERVED_FRAG_BIT_STR "(snort_decoder) WARNING: BAD-TRAFFIC IP reserved bit set"
#define DECODE_IP_UNASSIGNED_PROTO_STR "(snort_decoder) WARNING: BAD-TRAFFIC Unassigned/Reserved IP protocol"
#define DECODE_IP_BAD_PROTO_STR "(snort_decoder) WARNING: BAD-TRAFFIC Bad IP protocol"
#define DECODE_ICMP_PATH_MTU_DOS_STR "(snort_decoder) WARNING: ICMP PATH MTU denial of service attempt"
#define DECODE_ICMP_DOS_ATTEMPT_STR "(snort_decoder) WARNING: BAD-TRAFFIC linux ICMP header dos attempt"
#define DECODE_IPV6_ISATAP_SPOOF_STR "(snort_decoder) WARNING: BAD-TRAFFIC ISATAP-addressed IPv6 traffic spoofing attempt"
#define DECODE_PGM_NAK_OVERFLOW_STR "(snort_decoder) WARNING: BAD-TRAFFIC PGM nak list overflow attempt"
#define DECODE_IGMP_OPTIONS_DOS_STR "(snort_decoder) WARNING: DOS IGMP IP Options validation attempt"
#define DECODE_IP6_EXCESS_EXT_HDR_STR "(snort_decoder) WARNING: too many IP6 extension headers"
#define DECODE_ICMPV6_UNREACHABLE_NON_RFC_4443_CODE_STR "(snort_decoder) WARNING: ICMPv6 packet of type 1 (destination unreachable) with non-RFC 4443 code"
#define DECODE_IPV6_BAD_FRAG_PKT_STR "(snort_decoder) WARNING: bogus fragmentation packet. Possible BSD attack"
#define DECODE_ZERO_LENGTH_FRAG_STR "(snort_decoder) WARNING: fragment with zero length"
#define DECODE_ICMPV6_NODE_INFO_BAD_CODE_STR "(snort_decoder) WARNING: ICMPv6 node info query/response packet with a code greater than 2"
#define DECODE_IPV6_ROUTE_ZERO_STR "(snort decoder) WARNING: IPV6 routing type 0 extension header"
#define DECODE_ERSPAN_HDR_VERSION_MISMATCH_STR "(snort_decoder) WARNING: ERSpan Header version mismatch"
#define DECODE_ERSPAN2_DGRAM_LT_HDR_STR "(snort_decoder) WARNING: captured < ERSpan Type2 Header Length"
#define DECODE_ERSPAN3_DGRAM_LT_HDR_STR "(snort_decoder) WARNING: captured < ERSpan Type3 Header Length"
#define DECODE_AUTH_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated authentication header"
#define DECODE_AUTH_HDR_BAD_LEN_STR "(snort_decoder) WARNING: authentication header bad length"
#define DECODE_FPATH_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated FabricPath header"
#define DECODE_CISCO_META_HDR_TRUNC_STR "(snort_decoder) WARNING: truncated Cisco Metadata header"
#define DECODE_CISCO_META_HDR_OPT_LEN_STR "(snort_decoder) WARNING: Invalid Cisco Metadata option length"
#define DECODE_CISCO_META_HDR_OPT_TYPE_STR "(snort_decoder) WARNING: Invalid Cisco Metadata option type"
#define DECODE_CISCO_META_HDR_SGT_STR "(snort_decoder) WARNING: Invalid Cisco Metadata SGT"

/*  RPC decode preprocessor strings */
#define RPC_FRAG_TRAFFIC_STR "(spp_rpc_decode) Fragmented RPC Records"
#define RPC_MULTIPLE_RECORD_STR "(spp_rpc_decode) Multiple RPC Records"
#define RPC_LARGE_FRAGSIZE_STR  "(spp_rpc_decode) Large RPC Record Fragment"
#define RPC_INCOMPLETE_SEGMENT_STR "(spp_rpc_decode) Incomplete RPC segment"
#define RPC_ZERO_LENGTH_FRAGMENT_STR "(spp_rpc_decode) Zero-length RPC Fragment"

#define PSNG_TCP_PORTSCAN_STR "(portscan) TCP Portscan"
#define PSNG_TCP_DECOY_PORTSCAN_STR "(portscan) TCP Decoy Portscan"
#define PSNG_TCP_PORTSWEEP_STR "(portscan) TCP Portsweep"
#define PSNG_TCP_DISTRIBUTED_PORTSCAN_STR "(portscan) TCP Distributed Portscan"
#define PSNG_TCP_FILTERED_PORTSCAN_STR "(portscan) TCP Filtered Portscan"
#define PSNG_TCP_FILTERED_DECOY_PORTSCAN_STR "(portscan) TCP Filtered Decoy Portscan"
#define PSNG_TCP_FILTERED_DISTRIBUTED_PORTSCAN_STR "(portscan) TCP Filtered Distributed Portscan"
#define PSNG_TCP_PORTSWEEP_FILTERED_STR "(portscan) TCP Filtered Portsweep"

#define PSNG_IP_PORTSCAN_STR "(portscan) IP Protocol Scan"
#define PSNG_IP_DECOY_PORTSCAN_STR "(portscan) IP Decoy Protocol Scan"
#define PSNG_IP_PORTSWEEP_STR "(portscan) IP Protocol Sweep"
#define PSNG_IP_DISTRIBUTED_PORTSCAN_STR "(portscan) IP Distributed Protocol Scan"
#define PSNG_IP_FILTERED_PORTSCAN_STR "(portscan) IP Filtered Protocol Scan"
#define PSNG_IP_FILTERED_DECOY_PORTSCAN_STR "(portscan) IP Filtered Decoy Protocol Scan"
#define PSNG_IP_FILTERED_DISTRIBUTED_PORTSCAN_STR "(portscan) IP Filtered Distributed Protocol Scan"
#define PSNG_IP_PORTSWEEP_FILTERED_STR "(portscan) IP Filtered Protocol Sweep"

#define PSNG_UDP_PORTSCAN_STR "(portscan) UDP Portscan"
#define PSNG_UDP_DECOY_PORTSCAN_STR "(portscan) UDP Decoy Portscan"
#define PSNG_UDP_PORTSWEEP_STR "(portscan) UDP Portsweep"
#define PSNG_UDP_DISTRIBUTED_PORTSCAN_STR "(portscan) UDP Distributed Portscan"
#define PSNG_UDP_FILTERED_PORTSCAN_STR "(portscan) UDP Filtered Portscan"
#define PSNG_UDP_FILTERED_DECOY_PORTSCAN_STR "(portscan) UDP Filtered Decoy Portscan"
#define PSNG_UDP_FILTERED_DISTRIBUTED_PORTSCAN_STR "(portscan) UDP Filtered Distributed Portscan"
#define PSNG_UDP_PORTSWEEP_FILTERED_STR "(portscan) UDP Filtered Portsweep"

#define PSNG_ICMP_PORTSWEEP_STR "(portscan) ICMP Sweep"
#define PSNG_ICMP_PORTSWEEP_FILTERED_STR "(portscan) ICMP Filtered Sweep"

#define PSNG_OPEN_PORT_STR "(portscan) Open Port"

#define DECODE_BAD_MPLS_STR "(snort_decoder) WARNING: Bad MPLS Frame"
#define DECODE_BAD_MPLS_LABEL0_STR "(snort_decoder) WARNING: MPLS Label 0 Appears in Nonbottom Header"
#define DECODE_BAD_MPLS_LABEL1_STR "(snort_decoder) WARNING: MPLS Label 1 Appears in Bottom Header"
#define DECODE_BAD_MPLS_LABEL2_STR "(snort_decoder) WARNING: MPLS Label 2 Appears in Nonbottom Header"
#define DECODE_BAD_MPLS_LABEL3_STR "(snort_decoder) WARNING: MPLS Label 3 Appears in Header"
#define DECODE_MPLS_RESERVEDLABEL_STR "(snort_decoder) WARNING: MPLS Label 4, 5,.. or 15 Appears in Header"
#define DECODE_MPLS_LABEL_STACK_STR "(snort_decoder) WARNING: Too Many MPLS headers"
#define DECODE_MULTICAST_MPLS_STR "(snort_decoder) WARNING: Multicast MPLS traffic detected"

#define DECODE_DECODING_DEPTH_EXCEEDED_STR "(snort_decoder) WARNING: Too many levels for decoding"

#endif /* __GENERATORS_H__ */
