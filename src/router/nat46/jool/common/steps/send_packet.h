#ifndef SRC_MOD_COMMON_SEND_PACKET_H_
#define SRC_MOD_COMMON_SEND_PACKET_H_

/**
 * @file
 * Functions to artificially send homemade packets through the interfaces.
 * Basically, you initialize sk_buffs and this worries about putting them on the
 * network.
 *
 * We need this because the kernel assumes that when a packet enters a module,
 * a packet featuring the same layer-3 protocol exits the module. So we can't
 * just morph IPv4 packets into IPv6 ones and vice-versa; we need to ask the
 * kernel to drop the original packets and send new ones on our own.
 */

#include "mod/common/translation_state.h"

/**
 * Puts @state's outgoing skb on the network.
 *
 * Note that this function inherits from ip_local_out() and ip6_local_out() the
 * annoying side effect of freeing "out_skb", EVEN IF IT COULD NOT BE SENT.
 */
verdict sendpkt_send(struct xlation *state);

/**
 * joold's multicast packet sender. Sends the packet to the Netlink multicast
 * group, not the network.
 * Separated from joold for mocking reasons.
 */
void sendpkt_multicast(struct xlator *jool, struct sk_buff *skb);

#endif /* SRC_MOD_COMMON_SEND_PACKET_H_ */
