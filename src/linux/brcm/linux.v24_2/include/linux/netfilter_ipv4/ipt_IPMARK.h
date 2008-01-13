#ifndef _IPT_IPMARK_H_target
#define _IPT_IPMARK_H_target

struct ipt_ipmark_target_info {
	unsigned long andmask;
	unsigned long ormask;
	unsigned int addr;
};

#define IPT_IPMARK_SRC    0
#define IPT_IPMARK_DST    1

#endif /*_IPT_IPMARK_H_target*/
