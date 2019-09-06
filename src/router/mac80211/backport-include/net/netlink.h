#ifndef __BACKPORT_NET_NETLINK_H
#define __BACKPORT_NET_NETLINK_H
#include_next <net/netlink.h>
#include <linux/version.h>
#include <linux/in6.h>

#if LINUX_VERSION_IS_LESS(4,12,0)
#include <backport/magic.h>

static inline int nla_validate5(const struct nlattr *head,
				int len, int maxtype,
				const struct nla_policy *policy,
				struct netlink_ext_ack *extack)
{
	return nla_validate(head, len, maxtype, policy);
}
#define nla_validate4 nla_validate
#define nla_validate(...) \
	macro_dispatcher(nla_validate, __VA_ARGS__)(__VA_ARGS__)

static inline int nla_parse6(struct nlattr **tb, int maxtype,
			     const struct nlattr *head,
			     int len, const struct nla_policy *policy,
			     struct netlink_ext_ack *extack)
{
	return nla_parse(tb, maxtype, head, len, policy);
}
#define nla_parse5(...) nla_parse(__VA_ARGS__)
#define nla_parse(...) \
	macro_dispatcher(nla_parse, __VA_ARGS__)(__VA_ARGS__)

static inline int nlmsg_parse6(const struct nlmsghdr *nlh, int hdrlen,
			       struct nlattr *tb[], int maxtype,
			       const struct nla_policy *policy,
			       struct netlink_ext_ack *extack)
{
	return nlmsg_parse(nlh, hdrlen, tb, maxtype, policy);
}
#define nlmsg_parse5 nlmsg_parse
#define nlmsg_parse(...) \
	macro_dispatcher(nlmsg_parse, __VA_ARGS__)(__VA_ARGS__)

static inline int nlmsg_validate5(const struct nlmsghdr *nlh,
				  int hdrlen, int maxtype,
				  const struct nla_policy *policy,
				  struct netlink_ext_ack *extack)
{
	return nlmsg_validate(nlh, hdrlen, maxtype, policy);
}
#define nlmsg_validate4 nlmsg_validate
#define nlmsg_validate(...) \
	macro_dispatcher(nlmsg_validate, __VA_ARGS__)(__VA_ARGS__)

static inline int nla_parse_nested5(struct nlattr *tb[], int maxtype,
				    const struct nlattr *nla,
				    const struct nla_policy *policy,
				    struct netlink_ext_ack *extack)
{
	return nla_parse_nested(tb, maxtype, nla, policy);
}
#define nla_parse_nested4 nla_parse_nested
#define nla_parse_nested(...) \
	macro_dispatcher(nla_parse_nested, __VA_ARGS__)(__VA_ARGS__)

static inline int nla_validate_nested4(const struct nlattr *start, int maxtype,
				       const struct nla_policy *policy,
				       struct netlink_ext_ack *extack)
{
	return nla_validate_nested(start, maxtype, policy);
}
#define nla_validate_nested3 nla_validate_nested
#define nla_validate_nested(...) \
	macro_dispatcher(nla_validate_nested, __VA_ARGS__)(__VA_ARGS__)
#endif /* LINUX_VERSION_IS_LESS(4,12,0) */

#if LINUX_VERSION_IS_LESS(3,7,0)
/**
 * nla_put_s8 - Add a s8 netlink attribute to a socket buffer
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @value: numeric value
 */
#define nla_put_s8 LINUX_BACKPORT(nla_put_s8)
static inline int nla_put_s8(struct sk_buff *skb, int attrtype, s8 value)
{
	return nla_put(skb, attrtype, sizeof(s8), &value);
}

/**
 * nla_put_s16 - Add a s16 netlink attribute to a socket buffer
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @value: numeric value
 */
#define nla_put_s16 LINUX_BACKPORT(nla_put_s16)
static inline int nla_put_s16(struct sk_buff *skb, int attrtype, s16 value)
{
	return nla_put(skb, attrtype, sizeof(s16), &value);
}

/**
 * nla_put_s32 - Add a s32 netlink attribute to a socket buffer
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @value: numeric value
 */
#define nla_put_s32 LINUX_BACKPORT(nla_put_s32)
static inline int nla_put_s32(struct sk_buff *skb, int attrtype, s32 value)
{
	return nla_put(skb, attrtype, sizeof(s32), &value);
}

/**
 * nla_get_s32 - return payload of s32 attribute
 * @nla: s32 netlink attribute
 */
#define nla_get_s32 LINUX_BACKPORT(nla_get_s32)
static inline s32 nla_get_s32(const struct nlattr *nla)
{
	return *(s32 *) nla_data(nla);
}

/**
 * nla_get_s16 - return payload of s16 attribute
 * @nla: s16 netlink attribute
 */
#define nla_get_s16 LINUX_BACKPORT(nla_get_s16)
static inline s16 nla_get_s16(const struct nlattr *nla)
{
	return *(s16 *) nla_data(nla);
}

/**
 * nla_get_s8 - return payload of s8 attribute
 * @nla: s8 netlink attribute
 */
#define nla_get_s8 LINUX_BACKPORT(nla_get_s8)
static inline s8 nla_get_s8(const struct nlattr *nla)
{
	return *(s8 *) nla_data(nla);
}

/**
 * nla_get_s64 - return payload of s64 attribute
 * @nla: s64 netlink attribute
 */
#define nla_get_s64 LINUX_BACKPORT(nla_get_s64)
static inline s64 nla_get_s64(const struct nlattr *nla)
{
	s64 tmp;

	nla_memcpy(&tmp, nla, sizeof(tmp));

	return tmp;
}
#endif /* < 3.7.0 */

#if LINUX_VERSION_IS_LESS(3,5,0)
/*
 * This backports:
 * commit 569a8fc38367dfafd87454f27ac646c8e6b54bca
 * Author: David S. Miller <davem@davemloft.net>
 * Date:   Thu Mar 29 23:18:53 2012 -0400
 *
 *     netlink: Add nla_put_be{16,32,64}() helpers.
 */

#define nla_put_be16 LINUX_BACKPORT(nla_put_be16)
static inline int nla_put_be16(struct sk_buff *skb, int attrtype, __be16 value)
{
	return nla_put(skb, attrtype, sizeof(__be16), &value);
}

#define nla_put_be32 LINUX_BACKPORT(nla_put_be32)
static inline int nla_put_be32(struct sk_buff *skb, int attrtype, __be32 value)
{
	return nla_put(skb, attrtype, sizeof(__be32), &value);
}

#define nla_put_be64 LINUX_BACKPORT(nla_put_be64)
static inline int nla_put_be64(struct sk_buff *skb, int attrtype, __be64 value)
{
	return nla_put(skb, attrtype, sizeof(__be64), &value);
}
#endif /* < 3.5 */

#if LINUX_VERSION_IS_LESS(3,7,0)
#define NLA_S8 (NLA_BINARY + 1)
#define NLA_S16 (NLA_BINARY + 2)
#define NLA_S32 (NLA_BINARY + 3)
#define NLA_S64 (NLA_BINARY + 4)
#define __NLA_TYPE_MAX (NLA_BINARY + 5)

#undef NLA_TYPE_MAX
#define NLA_TYPE_MAX (__NLA_TYPE_MAX - 1)
#endif

#if LINUX_VERSION_IS_LESS(4,1,0)
#define nla_put_in_addr LINUX_BACKPORT(nla_put_in_addr)
static inline int nla_put_in_addr(struct sk_buff *skb, int attrtype,
				  __be32 addr)
{
	return nla_put_be32(skb, attrtype, addr);
}

#define nla_put_in6_addr LINUX_BACKPORT(nla_put_in6_addr)
static inline int nla_put_in6_addr(struct sk_buff *skb, int attrtype,
				   const struct in6_addr *addr)
{
	return nla_put(skb, attrtype, sizeof(*addr), addr);
}

static inline __be32 nla_get_in_addr(const struct nlattr *nla)
{
	return *(__be32 *) nla_data(nla);
}

static inline struct in6_addr nla_get_in6_addr(const struct nlattr *nla)
{
	struct in6_addr tmp;

	nla_memcpy(&tmp, nla, sizeof(tmp));
	return tmp;
}
#endif /* < 4.1 */

#if LINUX_VERSION_IS_LESS(4,4,0)
/**
 * nla_get_le32 - return payload of __le32 attribute
 * @nla: __le32 netlink attribute
 */
#define nla_get_le32 LINUX_BACKPORT(nla_get_le32)
static inline __le32 nla_get_le32(const struct nlattr *nla)
{
	return *(__le32 *) nla_data(nla);
}

/**
 * nla_get_le64 - return payload of __le64 attribute
 * @nla: __le64 netlink attribute
 */
#define nla_get_le64 LINUX_BACKPORT(nla_get_le64)
static inline __le64 nla_get_le64(const struct nlattr *nla)
{
	return *(__le64 *) nla_data(nla);
}
#endif /* < 4.4 */

#if LINUX_VERSION_IS_LESS(4,7,0)
/**
 * nla_need_padding_for_64bit - test 64-bit alignment of the next attribute
 * @skb: socket buffer the message is stored in
 *
 * Return true if padding is needed to align the next attribute (nla_data()) to
 * a 64-bit aligned area.
 */
#define nla_need_padding_for_64bit LINUX_BACKPORT(nla_need_padding_for_64bit)
static inline bool nla_need_padding_for_64bit(struct sk_buff *skb)
{
#ifndef CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS
       /* The nlattr header is 4 bytes in size, that's why we test
        * if the skb->data _is_ aligned.  A NOP attribute, plus
        * nlattr header for next attribute, will make nla_data()
        * 8-byte aligned.
        */
       if (IS_ALIGNED((unsigned long)skb_tail_pointer(skb), 8))
               return true;
#endif
       return false;
}
/**
 * nla_align_64bit - 64-bit align the nla_data() of next attribute
 * @skb: socket buffer the message is stored in
 * @padattr: attribute type for the padding
 *
 * Conditionally emit a padding netlink attribute in order to make
 * the next attribute we emit have a 64-bit aligned nla_data() area.
 * This will only be done in architectures which do not have
 * HAVE_EFFICIENT_UNALIGNED_ACCESS defined.
 *
 * Returns zero on success or a negative error code.
 */
#define nla_align_64bit LINUX_BACKPORT(nla_align_64bit)
static inline int nla_align_64bit(struct sk_buff *skb, int padattr)
{
       if (nla_need_padding_for_64bit(skb) &&
            !nla_reserve(skb, padattr, 0))
                return -EMSGSIZE;
       return 0;
}

/**
 * nla_total_size_64bit - total length of attribute including padding
 * @payload: length of payload
 */
#define nla_total_size_64bit LINUX_BACKPORT(nla_total_size_64bit)
static inline int nla_total_size_64bit(int payload)
{
       return NLA_ALIGN(nla_attr_size(payload))
#ifndef HAVE_EFFICIENT_UNALIGNED_ACCESS
               + NLA_ALIGN(nla_attr_size(0))
#endif
               ;
}
#define __nla_reserve_64bit LINUX_BACKPORT(__nla_reserve_64bit)
struct nlattr *__nla_reserve_64bit(struct sk_buff *skb, int attrtype,
				   int attrlen, int padattr);
#define nla_reserve_64bit LINUX_BACKPORT(nla_reserve_64bit)
struct nlattr *nla_reserve_64bit(struct sk_buff *skb, int attrtype,
				 int attrlen, int padattr);
#define __nla_put_64bit LINUX_BACKPORT(__nla_put_64bit)
void __nla_put_64bit(struct sk_buff *skb, int attrtype, int attrlen,
		     const void *data, int padattr);
#define nla_put_64bit LINUX_BACKPORT(nla_put_64bit)
int nla_put_64bit(struct sk_buff *skb, int attrtype, int attrlen,
		  const void *data, int padattr);
/**
 * nla_put_u64_64bit - Add a u64 netlink attribute to a skb and align it
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @value: numeric value
 * @padattr: attribute type for the padding
 */
#define nla_put_u64_64bit LINUX_BACKPORT(nla_put_u64_64bit)
static inline int nla_put_u64_64bit(struct sk_buff *skb, int attrtype,
                                    u64 value, int padattr)
{
        return nla_put_64bit(skb, attrtype, sizeof(u64), &value, padattr);
}

/**
 * nla_put_s64 - Add a s64 netlink attribute to a socket buffer and align it
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @value: numeric value
 * @padattr: attribute type for the padding
 */
#define nla_put_s64 LINUX_BACKPORT(nla_put_s64)
static inline int nla_put_s64(struct sk_buff *skb, int attrtype, s64 value,
			      int padattr)
{
	return nla_put_64bit(skb, attrtype, sizeof(s64), &value, padattr);
}
#endif /* < 4.7 */

#if LINUX_VERSION_IS_LESS(4,10,0)
/**
 * nla_memdup - duplicate attribute memory (kmemdup)
 * @src: netlink attribute to duplicate from
 * @gfp: GFP mask
 */
#define nla_memdump LINUX_BACKPORT(nla_memdup)
static inline void *nla_memdup(const struct nlattr *src, gfp_t gfp)
{
	return kmemdup(nla_data(src), nla_len(src), gfp);
}
#endif /* < 4.9 */

#endif /* __BACKPORT_NET_NETLINK_H */
