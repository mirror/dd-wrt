#ifndef SRC_MOD_COMMON_TYPES_H_
#define SRC_MOD_COMMON_TYPES_H_

/**
 * @file
 * Kernel-specific core data types and routines.
 */

#include "common/types.h"
#include <linux/netfilter.h>
#include <linux/kernel.h>
#include "common/xlat.h"
#include "mod/common/address.h"
#include "mod/common/error_pool.h"

/**
 * An indicator of what a function expects its caller to do with the packet
 * being translated.
 */
typedef enum verdict {
	/** "No problems thus far, processing of the packet can continue." */
	VERDICT_CONTINUE,
	/**
	 * "The packet should be dropped, no matter what."
	 * Typically, this is because it's corrupted.
	 *
	 * Code should rarely use this constant directly. Use drop() or
	 * drop_icmp() instead.
	 */
	VERDICT_DROP,
	/**
	 * This leads to NF_ACCEPT. Used when the packet cannot be translated,
	 * but maybe it was intended for the kernel.
	 *
	 * Code should rarely use this constant directly. Use untranlatable()
	 * or untranslatable_icmp() instead.
	 */
	VERDICT_UNTRANSLATABLE,
	/**
	 * "I need to keep the packet for a while. Do not free, access or modify
	 * it."
	 *
	 * The packet being stored is THE ORIGINAL PACKET.
	 * The "original packet" will be different from the "incoming packet" in
	 * hairpinning.
	 * Therefore, if your stealing/storing code doesn't include
	 * skb_original_skb(), then YOU HAVE A KERNEL PANIC.
	 *
	 * Code should rarely use this constant directly. Use stolen() instead.
	 */
	VERDICT_STOLEN,
} verdict;

/*
 * To test that you're not mixing up verdicts and int errors:
 *
 * 1. Comment the verdict typedef above out.
 * 2. Uncomment the one below.
 * 3. Compile.
 */
/*
typedef int *verdict;

static int vercontinue = 0;
static int verdrop = 1;
static int veruntranslatable = 2;
static int verstolen = 3;

const static verdict VERDICT_CONTINUE = &vercontinue;
const static verdict VERDICT_DROP = &verdrop;
const static verdict VERDICT_UNTRANSLATABLE = &veruntranslatable;
const static verdict VERDICT_STOLEN = &verstolen;
*/

/**
 * RFC 6146 tuple.
 *
 * A tuple is sort of a summary of a packet; it is a quick accesor for several
 * of its key elements.
 *
 * Keep in mind that the tuple's values do not always come from places you'd
 * normally expect. Unless you know ICMP errors are not involved, if the RFC
 * says "the tuple's source address", then you *MUST* extract the address from
 * the tuple, not from the packet. Conversely, if it says "the packet's source
 * address", then *DO NOT* extract it from the tuple for convenience. See
 * comments inside for more info.
 */
struct tuple {
	/**
	 * Most of the time, this is the packet's _source_ address and layer-4
	 * identifier. When the packet contains a inner packet, this is the
	 * inner packet's _destination_ address and l4 id.
	 */
	union transport_addr src;

	/**
	 * Most of the time, this is the packet's _destination_ address and
	 * layer-4 identifier. When the packet contains a inner packet, this is
	 * the inner packet's _source_ address and l4 id.
	 */
	union transport_addr dst;

	/**
	 * The packet's network protocol. This is the sure way to know which of
	 * the above union elements should be used.
	 */
	l3_protocol l3_proto;
	/**
	 * The packet's transport protocol that counts.
	 *
	 * Most of the time, this is the packet's simple l4-protocol. When the
	 * packet contains a inner packet, this is the inner packet's
	 * l4-protocol.
	 *
	 * This dictates whether this is a 5-tuple or a 3-tuple
	 * (see is_3_tuple()/is_5_tuple()).
	 */
	l4_protocol l4_proto;

/**
 * By the way: There's code that relies on src.addr<x>.l4 containing the same
 * value as dst.addr<x>.l4 when l4_proto == L4PROTO_ICMP (i. e. 3-tuples).
 */
#define icmp4_id src.addr4.l4
#define icmp6_id src.addr6.l4
};

/* IPv6 Tuple Printk Pattern */
#define T6PP TA6PP " -> " TA6PP " [%s]"
/* IPv6 Tuple Printk Arguments */
#define T6PA(t) TA6PA((t)->src.addr6), TA6PA((t)->dst.addr6), \
		l4proto_to_string((t)->l4_proto)

/* IPv4 Tuple Printk Pattern */
#define T4PP TA4PP " -> " TA4PP " [%s]"
/* IPv4 Tuple Printk Arguments */
#define T4PA(t) TA4PA((t)->src.addr4), TA4PA((t)->dst.addr4), \
		l4proto_to_string((t)->l4_proto)

/**
 * Returns true if @tuple represents a '3-tuple' (address-address-ICMP id), as
 * defined by RFC 6146.
 */
static inline bool is_3_tuple(struct tuple *tuple)
{
	return (tuple->l4_proto == L4PROTO_ICMP);
}

/**
 * Returns true if @tuple represents a '5-tuple'
 * (address-port-address-port-transport protocol), as defined by RFC 6146.
 */
static inline bool is_5_tuple(struct tuple *tuple)
{
	return !is_3_tuple(tuple);
}

/**
 * Prints @tuple pretty in the log.
 */
struct xlation;
void log_tuple(struct xlation *state, struct tuple *tuple);

/**
 * Returns true if @type (which is assumed to have been extracted from a ICMP
 * header) represents a packet involved in a ping.
 */
bool is_icmp6_info(__u8 type);
bool is_icmp4_info(__u8 type);

/**
 * Returns true if @type (which is assumed to have been extracted from a ICMP
 * header) represents a packet which is an error response.
 */
bool is_icmp6_error(__u8 type);
bool is_icmp4_error(__u8 type);

/* Moves all the elements from @src to the tail of @dst. */
static inline void list_move_all(struct list_head *src, struct list_head *dst)
{
	if (list_empty(src))
		return;

	dst->prev->next = src->next;
	src->next->prev = dst->prev;
	dst->prev = src->prev;
	dst->prev->next = dst;
	INIT_LIST_HEAD(src);
}

#endif /* SRC_MOD_COMMON_TYPES_H_ */
