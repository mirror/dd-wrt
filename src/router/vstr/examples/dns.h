#ifndef DNS_H
#define DNS_H

#define DNS_CLASS_IN     1
#define DNS_CLASS_CH     3
#define DNS_CLASS_HS     4
#define DNS_CLASS_NONE 254
#define DNS_CLASS_ALL  255

#define DNS_TYPE_ALL 255

#define DNS_TYPE_IN_A       1
#define DNS_TYPE_IN_NS      2
/* ... */
#define DNS_TYPE_IN_CNAME   5
#define DNS_TYPE_IN_SOA     6
/* ... */
/* #define DNS_TYPE_IN_WKS    11 */
#define DNS_TYPE_IN_PTR    12
#define DNS_TYPE_IN_HINFO  13
/* ... */
#define DNS_TYPE_IN_MX     15
#define DNS_TYPE_IN_TXT    16
/* ... */
#define DNS_TYPE_IN_AAAA   28
#define DNS_TYPE_IN_SRV    33
/* ... */
/* #define DNS_TYPE_IN_A6     38 -- experimental again */
/* ... */
#define DNS_TYPE_IN_IXFR  251
#define DNS_TYPE_IN_AXFR  252


#define DNS_TYPE_CH_A         1
#define DNS_TYPE_CH_TXT      16 /* version.bind */

#define DNS_HDR_QR         (1U<<15)
#define DNS_HDR_OPCOFF     (11)
#define DNS_HDR_OPCMASK    (15U<<DNS_HDR_OPCOFF)
#define DNS_HDR_OPC_QUERY  ( 0U<<DNS_HDR_OPCOFF)
#define DNS_HDR_OPC_IQUERY ( 1U<<DNS_HDR_OPCOFF)
#define DNS_HDR_OPC_STATUS ( 2U<<DNS_HDR_OPCOFF)
#define DNS_HDR_OPC_NOTIFY ( 4U<<DNS_HDR_OPCOFF)
#define DNS_HDR_OPC_UPDATE ( 5U<<DNS_HDR_OPCOFF)
#define DNS_HDR_AA         (1U<<10)
#define DNS_HDR_TC         (1U<<9)
#define DNS_HDR_RD         (1U<<8)
#define DNS_HDR_RA         (1U<<7)
#define DNS_HDR_ZOFF       (4)
#define DNS_HDR_ZMASK      (0x7U<<DNS_HDR_ZOFF)
#define DNS_HDR_ROFF       (0)
#define DNS_HDR_RMASK      (0xFU<<DNS_HDR_ROFF)
#define DNS_HDR_R_NONE     ( 0U<<DNS_HDR_ROFF)
#define DNS_HDR_R_BFMT     ( 1U<<DNS_HDR_ROFF)
#define DNS_HDR_R_SERV     ( 2U<<DNS_HDR_ROFF)
#define DNS_HDR_R_NAME     ( 3U<<DNS_HDR_ROFF)
#define DNS_HDR_R_NSUP     ( 4U<<DNS_HDR_ROFF)
#define DNS_HDR_R_REFU     ( 5U<<DNS_HDR_ROFF)
/* yxdomain 6 */
/* yxrrset  7 */
/* nxrrset  8 */
/* notauth  9 */
/* notzone 10 */

#define DNS_HDR_RSZ       (6)

#define DNS_LABEL_IS_PTR(x) ((0xC0 & (x)) == 0xC0)

struct Dns_base
{
 struct Vstr_base *io_w_serv;
 struct Vstr_base *io_w_user;

 struct Vlg       *io_dbg;
 
 unsigned int opt_recur : 1;
};

typedef struct Dns_base Dns_base;

extern const char *dns_name_type_ch(unsigned int num);
extern const char *dns_name_type_in(unsigned int num);
extern const char *dns_name_class(unsigned int num);
extern const char *dns_name_hdr_r(unsigned int num);

extern unsigned int dns_get_msg_len(Vstr_base *s1, size_t pos);
extern void dns_app_recq_pkt(struct Dns_base *, unsigned int qcount, ...);
extern void dns_dbg_prnt_pkt(struct Dns_base *, Vstr_base *pkt);
extern void dns_sc_ui_out(struct Dns_base *, Vstr_base *pkt);

#endif
