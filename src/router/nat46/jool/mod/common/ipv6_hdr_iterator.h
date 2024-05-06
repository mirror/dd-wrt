#ifndef SRC_MOD_COMMON_HDR_ITERATOR_H_
#define SRC_MOD_COMMON_HDR_ITERATOR_H_

/**
 * @file
 * Routines and structures that help traverse the extension headers of IPv6 packets.
 *
 * Everything in this file assumes the main IPv6 header is glued in memory to the extension headers,
 * preceding them (such as in a linearized sk_buff). This assumption is fine in this project
 * because early validation ensures all headers can be pulled (see pskb_may_pull()).
 */

#include <linux/types.h>
#include <linux/ipv6.h>


/**
 * An object that helps you traverse the IPv6 headers of a packet.
 */
struct hdr_iterator {
	/** Type of the header we're currently visiting (previous header's nexthdr value). */
	__u8 hdr_type;
	/**
	 * Header we're currently visiting. Might also be the payload, if the iteration ended.
	 * You can know what's here by querying "hdr_type".
	 */
	void const *data;
};

/**
 * Use this to initialize your header iterator.
 *
 * @param main_hdr The IPv6 header whose subheaders you want to traverse.
 * @return a initialized "hdr_iterator".
 */
#define HDR_ITERATOR_INIT(main_hdr) { \
	.hdr_type = (main_hdr)->nexthdr, \
	.data = (main_hdr) + 1, \
}

/**
 * Use this to initialize your "iterator".
 *
 * @param iterator The struct you want to initialize.
 * @param main_hdr The IPv6 header whose subheaders you want to traverse.
 */
void hdr_iterator_init(struct hdr_iterator *iterator,
		struct ipv6hdr const *main_hdr);
/**
 * Advances "iterator->data" one header and updates "iterator->hdr_type" accordingly. If "iterator"
 * has already reached the payload, nothing will happen.
 *
 * @param iterator iterator you want to move to the next header.
 * @return 0 if the iterator hit unrecognized data, EAGAIN if this function can be called again
 *		to reach another header.
 */
int hdr_iterator_next(struct hdr_iterator *iterator);
/**
 * Advances "iterator" to the end of the recognized header chain.
 *
 * @param iterator iterator you want to move to the end of its chain.
 */
void hdr_iterator_last(struct hdr_iterator *iterator);

/**
 * Internally uses an iterator to reach and return header "hdr_id" from the headers following
 * "ip6_hdr"'s.
 *
 * @param ip6_hdr fixed header from the packet you want the extension header from.
 * @param hdr_id header type you want.
 * @return header whose ID is "hdr_id" from "ip6_hdr"'s extension headers. Returns "NULL" if the
 *		header chain does not contain such a header.
 */
void const *hdr_iterator_find(struct ipv6hdr const *ip6_hdr, __u8 hdr_id);

#endif /* SRC_MOD_COMMON_HDR_ITERATOR_H_ */
