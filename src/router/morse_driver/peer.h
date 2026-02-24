#ifndef _MORSE_PEER_H_
#define _MORSE_PEER_H_
/*
 * Copyright 2024 Morse Micro
 */
#include "morse.h"
#include "command.h"

/**
 * List item for information storing on pre-associated peers. Used by AP type interfaces
 * to track the received bandwidth of frames from peers yet to join the BSS.
 */
struct morse_pre_assoc_peer {
	struct list_head list;
	/** Timestamp to age out entries */
	unsigned long last_used;
	/** Peer MAC address (used for lookup) */
	u8 addr[ETH_ALEN];
	/** The S1G bandwidth of the last mgmt frame received from the peer */
	u8 last_rx_bw_mhz;
};

/**
 * morse_pre_assoc_peer_init_interface() - Initialise pre-associated peer list on mors object.
 *                                         This list is shared between VIFs.
 *
 * @mors: The morse object
 */
void morse_pre_assoc_peer_list_init(struct morse *mors);

/**
 * morse_pre_assoc_peer_list_take() - Signal that a VIF is now using the shared pre-associated
 *                                    peer list.
 *
 * @mors: The morse object
 */
void morse_pre_assoc_peer_list_vif_take(struct morse *mors);

/**
 * morse_pre_assoc_peer_list_take() - Signal that a VIF no longer requires the shared
 *                                    pre-associated peer list. If no other VIFs are using
 *                                    the list, all peer records will be flushed.
 *
 * @mors: The morse object
 */
void morse_pre_assoc_peer_list_vif_release(struct morse *mors);

/**
 * morse_pre_assoc_peer_delete() - Delete an pre-associated peer with matching address.
 *
 * @mors: The morse object
 * @addr: The addr of the peer to remove
 *
 * Return: 0 if removed, else < 0 on failure
 */
int morse_pre_assoc_peer_delete(struct morse *mors, const u8 *addr);

/**
 * morse_pre_assoc_peer_update_rx_info() - Update RX information for a pre-associated peer.
 *
 * @mors: The morse object
 * @addr: The addr of the peer to update
 * @rc: The rate control information of the received frame
 *
 * Return: 0 if updated, else < 0 on failure
 */
int morse_pre_assoc_peer_update_rx_info(struct morse *mors, const u8 *addr, morse_rate_code_t rc);

/**
 * morse_pre_assoc_peer_get_last_rx_bw_mhz() - Retrieve the receive bandwidth of the last
 *                                             frame from this pre-associated peer.
 *
 * @mors: The morse object
 * @addr: The addr of the peer to retrieve info for
 *
 * Return: > 0 if bw_mhz found, else < 0 on failure
 */
int morse_pre_assoc_peer_get_last_rx_bw_mhz(struct morse *mors, const u8 *addr);

#endif /* !_MORSE_PEER_H_ */
