#ifndef _IPT_FWMON_H
#define _IPT_FWMON_H

/* Bitmask macros */
#define MASK(x,y) (x & y)
#define MASK_SET(x,y) x |= y
#define MASK_UNSET(x,y) x &= ~y

#define USE_MARK	0x00000001
#define USE_DROP	0x00000002
#define USE_SIZE	0x00000004

struct ipt_nldata
{	
	unsigned int flags;
	unsigned int mark;
	unsigned int size;
};

/* Old header */
struct netlink_t {
	unsigned int len;
	unsigned int mark;
	char iface[IFNAMSIZ];
};

#endif /*_IPT_FWMON_H*/
