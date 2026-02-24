#ifndef _MORSE_SKBQ_H_
#define _MORSE_SKBQ_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/skbuff.h>
#include <linux/workqueue.h>

#include "skb_header.h"

/* Size of off-chip SKB queue */
#ifndef MORSE_SKBQ_SIZE
#define MORSE_SKBQ_SIZE			(4 * 128 * 1024)
#endif

struct morse;

struct morse_skbq {
	u32 pkt_seq;		/* SKB sequence used in tx_status */
	u16 flags;
	u32 skbq_size;		/* current off loaded size */
	spinlock_t lock;
	struct morse *mors;	/* mainly for debugging */
	struct sk_buff_head skbq;
	struct sk_buff_head pending;	/* packets sent pending feedback */
	struct work_struct dispatch_work;
};

/**
 * morse_skbq_purge() - Remove and free all entries in sk_buff_head queue.
 *
 * @mq The Morse SKBQ object. If non-null, the MQ will be locked prior
 *     to the purge.
 * @skbq The queue to purge.
 *
 * Return: Number of SKBs purged from the queue
 */
int morse_skbq_purge(struct morse_skbq *mq, struct sk_buff_head *skbq);
void morse_skbq_purge_aged(struct morse *mors, struct morse_skbq *mq);
u32 morse_skbq_space(struct morse_skbq *mq);
u32 morse_skbq_size(struct morse_skbq *mq);
int morse_skbq_deq_num_items(struct morse_skbq *mq, struct sk_buff_head *skbq, int num_items);
struct sk_buff *morse_skbq_alloc_skb(struct morse_skbq *mq, unsigned int length);

/**
 * morse_skbq_skb_tx() - Enqueue a skb to be passed to the chip on the given channel.
 * @mq: The Morse SKBQ.
 * @skb: The skb to enqueue.
 * @tx_info: Transmission parameters (copied into the skb header).
 * @channel: The skb channel. Indicates to the chip how to treat this packet.
 *
 * The skb is always consumed by this function, even if an error occurs.
 *
 * Return: error code on failure.
 */
int morse_skbq_skb_tx(struct morse_skbq *mq, struct sk_buff **skb,
		      struct morse_skb_tx_info *tx_info, u8 channel);

int morse_skbq_put(struct morse_skbq *mq, struct sk_buff *skb);
int morse_skbq_enq(struct morse_skbq *mq, struct sk_buff_head *skbq);
int morse_skbq_enq_prepend(struct morse_skbq *mq, struct sk_buff_head *skbq);
int morse_skbq_tx_complete(struct morse_skbq *mq, struct sk_buff_head *skbq);
struct sk_buff *morse_skbq_tx_pending(struct morse_skbq *mq);
void morse_skbq_show(const struct morse_skbq *mq, struct seq_file *file);
void morse_skbq_init(struct morse *mors, struct morse_skbq *mq, u16 flags);
void morse_skbq_finish(struct morse_skbq *mq);
void morse_skb_remove_hdr_after_sent_to_chip(struct sk_buff *skb);

void morse_skbq_mon_dump(struct morse *mors, struct seq_file *file);

/**
 * @brief Set the max SKB TX queue length.
 *
 * @new_max_txq_len Modified max skb TX queue length.
 */
void morse_set_max_skb_txq_len(int new_max_txq_len);

/**
 * @brief Unlink a given SKB from mq->pending, and perform Q specific
 *        'finish' processing on the SKB.
 *
 * @note The MQ lock (mq->lock) must be held by the caller.
 *
 * @param mq     The MQ from which the SKB came. Assumption is that SKB exists
 *               within mq->pending.
 * @param skb    The SKB to perform the finish processing on.
 * @param tx_sts The TX status returned from the chip to indicate how the SKB
 *               was sent.
 *
 * @return error code (0) on success else non-zero
 */
int morse_skbq_skb_finish(struct morse_skbq *mq, struct sk_buff *skb,
			  struct morse_skb_tx_status *tx_sts);

/**
 * @brief Flush pending and in-flight tx SKBs from the queue.
 *
 * @param mq SKB queue
 *
 * @return number of elements flushed from the queue
 */
int morse_skbq_tx_flush(struct morse_skbq *mq);

/**
 * @brief For each pending SKB in the given SKBQ, check if its
 *        tx_status_lifetime has been reached. If so, remove it from the
 *        pending queue and free appropriately.
 *
 * @param mors  Morse context
 * @param mq    SKB queue
 *
 * @return number of pending tx statuses that got removed
 */
int morse_skbq_check_for_stale_tx(struct morse *mors, struct morse_skbq *mq);

/**
 * @brief Stop the mac80211 TX data Qs.
 *
 * @param mors Morse context
 */
void morse_skbq_stop_tx_queues(struct morse *mors);

/**
 * @brief Wake the mac80211 TX data Qs.
 *
 * @param mors
 */
void morse_skbq_may_wake_tx_queues(struct morse *mors);

/**
 * @brief Return the number of SKBs that are buffered
 *        and ready to be TXd. For MQs that are 'halted',
 *        this function will return 0.
 *
 * @param mq SKB queue.
 *
 * @return number of tx ready SKBs
 */
u32 morse_skbq_count_tx_ready(struct morse_skbq *mq);

/**
 * @brief Return the number of SKBs that are buffered.
 *
 * @param mq SKB queue.
 *
 * @return number of buffered SKBs
 */
u32 morse_skbq_count(struct morse_skbq *mq);

/**
 * morse_skbq_pending_count - Checks number of SKBs pending in the skb queue.
 *
 * @mq: SKB queue.
 *
 * Return: Number of skbs pending (awaiting tx status) in the queue.
 */
u32 morse_skbq_pending_count(struct morse_skbq *mq);

/**
 * @brief Pause the DATA Qs. This can only be called from the same context
 *        that could wake the Qs (i.e. ChipWQ).
 *
 * @param mors Morse context
 */
void morse_skbq_data_traffic_pause(struct morse *mors);

/**
 * @brief Resume/Un-pause the DATA Qs. This can only be called from the
 *        same context that could pause the Qs (i.e. ChipWQ).
 *
 * @param mors Morse context
 */
void morse_skbq_data_traffic_resume(struct morse *mors);

/**
 * @brief Verify checksum for the SKB to catch SDIO bus read errors.
 *
 * @param data The page containing the skb to verify the checksum
 *
 * @return true if the check matches the fw calculated checksum
 */
bool morse_validate_skb_checksum(u8 *data);

#endif /* !_MORSE_SKBQ_H_ */
