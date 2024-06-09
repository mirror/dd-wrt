#include <linux/gfp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>

struct dst_entry;

struct nf_conn_dst_cache {
	struct dst_entry *dst;
	int iif;
#if IS_ENABLED(CONFIG_IPV6)
	u32 cookie;
#endif

};

struct nf_conn_rtcache {
	struct nf_conn_dst_cache cached_dst[IP_CT_DIR_MAX];
};

static inline
struct nf_conn_rtcache *nf_ct_rtcache_find(const struct nf_conn *ct)
{
#if IS_ENABLED(CONFIG_NF_CONNTRACK_RTCACHE)
	return nf_ct_ext_find(ct, NF_CT_EXT_RTCACHE);
#else
	return NULL;
#endif
}

#if IS_ENABLED(CONFIG_NF_CONNTRACK_RTCACHE)
bool nf_conn_rtcache_match_dev(const struct sk_buff *skb,
			       const struct net_device *dev);
#else
static inline bool
nf_conn_rtcache_match_dev(const struct sk_buff *skb,
			  const struct net_device *dev)
{
	return false;
}
#endif

static inline int nf_conn_rtcache_iif_get(const struct nf_conn_rtcache *rtc,
					  enum ip_conntrack_dir dir)
{
	return rtc->cached_dst[dir].iif;
}
