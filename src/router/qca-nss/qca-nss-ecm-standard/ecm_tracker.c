/*
 **************************************************************************
 * Copyright (c) 2014-2015, 2018, 2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_TRACKER_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
int ecm_tracker_data_total = 0;				/* Data total for all skb list instances */
int ecm_tracker_data_buffer_total = 0;			/* Data buffer total allocated for all skb list instances */
int ecm_tracker_data_limit = ECM_TRACKER_GLOBAL_DATA_LIMIT_DEFAULT;
int ecm_tracker_data_buffer_limit = ECM_TRACKER_GLOBAL_DATA_BUFFER_LIMIT_DEFAULT;
							/* Tracked limit for data across all instances */
static DEFINE_SPINLOCK(ecm_tracker_lock);		/* Global lock for the tracker globals */
#endif

struct ecm_tracker_ip_protocols;

typedef bool (*ecm_tracker_ip_header_helper_method_t)(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
							/* A header helper method is used to inspect a protocol header, determine if it is valid, its size etc. and where any next header will be esp. important for IPv6 */

/*
 * Forward references for the header helpers
 */
static bool ecm_tracker_ip_header_helper_tcp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_udp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_icmp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_unknown(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_gre(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
#ifdef ECM_IPV6_ENABLE
static bool ecm_tracker_ip_header_helper_ipv6_generic(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_ipv6_fragment(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_ah(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
static bool ecm_tracker_ip_header_helper_ipv6_icmp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr);
#endif

/*
 * struct ecm_tracker_ip_protocols_known[]
 *	A list of protocols known to the tracker and which it will record the location of in its ecm_tracker_ip_header.
 *
 * This is especially important for IPv6 which can have multiple headers in its packet.
 */
static struct ecm_tracker_ip_protocols {
	uint8_t ip_protocol;					/* The IP protocol we want to detect and record its information */
	ecm_tracker_ip_protocol_type_t ecm_ip_protocol;		/* The ECM Tracker protocol identifier equivalent */
	char *name;						/* Visual name of the protocol */
	ecm_tracker_ip_header_helper_method_t header_helper;	/* A function used to help process the header, e.g. its size etc. When a NULL helper is located, header processing stops. */
} ecm_tracker_ip_protocols_known[256] =
{
#ifdef ECM_IPV6_ENABLE
	{0, ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_HBH, "ipv6_hbh", ecm_tracker_ip_header_helper_ipv6_generic},
#else
	{0, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "0", ecm_tracker_ip_header_helper_unknown},
#endif
	{1, ECM_TRACKER_IP_PROTOCOL_TYPE_ICMP, "icmp", ecm_tracker_ip_header_helper_icmp},
	{2, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "2", ecm_tracker_ip_header_helper_unknown},
	{3, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "3", ecm_tracker_ip_header_helper_unknown},
	{4, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "4", ecm_tracker_ip_header_helper_unknown},
	{5, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "5", ecm_tracker_ip_header_helper_unknown},
	{6, ECM_TRACKER_IP_PROTOCOL_TYPE_TCP, "tcp", ecm_tracker_ip_header_helper_tcp},
	{7, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "7", ecm_tracker_ip_header_helper_unknown},
	{8, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "8", ecm_tracker_ip_header_helper_unknown},
	{9, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "9", ecm_tracker_ip_header_helper_unknown},
	{10, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "10", ecm_tracker_ip_header_helper_unknown},
	{11, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "11", ecm_tracker_ip_header_helper_unknown},
	{12, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "12", ecm_tracker_ip_header_helper_unknown},
	{13, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "13", ecm_tracker_ip_header_helper_unknown},
	{14, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "14", ecm_tracker_ip_header_helper_unknown},
	{15, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "15", ecm_tracker_ip_header_helper_unknown},
	{16, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "16", ecm_tracker_ip_header_helper_unknown},
	{17, ECM_TRACKER_IP_PROTOCOL_TYPE_UDP, "udp", ecm_tracker_ip_header_helper_udp},
	{18, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "18", ecm_tracker_ip_header_helper_unknown},
	{19, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "19", ecm_tracker_ip_header_helper_unknown},
	{20, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "20", ecm_tracker_ip_header_helper_unknown},
	{21, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "21", ecm_tracker_ip_header_helper_unknown},
	{22, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "22", ecm_tracker_ip_header_helper_unknown},
	{23, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "23", ecm_tracker_ip_header_helper_unknown},
	{24, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "24", ecm_tracker_ip_header_helper_unknown},
	{25, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "25", ecm_tracker_ip_header_helper_unknown},
	{26, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "26", ecm_tracker_ip_header_helper_unknown},
	{27, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "27", ecm_tracker_ip_header_helper_unknown},
	{28, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "28", ecm_tracker_ip_header_helper_unknown},
	{29, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "29", ecm_tracker_ip_header_helper_unknown},
	{30, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "30", ecm_tracker_ip_header_helper_unknown},
	{31, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "31", ecm_tracker_ip_header_helper_unknown},
	{32, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "32", ecm_tracker_ip_header_helper_unknown},
	{33, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "33", ecm_tracker_ip_header_helper_unknown},
	{34, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "34", ecm_tracker_ip_header_helper_unknown},
	{35, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "35", ecm_tracker_ip_header_helper_unknown},
	{36, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "36", ecm_tracker_ip_header_helper_unknown},
	{37, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "37", ecm_tracker_ip_header_helper_unknown},
	{38, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "38", ecm_tracker_ip_header_helper_unknown},
	{39, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "39", ecm_tracker_ip_header_helper_unknown},
	{40, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "40", ecm_tracker_ip_header_helper_unknown},
	{41, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "41", ecm_tracker_ip_header_helper_unknown},
	{42, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "42", ecm_tracker_ip_header_helper_unknown},
#ifdef ECM_IPV6_ENABLE
	{43, ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_ROUTING, "ipv6_routing", ecm_tracker_ip_header_helper_ipv6_generic},
	{44, ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_FRAGMENT, "ipv6_fragment", ecm_tracker_ip_header_helper_ipv6_fragment},
#else
	{43, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "43", ecm_tracker_ip_header_helper_unknown},
	{44, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "44", ecm_tracker_ip_header_helper_unknown},
#endif
	{45, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "45", ecm_tracker_ip_header_helper_unknown},
	{46, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "46", ecm_tracker_ip_header_helper_unknown},
	{47, ECM_TRACKER_IP_PROTOCOL_TYPE_GRE, "gre", ecm_tracker_ip_header_helper_gre},
	{48, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "48", ecm_tracker_ip_header_helper_unknown},
	{49, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "49", ecm_tracker_ip_header_helper_unknown},
	{50, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "50", ecm_tracker_ip_header_helper_unknown},
#ifdef ECM_IPV6_ENABLE
	{51, ECM_TRACKER_IP_PROTOCOL_TYPE_AH, "ah", ecm_tracker_ip_header_helper_ah},
#else
	{51, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "51", ecm_tracker_ip_header_helper_unknown},
#endif
	{52, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "52", ecm_tracker_ip_header_helper_unknown},
	{53, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "53", ecm_tracker_ip_header_helper_unknown},
	{54, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "54", ecm_tracker_ip_header_helper_unknown},
	{55, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "55", ecm_tracker_ip_header_helper_unknown},
#ifdef ECM_IPV6_ENABLE
	{56, ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_ICMP, "ipv6_icmp", ecm_tracker_ip_header_helper_ipv6_icmp},
#else
	{56, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "56", ecm_tracker_ip_header_helper_unknown},
#endif
	{57, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "57", ecm_tracker_ip_header_helper_unknown},
	{58, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "58", ecm_tracker_ip_header_helper_unknown},
	{59, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "59", ecm_tracker_ip_header_helper_unknown},
#ifdef ECM_IPV6_ENABLE
	{60, ECM_TRACKER_IP_PROTOCOL_TYPE_IPV6_DO, "ipv6_do", ecm_tracker_ip_header_helper_ipv6_generic},
#else
	{60, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "60", ecm_tracker_ip_header_helper_unknown},
#endif
	{61, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "61", ecm_tracker_ip_header_helper_unknown},
	{62, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "62", ecm_tracker_ip_header_helper_unknown},
	{63, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "63", ecm_tracker_ip_header_helper_unknown},
	{64, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "64", ecm_tracker_ip_header_helper_unknown},
	{65, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "65", ecm_tracker_ip_header_helper_unknown},
	{66, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "66", ecm_tracker_ip_header_helper_unknown},
	{67, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "67", ecm_tracker_ip_header_helper_unknown},
	{68, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "68", ecm_tracker_ip_header_helper_unknown},
	{69, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "69", ecm_tracker_ip_header_helper_unknown},
	{70, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "70", ecm_tracker_ip_header_helper_unknown},
	{71, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "71", ecm_tracker_ip_header_helper_unknown},
	{72, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "72", ecm_tracker_ip_header_helper_unknown},
	{73, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "73", ecm_tracker_ip_header_helper_unknown},
	{74, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "74", ecm_tracker_ip_header_helper_unknown},
	{75, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "75", ecm_tracker_ip_header_helper_unknown},
	{76, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "76", ecm_tracker_ip_header_helper_unknown},
	{77, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "77", ecm_tracker_ip_header_helper_unknown},
	{78, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "78", ecm_tracker_ip_header_helper_unknown},
	{79, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "79", ecm_tracker_ip_header_helper_unknown},
	{80, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "80", ecm_tracker_ip_header_helper_unknown},
	{81, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "81", ecm_tracker_ip_header_helper_unknown},
	{82, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "82", ecm_tracker_ip_header_helper_unknown},
	{83, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "83", ecm_tracker_ip_header_helper_unknown},
	{84, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "84", ecm_tracker_ip_header_helper_unknown},
	{85, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "85", ecm_tracker_ip_header_helper_unknown},
	{86, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "86", ecm_tracker_ip_header_helper_unknown},
	{87, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "87", ecm_tracker_ip_header_helper_unknown},
	{88, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "88", ecm_tracker_ip_header_helper_unknown},
	{89, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "89", ecm_tracker_ip_header_helper_unknown},
	{90, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "90", ecm_tracker_ip_header_helper_unknown},
	{91, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "91", ecm_tracker_ip_header_helper_unknown},
	{92, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "92", ecm_tracker_ip_header_helper_unknown},
	{93, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "93", ecm_tracker_ip_header_helper_unknown},
	{94, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "94", ecm_tracker_ip_header_helper_unknown},
	{95, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "95", ecm_tracker_ip_header_helper_unknown},
	{96, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "96", ecm_tracker_ip_header_helper_unknown},
	{97, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "97", ecm_tracker_ip_header_helper_unknown},
	{98, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "98", ecm_tracker_ip_header_helper_unknown},
	{99, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "99", ecm_tracker_ip_header_helper_unknown},
	{100, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "100", ecm_tracker_ip_header_helper_unknown},
	{101, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "101", ecm_tracker_ip_header_helper_unknown},
	{102, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "102", ecm_tracker_ip_header_helper_unknown},
	{103, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "103", ecm_tracker_ip_header_helper_unknown},
	{104, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "104", ecm_tracker_ip_header_helper_unknown},
	{105, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "105", ecm_tracker_ip_header_helper_unknown},
	{106, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "106", ecm_tracker_ip_header_helper_unknown},
	{107, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "107", ecm_tracker_ip_header_helper_unknown},
	{108, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "108", ecm_tracker_ip_header_helper_unknown},
	{109, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "109", ecm_tracker_ip_header_helper_unknown},
	{110, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "110", ecm_tracker_ip_header_helper_unknown},
	{111, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "111", ecm_tracker_ip_header_helper_unknown},
	{112, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "112", ecm_tracker_ip_header_helper_unknown},
	{113, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "113", ecm_tracker_ip_header_helper_unknown},
	{114, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "114", ecm_tracker_ip_header_helper_unknown},
	{115, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "115", ecm_tracker_ip_header_helper_unknown},
	{116, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "116", ecm_tracker_ip_header_helper_unknown},
	{117, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "117", ecm_tracker_ip_header_helper_unknown},
	{118, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "118", ecm_tracker_ip_header_helper_unknown},
	{119, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "119", ecm_tracker_ip_header_helper_unknown},
	{120, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "120", ecm_tracker_ip_header_helper_unknown},
	{121, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "121", ecm_tracker_ip_header_helper_unknown},
	{122, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "122", ecm_tracker_ip_header_helper_unknown},
	{123, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "123", ecm_tracker_ip_header_helper_unknown},
	{124, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "124", ecm_tracker_ip_header_helper_unknown},
	{125, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "125", ecm_tracker_ip_header_helper_unknown},
	{126, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "126", ecm_tracker_ip_header_helper_unknown},
	{127, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "127", ecm_tracker_ip_header_helper_unknown},
	{128, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "128", ecm_tracker_ip_header_helper_unknown},
	{129, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "129", ecm_tracker_ip_header_helper_unknown},
	{130, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "130", ecm_tracker_ip_header_helper_unknown},
	{131, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "131", ecm_tracker_ip_header_helper_unknown},
	{132, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "132", ecm_tracker_ip_header_helper_unknown},
	{133, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "133", ecm_tracker_ip_header_helper_unknown},
	{134, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "134", ecm_tracker_ip_header_helper_unknown},
	{135, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "135", ecm_tracker_ip_header_helper_unknown},
	{136, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "136", ecm_tracker_ip_header_helper_unknown},
	{137, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "137", ecm_tracker_ip_header_helper_unknown},
	{138, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "138", ecm_tracker_ip_header_helper_unknown},
	{139, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "139", ecm_tracker_ip_header_helper_unknown},
	{140, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "140", ecm_tracker_ip_header_helper_unknown},
	{141, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "141", ecm_tracker_ip_header_helper_unknown},
	{142, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "142", ecm_tracker_ip_header_helper_unknown},
	{143, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "143", ecm_tracker_ip_header_helper_unknown},
	{144, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "144", ecm_tracker_ip_header_helper_unknown},
	{145, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "145", ecm_tracker_ip_header_helper_unknown},
	{146, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "146", ecm_tracker_ip_header_helper_unknown},
	{147, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "147", ecm_tracker_ip_header_helper_unknown},
	{148, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "148", ecm_tracker_ip_header_helper_unknown},
	{149, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "149", ecm_tracker_ip_header_helper_unknown},
	{150, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "150", ecm_tracker_ip_header_helper_unknown},
	{151, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "151", ecm_tracker_ip_header_helper_unknown},
	{152, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "152", ecm_tracker_ip_header_helper_unknown},
	{153, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "153", ecm_tracker_ip_header_helper_unknown},
	{154, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "154", ecm_tracker_ip_header_helper_unknown},
	{155, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "155", ecm_tracker_ip_header_helper_unknown},
	{156, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "156", ecm_tracker_ip_header_helper_unknown},
	{157, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "157", ecm_tracker_ip_header_helper_unknown},
	{158, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "158", ecm_tracker_ip_header_helper_unknown},
	{159, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "159", ecm_tracker_ip_header_helper_unknown},
	{160, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "160", ecm_tracker_ip_header_helper_unknown},
	{161, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "161", ecm_tracker_ip_header_helper_unknown},
	{162, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "162", ecm_tracker_ip_header_helper_unknown},
	{163, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "163", ecm_tracker_ip_header_helper_unknown},
	{164, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "164", ecm_tracker_ip_header_helper_unknown},
	{165, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "165", ecm_tracker_ip_header_helper_unknown},
	{166, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "166", ecm_tracker_ip_header_helper_unknown},
	{167, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "167", ecm_tracker_ip_header_helper_unknown},
	{168, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "168", ecm_tracker_ip_header_helper_unknown},
	{169, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "169", ecm_tracker_ip_header_helper_unknown},
	{170, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "170", ecm_tracker_ip_header_helper_unknown},
	{171, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "171", ecm_tracker_ip_header_helper_unknown},
	{172, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "172", ecm_tracker_ip_header_helper_unknown},
	{173, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "173", ecm_tracker_ip_header_helper_unknown},
	{174, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "174", ecm_tracker_ip_header_helper_unknown},
	{175, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "175", ecm_tracker_ip_header_helper_unknown},
	{176, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "176", ecm_tracker_ip_header_helper_unknown},
	{177, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "177", ecm_tracker_ip_header_helper_unknown},
	{178, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "178", ecm_tracker_ip_header_helper_unknown},
	{179, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "179", ecm_tracker_ip_header_helper_unknown},
	{180, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "180", ecm_tracker_ip_header_helper_unknown},
	{181, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "181", ecm_tracker_ip_header_helper_unknown},
	{182, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "182", ecm_tracker_ip_header_helper_unknown},
	{183, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "183", ecm_tracker_ip_header_helper_unknown},
	{184, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "184", ecm_tracker_ip_header_helper_unknown},
	{185, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "185", ecm_tracker_ip_header_helper_unknown},
	{186, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "186", ecm_tracker_ip_header_helper_unknown},
	{187, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "187", ecm_tracker_ip_header_helper_unknown},
	{188, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "188", ecm_tracker_ip_header_helper_unknown},
	{189, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "189", ecm_tracker_ip_header_helper_unknown},
	{190, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "190", ecm_tracker_ip_header_helper_unknown},
	{191, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "191", ecm_tracker_ip_header_helper_unknown},
	{192, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "192", ecm_tracker_ip_header_helper_unknown},
	{193, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "193", ecm_tracker_ip_header_helper_unknown},
	{194, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "194", ecm_tracker_ip_header_helper_unknown},
	{195, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "195", ecm_tracker_ip_header_helper_unknown},
	{196, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "196", ecm_tracker_ip_header_helper_unknown},
	{197, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "197", ecm_tracker_ip_header_helper_unknown},
	{198, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "198", ecm_tracker_ip_header_helper_unknown},
	{199, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "199", ecm_tracker_ip_header_helper_unknown},
	{200, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "200", ecm_tracker_ip_header_helper_unknown},
	{201, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "201", ecm_tracker_ip_header_helper_unknown},
	{202, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "202", ecm_tracker_ip_header_helper_unknown},
	{203, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "203", ecm_tracker_ip_header_helper_unknown},
	{204, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "204", ecm_tracker_ip_header_helper_unknown},
	{205, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "205", ecm_tracker_ip_header_helper_unknown},
	{206, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "206", ecm_tracker_ip_header_helper_unknown},
	{207, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "207", ecm_tracker_ip_header_helper_unknown},
	{208, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "208", ecm_tracker_ip_header_helper_unknown},
	{209, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "209", ecm_tracker_ip_header_helper_unknown},
	{210, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "210", ecm_tracker_ip_header_helper_unknown},
	{211, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "211", ecm_tracker_ip_header_helper_unknown},
	{212, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "212", ecm_tracker_ip_header_helper_unknown},
	{213, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "213", ecm_tracker_ip_header_helper_unknown},
	{214, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "214", ecm_tracker_ip_header_helper_unknown},
	{215, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "215", ecm_tracker_ip_header_helper_unknown},
	{216, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "216", ecm_tracker_ip_header_helper_unknown},
	{217, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "217", ecm_tracker_ip_header_helper_unknown},
	{218, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "218", ecm_tracker_ip_header_helper_unknown},
	{219, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "219", ecm_tracker_ip_header_helper_unknown},
	{220, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "220", ecm_tracker_ip_header_helper_unknown},
	{221, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "221", ecm_tracker_ip_header_helper_unknown},
	{222, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "222", ecm_tracker_ip_header_helper_unknown},
	{223, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "223", ecm_tracker_ip_header_helper_unknown},
	{224, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "224", ecm_tracker_ip_header_helper_unknown},
	{225, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "225", ecm_tracker_ip_header_helper_unknown},
	{226, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "226", ecm_tracker_ip_header_helper_unknown},
	{227, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "227", ecm_tracker_ip_header_helper_unknown},
	{228, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "228", ecm_tracker_ip_header_helper_unknown},
	{229, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "229", ecm_tracker_ip_header_helper_unknown},
	{230, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "230", ecm_tracker_ip_header_helper_unknown},
	{231, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "231", ecm_tracker_ip_header_helper_unknown},
	{232, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "232", ecm_tracker_ip_header_helper_unknown},
	{233, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "233", ecm_tracker_ip_header_helper_unknown},
	{234, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "234", ecm_tracker_ip_header_helper_unknown},
	{235, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "235", ecm_tracker_ip_header_helper_unknown},
	{236, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "236", ecm_tracker_ip_header_helper_unknown},
	{237, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "237", ecm_tracker_ip_header_helper_unknown},
	{238, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "238", ecm_tracker_ip_header_helper_unknown},
	{239, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "239", ecm_tracker_ip_header_helper_unknown},
	{240, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "240", ecm_tracker_ip_header_helper_unknown},
	{241, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "241", ecm_tracker_ip_header_helper_unknown},
	{242, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "242", ecm_tracker_ip_header_helper_unknown},
	{243, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "243", ecm_tracker_ip_header_helper_unknown},
	{244, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "244", ecm_tracker_ip_header_helper_unknown},
	{245, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "245", ecm_tracker_ip_header_helper_unknown},
	{246, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "246", ecm_tracker_ip_header_helper_unknown},
	{247, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "247", ecm_tracker_ip_header_helper_unknown},
	{248, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "248", ecm_tracker_ip_header_helper_unknown},
	{249, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "249", ecm_tracker_ip_header_helper_unknown},
	{250, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "250", ecm_tracker_ip_header_helper_unknown},
	{251, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "251", ecm_tracker_ip_header_helper_unknown},
	{252, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "252", ecm_tracker_ip_header_helper_unknown},
	{253, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "253", ecm_tracker_ip_header_helper_unknown},
	{254, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "254", ecm_tracker_ip_header_helper_unknown},
	{255, ECM_TRACKER_IP_PROTOCOL_TYPE_UNKNOWN, "255", ecm_tracker_ip_header_helper_unknown}
};

static char *ecm_tracker_sender_state_strings[] = {
	"Unknown",
	"Establishing",
	"Established",
	"Closing",
	"Closed",
	"Fault"
};

static char *ecm_tracker_connection_state_strings[] = {
	"Establishing",
	"Established",
	"Closing",
	"Closed",
	"Fault"
};

/*
 * ecm_tracker_ip_check_header_and_read()
 *	Check that we have a complete network-level IP header, check it and return true if so.
 */
bool ecm_tracker_ip_check_header_and_read(struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb)
{
	struct iphdr *v4_hdr = NULL;
	uint16_t remain;
#ifdef ECM_IPV6_ENABLE
	struct ipv6hdr *v6_hdr = NULL;
	int16_t this_header;
	int16_t prev_header;
	uint32_t offset;
#endif

	memset(ip_hdr, 0, sizeof(*ip_hdr));

	ip_hdr->skb = skb;

	/*
	 * What version of IP header are we dealing with?
	 */
	v4_hdr = skb_header_pointer(skb, 0, sizeof(struct iphdr), &ip_hdr->h.v4_hdr);
	if (v4_hdr && (v4_hdr->version == 4)) {
		DEBUG_TRACE("%px: skb: %px is ipv4\n", ip_hdr, skb);
		ip_hdr->is_v4 = true;
		goto version_check_done;
	}

#ifdef ECM_IPV6_ENABLE
	/*
	 * Try V6
	 */
	DEBUG_TRACE("skb: %px contains no v4 header\n", skb);
	v6_hdr = skb_header_pointer(skb, 0, sizeof(struct ipv6hdr), &ip_hdr->h.v6_hdr);
	if (!v6_hdr || (v6_hdr->version != 6)) {
		DEBUG_TRACE("skb: %px contains no v6 header\n", skb);
		return false;
	}
	DEBUG_TRACE("%px: skb: %px is ipv6\n", ip_hdr, skb);
	ip_hdr->is_v4 = false;
#else
	DEBUG_TRACE("skb: %px Other IP header versions unsupported\n", skb);
	return false;
#endif

version_check_done:
	if (ip_hdr->is_v4) {
		uint8_t protocol;
		int16_t next_unused;
		ecm_tracker_ip_protocol_type_t ecm_ip_protocol;
		struct ecm_tracker_ip_protocols *etip;
		struct ecm_tracker_ip_protocol_header *etiph;

		DEBUG_TRACE("%px: skb: %px ipv4\n", ip_hdr, skb);

		/*
		 * Process IPv4
		 */
		ECM_NIN4_ADDR_TO_IP_ADDR(ip_hdr->src_addr, v4_hdr->saddr);
		ECM_NIN4_ADDR_TO_IP_ADDR(ip_hdr->dest_addr, v4_hdr->daddr);
		ip_hdr->ip_header_length = v4_hdr->ihl;
		ip_hdr->ip_header_length <<= 2;
		if (ip_hdr->ip_header_length < 20) {
			DEBUG_WARN("%px: v4 invalid ip hdr len %d\n", skb, ip_hdr->ip_header_length);
			return false;
		}
		ip_hdr->total_length = ntohs(v4_hdr->tot_len);
		if (skb->len < ip_hdr->total_length) {
			DEBUG_WARN("%px: v4 invalid total len: %u skb len: %u\n", skb, ip_hdr->total_length, skb->len);
			return false;
		}
		remain = ip_hdr->total_length - ip_hdr->ip_header_length;

		/*
		 * Fragmented?
		 */
		ip_hdr->fragmented = (ntohs(v4_hdr->frag_off) & 0x3fff)? true : false;

		/*
		 * DSCP value is the 6 most significant bits of tos field, so left shifting
		 * the tos value by 2 gives the DSCP value.
		 */
		ip_hdr->dscp = v4_hdr->tos >> 2;

		/*
		 * DS field
		 */
		ip_hdr->ds = ipv4_get_dsfield(v4_hdr);

		/*
		 * TTL field
		 */
		ip_hdr->ttl = v4_hdr->ttl;

		/*
		 * Get the protocol and where the header info will be stored
		 */
		protocol = v4_hdr->protocol;
		ip_hdr->protocol = (int)v4_hdr->protocol;
		etip = &ecm_tracker_ip_protocols_known[protocol];	/* Get header helper */
		ecm_ip_protocol = etip->ecm_ip_protocol;
		etiph = &ip_hdr->headers[ecm_ip_protocol];		/* Get where the header detail is stored in ip_hdr->headers[] */

		DEBUG_TRACE("%px: v4 skb: %px, len: %d, ip_header_length: %u, total_length: %u, remain: %u, fragmented: %04x, protocol: %u, ecm_ip_protocol: %d src_addr: "
				ECM_IP_ADDR_DOT_FMT ", dest addr: " ECM_IP_ADDR_DOT_FMT "\n",
				ip_hdr,
				skb,
				skb->len,
				ip_hdr->ip_header_length,
				ip_hdr->total_length,
				remain,
				ip_hdr->fragmented,
				ip_hdr->protocol,
				ecm_ip_protocol,
				ECM_IP_ADDR_TO_DOT(ip_hdr->src_addr),
				ECM_IP_ADDR_TO_DOT(ip_hdr->dest_addr));

		/*
		 * Populate protocol detail
		 * IPv4 can only have one.
		 */
		if (!etip->header_helper(
				etip,
				etiph,
				ip_hdr,
				skb,
				(uint8_t)ip_hdr->protocol,
				ecm_ip_protocol,
				ip_hdr->ip_header_length,
				remain,
				&next_unused)) {
			DEBUG_WARN("%px: v4 header helper failed for: %u\n", skb, protocol);
			return false;
		}

		return true;
	}

#ifndef ECM_IPV6_ENABLE
	return false;
#else
	/*
	 * IPv6
	 */
	ECM_NIN6_ADDR_TO_IP_ADDR(ip_hdr->src_addr, v6_hdr->saddr);
	ECM_NIN6_ADDR_TO_IP_ADDR(ip_hdr->dest_addr, v6_hdr->daddr);
	ip_hdr->ip_header_length = 40;
	remain = ntohs(v6_hdr->payload_len);
	ip_hdr->total_length = remain + 40;
	if (skb->len < ip_hdr->total_length) {
		DEBUG_WARN("%px: v6 invalid total len: %u skb len: %u\n", skb, ip_hdr->total_length, skb->len);
		return false;
	}

	/*
	 * In IPv6 header, the version-class-flow_label is organized as:
	 * version: u8:4
	 * priority: u8:4
	 * flow_lbl[3]
	 *
	 * Version(31-28), CLASS(27-20), FLOW(19-0)
	 *
	 * The dscp value is the most 6 significant bit of traffic-class field. Its 4-bits
	 * belong to the priority field of the IPv6 header, the other 2-bits belong to the
	 * flow_lbl[0]. So, to calculate the dscp value, we need to right shift the priority by 2 and OR
	 * it with flow_lbl[0]'s most 2 significant bits.
	 *
	 * ECM_TRACKER_IPV6_FLOW_LBL_PRIORITY_MASK 0xC0
	 * ECM_TRACKER_IPV6_FLOW_LBL_PRIORITY_SHIFT 6
	 * ECM_TRACKER_IPV6_PRIORITY_SHIFT 2
	 */
	ip_hdr->dscp = (v6_hdr->priority << ECM_TRACKER_IPV6_PRIORITY_SHIFT) |
			((v6_hdr->flow_lbl[0] & ECM_TRACKER_IPV6_FLOW_LBL_PRIORITY_MASK) >> ECM_TRACKER_IPV6_FLOW_LBL_PRIORITY_SHIFT);

	/*
	 * DS field
	 */
	ip_hdr->ds = ipv6_get_dsfield(v6_hdr);

	/*
	 * hop_limit field
	 */
	ip_hdr->ttl = v6_hdr->hop_limit;

	/*
	 * Process headers until we run out of space, error, or we get the no next header marker for v6 (protocol 59).
	 */
	offset = 40;
	this_header = (int16_t)v6_hdr->nexthdr;
	prev_header = this_header;
	while ((remain > 0) && (this_header >= 0) && (this_header != 59)) {
		struct ecm_tracker_ip_protocols *etip;
		struct ecm_tracker_ip_protocol_header *etiph;
		ecm_tracker_ip_protocol_type_t ecm_ip_protocol;
		int16_t next_header;

		etip = &ecm_tracker_ip_protocols_known[this_header];	/* Get header helper */
		ecm_ip_protocol = etip->ecm_ip_protocol;
		etiph = &ip_hdr->headers[ecm_ip_protocol];		/* Get where the header detail is stored in ip_hdr->headers[] */

		/*
		 * If this IP header has already been seen then we abort
		 */
		if (etiph->size) {
			DEBUG_WARN("v6 skb: %px, protocol: %d already seen at offset: %u, size: %u\n",
					skb, this_header, etiph->offset, etiph->size);
			return false;
		}
		if (!etip->header_helper(
				etip,
				etiph,
				ip_hdr,
				skb,
				(uint8_t)this_header,
				ecm_ip_protocol,
				offset,
				remain,
				&next_header)) {
			DEBUG_WARN("%px: v6 header helper failed for: %d\n", skb, this_header);
			return false;
		}

		offset += etiph->size;
		DEBUG_ASSERT(remain >= etiph->size, "v6 remain: %u goes negative after header size: %u", remain, etiph->size);
		remain -= etiph->size;

		prev_header = this_header;
		this_header = next_header;
	}

	/*
	 * Generally the last protocol seen is the upper layer protocol
	 */
	ip_hdr->protocol = (int)prev_header;

	return true;
#endif
}
EXPORT_SYMBOL(ecm_tracker_ip_check_header_and_read);

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_tracker_ip_header_helper_ipv6_generic()
 *	Interpret a Generic IPv6 extension header
 */
static bool ecm_tracker_ip_header_helper_ipv6_generic(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	struct ipv6_gen_hdr {
		uint8_t next_protocol;
		uint8_t header_ext_len;
	} gen_header_buffer;
	struct ipv6_gen_hdr *gen_header;
	uint16_t hdr_size;

	/*
	 * At least 8 bytes
	 */
	if (remain < 8) {
		return false;
	}

	gen_header = skb_header_pointer(skb, offset, sizeof(struct ipv6_gen_hdr), &gen_header_buffer);
	if (!gen_header) {
		return false;
	}
	hdr_size = gen_header->header_ext_len;
	hdr_size <<= 3;
	hdr_size += 8;
	if (remain < hdr_size) {
		DEBUG_WARN("IPv6 extension: %px packet remain: %u too small for tcp header: %u\n", skb, remain, hdr_size);
		return false;
	}
	if (unlikely(ip_hdr->total_length < (offset + hdr_size))) {
		DEBUG_WARN("TCP packet %px too short (total_length: %u, require: %u)\n", skb, ip_hdr->total_length, offset + hdr_size);
		return false;
	}

	etiph->protocol_number = protocol;
	etiph->header_size = hdr_size;
	etiph->size = hdr_size;
	etiph->offset = offset;

	*next_hdr = (int16_t)gen_header->next_protocol;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_ipv6_fragment()
 *	Interpret a Generic IPv6 fragment header
 */
static bool ecm_tracker_ip_header_helper_ipv6_fragment(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
							struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	struct ipv6_gen_hdr {
		uint8_t next_protocol;
		uint8_t reserved;
#ifdef ECM_INTERFACE_MAP_T_ENABLE
		__be16 frag_off;
#endif
	} gen_header_buffer;
	struct ipv6_gen_hdr *gen_header;

	/*
	 * At least 8 bytes
	 */
	if (remain < 8) {
		return false;
	}

	gen_header = skb_header_pointer(skb, offset, sizeof(struct ipv6_gen_hdr), &gen_header_buffer);
	if (!gen_header) {
		return false;
	}
	if (unlikely(ip_hdr->total_length < (offset + 8))) {
		DEBUG_WARN("TCP packet %px too short (total_length: %u, require: %u)\n", skb, ip_hdr->total_length, offset + 8);
		return false;
	}

	/*
	 * The very presence of a fragmemt header says it's fragmented
	 */
	ip_hdr->fragmented = true;

#ifdef ECM_INTERFACE_MAP_T_ENABLE
	/*
	 * In general, any IPv6 packet with fragment header is a fragment
	 * packet.
	 * But, packet with dummy fragment header is an exception to this case
	 * and represent a complete IPv6 packet.
	 * So, below is a check to identify the presence of dummy fragment
	 * header, and if present, unset the fragment flag.
	 */
	if ((gen_header->frag_off & htons(IP6_OFFSET | IP6_MF)) == 0) {
		DEBUG_TRACE("Pkt has dummy header\n");
		ip_hdr->fragmented = false;
	}
#endif

	etiph->protocol_number = protocol;
	etiph->header_size = 8;
	etiph->size = 8;
	etiph->offset = offset;

	*next_hdr = (int16_t)gen_header->next_protocol;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_ah()
 *	Interpret an Authentication Header
 */
static bool ecm_tracker_ip_header_helper_ah(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	struct ah_gen_hdr {
		uint8_t next_protocol;
		uint8_t header_len;
	} gen_header_buffer;
	struct ah_gen_hdr *gen_header;
	uint16_t hdr_size;

	/*
	 * At least 8 bytes
	 */
	if (remain < 8) {
		return false;
	}

	gen_header = skb_header_pointer(skb, offset, sizeof(struct ah_gen_hdr), &gen_header_buffer);
	if (!gen_header) {
		return false;
	}

	hdr_size = gen_header->header_len + 2;
	hdr_size <<= 2;

	if (!ip_hdr->is_v4) {
		/*
		 * hdr_size needs to be a multiple of 8 in a v6 frame
		 */
		if (hdr_size % 8) {
			DEBUG_WARN("AH packet %px not multiple of 8 for v6 frame: %u\n", skb, hdr_size);
			return false;
		}
	}

	if (remain < hdr_size) {
		DEBUG_WARN("AH packet %px too short (total_length: %u, require: %u)\n", skb, ip_hdr->total_length, offset + hdr_size);
		return false;
	}
	if (unlikely(ip_hdr->total_length < (offset + hdr_size))) {
		DEBUG_WARN("AH packet %px too short (total_length: %u, require: %u)\n", skb, ip_hdr->total_length, offset + hdr_size);
		return false;
	}

	/*
	 * What header follows this one?
	 */
	*next_hdr = gen_header->next_protocol;

	etiph->protocol_number = protocol;
	etiph->header_size = hdr_size;
	etiph->size = hdr_size;
	etiph->offset = offset;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_ipv6_icmp()
 *	Interpret a ICMP V6 header
 */
static bool ecm_tracker_ip_header_helper_ipv6_icmp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	if (remain < 4) {
		DEBUG_WARN("v6 icmp: %px too small: %u\n", skb, remain);
		return false;
	}

	etiph->protocol_number = protocol;
	etiph->header_size = 4;
	etiph->size = remain;
	etiph->offset = offset;

	/*
	 * There is no header following a V6 ICMP header
	 */
	*next_hdr = -1;
	return true;
}

#endif

/*
 * ecm_tracker_ip_header_helper_tcp()
 *	Interpret a TCP header
 */
static bool ecm_tracker_ip_header_helper_tcp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	struct tcphdr tcp_header_buffer;
	struct tcphdr *tcp_header;
	uint16_t hdr_size;

	DEBUG_ASSERT((protocol == IPPROTO_TCP) && (ecm_ip_protocol == ECM_TRACKER_IP_PROTOCOL_TYPE_TCP), "Bad protocol: %u or ecm_ip_protocol: %d", protocol, ecm_ip_protocol);

	if (remain < 20) {
		return false;
	}

	tcp_header = skb_header_pointer(skb, offset, sizeof(struct tcphdr), &tcp_header_buffer);
	if (!tcp_header) {
		return false;
	}
	hdr_size = tcp_header->doff;
	hdr_size <<= 2;
	if (hdr_size < 20) {
		return false;
	}
	if (remain < hdr_size) {
		DEBUG_WARN("TCP packet: %px packet remain: %u too small for tcp header: %u\n", skb, remain, hdr_size);
		return false;
	}
	if (unlikely(ip_hdr->total_length < (offset + hdr_size))) {
		DEBUG_WARN("TCP packet %px too short (total_length: %u, require: %u)\n", skb, ip_hdr->total_length, offset + hdr_size);
		return false;
	}

	etiph->protocol_number = protocol;
	etiph->header_size = hdr_size;
	etiph->size = remain;
	etiph->offset = offset;

	/*
	 * There is no header following a TCP header
	 */
	*next_hdr = -1;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_gre()
 *	Interpret a GRE header
 */
static bool ecm_tracker_ip_header_helper_gre(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	uint32_t gre_hdr_buffer;
	uint32_t *gre_hdr_ptr;
	uint32_t gre_hdr;
	uint16_t hdr_size;

	DEBUG_ASSERT((protocol == IPPROTO_GRE) && (ecm_ip_protocol == ECM_TRACKER_IP_PROTOCOL_TYPE_GRE), "Bad protocol: %u or ecm_ip_protocol: %d", protocol, ecm_ip_protocol);

	if (remain < 4) {
		return false;
	}

	gre_hdr_ptr = skb_header_pointer(skb, offset, sizeof(gre_hdr_buffer), &gre_hdr_buffer);
	if (!gre_hdr_ptr) {
		return false;
	}
	gre_hdr = ntohl(*gre_hdr_ptr);
	hdr_size = 4;
	if (gre_hdr & 0x80000000) {
		/*
		 * Checksum
		 */
		hdr_size += 4;
	}
	if (gre_hdr & 0x20000000) {
		/*
		 * Key
		 */
		hdr_size += 4;
	}
	if (gre_hdr & 0x10000000) {
		/*
		 * Sequence
		 */
		hdr_size += 4;
	}
	if (remain < hdr_size) {
		DEBUG_WARN("GRE packet: %px packet remain: %u too small for tcp header: %u\n", skb, remain, hdr_size);
		return false;
	}
	if (unlikely(ip_hdr->total_length < (offset + hdr_size))) {
		DEBUG_WARN("GRE packet %px too short (total_length: %u, require: %u)\n", skb, ip_hdr->total_length, offset + hdr_size);
		return false;
	}

	etiph->protocol_number = protocol;
	etiph->header_size = hdr_size;
	etiph->size = remain;
	etiph->offset = offset;

	/*
	 * There is no header following a GRE header
	 */
	*next_hdr = -1;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_udp()
 *	Interpret a UDP header
 */
static bool ecm_tracker_ip_header_helper_udp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	DEBUG_ASSERT((protocol == IPPROTO_UDP) && (ecm_ip_protocol == ECM_TRACKER_IP_PROTOCOL_TYPE_UDP), "Bad protocol: %u or ecm_ip_protocol: %d", protocol, ecm_ip_protocol);

	DEBUG_TRACE("udp helper skb: %px, protocol: %u, ecm_ip_proto: %d, offset: %u, remain: %u\n", skb, protocol, ecm_ip_protocol, offset, remain);
	if (remain < 8) {
		DEBUG_TRACE("not enough UDP header: %u\n", remain);
		return false;
	}

	etiph->protocol_number = protocol;
	etiph->header_size = 8;
	etiph->size = remain;
	etiph->offset = offset;

	/*
	 * There is no header following a UDP header
	 */
	*next_hdr = -1;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_unknown()
 *	Interpret a unknown header
 */
static bool ecm_tracker_ip_header_helper_unknown(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
							struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	/*
	 * There is no header following an unknown header
	 */
	*next_hdr = -1;
	etiph->protocol_number = protocol;
	etiph->header_size = remain;
	etiph->size = remain;
	etiph->offset = offset;
	return true;
}

/*
 * ecm_tracker_ip_header_helper_icmp()
 *	Interpret a ICMP V4 header
 */
static bool ecm_tracker_ip_header_helper_icmp(struct ecm_tracker_ip_protocols *etip, struct ecm_tracker_ip_protocol_header *etiph, struct ecm_tracker_ip_header *ip_hdr,
						struct sk_buff *skb, uint8_t protocol, ecm_tracker_ip_protocol_type_t ecm_ip_protocol, uint32_t offset, uint32_t remain, int16_t *next_hdr)
{
	struct icmphdr icmp_header_buffer;
	struct icmphdr *icmp_header;

	DEBUG_ASSERT((protocol == IPPROTO_ICMP) && (ecm_ip_protocol == ECM_TRACKER_IP_PROTOCOL_TYPE_ICMP), "Bad protocol: %u or ecm_ip_protocol: %d", protocol, ecm_ip_protocol);

	if (remain < 8) {
		return false;
	}

	icmp_header = skb_header_pointer(skb, offset, sizeof(struct icmphdr), &icmp_header_buffer);
	if (!icmp_header) {
		return false;
	}
	switch(icmp_header->type) {
	case ICMP_SOURCE_QUENCH:
	case ICMP_REDIRECT:
	case ICMP_TIME_EXCEEDED:
	case ICMP_DEST_UNREACH:
		/*
		 * Should be at least our 8 bytes and minimum 20 bytes for an IP header.
		 * NOTE: We are not looking at options that extend this header!
		 */
		if (remain < 28) {
			DEBUG_WARN("icmp skb: %px type: %u too small: %u\n", skb, icmp_header->type, remain);
			return false;
		}
		etiph->header_size = 8;
		etiph->size = remain;
		break;

	case ICMP_ADDRESS:
	case ICMP_ADDRESSREPLY:
	case ICMP_TIMESTAMP:
		if (remain < 12) {
			DEBUG_WARN("icmp skb: %px type: %u too small: %u\n", skb, icmp_header->type, remain);
			return false;
		}
		etiph->header_size = 12;
		etiph->size = remain;
		break;
	case ICMP_TIMESTAMPREPLY:
		if (remain < 20) {
			DEBUG_WARN("icmp skb: %px type: %u too small: %u\n", skb, icmp_header->type, remain);
			return false;
		}
		etiph->header_size = 20;
		etiph->size = remain;
		break;
	case ICMP_PARAMETERPROB:
	case ICMP_INFO_REQUEST:
	case ICMP_INFO_REPLY:
	default:
		etiph->header_size = 8;
		etiph->size = remain;
	}
	etiph->protocol_number = protocol;
	etiph->offset = offset;

	/*
	 * There is no header following a ICMP header
	 */
	*next_hdr = -1;
	return true;
}

#ifdef ECM_TRACKER_DPI_SUPPORT_ENABLE
/*
 * ecm_tracker_data_limit_set()
 *	Set global tracked data limit
 */
void ecm_tracker_data_limit_set(uint32_t limit)
{
	int data_limit = (int)limit;
	DEBUG_ASSERT(data_limit > 0, "Invalid limit %d\n", data_limit);
	spin_lock_bh(&ecm_tracker_lock);
	ecm_tracker_data_limit = data_limit;
	spin_unlock_bh(&ecm_tracker_lock);
}
EXPORT_SYMBOL(ecm_tracker_data_limit_set);

/*
 * ecm_tracker_data_limit_get()
 *	Return global tracked data limit
 */
uint32_t ecm_tracker_data_limit_get(void)
{
	uint32_t data_total;
	spin_lock_bh(&ecm_tracker_lock);
	data_total = (uint32_t)ecm_tracker_data_limit;
	spin_unlock_bh(&ecm_tracker_lock);
	return data_total;
}
EXPORT_SYMBOL(ecm_tracker_data_limit_get);

/*
 * ecm_tracker_data_total_get()
 *	Return global tracked data quantity
 */
uint32_t ecm_tracker_data_total_get(void)
{
	uint32_t data_total;
	spin_lock_bh(&ecm_tracker_lock);
	data_total = (uint32_t)ecm_tracker_data_total;
	spin_unlock_bh(&ecm_tracker_lock);
	return data_total;
}
EXPORT_SYMBOL(ecm_tracker_data_total_get);

/*
 * ecm_tracker_data_buffer_total_get()
 *	Return global tracked data buffer size
 */
uint32_t ecm_tracker_data_buffer_total_get(void)
{
	uint32_t data_buffer_total;
	spin_lock_bh(&ecm_tracker_lock);
	data_buffer_total = (uint32_t)ecm_tracker_data_buffer_total;
	spin_unlock_bh(&ecm_tracker_lock);
	return data_buffer_total;
}
EXPORT_SYMBOL(ecm_tracker_data_buffer_total_get);

/*
 * ecm_tracker_data_total_increase()
 *	Increase global tracked data quantity
 *
 * If this function returns false then the increase has been denied and tracking of that data should not occur.
 * Therefore call this function BEFORE tracking the actual data.
 */
bool ecm_tracker_data_total_increase(uint32_t n, uint32_t data_buffer_size)
{
	spin_lock_bh(&ecm_tracker_lock);

	/*
	 * Would we exceed our global limit?
	 */
	DEBUG_ASSERT((ecm_tracker_data_total + (int)n) > 0, "bad total\n");
	if (((ecm_tracker_data_buffer_total + data_buffer_size)  > ecm_tracker_data_buffer_limit)
			|| ((ecm_tracker_data_total + n) > ecm_tracker_data_limit)) {
		spin_unlock_bh(&ecm_tracker_lock);
		return false;
	}
	ecm_tracker_data_buffer_total += data_buffer_size;
	ecm_tracker_data_total += (int)n;
	spin_unlock_bh(&ecm_tracker_lock);
	return true;
}
EXPORT_SYMBOL(ecm_tracker_data_total_increase);

/*
 * ecm_tracker_data_total_decrease()
 *	Decrease global tracked data quantity
 */
void ecm_tracker_data_total_decrease(uint32_t n, uint32_t data_buffer_size)
{
	spin_lock_bh(&ecm_tracker_lock);
	ecm_tracker_data_total -= (int)n;
	ecm_tracker_data_buffer_total -= data_buffer_size;
	DEBUG_ASSERT(ecm_tracker_data_total >= 0, "bad total\n");
	DEBUG_ASSERT(ecm_tracker_data_buffer_total >= 0, "bad total\n");
	spin_unlock_bh(&ecm_tracker_lock);
}
EXPORT_SYMBOL(ecm_tracker_data_total_decrease);
#endif

/*
 * ecm_tracker_module_get()
 *	Take a reference to the module
 */
void ecm_tracker_module_get(void)
{
	try_module_get(THIS_MODULE);
}
EXPORT_SYMBOL(ecm_tracker_module_get);

/*
 * ecm_tracker_module_put()
 *	Release a reference to the module
 */
void ecm_tracker_module_put(void)
{
	module_put(THIS_MODULE);
}
EXPORT_SYMBOL(ecm_tracker_module_put);

/*
 * ecm_tracker_sender_state_to_string()
 *	Convert a sender state to a string equivalent
 */
const char *ecm_tracker_sender_state_to_string(enum ecm_tracker_sender_states s)
{
	return ecm_tracker_sender_state_strings[s];
}
EXPORT_SYMBOL(ecm_tracker_sender_state_to_string);

/*
 * ecm_tracker_connection_state_to_string()
 *	Convert a connection state to its string equivalent
 */
const char *ecm_tracker_connection_state_to_string(enum ecm_tracker_connection_states s)
{
	return ecm_tracker_connection_state_strings[s];
}
EXPORT_SYMBOL(ecm_tracker_connection_state_to_string);
