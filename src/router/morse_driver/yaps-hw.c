/*
 * Copyright 2022-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "linux/crc7.h"

#include "yaps-hw.h"
#include "bus.h"
#include "debug.h"
#include "chip_if.h"
#include "utils.h"
#include "yaps.h"

#define YAPS_HW_WINDOW_SIZE_BYTES	32768
#define YAPS_MAX_PKT_SIZE_BYTES		16128
#define YAPS_DEFAULT_READ_SIZE_BYTES	512
#define YAPS_METADATA_PAGE_COUNT	1

/* SW-10556 WAR: Sometimes phandle page addresses get corrupted when the number of
 * pages avalailable is exactly the same as the number of pages required.
 */
#define YAPS_PHANDLE_CORRUPTION_WAR_EXTRA_PAGE	1

#define YAPS_PAGE_SIZE	256
#define SDIO_BLOCKSIZE	512

/* Calculate padding required for yaps transaction */
#define YAPS_CALC_PADDING(_bytes) ((_bytes) & 0x3 ? (4 - ((_bytes) & 0x3)) : 0)

/*
 * Yaps data stream delimiter is a 32 bit word with the following fields:
 *
 * pkt_size (14 bits) - Packet size not including delimiter or padding
 * pool_id  (3  bits) - Pool that pages should be allocated from.
 *                      Pool IDs defined in enum yaps_alloc_pool
 * padding  (2  bits) - Padding required to bring packet to word (4 byte) boundary
 * irq      (1  bit ) - Raise a PKT_IRQ on the YDS this is sent to
 * reserved (5  bits) - Reserved, must write as 0
 * crc      (7  bits) - YAPS CRC
 */

/* Packet size not including delimiter or padding */
#define YAPS_DELIM_GET_PKT_SIZE(_yaps_aux, _delim) \
	(((_delim) & 0x3FFF) - (_yaps_aux)->reserved_yaps_page_size)
#define YAPS_DELIM_SET_PKT_SIZE(_yaps_aux, _pkt_size) \
	(((_pkt_size) & 0x3FFF) + (_yaps_aux)->reserved_yaps_page_size)
#define YAPS_DELIM_GET_PHANDLE_SIZE(_delim) (((_delim) & 0x3FFF))

/* Pool that pages should be allocated from. Pool IDs defined in enum yaps_alloc_pool */
#define YAPS_DELIM_GET_POOL_ID(_delim)		(((_delim) >> 14) & 0x7)
#define YAPS_DELIM_SET_POOL_ID(_pool_id)	(((_pool_id) & 0x7) << 14)
/* Padding required to bring packet to word (4 byte) boundary */
#define YAPS_DELIM_GET_PADDING(_delim)		(((_delim) >> 17) & 0x3)
#define YAPS_DELIM_SET_PADDING(_padding)	(((_padding) & 0x3) << 17)
/* Raise a PKT_IRQ on the YDS this is sent to */
#define YAPS_DELIM_GET_IRQ(_delim)		(((_delim) >> 19) & 0x1)
#define YAPS_DELIM_SET_IRQ(_irq)		(((_irq) & 0x1) << 19)
/* Reserved, must write as 0 */
#define YAPS_DELIM_GET_RESERVED(_delim)		(((_delim) >> 20) & 0x1F)
#define YAPS_DELIM_SET_RESERVED(_reserved)	(((_reserved) & 0x1F) << 20)
/* YAPS CRC */
#define YAPS_DELIM_GET_CRC(_delim)		(((_delim) >> 25) & 0x7F)
#define YAPS_DELIM_SET_CRC(_crc)		(((_crc) & 0x7F) << 25)

#define MORSE_YAPS_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_YAPS, _m, _f, ##_a)
#define MORSE_YAPS_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_YAPS, _m, _f, ##_a)
#define MORSE_YAPS_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_YAPS, _m, _f, ##_a)
#define MORSE_YAPS_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_YAPS, _m, _f, ##_a)

/* This maps directly to the status window block in chip memory */
struct morse_yaps_status_registers {
	/* Allocation pools */
	u32 tc_tx_pool_num_pages;
	u32 tc_cmd_pool_num_pages;
	u32 tc_beacon_pool_num_pages;
	u32 tc_mgmt_pool_num_pages;
	u32 fc_rx_pool_num_pages;
	u32 fc_resp_pool_num_pages;
	u32 fc_tx_sts_pool_num_pages;
	u32 fc_aux_pool_num_pages;

	/* To chip/From chip queues for YDS/YSL */
	u32 tc_tx_num_pkts;
	u32 tc_cmd_num_pkts;
	u32 tc_beacon_num_pkts;
	u32 tc_mgmt_num_pkts;
	u32 fc_num_pkts;
	u32 fc_done_num_pkts;
	u32 fc_rx_bytes_in_queue;
	u32 tc_delim_crc_fail_detected;
	union {
		u32 fc_host_ysl_status; /* ECB */
		u32 scratch_0; /* ECA - unused */
	};
	union {
		u32 scratch_1; /* ECA. scratch_0 for ECB */
		u32 lock;
	};
	/* scratch 2/3 un-used for ECA and 1/2/3 un-used for ECB */
} __packed;

struct morse_yaps_hw_aux_data {
	unsigned long access_lock;

	u32 yds_addr;
	u32 ysl_addr;
	u32 status_regs_addr;

	/* Alloc pool sizes */
	u16 tc_tx_pool_size;
	u16 tc_cmd_pool_size;
	u8 tc_beacon_pool_size;
	u8 tc_mgmt_pool_size;
	u8 fc_rx_pool_size;
	u8 fc_resp_pool_size;
	u8 fc_tx_sts_pool_size;
	u8 fc_aux_pool_size;

	/* To chip/from chip queue sizes */
	u8 tc_tx_q_size;
	u8 tc_cmd_q_size;
	u8 tc_beacon_q_size;
	u8 tc_mgmt_q_size;
	u8 fc_q_size;
	u8 fc_done_q_size;

	u16 reserved_yaps_page_size;

	/* Buffers to/from chip to support large contiguous reads/writes
	 * Buffers must be aligned before use.
	 */
	char *to_chip_buffer;
	char *from_chip_buffer;

	/* Status registers for queues and aloc pools on chip
	 * This structure is filled directly by bus reads, so it is aligned to 8 bytes to support
	 * MORSE_SDIO_ALIGNMENT of 1, 2, 4 or 8. Stricter alignment requirements will trigger a
	 * warning in morse_yaps_hw_init().
	 */
	struct morse_yaps_status_registers status_regs __aligned(8);
};

static int yaps_hw_lock(struct morse_yaps *yaps)
{
	if (test_and_set_bit_lock(0, &yaps->aux_data->access_lock))
		return -1;
	return 0;
}

static void yaps_hw_unlock(struct morse_yaps *yaps)
{
	clear_bit_unlock(0, &yaps->aux_data->access_lock);
}

static void morse_yaps_fill_aux_data_from_hw_tbl(struct morse_yaps_hw_aux_data *aux_data,
						 struct morse_yaps_hw_table *tbl_ptr)
{
	aux_data->ysl_addr = __le32_to_cpu(tbl_ptr->ysl_addr);
	aux_data->yds_addr = __le32_to_cpu(tbl_ptr->yds_addr);
	aux_data->status_regs_addr = __le32_to_cpu(tbl_ptr->status_regs_addr);

	aux_data->tc_tx_pool_size = __le16_to_cpu(tbl_ptr->tc_tx_pool_size);
	aux_data->fc_rx_pool_size = __le16_to_cpu(tbl_ptr->fc_rx_pool_size);
	aux_data->tc_cmd_pool_size = tbl_ptr->tc_cmd_pool_size;
	aux_data->tc_beacon_pool_size = tbl_ptr->tc_beacon_pool_size;
	aux_data->tc_mgmt_pool_size = tbl_ptr->tc_mgmt_pool_size;
	aux_data->fc_resp_pool_size = tbl_ptr->fc_resp_pool_size;
	aux_data->fc_tx_sts_pool_size = tbl_ptr->fc_tx_sts_pool_size;
	aux_data->fc_aux_pool_size = tbl_ptr->fc_aux_pool_size;
	aux_data->tc_tx_q_size = tbl_ptr->tc_tx_q_size;
	aux_data->tc_cmd_q_size = tbl_ptr->tc_cmd_q_size;
	aux_data->tc_beacon_q_size = tbl_ptr->tc_beacon_q_size;
	aux_data->tc_mgmt_q_size = tbl_ptr->tc_mgmt_q_size;
	aux_data->fc_q_size = tbl_ptr->fc_q_size;
	aux_data->fc_done_q_size = tbl_ptr->fc_done_q_size;
	aux_data->reserved_yaps_page_size = le16_to_cpu(tbl_ptr->yaps_reserved_page_size);
}

static inline u8 morse_yaps_crc(u32 word)
{
	u8 crc = 0;
	int len = sizeof(word);

	/* Mask to look at only non-crc bits in both metadata word and delimiters */
	word &= 0x1ffffff;
	while (len--) {
		crc = crc7_be_byte(crc, (word >> 24) & 0xff);
		word <<= 8;
	}
	return crc >> 1;
}

static inline u32 morse_yaps_delimiter(struct morse_yaps *yaps,
				unsigned int size, u8 pool_id, bool irq)
{
	u32 delim = 0;

	delim |= YAPS_DELIM_SET_PKT_SIZE(yaps->aux_data, size);
	delim |= YAPS_DELIM_SET_PADDING(YAPS_CALC_PADDING(size));
	delim |= YAPS_DELIM_SET_POOL_ID(pool_id);
	delim |= YAPS_DELIM_SET_IRQ(irq);
	delim |= YAPS_DELIM_SET_CRC(morse_yaps_crc(delim));
	return delim;
}

static void morse_yaps_hw_enable_irqs(struct morse *mors, bool enable)
{
	morse_hw_irq_enable(mors, MORSE_INT_YAPS_FC_PKT_WAITING_IRQN, enable);
	morse_hw_irq_enable(mors, MORSE_INT_YAPS_FC_PACKET_FREED_UP_IRQN, enable);
}

void morse_yaps_hw_read_table(struct morse *mors, struct morse_yaps_hw_table *tbl_ptr)
{
	morse_yaps_fill_aux_data_from_hw_tbl(mors->chip_if->yaps->aux_data, tbl_ptr);
	/* Enable interrupts */
	morse_yaps_hw_enable_irqs(mors, true);
}

static unsigned int morse_yaps_pages_required(struct morse_yaps *yaps,
						unsigned int size_bytes)
{
	/* Always account for the first metadata page */
	return MORSE_INT_CEIL(size_bytes + yaps->aux_data->reserved_yaps_page_size,
				 YAPS_PAGE_SIZE) +
				 YAPS_METADATA_PAGE_COUNT +
				 YAPS_PHANDLE_CORRUPTION_WAR_EXTRA_PAGE;
}

/* Checks if a single pkt will fit in the chip using the pool/alloc holding
 * information from the last status register read.
 */
static bool morse_yaps_will_fit(struct morse_yaps *yaps, struct morse_yaps_pkt *pkt, bool update)
{
	bool will_fit = true;
	const int pages_required = morse_yaps_pages_required(yaps, pkt->skb->len);
	int *pool_pages_avail = NULL;
	int *pkts_in_queue = NULL;
	int queue_pkts_avail = 0;

	switch (pkt->tc_queue) {
	case MORSE_YAPS_TX_Q:
		pool_pages_avail = &yaps->aux_data->status_regs.tc_tx_pool_num_pages;
		pkts_in_queue = &yaps->aux_data->status_regs.tc_tx_num_pkts;
		queue_pkts_avail = yaps->aux_data->tc_tx_q_size - *pkts_in_queue;
		break;
	case MORSE_YAPS_CMD_Q:
		pool_pages_avail = &yaps->aux_data->status_regs.tc_cmd_pool_num_pages;
		pkts_in_queue = &yaps->aux_data->status_regs.tc_cmd_num_pkts;
		queue_pkts_avail = yaps->aux_data->tc_cmd_q_size - *pkts_in_queue;
		break;
	case MORSE_YAPS_BEACON_Q:
		pool_pages_avail = &yaps->aux_data->status_regs.tc_beacon_pool_num_pages;
		pkts_in_queue = &yaps->aux_data->status_regs.tc_beacon_num_pkts;
		queue_pkts_avail = yaps->aux_data->tc_beacon_q_size - *pkts_in_queue;
		break;
	case MORSE_YAPS_MGMT_Q:
		pool_pages_avail = &yaps->aux_data->status_regs.tc_mgmt_pool_num_pages;
		pkts_in_queue = &yaps->aux_data->status_regs.tc_mgmt_num_pkts;
		queue_pkts_avail = yaps->aux_data->tc_mgmt_q_size - *pkts_in_queue;
		break;
	default:
		MORSE_YAPS_ERR(yaps->mors, "yaps invalid tc queue\n");
	}

	MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, queue_pkts_avail < 0);

	if (pages_required > *pool_pages_avail)
		will_fit = false;

	if (queue_pkts_avail == 0)
		will_fit = false;

	if (will_fit && update) {
		*pool_pages_avail -= pages_required;
		*pkts_in_queue += 1;
	}

	return will_fit;
}

static int morse_yaps_hw_write_pkt_err_check(struct morse_yaps *yaps, struct morse_yaps_pkt *pkt)
{
	if (pkt->skb->len + yaps->aux_data->reserved_yaps_page_size > YAPS_MAX_PKT_SIZE_BYTES)
		return -EMSGSIZE;
	if (pkt->tc_queue > MORSE_YAPS_NUM_TC_Q)
		return -EINVAL;
	if (!morse_yaps_will_fit(yaps, pkt, true))
		return -EAGAIN;

	return 0;
}

static int morse_yaps_hw_write_pkts(struct morse_yaps *yaps,
				    struct morse_yaps_pkt pkts[], int num_pkts, int *num_pkts_sent)
{
	int ret = 0;
	int i;
	u32 delim = 0;
	char *to_chip_buffer_aligned = PTR_ALIGN(yaps->aux_data->to_chip_buffer,
						  yaps->mors->bus_ops->bulk_alignment);
	char *write_buf = to_chip_buffer_aligned;
	int tx_len;
	int batch_txn_len = 0;
	int pkts_pending = 0;
	bool delim_irq = false;

	ret = yaps_hw_lock(yaps);
	if (ret) {
		MORSE_YAPS_DBG(yaps->mors, "%s yaps lock failed %d\n", __func__, ret);
		return ret;
	}

	*num_pkts_sent = 0;

	/* Check packet conditions */
	ret = morse_yaps_hw_write_pkt_err_check(yaps, &pkts[0]);
	if (ret)
		goto exit;

	/* Batch packets into larger transactions. Send as many as we have space for. */
	for (i = 0; i < num_pkts; ++i) {
		/* packets are padded with skb_pad.
		 * Hence, the true size of the packet is not set in skb->len.
		 */
		u32 pkt_size = pkts[i].skb->len + YAPS_CALC_PADDING(pkts[i].skb->len);

		tx_len = pkt_size + sizeof(delim);

		/* Send when we have reached window size, don't split pkt over boundary */
		if ((batch_txn_len + tx_len) > YAPS_HW_WINDOW_SIZE_BYTES) {
			ret = morse_dm_write(yaps->mors, yaps->aux_data->yds_addr,
					     to_chip_buffer_aligned, batch_txn_len);

			batch_txn_len = 0;
			if (ret)
				goto exit;
			write_buf = to_chip_buffer_aligned;
			*num_pkts_sent += pkts_pending;
			pkts_pending = 0;
		}

		if ((i + 1) == num_pkts) {
			/* The last packet in the queue should have the IRQ set */
			delim_irq = true;
		} else {
			/*
			 * Since this is not the last packet, we can check for the next one:
			 * In case of errors in the next packet set the IRQ
			 */
			ret = morse_yaps_hw_write_pkt_err_check(yaps, &pkts[i + 1]);
			if (ret)
				delim_irq = true;
		}

		/* Build stream header */
		/* Always set IRQ for the last packet so the chip doesn't miss it */
		delim = morse_yaps_delimiter(yaps, pkt_size, pkts[i].tc_queue, delim_irq);
		*((__le32 *)write_buf) = cpu_to_le32(delim);
		memcpy(write_buf + sizeof(delim), pkts[i].skb->data, pkts[i].skb->len);

		write_buf += tx_len;
		batch_txn_len += tx_len;
		pkts_pending++;

		if (ret)
			goto exit;
	}

exit:
	if (batch_txn_len > 0) {
		ret = morse_dm_write(yaps->mors, yaps->aux_data->yds_addr,
				     to_chip_buffer_aligned, batch_txn_len);
		*num_pkts_sent += pkts_pending;
	}

	yaps_hw_unlock(yaps);
	return ret;
}

static bool morse_yaps_is_valid_delimiter(u32 delim)
{
	u8 calc_crc = morse_yaps_crc(delim);
	int pkt_size = YAPS_DELIM_GET_PHANDLE_SIZE(delim);
	int padding = YAPS_DELIM_GET_PADDING(delim);

	if (calc_crc != YAPS_DELIM_GET_CRC(delim))
		return false;

	if (pkt_size == 0)
		return false;

	if ((pkt_size + padding) > YAPS_MAX_PKT_SIZE_BYTES)
		return false;

	/* Pkt length + padding should not require more padding */
	if (YAPS_CALC_PADDING(pkt_size) != padding)
		return false;

	return true;
}

/* Returns the number of bytes waiting to be read from the chip, or a negative errno. */
static int morse_calc_bytes_remaining(struct morse_yaps *yaps)
{
	u32 bytes_in_queue = yaps->aux_data->status_regs.fc_rx_bytes_in_queue;
	u32 delim_overhead = yaps->aux_data->status_regs.fc_num_pkts * sizeof(u32);
	u32 reserved_bytes = yaps->aux_data->status_regs.fc_num_pkts *
			     yaps->aux_data->reserved_yaps_page_size;

	if (WARN_ON(bytes_in_queue > INT_MAX) ||
	    WARN_ON(delim_overhead > INT_MAX) ||
	    WARN_ON(reserved_bytes > INT_MAX))
		return -EIO;

	return (int)bytes_in_queue;
}

static int morse_yaps_hw_read_pkts(struct morse_yaps *yaps,
				   struct morse_yaps_pkt pkts[],
				   int num_pkts_max, int *num_pkts_received)
{
	int ret;
	int i = 0;
	char *from_chip_buffer_aligned = PTR_ALIGN(yaps->aux_data->from_chip_buffer,
						 yaps->mors->bus_ops->bulk_alignment);
	char *read_ptr = from_chip_buffer_aligned;
	int bytes_remaining = morse_calc_bytes_remaining(yaps);
	bool again = false;

	*num_pkts_received = 0;

	if (num_pkts_max == 0 || bytes_remaining == 0)
		return 0;
	if (bytes_remaining < 0)
		return bytes_remaining;

	if (bytes_remaining > YAPS_HW_WINDOW_SIZE_BYTES) {
		bytes_remaining = YAPS_HW_WINDOW_SIZE_BYTES;
		again = true;
	}

	/* This is more coarse-grained than it needs to be -
	 * once the data is read into a local buffer the lock can be released,
	 * however access to from_chip_buffer will need to be protected with
	 * its own lock
	 */
	ret = yaps_hw_lock(yaps);
	if (ret) {
		MORSE_YAPS_DBG(yaps->mors, "%s yaps lock failed %d\n", __func__, ret);
		return ret;
	}

	/* Read all available packets to the buffer */
	ret = morse_dm_read(yaps->mors, yaps->aux_data->ysl_addr,
			    from_chip_buffer_aligned, bytes_remaining);

	if (ret)
		goto exit;

	/* Split serialised packets from buffer */
	while (i < num_pkts_max && bytes_remaining > 0) {
		u32 delim;
		int total_len;
		int pkt_size;

		delim = le32_to_cpu(*((__le32 *)read_ptr));
		read_ptr += sizeof(delim);
		bytes_remaining -= sizeof(delim);

		/* End of stream */
		if (delim == 0x0)
			break;

		if (!morse_yaps_is_valid_delimiter(delim)) {
			/* This will start a hunt for a valid delimiter. Given the CRC is only 7 bit
			 * it's possible to find an invalid block with a valid delimiter, leading to
			 * desynchronisation.
			 */
			MORSE_YAPS_WARN(yaps->mors, "yaps invalid delim\n");
			break;
		}

		/* Total length in chip */
		pkt_size = YAPS_DELIM_GET_PKT_SIZE(yaps->aux_data, delim);
		total_len = pkt_size + YAPS_DELIM_GET_PADDING(delim);

		if (pkts[i].skb)
			MORSE_YAPS_ERR(yaps->mors, "yaps packet leak\n");

		/* SKB doesn't want padding */
		pkts[i].skb = dev_alloc_skb(pkt_size);
		if (!pkts[i].skb) {
			ret = -ENOMEM;
			MORSE_YAPS_ERR(yaps->mors, "yaps no mem for skb\n");
			goto exit;
		}
		skb_put(pkts[i].skb, pkt_size);

		if (total_len <= bytes_remaining) {
			/* Case where entire packet fits in the remaining window */
			memcpy(pkts[i].skb->data, read_ptr, pkt_size);
			read_ptr += total_len;
			bytes_remaining -= total_len;
		} else {
			/* Case where packet runs off the end of the window */
			const int read_overhang_len = total_len - bytes_remaining;
			const int pkt_overhang_len = pkt_size - bytes_remaining;

			yaps->mors->debug.page_stats.rx_split++;
			/* TODO remove the warning, this is not a kernel bug */
			MORSE_DBG_RATELIMITED(yaps->mors, "yaps split pkt\n");
			memcpy(pkts[i].skb->data, read_ptr, bytes_remaining);
			read_ptr = from_chip_buffer_aligned;

			ret = morse_dm_read(yaps->mors,
					    /* Offset by 4 to avoid retry logic */
					    yaps->aux_data->ysl_addr + 4,
					    read_ptr, read_overhang_len);

			if (ret)
				goto exit;

			memcpy(pkts[i].skb->data + bytes_remaining, read_ptr, pkt_overhang_len);
			read_ptr += read_overhang_len;
			bytes_remaining = 0;
		}
		pkts[i].fc_queue = YAPS_DELIM_GET_POOL_ID(delim);
		*num_pkts_received += 1;
		i++;
	}

	if (again)
		ret = -EAGAIN;

exit:
	yaps_hw_unlock(yaps);
	return ret;
}

static int morse_yaps_hw_update_status(struct morse_yaps *yaps)
{
	int ret;
	int tc_total_pkt_count;
	unsigned long reg_read_timeout;

	struct morse_yaps_status_registers *status_regs = &yaps->aux_data->status_regs;

	ret = yaps_hw_lock(yaps);
	if (ret) {
		MORSE_YAPS_DBG(yaps->mors, "%s yaps lock failed %d\n", __func__, ret);
		return ret;
	}

	reg_read_timeout = jiffies + msecs_to_jiffies(100);
	do {
		if (time_after(jiffies, reg_read_timeout)) {
			MORSE_YAPS_ERR(yaps->mors, "%s: timed out reading status registers: %d\n",
				       __func__, ret);
			ret = -ETIMEDOUT;
			break;
		}
		ret = morse_dm_read(yaps->mors, yaps->aux_data->status_regs_addr,
				    (u8 *)status_regs, sizeof(*status_regs));

		/* This loop is locking up the apps core on USB FPGA.
		 * Add a delay to allow it to process other tasks.
		 */
		if (yaps->mors->bus_type == MORSE_HOST_BUS_TYPE_USB &&
				MORSE_DEVICE_TYPE_IS_FPGA(yaps->mors->chip_id))
			usleep_range(20, 50);

	} while (!ret && status_regs->lock);

	if (ret) {
		if (ret != -ENODEV) {
			/* TODO remove, this is not a kernel bug */
			WARN_ON_ONCE(ret);
			MORSE_YAPS_ERR(yaps->mors, "%s: error reading yaps status registers: %d\n",
					__func__, ret);
		}
		goto exit_unlock;
	}

	status_regs->tc_tx_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->tc_tx_pool_num_pages);
	status_regs->tc_cmd_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->tc_cmd_pool_num_pages);
	status_regs->tc_beacon_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->tc_beacon_pool_num_pages);
	status_regs->tc_mgmt_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->tc_mgmt_pool_num_pages);
	status_regs->fc_rx_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->fc_rx_pool_num_pages);
	status_regs->fc_resp_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->fc_resp_pool_num_pages);
	status_regs->fc_tx_sts_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->fc_tx_sts_pool_num_pages);
	status_regs->fc_aux_pool_num_pages =
		le32_to_cpu((__force __le32)status_regs->fc_aux_pool_num_pages);
	status_regs->tc_tx_num_pkts =
		le32_to_cpu((__force __le32)status_regs->tc_tx_num_pkts);
	status_regs->tc_cmd_num_pkts =
		le32_to_cpu((__force __le32)status_regs->tc_cmd_num_pkts);
	status_regs->tc_beacon_num_pkts =
		le32_to_cpu((__force __le32)status_regs->tc_beacon_num_pkts);
	status_regs->tc_mgmt_num_pkts =
		le32_to_cpu((__force __le32)status_regs->tc_mgmt_num_pkts);
	status_regs->fc_num_pkts =
		le32_to_cpu((__force __le32)status_regs->fc_num_pkts);
	status_regs->fc_done_num_pkts = le32_to_cpu((__force __le32)status_regs->fc_done_num_pkts);
	status_regs->fc_rx_bytes_in_queue =
		le32_to_cpu((__force __le32)status_regs->fc_rx_bytes_in_queue);
	status_regs->tc_delim_crc_fail_detected =
	    le32_to_cpu((__force __le32)status_regs->tc_delim_crc_fail_detected);
	status_regs->lock = le32_to_cpu((__force __le32)status_regs->lock);
	status_regs->fc_host_ysl_status =
		le32_to_cpu((__force __le32)status_regs->fc_host_ysl_status);

	/* SW-7464
	 * tc_total_pkt_count accounts for the packets that have been sent to the chip and
	 * haven't been processed (or might have been processed but the status registers
	 * haven't been updated yet). These packets will get metadata later, but that is
	 * not reflected in the current metadata count. So if we don't consider them and push
	 * more packets, we will run out of metadata buffers.
	 */
	tc_total_pkt_count = status_regs->tc_tx_num_pkts +
	    status_regs->tc_cmd_num_pkts +
	    status_regs->tc_beacon_num_pkts + status_regs->tc_mgmt_num_pkts;

	if (status_regs->tc_delim_crc_fail_detected) {
		/* Host and chip have become desynchronised. This can happen if the chip
		 * crashes during a YAPS transaction. We cannot recover from this.
		 */
		/* TODO remove */
		WARN_ON_ONCE(status_regs->tc_delim_crc_fail_detected);
		MORSE_YAPS_ERR(yaps->mors, "%s: to-chip yaps delimiter CRC fail, pkt_count=%d\n",
			__func__, tc_total_pkt_count);
		ret = -EIO;
	}

exit_unlock:
	yaps_hw_unlock(yaps);

	return ret;
}

static void morse_yaps_hw_show(struct morse_yaps *yaps, struct seq_file *file)
{
	struct morse_yaps_status_registers *status_regs = &yaps->aux_data->status_regs;

	seq_printf(file, "flags:0x%01x\n", yaps->flags);
	seq_printf(file, "YDS addr: %x\n", yaps->aux_data->yds_addr);
	seq_printf(file, "YSL addr: %x\n", yaps->aux_data->ysl_addr);
	seq_printf(file, "Status addr: %x\n", yaps->aux_data->status_regs_addr);

	seq_puts(file, "YAPS status registers\n");
	seq_printf(file, "\tp_tx %d\n", status_regs->tc_tx_pool_num_pages);
	seq_printf(file, "\tp_cmd %d\n", status_regs->tc_cmd_pool_num_pages);
	seq_printf(file, "\tp_bcn %d\n", status_regs->tc_beacon_pool_num_pages);
	seq_printf(file, "\tp_mgmt %d\n", status_regs->tc_mgmt_pool_num_pages);
	seq_printf(file, "\tp_rx %d\n", status_regs->fc_rx_pool_num_pages);
	seq_printf(file, "\tp_resp %d\n", status_regs->fc_resp_pool_num_pages);
	seq_printf(file, "\tp_sts %d\n", status_regs->fc_tx_sts_pool_num_pages);
	seq_printf(file, "\tp_aux %d\n", status_regs->fc_aux_pool_num_pages);
	seq_printf(file, "\tq_tx_n %d\n", status_regs->tc_tx_num_pkts);
	seq_printf(file, "\tq_cmd_n %d\n", status_regs->tc_cmd_num_pkts);
	seq_printf(file, "\tq_bcn_n %d\n", status_regs->tc_beacon_num_pkts);
	seq_printf(file, "\tq_mgmt_n %d\n", status_regs->tc_mgmt_num_pkts);
	seq_printf(file, "\tq_fc_n %d\n", status_regs->fc_num_pkts);
	seq_printf(file, "\tq_fc_bytes %d\n", status_regs->fc_rx_bytes_in_queue);
	seq_printf(file, "\tq_fc_done_n %d\n", status_regs->fc_done_num_pkts);
	seq_printf(file, "\tdelim_crc_fail %d\n", status_regs->tc_delim_crc_fail_detected);
	seq_printf(file, "\tscratch_0%d\n", status_regs->scratch_0);
	seq_printf(file, "\tscratch_1 %d\n", status_regs->scratch_1);
}

const struct morse_yaps_ops morse_yaps_hw_ops = {
	.write_pkts = morse_yaps_hw_write_pkts,
	.read_pkts = morse_yaps_hw_read_pkts,
	.update_status = morse_yaps_hw_update_status,
	.show = morse_yaps_hw_show
};

int morse_yaps_hw_init(struct morse *mors)
{
	int ret = 0;
	int flags;
	struct morse_yaps *yaps = NULL;
	int aux_data_len = sizeof(struct morse_yaps_hw_aux_data);
	int alignment = mors->bus_ops->bulk_alignment;

	morse_claim_bus(mors);

	mors->chip_if = kzalloc(sizeof(*mors->chip_if), GFP_KERNEL);
	if (!mors->chip_if) {
		ret = -ENOMEM;
		goto err_exit;
	}

	mors->chip_if->yaps = kzalloc(sizeof(*mors->chip_if->yaps), GFP_KERNEL);
	if (!mors->chip_if->yaps) {
		ret = -ENOMEM;
		goto err_exit;
	}

	yaps = mors->chip_if->yaps;
	yaps->aux_data = kzalloc(aux_data_len, GFP_KERNEL);
	if (!yaps->aux_data) {
		ret = -ENOMEM;
		goto err_exit;
	}

	yaps->aux_data->to_chip_buffer = kzalloc(YAPS_HW_WINDOW_SIZE_BYTES + alignment - 1,
						 GFP_KERNEL);
	if (!yaps->aux_data->to_chip_buffer) {
		ret = -ENOMEM;
		goto err_exit;
	}

	yaps->aux_data->from_chip_buffer = kzalloc(YAPS_HW_WINDOW_SIZE_BYTES + alignment - 1,
						   GFP_KERNEL);
	if (!yaps->aux_data->from_chip_buffer) {
		ret = -ENOMEM;
		goto err_exit;
	}

	if (!IS_ALIGNED((uintptr_t)&yaps->aux_data->status_regs, alignment)) {
		MORSE_YAPS_WARN(mors, "%s: Status registers are not aligned to %d bytes\n",
				__func__, alignment);
	}

	yaps->ops = &morse_yaps_hw_ops;

	/* This is mostly for compatibility with pageset API.
	 * We just have one YAPS instance that does everything.
	 */
	flags = MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_COMMAND |
	    MORSE_CHIP_IF_FLAGS_DIR_TO_HOST | MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP;

	ret = morse_yaps_init(mors, yaps, flags);
	if (ret) {
		MORSE_YAPS_ERR(mors, "morse_yaps_init failed %d\n", ret);
		goto err_exit;
	}

	INIT_WORK(&mors->chip_if_work, morse_yaps_work);
	INIT_WORK(&mors->tx_stale_work, morse_yaps_stale_tx_work);

	/* yaps irq will claim and release the bus */
	morse_release_bus(mors);
	morse_hw_enable_stop_notifications(mors, true);

	return ret;

err_exit:
	if (yaps)
		morse_yaps_finish(yaps);
	morse_yaps_hw_finish(mors);
	morse_release_bus(mors);
	return ret;
}

void morse_yaps_hw_finish(struct morse *mors)
{
	struct morse_yaps *yaps;

	if (!mors->chip_if)
		return;

	yaps = mors->chip_if->yaps;
	morse_yaps_hw_enable_irqs(mors, false);
	morse_yaps_finish(yaps);
	cancel_work_sync(&mors->chip_if_work);
	cancel_work_sync(&mors->tx_stale_work);
	if (yaps->aux_data) {
		kfree(yaps->aux_data->from_chip_buffer);
		yaps->aux_data->from_chip_buffer = NULL;
		kfree(yaps->aux_data->to_chip_buffer);
		yaps->aux_data->to_chip_buffer = NULL;
		kfree(yaps->aux_data);
		yaps->aux_data = NULL;
	}
	yaps->ops = NULL;
	kfree(yaps);
	mors->chip_if->yaps = NULL;
	kfree(mors->chip_if);
	mors->chip_if = NULL;
}
