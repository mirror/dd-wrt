#include "common/config.h"

#ifndef __KERNEL__
#include <errno.h>
#endif

struct nla_policy joolnl_struct_list_policy[JNLAL_COUNT] = {
	[JNLAL_ENTRY] = { .type = NLA_NESTED }
};
struct nla_policy joolnl_plateau_list_policy[JNLAL_COUNT] = {
	[JNLAL_ENTRY] = { .type = NLA_U16 }
};

struct nla_policy joolnl_instance_entry_policy[JNLAIE_COUNT] = {
	[JNLAIE_NS] = { .type = NLA_U32 },
	[JNLAIE_XF] = { .type = NLA_U8 },
	[JNLAIE_INAME] = {
#ifdef __KERNEL__
		.type = NLA_NUL_STRING,
		/* Must not include the null char (see struct nla_policy) */
		.len = INAME_MAX_SIZE - 1,
#else
		.type = NLA_STRING,
		/* Must include the null char (see validate_nla()) */
		.maxlen = INAME_MAX_SIZE,
#endif
	},
};

struct nla_policy joolnl_prefix6_policy[JNLAP_COUNT] = {
	[JNLAP_ADDR] = JOOLNL_ADDR6_POLICY,
	[JNLAP_LEN] = { .type = NLA_U8 },
};

struct nla_policy joolnl_prefix4_policy[JNLAP_COUNT] = {
	[JNLAP_ADDR] = JOOLNL_ADDR4_POLICY,
	[JNLAP_LEN] = { .type = NLA_U8 },
};

struct nla_policy joolnl_taddr6_policy[JNLAT_COUNT] = {
	[JNLAT_ADDR] = JOOLNL_ADDR6_POLICY,
	[JNLAT_PORT] = { .type = NLA_U16 },
};

struct nla_policy joolnl_taddr4_policy[JNLAT_COUNT] = {
	[JNLAT_ADDR] = JOOLNL_ADDR4_POLICY,
	[JNLAT_PORT] = { .type = NLA_U16 },
};

struct nla_policy eam_policy[JNLAE_COUNT] = {
	[JNLAE_PREFIX6] = { .type = NLA_NESTED },
	[JNLAE_PREFIX4] = { .type = NLA_NESTED },
};

struct nla_policy joolnl_pool4_entry_policy[JNLAP4_COUNT] = {
	[JNLAP4_MARK] = { .type = NLA_U32 },
	[JNLAP4_ITERATIONS] = { .type = NLA_U32 },
	[JNLAP4_FLAGS] = { .type = NLA_U8 },
	[JNLAP4_PROTO] = { .type = NLA_U8 },
	[JNLAP4_PREFIX] = { .type = NLA_NESTED },
	[JNLAP4_PORT_MIN] = { .type = NLA_U16 },
	[JNLAP4_PORT_MAX] = { .type = NLA_U16 },
};

struct nla_policy joolnl_bib_entry_policy[JNLAB_COUNT] = {
	[JNLAB_SRC6] = { .type = NLA_NESTED },
	[JNLAB_SRC4] = { .type = NLA_NESTED },
	[JNLAB_PROTO] = { .type = NLA_U8 },
	[JNLAB_STATIC] = { .type = NLA_U8 },
};

struct nla_policy joolnl_session_entry_policy[JNLASE_COUNT] = {
	[JNLASE_SRC6] = { .type = NLA_NESTED },
	[JNLASE_DST6] = { .type = NLA_NESTED },
	[JNLASE_SRC4] = { .type = NLA_NESTED },
	[JNLASE_DST4] = { .type = NLA_NESTED },
	[JNLASE_PROTO] = { .type = NLA_U8 },
	[JNLASE_STATE] = { .type = NLA_U8 },
	[JNLASE_TIMER] = { .type = NLA_U8 },
	[JNLASE_EXPIRATION] = { .type = NLA_U32 },
};

struct nla_policy siit_globals_policy[JNLAG_COUNT] = {
	[JNLAG_ENABLED] = { .type = NLA_U8 },
	[JNLAG_POOL6] = { .type = NLA_NESTED },
	[JNLAG_LOWEST_IPV6_MTU] = { .type = NLA_U32 },
	[JNLAG_DEBUG] = { .type = NLA_U8 },
	[JNLAG_RESET_TC] = { .type = NLA_U8 },
	[JNLAG_RESET_TOS] = { .type = NLA_U8 },
	[JNLAG_TOS] = { .type = NLA_U8 },
	[JNLAG_PLATEAUS] = { .type = NLA_NESTED },
	[JNLAG_COMPUTE_CSUM_ZERO] = { .type = NLA_U8 },
	[JNLAG_HAIRPIN_MODE] = { .type = NLA_U8 },
	[JNLAG_RANDOMIZE_ERROR_ADDR] = { .type = NLA_U8 },
	[JNLAG_POOL6791V6] = { .type = NLA_NESTED },
	[JNLAG_POOL6791V4] = { .type = NLA_NESTED },
};

struct nla_policy nat64_globals_policy[JNLAG_COUNT] = {
	[JNLAG_ENABLED] = { .type = NLA_U8 },
	[JNLAG_POOL6] = { .type = NLA_NESTED },
	[JNLAG_LOWEST_IPV6_MTU] = { .type = NLA_U32 },
	[JNLAG_DEBUG] = { .type = NLA_U8 },
	[JNLAG_RESET_TC] = { .type = NLA_U8 },
	[JNLAG_RESET_TOS] = { .type = NLA_U8 },
	[JNLAG_TOS] = { .type = NLA_U8 },
	[JNLAG_PLATEAUS] = { .type = NLA_NESTED },
	[JNLAG_DROP_ICMP6_INFO] = { .type = NLA_U8 },
	[JNLAG_SRC_ICMP6_BETTER] = { .type = NLA_U8 },
	[JNLAG_F_ARGS] = { .type = NLA_U8 },
	[JNLAG_HANDLE_RST] = { .type = NLA_U8 },
	[JNLAG_TTL_TCP_EST] = { .type = NLA_U32 },
	[JNLAG_TTL_TCP_TRANS] = { .type = NLA_U32 },
	[JNLAG_TTL_UDP] = { .type = NLA_U32 },
	[JNLAG_TTL_ICMP] = { .type = NLA_U32 },
	[JNLAG_BIB_LOGGING] = { .type = NLA_U8 },
	[JNLAG_SESSION_LOGGING] = { .type = NLA_U8 },
	[JNLAG_DROP_BY_ADDR] = { .type = NLA_U8 },
	[JNLAG_DROP_EXTERNAL_TCP] = { .type = NLA_U8 },
	[JNLAG_MAX_STORED_PKTS] = { .type = NLA_U32 },
	[JNLAG_JOOLD_ENABLED] = { .type = NLA_U8 },
	[JNLAG_JOOLD_FLUSH_ASAP] = { .type = NLA_U8 },
	[JNLAG_JOOLD_FLUSH_DEADLINE] = { .type = NLA_U32 },
	[JNLAG_JOOLD_CAPACITY] = { .type = NLA_U32 },
	[JNLAG_JOOLD_MAX_PAYLOAD] = { .type = NLA_U32 },
	[JNLAG_JOOLD_MAX_SESSIONS_PER_PACKET] = { .type = NLA_U32 },
};

int iname_validate(const char *iname, bool allow_null)
{
	unsigned int i;

	if (!iname)
		return allow_null ? 0 : -EINVAL;

	for (i = 0; i < INAME_MAX_SIZE; i++) {
		if (iname[i] == '\0')
			return 0;
		if (iname[i] < 32) /* "if not printable" */
			break;
	}

	return -EINVAL;
}

int xt_validate(xlator_type xt)
{
	return (xt == XT_SIIT || xt == XT_NAT64) ? 0 : -EINVAL;
}

int xf_validate(xlator_framework xf)
{
#ifdef XTABLES_DISABLED
	return (xf == XF_NETFILTER) ? 0 : -EINVAL;
#else
	return (xf == XF_NETFILTER || xf == XF_IPTABLES) ? 0 : -EINVAL;
#endif
}

xlator_type xlator_flags2xt(xlator_flags flags)
{
	return flags & 0x03;
}

xlator_framework xlator_flags2xf(xlator_flags flags)
{
	return flags & 0x0C;
}

char const *xt2str(xlator_type xt)
{
	switch (xt) {
	case XT_SIIT:
		return "SIIT";
	case XT_NAT64:
		return "NAT64";
	}

	return "Unknown";
}
