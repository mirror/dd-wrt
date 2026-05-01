// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* PPE debugfs routines for display of PPE counters useful for debug. */

#include <linux/bitfield.h>
#include <linux/debugfs.h>
#include <linux/netdevice.h>
#include <linux/regmap.h>
#include <linux/seq_file.h>

#include "edma.h"
#include "ppe.h"
#include "ppe_config.h"
#include "ppe_debugfs.h"
#include "ppe_regs.h"

#define PPE_PKT_CNT_TBL_SIZE				3
#define PPE_DROP_PKT_CNT_TBL_SIZE			5

#define PPE_W0_PKT_CNT					GENMASK(31, 0)
#define PPE_W2_DROP_PKT_CNT_LOW				GENMASK(31, 8)
#define PPE_W3_DROP_PKT_CNT_HIGH			GENMASK(7, 0)

#define PPE_GET_PKT_CNT(tbl_cnt)			\
	u32_get_bits(*((u32 *)(tbl_cnt)), PPE_W0_PKT_CNT)
#define PPE_GET_DROP_PKT_CNT_LOW(tbl_cnt)		\
	u32_get_bits(*((u32 *)(tbl_cnt) + 0x2), PPE_W2_DROP_PKT_CNT_LOW)
#define PPE_GET_DROP_PKT_CNT_HIGH(tbl_cnt)		\
	u32_get_bits(*((u32 *)(tbl_cnt) + 0x3), PPE_W3_DROP_PKT_CNT_HIGH)

#define PRINT_COUNTER_PREFIX(desc, cnt_type)		\
	seq_printf(seq, "%-16s %16s", desc, cnt_type)

#define PRINT_CPU_CODE_COUNTER(cnt, code)		\
	seq_printf(seq, "%10u(cpucode:%d)", cnt, code)

#define PRINT_DROP_CODE_COUNTER(cnt, port, code)	\
	seq_printf(seq, "%10u(port=%d),dropcode:%d", cnt, port, code)

#define PRINT_SINGLE_COUNTER(tag, cnt, str, index)			\
do {									\
	if (!((tag) % 4))							\
		seq_printf(seq, "\n%-16s %16s", "", "");		\
	seq_printf(seq, "%10u(%s=%04d)", cnt, str, index);		\
} while (0)

#define PRINT_TWO_COUNTERS(tag, cnt0, cnt1, str, index)			\
do {									\
	if (!((tag) % 4))							\
		seq_printf(seq, "\n%-16s %16s", "", "");		\
	seq_printf(seq, "%10u/%u(%s=%04d)", cnt0, cnt1, str, index);	\
} while (0)

/**
 * enum ppe_cnt_size_type - PPE counter size type
 * @PPE_PKT_CNT_SIZE_1WORD: Counter size with single register
 * @PPE_PKT_CNT_SIZE_3WORD: Counter size with table of 3 words
 * @PPE_PKT_CNT_SIZE_5WORD: Counter size with table of 5 words
 *
 * PPE takes the different register size to record the packet counters.
 * It uses single register, or register table with 3 words or 5 words.
 * The counter with table size 5 words also records the drop counter.
 * There are also some other counter types occupying sizes less than 32
 * bits, which is not covered by this enumeration type.
 */
enum ppe_cnt_size_type {
	PPE_PKT_CNT_SIZE_1WORD,
	PPE_PKT_CNT_SIZE_3WORD,
	PPE_PKT_CNT_SIZE_5WORD,
};

static int ppe_pkt_cnt_get(struct ppe_device *ppe_dev, u32 reg,
			   enum ppe_cnt_size_type cnt_type,
			   u32 *cnt, u32 *drop_cnt)
{
	u32 drop_pkt_cnt[PPE_DROP_PKT_CNT_TBL_SIZE];
	u32 pkt_cnt[PPE_PKT_CNT_TBL_SIZE];
	u32 value;
	int ret;

	switch (cnt_type) {
	case PPE_PKT_CNT_SIZE_1WORD:
		ret = regmap_read(ppe_dev->regmap, reg, &value);
		if (ret)
			return ret;

		*cnt = value;
		break;
	case PPE_PKT_CNT_SIZE_3WORD:
		ret = regmap_bulk_read(ppe_dev->regmap, reg,
				       pkt_cnt, ARRAY_SIZE(pkt_cnt));
		if (ret)
			return ret;

		*cnt = PPE_GET_PKT_CNT(pkt_cnt);
		break;
	case PPE_PKT_CNT_SIZE_5WORD:
		ret = regmap_bulk_read(ppe_dev->regmap, reg,
				       drop_pkt_cnt, ARRAY_SIZE(drop_pkt_cnt));
		if (ret)
			return ret;

		*cnt = PPE_GET_PKT_CNT(drop_pkt_cnt);

		/* Drop counter with low 24 bits. */
		value  = PPE_GET_DROP_PKT_CNT_LOW(drop_pkt_cnt);
		*drop_cnt = FIELD_PREP(GENMASK(23, 0), value);

		/* Drop counter with high 8 bits. */
		value  = PPE_GET_DROP_PKT_CNT_HIGH(drop_pkt_cnt);
		*drop_cnt |= FIELD_PREP(GENMASK(31, 24), value);
		break;
	}

	return 0;
}

static void ppe_tbl_pkt_cnt_clear(struct ppe_device *ppe_dev, u32 reg,
				  enum ppe_cnt_size_type cnt_type)
{
	u32 drop_pkt_cnt[PPE_DROP_PKT_CNT_TBL_SIZE] = {};
	u32 pkt_cnt[PPE_PKT_CNT_TBL_SIZE] = {};

	switch (cnt_type) {
	case PPE_PKT_CNT_SIZE_1WORD:
		regmap_write(ppe_dev->regmap, reg, 0);
		break;
	case PPE_PKT_CNT_SIZE_3WORD:
		regmap_bulk_write(ppe_dev->regmap, reg,
				  pkt_cnt, ARRAY_SIZE(pkt_cnt));
		break;
	case PPE_PKT_CNT_SIZE_5WORD:
		regmap_bulk_write(ppe_dev->regmap, reg,
				  drop_pkt_cnt, ARRAY_SIZE(drop_pkt_cnt));
		break;
	}
}

/* The number of packets dropped because of no buffer available, no PPE
 * buffer assigned to these packets.
 */
static void ppe_port_rx_drop_counter_get(struct ppe_device *ppe_dev,
					 struct seq_file *seq)
{
	u32 reg, drop_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("PRX_DROP_CNT", "SILENT_DROP:");
	for (i = 0; i < PPE_DROP_CNT_TBL_ENTRIES; i++) {
		reg = PPE_DROP_CNT_TBL_ADDR + i * PPE_DROP_CNT_TBL_INC;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_1WORD,
				      &drop_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (drop_cnt > 0) {
			tag++;
			PRINT_SINGLE_COUNTER(tag, drop_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets dropped because hardware buffers were available
 * only partially for the packet.
 */
static void ppe_port_rx_bm_drop_counter_get(struct ppe_device *ppe_dev,
					    struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("PRX_BM_DROP_CNT", "OVERFLOW_DROP:");
	for (i = 0; i < PPE_DROP_STAT_TBL_ENTRIES; i++) {
		reg = PPE_DROP_STAT_TBL_ADDR + PPE_DROP_STAT_TBL_INC * i;

		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0) {
			tag++;
			PRINT_SINGLE_COUNTER(tag, pkt_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of currently occupied buffers, that can't be flushed. */
static void ppe_port_rx_bm_port_counter_get(struct ppe_device *ppe_dev,
					    struct seq_file *seq)
{
	int used_cnt, react_cnt;
	int ret, i, tag = 0;
	u32 reg, val;

	PRINT_COUNTER_PREFIX("PRX_BM_PORT_CNT", "USED/REACT:");
	for (i = 0; i < PPE_BM_USED_CNT_TBL_ENTRIES; i++) {
		reg = PPE_BM_USED_CNT_TBL_ADDR + i * PPE_BM_USED_CNT_TBL_INC;
		ret = regmap_read(ppe_dev->regmap, reg, &val);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		/* The number of PPE buffers used for caching the received
		 * packets before the pause frame sent.
		 */
		used_cnt = FIELD_GET(PPE_BM_USED_CNT_VAL, val);

		reg = PPE_BM_REACT_CNT_TBL_ADDR + i * PPE_BM_REACT_CNT_TBL_INC;
		ret = regmap_read(ppe_dev->regmap, reg, &val);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		/* The number of PPE buffers used for caching the received
		 * packets after pause frame sent out.
		 */
		react_cnt = FIELD_GET(PPE_BM_REACT_CNT_VAL, val);

		if (used_cnt > 0 || react_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, used_cnt, react_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets processed by the ingress parser module of PPE. */
static void ppe_parse_pkt_counter_get(struct ppe_device *ppe_dev,
				      struct seq_file *seq)
{
	u32 reg, cnt = 0, tunnel_cnt = 0;
	int i, ret, tag = 0;

	PRINT_COUNTER_PREFIX("IPR_PKT_CNT", "TPRX/IPRX:");
	for (i = 0; i < PPE_IPR_PKT_CNT_TBL_ENTRIES; i++) {
		reg = PPE_TPR_PKT_CNT_TBL_ADDR + i * PPE_TPR_PKT_CNT_TBL_INC;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_1WORD,
				      &tunnel_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		reg = PPE_IPR_PKT_CNT_TBL_ADDR + i * PPE_IPR_PKT_CNT_TBL_INC;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_1WORD,
				      &cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (tunnel_cnt > 0 || cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, tunnel_cnt, cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets received or dropped on the ingress direction. */
static void ppe_port_rx_counter_get(struct ppe_device *ppe_dev,
				    struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0, drop_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("PORT_RX_CNT", "RX/RX_DROP:");
	for (i = 0; i < PPE_PHY_PORT_RX_CNT_TBL_ENTRIES; i++) {
		reg = PPE_PHY_PORT_RX_CNT_TBL_ADDR + PPE_PHY_PORT_RX_CNT_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_5WORD,
				      &pkt_cnt, &drop_cnt);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, pkt_cnt, drop_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets received or dropped by the port. */
static void ppe_vp_rx_counter_get(struct ppe_device *ppe_dev,
				  struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0, drop_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("VPORT_RX_CNT", "RX/RX_DROP:");
	for (i = 0; i < PPE_PORT_RX_CNT_TBL_ENTRIES; i++) {
		reg = PPE_PORT_RX_CNT_TBL_ADDR + PPE_PORT_RX_CNT_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_5WORD,
				      &pkt_cnt, &drop_cnt);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, pkt_cnt, drop_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets received or dropped by layer 2 processing. */
static void ppe_pre_l2_counter_get(struct ppe_device *ppe_dev,
				   struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0, drop_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("PRE_L2_CNT", "RX/RX_DROP:");
	for (i = 0; i < PPE_PRE_L2_CNT_TBL_ENTRIES; i++) {
		reg = PPE_PRE_L2_CNT_TBL_ADDR + PPE_PRE_L2_CNT_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_5WORD,
				      &pkt_cnt, &drop_cnt);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, pkt_cnt, drop_cnt, "vsi", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of VLAN packets received by PPE. */
static void ppe_vlan_counter_get(struct ppe_device *ppe_dev,
				 struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("VLAN_CNT", "RX:");
	for (i = 0; i < PPE_VLAN_CNT_TBL_ENTRIES; i++) {
		reg = PPE_VLAN_CNT_TBL_ADDR + PPE_VLAN_CNT_TBL_INC * i;

		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0) {
			tag++;
			PRINT_SINGLE_COUNTER(tag, pkt_cnt, "vsi", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets handed to CPU by PPE. */
static void ppe_cpu_code_counter_get(struct ppe_device *ppe_dev,
				     struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0;
	int ret, i;

	PRINT_COUNTER_PREFIX("CPU_CODE_CNT", "CODE:");
	for (i = 0; i < PPE_DROP_CPU_CNT_TBL_ENTRIES; i++) {
		reg = PPE_DROP_CPU_CNT_TBL_ADDR + PPE_DROP_CPU_CNT_TBL_INC * i;

		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (!pkt_cnt)
			continue;

		/* There are 256 CPU codes saved in the first 256 entries
		 * of register table, and 128 drop codes for each PPE port
		 * (0-7), the total entries is 256 + 8 * 128.
		 */
		if (i < 256)
			PRINT_CPU_CODE_COUNTER(pkt_cnt, i);
		else
			PRINT_DROP_CODE_COUNTER(pkt_cnt, (i - 256) % 8,
						(i - 256) / 8);
		seq_putc(seq, '\n');
		PRINT_COUNTER_PREFIX("", "");
	}

	seq_putc(seq, '\n');
}

/* The number of packets forwarded by VLAN on the egress direction. */
static void ppe_eg_vsi_counter_get(struct ppe_device *ppe_dev,
				   struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("EG_VSI_CNT", "TX:");
	for (i = 0; i < PPE_EG_VSI_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_EG_VSI_COUNTER_TBL_ADDR + PPE_EG_VSI_COUNTER_TBL_INC * i;

		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0) {
			tag++;
			PRINT_SINGLE_COUNTER(tag, pkt_cnt, "vsi", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets trasmitted or dropped by port. */
static void ppe_vp_tx_counter_get(struct ppe_device *ppe_dev,
				  struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0, drop_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("VPORT_TX_CNT", "TX/TX_DROP:");
	for (i = 0; i < PPE_VPORT_TX_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_VPORT_TX_COUNTER_TBL_ADDR + PPE_VPORT_TX_COUNTER_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		reg = PPE_VPORT_TX_DROP_CNT_TBL_ADDR + PPE_VPORT_TX_DROP_CNT_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &drop_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0 || drop_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, pkt_cnt, drop_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets trasmitted or dropped on the egress direction. */
static void ppe_port_tx_counter_get(struct ppe_device *ppe_dev,
				    struct seq_file *seq)
{
	u32 reg, pkt_cnt = 0, drop_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("PORT_TX_CNT", "TX/TX_DROP:");
	for (i = 0; i < PPE_PORT_TX_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_PORT_TX_COUNTER_TBL_ADDR + PPE_PORT_TX_COUNTER_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		reg = PPE_PORT_TX_DROP_CNT_TBL_ADDR + PPE_PORT_TX_DROP_CNT_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &drop_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (pkt_cnt > 0 || drop_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, pkt_cnt, drop_cnt, "port", i);
		}
	}

	seq_putc(seq, '\n');
}

/* The number of packets transmitted or pending by the PPE queue. */
static void ppe_queue_tx_counter_get(struct ppe_device *ppe_dev,
				     struct seq_file *seq)
{
	u32 reg, val, pkt_cnt = 0, pend_cnt = 0;
	int ret, i, tag = 0;

	PRINT_COUNTER_PREFIX("QUEUE_TX_CNT", "TX/PEND:");
	for (i = 0; i < PPE_QUEUE_TX_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_QUEUE_TX_COUNTER_TBL_ADDR + PPE_QUEUE_TX_COUNTER_TBL_INC * i;
		ret = ppe_pkt_cnt_get(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD,
				      &pkt_cnt, NULL);
		if (ret) {
			seq_printf(seq, "ERROR %d\n", ret);
			return;
		}

		if (i < PPE_AC_UNICAST_QUEUE_CFG_TBL_ENTRIES) {
			reg = PPE_AC_UNICAST_QUEUE_CNT_TBL_ADDR +
			      PPE_AC_UNICAST_QUEUE_CNT_TBL_INC * i;
			ret = regmap_read(ppe_dev->regmap, reg, &val);
			if (ret) {
				seq_printf(seq, "ERROR %d\n", ret);
				return;
			}

			pend_cnt = FIELD_GET(PPE_AC_UNICAST_QUEUE_CNT_TBL_PEND_CNT, val);
		} else {
			reg = PPE_AC_MULTICAST_QUEUE_CNT_TBL_ADDR +
			      PPE_AC_MULTICAST_QUEUE_CNT_TBL_INC *
			      (i - PPE_AC_UNICAST_QUEUE_CFG_TBL_ENTRIES);
			ret = regmap_read(ppe_dev->regmap, reg, &val);
			if (ret) {
				seq_printf(seq, "ERROR %d\n", ret);
				return;
			}

			pend_cnt = FIELD_GET(PPE_AC_MULTICAST_QUEUE_CNT_TBL_PEND_CNT, val);
		}

		if (pkt_cnt > 0 || pend_cnt > 0) {
			tag++;
			PRINT_TWO_COUNTERS(tag, pkt_cnt, pend_cnt, "queue", i);
		}
	}

	seq_putc(seq, '\n');
}

/* Display the various packet counters of PPE. */
static int ppe_packet_counter_show(struct seq_file *seq, void *v)
{
	struct ppe_device *ppe_dev = seq->private;

	ppe_port_rx_drop_counter_get(ppe_dev, seq);
	ppe_port_rx_bm_drop_counter_get(ppe_dev, seq);
	ppe_port_rx_bm_port_counter_get(ppe_dev, seq);
	ppe_parse_pkt_counter_get(ppe_dev, seq);
	ppe_port_rx_counter_get(ppe_dev, seq);
	ppe_vp_rx_counter_get(ppe_dev, seq);
	ppe_pre_l2_counter_get(ppe_dev, seq);
	ppe_vlan_counter_get(ppe_dev, seq);
	ppe_cpu_code_counter_get(ppe_dev, seq);
	ppe_eg_vsi_counter_get(ppe_dev, seq);
	ppe_vp_tx_counter_get(ppe_dev, seq);
	ppe_port_tx_counter_get(ppe_dev, seq);
	ppe_queue_tx_counter_get(ppe_dev, seq);

	return 0;
}

static int ppe_packet_counter_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_packet_counter_show, inode->i_private);
}

static ssize_t ppe_packet_counter_clear(struct file *file,
					const char __user *buf,
					size_t count, loff_t *pos)
{
	struct ppe_device *ppe_dev = file_inode(file)->i_private;
	u32 reg;
	int i;

	for (i = 0; i < PPE_DROP_CNT_TBL_ENTRIES; i++) {
		reg = PPE_DROP_CNT_TBL_ADDR + i * PPE_DROP_CNT_TBL_INC;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_1WORD);
	}

	for (i = 0; i < PPE_DROP_STAT_TBL_ENTRIES; i++) {
		reg = PPE_DROP_STAT_TBL_ADDR + PPE_DROP_STAT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	for (i = 0; i < PPE_IPR_PKT_CNT_TBL_ENTRIES; i++) {
		reg = PPE_IPR_PKT_CNT_TBL_ADDR + i * PPE_IPR_PKT_CNT_TBL_INC;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_1WORD);

		reg = PPE_TPR_PKT_CNT_TBL_ADDR + i * PPE_TPR_PKT_CNT_TBL_INC;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_1WORD);
	}

	for (i = 0; i < PPE_VLAN_CNT_TBL_ENTRIES; i++) {
		reg = PPE_VLAN_CNT_TBL_ADDR + PPE_VLAN_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	for (i = 0; i < PPE_PRE_L2_CNT_TBL_ENTRIES; i++) {
		reg = PPE_PRE_L2_CNT_TBL_ADDR + PPE_PRE_L2_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_5WORD);
	}

	for (i = 0; i < PPE_PORT_TX_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_PORT_TX_DROP_CNT_TBL_ADDR + PPE_PORT_TX_DROP_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);

		reg = PPE_PORT_TX_COUNTER_TBL_ADDR + PPE_PORT_TX_COUNTER_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	for (i = 0; i < PPE_EG_VSI_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_EG_VSI_COUNTER_TBL_ADDR + PPE_EG_VSI_COUNTER_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	for (i = 0; i < PPE_VPORT_TX_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_VPORT_TX_COUNTER_TBL_ADDR + PPE_VPORT_TX_COUNTER_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);

		reg = PPE_VPORT_TX_DROP_CNT_TBL_ADDR + PPE_VPORT_TX_DROP_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	for (i = 0; i < PPE_QUEUE_TX_COUNTER_TBL_ENTRIES; i++) {
		reg = PPE_QUEUE_TX_COUNTER_TBL_ADDR + PPE_QUEUE_TX_COUNTER_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	ppe_tbl_pkt_cnt_clear(ppe_dev, PPE_EPE_DBG_IN_CNT_ADDR, PPE_PKT_CNT_SIZE_1WORD);
	ppe_tbl_pkt_cnt_clear(ppe_dev, PPE_EPE_DBG_OUT_CNT_ADDR, PPE_PKT_CNT_SIZE_1WORD);

	for (i = 0; i < PPE_DROP_CPU_CNT_TBL_ENTRIES; i++) {
		reg = PPE_DROP_CPU_CNT_TBL_ADDR + PPE_DROP_CPU_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_3WORD);
	}

	for (i = 0; i < PPE_PORT_RX_CNT_TBL_ENTRIES; i++) {
		reg = PPE_PORT_RX_CNT_TBL_ADDR + PPE_PORT_RX_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_5WORD);
	}

	for (i = 0; i < PPE_PHY_PORT_RX_CNT_TBL_ENTRIES; i++) {
		reg = PPE_PHY_PORT_RX_CNT_TBL_ADDR + PPE_PHY_PORT_RX_CNT_TBL_INC * i;
		ppe_tbl_pkt_cnt_clear(ppe_dev, reg, PPE_PKT_CNT_SIZE_5WORD);
	}

	return count;
}

static const struct file_operations ppe_debugfs_packet_counter_fops = {
	.owner   = THIS_MODULE,
	.open    = ppe_packet_counter_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write   = ppe_packet_counter_clear,
};

void ppe_debugfs_setup(struct ppe_device *ppe_dev)
{
	int ret;

	ppe_dev->debugfs_root = debugfs_create_dir("ppe", NULL);
	debugfs_create_file("packet_counters", 0444,
			    ppe_dev->debugfs_root,
			    ppe_dev,
			    &ppe_debugfs_packet_counter_fops);

	if (!ppe_dev->debugfs_root) {
		dev_err(ppe_dev->dev, "Error in PPE debugfs setup\n");
		return;
	}

	ret = edma_debugfs_setup(ppe_dev);
	if (ret) {
		dev_err(ppe_dev->dev, "Error in EDMA debugfs setup API. ret: %d\n", ret);
		debugfs_remove_recursive(ppe_dev->debugfs_root);
		ppe_dev->debugfs_root = NULL;
	}
}

void ppe_debugfs_teardown(struct ppe_device *ppe_dev)
{
	edma_debugfs_teardown();
	debugfs_remove_recursive(ppe_dev->debugfs_root);
	ppe_dev->debugfs_root = NULL;
}
