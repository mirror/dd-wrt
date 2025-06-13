// SPDX-License-Identifier: GPL-2.0-only

#include <net/dsa.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <linux/limits.h>
#include <asm/mach-rtl838x/mach-rtl83xx.h>

#include "rtl83xx.h"

static const u8 ipv4_ll_mcast_addr_base[ETH_ALEN] =
{ 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00 };
static const u8 ipv4_ll_mcast_addr_mask[ETH_ALEN] =
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
static const u8 ipv6_all_hosts_mcast_addr_base[ETH_ALEN] =
{ 0x33, 0x33, 0x00, 0x00, 0x00, 0x01 };
static const u8 ipv6_all_hosts_mcast_addr_mask[ETH_ALEN] =
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

extern struct rtl83xx_soc_info soc_info;

static void rtl83xx_init_stats(struct rtl838x_switch_priv *priv)
{
	mutex_lock(&priv->reg_mutex);

	/* Enable statistics module: all counters plus debug.
	 * On RTL839x all counters are enabled by default
	 */
	if (priv->family_id == RTL8380_FAMILY_ID)
		sw_w32_mask(0, 3, RTL838X_STAT_CTRL);

	/* Reset statistics counters */
	sw_w32_mask(0, 1, priv->r->stat_rst);

	mutex_unlock(&priv->reg_mutex);
}

static void rtldsa_enable_phy_polling(struct rtl838x_switch_priv *priv)
{
	u64 v = 0;

	msleep(1000);
	/* Enable all ports with a PHY, including the SFP-ports */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			v |= BIT_ULL(i);
	}

	priv->r->set_port_reg_le(v, priv->r->smi_poll_ctrl);

	/* PHY update complete, there is no global PHY polling enable bit on the 9300 */
	if (priv->family_id == RTL8390_FAMILY_ID)
		sw_w32_mask(0, BIT(7), RTL839X_SMI_GLB_CTRL);
	else if(priv->family_id == RTL9300_FAMILY_ID)
		sw_w32_mask(0, 0x8000, RTL838X_SMI_GLB_CTRL);
}

const struct rtl83xx_mib_list_item rtl838x_mib_list[] = {
	MIB_LIST_ITEM("dot1dTpPortInDiscards", MIB_ITEM(MIB_REG_STD, 0xec, 1)),
	MIB_LIST_ITEM("ifOutDiscards", MIB_ITEM(MIB_REG_STD, 0xd0, 1)),
	MIB_LIST_ITEM("DropEvents", MIB_ITEM(MIB_REG_STD, 0xa8, 1)),
	MIB_LIST_ITEM("tx_BroadcastPkts", MIB_ITEM(MIB_REG_STD, 0xa4, 1)),
	MIB_LIST_ITEM("tx_MulticastPkts", MIB_ITEM(MIB_REG_STD, 0xa0, 1)),
	MIB_LIST_ITEM("tx_UndersizePkts", MIB_ITEM(MIB_REG_STD, 0x98, 1)),
	MIB_LIST_ITEM("rx_UndersizeDropPkts", MIB_ITEM(MIB_REG_STD, 0x90, 1)),
	MIB_LIST_ITEM("tx_OversizePkts", MIB_ITEM(MIB_REG_STD, 0x8c, 1)),
	MIB_LIST_ITEM("Collisions", MIB_ITEM(MIB_REG_STD, 0x7c, 1)),
	MIB_LIST_ITEM("rx_MacDiscards", MIB_ITEM(MIB_REG_STD, 0x40, 1))
};

const struct rtl83xx_mib_desc rtl838x_mib = {
	.symbol_errors = MIB_ITEM(MIB_REG_STD, 0xb8, 1),

	.if_in_octets = MIB_ITEM(MIB_REG_STD, 0xf8, 2),
	.if_out_octets = MIB_ITEM(MIB_REG_STD, 0xf0, 2),
	.if_in_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xe8, 1),
	.if_in_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe4, 1),
	.if_in_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe0, 1),
	.if_out_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xdc, 1),
	.if_out_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xd8, 1),
	.if_out_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xd4, 1),
	.if_out_discards = MIB_ITEM(MIB_REG_STD, 0xd0, 1),
	.single_collisions = MIB_ITEM(MIB_REG_STD, 0xcc, 1),
	.multiple_collisions = MIB_ITEM(MIB_REG_STD, 0xc8, 1),
	.deferred_transmissions = MIB_ITEM(MIB_REG_STD, 0xc4, 1),
	.late_collisions = MIB_ITEM(MIB_REG_STD, 0xc0, 1),
	.excessive_collisions = MIB_ITEM(MIB_REG_STD, 0xbc, 1),
	.crc_align_errors = MIB_ITEM(MIB_REG_STD, 0x9c, 1),

	.unsupported_opcodes = MIB_ITEM(MIB_REG_STD, 0xb4, 1),

	.rx_undersize_pkts = MIB_ITEM(MIB_REG_STD, 0x94, 1),
	.rx_oversize_pkts = MIB_ITEM(MIB_REG_STD, 0x88, 1),
	.rx_fragments = MIB_ITEM(MIB_REG_STD, 0x84, 1),
	.rx_jabbers = MIB_ITEM(MIB_REG_STD, 0x80, 1),

	.tx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x78, 1),
		MIB_ITEM(MIB_REG_STD, 0x70, 1),
		MIB_ITEM(MIB_REG_STD, 0x68, 1),
		MIB_ITEM(MIB_REG_STD, 0x60, 1),
		MIB_ITEM(MIB_REG_STD, 0x58, 1),
		MIB_ITEM(MIB_REG_STD, 0x50, 1),
		MIB_ITEM(MIB_REG_STD, 0x48, 1)
	},
	.rx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x74, 1),
		MIB_ITEM(MIB_REG_STD, 0x6c, 1),
		MIB_ITEM(MIB_REG_STD, 0x64, 1),
		MIB_ITEM(MIB_REG_STD, 0x5c, 1),
		MIB_ITEM(MIB_REG_STD, 0x54, 1),
		MIB_ITEM(MIB_REG_STD, 0x4c, 1),
		MIB_ITEM(MIB_REG_STD, 0x44, 1)
	},
	.rmon_ranges = {
		{ 0, 64 },
		{ 65, 127 },
		{ 128, 255 },
		{ 256, 511 },
		{ 512, 1023 },
		{ 1024, 1518 },
		{ 1519, 10000 }
	},

	.drop_events = MIB_ITEM(MIB_REG_STD, 0xa8, 1),
	.collisions = MIB_ITEM(MIB_REG_STD, 0x7c, 1),

	.rx_pause_frames = MIB_ITEM(MIB_REG_STD, 0xb0, 1),
	.tx_pause_frames = MIB_ITEM(MIB_REG_STD, 0xac, 1),

	.list_count = ARRAY_SIZE(rtl838x_mib_list),
	.list = rtl838x_mib_list
};

const struct rtl83xx_mib_list_item rtl839x_mib_list[] = {
	MIB_LIST_ITEM("ifOutDiscards", MIB_ITEM(MIB_REG_STD, 0xd4, 1)),
	MIB_LIST_ITEM("dot1dTpPortInDiscards", MIB_ITEM(MIB_REG_STD, 0xd0, 1)),
	MIB_LIST_ITEM("DropEvents", MIB_ITEM(MIB_REG_STD, 0xa8, 1)),
	MIB_LIST_ITEM("tx_BroadcastPkts", MIB_ITEM(MIB_REG_STD, 0xa4, 1)),
	MIB_LIST_ITEM("tx_MulticastPkts", MIB_ITEM(MIB_REG_STD, 0xa0, 1)),
	MIB_LIST_ITEM("tx_UndersizePkts", MIB_ITEM(MIB_REG_STD, 0x98, 1)),
	MIB_LIST_ITEM("rx_UndersizeDropPkts", MIB_ITEM(MIB_REG_STD, 0x90, 1)),
	MIB_LIST_ITEM("tx_OversizePkts", MIB_ITEM(MIB_REG_STD, 0x8c, 1)),
	MIB_LIST_ITEM("Collisions", MIB_ITEM(MIB_REG_STD, 0x7c, 1)),
	MIB_LIST_ITEM("rx_LengthFieldError", MIB_ITEM(MIB_REG_STD, 0x40, 1)),
	MIB_LIST_ITEM("rx_FalseCarrierTimes", MIB_ITEM(MIB_REG_STD, 0x3c, 1)),
	MIB_LIST_ITEM("rx_UnderSizeOctets", MIB_ITEM(MIB_REG_STD, 0x38, 1)),
	MIB_LIST_ITEM("tx_Fragments", MIB_ITEM(MIB_REG_STD, 0x34, 1)),
	MIB_LIST_ITEM("tx_Jabbers", MIB_ITEM(MIB_REG_STD, 0x30, 1)),
	MIB_LIST_ITEM("tx_CRCAlignErrors", MIB_ITEM(MIB_REG_STD, 0x2c, 1)),
	MIB_LIST_ITEM("rx_FramingErrors", MIB_ITEM(MIB_REG_STD, 0x28, 1)),
	MIB_LIST_ITEM("rx_MacDiscards", MIB_ITEM(MIB_REG_STD, 0x24, 1))
};

const struct rtl83xx_mib_desc rtl839x_mib = {
	.symbol_errors = MIB_ITEM(MIB_REG_STD, 0xb8, 1),

	.if_in_octets = MIB_ITEM(MIB_REG_STD, 0xf8, 2),
	.if_out_octets = MIB_ITEM(MIB_REG_STD, 0xf0, 2),
	.if_in_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xec, 1),
	.if_in_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe8, 1),
	.if_in_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe4, 1),
	.if_out_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xe0, 1),
	.if_out_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xdc, 1),
	.if_out_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xd8, 1),
	.if_out_discards = MIB_ITEM(MIB_REG_STD, 0xd4, 1),
	.single_collisions = MIB_ITEM(MIB_REG_STD, 0xcc, 1),
	.multiple_collisions = MIB_ITEM(MIB_REG_STD, 0xc8, 1),
	.deferred_transmissions = MIB_ITEM(MIB_REG_STD, 0xc4, 1),
	.late_collisions = MIB_ITEM(MIB_REG_STD, 0xc0, 1),
	.excessive_collisions = MIB_ITEM(MIB_REG_STD, 0xbc, 1),
	.crc_align_errors = MIB_ITEM(MIB_REG_STD, 0x9c, 1),

	.unsupported_opcodes = MIB_ITEM(MIB_REG_STD, 0xb4, 1),

	.rx_undersize_pkts = MIB_ITEM(MIB_REG_STD, 0x94, 1),
	.rx_oversize_pkts = MIB_ITEM(MIB_REG_STD, 0x88, 1),
	.rx_fragments = MIB_ITEM(MIB_REG_STD, 0x84, 1),
	.rx_jabbers = MIB_ITEM(MIB_REG_STD, 0x80, 1),

	.tx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x78, 1),
		MIB_ITEM(MIB_REG_STD, 0x70, 1),
		MIB_ITEM(MIB_REG_STD, 0x68, 1),
		MIB_ITEM(MIB_REG_STD, 0x60, 1),
		MIB_ITEM(MIB_REG_STD, 0x58, 1),
		MIB_ITEM(MIB_REG_STD, 0x50, 1),
		MIB_ITEM(MIB_REG_STD, 0x48, 1)
	},
	.rx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x74, 1),
		MIB_ITEM(MIB_REG_STD, 0x6c, 1),
		MIB_ITEM(MIB_REG_STD, 0x64, 1),
		MIB_ITEM(MIB_REG_STD, 0x5c, 1),
		MIB_ITEM(MIB_REG_STD, 0x54, 1),
		MIB_ITEM(MIB_REG_STD, 0x4c, 1),
		MIB_ITEM(MIB_REG_STD, 0x44, 1)
	},
	.rmon_ranges = {
		{ 0, 64 },
		{ 65, 127 },
		{ 128, 255 },
		{ 256, 511 },
		{ 512, 1023 },
		{ 1024, 1518 },
		{ 1519, 12288 }
	},

	.drop_events = MIB_ITEM(MIB_REG_STD, 0xa8, 1),
	.collisions = MIB_ITEM(MIB_REG_STD, 0x7c, 1),

	.rx_pause_frames = MIB_ITEM(MIB_REG_STD, 0xb0, 1),
	.tx_pause_frames = MIB_ITEM(MIB_REG_STD, 0xac, 1),

	.list_count = ARRAY_SIZE(rtl839x_mib_list),
	.list = rtl839x_mib_list
};

const struct rtl83xx_mib_list_item rtl930x_mib_list[] = {
	MIB_LIST_ITEM("ifOutDiscards", MIB_ITEM(MIB_REG_STD, 0xbc, 1)),
	MIB_LIST_ITEM("dot1dTpPortInDiscards", MIB_ITEM(MIB_REG_STD, 0xb8, 1)),
	MIB_LIST_ITEM("DropEvents", MIB_ITEM(MIB_REG_STD, 0x90, 1)),
	MIB_LIST_ITEM("tx_BroadcastPkts", MIB_ITEM(MIB_REG_STD, 0x8c, 1)),
	MIB_LIST_ITEM("tx_MulticastPkts", MIB_ITEM(MIB_REG_STD, 0x88, 1)),
	MIB_LIST_ITEM("tx_CRCAlignErrors", MIB_ITEM(MIB_REG_STD, 0x84, 1)),
	MIB_LIST_ITEM("tx_UndersizePkts", MIB_ITEM(MIB_REG_STD, 0x7c, 1)),
	MIB_LIST_ITEM("tx_OversizePkts", MIB_ITEM(MIB_REG_STD, 0x74, 1)),
	MIB_LIST_ITEM("tx_Fragments", MIB_ITEM(MIB_REG_STD, 0x6c, 1)),
	MIB_LIST_ITEM("tx_Jabbers", MIB_ITEM(MIB_REG_STD, 0x64, 1)),
	MIB_LIST_ITEM("tx_Collisions", MIB_ITEM(MIB_REG_STD, 0x5c, 1)),
	MIB_LIST_ITEM("rx_UndersizeDropPkts", MIB_ITEM(MIB_REG_PRV, 0x7c, 1)),
	MIB_LIST_ITEM("tx_PktsFlexibleOctetsSet1", MIB_ITEM(MIB_REG_PRV, 0x68, 1)),
	MIB_LIST_ITEM("rx_PktsFlexibleOctetsSet1", MIB_ITEM(MIB_REG_PRV, 0x64, 1)),
	MIB_LIST_ITEM("tx_PktsFlexibleOctetsCRCSet1", MIB_ITEM(MIB_REG_PRV, 0x60, 1)),
	MIB_LIST_ITEM("rx_PktsFlexibleOctetsCRCSet1", MIB_ITEM(MIB_REG_PRV, 0x5c, 1)),
	MIB_LIST_ITEM("tx_PktsFlexibleOctetsSet0", MIB_ITEM(MIB_REG_PRV, 0x58, 1)),
	MIB_LIST_ITEM("rx_PktsFlexibleOctetsSet0", MIB_ITEM(MIB_REG_PRV, 0x54, 1)),
	MIB_LIST_ITEM("tx_PktsFlexibleOctetsCRCSet0", MIB_ITEM(MIB_REG_PRV, 0x50, 1)),
	MIB_LIST_ITEM("rx_PktsFlexibleOctetsCRCSet0", MIB_ITEM(MIB_REG_PRV, 0x4c, 1)),
	MIB_LIST_ITEM("LengthFieldError", MIB_ITEM(MIB_REG_PRV, 0x48, 1)),
	MIB_LIST_ITEM("FalseCarrierTimes", MIB_ITEM(MIB_REG_PRV, 0x44, 1)),
	MIB_LIST_ITEM("UndersizeOctets", MIB_ITEM(MIB_REG_PRV, 0x40, 1)),
	MIB_LIST_ITEM("FramingErrors", MIB_ITEM(MIB_REG_PRV, 0x3c, 1)),
	MIB_LIST_ITEM("ParserErrors", MIB_ITEM(MIB_REG_PRV, 0x38, 1)),
	MIB_LIST_ITEM("rx_MacDiscards", MIB_ITEM(MIB_REG_PRV, 0x34, 1)),
	MIB_LIST_ITEM("rx_MacIPGShortDrop", MIB_ITEM(MIB_REG_PRV, 0x30, 1))
};

const struct rtl83xx_mib_desc rtl930x_mib = {
	.symbol_errors = MIB_ITEM(MIB_REG_STD, 0xa0, 1),

	.if_in_octets = MIB_ITEM(MIB_REG_STD, 0xf8, 2),
	.if_out_octets = MIB_ITEM(MIB_REG_STD, 0xf0, 2),
	.if_in_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xe8, 2),
	.if_in_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe0, 2),
	.if_in_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xd8, 2),
	.if_out_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xd0, 2),
	.if_out_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xc8, 2),
	.if_out_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xc0, 2),
	.if_out_discards = MIB_ITEM(MIB_REG_STD, 0xbc, 1),
	.single_collisions = MIB_ITEM(MIB_REG_STD, 0xb4, 1),
	.multiple_collisions = MIB_ITEM(MIB_REG_STD, 0xb0, 1),
	.deferred_transmissions = MIB_ITEM(MIB_REG_STD, 0xac, 1),
	.late_collisions = MIB_ITEM(MIB_REG_STD, 0xa8, 1),
	.excessive_collisions = MIB_ITEM(MIB_REG_STD, 0xa4, 1),
	.crc_align_errors = MIB_ITEM(MIB_REG_STD, 0x80, 1),
	.rx_pkts_over_max_octets = MIB_ITEM(MIB_REG_PRV, 0x6c, 1),

	.unsupported_opcodes = MIB_ITEM(MIB_REG_STD, 0x9c, 1),

	.rx_undersize_pkts = MIB_ITEM(MIB_REG_STD, 0x78, 1),
	.rx_oversize_pkts = MIB_ITEM(MIB_REG_STD, 0x70, 1),
	.rx_fragments = MIB_ITEM(MIB_REG_STD, 0x68, 1),
	.rx_jabbers = MIB_ITEM(MIB_REG_STD, 0x60, 1),

	.tx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x58, 1),
		MIB_ITEM(MIB_REG_STD, 0x50, 1),
		MIB_ITEM(MIB_REG_STD, 0x48, 1),
		MIB_ITEM(MIB_REG_STD, 0x40, 1),
		MIB_ITEM(MIB_REG_STD, 0x38, 1),
		MIB_ITEM(MIB_REG_STD, 0x30, 1),
		MIB_ITEM(MIB_REG_PRV, 0x78, 1),
		MIB_ITEM(MIB_REG_PRV, 0x70, 1)
	},
	.rx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x54, 1),
		MIB_ITEM(MIB_REG_STD, 0x4c, 1),
		MIB_ITEM(MIB_REG_STD, 0x44, 1),
		MIB_ITEM(MIB_REG_STD, 0x3c, 1),
		MIB_ITEM(MIB_REG_STD, 0x34, 1),
		MIB_ITEM(MIB_REG_STD, 0x2c, 1),
		MIB_ITEM(MIB_REG_PRV, 0x74, 1),
		MIB_ITEM(MIB_REG_PRV, 0x6c, 1),
	},
	.rmon_ranges = {
		{ 0, 64 },
		{ 65, 127 },
		{ 128, 255 },
		{ 256, 511 },
		{ 512, 1023 },
		{ 1024, 1518 },
		{ 1519, 12288 },
		{ 12289, 65535 }
	},

	.drop_events = MIB_ITEM(MIB_REG_STD, 0x90, 1),
	.collisions = MIB_ITEM(MIB_REG_STD, 0x5c, 1),

	.rx_pause_frames = MIB_ITEM(MIB_REG_STD, 0x98, 1),
	.tx_pause_frames = MIB_ITEM(MIB_REG_STD, 0x94, 1),

	.list_count = ARRAY_SIZE(rtl930x_mib_list),
	.list = rtl930x_mib_list
};


/* DSA callbacks */


static enum dsa_tag_protocol rtldsa_get_tag_protocol(struct dsa_switch *ds,
						     int port,
						     enum dsa_tag_protocol mprot)
{
	/* The switch does not tag the frames, instead internally the header
	 * structure for each packet is tagged accordingly.
	 */
	return DSA_TAG_PROTO_TRAILER;
}

static void rtldsa_vlan_set_pvid(struct rtl838x_switch_priv *priv,
				  int port, int pvid)
{
	/* Set both inner and outer PVID of the port */
	priv->r->vlan_port_pvid_set(port, PBVLAN_TYPE_INNER, pvid);
	priv->r->vlan_port_pvid_set(port, PBVLAN_TYPE_OUTER, pvid);
	priv->r->vlan_port_pvidmode_set(port, PBVLAN_TYPE_INNER,
					PBVLAN_MODE_UNTAG_AND_PRITAG);
	priv->r->vlan_port_pvidmode_set(port, PBVLAN_TYPE_OUTER,
					PBVLAN_MODE_UNTAG_AND_PRITAG);

	priv->ports[port].pvid = pvid;
}

static void rtl83xx_mc_pmasks_setup(struct rtl838x_switch_priv *priv)
{
	u64 portmask = 0;

	/* RTL8380 and RTL8390 use an index into the portmask table to set the
	 * unknown multicast portmask, setup a default at a safe location
	 * On RTL93XX, the portmask is directly set in the profile,
	 * see e.g. rtl9300_vlan_profile_setup
	 */
	if (priv->family_id == RTL8380_FAMILY_ID)
		portmask = RTL838X_MC_PMASK_ALL_PORTS;
	else if (priv->family_id == RTL8390_FAMILY_ID)
		portmask = RTL839X_MC_PMASK_ALL_PORTS;
	else
		dev_err(priv->dev, "%s: unknown family_id %u\n", __func__,
			priv->family_id);

	priv->r->write_mcast_pmask(MC_PMASK_ALL_PORTS_IDX, portmask);
	priv->r->write_mcast_pmask(MC_PMASK_MIN_PORTS_IDX, priv->mc_router_portmask);
}

/* Initialize all VLANS */
static void rtldsa_vlan_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtl838x_vlan_info info;

	pr_debug("In %s\n", __func__);

	/* IGMP/MLD snooping disabled: */
	priv->r->vlan_profile_setup(priv, 0);
	/* IGMP snooping enabled, MLD snooping disabled: */
	priv->r->vlan_profile_setup(priv, RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4);
	/* IGMP snooping disabled, MLD snooping enabled: */
	priv->r->vlan_profile_setup(priv, RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6);
	/* IGMP/MLD snooping enabled: */
	priv->r->vlan_profile_setup(priv, RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4 |
					  RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6);

	priv->r->vlan_profile_dump(priv, 0);

	info.fid = 0;			/* Default Forwarding ID / MSTI */
	info.hash_uc_fid = false;	/* Do not build the L2 lookup hash with FID, but VID */
	info.hash_mc_fid = false;	/* Do the same for Multicast packets */
	info.profile_id = 0;		/* Use default Vlan Profile 0 */
	info.tagged_ports = 0;		/* Initially no port members */
	if (priv->family_id == RTL9310_FAMILY_ID) {
		info.if_id = 0;
		info.multicast_grp_mask = 0;
		info.l2_tunnel_list_id = -1;
	}

	/* Initialize normal VLANs 1-4095 */
	for (int i = 1; i < MAX_VLANS; i ++)
		priv->r->vlan_set_tagged(i, &info);

	/*
	 * Initialize the special VLAN 0 and reset PVIDs. The CPU port PVID
	 * is applied to packets from the CPU for untagged destinations,
	 * regardless if the actual ingress VID. Any port with untagged
	 * egress VLAN(s) must therefore be a member of VLAN 0 to support
	 * CPU port as ingress when VLAN filtering is enabled.
	 */
	for (int i = 0; i <= priv->cpu_port; i++) {
		rtldsa_vlan_set_pvid(priv, i, 0);
		info.tagged_ports |= BIT_ULL(i);
	}
	priv->r->vlan_set_tagged(0, &info);

	/* Set forwarding action based on inner VLAN tag */
	for (int i = 0; i < priv->cpu_port; i++)
		priv->r->vlan_fwd_on_inner(i, true);
}

static void rtl83xx_setup_bpdu_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->cpu_port; i++)
		priv->r->set_receive_management_action(i, BPDU, TRAP2CPU);
}

static void rtl83xx_setup_lldp_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->cpu_port; i++)
		priv->r->set_receive_management_action(i, LLDP, TRAP2CPU);
}

static void rtldsa_port_set_salrn(struct rtl838x_switch_priv *priv,
				  int port, bool enable)
{
	int shift = SALRN_PORT_SHIFT(port);
	int val = enable ? SALRN_MODE_HARDWARE : SALRN_MODE_DISABLED;

	sw_w32_mask(SALRN_MODE_MASK << shift, val << shift,
		    priv->r->l2_port_new_salrn(port));
}

static int rtl83xx_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s called\n", __func__);

	/* Disable MAC polling the PHY so that we can start configuration */
	priv->r->set_port_reg_le(0ULL, priv->r->smi_poll_ctrl);

	for (int i = 0; i < ds->num_ports; i++)
		priv->ports[i].enable = false;
	priv->ports[priv->cpu_port].enable = true;

	/* Configure ports so they are disabled by default, but once enabled
	 * they will work in isolated mode (only traffic between port and CPU).
	 */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy) {
			priv->ports[i].pm = BIT_ULL(priv->cpu_port);
			priv->r->traffic_set(i, BIT_ULL(i));
		}
	}
	priv->r->traffic_set(priv->cpu_port, BIT_ULL(priv->cpu_port));

	/* For standalone ports, forward packets even if a static fdb
	 * entry for the source address exists on another port.
	 */
	if (priv->r->set_static_move_action) {
		for (int i = 0; i <= priv->cpu_port; i++)
			priv->r->set_static_move_action(i, true);
	}

	if (priv->family_id == RTL8380_FAMILY_ID)
		rtl838x_print_matrix();
	else
		rtl839x_print_matrix();

	rtl83xx_init_stats(priv);

	rtl83xx_mc_pmasks_setup(priv);
	rtldsa_vlan_setup(ds);

	rtl83xx_setup_bpdu_traps(priv);
	rtl83xx_setup_lldp_traps(priv);

	ds->configure_vlan_while_not_filtering = true;

	priv->r->l2_learning_setup();

	rtldsa_port_set_salrn(priv, priv->cpu_port, false);
	ds->assisted_learning_on_cpu_port = true;

	/* Make sure all frames sent to the switch's MAC are trapped to the CPU-port
	 *  0: FWD, 1: DROP, 2: TRAP2CPU
	 */
	if (priv->family_id == RTL8380_FAMILY_ID)
		sw_w32(0x2, RTL838X_SPCL_TRAP_SWITCH_MAC_CTRL);
	else
		sw_w32(0x2, RTL839X_SPCL_TRAP_SWITCH_MAC_CTRL);

	/* Enable MAC Polling PHY again */
	rtldsa_enable_phy_polling(priv);
	pr_debug("Please wait until PHY is settled\n");
	msleep(1000);
	priv->r->pie_init(priv);

	return 0;
}

static int rtl93xx_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s called\n", __func__);

	/* Disable MAC polling the PHY so that we can start configuration */
	if (priv->family_id == RTL9300_FAMILY_ID)
		sw_w32(0, RTL930X_SMI_POLL_CTRL);

	if (priv->family_id == RTL9310_FAMILY_ID) {
		sw_w32(0, RTL931X_SMI_PORT_POLLING_CTRL);
		sw_w32(0, RTL931X_SMI_PORT_POLLING_CTRL + 4);
	}

	/* Disable all ports except CPU port */
	for (int i = 0; i < ds->num_ports; i++)
		priv->ports[i].enable = false;
	priv->ports[priv->cpu_port].enable = true;

	/* Configure ports so they are disabled by default, but once enabled
	 * they will work in isolated mode (only traffic between port and CPU).
	 */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy) {
			priv->ports[i].pm = BIT_ULL(priv->cpu_port);
			priv->r->traffic_set(i, BIT_ULL(i));
		}
	}
	priv->r->traffic_set(priv->cpu_port, BIT_ULL(priv->cpu_port));

	rtl930x_print_matrix();

	/* TODO: Initialize statistics */

	rtldsa_vlan_setup(ds);

	ds->configure_vlan_while_not_filtering = true;

	priv->r->l2_learning_setup();

	rtldsa_port_set_salrn(priv, priv->cpu_port, false);
	ds->assisted_learning_on_cpu_port = true;

	rtldsa_enable_phy_polling(priv);

	priv->r->pie_init(priv);

	priv->r->led_init(priv);

	return 0;
}

static int rtl93xx_get_sds(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct device_node *dn;
	u32 sds_num;

	if (!dev)
		return -1;
	if (dev->of_node) {
		dn = dev->of_node;
		if (of_property_read_u32(dn, "sds", &sds_num))
			sds_num = -1;
	} else {
		dev_err(dev, "No DT node.\n");
		return -1;
	}

	return sds_num;
}

static int rtl83xx_pcs_validate(struct phylink_pcs *pcs,
				unsigned long *supported,
				const struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	pr_debug("In %s port %d, state is %d", __func__, port, state->interface);

	if (!phy_interface_mode_is_rgmii(state->interface) &&
	    state->interface != PHY_INTERFACE_MODE_NA &&
	    state->interface != PHY_INTERFACE_MODE_1000BASEX &&
	    state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII &&
	    state->interface != PHY_INTERFACE_MODE_GMII &&
	    state->interface != PHY_INTERFACE_MODE_QSGMII &&
	    state->interface != PHY_INTERFACE_MODE_INTERNAL &&
	    state->interface != PHY_INTERFACE_MODE_SGMII) {
		bitmap_zero(supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
		dev_err(priv->ds->dev,
			"Unsupported interface: %d for port %d\n",
			state->interface, port);
		return -EINVAL;
	}

	/* Allow all the expected bits */
	phylink_set(mask, Autoneg);
	phylink_set_port_modes(mask);
	phylink_set(mask, Pause);
	phylink_set(mask, Asym_Pause);

	/* With the exclusion of MII and Reverse MII, we support Gigabit,
	 * including Half duplex
	 */
	if (state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII) {
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseT_Half);
	}

	/* On both the 8380 and 8382, ports 24-27 are SFP ports */
	if (port >= 24 && port <= 27 && priv->family_id == RTL8380_FAMILY_ID)
		phylink_set(mask, 1000baseX_Full);

	/* On the RTL839x family of SoCs, ports 48 to 51 are SFP ports */
	if (port >= 48 && port <= 51 && priv->family_id == RTL8390_FAMILY_ID)
		phylink_set(mask, 1000baseX_Full);

	phylink_set(mask, 10baseT_Half);
	phylink_set(mask, 10baseT_Full);
	phylink_set(mask, 100baseT_Half);
	phylink_set(mask, 100baseT_Full);

	bitmap_and(supported, supported, mask,
		   __ETHTOOL_LINK_MODE_MASK_NBITS);

	return 0;
}

static int rtl93xx_pcs_validate(struct phylink_pcs *pcs,
				unsigned long *supported,
				const struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	pr_debug("In %s port %d, state is %d (%s)", __func__, port, state->interface,
		 phy_modes(state->interface));

	if (!phy_interface_mode_is_rgmii(state->interface) &&
	    state->interface != PHY_INTERFACE_MODE_NA &&
	    state->interface != PHY_INTERFACE_MODE_1000BASEX &&
	    state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII &&
	    state->interface != PHY_INTERFACE_MODE_GMII &&
	    state->interface != PHY_INTERFACE_MODE_QSGMII &&
	    state->interface != PHY_INTERFACE_MODE_XGMII &&
	    state->interface != PHY_INTERFACE_MODE_HSGMII &&
	    state->interface != PHY_INTERFACE_MODE_2500BASEX &&
	    state->interface != PHY_INTERFACE_MODE_10GBASER &&
	    state->interface != PHY_INTERFACE_MODE_10GKR &&
	    state->interface != PHY_INTERFACE_MODE_USXGMII &&
	    state->interface != PHY_INTERFACE_MODE_INTERNAL &&
	    state->interface != PHY_INTERFACE_MODE_SGMII) {
		bitmap_zero(supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
		dev_err(priv->ds->dev,
			"Unsupported interface: %d for port %d\n",
			state->interface, port);
		return -EINVAL;
	}

	/* Allow all the expected bits */
	phylink_set(mask, Autoneg);
	phylink_set_port_modes(mask);
	phylink_set(mask, Pause);
	phylink_set(mask, Asym_Pause);

	/* With the exclusion of MII and Reverse MII, we support Gigabit,
	 * including Half duplex
	 */
	if (state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII) {
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseT_Half);
	}

	/* Internal phys of the RTL93xx family provide 10G */
	if (priv->ports[port].phy_is_integrated &&
	    state->interface == PHY_INTERFACE_MODE_1000BASEX) {
		phylink_set(mask, 1000baseX_Full);
	} else if (priv->ports[port].phy_is_integrated) {
		phylink_set(mask, 1000baseX_Full);
		phylink_set(mask, 10000baseKR_Full);
		phylink_set(mask, 10000baseSR_Full);
		phylink_set(mask, 10000baseCR_Full);
		phylink_set(mask, 10000baseLR_Full);
	}
	if (state->interface == PHY_INTERFACE_MODE_INTERNAL) {
		phylink_set(mask, 1000baseX_Full);
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 10000baseKR_Full);
		phylink_set(mask, 10000baseT_Full);
		phylink_set(mask, 10000baseSR_Full);
		phylink_set(mask, 10000baseCR_Full);
		phylink_set(mask, 10000baseLR_Full);
	}

	if (state->interface == PHY_INTERFACE_MODE_USXGMII) {
		phylink_set(mask, 2500baseT_Full);
		phylink_set(mask, 5000baseT_Full);
		phylink_set(mask, 10000baseT_Full);
	}

	if (state->interface == PHY_INTERFACE_MODE_HSGMII) {
		phylink_set(mask, 2500baseT_Full);
	}

	if (state->interface == PHY_INTERFACE_MODE_2500BASEX) {
		phylink_set(mask, 2500baseX_Full);
	}

	phylink_set(mask, 10baseT_Half);
	phylink_set(mask, 10baseT_Full);
	phylink_set(mask, 100baseT_Half);
	phylink_set(mask, 100baseT_Full);

	bitmap_and(supported, supported, mask,
		   __ETHTOOL_LINK_MODE_MASK_NBITS);
	pr_debug("%s leaving supported: %*pb", __func__, __ETHTOOL_LINK_MODE_MASK_NBITS, supported);

	return 0;
}

static void rtl83xx_pcs_get_state(struct phylink_pcs *pcs,
				  struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	u64 speed;
	u64 link;

	if (port < 0 || port > priv->cpu_port) {
		state->link = false;
		return;
	}

	state->link = 0;
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	if (link & BIT_ULL(port))
		state->link = 1;
	pr_debug("%s: link state port %d: %llx\n", __func__, port, link & BIT_ULL(port));

	state->duplex = 0;
	if (priv->r->get_port_reg_le(priv->r->mac_link_dup_sts) & BIT_ULL(port))
		state->duplex = 1;

	speed = priv->r->get_port_reg_le(priv->r->mac_link_spd_sts(port));
	speed >>= (port % 16) << 1;
	switch (speed & 0x3) {
	case 0:
		state->speed = SPEED_10;
		break;
	case 1:
		state->speed = SPEED_100;
		break;
	case 2:
		state->speed = SPEED_1000;
		break;
	case 3:
		if (priv->family_id == RTL9300_FAMILY_ID
			&& (port == 24 || port == 26)) /* Internal serdes */
			state->speed = SPEED_2500;
		else
			state->speed = SPEED_100; /* Is in fact 500Mbit */
	}

	state->pause &= (MLO_PAUSE_RX | MLO_PAUSE_TX);
	if (priv->r->get_port_reg_le(priv->r->mac_rx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_RX;
	if (priv->r->get_port_reg_le(priv->r->mac_tx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_TX;
}

static void rtl93xx_pcs_get_state(struct phylink_pcs *pcs,
				  struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	u64 speed;
	u64 link;
	u64 media;

	if (port < 0 || port > priv->cpu_port) {
		state->link = false;
		return;
	}

	/* On the RTL9300 for at least the RTL8226B PHY, the MAC-side link
	 * state needs to be read twice in order to read a correct result.
	 * This would not be necessary for ports connected e.g. to RTL8218D
	 * PHYs.
	 */
	state->link = 0;
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	if (link & BIT_ULL(port))
		state->link = 1;

	if (priv->family_id == RTL9310_FAMILY_ID)
		media = priv->r->get_port_reg_le(RTL931X_MAC_LINK_MEDIA_STS);

	if (priv->family_id == RTL9300_FAMILY_ID)
		media = sw_r32(RTL930X_MAC_LINK_MEDIA_STS);

	if (media & BIT_ULL(port))
		state->link = 1;

	pr_debug("%s: link state port %d: %llx, media %llx\n", __func__, port,
		 link & BIT_ULL(port), media);

	state->duplex = 0;
	if (priv->r->get_port_reg_le(priv->r->mac_link_dup_sts) & BIT_ULL(port))
		state->duplex = 1;

	speed = priv->r->get_port_reg_le(priv->r->mac_link_spd_sts(port));
	speed >>= (port % 8) << 2;
	switch (speed & 0xf) {
	case 0:
		state->speed = SPEED_10;
		break;
	case 1:
		state->speed = SPEED_100;
		break;
	case 2:
	case 7:
		state->speed = SPEED_1000;
		break;
	case 4:
		state->speed = SPEED_10000;
		break;
	case 5:
	case 8:
		state->speed = SPEED_2500;
		break;
	case 6:
		state->speed = SPEED_5000;
		break;
	default:
		pr_err("%s: unknown speed: %d\n", __func__, (u32)speed & 0xf);
	}

	if (priv->family_id == RTL9310_FAMILY_ID
		&& (port >= 52 && port <= 55)) { /* Internal serdes */
			state->speed = SPEED_10000;
			state->link = 1;
			state->duplex = 1;
	}

	pr_debug("%s: speed is: %d %d\n", __func__, (u32)speed & 0xf, state->speed);
	state->pause &= (MLO_PAUSE_RX | MLO_PAUSE_TX);
	if (priv->r->get_port_reg_le(priv->r->mac_rx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_RX;
	if (priv->r->get_port_reg_le(priv->r->mac_tx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_TX;
}

static int rtldsa_pcs_config(struct phylink_pcs *pcs, unsigned int neg_mode,
			     phy_interface_t interface,
			     const unsigned long *advertising,
			     bool permit_pause_to_mac)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;

	if (priv->r->pcs_config)
		return priv->r->pcs_config(pcs, neg_mode, interface, advertising, permit_pause_to_mac);

	return 0;
}

static void rtldsa_pcs_an_restart(struct phylink_pcs *pcs)
{
/* No restart functionality existed before we migrated to pcs */
}

static struct phylink_pcs *rtldsa_phylink_mac_select_pcs(struct dsa_switch *ds,
							 int port,
							 phy_interface_t interface)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	return &priv->pcs[port].pcs;
}

static void rtldsa_phylink_get_caps(struct dsa_switch *ds, int port,
				    struct phylink_config *config)
{
/*
 * This capability check will need some love. Depending on the model and the port
 * different link modes are supported. For now just enable all required values
 * so that we can make use of the ports.
 */
	__set_bit(PHY_INTERFACE_MODE_INTERNAL, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_GMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_QSGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_SGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_HSGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_XGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_USXGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_1000BASEX, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_2500BASEX, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_10GBASER, config->supported_interfaces);
}

static void rtldsa_phylink_mac_config(struct dsa_switch *ds, int port,
					unsigned int mode,
					const struct phylink_link_state *state)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (priv->r->phylink_mac_config)
		priv->r->phylink_mac_config(ds, port, mode, state);
}

static void rtldsa_phylink_mac_link_down(struct dsa_switch *ds, int port,
				     unsigned int mode,
				     phy_interface_t interface)
{

	struct rtl838x_switch_priv *priv = ds->priv;
	if (priv->r->phylink_mac_link_down)
	    priv->r->phylink_mac_link_down(ds, port, mode, interface);

}


static void rtldsa_phylink_mac_link_up(struct dsa_switch *ds, int port,
				   unsigned int mode,
				   phy_interface_t interface,
				   struct phy_device *phydev,
				   int speed, int duplex,
				   bool tx_pause, bool rx_pause)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	if (priv->r->phylink_mac_link_up)
	    priv->r->phylink_mac_link_up(ds, port, mode, interface, phydev, speed, duplex, tx_pause, rx_pause);

}

static const struct rtl83xx_mib_desc *rtl83xx_get_mib_desc(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	switch (priv->family_id) {
	case RTL8380_FAMILY_ID:
		return &rtl838x_mib;
	case RTL8390_FAMILY_ID:
		return &rtl839x_mib;
	case RTL9300_FAMILY_ID:
		return &rtl930x_mib;
	default:
		return NULL;
	}
}

static bool rtl83xx_read_mib_item(struct rtl838x_switch_priv *priv, int port,
				  const struct rtl83xx_mib_item *mib_item,
				  uint64_t *data)
{
	uint64_t tmp;
	int reg, reg_offset;

	switch (mib_item->reg) {
	case MIB_REG_STD:
		reg = priv->r->stat_port_std_mib;
		reg_offset = 256;
		break;
	case MIB_REG_PRV:
		reg = priv->r->stat_port_prv_mib;
		reg_offset = 128;
		break;
	default:
		return false;
	}

	*data = sw_r32(reg + (port + 1) * reg_offset - 4 - mib_item->offset);
	if (mib_item->size == 2) {
		tmp = sw_r32(reg + (port + 1) * reg_offset - 8 - mib_item->offset);
		*data |= tmp << 32;
	}

	return true;
}

static void rtldsa_get_strings(struct dsa_switch *ds,
			       int port, u32 stringset, u8 *data)
{
	const struct rtl83xx_mib_desc *mib_desc;

	if (stringset != ETH_SS_STATS)
		return;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	for (int i = 0; i < mib_desc->list_count; i++)
		ethtool_puts(&data, mib_desc->list[i].name);
}

static void rtldsa_get_ethtool_stats(struct dsa_switch *ds, int port,
				     uint64_t *data)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;
	const struct rtl83xx_mib_item *mib_item;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	for (int i = 0; i < mib_desc->list_count; i++) {
		mib_item = &mib_desc->list[i].item;
		rtl83xx_read_mib_item(priv, port, mib_item, &data[i]);
	}
}

static int rtldsa_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
	const struct rtl83xx_mib_desc *mib_desc;

	if (sset != ETH_SS_STATS)
		return 0;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return 0;

	return mib_desc->list_count;
}


static void rtl83xx_get_eth_phy_stats(struct dsa_switch *ds, int port,
				      struct ethtool_eth_phy_stats *phy_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	rtl83xx_read_mib_item(priv, port, &mib_desc->symbol_errors,
			      &phy_stats->SymbolErrorDuringCarrier);
}

static void rtl83xx_get_eth_mac_stats(struct dsa_switch *ds, int port,
				      struct ethtool_eth_mac_stats *mac_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;
	uint64_t val;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	/* Frame and octet counters are calculated based on RFC3635 */

	rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_ucast_pkts,
			      &mac_stats->FramesReceivedOK);
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_mcast_pkts,
				  &mac_stats->MulticastFramesReceivedOK))
		mac_stats->FramesReceivedOK += mac_stats->MulticastFramesReceivedOK;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_bcast_pkts,
				  &mac_stats->BroadcastFramesReceivedOK))
		mac_stats->FramesReceivedOK += mac_stats->BroadcastFramesReceivedOK;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->rx_pause_frames, &val))
		mac_stats->FramesReceivedOK += val;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->rx_pkts_over_max_octets, &val))
		mac_stats->FramesReceivedOK += val;

	rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_ucast_pkts,
			      &mac_stats->FramesTransmittedOK);
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_mcast_pkts,
				  &mac_stats->MulticastFramesXmittedOK))
		mac_stats->FramesTransmittedOK += mac_stats->MulticastFramesXmittedOK;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_bcast_pkts,
				  &mac_stats->BroadcastFramesXmittedOK))
		mac_stats->FramesTransmittedOK += mac_stats->BroadcastFramesXmittedOK;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->tx_pause_frames, &val))
		mac_stats->FramesTransmittedOK += val;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_discards, &val))
		mac_stats->FramesTransmittedOK -= val;

	rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_octets,
			      &mac_stats->OctetsReceivedOK);
	mac_stats->OctetsReceivedOK -= 18 * mac_stats->FramesReceivedOK;
	rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_octets,
			      &mac_stats->OctetsTransmittedOK);
	mac_stats->OctetsTransmittedOK -= 18 * mac_stats->FramesTransmittedOK;

	rtl83xx_read_mib_item(priv, port, &mib_desc->single_collisions,
			      &mac_stats->SingleCollisionFrames);
	rtl83xx_read_mib_item(priv, port, &mib_desc->multiple_collisions,
			      &mac_stats->MultipleCollisionFrames);
	rtl83xx_read_mib_item(priv, port, &mib_desc->deferred_transmissions,
			      &mac_stats->FramesWithDeferredXmissions);
	rtl83xx_read_mib_item(priv, port, &mib_desc->late_collisions,
			      &mac_stats->LateCollisions);
	rtl83xx_read_mib_item(priv, port, &mib_desc->excessive_collisions,
			      &mac_stats->FramesAbortedDueToXSColls);

	rtl83xx_read_mib_item(priv, port, &mib_desc->crc_align_errors,
			      &mac_stats->FrameCheckSequenceErrors);
}

static void rtl83xx_get_eth_ctrl_stats(struct dsa_switch *ds, int port,
				       struct ethtool_eth_ctrl_stats *ctrl_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	rtl83xx_read_mib_item(priv, port, &mib_desc->unsupported_opcodes,
			      &ctrl_stats->UnsupportedOpcodesReceived);
}

static void rtl83xx_get_rmon_stats(struct dsa_switch *ds, int port,
				   struct ethtool_rmon_stats *rmon_stats,
				   const struct ethtool_rmon_hist_range **ranges)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	rtl83xx_read_mib_item(priv, port, &mib_desc->rx_undersize_pkts,
			      &rmon_stats->undersize_pkts);
	rtl83xx_read_mib_item(priv, port, &mib_desc->rx_oversize_pkts,
			      &rmon_stats->oversize_pkts);
	rtl83xx_read_mib_item(priv, port, &mib_desc->rx_fragments,
			      &rmon_stats->fragments);
	rtl83xx_read_mib_item(priv, port, &mib_desc->rx_jabbers,
			      &rmon_stats->jabbers);

	for (int i = 0; i < ARRAY_SIZE(mib_desc->rx_pkts); i++) {
		if (mib_desc->rx_pkts[i].reg == MIB_REG_INVALID)
			break;

		rtl83xx_read_mib_item(priv, port, &mib_desc->rx_pkts[i],
				      &rmon_stats->hist[i]);
	}


	for (int i = 0; i < ARRAY_SIZE(mib_desc->tx_pkts); i++) {
		if (mib_desc->tx_pkts[i].reg == MIB_REG_INVALID)
			break;

		rtl83xx_read_mib_item(priv, port, &mib_desc->tx_pkts[i],
				      &rmon_stats->hist_tx[i]);
	}

	*ranges = mib_desc->rmon_ranges;
}

static void rtl83xx_get_stats64(struct dsa_switch *ds, int port,
				struct rtnl_link_stats64 *s)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;
	uint64_t val;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_ucast_pkts,  &s->rx_packets);
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_mcast_pkts, &s->multicast))
		s->rx_packets += s->multicast;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_bcast_pkts, &val))
		s->rx_packets += val;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->rx_pkts_over_max_octets, &val))
		s->rx_packets += val;

	rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_ucast_pkts, &s->tx_packets);
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_mcast_pkts, &val))
		s->tx_packets += val;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_bcast_pkts, &val))
		s->tx_packets += val;
	if (rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_discards, &val))
		s->tx_packets -= val;

	/* FCS for each packet has to be subtracted */
	rtl83xx_read_mib_item(priv, port, &mib_desc->if_in_octets, &s->rx_bytes);
	s->rx_bytes -= 4 * s->rx_packets;
	rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_octets, &s->tx_bytes);
	s->tx_bytes -= 4 * s->tx_packets;

	rtl83xx_read_mib_item(priv, port, &mib_desc->collisions, &s->collisions);

	rtl83xx_read_mib_item(priv, port, &mib_desc->drop_events, &s->rx_dropped);
	rtl83xx_read_mib_item(priv, port, &mib_desc->if_out_discards, &s->tx_dropped);

	rtl83xx_read_mib_item(priv, port, &mib_desc->crc_align_errors, &s->rx_crc_errors);
	s->rx_errors = s->rx_crc_errors;

	rtl83xx_read_mib_item(priv, port, &mib_desc->excessive_collisions, &s->tx_aborted_errors);
	rtl83xx_read_mib_item(priv, port, &mib_desc->late_collisions, &s->tx_window_errors);
	s->tx_errors = s->tx_aborted_errors + s->tx_window_errors;
}

static void rtl83xx_get_pause_stats(struct dsa_switch *ds, int port,
				    struct ethtool_pause_stats *pause_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib_desc;

	mib_desc = rtl83xx_get_mib_desc(ds);
	if (!mib_desc)
		return;

	rtl83xx_read_mib_item(priv, port, &mib_desc->tx_pause_frames,
			      &pause_stats->tx_pause_frames);
	rtl83xx_read_mib_item(priv, port, &mib_desc->rx_pause_frames,
			      &pause_stats->rx_pause_frames);
}

static int rtldsa_mc_group_alloc(struct rtl838x_switch_priv *priv, int port)
{
	int mc_group = find_first_zero_bit(priv->mc_group_bm, MAX_MC_GROUPS - 2);
	u64 portmask;

	if (mc_group >= MAX_MC_GROUPS - 2)
		return -1;

	set_bit(mc_group, priv->mc_group_bm);
	portmask = BIT_ULL(port) | priv->mc_router_portmask;
	priv->r->write_mcast_pmask(mc_group, portmask);

	return mc_group;
}

static u64 rtldsa_mc_group_add_port(struct rtl838x_switch_priv *priv, int mc_group, int port)
{
	u64 portmask = priv->r->read_mcast_pmask(mc_group);

	pr_debug("%s: %d\n", __func__, port);

	portmask |= BIT_ULL(port) | priv->mc_router_portmask;
	priv->r->write_mcast_pmask(mc_group, portmask);

	return portmask;
}

static u64 rtldsa_mc_group_del_ports(struct rtl838x_switch_priv *priv,
				     int mc_group, u64 portmask)
{
	portmask = priv->r->read_mcast_pmask(mc_group) & ~portmask;

	priv->r->write_mcast_pmask(mc_group, portmask);
	if (!portmask)
		clear_bit(mc_group, priv->mc_group_bm);

	return portmask;
}

static int rtldsa_port_enable(struct dsa_switch *ds, int port,
			      struct phy_device *phydev)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 v;

	pr_debug("%s: %x %d", __func__, (u32) priv, port);
	priv->ports[port].enable = true;

	/* enable inner tagging on egress, do not keep any tags */
	priv->r->vlan_port_keep_tag_set(port, 0, 1);

	if (dsa_is_cpu_port(ds, port))
		return 0;

	/* add port to switch mask of CPU_PORT */
	priv->r->traffic_enable(priv->cpu_port, port);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	/* add all other ports in the same bridge to switch mask of port */
	v = priv->r->traffic_get(port);
	v |= priv->ports[port].pm;
	priv->r->traffic_set(port, v);

	/* TODO: Figure out if this is necessary */
	if (priv->family_id == RTL9300_FAMILY_ID) {
		sw_w32_mask(0, BIT(port), RTL930X_L2_PORT_SABLK_CTRL);
		sw_w32_mask(0, BIT(port), RTL930X_L2_PORT_DABLK_CTRL);
	}

	if (priv->ports[port].sds_num < 0)
		priv->ports[port].sds_num = rtl93xx_get_sds(phydev);

	return 0;
}

static void rtldsa_port_disable(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 v;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);
	/* you can only disable user ports */
	if (!dsa_is_user_port(ds, port))
		return;

	/* BUG: This does not work on RTL931X */
	/* remove port from switch mask of CPU_PORT */
	priv->r->traffic_disable(priv->cpu_port, port);

	/* remove all other ports in the same bridge from switch mask of port */
	v = priv->r->traffic_get(port);
	v &= ~priv->ports[port].pm;
	priv->r->traffic_set(port, v);

	priv->ports[port].enable = false;
}

static int rtldsa_set_mac_eee(struct dsa_switch *ds, int port, struct ethtool_keee *e)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (e->eee_enabled && !priv->eee_enabled) {
		pr_info("Globally enabling EEE\n");
		priv->r->init_eee(priv, true);
	}

	priv->r->set_mac_eee(priv, port, e->eee_enabled);

	if (e->eee_enabled)
		pr_info("Enabled EEE for port %d\n", port);
	else
		pr_info("Disabled EEE for port %d\n", port);

	return 0;
}

#if 0
static int rtldsa_get_mac_eee(struct dsa_switch *ds, int port,
			       struct ethtool_eee *e)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int ret = 0;

	ret = priv->r->eee_port_ability(priv, e, port);

	if (!ret) {
		e->eee_enabled = priv->ports[port].eee_enabled;
		e->eee_active = !!(e->advertised & e->lp_advertised);
	}

	return ret;
}
#endif

static int rtldsa_get_mac_eee(struct dsa_switch *ds, int port, struct ethtool_keee *eee)
{
	return 0;
}

static int rtldsa_set_ageing_time(struct dsa_switch *ds, unsigned int msec)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	priv->r->set_ageing_time(msec);

	return 0;
}

static int rtldsa_port_bridge_join(struct dsa_switch *ds, int port,
				   struct dsa_bridge bridge,
				   bool *tx_fwd_offload,
				   struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 port_bitmap = BIT_ULL(priv->cpu_port), v;

	pr_debug("%s %x: %d %llx", __func__, (u32)priv, port, port_bitmap);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);
	for (int i = 0; i < ds->num_ports; i++) {
		/* Add this port to the port matrix of the other ports in the
		 * same bridge. If the port is disabled, port matrix is kept
		 * and not being setup until the port becomes enabled.
		 */
		if (dsa_is_user_port(ds, i) && !priv->is_lagmember[i] && i != port) {
			if (!dsa_port_offloads_bridge(dsa_to_port(ds, i), &bridge))
				continue;
			if (priv->ports[i].enable)
				priv->r->traffic_enable(i, port);

			priv->ports[i].pm |= BIT_ULL(port);
			port_bitmap |= BIT_ULL(i);
		}
	}

	/* Add all other ports to this port matrix. */
	if (priv->ports[port].enable) {
		priv->r->traffic_enable(priv->cpu_port, port);
		v = priv->r->traffic_get(port);
		v |= port_bitmap;
		priv->r->traffic_set(port, v);
	}
	priv->ports[port].pm |= port_bitmap;

	if (priv->r->set_static_move_action)
		priv->r->set_static_move_action(port, false);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtldsa_port_bridge_leave(struct dsa_switch *ds, int port,
				     struct dsa_bridge bridge)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 port_bitmap = 0, v;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);
	mutex_lock(&priv->reg_mutex);
	for (int i = 0; i < ds->num_ports; i++) {
		/* Remove this port from the port matrix of the other ports
		 * in the same bridge. If the port is disabled, port matrix
		 * is kept and not being setup until the port becomes enabled.
		 * And the other port's port matrix cannot be broken when the
		 * other port is still a VLAN-aware port.
		 */
		if (dsa_is_user_port(ds, i) && i != port) {
			if (!dsa_port_offloads_bridge(dsa_to_port(ds, i), &bridge))
				continue;
			if (priv->ports[i].enable)
				priv->r->traffic_disable(i, port);

			priv->ports[i].pm &= ~BIT_ULL(port);
			port_bitmap |= BIT_ULL(i);
		}
	}

	/* Remove all other ports from this port matrix. */
	if (priv->ports[port].enable) {
		v = priv->r->traffic_get(port);
		v &= ~port_bitmap;
		priv->r->traffic_set(port, v);
	}
	priv->ports[port].pm &= ~port_bitmap;

	if (priv->r->set_static_move_action)
		priv->r->set_static_move_action(port, true);

	mutex_unlock(&priv->reg_mutex);
}

void rtldsa_port_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
	u32 msti = 0;
	u32 port_state[4];
	int index, bit;
	int pos = port;
	struct rtl838x_switch_priv *priv = ds->priv;
	int n = priv->port_width << 1;

	/* Ports above or equal CPU port can never be configured */
	if (port >= priv->cpu_port)
		return;

	mutex_lock(&priv->reg_mutex);

	/* For the RTL839x and following, the bits are left-aligned, 838x and 930x
	 * have 64 bit fields, 839x and 931x have 128 bit fields
	 */
	if (priv->family_id == RTL8390_FAMILY_ID)
		pos += 12;
	if (priv->family_id == RTL9300_FAMILY_ID)
		pos += 3;
	if (priv->family_id == RTL9310_FAMILY_ID)
		pos += 8;

	index = n - (pos >> 4) - 1;
	bit = (pos << 1) % 32;

	priv->r->stp_get(priv, msti, port_state);

	pr_debug("Current state, port %d: %d\n", port, (port_state[index] >> bit) & 3);
	port_state[index] &= ~(3 << bit);

	switch (state) {
	case BR_STATE_DISABLED: /* 0 */
		port_state[index] |= (0 << bit);
		break;
	case BR_STATE_BLOCKING:  /* 4 */
	case BR_STATE_LISTENING: /* 1 */
		port_state[index] |= (1 << bit);
		break;
	case BR_STATE_LEARNING: /* 2 */
		port_state[index] |= (2 << bit);
		break;
	case BR_STATE_FORWARDING: /* 3 */
		port_state[index] |= (3 << bit);
	default:
		break;
	}

	priv->r->stp_set(priv, msti, port_state);

	mutex_unlock(&priv->reg_mutex);
}

void rtldsa_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (priv->r->fast_age)
		priv->r->fast_age(ds, port);

}
static int rtldsa_vlan_filtering(struct dsa_switch *ds, int port,
				 bool vlan_filtering,
				 struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s: port %d\n", __func__, port);
	mutex_lock(&priv->reg_mutex);

	if (vlan_filtering) {
		/* Enable ingress and egress filtering
		 * The VLAN_PORT_IGR_FILTER register uses 2 bits for each port to define
		 * the filter action:
		 * 0: Always Forward
		 * 1: Drop packet
		 * 2: Trap packet to CPU port
		 * The Egress filter used 1 bit per state (0: DISABLED, 1: ENABLED)
		 */
		if (port != priv->cpu_port) {
			priv->r->set_vlan_igr_filter(port, IGR_DROP);
			priv->r->set_vlan_egr_filter(port, EGR_ENABLE);
		}
		else {
			priv->r->set_vlan_igr_filter(port, IGR_TRAP);
			priv->r->set_vlan_egr_filter(port, EGR_DISABLE);
		}

	} else {
		/* Disable ingress and egress filtering */
		if (port != priv->cpu_port)
			priv->r->set_vlan_igr_filter(port, IGR_FORWARD);

		priv->r->set_vlan_egr_filter(port, EGR_DISABLE);
	}

	/* Do we need to do something to the CPU-Port, too? */
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtldsa_vlan_prepare(struct dsa_switch *ds, int port,
			       const struct switchdev_obj_port_vlan *vlan)
{
	struct rtl838x_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;

	priv->r->vlan_tables_read(0, &info);

	pr_debug("VLAN 0: Tagged ports %llx, untag %llx, profile %d, MC# %d, UC# %d, FID %x\n",
		info.tagged_ports, info.untagged_ports, info.profile_id,
		info.hash_mc_fid, info.hash_uc_fid, info.fid);

	priv->r->vlan_tables_read(1, &info);
	pr_debug("VLAN 1: Tagged ports %llx, untag %llx, profile %d, MC# %d, UC# %d, FID %x\n",
		info.tagged_ports, info.untagged_ports, info.profile_id,
		info.hash_mc_fid, info.hash_uc_fid, info.fid);
	priv->r->vlan_set_untagged(1, info.untagged_ports);
	pr_debug("SET: Untagged ports, VLAN %d: %llx\n", 1, info.untagged_ports);

	priv->r->vlan_set_tagged(1, &info);
	pr_debug("SET: Tagged ports, VLAN %d: %llx\n", 1, info.tagged_ports);

	return 0;
}

static int rtldsa_vlan_add(struct dsa_switch *ds, int port,
			   const struct switchdev_obj_port_vlan *vlan,
			   struct netlink_ext_ack *extack)
{
	struct rtl838x_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;
	int err;

	pr_debug("%s port %d, vid %d, flags %x\n",
		__func__, port, vlan->vid, vlan->flags);

	/* Let no one mess with our special VLAN 0 */
	if (!vlan->vid) return 0;

	if (vlan->vid >= MAX_VLANS) {
		dev_err(priv->dev, "VLAN out of range: %d", vlan->vid);
		return -ENOTSUPP;
	}

	err = rtldsa_vlan_prepare(ds, port, vlan);
	if (err)
		return err;

	mutex_lock(&priv->reg_mutex);

	/*
	 * Realtek switches copy frames as-is to/from the CPU. For a proper
	 * VLAN handling the 12 bit RVID field (= VLAN id) for incoming traffic
	 * and the 1 bit RVID_SEL field (0 = use inner tag, 1 = use outer tag)
	 * for outgoing traffic of the CPU tag structure need to be handled. As
	 * of now no such logic is in place. So for the CPU port keep the fixed
	 * PVID=0 from initial setup in place and ignore all subsequent settings.
	 */
	if (port != priv->cpu_port) {
		if (vlan->flags & BRIDGE_VLAN_INFO_PVID)
			rtldsa_vlan_set_pvid(priv, port, vlan->vid);
		else if (priv->ports[port].pvid == vlan->vid)
			rtldsa_vlan_set_pvid(priv, port, 0);
	}

	/* Get port memberships of this vlan */
	priv->r->vlan_tables_read(vlan->vid, &info);

	/* new VLAN? */
	if (!info.tagged_ports) {
		info.fid = 0;
		info.hash_mc_fid = false;
		info.hash_uc_fid = false;
		info.profile_id = 0;
	}

	/* sanitize untagged_ports - must be a subset */
	if (info.untagged_ports & ~info.tagged_ports)
		info.untagged_ports = 0;

	info.tagged_ports |= BIT_ULL(port);
	if (vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED)
		info.untagged_ports |= BIT_ULL(port);
	else
		info.untagged_ports &= ~BIT_ULL(port);

	priv->r->vlan_set_untagged(vlan->vid, info.untagged_ports);
	pr_debug("Untagged ports, VLAN %d: %llx\n", vlan->vid, info.untagged_ports);

	priv->r->vlan_set_tagged(vlan->vid, &info);
	pr_debug("Tagged ports, VLAN %d: %llx\n", vlan->vid, info.tagged_ports);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtldsa_vlan_del(struct dsa_switch *ds, int port,
			   const struct switchdev_obj_port_vlan *vlan)
{
	struct rtl838x_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;
	u16 pvid;

	pr_debug("%s: port %d, vid %d, flags %x\n",
		__func__, port, vlan->vid, vlan->flags);

	/* Let no one mess with our special VLAN 0 */
	if (!vlan->vid) return 0;

	if (vlan->vid >= MAX_VLANS) {
		dev_err(priv->dev, "VLAN out of range: %d", vlan->vid);
		return -ENOTSUPP;
	}

	mutex_lock(&priv->reg_mutex);
	pvid = priv->ports[port].pvid;

	/* Reset to default if removing the current PVID */
	if (vlan->vid == pvid) {
		rtldsa_vlan_set_pvid(priv, port, 0);
	}
	/* Get port memberships of this vlan */
	priv->r->vlan_tables_read(vlan->vid, &info);

	/* remove port from both tables */
	info.untagged_ports &= (~BIT_ULL(port));
	info.tagged_ports &= (~BIT_ULL(port));

	priv->r->vlan_set_untagged(vlan->vid, info.untagged_ports);
	pr_debug("Untagged ports, VLAN %d: %llx\n", vlan->vid, info.untagged_ports);

	priv->r->vlan_set_tagged(vlan->vid, &info);
	pr_debug("Tagged ports, VLAN %d: %llx\n", vlan->vid, info.tagged_ports);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtldsa_setup_l2_uc_entry(struct rtl838x_l2_entry *e, int port,
				     int vid, u64 mac)
{
	memset(e, 0, sizeof(*e));

	e->type = L2_UNICAST;
	e->valid = true;

	e->age = 3;
	e->is_static = true;

	e->port = port;

	e->rvid = e->vid = vid;
	e->is_ip_mc = e->is_ipv6_mc = false;
	u64_to_ether_addr(mac, e->mac);
}

static void rtldsa_setup_l2_mc_entry(struct rtl838x_l2_entry *e, int vid, u64 mac, int mc_group)
{
	memset(e, 0, sizeof(*e));

	e->type = L2_MULTICAST;
	e->valid = true;

	e->mc_portmask_index = mc_group;

	e->rvid = e->vid = vid;
	e->is_ip_mc = e->is_ipv6_mc = false;
	u64_to_ether_addr(mac, e->mac);
}

/* Uses the seed to identify a hash bucket in the L2 using the derived hash key and then loops
 * over the entries in the bucket until either a matching entry is found or an empty slot
 * Returns the filled in rtl838x_l2_entry and the index in the bucket when an entry was found
 * when an empty slot was found and must exist is false, the index of the slot is returned
 * when no slots are available returns -1
 */
static int rtldsa_find_l2_hash_entry(struct rtl838x_switch_priv *priv, u64 seed,
				     bool must_exist, struct rtl838x_l2_entry *e)
{
	int idx = -1;
	u32 key = priv->r->l2_hash_key(priv, seed);
	u64 entry;

	pr_debug("%s: using key %x, for seed %016llx\n", __func__, key, seed);
	/* Loop over all entries in the hash-bucket and over the second block on 93xx SoCs */
	for (int i = 0; i < priv->l2_bucket_size; i++) {
		entry = priv->r->read_l2_entry_using_hash(key, i, e);
		pr_debug("valid %d, mac %016llx\n", e->valid, ether_addr_to_u64(&e->mac[0]));
		if (must_exist && !e->valid)
			continue;
		if (!e->valid || ((entry & 0x0fffffffffffffffULL) == seed)) {
			idx = i > 3 ? ((key >> 14) & 0xffff) | i >> 1 : ((key << 2) | i) & 0xffff;
			break;
		}
	}

	return idx;
}

/* Uses the seed to identify an entry in the CAM by looping over all its entries
 * Returns the filled in rtl838x_l2_entry and the index in the CAM when an entry was found
 * when an empty slot was found the index of the slot is returned
 * when no slots are available returns -1
 */
static int rtldsa_find_l2_cam_entry(struct rtl838x_switch_priv *priv, u64 seed,
				    bool must_exist, struct rtl838x_l2_entry *e)
{
	int idx = -1;
	u64 entry;

	for (int i = 0; i < 64; i++) {
		entry = priv->r->read_cam(i, e);
		if (!must_exist && !e->valid) {
			if (idx < 0) /* First empty entry? */
				idx = i;
			break;
		} else if ((entry & 0x0fffffffffffffffULL) == seed) {
			pr_debug("Found entry in CAM\n");
			idx = i;
			break;
		}
	}

	return idx;
}

static int rtldsa_port_fdb_add(struct dsa_switch *ds, int port,
			       const unsigned char *addr, u16 vid,
			       const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	u64 seed = priv->r->l2_hash_seed(mac, vid);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtldsa_find_l2_hash_entry(priv, seed, false, &e);

	/* Found an existing or empty entry */
	if (idx >= 0) {
		rtldsa_setup_l2_uc_entry(&e, port, vid, mac);
		priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		goto out;
	}

	/* Hash buckets full, try CAM */
	idx = rtldsa_find_l2_cam_entry(priv, seed, false, &e);

	if (idx >= 0) {
		rtldsa_setup_l2_uc_entry(&e, port, vid, mac);
		priv->r->write_cam(idx, &e);
		goto out;
	}

	err = -ENOTSUPP;

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtldsa_port_fdb_del(struct dsa_switch *ds, int port,
			       const unsigned char *addr, u16 vid,
			       const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	u64 seed = priv->r->l2_hash_seed(mac, vid);

	pr_debug("In %s, mac %llx, vid: %d\n", __func__, mac, vid);
	mutex_lock(&priv->reg_mutex);

	idx = rtldsa_find_l2_hash_entry(priv, seed, true, &e);

	if (idx >= 0) {
		pr_debug("Found entry index %d, key %d and bucket %d\n", idx, idx >> 2, idx & 3);
		e.valid = false;
		priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		goto out;
	}

	/* Check CAM for spillover from hash buckets */
	idx = rtldsa_find_l2_cam_entry(priv, seed, true, &e);

	if (idx >= 0) {
		e.valid = false;
		priv->r->write_cam(idx, &e);
		goto out;
	}
	err = -ENOENT;

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtldsa_port_fdb_dump(struct dsa_switch *ds, int port,
				dsa_fdb_dump_cb_t *cb, void *data)
{
	struct rtl838x_l2_entry e;
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);

	for (int i = 0; i < priv->fib_entries; i++) {
		priv->r->read_l2_entry_using_hash(i >> 2, i & 0x3, &e);

		if (!e.valid)
			continue;

		if (e.port == port || e.port == RTL930X_PORT_IGNORE)
			cb(e.mac, e.vid, e.is_static, data);

		if (!((i + 1) % 64))
			cond_resched();
	}

	for (int i = 0; i < 64; i++) {
		priv->r->read_cam(i, &e);

		if (!e.valid)
			continue;

		if (e.port == port)
			cb(e.mac, e.vid, e.is_static, data);
	}

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int
rtldsa_mdb_add_ports_l2_hash_update(struct rtl838x_switch_priv *priv,
				    struct rtl838x_l2_entry *entry, u64 seed,
				    u64 mac, u16 vid, int mc_pmask_idx,
				    int port)
{
	dev_dbg(priv->dev, "Found an existing entry %016llx, mc_group %d\n",
		ether_addr_to_u64(entry->mac), entry->mc_portmask_index);

	if (mc_pmask_idx < 0) {
		rtldsa_mc_group_add_port(priv, entry->mc_portmask_index, port);
		return 0;
	}

	if (entry->mc_portmask_index == mc_pmask_idx)
		return 0;

	dev_warn(priv->dev, "Found entry %016llx with unexpected pmsk-id: %d\n",
		 ether_addr_to_u64(entry->mac),
		 entry->mc_portmask_index);

	clear_bit(entry->mc_portmask_index, priv->mc_group_bm);
	entry->mc_portmask_index = mc_pmask_idx;

	return 0;
}

static int
rtldsa_mdb_add_ports_l2_hash_create(struct rtl838x_switch_priv *priv,
				    struct rtl838x_l2_entry *entry, u64 seed,
				    u64 mac, u16 vid, int mc_pmask_idx,
				    int port, int idx)
{
	dev_dbg(priv->dev, "New entry for seed %016llx\n", seed);

	if (mc_pmask_idx < 0) {
		/* ToDo: instead of always allocating a new multicast port mask,
		 * we could safe some of these by first searching for an
		 * existing, suitable multicast port mask entry. And if found
		 * share it between multiple L2 multicast entries that use
		 * the same set of ports.
		 */
		mc_pmask_idx = rtldsa_mc_group_alloc(priv, port);
		if (mc_pmask_idx < 0)
			return -ENOTSUPP;
	}

	rtldsa_setup_l2_mc_entry(entry, vid, mac, mc_pmask_idx);
	priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, entry);

	return 0;
}

static int
rtldsa_mdb_add_ports_l2_hash(struct rtl838x_switch_priv *priv, u64 seed,
			     u64 mac, u16 vid, int mc_pmask_idx, int port)
{
	struct rtl838x_l2_entry entry;
	int idx;

	idx = rtldsa_find_l2_hash_entry(priv, seed, false, &entry);
	if (idx < 0)
		return -ENOTSUPP;

	/* Found an existing or empty entry */
	if (entry.valid)
		return rtldsa_mdb_add_ports_l2_hash_update(priv, &entry, seed, mac,
							   vid, mc_pmask_idx, port);

	return rtldsa_mdb_add_ports_l2_hash_create(priv, &entry, seed, mac, vid,
						   mc_pmask_idx, port, idx);
}

static int
rtldsa_mdb_add_ports_l2_cam_update(struct rtl838x_switch_priv *priv,
				   struct rtl838x_l2_entry *entry, u64 seed,
				   u64 mac, u16 vid, int mc_pmask_idx,
				   int port)
{
	dev_warn(priv->dev, "Found existing CAM entry %016llx, mc_group %d\n",
		 ether_addr_to_u64(entry->mac), entry->mc_portmask_index);

	if (mc_pmask_idx < 0) {
		rtldsa_mc_group_add_port(priv, entry->mc_portmask_index, port);
		return 0;
	}

	if (entry->mc_portmask_index == mc_pmask_idx)
		return 0;

	dev_warn(priv->dev, "Found entry %016llx with unexpected pmsk-id: %d\n",
		 ether_addr_to_u64(entry->mac),
		 entry->mc_portmask_index);

	clear_bit(entry->mc_portmask_index, priv->mc_group_bm);
	entry->mc_portmask_index = mc_pmask_idx;

	return 0;
}

static int
rtldsa_mdb_add_ports_l2_cam_create(struct rtl838x_switch_priv *priv,
				   struct rtl838x_l2_entry *entry, u64 seed,
				   u64 mac, u16 vid, int mc_pmask_idx,
				   int port, int idx)
{
	dev_warn(priv->dev, "New entry\n");

	if (mc_pmask_idx < 0) {
		mc_pmask_idx = rtldsa_mc_group_alloc(priv, port);
		if (mc_pmask_idx < 0)
			return -ENOTSUPP;
	}

	rtldsa_setup_l2_mc_entry(entry, vid, mac, mc_pmask_idx);
	priv->r->write_cam(idx, entry);

	return 0;
}

static int
rtldsa_mdb_add_ports_l2_cam(struct rtl838x_switch_priv *priv, u64 seed,
			    u64 mac, u16 vid, int mc_pmask_idx, int port)
{
	struct rtl838x_l2_entry entry;
	int idx;

	idx = rtldsa_find_l2_cam_entry(priv, seed, false, &entry);
	if (idx < 0)
		return -ENOTSUPP;

	if (entry.valid)
		return rtldsa_mdb_add_ports_l2_cam_update(priv, &entry, seed, mac,
							  vid, mc_pmask_idx, port);

	return rtldsa_mdb_add_ports_l2_cam_create(priv, &entry, seed, mac, vid,
						  mc_pmask_idx, port, idx);
}

static int
rtldsa_mdb_add_ports(struct rtl838x_switch_priv *priv, u64 mac, u16 vid,
		     int mc_pmask_idx, int port)
{
	u64 seed = priv->r->l2_hash_seed(mac, vid);
	int err;

	if (mc_pmask_idx >= 0 && port >= 0) {
		dev_err(priv->dev, "Both port mask index and specific port given.");
		return -EINVAL;
	}

	err = rtldsa_mdb_add_ports_l2_hash(priv, seed, mac, vid, mc_pmask_idx, port);
	if (!err)
		return 0;

	/* Hash buckets full, try CAM */
	return rtldsa_mdb_add_ports_l2_cam(priv, seed, mac, vid, mc_pmask_idx, port);
}

static int
rtldsa_mdb_add_all_ports(struct rtl838x_switch_priv *priv, u64 mac, u16 vid)
{
	if (priv->id >= RTL9300_FAMILY_ID)
		return -EOPNOTSUPP;

	return rtldsa_mdb_add_ports(priv, mac, vid, MC_PMASK_ALL_PORTS_IDX, -1);
}

static bool rtldsa_mac_is_unsnoop(const unsigned char *addr)
{
	/*
	 * RFC4541, section 2.1.2.2 + section 3:
	 * Unsnoopable address ranges must always be flooded.
	 *
	 * mapped MAC for 224.0.0.x -> 01:00:5e:00:00:xx
	 * mapped MAC for ff02::1 -> 33:33:00:00:00:01
	 */
	if (ether_addr_equal_masked(addr, ipv4_ll_mcast_addr_base,
				    ipv4_ll_mcast_addr_mask) ||
	    ether_addr_equal_masked(addr, ipv6_all_hosts_mcast_addr_base,
				    ipv6_all_hosts_mcast_addr_mask))
		return true;

	return false;
}

static bool rtldsa_mdb_is_active(struct rtl838x_switch_priv *priv,
				 const struct switchdev_obj_port_mdb *mdb)
{
	struct rtl838x_vlan_info info;

	if (mdb->vid >= MAX_VLANS)
		return false;

	priv->r->vlan_tables_read(mdb->vid, &info);

	if (ether_addr_is_ipv4_mcast(mdb->addr) &&
	    !(info.profile_id & RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4))
		return false;

	if (ether_addr_is_ipv6_mcast(mdb->addr) &&
	    !(info.profile_id & RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6))
		return false;

	return true;
}

static int rtldsa_port_mdb_add_checks(struct rtl838x_switch_priv *priv,
				      int port,
				      const struct switchdev_obj_port_mdb *mdb)
{
	if (priv->id >= RTL9300_FAMILY_ID)
		return -EOPNOTSUPP;

	dev_dbg(priv->dev, "In %s port %d, mac %pM, vid: %d\n", __func__,
		port, mdb->addr, mdb->vid);

	if (priv->is_lagmember[port]) {
		dev_dbg(priv->dev, "%s: %d is lag slave. ignore\n", __func__, port);
		return -EINVAL;
	}

	if (rtldsa_mac_is_unsnoop(mdb->addr)) {
		dev_dbg(priv->dev,
			"%s: %pM might belong to an unsnoopable IP. ignore\n",
			__func__, mdb->addr);
		return -EADDRNOTAVAIL;
	}

	return 0;
}

static int __rtldsa_port_mdb_add(struct dsa_switch *ds, int port,
				 const struct switchdev_obj_port_mdb *mdb,
				 const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(mdb->addr);
	int vid = mdb->vid;
	int err;

	dev_dbg(priv->dev, "In %s port %d, mac %llx, vid: %d\n", __func__, port, mac, vid);

	lockdep_assert_held_once(&priv->reg_mutex);

	if (!rtldsa_mdb_is_active(priv, mdb))
		return 0;

	err = rtldsa_mdb_add_ports(priv, mac, vid, -1, port);
	if (err)
		dev_err(ds->dev, "failed to add MDB entry\n");

	return err;
}

static int rtldsa_port_mdb_add(struct dsa_switch *ds, int port,
			       const struct switchdev_obj_port_mdb *mdb,
			       const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int err;

	err = rtldsa_port_mdb_add_checks(priv, port, mdb);

	if (err == -EADDRNOTAVAIL)
		return 0;
	else if (err < 0)
		return err;

	mutex_lock(&priv->reg_mutex);
	err = __rtldsa_port_mdb_add(ds, port, mdb, db);
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static void
rtldsa_port_mdb_del_l2_hash_entry(struct rtl838x_switch_priv *priv,
				  struct rtl838x_l2_entry *e, int idx,
				  u64 portmask)
{
	dev_dbg(priv->dev, "Found entry index %d, key %d and bucket %d\n",
		idx, idx >> 2, idx & 3);

	portmask = rtldsa_mc_group_del_ports(priv, e->mc_portmask_index, portmask);
	if (portmask)
		return;

	e->valid = false;
	priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, e);
}

static int
rtldsa_port_mdb_del_l2_hash(struct rtl838x_switch_priv *priv, u64 seed, int port)
{
	struct rtl838x_l2_entry e;
	int idx;

	idx = rtldsa_find_l2_hash_entry(priv, seed, true, &e);
	if (idx < 0)
		return -ENOTSUPP;

	rtldsa_port_mdb_del_l2_hash_entry(priv, &e, idx, BIT_ULL(port));
	return 0;
}

static void
rtldsa_port_mdb_del_l2_cam_entry(struct rtl838x_switch_priv *priv,
				 struct rtl838x_l2_entry *e, int idx,
				 u64 portmask)
{
	dev_dbg(priv->dev, "Found entry index %d, key %d and bucket %d\n",
		idx, idx >> 2, idx & 3);

	portmask = rtldsa_mc_group_del_ports(priv, e->mc_portmask_index, portmask);
	if (portmask)
		return;

	e->valid = false;
	priv->r->write_cam(idx, e);
}

static int
rtldsa_port_mdb_del_l2_cam(struct rtl838x_switch_priv *priv, u64 seed, int port)
{
	struct rtl838x_l2_entry e;
	int idx;

	idx = rtldsa_find_l2_hash_entry(priv, seed, true, &e);
	if (idx < 0)
		return -ENOTSUPP;

	rtldsa_port_mdb_del_l2_cam_entry(priv, &e, idx, BIT_ULL(port));
	return 0;
}

static int rtldsa_port_mdb_del(struct dsa_switch *ds, int port,
			       const struct switchdev_obj_port_mdb *mdb,
			       const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(mdb->addr);
	int vid = mdb->vid;
	u64 seed = priv->r->l2_hash_seed(mac, vid);
	int err = 0;

	dev_dbg(priv->dev, "In %s, port %d, mac %llx, vid: %d\n", __func__,
		port, mac, vid);

	if (priv->is_lagmember[port]) {
		pr_warn("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	if (rtldsa_mac_is_unsnoop(mdb->addr)) {
		dev_dbg(priv->dev,
			"%s: %pM might belong to an unsnoopable IP. ignore\n",
			__func__, mdb->addr);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);

	err = rtldsa_port_mdb_del_l2_hash(priv, seed, port);
	if (!err)
		goto out;

	/* Check CAM for spillover from hash buckets */
	err = rtldsa_port_mdb_del_l2_cam(priv, seed, port);

	/* TODO: Re-enable with a newer kernel: err = -ENOENT; */

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtldsa_mc_add_unsnoop_v4(struct rtl838x_switch_priv *priv, u16 vid)
{
	u8 addr[ETH_ALEN];
	int i, ret;

	memcpy(addr, ipv4_ll_mcast_addr_base, ETH_ALEN);

	/* ToDo: use IP_MULTICAST instead of L2_MULTICAST entries */
	for (i = 0; i <= U8_MAX; i++) {
		ret = rtldsa_mdb_add_all_ports(priv, ether_addr_to_u64(addr), vid);
		if (ret < 0)
			break;

		addr[5]++;
	}

	return ret;
}

static int rtldsa_mc_add_unsnoop_v6(struct rtl838x_switch_priv *priv, u16 vid)
{
	u64 addr = ether_addr_to_u64(ipv6_all_hosts_mcast_addr_base);

	/* ToDo: (maybe) use IP6_MULTICAST instead of L2_MULTICAST entries */
	return rtldsa_mdb_add_all_ports(priv, addr, vid);
}

static int __rtldsa_mc_add_unsnoop(struct rtl838x_switch_priv *priv,
				   const struct switchdev_mc_active mc_active,
				   u16 vid)
{
	if (mc_active.ip4_changed && mc_active.ip4)
		rtldsa_mc_add_unsnoop_v4(priv, vid);

	if (mc_active.ip6_changed && mc_active.ip6)
		rtldsa_mc_add_unsnoop_v6(priv, vid);

	return 0;
}

static int rtldsa_mc_add_unsnoop(struct rtl838x_switch_priv *priv,
				 const struct switchdev_mc_active mc_active)
{
	struct rtl838x_vlan_info info;
	int i;

	/* bridge multicast vlan snooping disabled, all VIDs */
	if (mc_active.vid < 0) {
		for (i = 1; i < MAX_VLANS; i++) {
			priv->r->vlan_tables_read(i, &info);

			if (!info.tagged_ports)
				continue;

			__rtldsa_mc_add_unsnoop(priv, mc_active, i);
		}
	/* bridge multicast vlan snooping enabled, specific VID */
	} else {
		__rtldsa_mc_add_unsnoop(priv, mc_active, mc_active.vid);
	}

	return 0;
}

static void rtldsa_port_mdb_snoop_flush(struct rtl838x_switch_priv *priv,
					bool ip6, short vid, u64 portmask)
{
	struct rtl838x_l2_entry e;
	int bucket, index;

	for (int i = 0; i < priv->fib_entries; i++) {
		bucket = i >> 2;
		index = i & 0x3;
		priv->r->read_l2_entry_using_hash(bucket, index, &e);

		if (!e.valid || e.type != L2_MULTICAST ||
		    (vid >= 0 && e.vid != vid))
			continue;

		if ((!ip6 && !ether_addr_is_ipv4_mcast(e.mac)) ||
		    (ip6 && !ether_addr_is_ipv6_mcast(e.mac)))
			continue;

		rtldsa_port_mdb_del_l2_hash_entry(priv, &e, i, portmask);
	}

	for (int i = 0; i < 64; i++) {
		priv->r->read_cam(i, &e);

		if (!e.valid || e.type != L2_MULTICAST ||
		    (vid >= 0 && e.vid != vid))
			continue;

		if ((!ip6 && !ether_addr_is_ipv4_mcast(e.mac)) ||
		    (ip6 && !ether_addr_is_ipv6_mcast(e.mac)))
			continue;

		rtldsa_port_mdb_del_l2_cam_entry(priv, &e, i, portmask);
	}
}

static void
rtldsa_port_mdb_snoop_flush_v4(struct rtl838x_switch_priv *priv, short vid,
			       u64 portmask)
{
	rtldsa_port_mdb_snoop_flush(priv, false, vid, portmask);
}

static void
rtldsa_port_mdb_snoop_flush_v6(struct rtl838x_switch_priv *priv, short vid,
			       u64 portmask)
{
	rtldsa_port_mdb_snoop_flush(priv, true, vid, portmask);
}

static int
rtldsa_port_replay_switchdev_objs(struct notifier_block *nb,
				  unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
	struct switchdev_notifier_port_obj_info *port_obj_info = ptr;
	const struct switchdev_obj_port_mdb *mdb;
	const struct dsa_db db = { 0 };
	const struct dsa_port *dp;
	int err;

	if (event != SWITCHDEV_PORT_OBJ_ADD || !dev)
		return 0;

	switch (port_obj_info->obj->id) {
	case SWITCHDEV_OBJ_ID_PORT_MDB:
	case SWITCHDEV_OBJ_ID_HOST_MDB:
		break;
	default:
		return 0;
	}

	dp = port_obj_info->info.ctx;
	mdb = SWITCHDEV_OBJ_PORT_MDB(port_obj_info->obj);

	err = rtldsa_port_mdb_add_checks(dp->ds->priv, dp->index, mdb);
	if (err < 0)
		return 0;

	__rtldsa_port_mdb_add(dp->ds, dp->index, mdb, db);

	return 0;
}

static struct notifier_block rtldsa_port_replay_switchdev_objs_nb = {
	.notifier_call = rtldsa_port_replay_switchdev_objs,
};

static void rtldsa_port_mdb_snoop_replay(struct rtl838x_switch_priv *priv)
{
	struct net_device *dev = to_net_dev(priv->dev);
	struct notifier_block *nb = &rtldsa_port_replay_switchdev_objs_nb;

	for (int i = 0; i < priv->cpu_port; i++) {
		dev = priv->ports[i].dp->user;
		if (!dev)
			continue;

		switchdev_bridge_port_replay(dev, dev, priv->ports[i].dp, NULL, nb, NULL);
	}
}

static void
rtldsa_port_mdb_update_entries(struct rtl838x_switch_priv *priv,
			       const struct switchdev_mc_active mc_active)
{
	if (mc_active.ip4_changed && !mc_active.ip4)
		rtldsa_port_mdb_snoop_flush_v4(priv, mc_active.vid, ~(0ULL));

	if (mc_active.ip6_changed && !mc_active.ip6)
		rtldsa_port_mdb_snoop_flush_v6(priv, mc_active.vid, ~(0ULL));

	rtldsa_mc_add_unsnoop(priv, mc_active);

	if ((mc_active.ip4_changed && mc_active.ip4) ||
	    (mc_active.ip6_changed && mc_active.ip6))
		rtldsa_port_mdb_snoop_replay(priv);
}

static void
rtldsa_port_mdb_update_flooding(struct rtl838x_switch_priv *priv,
				const struct switchdev_mc_active mc_active)
{
	int profile_id = 0;
	struct rtl838x_vlan_info info;
	int i;

	if (mc_active.ip4)
		profile_id |= RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4;
	if (mc_active.ip6)
		profile_id |= RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6;

	/* bridge multicast vlan snooping disabled, all VIDs */
	if (mc_active.vid < 0) {
		for (i = 1; i < MAX_VLANS; i++) {
			priv->r->vlan_tables_read(i, &info);

			if (!info.tagged_ports)
				continue;

			info.profile_id = profile_id;
			priv->r->vlan_set_tagged(i, &info);
		}
	/* bridge multicast vlan snooping enabled, specific VID */
	} else {
		priv->r->vlan_tables_read(mc_active.vid, &info);
		info.profile_id = profile_id;
		priv->r->vlan_set_tagged(mc_active.vid, &info);
	}
}

static int rtldsa_port_mdb_active(struct dsa_switch *ds, int port,
				  const struct switchdev_mc_active mc_active,
				  struct netlink_ext_ack *extack, bool handled)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (mc_active.vid >= MAX_VLANS)
		return -EINVAL;

	if (handled)
		return 0;

	mutex_lock(&priv->reg_mutex);

	rtldsa_port_mdb_update_flooding(priv, mc_active);
	rtldsa_port_mdb_update_entries(priv, mc_active);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtldsa_mc_group_add_mrouter(struct rtl838x_switch_priv *priv, int port)
{
	u64 portmask = BIT_ULL(port);

	if (portmask & priv->mc_router_portmask)
		return;

	priv->mc_router_portmask |= BIT_ULL(port);
}

static void rtldsa_mc_group_del_mrouter(struct rtl838x_switch_priv *priv, int port)
{
	u64 portmask = BIT_ULL(port);

	if (!(portmask & priv->mc_router_portmask))
		return;

	priv->mc_router_portmask &= ~BIT_ULL(port);

	rtldsa_port_mdb_snoop_flush_v4(priv, -1, portmask);
	rtldsa_port_mdb_snoop_flush_v6(priv, -1, portmask);
}

static void
rtldsa_port_mdb_update_unknown_ip_flood(struct rtl838x_switch_priv *priv)
{
	switch (priv->family_id) {
	case RTL8380_FAMILY_ID:
	case RTL8390_FAMILY_ID:
		rtl83xx_mc_pmasks_setup(priv);
		break;
	case RTL9300_FAMILY_ID:
	case RTL9310_FAMILY_ID:
		priv->r->vlan_profile_setup(priv, RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4);
		priv->r->vlan_profile_setup(priv, RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6);
		priv->r->vlan_profile_setup(priv, RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4 |
						  RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6);
		break;
	default:
		dev_err(priv->dev, "%s: unknown family_id %u\n", __func__,
			priv->family_id);
		break;
	}
}

static int
rtldsa_port_mdb_set_mrouter(struct dsa_switch *ds, int port, bool mrouter,
			    struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);

	if (mrouter)
		rtldsa_mc_group_add_mrouter(priv, port);
	else
		rtldsa_mc_group_del_mrouter(priv, port);

	rtldsa_port_mdb_snoop_replay(priv);
	rtldsa_port_mdb_update_unknown_ip_flood(priv);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtldsa_port_mirror_add(struct dsa_switch *ds, int port,
				   struct dsa_mall_mirror_tc_entry *mirror,
				   bool ingress, struct netlink_ext_ack *extack)
{
	/* We support 4 mirror groups, one destination port per group */
	int group;
	struct rtl838x_switch_priv *priv = ds->priv;
	int ctrl_reg, dpm_reg, spm_reg;

	pr_debug("In %s\n", __func__);

	for (group = 0; group < 4; group++) {
		if (priv->mirror_group_ports[group] == mirror->to_local_port)
			break;
	}
	if (group >= 4) {
		for (group = 0; group < 4; group++) {
			if (priv->mirror_group_ports[group] < 0)
				break;
		}
	}

	if (group >= 4)
		return -ENOSPC;

	ctrl_reg = priv->r->mir_ctrl + group * 4;
	dpm_reg = priv->r->mir_dpm + group * 4 * priv->port_width;
	spm_reg = priv->r->mir_spm + group * 4 * priv->port_width;

	pr_debug("Using group %d\n", group);
	mutex_lock(&priv->reg_mutex);

	if (priv->family_id == RTL8380_FAMILY_ID) {
		/* Enable mirroring to port across VLANs (bit 11) */
		sw_w32(1 << 11 | (mirror->to_local_port << 4) | 1, ctrl_reg);
	} else {
		/* Enable mirroring to destination port */
		sw_w32((mirror->to_local_port << 4) | 1, ctrl_reg);
	}

	if (ingress && (priv->r->get_port_reg_be(spm_reg) & (1ULL << port))) {
		mutex_unlock(&priv->reg_mutex);
		return -EEXIST;
	}
	if ((!ingress) && (priv->r->get_port_reg_be(dpm_reg) & (1ULL << port))) {
		mutex_unlock(&priv->reg_mutex);
		return -EEXIST;
	}

	if (ingress)
		priv->r->mask_port_reg_be(0, 1ULL << port, spm_reg);
	else
		priv->r->mask_port_reg_be(0, 1ULL << port, dpm_reg);

	priv->mirror_group_ports[group] = mirror->to_local_port;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtldsa_port_mirror_del(struct dsa_switch *ds, int port,
				    struct dsa_mall_mirror_tc_entry *mirror)
{
	int group = 0;
	struct rtl838x_switch_priv *priv = ds->priv;
	int ctrl_reg, dpm_reg, spm_reg;

	pr_debug("In %s\n", __func__);
	for (group = 0; group < 4; group++) {
		if (priv->mirror_group_ports[group] == mirror->to_local_port)
			break;
	}
	if (group >= 4)
		return;

	ctrl_reg = priv->r->mir_ctrl + group * 4;
	dpm_reg = priv->r->mir_dpm + group * 4 * priv->port_width;
	spm_reg = priv->r->mir_spm + group * 4 * priv->port_width;

	mutex_lock(&priv->reg_mutex);
	if (mirror->ingress) {
		/* Ingress, clear source port matrix */
		priv->r->mask_port_reg_be(1ULL << port, 0, spm_reg);
	} else {
		/* Egress, clear destination port matrix */
		priv->r->mask_port_reg_be(1ULL << port, 0, dpm_reg);
	}

	if (!(sw_r32(spm_reg) || sw_r32(dpm_reg))) {
		priv->mirror_group_ports[group] = -1;
		sw_w32(0, ctrl_reg);
	}

	mutex_unlock(&priv->reg_mutex);
}

static int rtldsa_port_pre_bridge_flags(struct dsa_switch *ds, int port,
					struct switchdev_brport_flags flags,
					struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	unsigned long features = 0;
	pr_debug("%s: %d %lX\n", __func__, port, flags.val);
	if (priv->r->enable_learning)
		features |= BR_LEARNING;
	if (priv->r->enable_flood)
		features |= BR_FLOOD;
	if (priv->r->enable_mcast_flood)
		features |= BR_MCAST_FLOOD;
	if (priv->r->enable_bcast_flood)
		features |= BR_BCAST_FLOOD;
	if (flags.mask & ~(features))
		return -EINVAL;

	return 0;
}

static int rtldsa_port_bridge_flags(struct dsa_switch *ds, int port,
				    struct switchdev_brport_flags flags,
				    struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s: %d %lX\n", __func__, port, flags.val);
	if (priv->r->enable_learning && (flags.mask & BR_LEARNING))
		priv->r->enable_learning(port, !!(flags.val & BR_LEARNING));

	if (priv->r->enable_flood && (flags.mask & BR_FLOOD))
		priv->r->enable_flood(port, !!(flags.val & BR_FLOOD));

	if (priv->r->enable_mcast_flood && (flags.mask & BR_MCAST_FLOOD))
		priv->r->enable_mcast_flood(port, !!(flags.val & BR_MCAST_FLOOD));

	if (priv->r->enable_bcast_flood && (flags.mask & BR_BCAST_FLOOD))
		priv->r->enable_bcast_flood(port, !!(flags.val & BR_BCAST_FLOOD));

	return 0;
}

static bool rtl83xx_lag_can_offload(struct dsa_switch *ds,
				      struct net_device *lag,
				      struct netdev_lag_upper_info *info)
{
	int id;

	id = dsa_lag_id(ds->dst, lag);
	if (id < 0 || id >= ds->num_lag_ids)
		return false;

	if (info->tx_type != NETDEV_LAG_TX_TYPE_HASH) {
		return false;
	}
	if (info->hash_type != NETDEV_LAG_HASH_L2 && info->hash_type != NETDEV_LAG_HASH_L23)
		return false;

	return true;
}

static int rtldsa_port_lag_change(struct dsa_switch *ds, int port)
{
	pr_debug("%s: %d\n", __func__, port);
	/* Nothing to be done... */

	return 0;
}

static int rtldsa_port_lag_join(struct dsa_switch *ds,
				int port,
				struct dsa_lag lag,
				struct netdev_lag_upper_info *info,
				struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int i, err = 0;

	if (!rtl83xx_lag_can_offload(ds, lag.dev, info))
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);

	for (i = 0; i < priv->n_lags; i++) {
		if ((!priv->lag_devs[i]) || (priv->lag_devs[i] == lag.dev))
			break;
	}
	if (port >= priv->cpu_port) {
		err = -EINVAL;
		goto out;
	}
	pr_info("port_lag_join: group %d, port %d\n",i, port);
	if (!priv->lag_devs[i])
		priv->lag_devs[i] = lag.dev;

	if (priv->lag_primary[i] == -1) {
		priv->lag_primary[i] = port;
	} else
		priv->is_lagmember[port] = 1;

	priv->lagmembers |= (1ULL << port);

	pr_debug("lag_members = %llX\n", priv->lagmembers);
	err = rtl83xx_lag_add(priv->ds, i, port, info);
	if (err) {
		err = -EINVAL;
		goto out;
	}

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtldsa_port_lag_leave(struct dsa_switch *ds, int port,
				 struct dsa_lag lag)
{
	int i, group = -1, err;
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);
	for (i = 0; i < priv->n_lags; i++) {
		if (priv->lags_port_members[i] & BIT_ULL(port)) {
			group = i;
			break;
		}
	}

	if (group == -1) {
		pr_warn("port_lag_leave: port %d is not a member\n", port);
		err = -EINVAL;
		goto out;
	}

	if (port >= priv->cpu_port) {
		err = -EINVAL;
		goto out;
	}
	pr_info("port_lag_del: group %d, port %d\n",group, port);
	priv->lagmembers &=~ (1ULL << port);
	priv->lag_primary[i] = -1;
	priv->is_lagmember[port] = 0;
	pr_debug("lag_members = %llX\n", priv->lagmembers);
	err = rtl83xx_lag_del(priv->ds, group, port);
	if (err) {
		err = -EINVAL;
		goto out;
	}
	if (!priv->lags_port_members[i])
		priv->lag_devs[i] = NULL;

out:
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

int dsa_phy_read(struct dsa_switch *ds, int phy_addr, int phy_reg)
{
	u32 val;
	u32 offset = 0;
	struct rtl838x_switch_priv *priv = ds->priv;

	if ((phy_addr >= 24) &&
	    (phy_addr <= 27) &&
	    (priv->ports[24].phy == PHY_RTL838X_SDS)) {
		if (phy_addr == 26)
			offset = 0x100;
		val = sw_r32(RTL838X_SDS4_FIB_REG0 + offset + (phy_reg << 2)) & 0xffff;
		return val;
	}

	read_phy(phy_addr, 0, phy_reg, &val);
	return val;
}

int dsa_phy_write(struct dsa_switch *ds, int phy_addr, int phy_reg, u16 val)
{
	u32 offset = 0;
	struct rtl838x_switch_priv *priv = ds->priv;

	if ((phy_addr >= 24) &&
	    (phy_addr <= 27) &&
	    (priv->ports[24].phy == PHY_RTL838X_SDS)) {
		if (phy_addr == 26)
			offset = 0x100;
		sw_w32(val, RTL838X_SDS4_FIB_REG0 + offset + (phy_reg << 2));
		return 0;
	}
	return write_phy(phy_addr, 0, phy_reg, val);
}

const struct phylink_pcs_ops rtl83xx_pcs_ops = {
	.pcs_an_restart		= rtldsa_pcs_an_restart,
 	.pcs_validate		= rtl83xx_pcs_validate,
 	.pcs_get_state		= rtl83xx_pcs_get_state,
	.pcs_config		= rtldsa_pcs_config,
};

const struct dsa_switch_ops rtl83xx_switch_ops = {
	.get_tag_protocol	= rtldsa_get_tag_protocol,
	.setup			= rtl83xx_setup,

	.phy_read		= dsa_phy_read,
	.phy_write		= dsa_phy_write,

	.phylink_get_caps	= rtldsa_phylink_get_caps,
	.phylink_mac_config	= rtldsa_phylink_mac_config,
	.phylink_mac_link_down	= rtldsa_phylink_mac_link_down,
	.phylink_mac_link_up	= rtldsa_phylink_mac_link_up,
	.phylink_mac_select_pcs	= rtldsa_phylink_mac_select_pcs,

	.get_strings		= rtldsa_get_strings,
	.get_ethtool_stats	= rtldsa_get_ethtool_stats,
	.get_sset_count		= rtldsa_get_sset_count,
	.get_eth_phy_stats	= rtl83xx_get_eth_phy_stats,
	.get_eth_mac_stats	= rtl83xx_get_eth_mac_stats,
	.get_eth_ctrl_stats	= rtl83xx_get_eth_ctrl_stats,
	.get_rmon_stats		= rtl83xx_get_rmon_stats,
	.get_stats64		= rtl83xx_get_stats64,
	.get_pause_stats	= rtl83xx_get_pause_stats,

	.port_enable		= rtldsa_port_enable,
	.port_disable		= rtldsa_port_disable,

	.get_mac_eee		= rtldsa_get_mac_eee,
	.set_mac_eee		= rtldsa_set_mac_eee,

	.set_ageing_time	= rtldsa_set_ageing_time,
	.port_bridge_join	= rtldsa_port_bridge_join,
	.port_bridge_leave	= rtldsa_port_bridge_leave,
	.port_stp_state_set	= rtldsa_port_stp_state_set,
	.port_fast_age		= rtldsa_fast_age,

	.port_vlan_filtering	= rtldsa_vlan_filtering,
	.port_vlan_add		= rtldsa_vlan_add,
	.port_vlan_del		= rtldsa_vlan_del,

	.port_fdb_add		= rtldsa_port_fdb_add,
	.port_fdb_del		= rtldsa_port_fdb_del,
	.port_fdb_dump		= rtldsa_port_fdb_dump,

	.port_mdb_add		= rtldsa_port_mdb_add,
	.port_mdb_del		= rtldsa_port_mdb_del,
	.port_mdb_active	= rtldsa_port_mdb_active,
	.port_mdb_set_mrouter	= rtldsa_port_mdb_set_mrouter,

	.port_mirror_add	= rtldsa_port_mirror_add,
	.port_mirror_del	= rtldsa_port_mirror_del,

	.port_lag_change	= rtldsa_port_lag_change,
	.port_lag_join		= rtldsa_port_lag_join,
	.port_lag_leave		= rtldsa_port_lag_leave,

	.port_pre_bridge_flags	= rtldsa_port_pre_bridge_flags,
	.port_bridge_flags	= rtldsa_port_bridge_flags,
};

const struct phylink_pcs_ops rtl93xx_pcs_ops = {
	.pcs_an_restart		= rtldsa_pcs_an_restart,
 	.pcs_validate		= rtl93xx_pcs_validate,
 	.pcs_get_state		= rtl93xx_pcs_get_state,
	.pcs_config		= rtldsa_pcs_config,
};

const struct dsa_switch_ops rtl930x_switch_ops = {
	.get_tag_protocol	= rtldsa_get_tag_protocol,
	.setup			= rtl93xx_setup,

	.phy_read		= dsa_phy_read,
	.phy_write		= dsa_phy_write,

	.phylink_get_caps	= rtldsa_phylink_get_caps,
	.phylink_mac_config	= rtldsa_phylink_mac_config,
	.phylink_mac_link_down	= rtldsa_phylink_mac_link_down,
	.phylink_mac_link_up	= rtldsa_phylink_mac_link_up,
	.phylink_mac_select_pcs	= rtldsa_phylink_mac_select_pcs,

	.get_strings		= rtldsa_get_strings,
	.get_ethtool_stats	= rtldsa_get_ethtool_stats,
	.get_sset_count		= rtldsa_get_sset_count,
	.get_eth_phy_stats	= rtl83xx_get_eth_phy_stats,
	.get_eth_mac_stats	= rtl83xx_get_eth_mac_stats,
	.get_eth_ctrl_stats	= rtl83xx_get_eth_ctrl_stats,
	.get_rmon_stats		= rtl83xx_get_rmon_stats,
	.get_stats64		= rtl83xx_get_stats64,
	.get_pause_stats	= rtl83xx_get_pause_stats,

	.port_enable		= rtldsa_port_enable,
	.port_disable		= rtldsa_port_disable,

	.get_mac_eee		= rtldsa_get_mac_eee,
	.set_mac_eee		= rtldsa_set_mac_eee,

	.set_ageing_time	= rtldsa_set_ageing_time,
	.port_bridge_join	= rtldsa_port_bridge_join,
	.port_bridge_leave	= rtldsa_port_bridge_leave,
	.port_stp_state_set	= rtldsa_port_stp_state_set,
	.port_fast_age		= rtldsa_fast_age,

	.port_vlan_filtering	= rtldsa_vlan_filtering,
	.port_vlan_add		= rtldsa_vlan_add,
	.port_vlan_del		= rtldsa_vlan_del,

	.port_fdb_add		= rtldsa_port_fdb_add,
	.port_fdb_del		= rtldsa_port_fdb_del,
	.port_fdb_dump		= rtldsa_port_fdb_dump,

	.port_mdb_add		= rtldsa_port_mdb_add,
	.port_mdb_del		= rtldsa_port_mdb_del,
	.port_mdb_active	= rtldsa_port_mdb_active,
	.port_mdb_set_mrouter	= rtldsa_port_mdb_set_mrouter,

	.port_mirror_add	= rtldsa_port_mirror_add,
	.port_mirror_del	= rtldsa_port_mirror_del,

	.port_lag_change	= rtldsa_port_lag_change,
	.port_lag_join		= rtldsa_port_lag_join,
	.port_lag_leave		= rtldsa_port_lag_leave,

	.port_pre_bridge_flags	= rtldsa_port_pre_bridge_flags,
	.port_bridge_flags	= rtldsa_port_bridge_flags,
};
