#ifndef NETLINK_NEXTHOP_ENCAP_H_
#define	NETLINK_NEXTHOP_ENCAP_H_

struct nh_encap_ops {
	uint16_t encap_type;

	int	(*build_msg)(struct nl_msg *msg, void *priv);
	int	(*parse_msg)(struct nlattr *nla, struct rtnl_nexthop *rtnh);

	int	(*compare)(void *a, void *b);

	void	(*dump)(void *priv, struct nl_dump_params *dp);
	void	(*destructor)(void *priv);
};

struct rtnl_nh_encap;

/*
 * generic nexthop encap
 */
void nh_set_encap(struct rtnl_nexthop *nh, struct rtnl_nh_encap *rtnh_encap);

int nh_encap_parse_msg(struct nlattr *encap, struct nlattr *encap_type,
		       struct rtnl_nexthop *rtnh);
int nh_encap_build_msg(struct nl_msg *msg, struct rtnl_nh_encap *rtnh_encap);

void nh_encap_dump(struct rtnl_nh_encap *rtnh_encap, struct nl_dump_params *dp);

int nh_encap_compare(struct rtnl_nh_encap *a, struct rtnl_nh_encap *b);

/*
 * MPLS encap
 */
extern struct nh_encap_ops mpls_encap_ops;
#endif
