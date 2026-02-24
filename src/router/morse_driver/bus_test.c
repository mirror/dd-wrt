/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/math64.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include "morse.h"
#include "bus.h"
#include "debug.h"

#ifdef CONFIG_MORSE_ENABLE_TEST_MODES

#define BUS_TEST_MAX_BLOCK_SIZE	(64 * 1024)
#define	BUS_TEST_SIZE_LIST {512, 560, 2048, 2048 + 128, \
				4 * 1024, 4 * 1024 + 3 * 128, \
				6 * 1024 + 508, 16 * 1024, \
				32 * 1024, BUS_TEST_MAX_BLOCK_SIZE}

/* Since different chips have different address spaces,
 * it is up to the developer to define a list of addresses
 * to be tested
 * Default configuration is the DMEM start address.
 */
#define BUS_TEST_READ_WRITE_ADDRESS_LIST {mors->cfg->regs->pager_base_address,}

/* Batch size for tput tests */
#define PROFILER_BATCH_SIZE	(16)

/* Number of bytes of 'data' in tput tests. Comes from MTU minus IP / TCP header */
#define PROFILER_PACKET_DATA_SIZE (1460)

#define PROFILER_OVERHEAD_DATA_SIZE	(sizeof(struct morse_buff_skb_header) + \
					sizeof(struct ieee80211_qos_hdr) + \
					8 /* llc snap hdr */ + \
					sizeof(struct iphdr) + sizeof(struct udphdr))

/* Number of SKBs to allocate in allocation profiler */
#define PROFILER_NUM_SKBS		(100)

/* Number of times to loop and average for bus throughput profiler */
#define PROFILER_TPUT_NUM_LOOPS		(10)

#define PROFILER_TIMING_PRINT_BUFFER_SIZE	(512)

/* Word invert read and write registers for mm6108 */
#define MM6108_WORD_INVERT_WR_REG	(0x10054108)
#define MM6108_WORD_INVERT_RD_REG	(0x1005410c)

/* For timing profiler */
enum {
	PROFILE_TYPE_CLAIM,
	PROFILE_TYPE_RELEASE,
	PROFILE_TYPE_RD32,
	PROFILE_TYPE_RDBULK,
	PROFILE_TYPE_WR32,
	PROFILE_TYPE_WRBULK,
	PROFILE_TYPE_NUM_ENTRIES
};

/* For timing profiler */
static const char * const profile_strings[] = {
	[PROFILE_TYPE_CLAIM] = "bus claim",
	[PROFILE_TYPE_RELEASE] = "bus release",
	[PROFILE_TYPE_RD32] = "read 32",
	[PROFILE_TYPE_RDBULK] = "read bulk",
	[PROFILE_TYPE_WR32] = "write 32",
	[PROFILE_TYPE_WRBULK] = "write bulk",
};

static int morse_bus_write_read_compare(struct morse *mors, int size, u8 value, u32 address)
{
	int ret;
	u8 *write_buff, *read_buff;

	write_buff = kmalloc(size, GFP_KERNEL);
	read_buff = kmalloc(size, GFP_KERNEL);

	memset(write_buff, value, size);
	ret = morse_dm_write(mors, address, write_buff, size);
	MORSE_INFO(mors, "%s: Writing %d bytes (0x%02X) to 0x%08X %s\n", __func__,
		   size, value, address, ret < 0 ? "FAILED" : "PASSED");
	if (ret < 0)
		goto exit;
	memset(read_buff, ~value, size);
	ret = morse_dm_read(mors, address, read_buff, size);
	MORSE_INFO(mors, "%s: Reading %d bytes from 0x%08X %s\n", __func__,
		   size, address, ret < 0 ? "FAILED" : "PASSED");
	if (ret < 0)
		goto exit;

	ret = memcmp(write_buff, read_buff, size);

	MORSE_INFO(mors, "%s: Verifying %d bytes %s\n", __func__,
		   size, ret != 0 ? "FAILED" : "PASSED");
	ret = ret ? -EPROTO : ret;

exit:
	kfree(write_buff);
	kfree(read_buff);
	return ret;
}

int morse_bus_test(struct morse *mors, const char *bus_name)
{
	int size_idx, ret;
	u32 chip_id;
	u32 cmp_size_list[] = BUS_TEST_SIZE_LIST;
	u32 address_list[] = BUS_TEST_READ_WRITE_ADDRESS_LIST;

	MORSE_WARN(mors, "---==[ START %s BUS TEST ]==---\n", bus_name);
	morse_claim_bus(mors);

	ret = morse_reg32_read(mors, MORSE_REG_CHIP_ID(mors), &chip_id);
	if (ret) {
		MORSE_ERR(mors, "%s: Chip ID read failed %d\n", __func__, ret);
		goto exit;
	}
	if (chip_id != mors->chip_id) {
		MORSE_ERR(mors, "%s Chip ID read 0x%04X does not match expected 0x%04X\n",
			  __func__, chip_id, mors->chip_id);
		ret = -1;
		goto exit;
	}
	MORSE_INFO(mors, "%s: Chip ID read passed 0x%04X\n", __func__, chip_id);

	for (size_idx = 0; size_idx < ARRAY_SIZE(cmp_size_list); size_idx++) {
		int cmp_size = cmp_size_list[size_idx];
		int address_idx;

		for (address_idx = 0; address_idx < ARRAY_SIZE(address_list); address_idx++) {
			MORSE_INFO(mors, "%s: Writing, reading and verifying:\n", __func__);
			ret =
			    morse_bus_write_read_compare(mors, cmp_size, 0xAA,
							 address_list[address_idx]);
			if (ret)
				goto exit;

			MORSE_INFO(mors, "%s: Clearing, reading and verifying:\n", __func__);
			ret =
			    morse_bus_write_read_compare(mors, cmp_size, 0,
							 address_list[address_idx]);
			if (ret)
				goto exit;
		}
	}

	ret = morse_reg32_read(mors, MORSE_REG_CHIP_ID(mors), &chip_id);
	MORSE_INFO(mors, "%s: Final Reading Chip ID %s\n", __func__, ret < 0 ? "FAILED" : "PASSED");

	if (ret < 0)
		goto exit;

exit:
	morse_release_bus(mors);
	MORSE_WARN(mors, "---==[ %s BUS TEST %s ]==---\n", bus_name, ret ? "FAILED" : "PASSED");
	return ret;
}

static void morse_bus_tput_timing_test(struct morse *mors)
{
	const u16 rounds = PROFILER_BATCH_SIZE;
	const u32 pkt_size = PROFILER_PACKET_DATA_SIZE;
	const u32 overhead_size = PROFILER_OVERHEAD_DATA_SIZE;
	const u32 write_size = ROUND_BYTES_TO_WORD(pkt_size + overhead_size);
	u32 read_err = 0;
	u32 write_err = 0;
	ktime_t start_ktime;
	ktime_t end_ktime;
	char *buffer;
	int count;
	int i, j;
	u8 *send_buffer = NULL;
	u32 reg_read_val;
	u32 reg_write_val = 0xdeadbeef;
	u8 val;
	ktime_t **total_times;

	u32 reg_addr_rd = MM6108_WORD_INVERT_RD_REG;
	u32 reg_addr_wr = MM6108_WORD_INVERT_WR_REG;

	/* Start of dmem */
	u32 dm_addr = mors->cfg->regs->pager_base_address;

	send_buffer = kmalloc(write_size, GFP_KERNEL);
	if (!send_buffer)
		return;

	total_times = kcalloc(PROFILE_TYPE_NUM_ENTRIES, sizeof(*total_times), GFP_KERNEL);
	if (!total_times) {
		kfree(send_buffer);
		return;
	}

	for (i = 0; i < PROFILE_TYPE_NUM_ENTRIES; i++) {
		total_times[i] = kmalloc_array(rounds, sizeof(**total_times), GFP_KERNEL);
		if (!total_times[i])
			goto exit;
	}

	/* Fill the buffer with changing data, increment by 0x11 so we have a predictable pattern */
	for (i = 0, val = 0; i < write_size; i++, val += 0x11)
		send_buffer[i] = val;

	dev_info(mors->dev, "Bus timing profiler\n");
	dev_info(mors->dev, "    packet size (bytes): %u\n", pkt_size);
	dev_info(mors->dev, "    overhead (bytes):    %u\n", overhead_size);
	dev_info(mors->dev, "    padding (bytes):     %u\n",
			write_size - (pkt_size + overhead_size));
	dev_info(mors->dev, "    rounds:              %u\n", rounds);

	for (j = 0; j < rounds; j++) {
		start_ktime = ktime_get();
		morse_claim_bus(mors);
		end_ktime = ktime_get();
		total_times[PROFILE_TYPE_CLAIM][j] = ktime_sub(end_ktime, start_ktime);

		start_ktime = ktime_get();
		morse_release_bus(mors);
		end_ktime = ktime_get();
		total_times[PROFILE_TYPE_RELEASE][j] = ktime_sub(end_ktime, start_ktime);
	}

	morse_claim_bus(mors);

	for (j = 0; j < rounds; j++) {
		start_ktime = ktime_get();
		if (mors->bus_ops->reg32_read(mors, reg_addr_rd, &reg_read_val))
			read_err++;

		end_ktime = ktime_get();
		total_times[PROFILE_TYPE_RD32][j] = ktime_sub(end_ktime, start_ktime);
	}

	for (j = 0; j < rounds; j++) {
		start_ktime = ktime_get();
		if (mors->bus_ops->dm_write(mors, dm_addr, send_buffer, write_size))
			write_err++;

		end_ktime = ktime_get();
		total_times[PROFILE_TYPE_WRBULK][j] = ktime_sub(end_ktime, start_ktime);
	}

	for (j = 0; j < rounds; j++) {
		start_ktime = ktime_get();
		if (mors->bus_ops->reg32_write(mors, reg_addr_wr, reg_write_val))
			write_err++;

		end_ktime = ktime_get();
		total_times[PROFILE_TYPE_WR32][j] = ktime_sub(end_ktime, start_ktime);
	}

	for (j = 0; j < rounds; j++) {
		start_ktime = ktime_get();
		if (mors->bus_ops->dm_read(mors, dm_addr, send_buffer, write_size))
			write_err++;

		end_ktime = ktime_get();
		total_times[PROFILE_TYPE_RDBULK][j] = ktime_sub(end_ktime, start_ktime);
	}

	morse_release_bus(mors);

	if (read_err || write_err) {
		dev_info(mors->dev,
				 "    Errors in IO (read: %d, write: %d)\n",
				 read_err, write_err);
		goto exit;
	}

	dev_info(mors->dev, "    timing (us)\n");

	buffer = kmalloc(PROFILER_TIMING_PRINT_BUFFER_SIZE, GFP_KERNEL);

	for (i = 0; i < PROFILE_TYPE_NUM_ENTRIES; i++) {
		count = snprintf(buffer,
						 PROFILER_TIMING_PRINT_BUFFER_SIZE,
						 "    %-11s:",
						 profile_strings[i]);
		for (j = 0; j < rounds; j++) {
			count += snprintf(buffer + count,
							(PROFILER_TIMING_PRINT_BUFFER_SIZE - count),
							" %4lld",
							ktime_to_us(total_times[i][j]));
		}
		dev_info(mors->dev, "%s\n", buffer);
	}

	kfree(buffer);

exit:
	kfree(send_buffer);
	for (i = 0; i < PROFILE_TYPE_NUM_ENTRIES; i++)
		kfree(total_times[i]);
	kfree(total_times);
}

static void morse_skb_allocator_test(struct morse *mors)
{
	const u32 pkt_size = PROFILER_PACKET_DATA_SIZE;
	const u32 overhead_size = PROFILER_OVERHEAD_DATA_SIZE;
	int i;
	struct sk_buff *skbs[PROFILER_NUM_SKBS];
	ktime_t start, end;
	u64 us_alloc, us_free;

	start = ktime_get();
	for (i = 0; i < ARRAY_SIZE(skbs); i++)
		skbs[i] = dev_alloc_skb(pkt_size + overhead_size);

	end = ktime_get();
	us_alloc = ktime_to_us(ktime_sub(end, start));

	start = ktime_get();
	for (i = 0; i < ARRAY_SIZE(skbs); i++)
		kfree_skb(skbs[i]);
	end = ktime_get();
	us_free = ktime_to_us(ktime_sub(end, start));

	dev_info(mors->dev, "SKB allocation profiler (%u skbs w/ %u bytes)\n",
			PROFILER_NUM_SKBS, pkt_size + overhead_size);
	dev_info(mors->dev, "    alloc: %llu us\n", us_alloc);
	dev_info(mors->dev, "    free:  %llu us\n", us_free);
}

static void morse_bus_tput_test(struct morse *mors)
{
	const u32 num_loops = PROFILER_TPUT_NUM_LOOPS;
	const u16 batch_size = PROFILER_BATCH_SIZE;
	const u32 pkt_size = PROFILER_PACKET_DATA_SIZE;
	const u32 overhead_size = PROFILER_OVERHEAD_DATA_SIZE;
	const u32 write_size = ROUND_BYTES_TO_WORD(pkt_size + overhead_size);
	u32 read_err = 0;
	u32 write_err = 0;
	ktime_t start;
	ktime_t end;
	u64 ms_elasped;
	u64 kbps;
	u64 total_pkt_bytes_written;
	int i;
	int j;
	u8 *send_buffer;
	u32 reg_read_val;
	u32 reg_write_val = 0xdeadbeef;
	u8 val;

	u32 reg_addr_rd = MM6108_WORD_INVERT_RD_REG;
	u32 reg_addr_wr = MM6108_WORD_INVERT_WR_REG;

	/* Start of dmem */
	u32 dm_addr = mors->cfg->regs->pager_base_address;

	send_buffer = kmalloc(write_size, GFP_KERNEL);

	if (!send_buffer)
		return;

	/* Fill the buffer with changing data, increment by 0x11 so we have a predictable pattern */
	for (i = 0, val = 0; i < write_size; i++, val += 0x11)
		send_buffer[i] = val;

	dev_info(mors->dev, "Bus IO write estimator\n");
	dev_info(mors->dev, "    packet size (bytes): %u\n", pkt_size);
	dev_info(mors->dev, "    overhead (bytes):    %u\n", overhead_size);
	dev_info(mors->dev, "    padding (bytes):     %u\n",
			write_size - (pkt_size + overhead_size));
	dev_info(mors->dev, "    batch(es):           %u\n", batch_size);
	dev_info(mors->dev, "    rounds:              %u\n", num_loops);

	start = ktime_get();

	for (i = 0; i < num_loops; i++) {
		morse_claim_bus(mors);

		for (j = 0; j < batch_size; j++)
			if (mors->bus_ops->reg32_read(mors, reg_addr_rd, &reg_read_val))
				read_err++;

		for (j = 0; j < batch_size; j++)
			if (mors->bus_ops->dm_write(mors, dm_addr, send_buffer, write_size))
				write_err++;

		for (j = 0; j < batch_size; j++)
			if (mors->bus_ops->reg32_write(mors, reg_addr_wr, reg_write_val))
				write_err++;

		morse_release_bus(mors);
		morse_claim_bus(mors);

		/* 1 extra read and write per batch to emulate reading / writing IRQ bit */
		if (mors->bus_ops->reg32_read(mors, reg_addr_rd, &reg_read_val))
			read_err++;

		if (mors->bus_ops->reg32_write(mors, reg_addr_wr, reg_write_val))
			write_err++;

		morse_release_bus(mors);
	}

	end = ktime_get();
	ms_elasped = ktime_to_ms(ktime_sub(end, start));

	if (read_err || write_err) {
		dev_info(mors->dev,
				 "    Errors in IO (read: %d, write: %d)\n",
				 read_err, write_err);
		goto exit;
	}

	total_pkt_bytes_written = (u64)pkt_size * num_loops * batch_size;
	dev_info(mors->dev, "    Wrote %llu bytes in %llu ms\n",
		total_pkt_bytes_written, ms_elasped);

	/* To convert bytes per msec -> kbps, multiply by 8 */
	kbps = div_u64(total_pkt_bytes_written, ms_elasped) * 8;

	dev_info(mors->dev, "    Estimated IO upper bound: %llu kbps\n", kbps);
exit:
	kfree(send_buffer);
}

static struct {
	ktime_t start;
	ktime_t end;
	struct completion *handled;
} irq_profiling = {
	.start = 0,
	.end = 0,
	.handled = NULL
};

void morse_bus_interrupt_profiler_irq(struct morse *mors)
{
	if (!irq_profiling.handled)
		return;

	irq_profiling.end = ktime_get_boottime();
	complete(irq_profiling.handled);
}

static void morse_bus_interrupt_profiler(struct morse *mors)
{
	int ret;
	unsigned int i;
	const unsigned int rounds = PROFILER_BATCH_SIZE;
	const unsigned int timeout_ms = 1000;
	DECLARE_COMPLETION_ONSTACK(irq_handled);
	ktime_t *irq_delays = NULL;
	char *print_buffer = NULL;
	int count;

	irq_delays = kcalloc(rounds, sizeof(*irq_delays), GFP_KERNEL);
	if (!irq_delays)
		goto exit;
	print_buffer = kzalloc(PROFILER_TIMING_PRINT_BUFFER_SIZE, GFP_KERNEL);
	if (!print_buffer)
		goto exit;

	dev_info(mors->dev, "Interrupt profiling (%u round(s))\n", rounds);
	morse_hw_irq_clear(mors);
	morse_hw_irq_enable(mors, MORSE_INT_BUS_IRQ_SELF_TEST_NUM, true);

	for (i = 0; i < rounds; i++) {
		morse_claim_bus(mors);
		irq_profiling.handled = &irq_handled;
		irq_profiling.start = ktime_get_boottime();
		morse_reg32_write(mors, MORSE_REG_INT1_SET(mors), MORSE_INT_BUS_IRQ_SELF_TEST);
		morse_release_bus(mors);

		ret = wait_for_completion_timeout(&irq_handled, msecs_to_jiffies(timeout_ms));

		/* Claim bus to prevent race on irq_profiling */
		morse_claim_bus(mors);
		irq_profiling.handled = NULL;
		if (ret == 0)
			irq_delays[i] = 0;
		else
			irq_delays[i] = ktime_sub(irq_profiling.end, irq_profiling.start);
		reinit_completion(&irq_handled);
		morse_release_bus(mors);
	}

	morse_hw_irq_enable(mors, MORSE_INT_BUS_IRQ_SELF_TEST_NUM, false);
	morse_hw_irq_clear(mors);

	dev_info(mors->dev, "    timing (us)\n");
	count = snprintf(print_buffer, PROFILER_TIMING_PRINT_BUFFER_SIZE, "    %-11s:",
			 "irq delay");
	for (i = 0; i < rounds; i++) {
		ktime_t delay = irq_delays[i];

		if (delay == 0)
			count += snprintf(print_buffer + count,
					  (PROFILER_TIMING_PRINT_BUFFER_SIZE - count),
					  " %4s", "?");
		else
			count += snprintf(print_buffer + count,
					  (PROFILER_TIMING_PRINT_BUFFER_SIZE - count),
					  " %4lld", ktime_to_us(delay));
	}

	dev_info(mors->dev, "%s\n", print_buffer);
exit:
	kfree(print_buffer);
	kfree(irq_delays);
}

void morse_bus_throughput_profiler(struct morse *mors)
{
	const char *mm610x_hw_str =  "MM610";
	const char *hw_vers = mors->cfg->get_hw_version(mors->chip_id);

	if (strncmp(hw_vers, mm610x_hw_str, strlen(mm610x_hw_str)) != 0) {
		MORSE_ERR(mors, "Bus throughput profiler only available for MM610x\n");
		return;
	}
	morse_bus_tput_test(mors);
	morse_bus_tput_timing_test(mors);
	morse_skb_allocator_test(mors);
	morse_bus_interrupt_profiler(mors);
}

#endif
