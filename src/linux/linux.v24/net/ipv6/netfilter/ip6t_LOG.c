/*
 * This is a module which is used for logging packets.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/spinlock.h>
#include <linux/icmpv6.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/ipv6.h>
#include <linux/netfilter_ipv6/ip6_tables.h>

MODULE_AUTHOR("Jan Rekorajski <baggins@pld.org.pl>");
MODULE_DESCRIPTION("IP6 tables LOG target module");
MODULE_LICENSE("GPL");

struct in_device;
#include <net/route.h>
#include <linux/netfilter_ipv6/ip6t_LOG.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])

/* FIXME evil kludge */

struct ahhdr {
	__u8  nexthdr;
	__u8  hdrlen;
	__u16 reserved;
	__u32 spi;
	__u32 seq_no;
};

struct esphdr {
	__u32 spi;
	__u32 seq_no;
};

/* Use lock to serialize, so printks don't overlap */
static spinlock_t log_lock = SPIN_LOCK_UNLOCKED;

/* One level of recursion won't kill us */
static void dump_packet(const struct ip6t_log_info *info,
			const struct sk_buff *skb, unsigned int ip6hoff,
			int recurse)
{
	u_int8_t currenthdr;
	int fragment;
	struct ipv6hdr *ipv6h;
	unsigned int ptr;
	unsigned int hdrlen = 0;

	if (skb->len - ip6hoff < sizeof(*ipv6h)) {
		printk("TRUNCATED");
		return;
	}
	ipv6h = (struct ipv6hdr *)(skb->data + ip6hoff);

	/* Max length: 88 "SRC=0000.0000.0000.0000.0000.0000.0000.0000 DST=0000.0000.0000.0000.0000.0000.0000.0000" */
	printk("SRC=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x ", NIP6(ipv6h->saddr));
	printk("DST=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x ", NIP6(ipv6h->daddr));

	/* Max length: 44 "LEN=65535 TC=255 HOPLIMIT=255 FLOWLBL=FFFFF " */
	printk("LEN=%Zu TC=%u HOPLIMIT=%u FLOWLBL=%u ",
	       ntohs(ipv6h->payload_len) + sizeof(struct ipv6hdr),
	       (ntohl(*(u_int32_t *)ipv6h) & 0x0ff00000) >> 20,
	       ipv6h->hop_limit,
	       (ntohl(*(u_int32_t *)ipv6h) & 0x000fffff));

	fragment = 0;
	ptr = ip6hoff + sizeof(struct ipv6hdr);
	currenthdr = ipv6h->nexthdr;
	while (currenthdr != NEXTHDR_NONE && ip6t_ext_hdr(currenthdr)) {
		struct ipv6_opt_hdr *hdr;

		if (skb->len - ptr < sizeof(*hdr)) {
			printk("TRUNCATED");
			return;
		}
		hdr = (struct ipv6_opt_hdr *)(skb->data + ptr);

		/* Max length: 48 "OPT (...) " */
		if (info->logflags & IP6T_LOG_IPOPT)
			printk("OPT ( ");

		switch (currenthdr) {
		case IPPROTO_FRAGMENT: {
			struct frag_hdr *fhdr;

			printk("FRAG:");
			if (skb->len - ptr < sizeof(*fhdr)) {
				printk("TRUNCATED ");
				return;
			}
			fhdr = (struct frag_hdr *)(skb->data + ptr);

			/* Max length: 6 "65535 " */
			printk("%u ", ntohs(fhdr->frag_off) & 0xFFF8);

			/* Max length: 11 "INCOMPLETE " */
			if (fhdr->frag_off & htons(0x0001))
				printk("INCOMPLETE ");

			printk("ID:%08x ", ntohl(fhdr->identification));

			if (ntohs(fhdr->frag_off) & 0xFFF8)
				fragment = 1;

			hdrlen = 8;

			break;
		}
		case IPPROTO_DSTOPTS:
		case IPPROTO_ROUTING:
		case IPPROTO_HOPOPTS:
			if (fragment) {
				if (info->logflags & IP6T_LOG_IPOPT)
					printk(")");
				return;
			}
			hdrlen = ipv6_optlen(hdr);
			break;
		/* Max Length */
		case IPPROTO_AH:
			if (info->logflags & IP6T_LOG_IPOPT) {
				struct ahhdr *ah;

				/* Max length: 3 "AH " */
				printk("AH ");

				if (fragment) {
					printk(")");
					return;
				}

				if (skb->len - ptr < sizeof(*ah)) {
					/*
					 * Max length: 26 "INCOMPLETE [65535 	
					 *  bytes] )"
					 */
					printk("INCOMPLETE [%u bytes] )",
					       skb->len - ptr);
					return;
				}
				ah = (struct ahhdr *)(skb->data + ptr);

				/* Length: 15 "SPI=0xF1234567 */
				printk("SPI=0x%x ", ntohl(ah->spi));

			}

			hdrlen = (hdr->hdrlen+2)<<2;
			break;
		case IPPROTO_ESP:
			if (info->logflags & IP6T_LOG_IPOPT) {
				struct esphdr *esph;

				/* Max length: 4 "ESP " */
				printk("ESP ");

				if (fragment) {
					printk(")");
					return;
				}

				/*
				 * Max length: 26 "INCOMPLETE [65535 bytes] )"
				 */
				if (skb->len - ptr < sizeof(*esph)) {
					printk("INCOMPLETE [%u bytes] )",
					       skb->len - ptr);
					return;
				}
				esph = (struct esphdr *)(skb->data + ptr);

				/* Length: 16 "SPI=0xF1234567 )" */
				printk("SPI=0x%x )", ntohl(esph->spi) );

			}
			return;
		default:
			/* Max length: 20 "Unknown Ext Hdr 255" */
			printk("Unknown Ext Hdr %u", currenthdr);
			return;
		}
		if (info->logflags & IP6T_LOG_IPOPT)
			printk(") ");

		currenthdr = hdr->nexthdr;
		ptr += hdrlen;
	}

	switch (currenthdr) {
	case IPPROTO_TCP: {
		struct tcphdr *tcph;

		/* Max length: 10 "PROTO=TCP " */
		printk("PROTO=TCP ");

		if (fragment)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		if (skb->len - ptr < sizeof(*tcph)) {
			printk("INCOMPLETE [%u bytes] ", skb->len - ptr);
			return;
		}
		tcph = (struct tcphdr *)(skb->data + ptr);

		/* Max length: 20 "SPT=65535 DPT=65535 " */
		printk("SPT=%u DPT=%u ",
		       ntohs(tcph->source), ntohs(tcph->dest));
		/* Max length: 30 "SEQ=4294967295 ACK=4294967295 " */
		if (info->logflags & IP6T_LOG_TCPSEQ)
			printk("SEQ=%u ACK=%u ",
			       ntohl(tcph->seq), ntohl(tcph->ack_seq));
		/* Max length: 13 "WINDOW=65535 " */
		printk("WINDOW=%u ", ntohs(tcph->window));
		/* Max length: 9 "RES=0x3C " */
		printk("RES=0x%02x ", (u_int8_t)(ntohl(tcp_flag_word(tcph) & TCP_RESERVED_BITS) >> 22));
		/* Max length: 32 "CWR ECE URG ACK PSH RST SYN FIN " */
		if (tcph->cwr)
			printk("CWR ");
		if (tcph->ece)
			printk("ECE ");
		if (tcph->urg)
			printk("URG ");
		if (tcph->ack)
			printk("ACK ");
		if (tcph->psh)
			printk("PSH ");
		if (tcph->rst)
			printk("RST ");
		if (tcph->syn)
			printk("SYN ");
		if (tcph->fin)
			printk("FIN ");
		/* Max length: 11 "URGP=65535 " */
		printk("URGP=%u ", ntohs(tcph->urg_ptr));

		if ((info->logflags & IP6T_LOG_TCPOPT)
		    && tcph->doff * 4 > sizeof(struct tcphdr)) {
			u_int8_t *op;
			unsigned int i;
			unsigned int optsize = tcph->doff * 4
					       - sizeof(struct tcphdr);

			if (skb->len - ptr - sizeof(struct tcphdr) < optsize) {
				printk("OPT (TRUNCATED)");
				return;
			}
			op = (u_int8_t *)(skb->data + ptr
					  + sizeof(struct tcphdr));

			/* Max length: 127 "OPT (" 15*4*2chars ") " */
			printk("OPT (");
			for (i =0; i < optsize; i++)
				printk("%02X", op[i]);
			printk(") ");
		}
		break;
	}
	case IPPROTO_UDP: {
		struct udphdr *udph;

		/* Max length: 10 "PROTO=UDP " */
		printk("PROTO=UDP ");

		if (fragment)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		if (skb->len - ptr < sizeof(*udph)) {
			printk("INCOMPLETE [%u bytes] ", skb->len - ptr);
			return;
		}
		udph = (struct udphdr *)(skb->data + ptr);

		/* Max length: 20 "SPT=65535 DPT=65535 " */
		printk("SPT=%u DPT=%u LEN=%u ",
		       ntohs(udph->source), ntohs(udph->dest),
		       ntohs(udph->len));
		break;
	}
	case IPPROTO_ICMPV6: {
		struct icmp6hdr *icmp6h;

		/* Max length: 13 "PROTO=ICMPv6 " */
		printk("PROTO=ICMPv6 ");

		if (fragment)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		if (skb->len - ptr < sizeof(*icmp6h)) {
			printk("INCOMPLETE [%u bytes] ", skb->len - ptr);
			return;
		}
		icmp6h = (struct icmp6hdr *)(skb->data + ptr);

		/* Max length: 18 "TYPE=255 CODE=255 " */
		printk("TYPE=%u CODE=%u ", icmp6h->icmp6_type, icmp6h->icmp6_code);

		switch (icmp6h->icmp6_type) {
		case ICMPV6_ECHO_REQUEST:
		case ICMPV6_ECHO_REPLY:
			/* Max length: 19 "ID=65535 SEQ=65535 " */
			printk("ID=%u SEQ=%u ",
				ntohs(icmp6h->icmp6_identifier),
				ntohs(icmp6h->icmp6_sequence));
			break;
		case ICMPV6_MGM_QUERY:
		case ICMPV6_MGM_REPORT:
		case ICMPV6_MGM_REDUCTION:
			break;

		case ICMPV6_PARAMPROB:
			/* Max length: 17 "POINTER=ffffffff " */
			printk("POINTER=%08x ", ntohl(icmp6h->icmp6_pointer));
			/* Fall through */
		case ICMPV6_DEST_UNREACH:
		case ICMPV6_PKT_TOOBIG:
		case ICMPV6_TIME_EXCEED:
			/* Max length: 3+maxlen */
			if (recurse) {
				printk("[");
				dump_packet(info, skb, ptr + sizeof(*icmp6h),
					    0);
				printk("] ");
			}

			/* Max length: 10 "MTU=65535 " */
			if (icmp6h->icmp6_type == ICMPV6_PKT_TOOBIG)
				printk("MTU=%u ", ntohl(icmp6h->icmp6_mtu));
		}
		break;
	}
	/* Max length: 10 "PROTO=255 " */
	default:
		printk("PROTO=%u ", currenthdr);
	}
}

static unsigned int
ip6t_log_target(struct sk_buff **pskb,
		unsigned int hooknum,
		const struct net_device *in,
		const struct net_device *out,
		const void *targinfo,
		void *userinfo)
{
	const struct ip6t_log_info *loginfo = targinfo;
	char level_string[4] = "< >";

	level_string[1] = '0' + (loginfo->level % 8);
	spin_lock_bh(&log_lock);
	printk(level_string);
	printk("%sIN=%s OUT=%s ",
		loginfo->prefix,
		in ? in->name : "",
		out ? out->name : "");
	if (in && !out) {
		unsigned int len;
		/* MAC logging for input chain only. */
		printk("MAC=");
		if ((*pskb)->dev && (len = (*pskb)->dev->hard_header_len) &&
		    (*pskb)->mac.raw != (*pskb)->nh.raw) {
			unsigned char *p = (*pskb)->mac.raw;
			int i;

			if ((*pskb)->dev->type == ARPHRD_SIT &&
			    (p -= ETH_HLEN) < (*pskb)->head)
				p = NULL;

			if (p != NULL) {
				for (i = 0; i < len; i++)
					printk("%02x%s", p[i],
					       i == len - 1 ? "" : ":");
			}
			printk(" ");

			if ((*pskb)->dev->type == ARPHRD_SIT) {
				struct iphdr *iph;
				
				iph = (struct iphdr *)(*pskb)->mac.raw;
				printk("TUNNEL=%u.%u.%u.%u->%u.%u.%u.%u ",
				       NIPQUAD(iph->saddr),
				       NIPQUAD(iph->daddr));
			}
		} else
			printk(" ");
	}

	dump_packet(loginfo, (*pskb), (u8*)(*pskb)->nh.ipv6h - (*pskb)->data,
		    1);
	printk("\n");
	spin_unlock_bh(&log_lock);

	return IP6T_CONTINUE;
}

static int ip6t_log_checkentry(const char *tablename,
			       const struct ip6t_entry *e,
			       void *targinfo,
			       unsigned int targinfosize,
			       unsigned int hook_mask)
{
	const struct ip6t_log_info *loginfo = targinfo;

	if (targinfosize != IP6T_ALIGN(sizeof(struct ip6t_log_info))) {
		DEBUGP("LOG: targinfosize %u != %u\n",
		       targinfosize, IP6T_ALIGN(sizeof(struct ip6t_log_info)));
		return 0;
	}

	if (loginfo->level >= 8) {
		DEBUGP("LOG: level %u >= 8\n", loginfo->level);
		return 0;
	}

	if (loginfo->prefix[sizeof(loginfo->prefix)-1] != '\0') {
		DEBUGP("LOG: prefix term %i\n",
		       loginfo->prefix[sizeof(loginfo->prefix)-1]);
		return 0;
	}

	return 1;
}

static struct ip6t_target ip6t_log_reg
= { { NULL, NULL }, "LOG", ip6t_log_target, ip6t_log_checkentry, NULL, 
    THIS_MODULE };

static int __init init(void)
{
	if (ip6t_register_target(&ip6t_log_reg))
		return -EINVAL;

	return 0;
}

static void __exit fini(void)
{
	ip6t_unregister_target(&ip6t_log_reg);
}

module_init(init);
module_exit(fini);
