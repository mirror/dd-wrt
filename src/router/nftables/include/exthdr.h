#ifndef NFTABLES_EXTHDR_H
#define NFTABLES_EXTHDR_H

#include <proto.h>
#include <tcpopt.h>
#include <ipopt.h>
#include <dccpopt.h>

enum exthdr_desc_id {
	EXTHDR_DESC_UNKNOWN	= 0,
	EXTHDR_DESC_HBH,
	EXTHDR_DESC_RT,
	EXTHDR_DESC_RT0,
	EXTHDR_DESC_RT2,
	EXTHDR_DESC_SRH,
	EXTHDR_DESC_FRAG,
	EXTHDR_DESC_DST,
	EXTHDR_DESC_MH,
	__EXTHDR_DESC_MAX
};
#define EXTHDR_DESC_MAX	(__EXTHDR_DESC_MAX - 1)

/**
 * struct exthdr_desc - extension header description
 *
 * @name:	extension header name
 * @type:	extension header protocol value
 * @templates:	header field templates
 */
struct exthdr_desc {
	const char			*name;
	enum exthdr_desc_id		id;
	uint8_t				type;
	struct proto_hdr_template	templates[10];
};

extern struct expr *exthdr_expr_alloc(const struct location *loc,
				      const struct exthdr_desc *desc,
				      uint8_t type);

extern const struct exthdr_desc *exthdr_find_proto(uint8_t proto);

extern void exthdr_init_raw(struct expr *expr, uint8_t type,
			    unsigned int offset, unsigned int len,
			    enum nft_exthdr_op op, uint32_t flags);

extern bool exthdr_find_template(struct expr *expr, const struct expr *mask,
				 unsigned int *shift);

enum hbh_hdr_fields {
	HBHHDR_INVALID,
	HBHHDR_NEXTHDR,
	HBHHDR_HDRLENGTH,
};

enum rt_hdr_fields {
	RTHDR_INVALID,
	RTHDR_NEXTHDR,
	RTHDR_HDRLENGTH,
	RTHDR_TYPE,
	RTHDR_SEG_LEFT,
};

enum rt0_hdr_fields {
	RT0HDR_INVALID,
	RT0HDR_RESERVED,
	RT0HDR_ADDR_1,
};

enum rt2_hdr_fields {
	RT2HDR_INVALID,
	RT2HDR_RESERVED,
	RT2HDR_ADDR,
};

enum rt4_hdr_fields {
	RT4HDR_INVALID,
	RT4HDR_LASTENT,
	RT4HDR_FLAGS,
	RT4HDR_TAG,
	RT4HDR_SID_1,
};

enum frag_hdr_fields {
	FRAGHDR_INVALID,
	FRAGHDR_NEXTHDR,
	FRAGHDR_RESERVED,
	FRAGHDR_FRAG_OFF,
	FRAGHDR_RESERVED2,
	FRAGHDR_MFRAGS,
	FRAGHDR_ID,
};

enum dst_hdr_fields {
	DSTHDR_INVALID,
	DSTHDR_NEXTHDR,
	DSTHDR_HDRLENGTH,
};

enum mh_hdr_fields {
	MHHDR_INVALID,
	MHHDR_NEXTHDR,
	MHHDR_HDRLENGTH,
	MHHDR_TYPE,
	MHHDR_RESERVED,
	MHHDR_CHECKSUM,
};

extern const struct expr_ops exthdr_expr_ops;
extern const struct exthdr_desc exthdr_hbh;
extern const struct exthdr_desc exthdr_rt;
extern const struct exthdr_desc exthdr_rt0;
extern const struct exthdr_desc exthdr_rt2;
extern const struct exthdr_desc exthdr_rt4;
extern const struct exthdr_desc exthdr_frag;
extern const struct exthdr_desc exthdr_dst;
extern const struct exthdr_desc exthdr_mh;
extern const struct datatype mh_type_type;

extern const struct stmt_ops exthdr_stmt_ops;

#endif /* NFTABLES_EXTHDR_H */
