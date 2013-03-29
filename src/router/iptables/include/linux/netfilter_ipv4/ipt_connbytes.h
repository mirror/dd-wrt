#ifndef _IPT_CONNBYTES_H
#define _IPT_CONNBYTES_H

enum xt_connbytes_what {
	XT_CONNBYTES_PKTS,
	XT_CONNBYTES_BYTES,
	XT_CONNBYTES_AVGPKT,
};

enum xt_connbytes_direction {
	XT_CONNBYTES_DIR_ORIGINAL,
	XT_CONNBYTES_DIR_REPLY,
	XT_CONNBYTES_DIR_BOTH,
};

struct xt_connbytes_info
{
	struct {
		aligned_u64 from;	/* count to be matched */
		aligned_u64 to;		/* count to be matched */
	} count;
	u_int8_t what;		/* ipt_connbytes_what */
	u_int8_t direction;	/* ipt_connbytes_direction */
};
#define ipt_connbytes_what xt_connbytes_what

#define IPT_CONNBYTES_PKTS	XT_CONNBYTES_PKTS
#define IPT_CONNBYTES_BYTES	XT_CONNBYTES_BYTES
#define IPT_CONNBYTES_AVGPKT	XT_CONNBYTES_AVGPKT

#define ipt_connbytes_direction 	xt_connbytes_direction
#define IPT_CONNBYTES_DIR_ORIGINAL 	XT_CONNBYTES_DIR_ORIGINAL
#define IPT_CONNBYTES_DIR_REPLY 	XT_CONNBYTES_DIR_REPLY
#define IPT_CONNBYTES_DIR_BOTH		XT_CONNBYTES_DIR_BOTH

#define ipt_connbytes_info xt_connbytes_info

#endif
