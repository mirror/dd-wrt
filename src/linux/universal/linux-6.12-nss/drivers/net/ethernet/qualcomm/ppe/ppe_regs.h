/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* PPE hardware register and table declarations. */
#ifndef __PPE_REGS_H__
#define __PPE_REGS_H__

#include <linux/bitfield.h>

/* PPE port mux select control register */
#define PPE_PORT_MUX_CTRL_ADDR			0x10
#define PPE_PORT6_SEL_XGMAC			BIT(13)
#define PPE_PORT5_SEL_XGMAC			BIT(12)
#define PPE_PORT4_SEL_XGMAC			BIT(11)
#define PPE_PORT3_SEL_XGMAC			BIT(10)
#define PPE_PORT2_SEL_XGMAC			BIT(9)
#define PPE_PORT1_SEL_XGMAC			BIT(8)
#define PPE_PORT5_SEL_PCS1			BIT(4)
#define PPE_PORT_SEL_XGMAC(x)			(BIT(8) << ((x) - 1))

/* PPE port LPI enable register */
#define PPE_LPI_EN_ADDR				0x400
#define PPE_LPI_PORT1_EN			BIT(0)
#define PPE_LPI_PORT2_EN			BIT(1)
#define PPE_LPI_PORT3_EN			BIT(2)
#define PPE_LPI_PORT4_EN			BIT(3)
#define PPE_LPI_PORT5_EN			BIT(4)
#define PPE_LPI_PORT6_EN			BIT(5)
#define PPE_LPI_PORT_EN(x)			(BIT(0) << ((x) - 1))

/* PPE scheduler configurations for buffer manager block. */
#define PPE_BM_SCH_CTRL_ADDR			0xb000
#define PPE_BM_SCH_CTRL_INC			4
#define PPE_BM_SCH_CTRL_SCH_DEPTH		GENMASK(7, 0)
#define PPE_BM_SCH_CTRL_SCH_OFFSET		GENMASK(14, 8)
#define PPE_BM_SCH_CTRL_SCH_EN			BIT(31)

/* PPE drop counters. */
#define PPE_DROP_CNT_TBL_ADDR			0xb024
#define PPE_DROP_CNT_TBL_ENTRIES		8
#define PPE_DROP_CNT_TBL_INC			4

/* BM port drop counters. */
#define PPE_DROP_STAT_TBL_ADDR			0xe000
#define PPE_DROP_STAT_TBL_ENTRIES		30
#define PPE_DROP_STAT_TBL_INC			0x10

#define PPE_EPE_DBG_IN_CNT_ADDR			0x26054
#define PPE_EPE_DBG_OUT_CNT_ADDR		0x26070

/* Egress VLAN counters. */
#define PPE_EG_VSI_COUNTER_TBL_ADDR		0x41000
#define PPE_EG_VSI_COUNTER_TBL_ENTRIES		64
#define PPE_EG_VSI_COUNTER_TBL_INC		0x10

/* Port TX counters. */
#define PPE_PORT_TX_COUNTER_TBL_ADDR		0x45000
#define PPE_PORT_TX_COUNTER_TBL_ENTRIES		8
#define PPE_PORT_TX_COUNTER_TBL_INC		0x10

/* Virtual port TX counters. */
#define PPE_VPORT_TX_COUNTER_TBL_ADDR		0x47000
#define PPE_VPORT_TX_COUNTER_TBL_ENTRIES	256
#define PPE_VPORT_TX_COUNTER_TBL_INC		0x10

/* Queue counters. */
#define PPE_QUEUE_TX_COUNTER_TBL_ADDR		0x4a000
#define PPE_QUEUE_TX_COUNTER_TBL_ENTRIES	300
#define PPE_QUEUE_TX_COUNTER_TBL_INC		0x10

/* RSS settings are to calculate the random RSS hash value generated during
 * packet receive to ARM cores. This hash is then used to generate the queue
 * offset used to determine the queue used to transmit the packet to ARM cores.
 */
#define PPE_RSS_HASH_MASK_ADDR			0xb4318
#define PPE_RSS_HASH_MASK_HASH_MASK		GENMASK(20, 0)
#define PPE_RSS_HASH_MASK_FRAGMENT		BIT(28)

#define PPE_RSS_HASH_SEED_ADDR			0xb431c
#define PPE_RSS_HASH_SEED_VAL			GENMASK(31, 0)

#define PPE_RSS_HASH_MIX_ADDR			0xb4320
#define PPE_RSS_HASH_MIX_ENTRIES		11
#define PPE_RSS_HASH_MIX_INC			4
#define PPE_RSS_HASH_MIX_VAL			GENMASK(4, 0)

#define PPE_RSS_HASH_FIN_ADDR			0xb4350
#define PPE_RSS_HASH_FIN_ENTRIES		5
#define PPE_RSS_HASH_FIN_INC			4
#define PPE_RSS_HASH_FIN_INNER			GENMASK(4, 0)
#define PPE_RSS_HASH_FIN_OUTER			GENMASK(9, 5)

#define PPE_RSS_HASH_MASK_IPV4_ADDR		0xb4380
#define PPE_RSS_HASH_MASK_IPV4_HASH_MASK	GENMASK(20, 0)
#define PPE_RSS_HASH_MASK_IPV4_FRAGMENT		BIT(28)

#define PPE_RSS_HASH_SEED_IPV4_ADDR		0xb4384
#define PPE_RSS_HASH_SEED_IPV4_VAL		GENMASK(31, 0)

#define PPE_RSS_HASH_MIX_IPV4_ADDR		0xb4390
#define PPE_RSS_HASH_MIX_IPV4_ENTRIES		5
#define PPE_RSS_HASH_MIX_IPV4_INC		4
#define PPE_RSS_HASH_MIX_IPV4_VAL		GENMASK(4, 0)

#define PPE_RSS_HASH_FIN_IPV4_ADDR		0xb43b0
#define PPE_RSS_HASH_FIN_IPV4_ENTRIES		5
#define PPE_RSS_HASH_FIN_IPV4_INC		4
#define PPE_RSS_HASH_FIN_IPV4_INNER		GENMASK(4, 0)
#define PPE_RSS_HASH_FIN_IPV4_OUTER		GENMASK(9, 5)

#define PPE_BM_SCH_CFG_TBL_ADDR			0xc000
#define PPE_BM_SCH_CFG_TBL_ENTRIES		128
#define PPE_BM_SCH_CFG_TBL_INC			0x10
#define PPE_BM_SCH_CFG_TBL_PORT_NUM		GENMASK(3, 0)
#define PPE_BM_SCH_CFG_TBL_DIR			BIT(4)
#define PPE_BM_SCH_CFG_TBL_VALID		BIT(5)
#define PPE_BM_SCH_CFG_TBL_SECOND_PORT_VALID	BIT(6)
#define PPE_BM_SCH_CFG_TBL_SECOND_PORT		GENMASK(11, 8)

/* PPE service code configuration for the ingress direction functions,
 * including bypass configuration for relevant PPE switch core functions
 * such as flow entry lookup bypass.
 */
#define PPE_SERVICE_TBL_ADDR			0x15000
#define PPE_SERVICE_TBL_ENTRIES			256
#define PPE_SERVICE_TBL_INC			0x10
#define PPE_SERVICE_W0_BYPASS_BITMAP		GENMASK(31, 0)
#define PPE_SERVICE_W1_RX_CNT_EN		BIT(0)

#define PPE_SERVICE_SET_BYPASS_BITMAP(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_SERVICE_W0_BYPASS_BITMAP)
#define PPE_SERVICE_SET_RX_CNT_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_SERVICE_W1_RX_CNT_EN)

/* PPE port egress VLAN configurations. */
#define PPE_PORT_EG_VLAN_TBL_ADDR		0x20020
#define PPE_PORT_EG_VLAN_TBL_ENTRIES		8
#define PPE_PORT_EG_VLAN_TBL_INC		4
#define PPE_PORT_EG_VLAN_TBL_VLAN_TYPE		BIT(0)
#define PPE_PORT_EG_VLAN_TBL_CTAG_MODE		GENMASK(2, 1)
#define PPE_PORT_EG_VLAN_TBL_STAG_MODE		GENMASK(4, 3)
#define PPE_PORT_EG_VLAN_TBL_VSI_TAG_MODE_EN	BIT(5)
#define PPE_PORT_EG_VLAN_TBL_PCP_PROP_CMD	BIT(6)
#define PPE_PORT_EG_VLAN_TBL_DEI_PROP_CMD	BIT(7)
#define PPE_PORT_EG_VLAN_TBL_TX_COUNTING_EN	BIT(8)

/* PPE queue counters enable/disable control. */
#define PPE_EG_BRIDGE_CONFIG_ADDR		0x20044
#define PPE_EG_BRIDGE_CONFIG_QUEUE_CNT_EN	BIT(2)

/* PPE service code configuration on the egress direction. */
#define PPE_EG_SERVICE_TBL_ADDR			0x43000
#define PPE_EG_SERVICE_TBL_ENTRIES		256
#define PPE_EG_SERVICE_TBL_INC			0x10
#define PPE_EG_SERVICE_W0_UPDATE_ACTION		GENMASK(31, 0)
#define PPE_EG_SERVICE_W1_NEXT_SERVCODE		GENMASK(7, 0)
#define PPE_EG_SERVICE_W1_HW_SERVICE		GENMASK(13, 8)
#define PPE_EG_SERVICE_W1_OFFSET_SEL		BIT(14)
#define PPE_EG_SERVICE_W1_TX_CNT_EN		BIT(15)

#define PPE_EG_SERVICE_SET_UPDATE_ACTION(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_EG_SERVICE_W0_UPDATE_ACTION)
#define PPE_EG_SERVICE_SET_NEXT_SERVCODE(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_EG_SERVICE_W1_NEXT_SERVCODE)
#define PPE_EG_SERVICE_SET_HW_SERVICE(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_EG_SERVICE_W1_HW_SERVICE)
#define PPE_EG_SERVICE_SET_OFFSET_SEL(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_EG_SERVICE_W1_OFFSET_SEL)
#define PPE_EG_SERVICE_SET_TX_CNT_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_EG_SERVICE_W1_TX_CNT_EN)

/* PPE port bridge configuration */
#define PPE_PORT_BRIDGE_CTRL_ADDR		0x60300
#define PPE_PORT_BRIDGE_CTRL_ENTRIES		8
#define PPE_PORT_BRIDGE_CTRL_INC		4
#define PPE_PORT_BRIDGE_NEW_LRN_EN		BIT(0)
#define PPE_PORT_BRIDGE_STA_MOVE_LRN_EN		BIT(3)
#define PPE_PORT_BRIDGE_TXMAC_EN		BIT(16)

/* PPE port control configurations for the traffic to the multicast queues. */
#define PPE_MC_MTU_CTRL_TBL_ADDR		0x60a00
#define PPE_MC_MTU_CTRL_TBL_ENTRIES		8
#define PPE_MC_MTU_CTRL_TBL_INC			4
#define PPE_MC_MTU_CTRL_TBL_MTU			GENMASK(13, 0)
#define PPE_MC_MTU_CTRL_TBL_MTU_CMD		GENMASK(15, 14)
#define PPE_MC_MTU_CTRL_TBL_TX_CNT_EN		BIT(16)

/* PPE VSI configurations */
#define PPE_VSI_TBL_ADDR			0x63800
#define PPE_VSI_TBL_ENTRIES			64
#define PPE_VSI_TBL_INC				0x10
#define PPE_VSI_W0_MEMBER_PORT_BITMAP		GENMASK(7, 0)
#define PPE_VSI_W0_UUC_BITMAP			GENMASK(15, 8)
#define PPE_VSI_W0_UMC_BITMAP			GENMASK(23, 16)
#define PPE_VSI_W0_BC_BITMAP			GENMASK(31, 24)
#define PPE_VSI_W1_NEW_ADDR_LRN_EN		BIT(0)
#define PPE_VSI_W1_NEW_ADDR_FWD_CMD		GENMASK(2, 1)
#define PPE_VSI_W1_STATION_MOVE_LRN_EN		BIT(3)
#define PPE_VSI_W1_STATION_MOVE_FWD_CMD		GENMASK(5, 4)

#define PPE_VSI_SET_MEMBER_PORT_BITMAP(tbl_cfg, value)		\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_VSI_W0_MEMBER_PORT_BITMAP)
#define PPE_VSI_SET_UUC_BITMAP(tbl_cfg, value)			\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_VSI_W0_UUC_BITMAP)
#define PPE_VSI_SET_UMC_BITMAP(tbl_cfg, value)			\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_VSI_W0_UMC_BITMAP)
#define PPE_VSI_SET_BC_BITMAP(tbl_cfg, value)			\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_VSI_W0_BC_BITMAP)
#define PPE_VSI_SET_NEW_ADDR_LRN_EN(tbl_cfg, value)		\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_VSI_W1_NEW_ADDR_LRN_EN)
#define PPE_VSI_SET_NEW_ADDR_FWD_CMD(tbl_cfg, value)		\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_VSI_W1_NEW_ADDR_FWD_CMD)
#define PPE_VSI_SET_STATION_MOVE_LRN_EN(tbl_cfg, value)		\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_VSI_W1_STATION_MOVE_LRN_EN)
#define PPE_VSI_SET_STATION_MOVE_FWD_CMD(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_VSI_W1_STATION_MOVE_FWD_CMD)

/* PPE port control configurations for the traffic to the unicast queues. */
#define PPE_MRU_MTU_CTRL_TBL_ADDR		0x65000
#define PPE_MRU_MTU_CTRL_TBL_ENTRIES		256
#define PPE_MRU_MTU_CTRL_TBL_INC		0x10
#define PPE_MRU_MTU_CTRL_W0_MRU			GENMASK(13, 0)
#define PPE_MRU_MTU_CTRL_W0_MRU_CMD		GENMASK(15, 14)
#define PPE_MRU_MTU_CTRL_W0_MTU			GENMASK(29, 16)
#define PPE_MRU_MTU_CTRL_W0_MTU_CMD		GENMASK(31, 30)
#define PPE_MRU_MTU_CTRL_W1_RX_CNT_EN		BIT(0)
#define PPE_MRU_MTU_CTRL_W1_TX_CNT_EN		BIT(1)
#define PPE_MRU_MTU_CTRL_W1_SRC_PROFILE		GENMASK(3, 2)
#define PPE_MRU_MTU_CTRL_W1_INNER_PREC_LOW	BIT(31)
#define PPE_MRU_MTU_CTRL_W2_INNER_PREC_HIGH	GENMASK(1, 0)

#define PPE_MRU_MTU_CTRL_SET_MRU(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_MRU_MTU_CTRL_W0_MRU)
#define PPE_MRU_MTU_CTRL_SET_MRU_CMD(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_MRU_MTU_CTRL_W0_MRU_CMD)
#define PPE_MRU_MTU_CTRL_SET_MTU(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_MRU_MTU_CTRL_W0_MTU)
#define PPE_MRU_MTU_CTRL_SET_MTU_CMD(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_MRU_MTU_CTRL_W0_MTU_CMD)
#define PPE_MRU_MTU_CTRL_SET_RX_CNT_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_MRU_MTU_CTRL_W1_RX_CNT_EN)
#define PPE_MRU_MTU_CTRL_SET_TX_CNT_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_MRU_MTU_CTRL_W1_TX_CNT_EN)

/* PPE service code configuration for destination port and counter. */
#define PPE_IN_L2_SERVICE_TBL_ADDR		0x66000
#define PPE_IN_L2_SERVICE_TBL_ENTRIES		256
#define PPE_IN_L2_SERVICE_TBL_INC		0x10
#define PPE_IN_L2_SERVICE_TBL_DST_PORT_ID_VALID	BIT(0)
#define PPE_IN_L2_SERVICE_TBL_DST_PORT_ID	GENMASK(4, 1)
#define PPE_IN_L2_SERVICE_TBL_DST_DIRECTION	BIT(5)
#define PPE_IN_L2_SERVICE_TBL_DST_BYPASS_BITMAP	GENMASK(29, 6)
#define PPE_IN_L2_SERVICE_TBL_RX_CNT_EN		BIT(30)
#define PPE_IN_L2_SERVICE_TBL_TX_CNT_EN		BIT(31)

/* L2 Port configurations */
#define PPE_L2_VP_PORT_TBL_ADDR			0x98000
#define PPE_L2_VP_PORT_TBL_ENTRIES		256
#define PPE_L2_VP_PORT_TBL_INC			0x10
#define PPE_L2_VP_PORT_W0_INVALID_VSI_FWD_EN	BIT(0)
#define PPE_L2_VP_PORT_W0_DST_INFO		GENMASK(9, 2)

#define PPE_L2_PORT_SET_INVALID_VSI_FWD_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_L2_VP_PORT_W0_INVALID_VSI_FWD_EN)
#define PPE_L2_PORT_SET_DST_INFO(tbl_cfg, value)		\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_L2_VP_PORT_W0_DST_INFO)

/* Port RX and RX drop counters. */
#define PPE_PORT_RX_CNT_TBL_ADDR		0x150000
#define PPE_PORT_RX_CNT_TBL_ENTRIES		256
#define PPE_PORT_RX_CNT_TBL_INC			0x20

/* Physical port RX and RX drop counters. */
#define PPE_PHY_PORT_RX_CNT_TBL_ADDR		0x156000
#define PPE_PHY_PORT_RX_CNT_TBL_ENTRIES		8
#define PPE_PHY_PORT_RX_CNT_TBL_INC		0x20

/* Counters for the packet to CPU port. */
#define PPE_DROP_CPU_CNT_TBL_ADDR		0x160000
#define PPE_DROP_CPU_CNT_TBL_ENTRIES		1280
#define PPE_DROP_CPU_CNT_TBL_INC		0x10

/* VLAN counters. */
#define PPE_VLAN_CNT_TBL_ADDR			0x178000
#define PPE_VLAN_CNT_TBL_ENTRIES		64
#define PPE_VLAN_CNT_TBL_INC			0x10

/* PPE L2 counters. */
#define PPE_PRE_L2_CNT_TBL_ADDR			0x17c000
#define PPE_PRE_L2_CNT_TBL_ENTRIES		64
#define PPE_PRE_L2_CNT_TBL_INC			0x20

/* Port TX drop counters. */
#define PPE_PORT_TX_DROP_CNT_TBL_ADDR		0x17d000
#define PPE_PORT_TX_DROP_CNT_TBL_ENTRIES	8
#define PPE_PORT_TX_DROP_CNT_TBL_INC		0x10

/* Virtual port TX counters. */
#define PPE_VPORT_TX_DROP_CNT_TBL_ADDR		0x17e000
#define PPE_VPORT_TX_DROP_CNT_TBL_ENTRIES	256
#define PPE_VPORT_TX_DROP_CNT_TBL_INC		0x10

/* Counters for the tunnel packet. */
#define PPE_TPR_PKT_CNT_TBL_ADDR		0x1d0080
#define PPE_TPR_PKT_CNT_TBL_ENTRIES		8
#define PPE_TPR_PKT_CNT_TBL_INC			4

/* Counters for the all packet received. */
#define PPE_IPR_PKT_CNT_TBL_ADDR		0x1e0080
#define PPE_IPR_PKT_CNT_TBL_ENTRIES		8
#define PPE_IPR_PKT_CNT_TBL_INC			4

/* PPE service code configuration for the tunnel packet. */
#define PPE_TL_SERVICE_TBL_ADDR			0x306000
#define PPE_TL_SERVICE_TBL_ENTRIES		256
#define PPE_TL_SERVICE_TBL_INC			4
#define PPE_TL_SERVICE_TBL_BYPASS_BITMAP	GENMASK(31, 0)

/* Port scheduler global config. */
#define PPE_PSCH_SCH_DEPTH_CFG_ADDR		0x400000
#define PPE_PSCH_SCH_DEPTH_CFG_INC		4
#define PPE_PSCH_SCH_DEPTH_CFG_SCH_DEPTH	GENMASK(7, 0)

/* PPE queue level scheduler configurations. */
#define PPE_L0_FLOW_MAP_TBL_ADDR		0x402000
#define PPE_L0_FLOW_MAP_TBL_ENTRIES		300
#define PPE_L0_FLOW_MAP_TBL_INC			0x10
#define PPE_L0_FLOW_MAP_TBL_FLOW_ID		GENMASK(5, 0)
#define PPE_L0_FLOW_MAP_TBL_C_PRI		GENMASK(8, 6)
#define PPE_L0_FLOW_MAP_TBL_E_PRI		GENMASK(11, 9)
#define PPE_L0_FLOW_MAP_TBL_C_NODE_WT		GENMASK(21, 12)
#define PPE_L0_FLOW_MAP_TBL_E_NODE_WT		GENMASK(31, 22)

#define PPE_L0_C_FLOW_CFG_TBL_ADDR		0x404000
#define PPE_L0_C_FLOW_CFG_TBL_ENTRIES		512
#define PPE_L0_C_FLOW_CFG_TBL_INC		0x10
#define PPE_L0_C_FLOW_CFG_TBL_NODE_ID		GENMASK(7, 0)
#define PPE_L0_C_FLOW_CFG_TBL_NODE_CREDIT_UNIT	BIT(8)

#define PPE_L0_E_FLOW_CFG_TBL_ADDR		0x406000
#define PPE_L0_E_FLOW_CFG_TBL_ENTRIES		512
#define PPE_L0_E_FLOW_CFG_TBL_INC		0x10
#define PPE_L0_E_FLOW_CFG_TBL_NODE_ID		GENMASK(7, 0)
#define PPE_L0_E_FLOW_CFG_TBL_NODE_CREDIT_UNIT	BIT(8)

#define PPE_L0_FLOW_PORT_MAP_TBL_ADDR		0x408000
#define PPE_L0_FLOW_PORT_MAP_TBL_ENTRIES	300
#define PPE_L0_FLOW_PORT_MAP_TBL_INC		0x10
#define PPE_L0_FLOW_PORT_MAP_TBL_PORT_NUM	GENMASK(3, 0)

#define PPE_L0_COMP_CFG_TBL_ADDR		0x428000
#define PPE_L0_COMP_CFG_TBL_ENTRIES		300
#define PPE_L0_COMP_CFG_TBL_INC			0x10
#define PPE_L0_COMP_CFG_TBL_SHAPER_METER_LEN	GENMASK(1, 0)
#define PPE_L0_COMP_CFG_TBL_NODE_METER_LEN	GENMASK(3, 2)

/* PPE queue to Ethernet DMA ring mapping table. */
#define PPE_RING_Q_MAP_TBL_ADDR			0x42a000
#define PPE_RING_Q_MAP_TBL_ENTRIES		24
#define PPE_RING_Q_MAP_TBL_INC			0x40

/* Table addresses for per-queue dequeue setting. */
#define PPE_DEQ_OPR_TBL_ADDR			0x430000
#define PPE_DEQ_OPR_TBL_ENTRIES			300
#define PPE_DEQ_OPR_TBL_INC			0x10
#define PPE_DEQ_OPR_TBL_DEQ_DISABLE		BIT(0)

/* PPE flow level scheduler configurations. */
#define PPE_L1_FLOW_MAP_TBL_ADDR		0x440000
#define PPE_L1_FLOW_MAP_TBL_ENTRIES		64
#define PPE_L1_FLOW_MAP_TBL_INC			0x10
#define PPE_L1_FLOW_MAP_TBL_FLOW_ID		GENMASK(3, 0)
#define PPE_L1_FLOW_MAP_TBL_C_PRI		GENMASK(6, 4)
#define PPE_L1_FLOW_MAP_TBL_E_PRI		GENMASK(9, 7)
#define PPE_L1_FLOW_MAP_TBL_C_NODE_WT		GENMASK(19, 10)
#define PPE_L1_FLOW_MAP_TBL_E_NODE_WT		GENMASK(29, 20)

#define PPE_L1_C_FLOW_CFG_TBL_ADDR		0x442000
#define PPE_L1_C_FLOW_CFG_TBL_ENTRIES		64
#define PPE_L1_C_FLOW_CFG_TBL_INC		0x10
#define PPE_L1_C_FLOW_CFG_TBL_NODE_ID		GENMASK(5, 0)
#define PPE_L1_C_FLOW_CFG_TBL_NODE_CREDIT_UNIT	BIT(6)

#define PPE_L1_E_FLOW_CFG_TBL_ADDR		0x444000
#define PPE_L1_E_FLOW_CFG_TBL_ENTRIES		64
#define PPE_L1_E_FLOW_CFG_TBL_INC		0x10
#define PPE_L1_E_FLOW_CFG_TBL_NODE_ID		GENMASK(5, 0)
#define PPE_L1_E_FLOW_CFG_TBL_NODE_CREDIT_UNIT	BIT(6)

#define PPE_L1_FLOW_PORT_MAP_TBL_ADDR		0x446000
#define PPE_L1_FLOW_PORT_MAP_TBL_ENTRIES	64
#define PPE_L1_FLOW_PORT_MAP_TBL_INC		0x10
#define PPE_L1_FLOW_PORT_MAP_TBL_PORT_NUM	GENMASK(3, 0)

#define PPE_L1_COMP_CFG_TBL_ADDR		0x46a000
#define PPE_L1_COMP_CFG_TBL_ENTRIES		64
#define PPE_L1_COMP_CFG_TBL_INC			0x10
#define PPE_L1_COMP_CFG_TBL_SHAPER_METER_LEN	GENMASK(1, 0)
#define PPE_L1_COMP_CFG_TBL_NODE_METER_LEN	GENMASK(3, 2)

/* PPE port scheduler configurations for egress. */
#define PPE_PSCH_SCH_CFG_TBL_ADDR		0x47a000
#define PPE_PSCH_SCH_CFG_TBL_ENTRIES		128
#define PPE_PSCH_SCH_CFG_TBL_INC		0x10
#define PPE_PSCH_SCH_CFG_TBL_DES_PORT		GENMASK(3, 0)
#define PPE_PSCH_SCH_CFG_TBL_ENS_PORT		GENMASK(7, 4)
#define PPE_PSCH_SCH_CFG_TBL_ENS_PORT_BITMAP	GENMASK(15, 8)
#define PPE_PSCH_SCH_CFG_TBL_DES_SECOND_PORT_EN	BIT(16)
#define PPE_PSCH_SCH_CFG_TBL_DES_SECOND_PORT	GENMASK(20, 17)

/* There are 15 BM ports and 4 BM groups supported by PPE.
 * BM port (0-7) is for EDMA port 0, BM port (8-13) is for
 * PPE physical port 1-6 and BM port 14 is for EIP port.
 */
#define PPE_BM_PORT_FC_MODE_ADDR		0x600100
#define PPE_BM_PORT_FC_MODE_ENTRIES		15
#define PPE_BM_PORT_FC_MODE_INC			0x4
#define PPE_BM_PORT_FC_MODE_EN			BIT(0)

#define PPE_BM_PORT_GROUP_ID_ADDR		0x600180
#define PPE_BM_PORT_GROUP_ID_ENTRIES		15
#define PPE_BM_PORT_GROUP_ID_INC		0x4
#define PPE_BM_PORT_GROUP_ID_SHARED_GROUP_ID	GENMASK(1, 0)

/* Counters for PPE buffers used for packets cached. */
#define PPE_BM_USED_CNT_TBL_ADDR		0x6001c0
#define PPE_BM_USED_CNT_TBL_ENTRIES		15
#define PPE_BM_USED_CNT_TBL_INC			0x4
#define PPE_BM_USED_CNT_VAL			GENMASK(10, 0)

/* Counters for PPE buffers used for packets received after pause frame sent. */
#define PPE_BM_REACT_CNT_TBL_ADDR		0x600240
#define PPE_BM_REACT_CNT_TBL_ENTRIES		15
#define PPE_BM_REACT_CNT_TBL_INC		0x4
#define PPE_BM_REACT_CNT_VAL			GENMASK(8, 0)

#define PPE_BM_SHARED_GROUP_CFG_ADDR		0x600290
#define PPE_BM_SHARED_GROUP_CFG_ENTRIES		4
#define PPE_BM_SHARED_GROUP_CFG_INC		0x4
#define PPE_BM_SHARED_GROUP_CFG_SHARED_LIMIT	GENMASK(10, 0)

#define PPE_BM_PORT_FC_CFG_TBL_ADDR		0x601000
#define PPE_BM_PORT_FC_CFG_TBL_ENTRIES		15
#define PPE_BM_PORT_FC_CFG_TBL_INC		0x10
#define PPE_BM_PORT_FC_W0_REACT_LIMIT		GENMASK(8, 0)
#define PPE_BM_PORT_FC_W0_RESUME_THRESHOLD	GENMASK(17, 9)
#define PPE_BM_PORT_FC_W0_RESUME_OFFSET		GENMASK(28, 18)
#define PPE_BM_PORT_FC_W0_CEILING_LOW		GENMASK(31, 29)
#define PPE_BM_PORT_FC_W1_CEILING_HIGH		GENMASK(7, 0)
#define PPE_BM_PORT_FC_W1_WEIGHT		GENMASK(10, 8)
#define PPE_BM_PORT_FC_W1_DYNAMIC		BIT(11)
#define PPE_BM_PORT_FC_W1_PRE_ALLOC		GENMASK(22, 12)

#define PPE_BM_PORT_FC_SET_REACT_LIMIT(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_BM_PORT_FC_W0_REACT_LIMIT)
#define PPE_BM_PORT_FC_SET_RESUME_THRESHOLD(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_BM_PORT_FC_W0_RESUME_THRESHOLD)
#define PPE_BM_PORT_FC_SET_RESUME_OFFSET(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_BM_PORT_FC_W0_RESUME_OFFSET)
#define PPE_BM_PORT_FC_SET_CEILING_LOW(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_BM_PORT_FC_W0_CEILING_LOW)
#define PPE_BM_PORT_FC_SET_CEILING_HIGH(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_BM_PORT_FC_W1_CEILING_HIGH)
#define PPE_BM_PORT_FC_SET_WEIGHT(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_BM_PORT_FC_W1_WEIGHT)
#define PPE_BM_PORT_FC_SET_DYNAMIC(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_BM_PORT_FC_W1_DYNAMIC)
#define PPE_BM_PORT_FC_SET_PRE_ALLOC(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_BM_PORT_FC_W1_PRE_ALLOC)

/* The queue base configurations based on destination port,
 * service code or CPU code.
 */
#define PPE_UCAST_QUEUE_MAP_TBL_ADDR		0x810000
#define PPE_UCAST_QUEUE_MAP_TBL_ENTRIES		3072
#define PPE_UCAST_QUEUE_MAP_TBL_INC		0x10
#define PPE_UCAST_QUEUE_MAP_TBL_PROFILE_ID	GENMASK(3, 0)
#define PPE_UCAST_QUEUE_MAP_TBL_QUEUE_ID	GENMASK(11, 4)

/* The queue offset configurations based on RSS hash value. */
#define PPE_UCAST_HASH_MAP_TBL_ADDR		0x830000
#define PPE_UCAST_HASH_MAP_TBL_ENTRIES		4096
#define PPE_UCAST_HASH_MAP_TBL_INC		0x10
#define PPE_UCAST_HASH_MAP_TBL_HASH		GENMASK(7, 0)

/* The queue offset configurations based on PPE internal priority. */
#define PPE_UCAST_PRIORITY_MAP_TBL_ADDR		0x842000
#define PPE_UCAST_PRIORITY_MAP_TBL_ENTRIES	256
#define PPE_UCAST_PRIORITY_MAP_TBL_INC		0x10
#define PPE_UCAST_PRIORITY_MAP_TBL_CLASS	GENMASK(3, 0)

/* PPE unicast queue (0-255) configurations. */
#define PPE_AC_UNICAST_QUEUE_CFG_TBL_ADDR	0x848000
#define PPE_AC_UNICAST_QUEUE_CFG_TBL_ENTRIES	256
#define PPE_AC_UNICAST_QUEUE_CFG_TBL_INC	0x10
#define PPE_AC_UNICAST_QUEUE_CFG_W0_EN		BIT(0)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_WRED_EN	BIT(1)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_FC_EN	BIT(2)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_CLR_AWARE	BIT(3)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_GRP_ID	GENMASK(5, 4)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_PRE_LIMIT	GENMASK(16, 6)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_DYNAMIC	BIT(17)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_WEIGHT	GENMASK(20, 18)
#define PPE_AC_UNICAST_QUEUE_CFG_W0_THRESHOLD	GENMASK(31, 21)
#define PPE_AC_UNICAST_QUEUE_CFG_W3_GRN_RESUME	GENMASK(23, 13)

#define PPE_AC_UNICAST_QUEUE_SET_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_UNICAST_QUEUE_CFG_W0_EN)
#define PPE_AC_UNICAST_QUEUE_SET_GRP_ID(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_UNICAST_QUEUE_CFG_W0_GRP_ID)
#define PPE_AC_UNICAST_QUEUE_SET_PRE_LIMIT(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_UNICAST_QUEUE_CFG_W0_PRE_LIMIT)
#define PPE_AC_UNICAST_QUEUE_SET_DYNAMIC(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_UNICAST_QUEUE_CFG_W0_DYNAMIC)
#define PPE_AC_UNICAST_QUEUE_SET_WEIGHT(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_UNICAST_QUEUE_CFG_W0_WEIGHT)
#define PPE_AC_UNICAST_QUEUE_SET_THRESHOLD(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_UNICAST_QUEUE_CFG_W0_THRESHOLD)
#define PPE_AC_UNICAST_QUEUE_SET_GRN_RESUME(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x3, value, PPE_AC_UNICAST_QUEUE_CFG_W3_GRN_RESUME)

/* PPE multicast queue (256-299) configurations. */
#define PPE_AC_MULTICAST_QUEUE_CFG_TBL_ADDR	0x84a000
#define PPE_AC_MULTICAST_QUEUE_CFG_TBL_ENTRIES	44
#define PPE_AC_MULTICAST_QUEUE_CFG_TBL_INC	0x10
#define PPE_AC_MULTICAST_QUEUE_CFG_W0_EN	BIT(0)
#define PPE_AC_MULTICAST_QUEUE_CFG_W0_FC_EN	BIT(1)
#define PPE_AC_MULTICAST_QUEUE_CFG_W0_CLR_AWARE	BIT(2)
#define PPE_AC_MULTICAST_QUEUE_CFG_W0_GRP_ID	GENMASK(4, 3)
#define PPE_AC_MULTICAST_QUEUE_CFG_W0_PRE_LIMIT	GENMASK(15, 5)
#define PPE_AC_MULTICAST_QUEUE_CFG_W0_THRESHOLD	GENMASK(26, 16)
#define PPE_AC_MULTICAST_QUEUE_CFG_W2_RESUME	GENMASK(17, 7)

#define PPE_AC_MULTICAST_QUEUE_SET_EN(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_MULTICAST_QUEUE_CFG_W0_EN)
#define PPE_AC_MULTICAST_QUEUE_SET_GRN_GRP_ID(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_MULTICAST_QUEUE_CFG_W0_GRP_ID)
#define PPE_AC_MULTICAST_QUEUE_SET_GRN_PRE_LIMIT(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_MULTICAST_QUEUE_CFG_W0_PRE_LIMIT)
#define PPE_AC_MULTICAST_QUEUE_SET_GRN_THRESHOLD(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)tbl_cfg, value, PPE_AC_MULTICAST_QUEUE_CFG_W0_THRESHOLD)
#define PPE_AC_MULTICAST_QUEUE_SET_GRN_RESUME(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x2, value, PPE_AC_MULTICAST_QUEUE_CFG_W2_RESUME)

/* PPE admission control group (0-3) configurations */
#define PPE_AC_GRP_CFG_TBL_ADDR			0x84c000
#define PPE_AC_GRP_CFG_TBL_ENTRIES		0x4
#define PPE_AC_GRP_CFG_TBL_INC			0x10
#define PPE_AC_GRP_W0_AC_EN			BIT(0)
#define PPE_AC_GRP_W0_AC_FC_EN			BIT(1)
#define PPE_AC_GRP_W0_CLR_AWARE			BIT(2)
#define PPE_AC_GRP_W0_THRESHOLD_LOW		GENMASK(31, 25)
#define PPE_AC_GRP_W1_THRESHOLD_HIGH		GENMASK(3, 0)
#define PPE_AC_GRP_W1_BUF_LIMIT			GENMASK(14, 4)
#define PPE_AC_GRP_W2_RESUME_GRN		GENMASK(15, 5)
#define PPE_AC_GRP_W2_PRE_ALLOC			GENMASK(26, 16)

#define PPE_AC_GRP_SET_BUF_LIMIT(tbl_cfg, value)	\
	u32p_replace_bits((u32 *)(tbl_cfg) + 0x1, value, PPE_AC_GRP_W1_BUF_LIMIT)

/* Counters for packets handled by unicast queues (0-255). */
#define PPE_AC_UNICAST_QUEUE_CNT_TBL_ADDR	0x84e000
#define PPE_AC_UNICAST_QUEUE_CNT_TBL_ENTRIES	256
#define PPE_AC_UNICAST_QUEUE_CNT_TBL_INC	0x10
#define PPE_AC_UNICAST_QUEUE_CNT_TBL_PEND_CNT	GENMASK(12, 0)

/* Counters for packets handled by multicast queues (256-299). */
#define PPE_AC_MULTICAST_QUEUE_CNT_TBL_ADDR	0x852000
#define PPE_AC_MULTICAST_QUEUE_CNT_TBL_ENTRIES	44
#define PPE_AC_MULTICAST_QUEUE_CNT_TBL_INC	0x10
#define PPE_AC_MULTICAST_QUEUE_CNT_TBL_PEND_CNT	GENMASK(12, 0)

/* Table addresses for per-queue enqueue setting. */
#define PPE_ENQ_OPR_TBL_ADDR			0x85c000
#define PPE_ENQ_OPR_TBL_ENTRIES			300
#define PPE_ENQ_OPR_TBL_INC			0x10
#define PPE_ENQ_OPR_TBL_ENQ_DISABLE		BIT(0)

/* PPE GMAC and XGMAC register base address */
#define PPE_PORT_GMAC_ADDR(x)			(0x001000 + ((x) - 1) * 0x200)
#define PPE_PORT_XGMAC_ADDR(x)			(0x500000 + ((x) - 1) * 0x4000)

/* GMAC enable register */
#define GMAC_ENABLE_ADDR			0x0
#define GMAC_TXFCEN				BIT(6)
#define GMAC_RXFCEN				BIT(5)
#define GMAC_DUPLEX_FULL			BIT(4)
#define GMAC_TXEN				BIT(1)
#define GMAC_RXEN				BIT(0)

#define GMAC_TRXEN				\
	(GMAC_TXEN | GMAC_RXEN)
#define GMAC_ENABLE_ALL				\
	(GMAC_TXFCEN | GMAC_RXFCEN | GMAC_DUPLEX_FULL | GMAC_TXEN | GMAC_RXEN)

/* GMAC speed register */
#define GMAC_SPEED_ADDR				0x4
#define GMAC_SPEED_M				GENMASK(1, 0)
#define GMAC_SPEED_10				0
#define GMAC_SPEED_100				1
#define GMAC_SPEED_1000				2

/* GMAC MAC address register */
#define GMAC_GOL_ADDR0_ADDR			0x8
#define GMAC_ADDR_BYTE5				GENMASK(15, 8)
#define GMAC_ADDR_BYTE4				GENMASK(7, 0)

#define GMAC_GOL_ADDR1_ADDR			0xC
#define GMAC_ADDR_BYTE0				GENMASK(31, 24)
#define GMAC_ADDR_BYTE1				GENMASK(23, 16)
#define GMAC_ADDR_BYTE2				GENMASK(15, 8)
#define GMAC_ADDR_BYTE3				GENMASK(7, 0)

/* GMAC control register */
#define GMAC_CTRL_ADDR				0x18
#define GMAC_TX_THD_M				GENMASK(27, 24)
#define GMAC_MAXFRAME_SIZE_M			GENMASK(21, 8)
#define GMAC_CRS_SEL				BIT(6)

#define GMAC_CTRL_MASK				\
	(GMAC_TX_THD_M | GMAC_MAXFRAME_SIZE_M | GMAC_CRS_SEL)

/* GMAC debug control register */
#define GMAC_DBG_CTRL_ADDR			0x1c
#define GMAC_HIGH_IPG_M				GENMASK(15, 8)

/* GMAC jumbo size register */
#define GMAC_JUMBO_SIZE_ADDR			0x30
#define GMAC_JUMBO_SIZE_M			GENMASK(13, 0)

/* GMAC MIB control register */
#define GMAC_MIB_CTRL_ADDR			0x34
#define GMAC_MIB_RD_CLR				BIT(2)
#define GMAC_MIB_RST				BIT(1)
#define GMAC_MIB_EN				BIT(0)

#define GMAC_MIB_CTRL_MASK			\
	(GMAC_MIB_RD_CLR | GMAC_MIB_RST | GMAC_MIB_EN)

/* GMAC MIB counter registers */
#define GMAC_RXBROAD_ADDR			0x40
#define GMAC_RXPAUSE_ADDR			0x44
#define GMAC_RXMULTI_ADDR			0x48
#define GMAC_RXFCSERR_ADDR			0x4C
#define GMAC_RXALIGNERR_ADDR			0x50
#define GMAC_RXRUNT_ADDR			0x54
#define GMAC_RXFRAG_ADDR			0x58
#define GMAC_RXJUMBOFCSERR_ADDR			0x5C
#define GMAC_RXJUMBOALIGNERR_ADDR		0x60
#define GMAC_RXPKT64_ADDR			0x64
#define GMAC_RXPKT65TO127_ADDR			0x68
#define GMAC_RXPKT128TO255_ADDR			0x6C
#define GMAC_RXPKT256TO511_ADDR			0x70
#define GMAC_RXPKT512TO1023_ADDR		0x74
#define GMAC_RXPKT1024TO1518_ADDR		0x78
#define GMAC_RXPKT1519TOX_ADDR			0x7C
#define GMAC_RXTOOLONG_ADDR			0x80
#define GMAC_RXBYTE_G_ADDR			0x84
#define GMAC_RXBYTE_B_ADDR			0x8C
#define GMAC_RXUNI_ADDR				0x94
#define GMAC_TXBROAD_ADDR			0xA0
#define GMAC_TXPAUSE_ADDR			0xA4
#define GMAC_TXMULTI_ADDR			0xA8
#define GMAC_TXUNDERRUN_ADDR			0xAC
#define GMAC_TXPKT64_ADDR			0xB0
#define GMAC_TXPKT65TO127_ADDR			0xB4
#define GMAC_TXPKT128TO255_ADDR			0xB8
#define GMAC_TXPKT256TO511_ADDR			0xBC
#define GMAC_TXPKT512TO1023_ADDR		0xC0
#define GMAC_TXPKT1024TO1518_ADDR		0xC4
#define GMAC_TXPKT1519TOX_ADDR			0xC8
#define GMAC_TXBYTE_ADDR			0xCC
#define GMAC_TXCOLLISIONS_ADDR			0xD4
#define GMAC_TXABORTCOL_ADDR			0xD8
#define GMAC_TXMULTICOL_ADDR			0xDC
#define GMAC_TXSINGLECOL_ADDR			0xE0
#define GMAC_TXEXCESSIVEDEFER_ADDR		0xE4
#define GMAC_TXDEFER_ADDR			0xE8
#define GMAC_TXLATECOL_ADDR			0xEC
#define GMAC_TXUNI_ADDR				0xF0

/* XGMAC TX configuration register */
#define XGMAC_TX_CONFIG_ADDR			0x0
#define XGMAC_SPEED_M				GENMASK(31, 29)
#define XGMAC_SPEED_10000_USXGMII		FIELD_PREP(XGMAC_SPEED_M, 4)
#define XGMAC_SPEED_10000			FIELD_PREP(XGMAC_SPEED_M, 0)
#define XGMAC_SPEED_5000			FIELD_PREP(XGMAC_SPEED_M, 5)
#define XGMAC_SPEED_2500_USXGMII		FIELD_PREP(XGMAC_SPEED_M, 6)
#define XGMAC_SPEED_2500			FIELD_PREP(XGMAC_SPEED_M, 2)
#define XGMAC_SPEED_1000			FIELD_PREP(XGMAC_SPEED_M, 3)
#define XGMAC_SPEED_100				XGMAC_SPEED_1000
#define XGMAC_SPEED_10				XGMAC_SPEED_1000
#define XGMAC_JD				BIT(16)
#define XGMAC_TXEN				BIT(0)

/* XGMAC RX configuration register */
#define XGMAC_RX_CONFIG_ADDR			0x4
#define XGMAC_GPSL_M				GENMASK(29, 16)
#define XGMAC_WD				BIT(7)
#define XGMAC_GPSLEN				BIT(6)
#define XGMAC_CST				BIT(2)
#define XGMAC_ACS				BIT(1)
#define XGMAC_RXEN				BIT(0)

#define XGMAC_RX_CONFIG_MASK			\
	(XGMAC_GPSL_M | XGMAC_WD | XGMAC_GPSLEN | XGMAC_CST | \
	 XGMAC_ACS | XGMAC_RXEN)

/* XGMAC packet filter register */
#define XGMAC_PKT_FILTER_ADDR			0x8
#define XGMAC_RA				BIT(31)
#define XGMAC_PCF_M				GENMASK(7, 6)
#define XGMAC_PR				BIT(0)

#define XGMAC_PKT_FILTER_MASK			\
	(XGMAC_RA | XGMAC_PCF_M | XGMAC_PR)
#define XGMAC_PKT_FILTER_VAL			\
	(XGMAC_RA | XGMAC_PR | FIELD_PREP(XGMAC_PCF_M, 0x2))

/* XGMAC watchdog timeout register */
#define XGMAC_WD_TIMEOUT_ADDR			0xc
#define XGMAC_PWE				BIT(8)
#define XGMAC_WTO_M				GENMASK(3, 0)

#define XGMAC_WD_TIMEOUT_MASK			\
	(XGMAC_PWE | XGMAC_WTO_M)
#define XGMAC_WD_TIMEOUT_VAL			\
	(XGMAC_PWE | FIELD_PREP(XGMAC_WTO_M, 0xb))

/* XGMAC TX flow control register */
#define XGMAC_TX_FLOW_CTRL_ADDR			0x70
#define XGMAC_PAUSE_TIME_M			GENMASK(31, 16)
#define XGMAC_TXFCEN				BIT(1)

/* XGMAC RX flow control register */
#define XGMAC_RX_FLOW_CTRL_ADDR			0x90
#define XGMAC_RXFCEN				BIT(0)

/* XGMAC MAC address register */
#define XGMAC_ADDR0_H_ADDR			0x300
#define XGMAC_ADDR_EN				BIT(31)
#define XGMAC_ADDRH				GENMASK(15, 0)

#define XGMAC_ADDR0_L_ADDR			0x304
#define XGMAC_ADDRL				GENMASK(31, 0)

/* XGMAC management counters control register */
#define XGMAC_MMC_CTRL_ADDR			0x800
#define XGMAC_MCF				BIT(3)
#define XGMAC_CNTRST				BIT(0)

/* XGMAC MIB counter registers */
#define XGMAC_TXBYTE_GB_ADDR			0x814
#define XGMAC_TXPKT_GB_ADDR			0x81C
#define XGMAC_TXBROAD_G_ADDR			0x824
#define XGMAC_TXMULTI_G_ADDR			0x82C
#define XGMAC_TXPKT64_GB_ADDR			0x834
#define XGMAC_TXPKT65TO127_GB_ADDR		0x83C
#define XGMAC_TXPKT128TO255_GB_ADDR		0x844
#define XGMAC_TXPKT256TO511_GB_ADDR		0x84C
#define XGMAC_TXPKT512TO1023_GB_ADDR		0x854
#define XGMAC_TXPKT1024TOMAX_GB_ADDR		0x85C
#define XGMAC_TXUNI_GB_ADDR			0x864
#define XGMAC_TXMULTI_GB_ADDR			0x86C
#define XGMAC_TXBROAD_GB_ADDR			0x874
#define XGMAC_TXUNDERFLOW_ERR_ADDR		0x87C
#define XGMAC_TXBYTE_G_ADDR			0x884
#define XGMAC_TXPKT_G_ADDR			0x88C
#define XGMAC_TXPAUSE_ADDR			0x894
#define XGMAC_TXVLAN_G_ADDR			0x89C
#define XGMAC_TXLPI_USEC_ADDR			0x8A4
#define XGMAC_TXLPI_TRAN_ADDR			0x8A8
#define XGMAC_RXPKT_GB_ADDR			0x900
#define XGMAC_RXBYTE_GB_ADDR			0x908
#define XGMAC_RXBYTE_G_ADDR			0x910
#define XGMAC_RXBROAD_G_ADDR			0x918
#define XGMAC_RXMULTI_G_ADDR			0x920
#define XGMAC_RXCRC_ERR_ADDR			0x928
#define XGMAC_RXRUNT_ERR_ADDR			0x930
#define XGMAC_RXJABBER_ERR_ADDR			0x934
#define XGMAC_RXUNDERSIZE_G_ADDR		0x938
#define XGMAC_RXOVERSIZE_G_ADDR			0x93C
#define XGMAC_RXPKT64_GB_ADDR			0x940
#define XGMAC_RXPKT65TO127_GB_ADDR		0x948
#define XGMAC_RXPKT128TO255_GB_ADDR		0x950
#define XGMAC_RXPKT256TO511_GB_ADDR		0x958
#define XGMAC_RXPKT512TO1023_GB_ADDR		0x960
#define XGMAC_RXPKT1024TOMAX_GB_ADDR		0x968
#define XGMAC_RXUNI_G_ADDR			0x970
#define XGMAC_RXLEN_ERR_ADDR			0x978
#define XGMAC_RXOUTOFRANGE_ADDR			0x980
#define XGMAC_RXPAUSE_ADDR			0x988
#define XGMAC_RXFIFOOVERFLOW_ADDR		0x990
#define XGMAC_RXVLAN_GB_ADDR			0x998
#define XGMAC_RXWATCHDOG_ERR_ADDR		0x9A0
#define XGMAC_RXLPI_USEC_ADDR			0x9A4
#define XGMAC_RXLPI_TRAN_ADDR			0x9A8
#define XGMAC_RXDISCARD_GB_ADDR			0x9AC
#define XGMAC_RXDISCARDBYTE_GB_ADDR		0x9B4

#define EDMA_BASE_OFFSET			0xb00000

/* EDMA register offsets */
#define EDMA_REG_MAS_CTRL_ADDR			0x0
#define EDMA_REG_PORT_CTRL_ADDR			0x4
#define EDMA_REG_VLAN_CTRL_ADDR			0x8
#define EDMA_REG_RXDESC2FILL_MAP_0_ADDR		0x14
#define EDMA_REG_RXDESC2FILL_MAP_1_ADDR		0x18
#define EDMA_REG_RXDESC2FILL_MAP_2_ADDR		0x1c
#define EDMA_REG_TXQ_CTRL_ADDR			0x20
#define EDMA_REG_TXQ_CTRL_2_ADDR		0x24
#define EDMA_REG_TXQ_FC_0_ADDR			0x28
#define EDMA_REG_TXQ_FC_1_ADDR			0x30
#define EDMA_REG_TXQ_FC_2_ADDR			0x34
#define EDMA_REG_TXQ_FC_3_ADDR			0x38
#define EDMA_REG_RXQ_CTRL_ADDR			0x3c
#define EDMA_REG_MISC_ERR_QID_ADDR		0x40
#define EDMA_REG_RXQ_FC_THRE_ADDR		0x44
#define EDMA_REG_DMAR_CTRL_ADDR			0x48
#define EDMA_REG_AXIR_CTRL_ADDR			0x4c
#define EDMA_REG_AXIW_CTRL_ADDR			0x50
#define EDMA_REG_MIN_MSS_ADDR			0x54
#define EDMA_REG_LOOPBACK_CTRL_ADDR		0x58
#define EDMA_REG_MISC_INT_STAT_ADDR		0x5c
#define EDMA_REG_MISC_INT_MASK_ADDR		0x60
#define EDMA_REG_DBG_CTRL_ADDR			0x64
#define EDMA_REG_DBG_DATA_ADDR			0x68
#define EDMA_REG_TX_TIMEOUT_THRESH_ADDR		0x6c
#define EDMA_REG_REQ0_FIFO_THRESH_ADDR		0x80
#define EDMA_REG_WB_OS_THRESH_ADDR		0x84
#define EDMA_REG_MISC_ERR_QID_REG2_ADDR		0x88
#define EDMA_REG_TXDESC2CMPL_MAP_0_ADDR		0x8c
#define EDMA_REG_TXDESC2CMPL_MAP_1_ADDR		0x90
#define EDMA_REG_TXDESC2CMPL_MAP_2_ADDR		0x94
#define EDMA_REG_TXDESC2CMPL_MAP_3_ADDR		0x98
#define EDMA_REG_TXDESC2CMPL_MAP_4_ADDR		0x9c
#define EDMA_REG_TXDESC2CMPL_MAP_5_ADDR		0xa0

/* Tx descriptor ring configuration register addresses */
#define EDMA_REG_TXDESC_BA(n)		(0x1000 + (0x1000 * (n)))
#define EDMA_REG_TXDESC_PROD_IDX(n)	(0x1004 + (0x1000 * (n)))
#define EDMA_REG_TXDESC_CONS_IDX(n)	(0x1008 + (0x1000 * (n)))
#define EDMA_REG_TXDESC_RING_SIZE(n)	(0x100c + (0x1000 * (n)))
#define EDMA_REG_TXDESC_CTRL(n)		(0x1010 + (0x1000 * (n)))
#define EDMA_REG_TXDESC_BA2(n)		(0x1014 + (0x1000 * (n)))

/* RxFill ring configuration register addresses */
#define EDMA_REG_RXFILL_BA(n)		(0x29000 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_PROD_IDX(n)	(0x29004 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_CONS_IDX(n)	(0x29008 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_RING_SIZE(n)	(0x2900c + (0x1000 * (n)))
#define EDMA_REG_RXFILL_BUFFER1_SIZE(n)	(0x29010 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_FC_THRE(n)	(0x29014 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_UGT_THRE(n)	(0x29018 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_RING_EN(n)	(0x2901c + (0x1000 * (n)))
#define EDMA_REG_RXFILL_DISABLE(n)	(0x29020 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_DISABLE_DONE(n)	(0x29024 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_INT_STAT(n)	(0x31000 + (0x1000 * (n)))
#define EDMA_REG_RXFILL_INT_MASK(n)	(0x31004 + (0x1000 * (n)))

/* Rx descriptor ring configuration register addresses */
#define EDMA_REG_RXDESC_BA(n)		(0x39000 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_PROD_IDX(n)	(0x39004 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_CONS_IDX(n)	(0x39008 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_RING_SIZE(n)	(0x3900c + (0x1000 * (n)))
#define EDMA_REG_RXDESC_FC_THRE(n)	(0x39010 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_UGT_THRE(n)	(0x39014 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_CTRL(n)		(0x39018 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_BPC(n)		(0x3901c + (0x1000 * (n)))
#define EDMA_REG_RXDESC_DISABLE(n)	(0x39020 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_DISABLE_DONE(n)	(0x39024 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_PREHEADER_BA(n)	(0x39028 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_INT_STAT(n)	(0x59000 + (0x1000 * (n)))
#define EDMA_REG_RXDESC_INT_MASK(n)	(0x59004 + (0x1000 * (n)))

#define EDMA_REG_RX_MOD_TIMER(n)	(0x59008 + (0x1000 * (n)))
#define EDMA_REG_RX_INT_CTRL(n)		(0x5900c + (0x1000 * (n)))

/* Tx completion ring configuration register addresses */
#define EDMA_REG_TXCMPL_BA(n)		(0x79000 + (0x1000 * (n)))
#define EDMA_REG_TXCMPL_PROD_IDX(n)	(0x79004 + (0x1000 * (n)))
#define EDMA_REG_TXCMPL_CONS_IDX(n)	(0x79008 + (0x1000 * (n)))
#define EDMA_REG_TXCMPL_RING_SIZE(n)	(0x7900c + (0x1000 * (n)))
#define EDMA_REG_TXCMPL_UGT_THRE(n)	(0x79010 + (0x1000 * (n)))
#define EDMA_REG_TXCMPL_CTRL(n)		(0x79014 + (0x1000 * (n)))
#define EDMA_REG_TXCMPL_BPC(n)		(0x79018 + (0x1000 * (n)))

#define EDMA_REG_TX_INT_STAT(n)		(0x99000 + (0x1000 * (n)))
#define EDMA_REG_TX_INT_MASK(n)		(0x99004 + (0x1000 * (n)))
#define EDMA_REG_TX_MOD_TIMER(n)	(0x99008 + (0x1000 * (n)))
#define EDMA_REG_TX_INT_CTRL(n)		(0x9900c + (0x1000 * (n)))

/* EDMA_QID2RID_TABLE_MEM register field masks */
#define EDMA_RX_RING_ID_QUEUE0_MASK	GENMASK(7, 0)
#define EDMA_RX_RING_ID_QUEUE1_MASK	GENMASK(15, 8)
#define EDMA_RX_RING_ID_QUEUE2_MASK	GENMASK(23, 16)
#define EDMA_RX_RING_ID_QUEUE3_MASK	GENMASK(31, 24)

/* EDMA_REG_PORT_CTRL register bit definitions */
#define EDMA_PORT_PAD_EN			0x1
#define EDMA_PORT_EDMA_EN			0x2

/* EDMA_REG_DMAR_CTRL register field masks */
#define EDMA_DMAR_REQ_PRI_MASK			GENMASK(2, 0)
#define EDMA_DMAR_BURST_LEN_MASK		BIT(3)
#define EDMA_DMAR_TXDATA_OUTSTANDING_NUM_MASK	GENMASK(8, 4)
#define EDMA_DMAR_TXDESC_OUTSTANDING_NUM_MASK	GENMASK(11, 9)
#define EDMA_DMAR_RXFILL_OUTSTANDING_NUM_MASK	GENMASK(14, 12)

#define EDMA_BURST_LEN_ENABLE			0

/* Tx timeout threshold */
#define EDMA_TX_TIMEOUT_THRESH_VAL		0xFFFF

/* Rx descriptor ring base address mask */
#define EDMA_RXDESC_BA_MASK			0xffffffff

/* Rx Descriptor ring pre-header base address mask */
#define EDMA_RXDESC_PREHEADER_BA_MASK		0xffffffff

/* Tx descriptor prod ring index mask */
#define EDMA_TXDESC_PROD_IDX_MASK		0xffff

/* Tx descriptor consumer ring index mask */
#define EDMA_TXDESC_CONS_IDX_MASK		0xffff

/* Tx descriptor ring size mask */
#define EDMA_TXDESC_RING_SIZE_MASK		0xffff

/* Tx descriptor ring enable */
#define EDMA_TXDESC_TX_ENABLE			0x1

#define EDMA_TXDESC_CTRL_TXEN_MASK		BIT(0)
#define EDMA_TXDESC_CTRL_FC_GRP_ID_MASK		GENMASK(3, 1)

/* Tx completion ring prod index mask */
#define EDMA_TXCMPL_PROD_IDX_MASK		0xffff

/* Tx completion ring urgent threshold mask */
#define EDMA_TXCMPL_LOW_THRE_MASK		0xffff
#define EDMA_TXCMPL_LOW_THRE_SHIFT		0

/* EDMA_REG_TX_MOD_TIMER mask */
#define EDMA_TX_MOD_TIMER_INIT_MASK		0xffff
#define EDMA_TX_MOD_TIMER_INIT_SHIFT		0

/* Rx fill ring prod index mask */
#define EDMA_RXFILL_PROD_IDX_MASK		0xffff

/* Rx fill ring consumer index mask */
#define EDMA_RXFILL_CONS_IDX_MASK		0xffff

/* Rx fill ring size mask */
#define EDMA_RXFILL_RING_SIZE_MASK		0xffff

/* Rx fill ring flow control threshold masks */
#define EDMA_RXFILL_FC_XON_THRE_MASK		0x7ff
#define EDMA_RXFILL_FC_XON_THRE_SHIFT		12
#define EDMA_RXFILL_FC_XOFF_THRE_MASK		0x7ff
#define EDMA_RXFILL_FC_XOFF_THRE_SHIFT		0

/* Rx fill ring enable bit */
#define EDMA_RXFILL_RING_EN			0x1

/* Rx desc ring prod index mask */
#define EDMA_RXDESC_PROD_IDX_MASK		0xffff

/* Rx descriptor ring cons index mask */
#define EDMA_RXDESC_CONS_IDX_MASK		0xffff

/* Rx descriptor ring size masks */
#define EDMA_RXDESC_RING_SIZE_MASK		0xffff
#define EDMA_RXDESC_PL_OFFSET_MASK		0x1ff
#define EDMA_RXDESC_PL_OFFSET_SHIFT		16
#define EDMA_RXDESC_PL_DEFAULT_VALUE		0

/* Rx descriptor ring flow control threshold masks */
#define EDMA_RXDESC_FC_XON_THRE_MASK		0x7ff
#define EDMA_RXDESC_FC_XON_THRE_SHIFT		12
#define EDMA_RXDESC_FC_XOFF_THRE_MASK		0x7ff
#define EDMA_RXDESC_FC_XOFF_THRE_SHIFT		0

/* Rx descriptor ring urgent threshold mask */
#define EDMA_RXDESC_LOW_THRE_MASK		0xffff
#define EDMA_RXDESC_LOW_THRE_SHIFT		0

/* Rx descriptor ring enable bit */
#define EDMA_RXDESC_RX_EN			0x1

/* Tx interrupt status bit */
#define EDMA_TX_INT_MASK_PKT_INT		0x1

/* Rx interrupt mask */
#define EDMA_RXDESC_INT_MASK_PKT_INT		0x1

#define EDMA_MASK_INT_DISABLE			0x0
#define EDMA_MASK_INT_CLEAR			0x0

/* EDMA_REG_RX_MOD_TIMER register field masks */
#define EDMA_RX_MOD_TIMER_INIT_MASK		0xffff
#define EDMA_RX_MOD_TIMER_INIT_SHIFT		0

/* EDMA Ring mask */
#define EDMA_RING_DMA_MASK			0xffffffff

/* RXDESC threshold interrupt. */
#define EDMA_RXDESC_UGT_INT_STAT		0x2

/* RXDESC timer interrupt */
#define EDMA_RXDESC_PKT_INT_STAT		0x1

/* RXDESC Interrupt status mask */
#define EDMA_RXDESC_RING_INT_STATUS_MASK \
	(EDMA_RXDESC_UGT_INT_STAT | EDMA_RXDESC_PKT_INT_STAT)

/* TXCMPL threshold interrupt. */
#define EDMA_TXCMPL_UGT_INT_STAT		0x2

/* TXCMPL timer interrupt */
#define EDMA_TXCMPL_PKT_INT_STAT		0x1

/* TXCMPL Interrupt status mask */
#define EDMA_TXCMPL_RING_INT_STATUS_MASK \
	(EDMA_TXCMPL_UGT_INT_STAT | EDMA_TXCMPL_PKT_INT_STAT)

#define EDMA_TXCMPL_RETMODE_OPAQUE		0x0

#define EDMA_RXDESC_LOW_THRE			0
#define EDMA_RX_MOD_TIMER_INIT			1000
#define EDMA_RX_NE_INT_EN			0x2

#define EDMA_TX_MOD_TIMER			150

#define EDMA_TX_INITIAL_PROD_IDX		0x0
#define EDMA_TX_NE_INT_EN			0x2

/* EDMA misc error mask */
#define EDMA_MISC_AXI_RD_ERR_MASK		BIT(0)
#define EDMA_MISC_AXI_WR_ERR_MASK		BIT(1)
#define EDMA_MISC_RX_DESC_FIFO_FULL_MASK	BIT(2)
#define EDMA_MISC_RX_ERR_BUF_SIZE_MASK		BIT(3)
#define EDMA_MISC_TX_SRAM_FULL_MASK		BIT(4)
#define EDMA_MISC_TX_CMPL_BUF_FULL_MASK		BIT(5)

#define EDMA_MISC_DATA_LEN_ERR_MASK		BIT(6)
#define EDMA_MISC_TX_TIMEOUT_MASK		BIT(7)

/* EDMA txdesc2cmpl map */
#define EDMA_TXDESC2CMPL_MAP_TXDESC_MASK		0x1F

/* EDMA rxdesc2fill map */
#define EDMA_RXDESC2FILL_MAP_RXDESC_MASK	0x7

#endif
