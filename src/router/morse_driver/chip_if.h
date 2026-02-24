/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _MORSE_CHIP_IF_H_
#define _MORSE_CHIP_IF_H_

#include "pageset.h"
#include "pager_if_hw.h"
#include "pager_if_sw.h"
#include "yaps.h"
#include "yaps-hw.h"

/** Chip IF interrupt mask. We may use any interrupts in this range */
#define MORSE_CHIP_IF_IRQ_MASK_ALL	(GENMASK(13, 0) | \
			MORSE_PAGER_IRQ_BYPASS_TX_STATUS_AVAILABLE | \
			MORSE_PAGER_IRQ_BYPASS_CMD_RESP_AVAILABLE)

enum morse_chip_if_flags {
	MORSE_CHIP_IF_FLAGS_DIR_TO_HOST = BIT(0),
	MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP = BIT(1),
	MORSE_CHIP_IF_FLAGS_COMMAND = BIT(2),
	/* Note: There is no support for beacon-specific pagesets yet */
	MORSE_CHIP_IF_FLAGS_BEACON = BIT(3),
	MORSE_CHIP_IF_FLAGS_DATA = BIT(4)
};

enum morse_chip_if {
	MORSE_CHIP_IF_PAGESET,
	MORSE_CHIP_IF_YAPS
};

/** Event flags for talking to chip interface from skbq or pager */
enum morse_chip_if_event_flags {
	MORSE_RX_PEND,
	MORSE_PAGE_RETURN_PEND,
	MORSE_TX_COMMAND_PEND,
	MORSE_TX_BEACON_PEND,
	MORSE_TX_MGMT_PEND,
	MORSE_TX_DATA_PEND,
	MORSE_TX_PACKET_FREED_UP_PEND,
	MORSE_DATA_TRAFFIC_PAUSE_PEND,
	MORSE_DATA_TRAFFIC_RESUME_PEND,
	MORSE_UPDATE_HW_CLOCK_REFERENCE,
};

struct chip_if_ops {
	/**
	 * Initialises the chip interface
	 *
	 * @mors: Morse chip struct
	 *
	 * @return: Error code
	 */
	int (*init)(struct morse *mors);

	/**
	 * Initialises the chip interface after hardware restart
	 *
	 * @mors: Morse chip struct
	 *
	 * @return: Error code
	 */
	int (*hw_restarted)(struct morse *mors);

	/**
	 * Flush all tx data queues.
	 *
	 * @mors: Morse chip struct
	 */
	void (*flush_tx_data)(struct morse *mors);

	/**
	 * Flush all cmd queues.
	 *
	 * @mors: Morse chip struct
	 */
	void (*flush_cmds)(struct morse *mors);

	/**
	 * Cleans up chip interface.
	 *
	 * @mors: Morse chip struct
	 */
	void (*finish)(struct morse *mors);

	/**
	 * Gets a pointer to the tx queue array and the number of items in the array.
	 * @mors: Morse object
	 * @qs: Pointer to store pointer to queue array
	 * @num_qs: Pointer to store count of items in queue array
	 */
	void (*skbq_get_tx_qs)(struct morse *mors, struct morse_skbq **qs, int *num_qs);

	/**
	 * Gets the command skbq
	 * @mors: Morse object
	 *
	 * @returns command skbq
	 */
	struct morse_skbq *(*skbq_cmd_tc_q)(struct morse *mors);

	/**
	 * Gets the beacon skbq
	 * @mors: Morse object
	 *
	 * @returns beacon skbq
	 */
	struct morse_skbq *(*skbq_bcn_tc_q)(struct morse *mors);

	/**
	 * Gets the mgmt skbq
	 * @mors: Morse object
	 *
	 * @returns mgmt skbq
	 */
	struct morse_skbq *(*skbq_mgmt_tc_q)(struct morse *mors);

	/**
	 * Gets the tx skbq associated with given aci
	 * @mors: Morse object
	 * @aci: access category
	 *
	 * @returns skbq for skbs of given aci
	 */
	struct morse_skbq *(*skbq_tc_q_from_aci)(struct morse *mors, int aci);

	/**
	 * A callback that is called when a hostsync interrupt is raised
	 * @mors: Morse object
	 * @status: Hostsync interrupt register (bitmask of hostsync IRQs)
	 *
	 * @returns 0 if successful, otherwise error code
	 */
	int (*chip_if_handle_irq)(struct morse *mors, u32 status);

	/**
	 * Counts the total number of TX SKBs across all the queue types
	 * contained in the chip interface object. It includes the SKBs yet to
	 * be sent, and the SKBs that are awaiting a status return from the chip.
	 * @mors: Morse object
	 *
	 * @returns count
	 */
	int (*skbq_get_tx_buffered_count)(struct morse *mors);

	/**
	 * Counts the total number of TX SKBs that are pending a status return
	 * from the chip across all the queue types contained in the chip interface
	 * object.
	 * @mors: Morse object
	 *
	 * @returns count
	 */
	int (*skbq_get_tx_status_pending_count)(struct morse *mors);
};

struct morse_chip_if_state {
	enum morse_chip_if active_chip_if;
	union {
		struct {
			int pager_count;
			struct morse_pager *pagers;
			int pageset_count;
			struct morse_pageset *pagesets;
			struct morse_pageset *to_chip_pageset;
			struct morse_pageset *from_chip_pageset;
			struct {
				struct {
					u32 location;
					DECLARE_KFIFO(to_process, u32,
						      MORSE_PAGER_BYPASS_TX_STATUS_FIFO_DEPTH);
				} tx_sts;
				struct {
					u32 location;
					DECLARE_KFIFO(to_process, u32,
						      MORSE_PAGER_BYPASS_CMD_RESP_FIFO_DEPTH);
				} cmd_resp;
			} bypass;
			struct morse_pager_pkt_memory pkt_memory;
		};
		struct {
			struct morse_yaps *yaps;
		};
	};
	/* See enum morse_chip_if_event_flags for values */
	unsigned long event_flags;
	bool validate_skb_checksum;
};

struct morse_chip_if_host_table {
	union {
		struct {
			u32 rb_count;
			struct morse_pager_sw_table rb_table[];
		} __packed;
		struct {
			u32 pager_count;
			struct morse_pager_hw_entry pager_table[];
		} __packed;
	};
} __packed;

#endif /* !_MORSE_CHIP_IF_H_ */
