#include "mod/common/ipv6_hdr_iterator.h"
#include <net/ipv6.h>

void hdr_iterator_init(struct hdr_iterator *iterator,
		struct ipv6hdr const *main_hdr)
{
	struct hdr_iterator defaults = HDR_ITERATOR_INIT(main_hdr);
	memcpy(iterator, &defaults, sizeof(defaults));
}

int hdr_iterator_next(struct hdr_iterator *iterator)
{
	union {
		struct ipv6_opt_hdr const *opt;
		struct frag_hdr const *frag;
	} hdr;

	switch (iterator->hdr_type) {
	case NEXTHDR_HOP:
	case NEXTHDR_ROUTING:
	case NEXTHDR_DEST:
		hdr.opt = iterator->data;
		iterator->hdr_type = hdr.opt->nexthdr;
		iterator->data += 8 + 8 * hdr.opt->hdrlen;
		break;

	case NEXTHDR_FRAGMENT:
		hdr.frag = iterator->data;
		iterator->hdr_type = hdr.frag->nexthdr;
		iterator->data += sizeof(*hdr.frag);
		break;

	default:
		return 0;
	}

	return EAGAIN; /* It's positive because it's not really an error ;p */
}

void hdr_iterator_last(struct hdr_iterator *iterator)
{
	while (hdr_iterator_next(iterator) == EAGAIN)
		/* Void on purpose. */;
}

void const *hdr_iterator_find(struct ipv6hdr const *ip6_hdr, __u8 hdr_id)
{
	struct hdr_iterator iterator = HDR_ITERATOR_INIT(ip6_hdr);

	do {
		if (iterator.hdr_type == hdr_id)
			return iterator.data;
	} while (hdr_iterator_next(&iterator) == EAGAIN);

	return NULL;
}
