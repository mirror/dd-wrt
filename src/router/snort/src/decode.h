/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id$ */


#ifndef __DECODE_H__
#define __DECODE_H__


/*  I N C L U D E S  **********************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#else /* !WIN32 */
#include <netinet/in_systm.h>
#ifndef IFNAMSIZ
#define IFNAMESIZ MAX_ADAPTER_NAME
#endif /* !IFNAMSIZ */
#endif /* !WIN32 */

#include <daq.h>
#include <sfbpf_dlt.h>

#include "bitop.h"
#include "ipv6_port.h"
#include "sf_ip.h"
#include "protocols.h"
#include "sfdaq.h"
#include "util.h"

/*  D E F I N E S  ************************************************************/

#define ETHERNET_MTU                  1500
#define ETHERNET_TYPE_IP              0x0800
#define ETHERNET_TYPE_ARP             0x0806
#define ETHERNET_TYPE_REVARP          0x8035
#define ETHERNET_TYPE_EAPOL           0x888e
#define ETHERNET_TYPE_IPV6            0x86dd
#define ETHERNET_TYPE_IPX             0x8137
#define ETHERNET_TYPE_PPPoE_DISC      0x8863 /* discovery stage */
#define ETHERNET_TYPE_PPPoE_SESS      0x8864 /* session stage */
#define ETHERNET_TYPE_8021Q           0x8100
#define ETHERNET_TYPE_LOOP            0x9000
#define ETHERNET_TYPE_MPLS_UNICAST    0x8847
#define ETHERNET_TYPE_MPLS_MULTICAST  0x8848

#define ETH_DSAP_SNA                  0x08    /* SNA */
#define ETH_SSAP_SNA                  0x00    /* SNA */
#define ETH_DSAP_STP                  0x42    /* Spanning Tree Protocol */
#define ETH_SSAP_STP                  0x42    /* Spanning Tree Protocol */
#define ETH_DSAP_IP                   0xaa    /* IP */
#define ETH_SSAP_IP                   0xaa    /* IP */

#define ETH_ORG_CODE_ETHR              0x000000    /* Encapsulated Ethernet */
#define ETH_ORG_CODE_CDP               0x00000c    /* Cisco Discovery Proto */

#define ETHERNET_HEADER_LEN             14
#define ETHERNET_MAX_LEN_ENCAP          1518    /* 802.3 (+LLC) or ether II ? */
#define PPPOE_HEADER_LEN                20    /* ETHERNET_HEADER_LEN + 6 */

#define VLAN_HEADER_LEN                  4
#ifndef NO_NON_ETHER_DECODER
#define MINIMAL_TOKENRING_HEADER_LEN    22
#define MINIMAL_IEEE80211_HEADER_LEN    10    /* Ack frames and others */
#define IEEE802_11_DATA_HDR_LEN         24    /* Header for data packets */
#define TR_HLEN                         MINIMAL_TOKENRING_HEADER_LEN
#define TOKENRING_LLC_LEN                8
#define SLIP_HEADER_LEN                 16

/* Frame type/subype combinations with version = 0 */
        /*** FRAME TYPE *****  HEX ****  SUBTYPE TYPE  DESCRIPT ********/
#define WLAN_TYPE_MGMT_ASREQ   0x0      /* 0000    00  Association Req */
#define WLAN_TYPE_MGMT_ASRES   0x10     /* 0001    00  Assocaition Res */
#define WLAN_TYPE_MGMT_REREQ   0x20     /* 0010    00  Reassoc. Req.   */
#define WLAN_TYPE_MGMT_RERES   0x30     /* 0011    00  Reassoc. Resp.  */
#define WLAN_TYPE_MGMT_PRREQ   0x40     /* 0100    00  Probe Request   */
#define WLAN_TYPE_MGMT_PRRES   0x50     /* 0101    00  Probe Response  */ 
#define WLAN_TYPE_MGMT_BEACON  0x80     /* 1000    00  Beacon          */
#define WLAN_TYPE_MGMT_ATIM    0x90     /* 1001    00  ATIM message    */
#define WLAN_TYPE_MGMT_DIS     0xa0     /* 1010    00  Disassociation  */
#define WLAN_TYPE_MGMT_AUTH    0xb0     /* 1011    00  Authentication  */
#define WLAN_TYPE_MGMT_DEAUTH  0xc0     /* 1100    00  Deauthentication*/

#define WLAN_TYPE_CONT_PS      0xa4     /* 1010    01  Power Save      */
#define WLAN_TYPE_CONT_RTS     0xb4     /* 1011    01  Request to send */
#define WLAN_TYPE_CONT_CTS     0xc4     /* 1100    01  Clear to sene   */
#define WLAN_TYPE_CONT_ACK     0xd4     /* 1101    01  Acknowledgement */
#define WLAN_TYPE_CONT_CFE     0xe4     /* 1110    01  Cont. Free end  */
#define WLAN_TYPE_CONT_CFACK   0xf4     /* 1111    01  CF-End + CF-Ack */

#define WLAN_TYPE_DATA_DATA    0x08     /* 0000    10  Data            */
#define WLAN_TYPE_DATA_DTCFACK 0x18     /* 0001    10  Data + CF-Ack   */
#define WLAN_TYPE_DATA_DTCFPL  0x28     /* 0010    10  Data + CF-Poll  */
#define WLAN_TYPE_DATA_DTACKPL 0x38     /* 0011    10  Data+CF-Ack+CF-Pl */
#define WLAN_TYPE_DATA_NULL    0x48     /* 0100    10  Null (no data)  */
#define WLAN_TYPE_DATA_CFACK   0x58     /* 0101    10  CF-Ack (no data)*/
#define WLAN_TYPE_DATA_CFPL    0x68     /* 0110    10  CF-Poll (no data)*/
#define WLAN_TYPE_DATA_ACKPL   0x78     /* 0111    10  CF-Ack+CF-Poll  */

/*** Flags for IEEE 802.11 Frame Control ***/
/* The following are designed to be bitwise-AND-d in an 8-bit u_char */
#define WLAN_FLAG_TODS      0x0100    /* To DS Flag   10000000 */
#define WLAN_FLAG_FROMDS    0x0200    /* From DS Flag 01000000 */
#define WLAN_FLAG_FRAG      0x0400    /* More Frag    00100000 */
#define WLAN_FLAG_RETRY     0x0800    /* Retry Flag   00010000 */
#define WLAN_FLAG_PWRMGMT   0x1000    /* Power Mgmt.  00001000 */
#define WLAN_FLAG_MOREDAT   0x2000    /* More Data    00000100 */
#define WLAN_FLAG_WEP       0x4000    /* Wep Enabled  00000010 */
#define WLAN_FLAG_ORDER     0x8000    /* Strict Order 00000001 */

/* IEEE 802.1x eapol types */
#define EAPOL_TYPE_EAP      0x00      /* EAP packet */
#define EAPOL_TYPE_START    0x01      /* EAPOL start */
#define EAPOL_TYPE_LOGOFF   0x02      /* EAPOL Logoff */
#define EAPOL_TYPE_KEY      0x03      /* EAPOL Key */
#define EAPOL_TYPE_ASF      0x04      /* EAPOL Encapsulated ASF-Alert */

/* Extensible Authentication Protocol Codes RFC 2284*/
#define EAP_CODE_REQUEST    0x01   
#define EAP_CODE_RESPONSE   0x02
#define EAP_CODE_SUCCESS    0x03
#define EAP_CODE_FAILURE    0x04
/* EAP Types */
#define EAP_TYPE_IDENTITY   0x01
#define EAP_TYPE_NOTIFY     0x02
#define EAP_TYPE_NAK        0x03
#define EAP_TYPE_MD5        0x04
#define EAP_TYPE_OTP        0x05
#define EAP_TYPE_GTC        0x06
#define EAP_TYPE_TLS        0x0d
#endif  // NO_NON_ETHER_DECODER

/* Cisco HDLC header values */
#define CHDLC_HEADER_LEN        4
#define CHDLC_ADDR_UNICAST      0x0f
#define CHDLC_ADDR_MULTICAST    0x8f
#define CHDLC_ADDR_BROADCAST    0xff
#define CHDLC_CTRL_UNNUMBERED   0x03

/* Teredo values */
#define TEREDO_PORT 3544
#define TEREDO_INDICATOR_ORIGIN 0x00
#define TEREDO_INDICATOR_ORIGIN_LEN 8
#define TEREDO_INDICATOR_AUTH 0x01
#define TEREDO_INDICATOR_AUTH_MIN_LEN 13
#define TEREDO_MIN_LEN 2

/* ESP constants */
#define ESP_HEADER_LEN 8
#define ESP_AUTH_DATA_LEN 12
#define ESP_TRAILER_LEN 2

#define MAX_PORTS 65536

/* ppp header structure
 *
 * Actually, this is the header for RFC1332 Section 3
 * IPCP Configuration Options for sending IP datagrams over a PPP link
 *
 */
struct ppp_header {
    unsigned char  address;
    unsigned char  control;
    unsigned short protocol;
};

#ifndef PPP_HDRLEN
    #define PPP_HDRLEN          sizeof(struct ppp_header)
#endif

#define PPP_IP         0x0021        /* Internet Protocol */
#define PPP_VJ_COMP    0x002d        /* VJ compressed TCP/IP */
#define PPP_VJ_UCOMP   0x002f        /* VJ uncompressed TCP/IP */
#define PPP_IPX        0x002b        /* Novell IPX Protocol */

/* otherwise defined in /usr/include/ppp_defs.h */
#ifndef PPP_MTU
    #define PPP_MTU                 1500
#endif

/* NULL aka LoopBack interfaces */
#define NULL_HDRLEN             4

/* enc interface */
struct enc_header {
    uint32_t af;
    uint32_t spi;
    uint32_t flags;
};
#define ENC_HEADER_LEN          12

/* otherwise defined in /usr/include/ppp_defs.h */
#define IP_HEADER_LEN           20
#define TCP_HEADER_LEN          20
#define UDP_HEADER_LEN          8
#define ICMP_HEADER_LEN         4
#define ICMP_NORMAL_LEN         8

#define IP_OPTMAX               40
#define IP6_EXTMAX              40
#define TCP_OPTLENMAX           40 /* (((2^4) - 1) * 4  - TCP_HEADER_LEN) */

#ifndef IP_MAXPACKET
#define IP_MAXPACKET    65535        /* maximum packet size */
#endif /* IP_MAXPACKET */


/* http://www.iana.org/assignments/ipv6-parameters
 *
 * IPv6 Options (not Extension Headers)
 */
#define IP6_OPT_TUNNEL_ENCAP    0x04
#define IP6_OPT_QUICK_START     0x06
#define IP6_OPT_CALIPSO         0x07
#define IP6_OPT_HOME_ADDRESS    0xC9
#define IP6_OPT_ENDPOINT_IDENT  0x8A

// these are bits in th_flags:
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_RES2 TH_ECE  // TBD TH_RES* should be deleted (see log.c)
#define TH_RES1 TH_CWR
#define TH_NORESERVED (TH_FIN|TH_SYN|TH_RST|TH_PUSH|TH_ACK|TH_URG)

// these are bits in th_offx2:
#define TH_RSV  0x0E  // reserved bits
#define TH_NS   0x01  // ECN nonce bit

/* http://www.iana.org/assignments/tcp-parameters
 *
 * tcp options stuff. used to be in <netinet/tcp.h> but it breaks
 * things on AIX
 */
#define TCPOPT_EOL              0   /* End of Option List [RFC793] */
#define TCPOLEN_EOL             1   /* Always one byte */

#define TCPOPT_NOP              1   /* No-Option [RFC793] */
#define TCPOLEN_NOP             1   /* Always one byte */

#define TCPOPT_MAXSEG           2   /* Maximum Segment Size [RFC793] */
#define TCPOLEN_MAXSEG          4   /* Always 4 bytes */

#define TCPOPT_WSCALE           3   /* Window scaling option [RFC1323] */
#define TCPOLEN_WSCALE          3   /* 1 byte with logarithmic values */

#define TCPOPT_SACKOK           4    /* Experimental [RFC2018]*/
#define TCPOLEN_SACKOK          2

#define TCPOPT_SACK             5    /* Experimental [RFC2018] variable length */

#define TCPOPT_ECHO             6    /* Echo (obsoleted by option 8)      [RFC1072] */
#define TCPOLEN_ECHO            6    /* 6 bytes  */

#define TCPOPT_ECHOREPLY        7    /* Echo Reply (obsoleted by option 8)[RFC1072] */
#define TCPOLEN_ECHOREPLY       6    /* 6 bytes  */

#define TCPOPT_TIMESTAMP        8   /* Timestamp [RFC1323], 10 bytes */
#define TCPOLEN_TIMESTAMP       10

#define TCPOPT_PARTIAL_PERM     9   /* Partial Order Permitted/ Experimental [RFC1693] */
#define TCPOLEN_PARTIAL_PERM    2   /* Partial Order Permitted/ Experimental [RFC1693] */

#define TCPOPT_PARTIAL_SVC      10  /*  Partial Order Profile [RFC1693] */
#define TCPOLEN_PARTIAL_SVC     3   /*  3 bytes long -- Experimental */

/* atleast decode T/TCP options... */
#define TCPOPT_CC               11  /*  T/TCP Connection count  [RFC1644] */
#define TCPOPT_CC_NEW           12  /*  CC.NEW [RFC1644] */
#define TCPOPT_CC_ECHO          13  /*  CC.ECHO [RFC1644] */
#define TCPOLEN_CC             6  /* page 17 of rfc1644 */
#define TCPOLEN_CC_NEW         6  /* page 17 of rfc1644 */
#define TCPOLEN_CC_ECHO        6  /* page 17 of rfc1644 */

#define TCPOPT_ALTCSUM          15  /* TCP Alternate Checksum Data [RFC1146], variable length */
#define TCPOPT_SKEETER          16  /* Skeeter [Knowles] */
#define TCPOPT_BUBBA            17  /* Bubba   [Knowles] */

#define TCPOPT_TRAILER_CSUM     18  /* Trailer Checksum Option [Subbu & Monroe] */
#define TCPOLEN_TRAILER_CSUM  3  

#define TCPOPT_MD5SIG           19  /* MD5 Signature Option [RFC2385] */
#define TCPOLEN_MD5SIG        18

/* Space Communications Protocol Standardization */
#define TCPOPT_SCPS             20  /* Capabilities [Scott] */
#define TCPOPT_SELNEGACK        21  /* Selective Negative Acknowledgements [Scott] */
#define TCPOPT_RECORDBOUND         22  /* Record Boundaries [Scott] */
#define TCPOPT_CORRUPTION          23  /* Corruption experienced [Scott] */

#define TCPOPT_SNAP                24  /* SNAP [Sukonnik] -- anyone have info?*/
#define TCPOPT_UNASSIGNED          25  /* Unassigned (released 12/18/00) */
#define TCPOPT_COMPRESSION         26  /* TCP Compression Filter [Bellovin] */
/* http://www.research.att.com/~smb/papers/draft-bellovin-tcpcomp-00.txt*/

#define TCP_OPT_TRUNC -1
#define TCP_OPT_BADLEN -2

/* Why are these lil buggers here? Never Used. -- cmg */
#define TCPOLEN_TSTAMP_APPA     (TCPOLEN_TIMESTAMP+2)    /* appendix A / rfc 1323 */
#define TCPOPT_TSTAMP_HDR    \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

/*
 * Default maximum segment size for TCP.
 * With an IP MSS of 576, this is 536,
 * but 512 is probably more convenient.
 * This should be defined as MIN(512, IP_MSS - sizeof (struct tcpiphdr)).
 */

#ifndef TCP_MSS
    #define    TCP_MSS      512
#endif

#ifndef TCP_MAXWIN
    #define    TCP_MAXWIN   65535    /* largest value for (unscaled) window */
#endif

#ifndef TCP_MAX_WINSHIFT 
    #define TCP_MAX_WINSHIFT    14    /* maximum window shift */
#endif

/*
 * User-settable options (used with setsockopt).
 */
#ifndef TCP_NODELAY
    #define    TCP_NODELAY   0x01    /* don't delay send to coalesce packets */
#endif

#ifndef TCP_MAXSEG
    #define    TCP_MAXSEG    0x02    /* set maximum segment size */
#endif

#define SOL_TCP        6    /* TCP level */



#define L2TP_PORT           1701
#define DHCP_CLIENT_PORT    68
#define DHCP_SERVER_PORT    67

#ifndef NO_NON_ETHER_DECODER
/* Start Token Ring */
#define TR_ALEN             6        /* octets in an Ethernet header */
#define IPARP_SAP           0xaa

#define AC                  0x10
#define LLC_FRAME           0x40

#define TRMTU                      2000    /* 2000 bytes            */
#define TR_RII                     0x80
#define TR_RCF_DIR_BIT             0x80
#define TR_RCF_LEN_MASK            0x1f00
#define TR_RCF_BROADCAST           0x8000    /* all-routes broadcast   */
#define TR_RCF_LIMITED_BROADCAST   0xC000    /* single-route broadcast */
#define TR_RCF_FRAME2K             0x20
#define TR_RCF_BROADCAST_MASK      0xC000
/* End Token Ring */

/* Start FDDI */
#define FDDI_ALLC_LEN                   13
#define FDDI_ALEN                       6
#define FDDI_MIN_HLEN                   (FDDI_ALLC_LEN + 3)

#define FDDI_DSAP_SNA                   0x08    /* SNA */
#define FDDI_SSAP_SNA                   0x00    /* SNA */
#define FDDI_DSAP_STP                   0x42    /* Spanning Tree Protocol */
#define FDDI_SSAP_STP                   0x42    /* Spanning Tree Protocol */
#define FDDI_DSAP_IP                    0xaa    /* IP */
#define FDDI_SSAP_IP                    0xaa    /* IP */

#define FDDI_ORG_CODE_ETHR              0x000000    /* Encapsulated Ethernet */
#define FDDI_ORG_CODE_CDP               0x00000c    /* Cisco Discovery
                             * Proto(?) */

#define ETHERNET_TYPE_CDP               0x2000    /* Cisco Discovery Protocol */
/* End FDDI */
#endif  // NO_NON_ETHER_DECODER

#define ARPOP_REQUEST   1    /* ARP request                  */
#define ARPOP_REPLY     2    /* ARP reply                    */
#define ARPOP_RREQUEST  3    /* RARP request                 */
#define ARPOP_RREPLY    4    /* RARP reply                   */

/* PPPoE types */
#define PPPoE_CODE_SESS 0x00 /* PPPoE session */
#define PPPoE_CODE_PADI 0x09 /* PPPoE Active Discovery Initiation */
#define PPPoE_CODE_PADO 0x07 /* PPPoE Active Discovery Offer */
#define PPPoE_CODE_PADR 0x19 /* PPPoE Active Discovery Request */
#define PPPoE_CODE_PADS 0x65 /* PPPoE Active Discovery Session-confirmation */
#define PPPoE_CODE_PADT 0xa7 /* PPPoE Active Discovery Terminate */

/* PPPoE tag types */
#define PPPoE_TAG_END_OF_LIST        0x0000
#define PPPoE_TAG_SERVICE_NAME       0x0101
#define PPPoE_TAG_AC_NAME            0x0102
#define PPPoE_TAG_HOST_UNIQ          0x0103
#define PPPoE_TAG_AC_COOKIE          0x0104
#define PPPoE_TAG_VENDOR_SPECIFIC    0x0105
#define PPPoE_TAG_RELAY_SESSION_ID   0x0110
#define PPPoE_TAG_SERVICE_NAME_ERROR 0x0201
#define PPPoE_TAG_AC_SYSTEM_ERROR    0x0202
#define PPPoE_TAG_GENERIC_ERROR      0x0203


#define ICMP_ECHOREPLY          0    /* Echo Reply                   */
#define ICMP_DEST_UNREACH       3    /* Destination Unreachable      */
#define ICMP_SOURCE_QUENCH      4    /* Source Quench                */
#define ICMP_REDIRECT           5    /* Redirect (change route)      */
#define ICMP_ECHO               8    /* Echo Request                 */
#define ICMP_ROUTER_ADVERTISE   9    /* Router Advertisement         */
#define ICMP_ROUTER_SOLICIT     10    /* Router Solicitation          */
#define ICMP_TIME_EXCEEDED      11    /* Time Exceeded                */
#define ICMP_PARAMETERPROB      12    /* Parameter Problem            */
#define ICMP_TIMESTAMP          13    /* Timestamp Request            */
#define ICMP_TIMESTAMPREPLY     14    /* Timestamp Reply              */
#define ICMP_INFO_REQUEST       15    /* Information Request          */
#define ICMP_INFO_REPLY         16    /* Information Reply            */
#define ICMP_ADDRESS            17    /* Address Mask Request         */
#define ICMP_ADDRESSREPLY       18    /* Address Mask Reply           */
#define NR_ICMP_TYPES           18

/* Codes for ICMP UNREACHABLES */
#define ICMP_NET_UNREACH        0    /* Network Unreachable          */
#define ICMP_HOST_UNREACH       1    /* Host Unreachable             */
#define ICMP_PROT_UNREACH       2    /* Protocol Unreachable         */
#define ICMP_PORT_UNREACH       3    /* Port Unreachable             */
#define ICMP_FRAG_NEEDED        4    /* Fragmentation Needed/DF set  */
#define ICMP_SR_FAILED          5    /* Source Route failed          */
#define ICMP_NET_UNKNOWN        6
#define ICMP_HOST_UNKNOWN       7
#define ICMP_HOST_ISOLATED      8
#define ICMP_PKT_FILTERED_NET   9
#define ICMP_PKT_FILTERED_HOST  10
#define ICMP_NET_UNR_TOS        11
#define ICMP_HOST_UNR_TOS       12
#define ICMP_PKT_FILTERED       13    /* Packet filtered */
#define ICMP_PREC_VIOLATION     14    /* Precedence violation */
#define ICMP_PREC_CUTOFF        15    /* Precedence cut off */
#define NR_ICMP_UNREACH         15    /* instead of hardcoding immediate
                                       * value */

#define ICMP_REDIR_NET          0
#define ICMP_REDIR_HOST         1
#define ICMP_REDIR_TOS_NET      2
#define ICMP_REDIR_TOS_HOST     3

#define ICMP_TIMEOUT_TRANSIT    0
#define ICMP_TIMEOUT_REASSY     1

#define ICMP_PARAM_BADIPHDR     0
#define ICMP_PARAM_OPTMISSING   1
#define ICMP_PARAM_BAD_LENGTH   2

/* ip option type codes */
#ifndef IPOPT_EOL
    #define IPOPT_EOL            0x00
#endif

#ifndef IPOPT_NOP
    #define IPOPT_NOP            0x01
#endif

#ifndef IPOPT_RR
    #define IPOPT_RR             0x07
#endif

#ifndef IPOPT_RTRALT
    #define IPOPT_RTRALT         0x94
#endif

#ifndef IPOPT_TS
    #define IPOPT_TS             0x44
#endif

#ifndef IPOPT_SECURITY
    #define IPOPT_SECURITY       0x82
#endif

#ifndef IPOPT_LSRR
    #define IPOPT_LSRR           0x83
#endif

#ifndef IPOPT_LSRR_E
    #define IPOPT_LSRR_E         0x84
#endif

#ifndef IPOPT_ESEC
    #define IPOPT_ESEC           0x85
#endif

#ifndef IPOPT_SATID
    #define IPOPT_SATID          0x88
#endif

#ifndef IPOPT_SSRR
    #define IPOPT_SSRR           0x89
#endif


/* tcp option codes */
#define TOPT_EOL                0x00
#define TOPT_NOP                0x01
#define TOPT_MSS                0x02
#define TOPT_WS                 0x03
#define TOPT_TS                 0x08
#ifndef TCPOPT_WSCALE
    #define TCPOPT_WSCALE           3     /* window scale factor (rfc1072) */
#endif
#ifndef TCPOPT_SACKOK
    #define    TCPOPT_SACKOK        4     /* selective ack ok (rfc1072) */
#endif
#ifndef TCPOPT_SACK
    #define    TCPOPT_SACK          5     /* selective ack (rfc1072) */
#endif
#ifndef TCPOPT_ECHO
    #define TCPOPT_ECHO             6     /* echo (rfc1072) */
#endif
#ifndef TCPOPT_ECHOREPLY
    #define TCPOPT_ECHOREPLY        7     /* echo (rfc1072) */
#endif
#ifndef TCPOPT_TIMESTAMP
    #define TCPOPT_TIMESTAMP        8     /* timestamps (rfc1323) */
#endif
#ifndef TCPOPT_CC
    #define TCPOPT_CC               11    /* T/TCP CC options (rfc1644) */
#endif
#ifndef TCPOPT_CCNEW
    #define TCPOPT_CCNEW            12    /* T/TCP CC options (rfc1644) */
#endif
#ifndef TCPOPT_CCECHO
    #define TCPOPT_CCECHO           13    /* T/TCP CC options (rfc1644) */
#endif

#define EXTRACT_16BITS(p) ((u_short) ntohs (*(u_short *)(p)))

#ifdef WORDS_MUSTALIGN

#if defined(__GNUC__)
/* force word-aligned ntohl parameter */
    #define EXTRACT_32BITS(p)  ({ uint32_t __tmp; memmove(&__tmp, (p), sizeof(uint32_t)); (uint32_t) ntohl(__tmp);})
#endif /* __GNUC__ */

#else

/* allows unaligned ntohl parameter - dies w/SIGBUS on SPARCs */
    #define EXTRACT_32BITS(p) ((uint32_t) ntohl (*(uint32_t *)(p)))

#endif                /* WORDS_MUSTALIGN */

/* packet status flags */
#define PKT_REBUILT_FRAG     0x00000001  /* is a rebuilt fragment */
#define PKT_REBUILT_STREAM   0x00000002  /* is a rebuilt stream */
#define PKT_STREAM_UNEST_UNI 0x00000004  /* is from an unestablished stream and
                                          * we've only seen traffic in one direction */
#define PKT_PSEUDO           0x00000008  /* pseudo packet */

#define PKT_STREAM_EST       0x00000010  /* is from an established stream */
#define PKT_STATELESS        0x00000020  /* Packet has matched a stateless rule */
#define PKT_FROM_SERVER      0x00000040  /* this packet came from the server
                                            side of a connection (TCP) */
#define PKT_FROM_CLIENT      0x00000080  /* this packet came from the client
                                            side of a connection (TCP) */

#define PKT_HTTP_DECODE      0x00000100  /* this packet has normalized http */
/* Replaces PKT_OBFUSCATED which is no longer in use because we no longer
 * modify the packet when doing IP obfuscation */
#define PKT_PAYLOAD_OBFUSCATE  0x00000200
#define PKT_STREAM_INSERT    0x00000400  /* this packet has been inserted into stream4 */
#define PKT_ALT_DECODE       0x00000800  /* this packet has been normalized by telnet

                                    (only set when we must look at an alernative buffer) */

#define PKT_STREAM_TWH       0x00001000
#define PKT_IGNORE_PORT      0x00002000  /* this packet should be ignored, based on port */
#define PKT_PASS_RULE        0x00004000  /* this packet has matched a pass rule */
#define PKT_NO_DETECT        0x00008000  /* this packet should not be preprocessed */

#define PKT_PREPROC_RPKT     0x00010000  /* set in original packet to indicate a preprocessor
                                          * has a reassembled packet */
#define PKT_DCE_RPKT         0x00020000  /* this packet is a DCE/RPC reassembled one */
#define PKT_IP_RULE          0x00040000  /* this packet is being evaluated against an IP rule */
#define PKT_IP_RULE_2ND      0x00080000  /* this packet is being evaluated against an IP rule */

#define PKT_SMB_SEG          0x00100000  /* this is an SMB desegmented packet */
#define PKT_DCE_SEG          0x00200000  /* this is a DCE/RPC desegmented packet */
#define PKT_DCE_FRAG         0x00400000  /* this is a DCE/RPC defragmented packet */
#define PKT_SMB_TRANS        0x00800000  /* this is an SMB Transact reassembled packet */

#define PKT_DCE_PKT          0x01000000  /* a DCE packet processed by DCE/RPC pp */
#define PKT_RPC_PKT          0x02000000  /* an ONC RPC packet processed by rpc decode pp */

#define PKT_LOGGED           0x04000000  /* this packet has been logged */
#ifdef NORMALIZER
#define PKT_RESIZED          0x08000000  /* packet has new size; must set modified too */
#endif
#define PKT_MODIFIED         0x10000000  /* packet had normalizations, etc. */

#define PKT_HTTP_RESP_BODY   0x20000000  /* packet has non-zipped HTTP response Body */
#define PKT_ALLOW_MULTIPLE_DETECT 0x40000000  /* packet has either pipelined mime attachements or pipeline http requests */

#define PKT_UNSURE_ENCAP     0x80000000  /* packet has a potentially incorrect encapsulation layer. don't alert if "next layer" is invalid. */

/* error flags */
#define PKT_ERR_CKSUM_IP     0x01
#define PKT_ERR_CKSUM_TCP    0x02
#define PKT_ERR_CKSUM_UDP    0x04
#define PKT_ERR_CKSUM_ICMP   0x08
#define PKT_ERR_CKSUM_IGMP   0x10
#define PKT_ERR_CKSUM_ANY    0x1F
#define PKT_ERR_BAD_TTL      0x20

/* Only include application layer reassembled data
 * flags here - no PKT_REBUILT_FRAG */
#define REASSEMBLED_PACKET_FLAGS \
    (PKT_REBUILT_STREAM|PKT_SMB_SEG|PKT_DCE_SEG|PKT_DCE_FRAG|PKT_SMB_TRANS)

/*  D A T A  S T R U C T U R E S  *********************************************/

#ifndef NO_NON_ETHER_DECODER
/* Start Token Ring Data Structures */

#ifdef _MSC_VER
    /* Visual C++ pragma to disable warning messages about nonstandard bit field type */
    #pragma warning( disable : 4214 )  
#endif

/* LLC structure */
typedef struct _Trh_llc
{
    uint8_t dsap;
    uint8_t ssap;
    uint8_t protid[3];
    uint16_t ethertype;
}        Trh_llc;

/* RIF structure
 * Linux/tcpdump patch defines tokenring header in dump way, since not
 * every tokenring header with have RIF data... we define it separately, and
 * a bit more split up
 */

#ifdef _MSC_VER
  /* Visual C++ pragma to disable warning messages about nonstandard bit field type */
  #pragma warning( disable : 4214 )  
#endif


/* These are macros to use the bitlevel accesses in the Trh_Mr header

   they haven't been tested and they aren't used much so here is a
   listing of what used to be there

   #if defined(WORDS_BIGENDIAN)
      uint16_t bcast:3, len:5, dir:1, lf:3, res:4;
   #else
      uint16_t len:5,         length of RIF field, including RC itself
      bcast:3,       broadcast indicator 
      res:4,         reserved 
      lf:3,      largest frame size 
      dir:1;         direction
*/

#define TRH_MR_BCAST(trhmr)  ((ntohs((trhmr)->bcast_len_dir_lf_res) & 0xe000) >> 13)
#define TRH_MR_LEN(trhmr)    ((ntohs((trhmr)->bcast_len_dir_lf_res) & 0x1F00) >> 8)
#define TRH_MR_DIR(trhmr)    ((ntohs((trhmr)->bcast_len_dir_lf_res) & 0x0080) >> 8)
#define TRH_MR_LF(trhmr)     ((ntohs((trhmr)->bcast_len_dir_lf_res) & 0x0070) >> 7)
#define TRH_MR_RES(trhmr)     ((ntohs((trhmr)->bcast_len_dir_lf_res) & 0x000F))

typedef struct _Trh_mr
{
    uint16_t bcast_len_dir_lf_res; /* broadcast/res/framesize/direction */
    uint16_t rseg[8];
}       Trh_mr;
#ifdef _MSC_VER
  /* Visual C++ pragma to enable warning messages about nonstandard bit field type */
  #pragma warning( default : 4214 )
#endif


typedef struct _Trh_hdr
{
    uint8_t ac;        /* access control field */
    uint8_t fc;        /* frame control field */
    uint8_t daddr[TR_ALEN];    /* src address */
    uint8_t saddr[TR_ALEN];    /* dst address */
}        Trh_hdr;

#ifdef WIN32
    /* Visual C++ pragma to enable warning messages about nonstandard bit field type */
    #pragma warning( default : 4214 )
#endif
/* End Token Ring Data Structures */


/* Start FDDI Data Structures */

/* FDDI header is always this: -worm5er */
typedef struct _Fddi_hdr
{
    uint8_t fc;        /* frame control field */
    uint8_t daddr[FDDI_ALEN];  /* src address */
    uint8_t saddr[FDDI_ALEN];  /* dst address */
}         Fddi_hdr;

/* splitting the llc up because of variable lengths of the LLC -worm5er */
typedef struct _Fddi_llc_saps
{
    uint8_t dsap;
    uint8_t ssap;
}              Fddi_llc_saps;

/* I've found sna frames have two addition bytes after the llc saps -worm5er */
typedef struct _Fddi_llc_sna
{
    uint8_t ctrl_fld[2];
}             Fddi_llc_sna;

/* I've also found other frames that seem to have only one byte...  We're only
really intersted in the IP data so, until we want other, I'm going to say
the data is one byte beyond this frame...  -worm5er */
typedef struct _Fddi_llc_other
{
    uint8_t ctrl_fld[1];
}               Fddi_llc_other;

/* Just like TR the ip/arp data is setup as such: -worm5er */
typedef struct _Fddi_llc_iparp
{
    uint8_t ctrl_fld;
    uint8_t protid[3];
    uint16_t ethertype;
}               Fddi_llc_iparp;

/* End FDDI Data Structures */


/* 'Linux cooked captures' data
 * (taken from tcpdump source).
 */

#define SLL_HDR_LEN     16              /* total header length */
#define SLL_ADDRLEN     8               /* length of address field */
typedef struct _SLLHdr {
        uint16_t       sll_pkttype;    /* packet type */
        uint16_t       sll_hatype;     /* link-layer address type */
        uint16_t       sll_halen;      /* link-layer address length */
        uint8_t        sll_addr[SLL_ADDRLEN];  /* link-layer address */
        uint16_t       sll_protocol;   /* protocol */
} SLLHdr;


/*
 * Snort supports 3 versions of the OpenBSD pflog header:
 *
 * Pflog1_Hdr:  CVS = 1.3,  DLT_OLD_PFLOG = 17,  Length = 28
 * Pflog2_Hdr:  CVS = 1.8,  DLT_PFLOG     = 117, Length = 48
 * Pflog3_Hdr:  CVS = 1.12, DLT_PFLOG     = 117, Length = 64
 *
 * Since they have the same DLT, Pflog{2,3}Hdr are distinguished
 * by their actual length.  The minimum required length excludes
 * padding.
 */
/* Old OpenBSD pf firewall pflog0 header
 * (information from pf source in kernel)
 * the rule, reason, and action codes tell why the firewall dropped it -fleck
 */

typedef struct _Pflog1_hdr
{
    uint32_t af;
    char intf[IFNAMSIZ];
    int16_t rule;
    uint16_t reason;
    uint16_t action;
    uint16_t dir;
} Pflog1Hdr;

#define PFLOG1_HDRLEN (sizeof(struct _Pflog1_hdr))

/*
 * Note that on OpenBSD, af type is sa_family_t. On linux, that's an unsigned
 * short, but on OpenBSD, that's a uint8_t, so we should explicitly use uint8_t
 * here.  - ronaldo
 */

#define PFLOG_RULELEN 16
#define PFLOG_PADLEN  3

typedef struct _Pflog2_hdr
{
    int8_t   length;
    uint8_t  af;
    uint8_t  action;
    uint8_t  reason;
    char     ifname[IFNAMSIZ];
    char     ruleset[PFLOG_RULELEN];
    uint32_t rulenr;
    uint32_t subrulenr;
    uint8_t  dir;
    uint8_t  pad[PFLOG_PADLEN];
} Pflog2Hdr;

#define PFLOG2_HDRLEN (sizeof(struct _Pflog2_hdr))
#define PFLOG2_HDRMIN (PFLOG2_HDRLEN - PFLOG_PADLEN)

typedef struct _Pflog3_hdr
{
    int8_t   length;
    uint8_t  af;
    uint8_t  action;
    uint8_t  reason;
    char     ifname[IFNAMSIZ];
    char     ruleset[PFLOG_RULELEN];
    uint32_t rulenr;
    uint32_t subrulenr;
    uint32_t uid;
    uint32_t pid;
    uint32_t rule_uid;
    uint32_t rule_pid;
    uint8_t  dir;
    uint8_t  pad[PFLOG_PADLEN];
} Pflog3Hdr;

#define PFLOG3_HDRLEN (sizeof(struct _Pflog3_hdr))
#define PFLOG3_HDRMIN (PFLOG3_HDRLEN - PFLOG_PADLEN)

/*
 * ssl_pkttype values.
 */

#define LINUX_SLL_HOST          0
#define LINUX_SLL_BROADCAST     1
#define LINUX_SLL_MULTICAST     2
#define LINUX_SLL_OTHERHOST     3
#define LINUX_SLL_OUTGOING      4

/* ssl protocol values */

#define LINUX_SLL_P_802_3       0x0001  /* Novell 802.3 frames without 802.2 LLC header */
#define LINUX_SLL_P_802_2       0x0004  /* 802.2 frames (not D/I/X Ethernet) */
#endif  // NO_NON_ETHER_DECODER


#ifdef _MSC_VER
  /* Visual C++ pragma to disable warning messages 
   * about nonstandard bit field type 
   */
  #pragma warning( disable : 4214 )  
#endif

#define VTH_PRIORITY(vh)  ((ntohs((vh)->vth_pri_cfi_vlan) & 0xe000) >> 13)
#define VTH_CFI(vh)       ((ntohs((vh)->vth_pri_cfi_vlan) & 0x0100) >> 12)
#define VTH_VLAN(vh)      ((unsigned short)(ntohs((vh)->vth_pri_cfi_vlan) & 0x0FFF))

typedef struct _VlanTagHdr
{
    uint16_t vth_pri_cfi_vlan;
    uint16_t vth_proto;  /* protocol field... */
} VlanTagHdr;
#ifdef _MSC_VER
  /* Visual C++ pragma to enable warning messages about nonstandard bit field type */
  #pragma warning( default : 4214 )
#endif


typedef struct _EthLlc
{
    uint8_t dsap;
    uint8_t ssap;
} EthLlc;

typedef struct _EthLlcOther
{
    uint8_t ctrl;
    uint8_t org_code[3];
    uint16_t proto_id;
} EthLlcOther;

/* We must twiddle to align the offset the ethernet header and align
 * the IP header on solaris -- maybe this will work on HPUX too.
 */
#if defined (SOLARIS) || defined (SUNOS) || defined (__sparc__) || defined(__sparc64__) || defined (HPUX)
#define SPARC_TWIDDLE       2
#else
#define SPARC_TWIDDLE       0
#endif

/* 
 * Ethernet header
 */

typedef struct _EtherHdr
{
    uint8_t ether_dst[6];
    uint8_t ether_src[6];
    uint16_t ether_type;

} EtherHdr;


#ifndef NO_NON_ETHER_DECODER
/*
 *  Wireless Header (IEEE 802.11)
 */
typedef struct _WifiHdr
{
  uint16_t frame_control;
  uint16_t duration_id;
  uint8_t  addr1[6];
  uint8_t  addr2[6];
  uint8_t  addr3[6];
  uint16_t seq_control;
  uint8_t  addr4[6];
} WifiHdr;
#endif  // NO_NON_ETHER_DECODER


/* Can't add any fields not in the real header here 
   because of how the decoder uses structure overlaying */
#ifdef _MSC_VER
  /* Visual C++ pragma to disable warning messages 
   * about nonstandard bit field type 
   */
  #pragma warning( disable : 4214 )  
#endif

/* tcpdump shows us the way to cross platform compatibility */
#define IP_VER(iph)    (((iph)->ip_verhl & 0xf0) >> 4)
#define IP_HLEN(iph)   ((iph)->ip_verhl & 0x0f)

/* we need to change them as well as get them */
#define SET_IP_VER(iph, value)  ((iph)->ip_verhl = (unsigned char)(((iph)->ip_verhl & 0x0f) | (value << 4)))
#define SET_IP_HLEN(iph, value)  ((iph)->ip_verhl = (unsigned char)(((iph)->ip_verhl & 0xf0) | (value & 0x0f)))

#define NUM_IP_PROTOS 256

/* Last updated 6/2/2010.
   Source: http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xml */
#define MIN_UNASSIGNED_IP_PROTO 143

#ifndef IPPROTO_SWIPE
#define IPPROTO_SWIPE           53
#endif
#ifndef IPPROTO_IP_MOBILITY
#define IPPROTO_IP_MOBILITY     55
#endif
#ifndef IPPROTO_SUN_ND
#define IPPROTO_SUN_ND          77
#endif
#ifndef IPPROTO_PIM
#define IPPROTO_PIM             103
#endif

typedef struct _IPHdr
{
    uint8_t ip_verhl;      /* version & header length */
    uint8_t ip_tos;        /* type of service */
    uint16_t ip_len;       /* datagram length */
    uint16_t ip_id;        /* identification  */
    uint16_t ip_off;       /* fragment offset */
    uint8_t ip_ttl;        /* time to live field */
    uint8_t ip_proto;      /* datagram protocol */
    uint16_t ip_csum;      /* checksum */
    struct in_addr ip_src;  /* source IP */
    struct in_addr ip_dst;  /* dest IP */
} IPHdr;

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
    sfip_t ip_src;          /* source IP */
    sfip_t ip_dst;          /* dest IP */
} IP4Hdr;

typedef struct _IPv6Hdr
{ 
    uint32_t vcl;      /* version, class, and label */
    uint16_t len;      /* length of the payload */
    uint8_t  next;     /* next header
                         * Uses the same flags as
                         * the IPv4 protocol field */
    uint8_t  hop_lmt;  /* hop limit */ 
    sfip_t ip_src;
    sfip_t ip_dst;
} IP6Hdr; 

/* IPv6 address */
#ifndef s6_addr
struct in6_addr
{
    union
    {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    } in6_u;
#define s6_addr         in6_u.u6_addr8
#define s6_addr16       in6_u.u6_addr16
#define s6_addr32       in6_u.u6_addr32
};
#endif

typedef struct _IP6RawHdr
{
    union
    {
        struct _IP6HdrCtl
        {
            uint32_t ip6_un1_flow;   /* 4 bits version, 8 bits TC,
                                        20 bits flow-ID */
            uint16_t ip6_un1_plen;   /* payload length */
            uint8_t  ip6_un1_nxt;    /* next header */
            uint8_t  ip6_un1_hlim;   /* hop limit */
        } IP6HdrCtl;
        uint8_t ip6_un2_vfc;       /* 4 bits version, top 4 bits tclass */
    } IP6Ctl;

    struct in6_addr ip6_src;      /* source address */
    struct in6_addr ip6_dst;      /* destination address */
} IP6RawHdr;

#define ip6vfc   IP6Ctl.ip6_un2_vfc
#define ip6flow  IP6Ctl.IP6HdrCtl.ip6_un1_flow
#define ip6plen  IP6Ctl.IP6HdrCtl.ip6_un1_plen
#define ip6nxt   IP6Ctl.IP6HdrCtl.ip6_un1_nxt
#define ip6hlim  IP6Ctl.IP6HdrCtl.ip6_un1_hlim
#define ip6hops  IP6Ctl.IP6HdrCtl.ip6_un1_hlim

#define IP6_HDR_LEN 40

#ifndef IP_PROTO_HOPOPTS
# define IP_PROTO_HOPOPTS    0
#endif

#define IP_PROTO_NONE       59
#define IP_PROTO_ROUTING    43
#define IP_PROTO_FRAGMENT   44
#define IP_PROTO_AH         51
#define IP_PROTO_DSTOPTS    60
#define IP_PROTO_ICMPV6     58
#define IP_PROTO_IPV6       41
#define IP_PROTO_IPIP       4

#define IP6F_OFFSET_MASK    0xfff8  /* mask out offset from _offlg */
#define IP6F_MF_MASK        0x0001  /* more-fragments flag */

#define IP6F_OFFSET(fh) ((ntohs((fh)->ip6f_offlg) & IP6F_OFFSET_MASK) >> 3)
#define IP6F_RES(fh) (fh)->ip6f_reserved
#define IP6F_MF(fh) (ntohs((fh)->ip6f_offlg) & IP6F_MF_MASK )

/* to store references to IP6 Extension Headers */
typedef struct _IP6Option
{
    uint8_t type;
    const uint8_t *data;
} IP6Option;

/* Generic Extension Header */
typedef struct _IP6Extension
{
    uint8_t ip6e_nxt;
    uint8_t ip6e_len;
    /* options follow */
    uint8_t ip6e_pad[6];
} IP6Extension;

typedef struct _IP6HopByHop
{
    uint8_t ip6hbh_nxt;
    uint8_t ip6hbh_len;
    /* options follow */
    uint8_t ip6hbh_pad[6];
} IP6HopByHop;

typedef struct _IP6Dest
{
    uint8_t ip6dest_nxt;
    uint8_t ip6dest_len;
    /* options follow */
    uint8_t ip6dest_pad[6];
} IP6Dest;

typedef struct _IP6Route
{
    uint8_t ip6rte_nxt;
    uint8_t ip6rte_len;
    uint8_t ip6rte_type;
    uint8_t ip6rte_seg_left;
    /* type specific data follows */
} IP6Route;

typedef struct _IP6Route0
{
    uint8_t ip6rte0_nxt;
    uint8_t ip6rte0_len;
    uint8_t ip6rte0_type;
    uint8_t ip6rte0_seg_left;
    uint8_t ip6rte0_reserved;
    uint8_t ip6rte0_bitmap[3];
    struct in6_addr ip6rte0_addr[1];  /* Up to 23 IP6 addresses */
} IP6Route0;

/* Fragment header */
typedef struct _IP6Frag
{
    uint8_t   ip6f_nxt;     /* next header */
    uint8_t   ip6f_reserved;    /* reserved field */
    uint16_t  ip6f_offlg;   /* offset, reserved, and flag */
    uint32_t  ip6f_ident;   /* identification */
} IP6Frag;

typedef struct _ICMP6
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;

} ICMP6Hdr;

typedef struct _ICMP6TooBig
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint32_t mtu;
} ICMP6TooBig;

typedef struct _ICMP6RouterAdvertisement
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint8_t num_addrs;
    uint8_t addr_entry_size;
    uint16_t lifetime;
    uint32_t reachable_time;
    uint32_t retrans_time;
} ICMP6RouterAdvertisement;

typedef struct _ICMP6RouterSolicitation
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint32_t reserved;
} ICMP6RouterSolicitation;

#define ICMP6_UNREACH 1
#define ICMP6_BIG    2
#define ICMP6_TIME   3
#define ICMP6_PARAMS 4
#define ICMP6_ECHO   128
#define ICMP6_REPLY  129
#define ICMP6_SOLICITATION 133
#define ICMP6_ADVERTISEMENT 134

/* Minus 1 due to the 'body' field  */
#define ICMP6_MIN_HEADER_LEN (sizeof(ICMP6Hdr) )

struct _Packet;

/* IPHeader access calls */
sfip_t *    ip4_ret_src(struct _Packet *);
sfip_t *    ip4_ret_dst(struct _Packet *);
uint16_t   ip4_ret_tos(struct _Packet *);
uint8_t    ip4_ret_ttl(struct _Packet *);
uint16_t   ip4_ret_len(struct _Packet *);
uint32_t   ip4_ret_id(struct _Packet *);
uint8_t    ip4_ret_proto(struct _Packet *);
uint16_t   ip4_ret_off(struct _Packet *);
uint8_t    ip4_ret_ver(struct _Packet *);
uint8_t    ip4_ret_hlen(struct _Packet *);

sfip_t *    orig_ip4_ret_src(struct _Packet *);
sfip_t *    orig_ip4_ret_dst(struct _Packet *);
uint16_t   orig_ip4_ret_tos(struct _Packet *);
uint8_t    orig_ip4_ret_ttl(struct _Packet *);
uint16_t   orig_ip4_ret_len(struct _Packet *);
uint32_t   orig_ip4_ret_id(struct _Packet *);
uint8_t    orig_ip4_ret_proto(struct _Packet *);
uint16_t   orig_ip4_ret_off(struct _Packet *);
uint8_t    orig_ip4_ret_ver(struct _Packet *);
uint8_t    orig_ip4_ret_hlen(struct _Packet *);

sfip_t *    ip6_ret_src(struct _Packet *);
sfip_t *    ip6_ret_dst(struct _Packet *);
uint16_t   ip6_ret_toc(struct _Packet *);
uint8_t    ip6_ret_hops(struct _Packet *);
uint16_t   ip6_ret_len(struct _Packet *);
uint32_t   ip6_ret_id(struct _Packet *);
uint8_t    ip6_ret_next(struct _Packet *);
uint16_t   ip6_ret_off(struct _Packet *);
uint8_t    ip6_ret_ver(struct _Packet *);
uint8_t    ip6_ret_hlen(struct _Packet *);

sfip_t *    orig_ip6_ret_src(struct _Packet *);
sfip_t *    orig_ip6_ret_dst(struct _Packet *);
uint16_t   orig_ip6_ret_toc(struct _Packet *);
uint8_t    orig_ip6_ret_hops(struct _Packet *);
uint16_t   orig_ip6_ret_len(struct _Packet *);
uint32_t   orig_ip6_ret_id(struct _Packet *);
uint8_t    orig_ip6_ret_next(struct _Packet *);
uint16_t   orig_ip6_ret_off(struct _Packet *);
uint8_t    orig_ip6_ret_ver(struct _Packet *);
uint8_t    orig_ip6_ret_hlen(struct _Packet *);

typedef struct _IPH_API 
{
    sfip_t *    (*iph_ret_src)(struct _Packet *);
    sfip_t *    (*iph_ret_dst)(struct _Packet *);
    uint16_t   (*iph_ret_tos)(struct _Packet *);
    uint8_t    (*iph_ret_ttl)(struct _Packet *);
    uint16_t   (*iph_ret_len)(struct _Packet *);
    uint32_t   (*iph_ret_id)(struct _Packet *);
    uint8_t    (*iph_ret_proto)(struct _Packet *);
    uint16_t   (*iph_ret_off)(struct _Packet *);
    uint8_t    (*iph_ret_ver)(struct _Packet *);
    uint8_t    (*iph_ret_hlen)(struct _Packet *);

    sfip_t *    (*orig_iph_ret_src)(struct _Packet *);
    sfip_t *    (*orig_iph_ret_dst)(struct _Packet *);
    uint16_t   (*orig_iph_ret_tos)(struct _Packet *);
    uint8_t    (*orig_iph_ret_ttl)(struct _Packet *);
    uint16_t   (*orig_iph_ret_len)(struct _Packet *);
    uint32_t   (*orig_iph_ret_id)(struct _Packet *);
    uint8_t    (*orig_iph_ret_proto)(struct _Packet *);
    uint16_t   (*orig_iph_ret_off)(struct _Packet *);
    uint8_t    (*orig_iph_ret_ver)(struct _Packet *);
    uint8_t    (*orig_iph_ret_hlen)(struct _Packet *);
    char ver;
} IPH_API;

#ifdef SUP_IP6
extern IPH_API ip4;
extern IPH_API ip6;

#define IPH_API_V4 4
#define IPH_API_V6 6

#define iph_is_valid(p) (p->family != NO_IP)
#define NO_IP 0
#endif

#ifdef _MSC_VER
  /* Visual C++ pragma to enable warning messages about nonstandard bit field type */
  #pragma warning( default : 4214 )
#endif


/* Can't add any fields not in the real header here 
   because of how the decoder uses structure overlaying */
#ifdef _MSC_VER
  /* Visual C++ pragma to disable warning 
   * messages about nonstandard bit field type 
   */
  #pragma warning( disable : 4214 )  
#endif

#ifndef IPPROTO_IPIP
#define IPPROTO_IPIP 4
#endif

/* GRE related stuff */
typedef struct _GREHdr
{
    uint8_t flags;
    uint8_t version;
    uint16_t ether_type;

} GREHdr;

#ifdef GRE

#ifndef IPPROTO_GRE
#define IPPROTO_GRE 47
#endif

#define GRE_TYPE_TRANS_BRIDGING 0x6558
#define GRE_TYPE_PPP            0x880B

#define GRE_HEADER_LEN 4
#define GRE_CHKSUM_LEN 2
#define GRE_OFFSET_LEN 2
#define GRE_KEY_LEN 4
#define GRE_SEQ_LEN 4
#define GRE_SRE_HEADER_LEN 4

#define GRE_CHKSUM(x)  (x->flags & 0x80)
#define GRE_ROUTE(x)   (x->flags & 0x40)
#define GRE_KEY(x)     (x->flags & 0x20)
#define GRE_SEQ(x)     (x->flags & 0x10)
#define GRE_SSR(x)     (x->flags & 0x08)
#define GRE_RECUR(x)   (x->flags & 0x07)
#define GRE_VERSION(x)   (x->version & 0x07)
#define GRE_FLAGS(x)     (x->version & 0xF8)
#define GRE_PROTO(x)  ntohs(x->ether_type)

/* GRE version 1 used with PPTP */
#define GRE_V1_HEADER_LEN 8
#define GRE_V1_ACK_LEN 4
#define GRE_V1_FLAGS(x)  (x->version & 0x78)
#define GRE_V1_ACK(x)    (x->version & 0x80)

#endif  /* GRE */


/* more macros for TCP offset */
#define TCP_OFFSET(tcph)        (((tcph)->th_offx2 & 0xf0) >> 4)
#define TCP_X2(tcph)            ((tcph)->th_offx2 & 0x0f)

#define TCP_ISFLAGSET(tcph, flags) (((tcph)->th_flags & (flags)) == (flags))

/* we need to change them as well as get them */
#define SET_TCP_OFFSET(tcph, value)  ((tcph)->th_offx2 = (unsigned char)(((tcph)->th_offx2 & 0x0f) | (value << 4)))
#define SET_TCP_X2(tcph, value)  ((tcph)->th_offx2 = (unsigned char)(((tcph)->th_offx2 & 0xf0) | (value & 0x0f)))

typedef struct _TCPHdr
{
    uint16_t th_sport;     /* source port */
    uint16_t th_dport;     /* destination port */
    uint32_t th_seq;       /* sequence number */
    uint32_t th_ack;       /* acknowledgement number */
    uint8_t th_offx2;      /* offset and reserved */
    uint8_t th_flags;
    uint16_t th_win;       /* window */
    uint16_t th_sum;       /* checksum */
    uint16_t th_urp;       /* urgent pointer */

}       TCPHdr;
#ifdef _MSC_VER
  /* Visual C++ pragma to enable warning messages 
   * about nonstandard bit field type 
   */
  #pragma warning( default : 4214 )
#endif


typedef struct _UDPHdr
{
    uint16_t uh_sport;
    uint16_t uh_dport;
    uint16_t uh_len;
    uint16_t uh_chk;

}       UDPHdr;


typedef struct _ICMPHdr
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    union
    {
        struct
        {
            uint8_t pptr;
            uint8_t pres1;
            uint16_t pres2;
        } param;

        struct in_addr gwaddr;

        struct idseq
        {
            uint16_t id;
            uint16_t seq;
        } idseq;

        uint32_t sih_void;

        struct pmtu 
        {
            uint16_t ipm_void;
            uint16_t nextmtu;
        } pmtu;

        struct rtradv 
        {
            uint8_t num_addrs;
            uint8_t wpa;
            uint16_t lifetime;
        } rtradv;
    } icmp_hun;

#define s_icmp_pptr       icmp_hun.param.pptr
#define s_icmp_gwaddr     icmp_hun.gwaddr
#define s_icmp_id         icmp_hun.idseq.id
#define s_icmp_seq        icmp_hun.idseq.seq
#define s_icmp_void       icmp_hun.sih_void
#define s_icmp_pmvoid     icmp_hun.pmtu.ipm_void
#define s_icmp_nextmtu    icmp_hun.pmtu.nextmtu
#define s_icmp_num_addrs  icmp_hun.rtradv.num_addrs
#define s_icmp_wpa        icmp_hun.rtradv.wpa
#define s_icmp_lifetime   icmp_hun.rtradv.lifetime

    union 
    {
        /* timestamp */
        struct ts 
        {
            uint32_t otime;
            uint32_t rtime;
            uint32_t ttime;
        } ts;
        
        /* IP header for unreach */
        struct ih_ip  
        {
            IPHdr *ip;
            /* options and then 64 bits of data */
        } ip;
        
        struct ra_addr 
        {
            uint32_t addr;
            uint32_t preference;
        } radv;

        uint32_t mask;

        char    data[1];

    } icmp_dun;
#define s_icmp_otime      icmp_dun.ts.otime
#define s_icmp_rtime      icmp_dun.ts.rtime
#define s_icmp_ttime      icmp_dun.ts.ttime
#define s_icmp_ip         icmp_dun.ih_ip
#define s_icmp_radv       icmp_dun.radv
#define s_icmp_mask       icmp_dun.mask
#define s_icmp_data       icmp_dun.data

}        ICMPHdr;


typedef struct _ARPHdr
{
    uint16_t ar_hrd;       /* format of hardware address   */
    uint16_t ar_pro;       /* format of protocol address   */
    uint8_t ar_hln;        /* length of hardware address   */
    uint8_t ar_pln;        /* length of protocol address   */
    uint16_t ar_op;        /* ARP opcode (command)         */
}       ARPHdr;



typedef struct _EtherARP
{
    ARPHdr ea_hdr;      /* fixed-size header */
    uint8_t arp_sha[6];    /* sender hardware address */
    uint8_t arp_spa[4];    /* sender protocol address */
    uint8_t arp_tha[6];    /* target hardware address */
    uint8_t arp_tpa[4];    /* target protocol address */
}         EtherARP;


#ifndef NO_NON_ETHER_DECODER
typedef struct _EtherEapol
{
    uint8_t  version;  /* EAPOL proto version */
    uint8_t  eaptype;  /* EAPOL Packet type */
    uint16_t len;  /* Packet body length */
}         EtherEapol;

typedef struct _EAPHdr
{
    uint8_t code;
    uint8_t id;
    uint16_t len;
}         EAPHdr;

typedef struct _EapolKey
{
  uint8_t type;
  uint8_t length[2];
  uint8_t counter[8];
  uint8_t iv[16];
  uint8_t index;
  uint8_t sig[16];
}       EapolKey;
#endif  // NO_NON_ETHER_DECODER

typedef struct _Options
{
    uint8_t code;
    uint8_t len; /* length of the data section */
    const uint8_t *data;
} Options;

/* PPPoEHdr Header; EtherHdr plus the PPPoE Header */
typedef struct _PPPoEHdr
{
    EtherHdr ethhdr;            /* ethernet header */
    unsigned char ver_type;     /* pppoe version/type */
    unsigned char code;         /* pppoe code CODE_* */
    unsigned short session;     /* session id */
    unsigned short length;      /* payload length */
                                /* payload follows */
} PPPoEHdr;

/* PPPoE tag; the payload is a sequence of these */
typedef struct _PPPoE_Tag
{
    unsigned short type;    /* tag type TAG_* */
    unsigned short length;    /* tag length */
                            /* payload follows */
} PPPoE_Tag;

#define MPLS_HEADER_LEN    4
#define NUM_RESERVED_LABELS    16

typedef struct _MplsHdr
{
    uint32_t label;
    uint8_t  exp;
    uint8_t  bos;
    uint8_t  ttl;
} MplsHdr;

#define LAYER_MAX  32

// REMEMBER match any changes you make here in:
// dynamic-plugins/sf_engine/sf_snort_packet.h
typedef struct _Packet
{
    const DAQ_PktHdr_t *pkth;    // packet meta data
    const uint8_t *pkt;         // raw packet data

    //vvv------------------------------------------------
    // TODO convenience stuff to be refactored for layers
    //^^^------------------------------------------------

    //vvv-----------------------------
    EtherARP *ah;
    const EtherHdr *eh;         /* standard TCP/IP/Ethernet/ARP headers */
    const VlanTagHdr *vh;
    EthLlc *ehllc;
    EthLlcOther *ehllcother;
    const GREHdr *greh;
    uint32_t *mpls;

    const IPHdr *iph, *orig_iph;/* and orig. headers for ICMP_*_UNREACH family */
    const IPHdr *inner_iph;     /* if IP-in-IP, this will be the inner IP header */
    const IPHdr *outer_iph;     /* if IP-in-IP, this will be the outer IP header */
    const TCPHdr *tcph, *orig_tcph;
    const UDPHdr *udph, *orig_udph;
    const UDPHdr *inner_udph;   /* if Teredo + UDP, this will be the inner UDP header */
    const UDPHdr *outer_udph;   /* if Teredo + UDP, this will be the outer UDP header */
    const ICMPHdr *icmph, *orig_icmph;

    const uint8_t *data;        /* packet payload pointer */
    const uint8_t *ip_data;     /* IP payload pointer */
    const uint8_t *outer_ip_data;  /* Outer IP payload pointer */
    const uint8_t *ip_frag_start;
    const uint8_t *ip_options_data;
    const uint8_t *tcp_options_data;
    //^^^-----------------------------

    void *ssnptr;               /* for tcp session tracking info... */
    void *fragtracker;          /* for ip fragmentation tracking info... */
    void *flow;                 /* for flow info */
    void *streamptr;            /* for tcp pkt dump */
    
    //vvv-----------------------------
    IP4Hdr *ip4h, *orig_ip4h;   /* SUP_IP6 members */
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
    int bytes_to_inspect;       /* Number of bytes to check against rules */
                                /* this is not set - always 0 (inspect all) */

    /* int ip_payload_len; */   /* Replacement for IP_LEN(p->iph->ip_len) << 2 */
    /* int ip_payload_off; */   /* IP_LEN(p->iph->ip_len) << 2 + p->data */
    //^^^-----------------------------

    uint32_t preprocessor_bits; /* flags for preprocessors to check */
    uint32_t preproc_reassembly_pkt_bits;

    uint32_t caplen;
    uint32_t http_pipeline_count; /* Counter for HTTP pipelined requests */
    uint32_t packet_flags;      /* special flags for the packet */
    uint16_t proto_bits;
    uint16_t data_flags;

    //vvv-----------------------------
    uint16_t dsize;             /* packet payload size */
    uint16_t ip_dsize;          /* IP payload size */
    uint16_t alt_dsize;         /* the dsize of a packet before munging (used for log)*/
    uint16_t actual_ip_len;     /* for logging truncated pkts (usually by small snaplen)*/
    uint16_t outer_ip_dsize;    /* Outer IP payload size */
    //^^^-----------------------------

    uint16_t frag_offset;       /* fragment offset number */
    uint16_t ip_frag_len;
    uint16_t ip_options_len;
    uint16_t tcp_options_len;

    //vvv-----------------------------
    uint16_t sp;                /* source port (TCP/UDP) */
    uint16_t dp;                /* dest port (TCP/UDP) */
    uint16_t orig_sp;           /* source port (TCP/UDP) of original datagram */
    uint16_t orig_dp;           /* dest port (TCP/UDP) of original datagram */
    //^^^-----------------------------
    // and so on ...

    int16_t application_protocol_ordinal;

    uint8_t frag_flag;          /* flag to indicate a fragmented packet */
    uint8_t mf;                 /* more fragments flag */
    uint8_t df;                 /* don't fragment flag */
    uint8_t rf;                 /* IP reserved bit */

    uint8_t uri_count;          /* number of URIs in this packet */
    uint8_t error_flags;        /* flags indicate checksum errors, bad TTLs, etc. */
    uint8_t encapsulated;

    uint8_t ip_option_count;    /* number of options in this packet */
    uint8_t tcp_option_count;
    uint8_t ip6_extension_count;
    uint8_t ip6_frag_index;

    uint8_t ip_lastopt_bad;     /* flag to indicate that option decoding was
                                   halted due to a bad option */
    uint8_t tcp_lastopt_bad;    /* flag to indicate that option decoding was
                                   halted due to a bad option */

    uint8_t next_layer;         /* index into layers for next encap */

#ifndef NO_NON_ETHER_DECODER
    const Fddi_hdr *fddihdr;    /* FDDI support headers */
    Fddi_llc_saps *fddisaps;
    Fddi_llc_sna *fddisna;
    Fddi_llc_iparp *fddiiparp;    
    Fddi_llc_other *fddiother;

    const Trh_hdr *trh;         /* Token Ring support headers */
    Trh_llc *trhllc;
    Trh_mr *trhmr;

    Pflog1Hdr *pf1h;            /* OpenBSD pflog interface header - version 1 */
    Pflog2Hdr *pf2h;            /* OpenBSD pflog interface header - version 2 */
    Pflog3Hdr *pf3h;            /* OpenBSD pflog interface header - version 3 */

#ifdef DLT_LINUX_SLL
    const SLLHdr *sllh;         /* Linux cooked sockets header */
#endif
#ifdef DLT_IEEE802_11
    const WifiHdr *wifih;       /* wireless LAN header */
#endif
    const PPPoEHdr *pppoeh;     /* Encapsulated PPP of Ether header */

    const EtherEapol *eplh;     /* 802.1x EAPOL header */
    const EAPHdr *eaph;
    const uint8_t *eaptype;
    EapolKey *eapolk;
#endif

    // nothing after this point is zeroed ...
    Options ip_options[IP_OPTMAX];         /* ip options decode structure */
    Options tcp_options[TCP_OPTLENMAX];    /* tcp options decode struct */
    IP6Option ip6_extensions[IP6_EXTMAX];  /* IPv6 Extension References */

    const IP6RawHdr* raw_ip6h;  // innermost raw ip6 header
    Layer layers[LAYER_MAX];    /* decoded encapsulations */
    uint16_t max_dsize;

    /**policyId provided in configuration file. Used for correlating configuration 
     * with event output
     */
    uint16_t configPolicyId;

} Packet;

#define PKT_ZERO_LEN offsetof(Packet, ip_options)

#define PROTO_BIT__NONE     0x0000
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
#define GET_PKT_SEQ(p) (ntohl(p->tcph->th_seq))

/* Macros to deal with sequence numbers - p810 TCP Illustrated vol 2 */
#define SEQ_LT(a,b)  ((int)((a) - (b)) <  0)
#define SEQ_LEQ(a,b) ((int)((a) - (b)) <= 0)
#define SEQ_GT(a,b)  ((int)((a) - (b)) >  0)
#define SEQ_GEQ(a,b) ((int)((a) - (b)) >= 0)
#define SEQ_EQ(a,b)  ((int)((a) - (b)) == 0)

#ifdef SUP_IP6 
/* Sets the callbacks to point at the family selected by 
 *  * "family".  "family" is either AF_INET or AF_INET6 */
#define CALLBACK_IP 0
#define CALLBACK_ICMP_ORIG 1

static INLINE void set_callbacks(struct _Packet *p, int family, char orig)
{
    if (p == NULL) 
    {
        ErrorMessage("%s(%d) Can't set iph api callback: Packet is NULL.\n",
                     __FILE__, __LINE__);
        return;
    }

    if (orig == CALLBACK_IP)
    {
        if(family == AF_INET)
            p->iph_api = &ip4;
        else
            p->iph_api = &ip6;

        p->family = family;
    }
    else if (orig == CALLBACK_ICMP_ORIG)
    {
        if(family == AF_INET)
            p->orig_iph_api = &ip4;
        else
            p->orig_iph_api = &ip6;

        p->orig_family = family;
    }
    else
    {
        ErrorMessage("%s(%d) Can't set iph api callback: Invalid callback "
                     "type: %c.\n", __FILE__, __LINE__, orig);
        return;
    }
}
#endif

typedef struct s_pseudoheader
{
    uint32_t sip, dip; 
    uint8_t  zero;     
    uint8_t  protocol; 
    uint16_t len; 

} PSEUDO_HDR;

/* Default classification for decoder alerts */
#define DECODE_CLASS 25 

typedef struct _DecoderFlags
{
    char decode_alerts;   /* if decode.c alerts are going to be enabled */
    char oversized_alert;   /* alert if garbage after tcp/udp payload */
    char oversized_drop;   /* alert if garbage after tcp/udp payload */
    char drop_alerts;     /* drop alerts from decoder */
    char tcpopt_experiment;  /* TcpOptions Decoder */
    char drop_tcpopt_experiment; /* Drop alerts from TcpOptions Decoder */
    char tcpopt_obsolete;    /* Alert on obsolete TCP options */
    char drop_tcpopt_obsolete; /* Drop on alerts from obsolete TCP options */
    char tcpopt_ttcp;        /* Alert on T/TCP options */
    char drop_tcpopt_ttcp;   /* Drop on alerts from T/TCP options */
    char tcpopt_decode;      /* alert on decoder inconsistencies */
    char drop_tcpopt_decode; /* Drop on alerts from decoder inconsistencies */
    char ipopt_decode;      /* alert on decoder inconsistencies */
    char drop_ipopt_decode; /* Drop on alerts from decoder inconsistencies */

    /* To be moved to the frag preprocessor once it supports IPv6 */
    char ipv6_bad_frag_pkt;
    char bsd_icmp_frag;
    char drop_bad_ipv6_frag;    

} DecoderFlags;

#define        ALERTMSG_LENGTH 256


/*  P R O T O T Y P E S  ******************************************************/

// root decoders
void DecodeEthPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodePppPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeNullPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeRawPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeRawPkt6(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodePPPoEPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);

// chained decoders
void DecodeARP(const uint8_t *, uint32_t, Packet *);
void DecodeEthLoopback(const uint8_t *, uint32_t, Packet *);
void DecodeVlan(const uint8_t *, const uint32_t, Packet *);
void DecodePppPktEncapsulated(Packet *, const uint32_t, const uint8_t *);
void DecodeIP(const uint8_t *, const uint32_t, Packet *);
void DecodeIPV6(const uint8_t *, uint32_t, Packet *);
void DecodeTCP(const uint8_t *, const uint32_t, Packet *);
void DecodeUDP(const uint8_t *, const uint32_t, Packet *);
void DecodeICMP(const uint8_t *, const uint32_t, Packet *);
void DecodeICMPEmbeddedIP(const uint8_t *, const uint32_t, Packet *);
void DecodeICMPEmbeddedIP6(const uint8_t *, const uint32_t, Packet *);
void DecodeIPOptions(const uint8_t *, uint32_t, Packet *);
void DecodeTCPOptions(const uint8_t *, uint32_t, Packet *);
void DecodeTeredo(const uint8_t *, uint32_t, Packet *);
void DecodeESP(const uint8_t *, uint32_t, Packet *);

#ifdef GRE
void DecodeGRE(const uint8_t *, const uint32_t, Packet *);
void DecodeTransBridging(const uint8_t *, const uint32_t, Packet *);
#endif  /* GRE */
void DecoderAlertEncapsulated(Packet *, int, const char *, const uint8_t *, uint32_t);

#ifdef MPLS
int isPrivateIP(uint32_t addr);
void DecodeEthOverMPLS(Packet * p, const DAQ_PktHdr_t* pkthdr, const uint8_t * pkt);
void DecodeMPLS(const uint8_t * pkt, DAQ_PktHdr_t * pkthdr, Packet * p);
#endif

#ifndef NO_NON_ETHER_DECODER
void DecodeTRPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeFDDIPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeLinuxSLLPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeIEEE80211Pkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeSlipPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeI4LRawIPPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeI4LCiscoIPPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeChdlcPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodePflog(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeOldPflog(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodePppSerialPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);
void DecodeEncPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);

void DecodeEAP(const uint8_t *, const uint32_t, Packet *);
void DecodeEapol(const uint8_t *, uint32_t, Packet *);
void DecodeEapolKey(const uint8_t *, uint32_t, Packet *);
void DecodeIPX(const uint8_t *, uint32_t, Packet *);
#endif  // NO_NON_ETHER_DECODER

void BsdFragHashInit(int max);
void BsdFragHashCleanup(void);
void BsdFragHashReset(void);

#if defined(WORDS_MUSTALIGN) && !defined(__GNUC__)
uint32_t EXTRACT_32BITS (u_char *);
#endif /* WORDS_MUSTALIGN && !__GNUC__ */

extern void UpdateDecodeRulesArray(uint32_t sid, int bOn, int bAll);

/*Decode functions that need to be called once the policies are set */
extern void DecodePolicySpecific(Packet *);

/* XXX not sure where this guy needs to live at the moment */
typedef struct _PortList
{
    int ports[32];   /* 32 is kind of arbitrary */

    int num_entries;

} PortList;

void InitSynToMulticastDstIp( void );
void SynToMulticastDstIpDestroy( void );

#define SFTARGET_UNKNOWN_PROTOCOL -1

static INLINE int PacketWasCooked(Packet* p)
{
    return ( p->packet_flags &
        ( PKT_REBUILT_STREAM | PKT_REBUILT_FRAG |
          PKT_DCE_RPKT | PKT_DCE_SEG | PKT_DCE_FRAG |
          PKT_SMB_SEG | PKT_SMB_TRANS | PKT_PSEUDO) ) != 0;
}

#endif  /* __DECODE_H__ */

