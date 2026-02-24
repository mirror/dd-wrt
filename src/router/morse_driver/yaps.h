/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _MORSE_YAPS_H_
#define _MORSE_YAPS_H_

#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include "chip_if.h"

#include "skbq.h"

/**
 * Number of TX queues used to store different priority packets
 *
 * Nominally, this should be equal to the number of QoS queues the chip supports
 *
 */
#define YAPS_TX_SKBQ_MAX			4

/* Enable to support benchmarking the interface */

#define MORSE_YAPS_SUPPORTS_BENCHMARK

extern const struct chip_if_ops morse_yaps_ops;

enum morse_yaps_to_chip_q {
	MORSE_YAPS_TX_Q = 0,
	MORSE_YAPS_CMD_Q,
	MORSE_YAPS_BEACON_Q,
	MORSE_YAPS_MGMT_Q,
	/* Keep this last */
	MORSE_YAPS_NUM_TC_Q
};

enum morse_yaps_from_chip_q {
	MORSE_YAPS_RX_Q = 4,
	MORSE_YAPS_CMD_RESP_Q,
	MORSE_YAPS_TX_STATUS_Q,
	MORSE_YAPS_AUX_Q,

	/* Keep this last */
	MORSE_YAPS_NUM_FC_Q
};

struct morse_yaps_pkt {
	/* For to-chip transfers the skb will be initialised by the caller.
	 * For from-chip transfers the skb will be initialised by the callee.
	 */
	struct sk_buff *skb;
	union {
		/* Which queue to send to for to chip packets */
		enum morse_yaps_to_chip_q tc_queue;
		/* Which queue a packet was received for from chip packets */
		enum morse_yaps_from_chip_q fc_queue;
	};
};

struct morse_yaps {
	struct morse *mors;
	struct morse_yaps_hw_aux_data *aux_data;

	const struct morse_yaps_ops *ops;
	struct morse_skbq data_tx_qs[YAPS_TX_SKBQ_MAX];
	struct morse_skbq beacon_q;
	struct morse_skbq mgmt_q;
	struct morse_skbq data_rx_q;
	struct morse_skbq cmd_q;
	struct morse_skbq cmd_resp_q;

#ifdef MORSE_YAPS_SUPPORTS_BENCHMARK
	atomic_t benchmark_cnt_fc;
	atomic_t benchmark_cnt_tc;
#endif

	struct {
		struct timer_list timer;
		unsigned long retry_expiry;
		bool is_full;
	} chip_queue_full;

	u8 flags;

	/**
	 * @finish: Chip interface is stopping, new work should not be enqueued.
	 */
	bool finish;
};

struct morse_yaps_ops {
	/**
	 * Writes a packet to the chip
	 *
	 * @yaps: Pointer to yaps instance
	 * @q: Allocation queue to pull pages from
	 * @pkts: Array of packets to be sent
	 * @num_pkts: Number of packets in pkts array
	 * @num_pkts_sent: Output number of packets actually sent
	 *
	 * @return: Error code
	 */
	int (*write_pkts)(struct morse_yaps *yaps, struct morse_yaps_pkt pkts[],
			  int num_pkts, int *num_pkts_sent);

	/**
	 * Reads a series of packets from the chip. May not completely empty
	 * the chip, the caller needs to check size_received and compare it to
	 * queued size to determine if all packets have been read.
	 *
	 * @yaps: Pointer to yaps instance
	 * @pkts: Array of pkts to receive into
	 * @num_pkts_max: Number of pkt in pkts array
	 * @num_pkts_received: Output number of packets read from device
	 *
	 * @return: Error code
	 *          Note: If pkt is not available -EAGAIN is returned
	 */
	int (*read_pkts)(struct morse_yaps *yaps, struct morse_yaps_pkt pkts[],
			 int num_pkts_max, int *num_pkts_received);

	/**
	 * Reads the yaps status registers and updates internal driver state.
	 * Should be called before read_pkts or write_pkts to find out how many
	 * packets are available or space available in buffers.
	 *
	 * @yaps: Pointer to yaps instance
	 *
	 * Return: zero on success, or a negative error code.
	 *
	 * If this function returns an error, the caller must not refer to
	 * the status registers, nor call read_pkts() or write_pkts().
	 */
	int (*update_status)(struct morse_yaps *yaps);

	/**
	 * Print debugging info to file
	 *
	 * @yaps: Pointer to yaps instance
	 * @file: file to print to
	 */
	void (*show)(struct morse_yaps *yaps, struct seq_file *file);
};

/**
 * Initialise the morse yaps instance.
 * Does not perform any initialisation of the underlying pager implementation,
 * it is expected you call the implementation specific init on *pager first.
 *
 * @mors: Morse chip instance
 * @yaps: Pointer to yaps struct to initialise
 * @flags: Yaps flags (see yaps.h)
 *
 * @return: Error code
 */
int morse_yaps_init(struct morse *mors, struct morse_yaps *yaps, u8 flags);

/**
 * Prints info about the yaps instance to a file.
 *
 * @yaps: Pointer to yaps struct to print
 * @file: Pointer to file to print to
 */
void morse_yaps_show(struct morse_yaps *yaps, struct seq_file *file);

/**
 * Cleans up memory used by yaps instance.
 *
 * @yaps: Pointer to yaps struct to delete.
 */
void morse_yaps_finish(struct morse_yaps *yaps);

/**
 * Work function executed to perform yaps operations
 *
 * @work: Pointer to work struct.
 */
void morse_yaps_work(struct work_struct *work);

/**
 * Work function to remove stale pending tx SKBs
 *
 * @work: Pointer to the work struct.
 */
void morse_yaps_stale_tx_work(struct work_struct *work);

/**
 * Return a count of all the TX SKBs awaiting a status return
 *
 * @mors: Morse chip instance
 *
 * @return int
 */
int morse_yaps_get_tx_status_pending_count(struct morse *mors);

/**
 * Return a count of all the TX SKBs buffered
 *
 * @mors: Morse chip instance
 *
 * @return int
 */
int morse_yaps_get_tx_buffered_count(struct morse *mors);

/**
 * Runs a benchmark and prints info about the yaps performance to a file.
 *
 * @yaps: Pointer to yaps struct
 * @file: Pointer to file to print to
 */
int morse_yaps_benchmark(struct morse *mors, struct seq_file *file);

#endif /* !_MORSE_YAPS_H_ */
