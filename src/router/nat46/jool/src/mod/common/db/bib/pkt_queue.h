#ifndef SRC_MOD_NAT64_BIB_PKT_QUEUE_H_
#define SRC_MOD_NAT64_BIB_PKT_QUEUE_H_

/**
 * @file
 * As the name implies, this is just a small database of packets. These packets
 * are meant to be replied (in the form of an ICMP error) in the future.
 *
 * You can find the specifications for this in pages 28 and 29 (look up
 * "simultaneous open of TCP connections"), and 30 (look up "stored is sent
 * back") from RFC 6146.
 *
 * The RFC gets outright nonsensical and insufficient here; an entire 5-page
 * section explaining the rationale of this cumbersome hack, a bunch of
 * references towards former RFCs and an appendix exemplifying expected packet
 * flow would not have felt out of place. There is a huge chunk of this that is
 * left unspecified (such as semantics regarding finding sessions lacking src6
 * and what kinds of packets should cancel ICMP errors and which shouldn't), so
 * this code is the result of a bunch of research, common sense and trial and
 * error.
 *
 * In any case, these requirements seem to exist to comply with REQ-4 of RFC
 * 5382 (http://ietf.10.n7.nabble.com/Simultaneous-connect-td222455.html),
 * except RFC 5382 wants us to cancel the ICMP error "If during this interval
 * the NAT receives and translates an outbound SYN for the connection", but this
 * is not very explicit in the specification of the V4_INIT state in RFC 6146.
 * I mean it's the only state where the session expiration triggers the ICMP
 * message, but it'd be nice to see confirmation that the stored packet can be
 * forgotten about.
 *
 * However, Marcelo Bagnulo's seemingly final comments really bend me over to
 * RFC 5382's behavior: "well, it may be sent inside an ICMP error message in
 * case the state times out and the V& SYN has not arrived."
 * (http://www.ietf.org/mail-archive/web/behave/current/msg08660.html)
 *
 * So yeah, "Packet Storage". This is how I understand it:
 *
 * If a NAT64 receives a IPv4-UDP or a IPv4-ICMP packet for which it has no
 * state, it should reply a ICMP error because it doesn't know which IPv6 node
 * the packet should be forwarded to.
 *
 * On the other hand, if a NAT64 receives a IPv4-TCP packet for which it has no
 * state, it should not immediately reply the ICMP error because the IPv4
 * endpoint could be attempting a "Simultaneous Open (SO) of TCP Connections"
 * (http://tools.ietf.org/html/rfc5128#section-3.4). What happens is the NAT64
 * stores the packet for 6 seconds; if an IPv6 packet counterpart arrives,
 * the NAT64 drops the original packet (the IPv4 node will eventually realize
 * this on its own by means of the handshake), otherwise a ICMP error containing
 * the original IPv4 packet is generated (because there's no SO going on).
 *
 * I believe that the point of this is to allow the endpoints to assemble a
 * connection whose BIB has a predictable pool4 address. (Because when choosing
 * the IPv6 packet's mask, Jool will be forced to use the IPv4 packet's
 * destination address and port.)
 *
 * So in summary, this is what needs to be done when a NAT64 receives a v4 SYN
 * (packet A):
 *
 * 1. Store packet A with session [src6,dst6,src4,dst4] = [a,b',c,b]
 *    (where b' is the pool6 prefix + b)
 *    Often, src6 is not available (because the BIB entry does not exist),
 *    so store [*,b',c,b] instead.
 * 2. If packet B (which is being translated into C) matches A's session***,
 *        Forget packet A.
 *        Continue from the V4 INIT state as normal.
 * 3. If packet B hasn't arrived after 6 seconds (the V4 INIT state times out),
 *        Wrap A in an ICMP error and send it to the source.
 *        Forget packet A.
 *
 * *** whether this means that B should match [*,b'] or C should match [c,b] is
 * anyone's guess. I'm going with the former since (given that c is random in
 * 6 to 4) it seems easier to set up from a hole puncher's perspective. Also,
 * the RFC wants us to store dst6, and there's no use for it otherwise.
 * It's weird because it means that we need a second lookup (almost always
 * guaranteed to fail) in the CLOSED state.
 *
 * Now, there are two types of stored packets:
 * 1. Packets stored because there was no BIB entry. ([*,b',c,b] packets)
 * 2. Packets stored because Address-Dependent Filtering doesn't trust them.
 *    ([a,b',c,b] packets)
 *
 * Both are to be either canceled by a suitable v6 packet or timed out and
 * encapsulated in an ICMP error.
 *
 * We're storing packets type 2 in the core BIB/session database, not here.
 * Why? Because they are associated with a valid session, which is associated
 * with a valid BIB entry. And those are supposed to be stored in the core
 * BIB/session database for the sake of the TCP state machine and stuff. Storing
 * a redundant session in this module just to free tabled_session objects from
 * the stored field is the kind of micro-optimization that is nothing but asking
 * for trouble.
 *
 * So why can't we store type 2 packets in the core module as well? Because
 * BIBless sessions cannot be stored on a two-layered tree whose first and main
 * layer is BIB entries.
 *
 * So even though the RFC wants us to think that the two types of packets are
 * one and the same thing, we need to treat them quite differently. Again, this
 * module is the section of the code that deals with type 1 packets.
 */

#include "common/config.h"
#include "mod/common/packet.h"
#include "mod/common/db/pool4/db.h"

struct pktqueue;

struct pktqueue_session {
	struct ipv6_transport_addr dst6;
	struct ipv4_transport_addr src4;
	struct ipv4_transport_addr dst4;

	struct sk_buff *skb;

	unsigned long update_time;
	/** Links this packet to the list. See @node_list. */
	struct list_head list_hook;
	/** Links this packet to the tree. See @node_tree. */
	struct rb_node tree_hook;
};

/**
 * Call during initialization for the remaining functions to work properly.
 */
struct pktqueue *pktqueue_alloc(void);
/**
 * Call during destruction to avoid memory leaks.
 */
void pktqueue_release(struct pktqueue *queue);

/**
 * Stores packet @pkt.
 */
int pktqueue_add(struct pktqueue *queue, struct packet *pkt,
		struct ipv6_transport_addr *dst6, bool too_many);
void pktqueue_rm(struct pktqueue *queue, struct ipv4_transport_addr *src4);

struct pktqueue_session *pktqueue_find(struct pktqueue *queue,
		struct ipv6_transport_addr *addr,
		struct mask_domain *masks);
void pktqueue_put_node(struct xlator *jool, struct pktqueue_session *node);

/**
 * In a perfect world, `prepare_clean` and `clean` would be a single function.
 * But we don't want to hold a lock while sending ICMP errors so this is not a
 * perfect world.
 *
 * First lock, then call `prepare_clean` (sending in an empty list), then
 * unlock, then call `clean`. (Using the same list, obviously.)
 * This keeps the concurrence-sensitive stuff inside the lock and the slow stuff
 * outside.
 */
unsigned int pktqueue_prepare_clean(struct pktqueue *queue,
		struct list_head *probes);
/**
 * Sends the ICMP errors contained in the @probe list.
 */
void pktqueue_clean(struct list_head *probes);


#endif /* SRC_MOD_NAT64_BIB_PKT_QUEUE_H_ */
