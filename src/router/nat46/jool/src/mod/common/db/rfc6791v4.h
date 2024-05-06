#ifndef SRC_MOD_SIIT_RFC6791V4_H_
#define SRC_MOD_SIIT_RFC6791V4_H_

/**
 * @file
 * This is RFC 6791's pool of addresses.
 *
 * "The recommended approach to source selection is to use a single (or
 * small pool of) public IPv4 address as the source address of the
 * translated ICMP message and leverage the ICMP extension [RFC5837] to
 * include the IPv6 address as an Interface IP Address Sub-Object."
 *
 * The ICMP extension thing has not been implemented yet.
 */

#include "mod/common/translation_state.h"

int rfc6791v4_find(struct xlation *state, struct in_addr *result);

#endif /* SRC_MOD_SIIT_RFC6791V4_H_ */
