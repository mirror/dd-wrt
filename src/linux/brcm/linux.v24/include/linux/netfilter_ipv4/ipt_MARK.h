#ifndef _IPT_MARK_H_target
#define _IPT_MARK_H_target

struct ipt_mark_target_info {
	unsigned long mark;
};

struct xt_mark_tginfo2 {
	__u32 mark, mask;
}; 

#endif /*_IPT_MARK_H_target*/
