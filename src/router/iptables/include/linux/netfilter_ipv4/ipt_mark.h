#ifndef _IPT_MARK_H
#define _IPT_MARK_H

struct ipt_mark_info {
#ifdef KERNEL_64_USERSPACE_32
    unsigned long long mark, mask;
#else
    unsigned long mark, mask;
#endif
    u_int8_t invert;
};

struct xt_mark_mtinfo1 {
	u_int32_t mark, mask;
	u_int8_t invert;
};

#endif /*_IPT_MARK_H*/
