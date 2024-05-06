#include "usr/nl/stats.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

#define DEFINE_STAT(_id, _doc) \
	[_id] = { \
		.id = _id, \
		.name = #_id, \
		.doc = _doc, \
	}

#define TC "Translations cancelled: "

static struct joolnl_stat_metadata const jstat_metadatas[] = {
	DEFINE_STAT(JSTAT_RECEIVED6, "Total IPv6 packets received by the instance so far."),
	DEFINE_STAT(JSTAT_RECEIVED4, "Total IPv4 packets received by the instance so far."),
	DEFINE_STAT(JSTAT_SUCCESS, "Successful translations. (Note: 'Successful translation' does not imply that the packet was actually delivered.)"),
	DEFINE_STAT(JSTAT_BIB_ENTRIES, "Number of BIB entries currently held in the BIB."),
	DEFINE_STAT(JSTAT_SESSIONS, "Number of session entries currently held in the BIB."),
	DEFINE_STAT(JSTAT_ENOMEM, "Memory allocation failures."),
	DEFINE_STAT(JSTAT_XLATOR_DISABLED, TC "Translator was manually disabled."),
	DEFINE_STAT(JSTAT_POOL6_UNSET, TC "pool6 was unset."),
	DEFINE_STAT(JSTAT_SKB_SHARED, TC "Packet was shared. (In the kernel, when packets are 'shared', they cannot be modified.)"),
	DEFINE_STAT(JSTAT_L3HDR_OFFSET, TC "Packet corrupted; Network header offset is not relative to skb->data."),
	DEFINE_STAT(JSTAT_SKB_TRUNCATED, TC "Packet corrupted; Data stopped in the middle of a header."),
	DEFINE_STAT(JSTAT_HDR6, TC "Some IPv6 header field was bogus. (Eg. version was not 6.)"),
	DEFINE_STAT(JSTAT_HDR4, TC "Some IPv4 header field was bogus. (Eg. version was not 4.)"),
	DEFINE_STAT(JSTAT_UNKNOWN_L4_PROTO, TC "Packet carried an unknown transport protocol. (Untranslatable by NAT64.)"),
	DEFINE_STAT(JSTAT_UNKNOWN_ICMP6_TYPE, TC "ICMPv6 header's type and code values have no ICMPv4 counterpart."),
	DEFINE_STAT(JSTAT_UNKNOWN_ICMP4_TYPE, TC "ICMPv4 header's type and code values have no ICMPv6 counterpart."),
	DEFINE_STAT(JSTAT_DOUBLE_ICMP6_ERROR, TC "ICMPv6 error contained another ICMPv6 error. (Which is illegal.)"),
	DEFINE_STAT(JSTAT_DOUBLE_ICMP4_ERROR, TC "ICMPv4 error contained another ICMPv4 error. (Which is illegal.)"),
	DEFINE_STAT(JSTAT_UNKNOWN_PROTO_INNER, TC "ICMP error's inner packet had an unknown transport protocol. (Untranslatable by NAT64.)"),
	DEFINE_STAT(JSTAT_HAIRPIN_LOOP, TC "Incoming IPv6 packet's source address matches pool6. (Only the destination address should match pool6.)\n"
			"You have to think of the IPv4 network as an IPv6 network whose prefix is pool6. If your actual IPv6 client also has the pool6 prefix, then your setup risks IP address collision.\n"
			"Either change the client's address or fix your pool6 so it represents a unique network."),
	DEFINE_STAT(JSTAT_POOL6_MISMATCH, TC "IPv6 packet's destination address did not match pool6. (ie. Packet was not meant to be translated.)"),
	DEFINE_STAT(JSTAT_POOL4_MISMATCH, TC "IPv4 packet's destination address and transport protocol did not match pool4. (ie. Packet was not meant to be translated.)\n"
			"If the instance is a Netfilter translator, this counter increases randomly from normal operation, and is harmless.\n"
			"If the instance is an iptables translator, this counter being positive suggests a mismatch between the IPv4 iptables rule(s) and the instance's configuration."),
	DEFINE_STAT(JSTAT_ICMP6_FILTER, "Packets filtered by `drop-icmpv6-info` policy."),
	/* TODO (warning) This one might signal a programming error. */
	DEFINE_STAT(JSTAT_UNTRANSLATABLE_DST6, TC "IPv6 packet's destination address did not match pool6."),
	DEFINE_STAT(JSTAT_UNTRANSLATABLE_DST4, TC "IPv4 packet's source address could not be translated with the given pool6."),
	DEFINE_STAT(JSTAT_6056_F, TC "Unable to hash packet fields; cannot compute source port. (From my reading of the 4.15 kernel, this can only happen due to memory allocation failures, but YMMV.)"),
	DEFINE_STAT(JSTAT_MASK_DOMAIN_NOT_FOUND, TC "There was no pool4 entry whose protocol and mark matched the incoming IPv6 packet."),
	DEFINE_STAT(JSTAT_BIB6_NOT_FOUND, TC "IPv6 packet did not match a BIB entry from the database, and one could not be created."),
	DEFINE_STAT(JSTAT_BIB4_NOT_FOUND, TC "IPv4 packet did not match a BIB entry from the database."),
	DEFINE_STAT(JSTAT_SESSION_NOT_FOUND, TC "Packet was an ICMP error, but did not match a session entry from the database. (Which means that the original packet couldn't have been translated.)"),
	DEFINE_STAT(JSTAT_ADF, "Packets filtered by `address-dependent-filtering` policy."),
	DEFINE_STAT(JSTAT_V4_SYN, "Packets filtered by `drop-externally-initiated-tcp` policy."),
	DEFINE_STAT(JSTAT_SYN6_EXPECTED, TC "Incoming IPv6 packet was the first of a TCP connection, but its SYN flag was disabled."),
	DEFINE_STAT(JSTAT_SYN4_EXPECTED, TC "Incoming IPv4 packet was the first of a TCP connection, but its SYN flag was disabled."),
	DEFINE_STAT(JSTAT_TYPE1PKT, "Total number of Type 1 packets stored. (See https://github.com/NICMx/Jool/blob/584a846d09e891a0cd6342426b7a25c6478c90d6/src/mod/nat64/bib/pkt_queue.h#L77) (This counter is not decremented when a packet leaves the queue.)"),
	DEFINE_STAT(JSTAT_TYPE2PKT, "Total number of Type 2 packets stored. (See https://github.com/NICMx/Jool/blob/584a846d09e891a0cd6342426b7a25c6478c90d6/src/mod/nat64/bib/pkt_queue.h#L77) (This counter is not decremented when a packet leaves the queue.)"),
	DEFINE_STAT(JSTAT_SO_EXISTS, TC "Packet was a Simultaneous Open retry. (Client was trying to punch a hole, and was being unnecessarily greedy.)"),
	DEFINE_STAT(JSTAT_SO_FULL, TC "Packet queue was full, so the Simultaneous Open attempt was denied. (Too many clients were trying to punch holes.)"),
	DEFINE_STAT(JSTAT64_SRC, TC "IPv6 packet's source address did not match pool6 nor any EAMT entries, or the resulting address was denylist4ed."),
	DEFINE_STAT(JSTAT64_DST, TC "IPv6 packet's destination address did not match pool6 nor any EAMT entries, or the resulting address was denylist4ed."),
	DEFINE_STAT(JSTAT64_PSKB_COPY, TC "It was not possible to allocate the IPv4 counterpart of the IPv6 packet. (The kernel's pskb_copy() function failed.)"),
	DEFINE_STAT(JSTAT64_6791_ENOENT, TC "The rfc6791v4 prefix was needed, but it was unset and a suitable replacement could not be found. Cause is unknown."),
	DEFINE_STAT(JSTAT64_ICMP_CSUM, TC "Incoming ICMPv6 error packet's checksum was incorrect."),
	DEFINE_STAT(JSTAT64_UNTRANSLATABLE_PARAM_PROB_PTR, TC "Packet was an ICMv6 Parameter Problem error message, but its pointer was untranslatable."),
	DEFINE_STAT(JSTAT64_TTL, TC "IPv6 packet's Hop Limit field was 0 or 1."),
	DEFINE_STAT(JSTAT64_FRAGMENTED_ICMP, TC "IPv6 Packet was fragmented and ICMP, so its checksum was impossible to translate. (Unknown total packet length from IPv6 pseudoheader.)"),
	DEFINE_STAT(JSTAT64_2XFRAG, TC "IPv6 packet has two fragment headers."),
	DEFINE_STAT(JSTAT64_FRAG_THEN_EXT, TC "IPv6 packet has a Hop-by-Hop, Destination or Routing header after the Fragment header."),
	DEFINE_STAT(JSTAT64_SEGMENTS_LEFT, TC "IPv6 packet had a Segments Left field, and it was nonzero."),
	DEFINE_STAT(JSTAT46_SRC, TC "IPv4 packet's source address was denylist4ed, or did not match pool6 nor any EAMT entries."),
	DEFINE_STAT(JSTAT46_DST, TC "IPv4 packet's destination address was denylist4ed, or did not match pool6 nor any EAMT entries."),
	DEFINE_STAT(JSTAT46_PSKB_COPY, TC "It was not possible to allocate the IPv6 counterpart of the IPv4 packet. (The kernel's __pskb_copy() function failed.)"),
	DEFINE_STAT(JSTAT46_6791_ENOENT, TC "The rfc6791v6 prefix was needed, but it was unset and a suitable replacement could not be found. Cause is unknown."),
	DEFINE_STAT(JSTAT46_ICMP_CSUM, TC "Incoming ICMPv4 error packet's checksum was incorrect."),
	DEFINE_STAT(JSTAT46_UNTRANSLATABLE_PARAM_PROBLEM_PTR, TC "Packet was an ICMv4 Parameter Problem error message, but its pointer was untranslatable."),
	DEFINE_STAT(JSTAT46_TTL, TC "IPv4 packet's TTL field was 0 or 1."),
	DEFINE_STAT(JSTAT46_FRAGMENTED_ICMP, TC "IPv4 Packet was fragmented and ICMP, so its checksum was impossible to translate. (Unknown total packet length for IPv6 pseudoheader.)"),
	DEFINE_STAT(JSTAT46_SRC_ROUTE, TC "Packet had an unexpired Source Route. (Untranslatable.)"),
	DEFINE_STAT(JSTAT46_FRAGMENTED_ZERO_CSUM, TC "IPv4 packet's UDP checksum was zero. Jool dropped the packet because --amend-udp-checksum-zero was disabled, and/or the packet was fragmented.\n"
			"(In IPv4, the UDP checksum is optional, but in IPv6 it is not. Because stateless translators do not collect fragments, they cannot compute packet-wide checksums from scratch. Zero-checksum UDP fragments are thus untranslatable.)"),
	DEFINE_STAT(JSTAT46_BAD_MTU, TC "Translated packet was IPv6, but the interface through which it was routed had an illegal MTU. (< 1280)"),
	DEFINE_STAT(JSTAT_FAILED_ROUTES, TC "The translated packet could not be routed; the kernel's routing function errored. Cause is unknown. (It usually happens because the packet's destination address could not be found in the routing table.)"),
	DEFINE_STAT(JSTAT_PKT_TOO_BIG, TC "Translated IPv4 packet did not fit in the outgoing interface's MTU. A Packet Too Big or Fragmentation Needed ICMP error was returned to the client."),
	DEFINE_STAT(JSTAT_DST_OUTPUT, TC "Translation was successful but the kernel's packet dispatch function (dst_output()) returned nonzero."),
	DEFINE_STAT(JSTAT_ICMP6ERR_SUCCESS, "ICMPv6 errors (created by Jool, not translated) sent successfully."),
	DEFINE_STAT(JSTAT_ICMP6ERR_FAILURE, "ICMPv6 errors (created by Jool, not translated) that could not be sent."),
	DEFINE_STAT(JSTAT_ICMP4ERR_SUCCESS, "ICMPv4 errors (created by Jool, not translated) sent successfully."),
	DEFINE_STAT(JSTAT_ICMP4ERR_FAILURE, "ICMPv4 errors (created by Jool, not translated) that could not be sent."),
	DEFINE_STAT(JSTAT_ICMPEXT_BIG, "Illegal ICMP header length. (Exceeds available payload in packet.)"),
	DEFINE_STAT(JSTAT_UNKNOWN, TC "Programming error found. The module recovered, but the packet was dropped."),
	DEFINE_STAT(JSTAT_PADDING, "Dummy; ignore this one."),
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static struct jool_result validate_stats(void)
{
	unsigned int i;

	if (ARRAY_SIZE(jstat_metadatas) != JSTAT_COUNT)
		goto failure;

	for (i = 0; i < JSTAT_COUNT; i++) {
		if (i != jstat_metadatas[i].id)
			goto failure;
	}

	return result_success();

failure:
	return result_from_error(
		-EINVAL,
		"Programming error: The jstat_metadatas array does not match the jool_stat_id enum."
	);
}

struct query_args {
	joolnl_stats_foreach_cb cb;
	void *args;
	bool done;
	enum jool_stat_id last;
};

static struct jool_result stats_query_response(struct nl_msg *response,
		void *args)
{
	struct genlmsghdr *ghdr;
	struct nlattr *head, *attr;
	int len, rem;
	struct joolnl_stat stat;
	struct query_args *qargs = args;
	struct jool_result result;

	result = joolnl_init_foreach(response, &qargs->done);
	if (result.error)
		return result;

	ghdr = nlmsg_data(nlmsg_hdr(response));
	head = genlmsg_attrdata(ghdr, sizeof(struct joolnlhdr));
	len = genlmsg_attrlen(ghdr, sizeof(struct joolnlhdr));

	nla_for_each_attr(attr, head, len, rem) {
		qargs->last = nla_type(attr);
		if (qargs->last < 1 || qargs->last >= JSTAT_PADDING)
			goto bad_id;

		stat.meta = jstat_metadatas[qargs->last];
		stat.value = nla_get_u64(attr);
		result = qargs->cb(&stat, qargs->args);
		if (result.error)
			return result;
	}

	return result_success();

bad_id:
	return result_from_error(
		-EINVAL,
		"The kernel module returned an unknown stat counter."
	);
}

struct jool_result joolnl_stats_foreach(struct joolnl_socket *sk,
		char const *iname, joolnl_stats_foreach_cb cb, void *args)
{
	struct nl_msg *msg;
	struct query_args qargs;
	struct jool_result result;

	result = validate_stats();
	if (result.error)
		return result;

	qargs.cb = cb;
	qargs.args = args;
	qargs.done = true;
	qargs.last = 0;

	do {
		result = joolnl_alloc_msg(sk, iname, JNLOP_STATS_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (qargs.last && (nla_put_u8(msg, JNLAR_OFFSET_U8, qargs.last) < 0)) {
			nlmsg_free(msg);
			return joolnl_err_msgsize();
		}

		result = joolnl_request(sk, msg, stats_query_response, &qargs);
		if (result.error)
			return result;
	} while (!qargs.done);

	return result_success();
}
