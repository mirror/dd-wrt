/* vi: set sw=4 ts=4: */
#ifndef UTILS_H
#define UTILS_H 1

#include "libnetlink.h"
#include "ll_map.h"
#include "rtm_map.h"

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

extern family_t preferred_family;
extern smallint show_stats;    /* UNUSED */
extern smallint show_details;  /* UNUSED */
extern smallint show_raw;      /* UNUSED */
extern smallint resolve_hosts; /* UNUSED */
extern smallint oneline;
extern char _SL_;

#ifndef IPPROTO_ESP
#define IPPROTO_ESP  50
#endif
#ifndef IPPROTO_AH
#define IPPROTO_AH  51
#endif
#ifndef IPPROTO_ETHERIP
#define IPPROTO_ETHERIP	97
#endif

#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)  char x[SPRINT_BSIZE]

typedef struct
{
	uint16_t flags;
	uint16_t bytelen;
	int16_t bitlen;
	/* These next two fields match rtvia */
	uint16_t family;
	uint32_t data[64];
} inet_prefix;

enum {
	PREFIXLEN_SPECIFIED	= (1 << 0),
	ADDRTYPE_INET		= (1 << 1),
	ADDRTYPE_UNSPEC		= (1 << 2),
	ADDRTYPE_MULTI		= (1 << 3),

	ADDRTYPE_INET_UNSPEC	= ADDRTYPE_INET | ADDRTYPE_UNSPEC,
	ADDRTYPE_INET_MULTI	= ADDRTYPE_INET | ADDRTYPE_MULTI
};

#define DN_MAXADDL 20
#ifndef AF_DECnet
#define AF_DECnet 12
#endif

struct dn_naddr {
	unsigned short a_len;
	unsigned char  a_addr[DN_MAXADDL];
};

#define IPX_NODE_LEN 6

struct ipx_addr {
	uint32_t ipx_net;
	uint8_t  ipx_node[IPX_NODE_LEN];
};

char** next_arg(char **argv) FAST_FUNC;
#define NEXT_ARG() do { argv = next_arg(argv); } while (0)

static inline void inet_prefix_reset(inet_prefix *p)
{
	p->flags = 0;
}

uint32_t get_addr32(char *name) FAST_FUNC;
int get_addr_1(inet_prefix *dst, const char *arg, int family) FAST_FUNC;
/*void get_prefix_1(inet_prefix *dst, char *arg, int family) FAST_FUNC;*/
int get_addr(inet_prefix *dst, const char *arg, int family) FAST_FUNC;
void get_prefix(inet_prefix *dst, char *arg, int family) FAST_FUNC;

unsigned get_unsigned(char *arg, const char *errmsg) FAST_FUNC;
uint32_t get_u32(char *arg, const char *errmsg) FAST_FUNC;
uint8_t get_u8(char *arg, const char *errmsg) FAST_FUNC;
uint16_t get_u16(const char *arg, const char *errmsg) FAST_FUNC;

const char *rt_addr_n2a(int af, void *addr) FAST_FUNC;
#ifdef RESOLVE_HOSTNAMES
const char *format_host(int af, int len, void *addr) FAST_FUNC;
#else
#define format_host(af, len, addr) \
	rt_addr_n2a(af, addr)
#endif

void invarg_1_to_2(const char *, const char *) FAST_FUNC NORETURN;
void duparg(const char *, const char *) FAST_FUNC NORETURN;
void duparg2(const char *, const char *) FAST_FUNC NORETURN;

int inet_addr_match(const inet_prefix *a, const inet_prefix *b, int bits) FAST_FUNC;

//const char *dnet_ntop(int af, const void *addr, char *str, size_t len);
//int dnet_pton(int af, const char *src, void *addr);

//const char *ipx_ntop(int af, const void *addr, char *str, size_t len);
//int ipx_pton(int af, const char *src, void *addr);

unsigned get_hz(void) FAST_FUNC;

/* Reference: RFC 5462, RFC 3032
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                Label                  | TC  |S|       TTL     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *	Label:  Label Value, 20 bits
 *	TC:     Traffic Class field, 3 bits
 *	S:      Bottom of Stack, 1 bit
 *	TTL:    Time to Live, 8 bits
 */

struct mpls_label {
	__be32 entry;
};

#define MPLS_LS_LABEL_MASK      0xFFFFF000
#define MPLS_LS_LABEL_SHIFT     12
#define MPLS_LS_TC_MASK         0x00000E00
#define MPLS_LS_TC_SHIFT        9
#define MPLS_LS_S_MASK          0x00000100
#define MPLS_LS_S_SHIFT         8
#define MPLS_LS_TTL_MASK        0x000000FF
#define MPLS_LS_TTL_SHIFT       0

/* Reserved labels */
#define MPLS_LABEL_IPV4NULL		0 /* RFC3032 */
#define MPLS_LABEL_RTALERT		1 /* RFC3032 */
#define MPLS_LABEL_IPV6NULL		2 /* RFC3032 */
#define MPLS_LABEL_IMPLNULL		3 /* RFC3032 */
#define MPLS_LABEL_ENTROPY		7 /* RFC6790 */
#define MPLS_LABEL_GAL			13 /* RFC5586 */
#define MPLS_LABEL_OAMALERT		14 /* RFC3429 */
#define MPLS_LABEL_EXTENSION		15 /* RFC7274 */

#define MPLS_LABEL_FIRST_UNRESERVED	16 /* RFC3032 */

/* These are embedded into IFLA_STATS_AF_SPEC:
 * [IFLA_STATS_AF_SPEC]
 * -> [AF_MPLS]
 *    -> [MPLS_STATS_xxx]
 *
 * Attributes:
 * [MPLS_STATS_LINK] = {
 *     struct mpls_link_stats
 * }
 */
enum {
	MPLS_STATS_UNSPEC, /* also used as 64bit pad attribute */
	MPLS_STATS_LINK,
	__MPLS_STATS_MAX,
};

#define MPLS_STATS_MAX (__MPLS_STATS_MAX - 1)

struct mpls_link_stats {
	__u64	rx_packets;		/* total packets received	*/
	__u64	tx_packets;		/* total packets transmitted	*/
	__u64	rx_bytes;		/* total bytes received		*/
	__u64	tx_bytes;		/* total bytes transmitted	*/
	__u64	rx_errors;		/* bad packets received		*/
	__u64	tx_errors;		/* packet transmit problems	*/
	__u64	rx_dropped;		/* packet dropped on receive	*/
	__u64	tx_dropped;		/* packet dropped on transmit	*/
	__u64	rx_noroute;		/* no route for packet dest	*/
};



POP_SAVED_FUNCTION_VISIBILITY

#endif
