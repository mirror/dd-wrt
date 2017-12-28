#ifndef __LINUX_GRE_H
#define __LINUX_GRE_H

#include <linux/skbuff.h>

#define GREPROTO_CISCO		0
#define GREPROTO_PPTP		1
#define GREPROTO_MAX		2

/* handle protocols with non-standard GRE header by ids that do not overlap
 * with possible standard GRE protocol versions (0x00 - 0x7f)
 */
#define GREPROTO_NONSTD_BASE		0x80
#define GREPROTO_NONSTD_EOIP		(0 + GREPROTO_NONSTD_BASE)
#define GREPROTO_NONSTD_MAX		(1 + GREPROTO_NONSTD_BASE)


struct gre_protocol {
	int  (*handler)(struct sk_buff *skb);
	void (*err_handler)(struct sk_buff *skb, u32 info);
};

int gre_add_protocol(const struct gre_protocol *proto, u8 version);
int gre_del_protocol(const struct gre_protocol *proto, u8 version);

#endif
