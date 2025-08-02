// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>

#include "rtl83xx.h"

#define RTL931X_VLAN_PORT_TAG_STS_INTERNAL			0x0
#define RTL931X_VLAN_PORT_TAG_STS_UNTAG				0x1
#define RTL931X_VLAN_PORT_TAG_STS_TAGGED			0x2
#define RTL931X_VLAN_PORT_TAG_STS_PRIORITY_TAGGED		0x3

#define RTL931X_VLAN_PORT_TAG_CTRL_BASE				0x4860
/* port 0-56 */
#define RTL931X_VLAN_PORT_TAG_CTRL(port) \
		RTL931X_VLAN_PORT_TAG_CTRL_BASE + (port << 2)
#define RTL931X_VLAN_PORT_TAG_EGR_OTAG_STS_MASK			GENMASK(13,12)
#define RTL931X_VLAN_PORT_TAG_EGR_ITAG_STS_MASK			GENMASK(11,10)
#define RTL931X_VLAN_PORT_TAG_EGR_OTAG_KEEP_MASK		GENMASK(9,9)
#define RTL931X_VLAN_PORT_TAG_EGR_ITAG_KEEP_MASK		GENMASK(8,8)
#define RTL931X_VLAN_PORT_TAG_IGR_OTAG_KEEP_MASK		GENMASK(7,7)
#define RTL931X_VLAN_PORT_TAG_IGR_ITAG_KEEP_MASK		GENMASK(6,6)
#define RTL931X_VLAN_PORT_TAG_OTPID_IDX_MASK			GENMASK(5,4)
#define RTL931X_VLAN_PORT_TAG_OTPID_KEEP_MASK			GENMASK(3,3)
#define RTL931X_VLAN_PORT_TAG_ITPID_IDX_MASK			GENMASK(2,1)
#define RTL931X_VLAN_PORT_TAG_ITPID_KEEP_MASK			GENMASK(0,0)

extern struct mutex smi_lock;
extern struct rtl83xx_soc_info soc_info;

/* Definition of the RTL931X-specific template field IDs as used in the PIE */
enum template_field_id {
	TEMPLATE_FIELD_SPM0 = 1,
	TEMPLATE_FIELD_SPM1 = 2,
	TEMPLATE_FIELD_SPM2 = 3,
	TEMPLATE_FIELD_SPM3 = 4,
	TEMPLATE_FIELD_DMAC0 = 9,
	TEMPLATE_FIELD_DMAC1 = 10,
	TEMPLATE_FIELD_DMAC2 = 11,
	TEMPLATE_FIELD_SMAC0 = 12,
	TEMPLATE_FIELD_SMAC1 = 13,
	TEMPLATE_FIELD_SMAC2 = 14,
	TEMPLATE_FIELD_ETHERTYPE = 15,
	TEMPLATE_FIELD_OTAG = 16,
	TEMPLATE_FIELD_ITAG = 17,
	TEMPLATE_FIELD_SIP0 = 18,
	TEMPLATE_FIELD_SIP1 = 19,
	TEMPLATE_FIELD_DIP0 = 20,
	TEMPLATE_FIELD_DIP1 = 21,
	TEMPLATE_FIELD_IP_TOS_PROTO = 22,
	TEMPLATE_FIELD_L4_SPORT = 23,
	TEMPLATE_FIELD_L4_DPORT = 24,
	TEMPLATE_FIELD_L34_HEADER = 25,
	TEMPLATE_FIELD_TCP_INFO = 26,
	TEMPLATE_FIELD_SIP2 = 34,
	TEMPLATE_FIELD_SIP3 = 35,
	TEMPLATE_FIELD_SIP4 = 36,
	TEMPLATE_FIELD_SIP5 = 37,
	TEMPLATE_FIELD_SIP6 = 38,
	TEMPLATE_FIELD_SIP7 = 39,
	TEMPLATE_FIELD_DIP2 = 42,
	TEMPLATE_FIELD_DIP3 = 43,
	TEMPLATE_FIELD_DIP4 = 44,
	TEMPLATE_FIELD_DIP5 = 45,
	TEMPLATE_FIELD_DIP6 = 46,
	TEMPLATE_FIELD_DIP7 = 47,
	TEMPLATE_FIELD_FLOW_LABEL = 49,
	TEMPLATE_FIELD_DSAP_SSAP = 50,
	TEMPLATE_FIELD_FWD_VID = 52,
	TEMPLATE_FIELD_RANGE_CHK = 53,
	TEMPLATE_FIELD_SLP = 55,
	TEMPLATE_FIELD_DLP = 56,
	TEMPLATE_FIELD_META_DATA = 57,
	TEMPLATE_FIELD_FIRST_MPLS1 = 60,
	TEMPLATE_FIELD_FIRST_MPLS2 = 61,
	TEMPLATE_FIELD_DPM3 = 8,
};

/* The meaning of TEMPLATE_FIELD_VLAN depends on phase and the configuration in
 * RTL931X_PIE_CTRL. We use always the same definition and map to the inner VLAN tag:
 */
#define TEMPLATE_FIELD_VLAN TEMPLATE_FIELD_ITAG

/* Number of fixed templates predefined in the RTL9310 SoC */
#define N_FIXED_TEMPLATES 5
/* RTL931x specific predefined templates */
static enum template_field_id fixed_templates[N_FIXED_TEMPLATES][N_FIXED_FIELDS_RTL931X] =
{
	{
		TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
		TEMPLATE_FIELD_SMAC0, TEMPLATE_FIELD_SMAC1, TEMPLATE_FIELD_SMAC2,
		TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_DSAP_SSAP,
		TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1,
		TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	}, {
		TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
		TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_TCP_INFO,
		TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT, TEMPLATE_FIELD_VLAN,
		TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1,
		TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	}, {
		TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
		TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_IP_TOS_PROTO,
		TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
		TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT,
		TEMPLATE_FIELD_META_DATA, TEMPLATE_FIELD_SLP
	}, {
		TEMPLATE_FIELD_DIP0, TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_DIP2,
		TEMPLATE_FIELD_DIP3, TEMPLATE_FIELD_DIP4, TEMPLATE_FIELD_DIP5,
		TEMPLATE_FIELD_DIP6, TEMPLATE_FIELD_DIP7, TEMPLATE_FIELD_IP_TOS_PROTO,
		TEMPLATE_FIELD_TCP_INFO, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT,
		TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SLP
	}, {
		TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_SIP2,
		TEMPLATE_FIELD_SIP3, TEMPLATE_FIELD_SIP4, TEMPLATE_FIELD_SIP5,
		TEMPLATE_FIELD_SIP6, TEMPLATE_FIELD_SIP7, TEMPLATE_FIELD_META_DATA,
		TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1,
		TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	},
};

inline void rtl931x_exec_tbl0_cmd(u32 cmd)
{
	sw_w32(cmd, RTL931X_TBL_ACCESS_CTRL_0);
	do { } while (sw_r32(RTL931X_TBL_ACCESS_CTRL_0) & (1 << 20));
}

inline void rtl931x_exec_tbl1_cmd(u32 cmd)
{
	sw_w32(cmd, RTL931X_TBL_ACCESS_CTRL_1);
	do { } while (sw_r32(RTL931X_TBL_ACCESS_CTRL_1) & (1 << 17));
}

inline int rtl931x_tbl_access_data_0(int i)
{
	return RTL931X_TBL_ACCESS_DATA_0(i);
}

static int
rtl931x_vlan_profile_get(int idx, struct rtl83xx_vlan_profile *profile)
{
	u32 p[7];

	if (idx < 0 || idx > RTL931X_VLAN_PROFILE_MAX)
		return -EINVAL;

	p[0] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx));
	p[1] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx) + 4);
	p[2] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx) + 8);
	p[3] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx) + 12);
	p[4] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx) + 16);
	p[5] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx) + 20);
	p[6] = sw_r32(RTL931X_VLAN_PROFILE_SET(idx) + 24);

	*profile = (struct rtl83xx_vlan_profile) {
		.l2_learn = RTL931X_VLAN_L2_LEARN_EN_R(p),
		.unkn_mc_fld.pmsks = {
			.l2 = RTL931X_VLAN_L2_UNKN_MC_FLD_PMSK(p),
			.ip = RTL931X_VLAN_IP4_UNKN_MC_FLD_PMSK(p),
			.ip6 = RTL931X_VLAN_IP6_UNKN_MC_FLD_PMSK(p),
		},
	};

	return 0;
}

static void rtl931x_vlan_profile_dump(struct rtl838x_switch_priv *priv, int idx)
{
	struct rtl83xx_vlan_profile p;

	if (rtl931x_vlan_profile_get(idx, &p) < 0)
		return;

	dev_dbg(priv->dev,
		"VLAN %d: L2 learning: %d, L2 Unknown MultiCast Field %llx, \
		 IPv4 Unknown MultiCast Field %llx, IPv6 Unknown MultiCast Field: %llx",
		idx, p.l2_learn, p.unkn_mc_fld.pmsks.l2,
		p.unkn_mc_fld.pmsks.ip, p.unkn_mc_fld.pmsks.ip6);
}

static void rtl931x_stp_get(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 20 | /* Execute cmd */
	          0 << 19 | /* Read */
	          5 << 15 | /* Table type 0b101 */
	          (msti & 0x3fff);
	priv->r->exec_tbl0_cmd(cmd);

	for (int i = 0; i < 4; i++)
		port_state[i] = sw_r32(priv->r->tbl_access_data_0(i));
}

static void rtl931x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 20 | /* Execute cmd */
	          1 << 19 | /* Write */
	          5 << 15 | /* Table type 0b101 */
	          (msti & 0x3fff);
	for (int i = 0; i < 4; i++)
		sw_w32(port_state[i], priv->r->tbl_access_data_0(i));
	priv->r->exec_tbl0_cmd(cmd);
}

inline static int rtl931x_trk_mbr_ctr(int group)
{
	return RTL931X_TRK_MBR_CTRL + (group << 2);
}

static void rtl931x_vlan_tables_read(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v, w, x, y;
	/* Read VLAN table (3) via register 0 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 3);

	rtl_table_read(r, vlan);
	v = sw_r32(rtl_table_data(r, 0));
	w = sw_r32(rtl_table_data(r, 1));
	x = sw_r32(rtl_table_data(r, 2));
	y = sw_r32(rtl_table_data(r, 3));
	rtl_table_release(r);

	pr_debug("VLAN_READ %d: %08x %08x %08x %08x\n", vlan, v, w, x, y);
	info->tagged_ports = ((u64) v) << 25 | (w >> 7);
	info->profile_id = (x >> 16) & 0xf;
	info->fid = w & 0x7f;				/* AKA MSTI depending on context */
	info->hash_uc_fid = !!(x & BIT(31));
	info->hash_mc_fid = !!(x & BIT(30));
	info->if_id = (x >> 20) & 0x3ff;
	info->multicast_grp_mask = x & 0xffff;
	if (y & BIT(31))
		info->l2_tunnel_list_id = y >> 18;
	else
		info->l2_tunnel_list_id = -1;
	pr_debug("%s read tagged %016llx, profile-id %d, uc %d, mc %d, intf-id %d\n", __func__,
		info->tagged_ports, info->profile_id, info->hash_uc_fid, info->hash_mc_fid,
		info->if_id);

	/* Read UNTAG table via table register 3 */
	r = rtl_table_get(RTL9310_TBL_3, 0);
	rtl_table_read(r, vlan);
	info->untagged_ports = ((u64)sw_r32(rtl_table_data(r, 0))) << 25;
	info->untagged_ports |= sw_r32(rtl_table_data(r, 1)) >> 7;

	rtl_table_release(r);
}

static void rtl931x_vlan_set_tagged(u32 vlan, struct rtl838x_vlan_info *info)
{
	struct table_reg *r;
	u32 v, w, x, y;

	v = info->tagged_ports >> 25;
	w = (info->tagged_ports & GENMASK(24, 0)) << 7;
	w |= info->fid & 0x7f;
	x = info->hash_uc_fid ? BIT(31) : 0;
	x |= info->hash_mc_fid ? BIT(30) : 0;
	x |= info->if_id & 0x3ff << 20;
	x |= (info->profile_id & 0xf) << 16;
	x |= info->multicast_grp_mask & 0xffff;
	if (info->l2_tunnel_list_id >= 0) {
		y = info->l2_tunnel_list_id << 18;
		y |= BIT(31);
	} else {
		y = 0;
	}

	r = rtl_table_get(RTL9310_TBL_0, 3);
	sw_w32(v, rtl_table_data(r, 0));
	sw_w32(w, rtl_table_data(r, 1));
	sw_w32(x, rtl_table_data(r, 2));
	sw_w32(y, rtl_table_data(r, 3));

	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

static void rtl931x_vlan_set_untagged(u32 vlan, u64 portmask)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 0);

	rtl839x_set_port_reg_be(portmask << 7, rtl_table_data(r, 0));
	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

static inline int rtl931x_mac_force_mode_ctrl(int p)
{
	return RTL931X_MAC_FORCE_MODE_CTRL + (p << 2);
}

static inline int rtl931x_mac_link_spd_sts(int p)
{
	return RTL931X_MAC_LINK_SPD_STS + (((p >> 3) << 2));
}

static inline int rtl931x_mac_port_ctrl(int p)
{
	return RTL931X_MAC_L2_PORT_CTRL(p);
}

static inline int rtl931x_l2_port_new_salrn(int p)
{
	return RTL931X_L2_PORT_NEW_SALRN(p);
}

static inline int rtl931x_l2_port_new_sa_fwd(int p)
{
	return RTL931X_L2_PORT_NEW_SA_FWD(p);
}

irqreturn_t rtl931x_switch_irq(int irq, void *dev_id)
{
	struct dsa_switch *ds = dev_id;
	u32 status = sw_r32(RTL931X_ISR_GLB_SRC);
	u64 ports = rtl839x_get_port_reg_le(RTL931X_ISR_PORT_LINK_STS_CHG);
	u64 link;

	/* Clear status */
	rtl839x_set_port_reg_le(ports, RTL931X_ISR_PORT_LINK_STS_CHG);
	pr_debug("RTL931X Link change: status: %x, ports %016llx\n", status, ports);

	link = rtl839x_get_port_reg_le(RTL931X_MAC_LINK_STS);
	/* Must re-read this to get correct status */
	link = rtl839x_get_port_reg_le(RTL931X_MAC_LINK_STS);
	pr_debug("RTL931X Link change: status: %x, link status %016llx\n", status, link);

	for (int i = 0; i < 56; i++) {
		if (ports & BIT_ULL(i)) {
			if (link & BIT_ULL(i)) {
				pr_debug("%s port %d up\n", __func__, i);
				dsa_port_phylink_mac_change(ds, i, true);
			} else {
				pr_debug("%s port %d down\n", __func__, i);
				dsa_port_phylink_mac_change(ds, i, false);
			}
		}
	}

	return IRQ_HANDLED;
}

int rtl931x_write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	u32 v;
	int err = 0;

	val &= 0xffff;
	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);
	pr_debug("%s: writing to phy %d %d %d %d\n", __func__, port, page, reg, val);
	/* Clear both port registers */
	sw_w32(0, RTL931X_SMI_INDRT_ACCESS_CTRL_2);
	sw_w32(0, RTL931X_SMI_INDRT_ACCESS_CTRL_2 + 4);
	sw_w32_mask(0, BIT(port % 32), RTL931X_SMI_INDRT_ACCESS_CTRL_2 + (port / 32) * 4);

	sw_w32_mask(0xffff, val, RTL931X_SMI_INDRT_ACCESS_CTRL_3);

	v = reg << 6 | page << 11 ;
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	sw_w32(0x1ff, RTL931X_SMI_INDRT_ACCESS_CTRL_1);

	v |= BIT(4) | 1; /* Write operation and execute */
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
	} while (sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0) & 0x1);

	if (sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0) & 0x2)
		err = -EIO;

	mutex_unlock(&smi_lock);

	return err;
}

int rtl931x_read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	u32 v;

	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);

	sw_w32(port << 5, RTL931X_SMI_INDRT_ACCESS_BC_PHYID_CTRL);

	v = reg << 6 | page << 11 | 1;
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
	} while (sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0) & 0x1);

	v = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0);
	*val = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_3);
	*val = (*val & 0xffff0000) >> 16;

	pr_debug("%s: port %d, page: %d, reg: %x, val: %x, v: %08x\n",
		__func__, port, page, reg, *val, v);

	mutex_unlock(&smi_lock);

	return 0;
}

/* Read an mmd register of the PHY */
int rtl931x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val)
{
	int err = 0;
	u32 v;
	/* Select PHY register type
	 * If select 1G/10G MMD register type, registers EXT_PAGE, MAIN_PAGE and REG settings are don’t care.
	 * 0x0  Normal register (Clause 22)
	 * 0x1: 1G MMD register (MMD via Clause 22 registers 13 and 14)
	 * 0x2: 10G MMD register (MMD via Clause 45)
	 */
	int type = 2;

	mutex_lock(&smi_lock);

	/* Set PHY to access via port-number */
	sw_w32(port << 5, RTL931X_SMI_INDRT_ACCESS_BC_PHYID_CTRL);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL931X_SMI_INDRT_ACCESS_MMD_CTRL);

	v = type << 2 | BIT(0); /* MMD-access-type | EXEC */
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
		v = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0);
	} while (v & BIT(0));

	/* Check for error condition */
	if (v & BIT(1))
		err = -EIO;

	*val = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_3) >> 16;

	pr_debug("%s: port %d, dev: %x, regnum: %x, val: %x (err %d)\n", __func__,
		 port, devnum, regnum, *val, err);

	mutex_unlock(&smi_lock);

	return err;
}

/* Write to an mmd register of the PHY */
int rtl931x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val)
{
	int err = 0;
	u32 v;
	int type = 1;

	mutex_lock(&smi_lock);

	/* Set PHY to access via port-mask */
	sw_w32(port << 5, RTL931X_SMI_INDRT_ACCESS_BC_PHYID_CTRL);
//	pm = (u64)1 << port;
//	sw_w32((u32)pm, RTL931X_SMI_INDRT_ACCESS_CTRL_2);
//	sw_w32((u32)(pm >> 32), RTL931X_SMI_INDRT_ACCESS_CTRL_2 + 4);

	/* Set data to write */
	sw_w32_mask(0xffff, val, RTL931X_SMI_INDRT_ACCESS_CTRL_3);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL931X_SMI_INDRT_ACCESS_MMD_CTRL);

	v = BIT(4) | type << 2 | BIT(0); /* WRITE | MMD-access-type | EXEC */
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
		v = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0);
	} while (v & BIT(0));

	pr_debug("%s: port %d, dev: %x, regnum: %x, val: %x (err %d)\n", __func__,
		 port, devnum, regnum, val, err);
	mutex_unlock(&smi_lock);

	return err;
}
#define HASH_PICK(val, lsb, len)   ((val & (((1 << len) - 1) << lsb)) >> lsb)

static u32 rtl931x_l3_hash4(u32 ip, int algorithm, bool move_dip)
{
	u32 rows[4];
	u32 hash;
	u32 s0, s1, pH;

	memset(rows, 0, sizeof(rows));

	rows[0] = HASH_PICK(ip, 27, 5);
	rows[1] = HASH_PICK(ip, 18, 9);
	rows[2] = HASH_PICK(ip, 9, 9);

	if (!move_dip)
		rows[3] = HASH_PICK(ip, 0, 9);

	if (!algorithm) {
		hash = rows[0] ^ rows[1] ^ rows[2] ^ rows[3];
	} else {
		s0 = rows[0] + rows[1] + rows[2];
		s1 = (s0 & 0x1ff) + ((s0 & (0x1ff << 9)) >> 9);
		pH = (s1 & 0x1ff) + ((s1 & (0x1ff << 9)) >> 9);
		hash = pH ^ rows[3];
	}
	return hash;
}

// Currently not used
// static u32 rtl930x_l3_hash6(struct in6_addr *ip6, int algorithm, bool move_dip)
// {
// 	u32 rows[16];
// 	u32 hash;
// 	u32 s0, s1, pH;

// 	rows[0] = (HASH_PICK(ip6->s6_addr[0], 6, 2) << 0);
// 	rows[1] = (HASH_PICK(ip6->s6_addr[0], 0, 6) << 3) | HASH_PICK(ip6->s6_addr[1], 5, 3);
// 	rows[2] = (HASH_PICK(ip6->s6_addr[1], 0, 5) << 4) | HASH_PICK(ip6->s6_addr[2], 4, 4);
// 	rows[3] = (HASH_PICK(ip6->s6_addr[2], 0, 4) << 5) | HASH_PICK(ip6->s6_addr[3], 3, 5);
// 	rows[4] = (HASH_PICK(ip6->s6_addr[3], 0, 3) << 6) | HASH_PICK(ip6->s6_addr[4], 2, 6);
// 	rows[5] = (HASH_PICK(ip6->s6_addr[4], 0, 2) << 7) | HASH_PICK(ip6->s6_addr[5], 1, 7);
// 	rows[6] = (HASH_PICK(ip6->s6_addr[5], 0, 1) << 8) | HASH_PICK(ip6->s6_addr[6], 0, 8);
// 	rows[7] = (HASH_PICK(ip6->s6_addr[7], 0, 8) << 1) | HASH_PICK(ip6->s6_addr[8], 7, 1);
// 	rows[8] = (HASH_PICK(ip6->s6_addr[8], 0, 7) << 2) | HASH_PICK(ip6->s6_addr[9], 6, 2);
// 	rows[9] = (HASH_PICK(ip6->s6_addr[9], 0, 6) << 3) | HASH_PICK(ip6->s6_addr[10], 5, 3);
// 	rows[10] = (HASH_PICK(ip6->s6_addr[10], 0, 5) << 4) | HASH_PICK(ip6->s6_addr[11], 4, 4);
// 	if (!algorithm) {
// 		rows[11] = (HASH_PICK(ip6->s6_addr[11], 0, 4) << 5) |
// 		           (HASH_PICK(ip6->s6_addr[12], 3, 5) << 0);
// 		rows[12] = (HASH_PICK(ip6->s6_addr[12], 0, 3) << 6) |
// 		           (HASH_PICK(ip6->s6_addr[13], 2, 6) << 0);
// 		rows[13] = (HASH_PICK(ip6->s6_addr[13], 0, 2) << 7) |
// 		           (HASH_PICK(ip6->s6_addr[14], 1, 7) << 0);
// 		if (!move_dip) {
// 			rows[14] = (HASH_PICK(ip6->s6_addr[14], 0, 1) << 8) |
// 			           (HASH_PICK(ip6->s6_addr[15], 0, 8) << 0);
// 		}
// 		hash = rows[0] ^ rows[1] ^ rows[2] ^ rows[3] ^ rows[4] ^
// 		       rows[5] ^ rows[6] ^ rows[7] ^ rows[8] ^ rows[9] ^
// 		       rows[10] ^ rows[11] ^ rows[12] ^ rows[13] ^ rows[14];
// 	} else {
// 		rows[11] = (HASH_PICK(ip6->s6_addr[11], 0, 4) << 5);
// 		rows[12] = (HASH_PICK(ip6->s6_addr[12], 3, 5) << 0);
// 		rows[13] = (HASH_PICK(ip6->s6_addr[12], 0, 3) << 6) |
// 		           HASH_PICK(ip6->s6_addr[13], 2, 6);
// 		rows[14] = (HASH_PICK(ip6->s6_addr[13], 0, 2) << 7) |
// 		           HASH_PICK(ip6->s6_addr[14], 1, 7);
// 		if (!move_dip) {
// 			rows[15] = (HASH_PICK(ip6->s6_addr[14], 0, 1) << 8) |
// 			           (HASH_PICK(ip6->s6_addr[15], 0, 8) << 0);
// 		}
// 		s0 = rows[12] + rows[13] + rows[14];
// 		s1 = (s0 & 0x1ff) + ((s0 & (0x1ff << 9)) >> 9);
// 		pH = (s1 & 0x1ff) + ((s1 & (0x1ff << 9)) >> 9);
// 		hash = rows[0] ^ rows[1] ^ rows[2] ^ rows[3] ^ rows[4] ^
// 		       rows[5] ^ rows[6] ^ rows[7] ^ rows[8] ^ rows[9] ^
// 		       rows[10] ^ rows[11] ^ pH ^ rows[15];
// 	}
// 	return hash;
// }

/* Read a prefix route entry from the L3_PREFIX_ROUTE_IPUC table
 * We currently only support IPv4 and IPv6 unicast route
 */
static void rtl931x_route_read(int idx, struct rtl83xx_route *rt)
{
	u32 v, ip4_m;
	bool host_route, default_route;
	struct in6_addr ip6_m;

	/* Read L3_PREFIX_ROUTE_IPUC table (2) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 2);

	rtl_table_read(r, idx);
	/* The table has a size of 11 registers */
	rt->attr.valid = !!(sw_r32(rtl_table_data(r, 0)) & BIT(31));
	if (!rt->attr.valid)
		goto out;

	rt->attr.type = (sw_r32(rtl_table_data(r, 0)) >> 29) & 0x3;

	v = sw_r32(rtl_table_data(r, 10));
	host_route = !!(v & BIT(21));
	default_route = !!(v & BIT(20));
	rt->prefix_len = -1;
	pr_debug("%s: host route %d, default_route %d\n", __func__, host_route, default_route);

	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		rt->dst_ip = sw_r32(rtl_table_data(r, 4));
		ip4_m = sw_r32(rtl_table_data(r, 9));
		pr_debug("%s: Read ip4 mask: %08x\n", __func__, ip4_m);
		rt->prefix_len = host_route ? 32 : -1;
		rt->prefix_len = (rt->prefix_len < 0 && default_route) ? 0 : -1;
		if (rt->prefix_len < 0)
			rt->prefix_len = inet_mask_len(ip4_m);
		break;
	case 2: /* IPv6 Unicast route */
		ipv6_addr_set(&rt->dst_ip6,
			      sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
			      sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)));
		ipv6_addr_set(&ip6_m,
			      sw_r32(rtl_table_data(r, 6)), sw_r32(rtl_table_data(r, 7)),
			      sw_r32(rtl_table_data(r, 8)), sw_r32(rtl_table_data(r, 9)));
		rt->prefix_len = host_route ? 128 : 0;
		rt->prefix_len = (rt->prefix_len < 0 && default_route) ? 0 : -1;
		if (rt->prefix_len < 0)
			rt->prefix_len = find_last_bit((unsigned long int *)&ip6_m.s6_addr32,
							 128);
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		pr_warn("%s: route type not supported\n", __func__);
		goto out;
	}

	rt->attr.hit = !!(v & BIT(22));
	rt->attr.action = (v >> 18) & 3;
	rt->nh.id = (v >> 7) & 0x7ff;
	rt->attr.ttl_dec = !!(v & BIT(6));
	rt->attr.ttl_check = !!(v & BIT(5));
	rt->attr.dst_null = !!(v & BIT(4));
	rt->attr.qos_as = !!(v & BIT(3));
	rt->attr.qos_prio =  v & 0x7;
	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action,
		rt->attr.ttl_dec, rt->attr.ttl_check, rt->attr.dst_null);
	pr_debug("%s: GW: %pI4, prefix_len: %d\n", __func__, &rt->dst_ip, rt->prefix_len);
out:
	rtl_table_release(r);
}

static void rtl931x_net6_mask(int prefix_len, struct in6_addr *ip6_m)
{
	int o, b;
	/* Define network mask */
	o = prefix_len >> 3;
	b = prefix_len & 0x7;
	memset(ip6_m->s6_addr, 0xff, o);
	ip6_m->s6_addr[o] |= b ? 0xff00 >> b : 0x00;
}

/* Read a host route entry from the table using its index
 * We currently only support IPv4 and IPv6 unicast route
 */
static void rtl931x_host_route_read(int idx, struct rtl83xx_route *rt)
{
	u32 v;
	/* Read L3_HOST_ROUTE_IPUC table (1) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 1);

	idx = ((idx / 6) * 8) + (idx % 6);

	pr_debug("In %s, physical index %d\n", __func__, idx);
	rtl_table_read(r, idx);
	/* The table has a size of 5 (for UC, 11 for MC) registers */
	v = sw_r32(rtl_table_data(r, 0));
	rt->attr.valid = !!(v & BIT(31));
	if (!rt->attr.valid)
		goto out;
	rt->attr.type = (v >> 29) & 0x3;
	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		rt->dst_ip = sw_r32(rtl_table_data(r, 4));
		break;
	case 2: /* IPv6 Unicast route */
		ipv6_addr_set(&rt->dst_ip6,
			      sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 2)),
			      sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 0)));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		pr_warn("%s: route type not supported\n", __func__);
		goto out;
	}

	rt->attr.hit = !!(v & BIT(20));
	rt->attr.dst_null = !!(v & BIT(19));
	rt->attr.action = (v >> 17) & 3;
	rt->nh.id = (v >> 6) & 0x7ff;
	rt->attr.ttl_dec = !!(v & BIT(5));
	rt->attr.ttl_check = !!(v & BIT(4));
	rt->attr.qos_as = !!(v & BIT(3));
	rt->attr.qos_prio =  v & 0x7;
	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action, rt->attr.ttl_dec, rt->attr.ttl_check,
		rt->attr.dst_null);
	pr_debug("%s: Destination: %pI4\n", __func__, &rt->dst_ip);

out:
	rtl_table_release(r);
}

/* Write a host route entry from the table using its index
 * We currently only support IPv4 and IPv6 unicast route
 */
static void rtl931x_host_route_write(int idx, struct rtl83xx_route *rt)
{
	u32 v;
	/* Access L3_HOST_ROUTE_IPUC table (1) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 1);
	/* The table has a size of 5 (for UC, 11 for MC) registers */

	idx = ((idx / 6) * 8) + (idx % 6);

	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action, rt->attr.ttl_dec, rt->attr.ttl_check,
		rt->attr.dst_null);
	pr_debug("%s: GW: %pI4, prefix_len: %d\n", __func__, &rt->dst_ip, rt->prefix_len);

	v = BIT(31); /* Entry is valid */
	v |= (rt->attr.type & 0x3) << 29;
	v |= rt->attr.hit ? BIT(20) : 0;
	v |= rt->attr.dst_null ? BIT(19) : 0;
	v |= (rt->attr.action & 0x3) << 17;
	v |= (rt->nh.id & 0x7ff) << 6;
	v |= rt->attr.ttl_dec ? BIT(5) : 0;
	v |= rt->attr.ttl_check ? BIT(4) : 0;
	v |= rt->attr.qos_as ? BIT(3) : 0;
	v |= rt->attr.qos_prio & 0x7;

	sw_w32(v, rtl_table_data(r, 0));
	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		sw_w32(0, rtl_table_data(r, 1));
		sw_w32(0, rtl_table_data(r, 2));
		sw_w32(0, rtl_table_data(r, 3));
		sw_w32(rt->dst_ip, rtl_table_data(r, 4));
		break;
	case 2: /* IPv6 Unicast route */
		sw_w32(rt->dst_ip6.s6_addr32[0], rtl_table_data(r, 1));
		sw_w32(rt->dst_ip6.s6_addr32[1], rtl_table_data(r, 2));
		sw_w32(rt->dst_ip6.s6_addr32[2], rtl_table_data(r, 3));
		sw_w32(rt->dst_ip6.s6_addr32[3], rtl_table_data(r, 4));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		pr_warn("%s: route type not supported\n", __func__);
		goto out;
	}

	rtl_table_write(r, idx);

out:
	rtl_table_release(r);
}

/* Look up the index of a prefix route in the routing table CAM for unicast IPv4/6 routes
 * using hardware offload.
 */
static int rtl931x_route_lookup_hw(struct rtl83xx_route *rt)
{
	u32 ip4_m, v;
	struct in6_addr ip6_m;

	if (rt->attr.type == 1 || rt->attr.type == 3) /* Hardware only supports UC routes */
		return -1;

	sw_w32_mask(0x3 << 22, rt->attr.type, RTL931X_L3_HW_LU_KEY_CTRL);
	if (rt->attr.type) { /* IPv6 */
		rtl931x_net6_mask(rt->prefix_len, &ip6_m);
		for (int i = 0; i < 4; i++)
			sw_w32(rt->dst_ip6.s6_addr32[0] & ip6_m.s6_addr32[0],
			       RTL931X_L3_HW_LU_KEY_IP_CTRL + (i << 2));
	} else { /* IPv4 */
		ip4_m = inet_make_mask(rt->prefix_len);
		sw_w32(0, RTL931X_L3_HW_LU_KEY_IP_CTRL);
		sw_w32(0, RTL931X_L3_HW_LU_KEY_IP_CTRL + 4);
		sw_w32(0, RTL931X_L3_HW_LU_KEY_IP_CTRL + 8);
		v = rt->dst_ip & ip4_m;
		pr_debug("%s: searching for %pI4\n", __func__, &v);
		sw_w32(v, RTL931X_L3_HW_LU_KEY_IP_CTRL + 12);
	}

	/* Execute CAM lookup in SoC */
	sw_w32(BIT(15), RTL931X_L3_HW_LU_CTRL);

	/* Wait until execute bit clears and result is ready */
	do {
		v = sw_r32(RTL931X_L3_HW_LU_CTRL);
	} while (v & BIT(15));

	pr_debug("%s: found: %d, index: %d\n", __func__, !!(v & BIT(14)), v & 0x1ff);

	/* Test if search successful (BIT 14 set) */
	if (v & BIT(14))
		return v & 0x1ff;

	return -1;
}

static int rtl931x_find_l3_slot(struct rtl83xx_route *rt, bool must_exist)
{
	int slot_width, algorithm, addr, idx;
	u32 hash;
	struct rtl83xx_route route_entry;

	/* IPv6 entries take up 3 slots */
	slot_width = (rt->attr.type == 0) || (rt->attr.type == 2) ? 1 : 3;

	for (int t = 0; t < 2; t++) {
		algorithm = (sw_r32(RTL931X_L3_HOST_TBL_CTRL) >> (2 + t)) & 0x1;
		hash = rtl931x_l3_hash4(rt->dst_ip, algorithm, false);

		pr_debug("%s: table %d, algorithm %d, hash %04x\n", __func__, t, algorithm, hash);

		for (int s = 0; s < 6; s += slot_width) {
			addr = (t << 12) | ((hash & 0x1ff) << 3) | s;
			pr_debug("%s physical address %d\n", __func__, addr);
			idx = ((addr / 8) * 6) + (addr % 8);
			pr_debug("%s logical address %d\n", __func__, idx);

			rtl931x_host_route_read(idx, &route_entry);
			pr_debug("%s route valid %d, route dest: %pI4, hit %d\n", __func__,
				rt->attr.valid, &rt->dst_ip, rt->attr.hit);
			if (!must_exist && rt->attr.valid)
				return idx;
			if (must_exist && route_entry.dst_ip == rt->dst_ip)
				return idx;
		}
	}

	return -1;
}

/* Write a prefix route into the routing table CAM at position idx
 * Currently only IPv4 and IPv6 unicast routes are supported
 */
static void rtl931x_route_write(int idx, struct rtl83xx_route *rt)
{
	u32 v, ip4_m;
	struct in6_addr ip6_m;
	/* Access L3_PREFIX_ROUTE_IPUC table (2) via register RTL9310_TBL_3 */
	/* The table has a size of 11 registers (20 for MC) */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 2);

	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: nexthop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action,
		rt->attr.ttl_dec, rt->attr.ttl_check, rt->attr.dst_null);
	pr_debug("%s: GW: %pI4, prefix_len: %d\n", __func__, &rt->dst_ip, rt->prefix_len);

	v = rt->attr.valid ? BIT(31) : 0;
	v |= (rt->attr.type & 0x3) << 29;
	sw_w32(v, rtl_table_data(r, 0));

	v = rt->attr.hit ? BIT(22) : 0;
	v |= (rt->attr.action & 0x3) << 18;
	v |= (rt->nh.id & 0x7ff) << 7;
	v |= rt->attr.ttl_dec ? BIT(6) : 0;
	v |= rt->attr.ttl_check ? BIT(5) : 0;
	v |= rt->attr.dst_null ? BIT(6) : 0;
	v |= rt->attr.qos_as ? BIT(6) : 0;
	v |= rt->attr.qos_prio & 0x7;
	v |= rt->prefix_len == 0 ? BIT(20) : 0; /* set default route bit */

	/* set bit mask for entry type always to 0x3 */
	sw_w32(0x3 << 29, rtl_table_data(r, 5));

	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		sw_w32(0, rtl_table_data(r, 1));
		sw_w32(0, rtl_table_data(r, 2));
		sw_w32(0, rtl_table_data(r, 3));
		sw_w32(rt->dst_ip, rtl_table_data(r, 4));

		v |= rt->prefix_len == 32 ? BIT(21) : 0; /* set host-route bit */
		ip4_m = inet_make_mask(rt->prefix_len);
		sw_w32(0, rtl_table_data(r, 6));
		sw_w32(0, rtl_table_data(r, 7));
		sw_w32(0, rtl_table_data(r, 8));
		sw_w32(ip4_m, rtl_table_data(r, 9));
		break;
	case 2: /* IPv6 Unicast route */
		sw_w32(rt->dst_ip6.s6_addr32[0], rtl_table_data(r, 1));
		sw_w32(rt->dst_ip6.s6_addr32[1], rtl_table_data(r, 2));
		sw_w32(rt->dst_ip6.s6_addr32[2], rtl_table_data(r, 3));
		sw_w32(rt->dst_ip6.s6_addr32[3], rtl_table_data(r, 4));

		v |= rt->prefix_len == 128 ? BIT(21) : 0; /* set host-route bit */

		rtl931x_net6_mask(rt->prefix_len, &ip6_m);

		sw_w32(ip6_m.s6_addr32[0], rtl_table_data(r, 6));
		sw_w32(ip6_m.s6_addr32[1], rtl_table_data(r, 7));
		sw_w32(ip6_m.s6_addr32[2], rtl_table_data(r, 8));
		sw_w32(ip6_m.s6_addr32[3], rtl_table_data(r, 9));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		pr_warn("%s: route type not supported\n", __func__);
		rtl_table_release(r);
		return;
	}
	sw_w32(v, rtl_table_data(r, 10));

	pr_debug("%s: %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n", __func__,
		sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
		sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)), sw_r32(rtl_table_data(r, 5)),
		sw_r32(rtl_table_data(r, 6)), sw_r32(rtl_table_data(r, 7)), sw_r32(rtl_table_data(r, 8)),
		sw_r32(rtl_table_data(r, 9)), sw_r32(rtl_table_data(r, 10)));

	rtl_table_write(r, idx);
	rtl_table_release(r);
}


/* Get the destination MAC and L3 egress interface ID of a nexthop entry from
 * the SoC's L3_NEXTHOP table
 */
static void rtl931x_get_l3_nexthop(int idx, u16 *dmac_id, u16 *interface)
{
	u32 v;
	/* Read L3_NEXTHOP table (3) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 3);

	rtl_table_read(r, idx);
	/* The table has a size of 1 register */
	v = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);

	*dmac_id = (v >> 7) & 0x7fff;
	*interface = v & 0x7f;
}

// Currently not used
// static int rtl930x_l3_mtu_del(struct rtl838x_switch_priv *priv, int mtu)
// {
// 	int i;

// 	for (i = 0; i < MAX_INTF_MTUS; i++) {
// 		if (mtu == priv->intf_mtus[i])
// 			break;
// 	}
// 	if (i >= MAX_INTF_MTUS || !priv->intf_mtu_count[i]) {
// 		pr_err("%s: No MTU slot found for MTU: %d\n", __func__, mtu);
// 		return -EINVAL;
// 	}

// 	priv->intf_mtu_count[i]--;
// }

// Currently not used
// static int rtl930x_l3_mtu_add(struct rtl838x_switch_priv *priv, int mtu)
// {
// 	int i, free_mtu;
// 	int mtu_id;

// 	/* Try to find an existing mtu-value or a free slot */
// 	free_mtu = MAX_INTF_MTUS;
// 	for (i = 0; i < MAX_INTF_MTUS && priv->intf_mtus[i] != mtu; i++) {
// 		if ((!priv->intf_mtu_count[i]) && (free_mtu == MAX_INTF_MTUS))
// 			free_mtu = i;
// 	}
// 	i = (i < MAX_INTF_MTUS) ? i : free_mtu;
// 	if (i < MAX_INTF_MTUS) {
// 		mtu_id = i;
// 	} else {
// 		pr_err("%s: No free MTU slot available!\n", __func__);
// 		return -EINVAL;
// 	}

// 	priv->intf_mtus[i] = mtu;
// 	pr_debug("Writing MTU %d to slot %d\n", priv->intf_mtus[i], i);
// 	/* Set MTU-value of the slot TODO: distinguish between IPv4/IPv6 routes / slots */
// 	sw_w32_mask(0xffff << ((i % 2) * 16), priv->intf_mtus[i] << ((i % 2) * 16),
// 		    RTL930X_L3_IP_MTU_CTRL(i));
// 	sw_w32_mask(0xffff << ((i % 2) * 16), priv->intf_mtus[i] << ((i % 2) * 16),
// 		    RTL930X_L3_IP6_MTU_CTRL(i));

// 	priv->intf_mtu_count[i]++;

// 	return mtu_id;
// }


// Currently not used
// /* Creates an interface for a route by setting up the HW tables in the SoC
// static int rtl930x_l3_intf_add(struct rtl838x_switch_priv *priv, struct rtl838x_l3_intf *intf)
// {
// 	int i, intf_id, mtu_id;
// 	/* number of MTU-values < 16384 *\/

// 	/* Use the same IPv6 mtu as the ip4 mtu for this route if unset */
// 	intf->ip6_mtu = intf->ip6_mtu ? intf->ip6_mtu : intf->ip4_mtu;

// 	mtu_id = rtl930x_l3_mtu_add(priv, intf->ip4_mtu);
// 	pr_debug("%s: added mtu %d with mtu-id %d\n", __func__, intf->ip4_mtu, mtu_id);
// 	if (mtu_id < 0)
// 		return -ENOSPC;
// 	intf->ip4_mtu_id = mtu_id;
// 	intf->ip6_mtu_id = mtu_id;

// 	for (i = 0; i < MAX_INTERFACES; i++) {
// 		if (!priv->interfaces[i])
// 			break;
// 	}
// 	if (i >= MAX_INTERFACES) {
// 		pr_err("%s: cannot find free interface entry\n", __func__);
// 		return -EINVAL;
// 	}
// 	intf_id = i;
// 	priv->interfaces[i] = kzalloc(sizeof(struct rtl838x_l3_intf), GFP_KERNEL);
// 	if (!priv->interfaces[i]) {
// 		pr_err("%s: no memory to allocate new interface\n", __func__);
// 		return -ENOMEM;
// 	}
// }

/* Set the destination MAC and L3 egress interface ID for a nexthop entry in the SoC's
 * L3_NEXTHOP table. The nexthop entry is identified by idx.
 * dmac_id is the reference to the L2 entry in the L2 forwarding table, special values are
 * 0x7ffe: TRAP2CPU
 * 0x7ffd: TRAP2MASTERCPU
 * 0x7fff: DMAC_ID_DROP
 */
static void rtl931x_set_l3_nexthop(int idx, u16 dmac_id, u16 interface)
{
	/* Access L3_NEXTHOP table (3) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 3);

	pr_debug("%s: Writing to L3_NEXTHOP table, index %d, dmac_id %d, interface %d\n",
		__func__, idx, dmac_id, interface);
	sw_w32(((dmac_id & 0x7fff) << 7) | (interface & 0x7f), rtl_table_data(r, 0));

	pr_debug("%s: %08x\n", __func__, sw_r32(rtl_table_data(r,0)));
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

/* Reads a MAC entry for L3 termination as entry point for routing
 * from the hardware table
 * idx is the index into the L3_ROUTER_MAC table
 */
static void rtl931x_get_l3_router_mac(u32 idx, struct rtl93xx_rt_mac *m)
{
	u32 v, w;
	/* Read L3_ROUTER_MAC table (0) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 0);

	rtl_table_read(r, idx);
	/* The table has a size of 7 registers, 64 entries */
	v = sw_r32(rtl_table_data(r, 0));
	w = sw_r32(rtl_table_data(r, 3));
	m->valid = !!(v & BIT(20));
	if (!m->valid)
		goto out;

	m->p_type = !!(v & BIT(19));
	m->p_id = (v >> 13) & 0x3f;  /* trunk id of port */
	m->vid = v & 0xfff;
	m->vid_mask = w & 0xfff;
	m->action = sw_r32(rtl_table_data(r, 6)) & 0x7;
	m->mac_mask = ((((u64)sw_r32(rtl_table_data(r, 5))) << 32) & 0xffffffffffffULL) |
	              (sw_r32(rtl_table_data(r, 4)));
	m->mac = ((((u64)sw_r32(rtl_table_data(r, 1))) << 32) & 0xffffffffffffULL) |
	         (sw_r32(rtl_table_data(r, 2)));
	/* Bits L3_INTF and BMSK_L3_INTF are 0 */

out:
	rtl_table_release(r);
}

/* Writes a MAC entry for L3 termination as entry point for routing
 * into the hardware table
 * idx is the index into the L3_ROUTER_MAC table
 */
static void rtl931x_set_l3_router_mac(u32 idx, struct rtl93xx_rt_mac *m)
{
	u32 v, w;
	/* Read L3_ROUTER_MAC table (0) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 0);

	/* The table has a size of 7 registers, 64 entries */
	v = BIT(20); /* mac entry valid, port type is 0: individual */
	v |= (m->p_id & 0x3f) << 13;
	v |= (m->vid & 0xfff); /* Set the interface_id to the vlan id */

	w = m->vid_mask;
	w |= (m->p_id_mask & 0x3f) << 13;

	sw_w32(v, rtl_table_data(r, 0));
	sw_w32(w, rtl_table_data(r, 3));

	/* Set MAC address, L3_INTF (bit 12 in register 1) needs to be 0 */
	sw_w32((u32)(m->mac), rtl_table_data(r, 2));
	sw_w32(m->mac >> 32, rtl_table_data(r, 1));

	/* Set MAC address mask, BMSK_L3_INTF (bit 12 in register 5) needs to be 0 */
	sw_w32((u32)(m->mac_mask >> 32), rtl_table_data(r, 4));
	sw_w32((u32)m->mac_mask, rtl_table_data(r, 5));

	sw_w32(m->action & 0x7, rtl_table_data(r, 6));

	pr_debug("%s writing index %d: %08x %08x %08x %08x %08x %08x %08x\n", __func__, idx,
		sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
		sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)), sw_r32(rtl_table_data(r, 5)),
		sw_r32(rtl_table_data(r, 6))
	);
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

void rtl931x_print_matrix(void)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 1);

	for (int i = 0; i < 64; i++) {
		rtl_table_read(r, i);
		pr_info("> %08x %08x\n", sw_r32(rtl_table_data(r, 0)),
			sw_r32(rtl_table_data(r, 1)));
	}
	rtl_table_release(r);
}

static void rtl931x_set_receive_management_action(int port, rma_ctrl_t type, action_type_t action)
{
	u32 value = 0;

	/* hack for value mapping */
	if (type == GRATARP && action == COPY2CPU)
		action = TRAP2MASTERCPU;

	switch(action) {
	case FORWARD:
		value = 0;
		break;
	case DROP:
		value = 1;
		break;
	case TRAP2CPU:
		value = 2;
		break;
	case TRAP2MASTERCPU:
		value = 3;
		break;
	case FLOODALL:
		value = 4;
		break;
	default:
		break;
	}

	switch(type) {
	case BPDU:
		sw_w32_mask(7 << ((port % 10) * 3), value << ((port % 10) * 3), RTL931X_RMA_BPDU_CTRL + ((port / 10) << 2));
	break;
	case PTP:
		/* udp */
		sw_w32_mask(3 << 2, value << 2, RTL931X_RMA_PTP_CTRL + (port << 2));
		/* eth2 */
		sw_w32_mask(3, value, RTL931X_RMA_PTP_CTRL + (port << 2));
	break;
	case PTP_UDP:
		sw_w32_mask(3 << 2, value << 2, RTL931X_RMA_PTP_CTRL + (port << 2));
	break;
	case PTP_ETH2:
		sw_w32_mask(3, value, RTL931X_RMA_PTP_CTRL + (port << 2));
	break;
	case LLDP:
		sw_w32_mask(7 << ((port % 10) * 3), value << ((port % 10) * 3), RTL931X_RMA_LLDP_CTRL + ((port / 10) << 2));
	break;
	case EAPOL:
		sw_w32_mask(7 << ((port % 10) * 3), value << ((port % 10) * 3), RTL931X_RMA_EAPOL_CTRL + ((port / 10) << 2));
	break;
	case GRATARP:
		sw_w32_mask(3 << ((port & 0xf) << 1), value << ((port & 0xf) << 1), RTL931X_TRAP_ARP_GRAT_PORT_ACT + ((port >> 4) << 2));
	break;
	}
}

static u64 rtl931x_traffic_get(int source)
{
	u64 v;
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 1);

	rtl_table_read(r, source);
	v = sw_r32(rtl_table_data(r, 0));
	v <<= 32;
	v |= sw_r32(rtl_table_data(r, 1));
	v >>= 7;
	rtl_table_release(r);

	return v;
}

/* Enable traffic between a source port and a destination port matrix */
static void rtl931x_traffic_set(int source, u64 dest_matrix)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 1);

	sw_w32(dest_matrix >> (32 - 7), rtl_table_data(r, 0));
	sw_w32(dest_matrix << 7, rtl_table_data(r, 1));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static void rtl931x_traffic_enable(int source, int dest)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 1);
	rtl_table_read(r, source);
	sw_w32_mask(0, BIT((dest + 7) % 32), rtl_table_data(r, (dest + 7) / 32 ? 0 : 1));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static void rtl931x_traffic_disable(int source, int dest)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 1);
	rtl_table_read(r, source);
	sw_w32_mask(BIT((dest + 7) % 32), 0, rtl_table_data(r, (dest + 7) / 32 ? 0 : 1));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

/* Enables or disables the EEE/EEEP capability of a port */
static void rtl931x_port_eee_set(struct rtl838x_switch_priv *priv, int port, bool enable)
{
	u32 v;
	u32 eee_mask = 1 << 18 | 1 << 19 | 1 << 20 | 1 << 23; //100, 500, 1000, 10g (as specified by sdk)

	/* This works only for Ethernet ports, and on the RTL838X, ports from 24 are SFP */
	if (port >= 56)
		return;
	
	pr_debug("In %s: setting port %d to %d\n", __func__, port, enable);
	v = enable ? eee_mask : 0x0;
	
	/* Set EEE state for 100 (bit 9) & 1000MBit (bit 10) */
	sw_w32_mask(eee_mask, v, rtl931x_mac_force_mode_ctrl(port));

	/* Set TX/RX EEE state */
	if (enable) {
		sw_w32(sw_r32(RTL931X_EEE_PORT_TX_EN(port)) | RTL931X_EEE_PORT_TX_EN_MASK(port), RTL931X_EEE_PORT_TX_EN(port));
		sw_w32(sw_r32(RTL931X_EEE_PORT_RX_EN(port)) | RTL931X_EEE_PORT_RX_EN_MASK(port), RTL931X_EEE_PORT_RX_EN(port));
	} else {
		sw_w32(sw_r32(RTL931X_EEE_PORT_TX_EN(port)) & ~RTL931X_EEE_PORT_TX_EN_MASK(port), RTL931X_EEE_PORT_TX_EN(port));
		sw_w32(sw_r32(RTL931X_EEE_PORT_RX_EN(port)) & ~RTL931X_EEE_PORT_RX_EN_MASK(port), RTL931X_EEE_PORT_RX_EN(port));
	}
	priv->ports[port].eee_enabled = enable;
}


#if 0
/* Get EEE own capabilities and negotiation result */
static int rtl931x_eee_port_ability(struct rtl838x_switch_priv *priv,
				    struct ethtool_eee *e, int port)
{
	u32 link, a;

	if (port >= 56)
		return -ENOTSUPP;

	e->supported = SUPPORTED_100baseT_Full |
	               SUPPORTED_1000baseT_Full |
	               SUPPORTED_2500baseX_Full |
	               SUPPORTED_10000baseT_Full;

	pr_debug("In %s, port %d\n", __func__, port);
	link = sw_r32(RTL931X_MAC_LINK_STS);
	link = sw_r32(RTL931X_MAC_LINK_STS);
	if (!(link & BIT(port)))
		return 0;

	pr_debug("Setting advertised\n");
	if (sw_r32(rtl931x_mac_force_mode_ctrl(port)) & BIT(18))
		e->advertised |= ADVERTISED_100baseT_Full;

	if (sw_r32(rtl931x_mac_force_mode_ctrl(port)) & BIT(20))
		e->advertised |= ADVERTISED_1000baseT_Full;

	if (priv->ports[port].is2G5 && sw_r32(rtl931x_mac_force_mode_ctrl(port)) & BIT(21)) {
		pr_debug("ADVERTISING 2.5G EEE\n");
		e->advertised |= ADVERTISED_2500baseX_Full;
	}

	if (priv->ports[port].is10G && sw_r32(rtl931x_mac_force_mode_ctrl(port)) & BIT(23))
		e->advertised |= ADVERTISED_10000baseT_Full;

	a = sw_r32(RTL931X_MAC_EEE_ABLTY(port));
	a = sw_r32(RTL931X_MAC_EEE_ABLTY(port));
	pr_debug("Link partner: %08x\n", a);
	if (a & RTL931X_MAC_EEE_ABLTY_MASK(port)) {
		e->lp_advertised = ADVERTISED_100baseT_Full;
		e->lp_advertised |= ADVERTISED_1000baseT_Full;
		if (priv->ports[port].is2G5)
			e->lp_advertised |= ADVERTISED_2500baseX_Full;
		if (priv->ports[port].is10G)
			e->lp_advertised |= ADVERTISED_10000baseT_Full;
	}

	return 0;
}
#endif

static void rtl931x_init_eee(struct rtl838x_switch_priv *priv, bool enable)
{
	pr_debug("Setting up EEE, state: %d\n", enable);

	/* Enable EEE MAC support on ports */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			rtl931x_port_eee_set(priv, i, enable);
	}
	priv->eee_enabled = enable;
}

static u64 rtl931x_l2_hash_seed(u64 mac, u32 vid)
{
	u64 v = vid;

	v <<= 48;
	v |= mac;

	return v;
}

/* Calculate both the block 0 and the block 1 hash by applyingthe same hash
 * algorithm as the one used currently by the ASIC to the seed, and return
 * both hashes in the lower and higher word of the return value since only 12 bit of
 * the hash are significant.
 */
static u32 rtl931x_l2_hash_key(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 h, h0, h1, h2, h3, h4, k0, k1;

	h0 = seed & 0xfff;
	h1 = (seed >> 12) & 0xfff;
	h2 = (seed >> 24) & 0xfff;
	h3 = (seed >> 36) & 0xfff;
	h4 = (seed >> 48) & 0xfff;
	h4 = ((h4 & 0x7) << 9) | ((h4 >> 3) & 0x1ff);
	k0 = h0 ^ h1 ^ h2 ^ h3 ^ h4;

	h0 = seed & 0xfff;
	h0 = ((h0 & 0x1ff) << 3) | ((h0 >> 9) & 0x7);
	h1 = (seed >> 12) & 0xfff;
	h1 = ((h1 & 0x3f) << 6) | ((h1 >> 6) & 0x3f);
	h2 = (seed >> 24) & 0xfff;
	h3 = (seed >> 36) & 0xfff;
	h3 = ((h3 & 0x3f) << 6) | ((h3 >> 6) & 0x3f);
	h4 = (seed >> 48) & 0xfff;
	k1 = h0 ^ h1 ^ h2 ^ h3 ^ h4;

	/* Algorithm choice for block 0 */
	if (sw_r32(RTL931X_L2_CTRL) & BIT(0))
		h = k1;
	else
		h = k0;

	/* Algorithm choice for block 1
	 * Since k0 and k1 are < 4096, adding 4096 will offset the hash into the second
	 * half of hash-space
	 * 4096 is in fact the hash-table size 32768 divided by 4 hashes per bucket
	 * divided by 2 to divide the hash space in 2
	 */
	if (sw_r32(RTL931X_L2_CTRL) & BIT(1))
		h |= (k1 + 4096) << 16;
	else
		h |= (k0 + 4096) << 16;

	return h;
}

/* Fills an L2 entry structure from the SoC registers */
static void rtl931x_fill_l2_entry(u32 r[], struct rtl838x_l2_entry *e)
{
	pr_debug("In %s valid?\n", __func__);
	e->valid = !!(r[0] & BIT(31));
	if (!e->valid)
		return;

	pr_debug("%s: entry valid, raw: %08x %08x %08x %08x\n", __func__, r[0], r[1], r[2], r[3]);
	e->is_ip_mc = false;
	e->is_ipv6_mc = false;

	e->mac[0] = r[0] >> 8;
	e->mac[1] = r[0];
	e->mac[2] = r[1] >> 24;
	e->mac[3] = r[1] >> 16;
	e->mac[4] = r[1] >> 8;
	e->mac[5] = r[1];

	e->is_open_flow = !!(r[0] & BIT(30));
	e->is_pe_forward = !!(r[0] & BIT(29));
	e->next_hop = !!(r[2] & BIT(30));
	e->rvid = (r[0] >> 16) & 0xfff;

	/* Is it a unicast entry? check multicast bit */
	if (!(e->mac[0] & 1)) {
		e->type = L2_UNICAST;
		e->is_l2_tunnel = !!(r[2] & BIT(31));
		e->is_static = !!(r[2] & BIT(13));
		e->port = (r[2] >> 19) & 0x3ff;
		/* Check for trunk port */
		if (r[2] & BIT(29)) {
			e->is_trunk = true;
			e->stack_dev = (e->port >> 9) & 1;
			e->trunk = e->port & 0x3f;
		} else {
			e->is_trunk = false;
			e->stack_dev = (e->port >> 6) & 0xf;
			e->port = e->port & 0x3f;
		}

		e->block_da = !!(r[2] & BIT(14));
		e->block_sa = !!(r[2] & BIT(15));
		e->suspended = !!(r[2] & BIT(12));
		e->age = (r[2] >> 16) & 3;

		/* the UC_VID field in hardware is used for the VID or for the route id */
		if (e->next_hop) {
			e->nh_route_id = r[2] & 0x7ff;
			e->vid = 0;
		} else {
			e->vid = r[2] & 0xfff;
			e->nh_route_id = 0;
		}
		if (e->is_l2_tunnel)
			e->l2_tunnel_id = ((r[2] & 0xff) << 4) | (r[3] >> 28);
		/* TODO: Implement VLAN conversion */
	} else {
		e->type = L2_MULTICAST;
		e->is_local_forward = !!(r[2] & BIT(31));
		e->is_remote_forward = !!(r[2] & BIT(17));
		e->mc_portmask_index = (r[2] >> 18) & 0xfff;
		e->l2_tunnel_list_id = (r[2] >> 4) & 0x1fff;
	}
}

/* Fills the 3 SoC table registers r[] with the information of in the rtl838x_l2_entry */
static void rtl931x_fill_l2_row(u32 r[], struct rtl838x_l2_entry *e)
{
	u32 port;

	if (!e->valid) {
		r[0] = r[1] = r[2] = 0;
		return;
	}

	r[2] = BIT(31);	/* Set valid bit */

	r[0] = ((u32)e->mac[0]) << 24 |
	       ((u32)e->mac[1]) << 16 |
	       ((u32)e->mac[2]) << 8 |
	       ((u32)e->mac[3]);
	r[1] = ((u32)e->mac[4]) << 24 |
	       ((u32)e->mac[5]) << 16;

	r[2] |= e->next_hop ? BIT(12) : 0;

	if (e->type == L2_UNICAST) {
		r[2] |= e->is_static ? BIT(14) : 0;
		r[1] |= e->rvid & 0xfff;
		r[2] |= (e->port & 0x3ff) << 20;
		if (e->is_trunk) {
			r[2] |= BIT(30);
			port = e->stack_dev << 9 | (e->port & 0x3f);
		} else {
			port = (e->stack_dev & 0xf) << 6;
			port |= e->port & 0x3f;
		}
		r[2] |= port << 20;
		r[2] |= e->block_da ? BIT(15) : 0;
		r[2] |= e->block_sa ? BIT(17) : 0;
		r[2] |= e->suspended ? BIT(13) : 0;
		r[2] |= (e->age & 0x3) << 17;
		/* the UC_VID field in hardware is used for the VID or for the route id */
		if (e->next_hop)
			r[2] |= e->nh_route_id & 0x7ff;
		else
			r[2] |= e->vid & 0xfff;
	} else { /* L2_MULTICAST */
		r[2] |= (e->mc_portmask_index & 0x3ff) << 16;
		r[2] |= e->mc_mac_index & 0x7ff;
	}
}

/* Read an L2 UC or MC entry out of a hash bucket of the L2 forwarding table
 * hash is the id of the bucket and pos is the position of the entry in that bucket
 * The data read from the SoC is filled into rtl838x_l2_entry
 */
static u64 rtl931x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[4];
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 0);
	u32 idx;
	u64 mac;
	u64 seed;

	pr_debug("%s: hash %08x, pos: %d\n", __func__, hash, pos);

	/* On the RTL93xx, 2 different hash algorithms are used making it a total of
	 * 8 buckets that need to be searched, 4 for each hash-half
	 * Use second hash space when bucket is between 4 and 8
	 */
	if (pos >= 4) {
		pos -= 4;
		hash >>= 16;
	} else {
		hash &= 0xffff;
	}

	idx = (0 << 14) | (hash << 2) | pos; /* Search SRAM, with hash and at pos in bucket */
	pr_debug("%s: NOW hash %08x, pos: %d\n", __func__, hash, pos);

	rtl_table_read(q, idx);
	for (int i = 0; i < 4; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl931x_fill_l2_entry(r, e);

	pr_debug("%s: valid: %d, nh: %d\n", __func__, e->valid, e->next_hop);
	if (!e->valid)
		return 0;

	mac = ((u64)e->mac[0]) << 40 |
	      ((u64)e->mac[1]) << 32 |
	      ((u64)e->mac[2]) << 24 |
	      ((u64)e->mac[3]) << 16 |
	      ((u64)e->mac[4]) << 8 |
	      ((u64)e->mac[5]);

	seed = rtl931x_l2_hash_seed(mac, e->rvid);
	pr_debug("%s: mac %016llx, seed %016llx\n", __func__, mac, seed);

	/* return vid with concatenated mac as unique id */
	return seed;
}

static u64 rtl931x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	return 0;
}

static void rtl931x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
}

static void rtl931x_write_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[4];
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 0);
	u32 idx = (0 << 14) | (hash << 2) | pos; /* Access SRAM, with hash and at pos in bucket */

	pr_debug("%s: hash %d, pos %d\n", __func__, hash, pos);
	pr_debug("%s: index %d -> mac %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, idx,
		e->mac[0], e->mac[1], e->mac[2], e->mac[3],e->mac[4],e->mac[5]);

	rtl931x_fill_l2_row(r, e);
	pr_debug("%s: %d: %08x %08x %08x\n", __func__, idx, r[0], r[1], r[2]);

	for (int i = 0; i < 4; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static void rtl931x_vlan_fwd_on_inner(int port, bool is_set)
{
	/* Always set all tag modes to fwd based on either inner or outer tag */
	if (is_set)
		sw_w32_mask(0xf, 0, RTL931X_VLAN_PORT_FWD + (port << 2));
	else
		sw_w32_mask(0, 0xf, RTL931X_VLAN_PORT_FWD + (port << 2));
}

static void
rtl931x_vlan_profile_setup(struct rtl838x_switch_priv *priv, int profile)
{
	u32 p[7];

	pr_debug("In %s\n", __func__);

	if (profile > 15)
		return;

	p[0] = sw_r32(RTL931X_VLAN_PROFILE_SET(profile));

	/* Enable routing of Ipv4/6 Unicast and IPv4/6 Multicast traffic */
	/* p[0] |= BIT(17) | BIT(16) | BIT(13) | BIT(12); */
	p[0] |= 0x3 << 11; /* COPY2CPU */

	p[1] = RTL931X_VLAN_L2_UNKN_MC_FLD_H(RTL931X_MC_PMASK_ALL_PORTS);
	p[2] = RTL931X_VLAN_L2_UNKN_MC_FLD_L(RTL931X_MC_PMASK_ALL_PORTS);

	if (profile & RTLDSA_VLAN_PROFILE_MC_ACTIVE_V4) {
		p[3] = RTL931X_VLAN_IP4_UNKN_MC_FLD_H(priv->mc_router_portmask);
		p[4] = RTL931X_VLAN_IP4_UNKN_MC_FLD_L(priv->mc_router_portmask);
	} else {
		p[3] = RTL931X_VLAN_IP4_UNKN_MC_FLD_H(RTL931X_MC_PMASK_ALL_PORTS);
		p[4] = RTL931X_VLAN_IP4_UNKN_MC_FLD_L(RTL931X_MC_PMASK_ALL_PORTS);
	}

	if (profile & RTLDSA_VLAN_PROFILE_MC_ACTIVE_V6) {
		p[5] = RTL931X_VLAN_IP6_UNKN_MC_FLD_H(priv->mc_router_portmask);
		p[6] = RTL931X_VLAN_IP6_UNKN_MC_FLD_L(priv->mc_router_portmask);
	} else {
		p[5] = RTL931X_VLAN_IP6_UNKN_MC_FLD_H(RTL931X_MC_PMASK_ALL_PORTS);
		p[6] = RTL931X_VLAN_IP6_UNKN_MC_FLD_L(RTL931X_MC_PMASK_ALL_PORTS);
	}

	for (int i = 0; i < 7; i++)
		sw_w32(p[i], RTL931X_VLAN_PROFILE_SET(profile) + i * 4);
	pr_debug("Leaving %s\n", __func__);
}

static void rtl931x_l2_learning_setup(void)
{
	/* Portmask for flooding broadcast traffic */
	rtl839x_set_port_reg_be(RTL931X_MC_PMASK_ALL_PORTS, RTL931X_L2_BC_FLD_PMSK);

	/* Portmask for flooding unicast traffic with unknown destination */
	rtl839x_set_port_reg_be(RTL931X_MC_PMASK_ALL_PORTS, RTL931X_L2_UNKN_UC_FLD_PMSK);

	/* Limit learning to maximum: 64k entries, after that just flood (bits 0-2) */
	sw_w32((0xffff << 3) | FORWARD, RTL931X_L2_LRN_CONSTRT_CTRL);
}

static u64 rtl931x_read_mcast_pmask(int idx)
{
	u64 portmask;
	/* Read MC_PMSK (2) via register RTL9310_TBL_0 */
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	portmask <<= 32;
	portmask |= sw_r32(rtl_table_data(q, 1));
	portmask >>= 7;
	rtl_table_release(q);

	pr_debug("%s: Index idx %d has portmask %016llx\n", __func__, idx, portmask);

	return portmask;
}

static void rtl931x_write_mcast_pmask(int idx, u64 portmask)
{
	u64 pm = portmask;

	/* Access MC_PMSK (2) via register RTL9310_TBL_0 */
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 2);

	pr_debug("%s: Index idx %d has portmask %016llx\n", __func__, idx, pm);
	pm <<= 7;
	sw_w32((u32)(pm >> 32), rtl_table_data(q, 0));
	sw_w32((u32)pm, rtl_table_data(q, 1));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static int rtl931x_set_ageing_time(unsigned long msec)
{
	int t = sw_r32(RTL931X_L2_AGE_CTRL);

	t &= 0x1FFFFF;
	t = (t * 8) / 10;
	pr_debug("L2 AGING time: %d sec\n", t);

	t = (msec / 100 + 7) / 8;
	t = t > 0x1FFFFF ? 0x1FFFFF : t;
	sw_w32_mask(0x1FFFFF, t, RTL931X_L2_AGE_CTRL);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(RTL931X_L2_PORT_AGE_CTRL));

	return 0;
}

static void rtl931x_pie_lookup_enable(struct rtl838x_switch_priv *priv, int index)
{
	int block = index / PIE_BLOCK_SIZE;

	sw_w32_mask(0, BIT(block), RTL931X_PIE_BLK_LOOKUP_CTRL);
}

/* Fills the data in the intermediate representation in the pie_rule structure
 * into a data field for a given template field field_type
 * TODO: This function looks very similar to the function of the RTL9310, but
 * since it uses the physical template_field_id, which are different for each
 * SoC and there are other field types, it is actually not. If we would also use
 * an intermediate representation for a field type, we would could have one
 * pie_data_fill function for all SoCs, provided we have also for each SoC a
 * function to map between physical and intermediate field type
 */

static int rtl931x_pie_data_fill(enum template_field_id field_type, struct pie_rule *pr, u16 *data, u16 *data_m)
{
	*data = *data_m = 0;

	switch (field_type) {
	case TEMPLATE_FIELD_SPM0:
		*data = pr->spm;
		*data_m = pr->spm_m;
		break;
	case TEMPLATE_FIELD_SPM1:
		*data = pr->spm >> 16;
		*data_m = pr->spm_m >> 16;
		break;
	case TEMPLATE_FIELD_OTAG:
		*data = pr->otag;
		*data_m = pr->otag_m;
		break;
	case TEMPLATE_FIELD_SMAC0:
		*data = pr->smac[4];
		*data = (*data << 8) | pr->smac[5];
		*data_m = pr->smac_m[4];
		*data_m = (*data_m << 8) | pr->smac_m[5];
		break;
	case TEMPLATE_FIELD_SMAC1:
		*data = pr->smac[2];
		*data = (*data << 8) | pr->smac[3];
		*data_m = pr->smac_m[2];
		*data_m = (*data_m << 8) | pr->smac_m[3];
		break;
	case TEMPLATE_FIELD_SMAC2:
		*data = pr->smac[0];
		*data = (*data << 8) | pr->smac[1];
		*data_m = pr->smac_m[0];
		*data_m = (*data_m << 8) | pr->smac_m[1];
		break;
	case TEMPLATE_FIELD_DMAC0:
		*data = pr->dmac[4];
		*data = (*data << 8) | pr->dmac[5];
		*data_m = pr->dmac_m[4];
		*data_m = (*data_m << 8) | pr->dmac_m[5];
		break;
	case TEMPLATE_FIELD_DMAC1:
		*data = pr->dmac[2];
		*data = (*data << 8) | pr->dmac[3];
		*data_m = pr->dmac_m[2];
		*data_m = (*data_m << 8) | pr->dmac_m[3];
		break;
	case TEMPLATE_FIELD_DMAC2:
		*data = pr->dmac[0];
		*data = (*data << 8) | pr->dmac[1];
		*data_m = pr->dmac_m[0];
		*data_m = (*data_m << 8) | pr->dmac_m[1];
		break;
	case TEMPLATE_FIELD_ETHERTYPE:
		*data = pr->ethertype;
		*data_m = pr->ethertype_m;
		break;
	case TEMPLATE_FIELD_ITAG:
		*data = pr->itag;
		*data_m = pr->itag_m;
		break;
	case TEMPLATE_FIELD_SIP0:
		if (pr->is_ipv6) {
			*data = pr->sip6.s6_addr16[7];
			*data_m = pr->sip6_m.s6_addr16[7];
		} else {
			*data = pr->sip;
			*data_m = pr->sip_m;
		}
		break;
	case TEMPLATE_FIELD_SIP1:
		if (pr->is_ipv6) {
			*data = pr->sip6.s6_addr16[6];
			*data_m = pr->sip6_m.s6_addr16[6];
		} else {
			*data = pr->sip >> 16;
			*data_m = pr->sip_m >> 16;
		}
		break;
	case TEMPLATE_FIELD_SIP2:
	case TEMPLATE_FIELD_SIP3:
	case TEMPLATE_FIELD_SIP4:
	case TEMPLATE_FIELD_SIP5:
	case TEMPLATE_FIELD_SIP6:
	case TEMPLATE_FIELD_SIP7:
		*data = pr->sip6.s6_addr16[5 - (field_type - TEMPLATE_FIELD_SIP2)];
		*data_m = pr->sip6_m.s6_addr16[5 - (field_type - TEMPLATE_FIELD_SIP2)];
		break;
	case TEMPLATE_FIELD_DIP0:
		if (pr->is_ipv6) {
			*data = pr->dip6.s6_addr16[7];
			*data_m = pr->dip6_m.s6_addr16[7];
		} else {
			*data = pr->dip;
			*data_m = pr->dip_m;
		}
		break;
		case TEMPLATE_FIELD_DIP1:
		if (pr->is_ipv6) {
			*data = pr->dip6.s6_addr16[6];
			*data_m = pr->dip6_m.s6_addr16[6];
		} else {
			*data = pr->dip >> 16;
			*data_m = pr->dip_m >> 16;
		}
		break;
	case TEMPLATE_FIELD_DIP2:
	case TEMPLATE_FIELD_DIP3:
	case TEMPLATE_FIELD_DIP4:
	case TEMPLATE_FIELD_DIP5:
	case TEMPLATE_FIELD_DIP6:
	case TEMPLATE_FIELD_DIP7:
		*data = pr->dip6.s6_addr16[5 - (field_type - TEMPLATE_FIELD_DIP2)];
		*data_m = pr->dip6_m.s6_addr16[5 - (field_type - TEMPLATE_FIELD_DIP2)];
		break;
	case TEMPLATE_FIELD_IP_TOS_PROTO:
		*data = pr->tos_proto;
		*data_m = pr->tos_proto_m;
		break;
	case TEMPLATE_FIELD_L4_SPORT:
		*data = pr->sport;
		*data_m = pr->sport_m;
		break;
	case TEMPLATE_FIELD_L4_DPORT:
		*data = pr->dport;
		*data_m = pr->dport_m;
		break;
	case TEMPLATE_FIELD_DSAP_SSAP:
		*data = pr->dsap_ssap;
		*data_m = pr->dsap_ssap_m;
		break;
	case TEMPLATE_FIELD_TCP_INFO:
		*data = pr->tcp_info;
		*data_m = pr->tcp_info_m;
		break;
	case TEMPLATE_FIELD_RANGE_CHK:
		pr_debug("TEMPLATE_FIELD_RANGE_CHK: not configured\n");
		break;
	default:
		pr_debug("%s: unknown field %d\n", __func__, field_type);
		return -1;
	}

	return 0;
}

/* Reads the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure and fills in the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9310,
 * however the RTL931X has 2 more registers / fields and the physical field-ids are different
 * on all SoCs
 * On the RTL9310 the mask fields are not word-aligend!
 */
static void rtl931x_write_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		u16 data, data_m;

		rtl931x_pie_data_fill(t[i], pr, &data, &data_m);

		/* On the RTL9310, the mask fields are not word aligned! */
		if (!(i % 2)) {
			r[5 - i / 2] = data;
			r[12 - i / 2] |= ((u32)data_m << 8);
		} else {
			r[5 - i / 2] |= ((u32)data) << 16;
			r[12 - i / 2] |= ((u32)data_m) << 24;
			r[11 - i / 2] |= ((u32)data_m) >> 8;
		}
	}
}

// Currently unused
// static void rtl931x_read_pie_fixed_fields(u32 r[], struct pie_rule *pr)
// {
// 	pr->mgnt_vlan = r[7] & BIT(31);
// 	if (pr->phase == PHASE_IACL)
// 		pr->dmac_hit_sw = r[7] & BIT(30);
// 	else  /* TODO: EACL/VACL phase handling */
// 		pr->content_too_deep = r[7] & BIT(30);
// 	pr->not_first_frag = r[7]  & BIT(29);
// 	pr->frame_type_l4 = (r[7] >> 26) & 7;
// 	pr->frame_type = (r[7] >> 24) & 3;
// 	pr->otag_fmt = (r[7] >> 23) & 1;
// 	pr->itag_fmt = (r[7] >> 22) & 1;
// 	pr->otag_exist = (r[7] >> 21) & 1;
// 	pr->itag_exist = (r[7] >> 20) & 1;
// 	pr->frame_type_l2 = (r[7] >> 18) & 3;
// 	pr->igr_normal_port = (r[7] >> 17) & 1;
// 	pr->tid = (r[7] >> 16) & 1;

// 	pr->mgnt_vlan_m = r[14] & BIT(15);
// 	if (pr->phase == PHASE_IACL)
// 		pr->dmac_hit_sw_m = r[14] & BIT(14);
// 	else
// 		pr->content_too_deep_m = r[14] & BIT(14);
// 	pr->not_first_frag_m = r[14] & BIT(13);
// 	pr->frame_type_l4_m = (r[14] >> 10) & 7;
// 	pr->frame_type_m = (r[14] >> 8) & 3;
// 	pr->otag_fmt_m = r[14] & BIT(7);
// 	pr->itag_fmt_m = r[14] & BIT(6);
// 	pr->otag_exist_m = r[14] & BIT(5);
// 	pr->itag_exist_m = r[14] & BIT (4);
// 	pr->frame_type_l2_m = (r[14] >> 2) & 3;
// 	pr->igr_normal_port_m = r[14] & BIT(1);
// 	pr->tid_m = r[14] & 1;

// 	pr->valid = r[15] & BIT(31);
// 	pr->cond_not = r[15] & BIT(30);
// 	pr->cond_and1 = r[15] & BIT(29);
// 	pr->cond_and2 = r[15] & BIT(28);
// }

static void rtl931x_write_pie_fixed_fields(u32 r[],  struct pie_rule *pr)
{
	r[7] |= pr->mgnt_vlan ? BIT(31) : 0;
	if (pr->phase == PHASE_IACL)
		r[7] |= pr->dmac_hit_sw ? BIT(30) : 0;
	else
		r[7] |= pr->content_too_deep ? BIT(30) : 0;
	r[7] |= pr->not_first_frag ? BIT(29) : 0;
	r[7] |= ((u32) (pr->frame_type_l4 & 0x7)) << 26;
	r[7] |= ((u32) (pr->frame_type & 0x3)) << 24;
	r[7] |= pr->otag_fmt ? BIT(23) : 0;
	r[7] |= pr->itag_fmt ? BIT(22) : 0;
	r[7] |= pr->otag_exist ? BIT(21) : 0;
	r[7] |= pr->itag_exist ? BIT(20) : 0;
	r[7] |= ((u32) (pr->frame_type_l2 & 0x3)) << 18;
	r[7] |= pr->igr_normal_port ? BIT(17) : 0;
	r[7] |= ((u32) (pr->tid & 0x1)) << 16;

	r[14] |= pr->mgnt_vlan_m ? BIT(15) : 0;
	if (pr->phase == PHASE_IACL)
		r[14] |= pr->dmac_hit_sw_m ? BIT(14) : 0;
	else
		r[14] |= pr->content_too_deep_m ? BIT(14) : 0;
	r[14] |= pr->not_first_frag_m ? BIT(13) : 0;
	r[14] |= ((u32) (pr->frame_type_l4_m & 0x7)) << 10;
	r[14] |= ((u32) (pr->frame_type_m & 0x3)) << 8;
	r[14] |= pr->otag_fmt_m ? BIT(7) : 0;
	r[14] |= pr->itag_fmt_m ? BIT(6) : 0;
	r[14] |= pr->otag_exist_m ? BIT(5) : 0;
	r[14] |= pr->itag_exist_m ? BIT(4) : 0;
	r[14] |= ((u32) (pr->frame_type_l2_m & 0x3)) << 2;
	r[14] |= pr->igr_normal_port_m ? BIT(1) : 0;
	r[14] |= (u32) (pr->tid_m & 0x1);

	r[15] |= pr->valid ? BIT(31) : 0;
	r[15] |= pr->cond_not ? BIT(30) : 0;
	r[15] |= pr->cond_and1 ? BIT(29) : 0;
	r[15] |= pr->cond_and2 ? BIT(28) : 0;
}

static void rtl931x_write_pie_action(u32 r[],  struct pie_rule *pr)
{
	/* Either drop or forward */
	if (pr->drop) {
		r[15] |= BIT(11) | BIT(12) | BIT(13); /* Do Green, Yellow and Red drops */
		/* Actually DROP, not PERMIT in Green / Yellow / Red */
		r[16] |= BIT(27) | BIT(28) | BIT(29);
	} else {
		r[15] |= pr->fwd_sel ? BIT(14) : 0;
		r[16] |= pr->fwd_act << 24;
		r[16] |= BIT(21); /* We overwrite any drop */
	}
	if (pr->phase == PHASE_VACL)
		r[16] |= pr->fwd_sa_lrn ? BIT(22) : 0;
	r[15] |= pr->bypass_sel ? BIT(10) : 0;
	r[15] |= pr->nopri_sel ? BIT(21) : 0;
	r[15] |= pr->tagst_sel ? BIT(20) : 0;
	r[15] |= pr->ovid_sel ? BIT(18) : 0;
	r[15] |= pr->ivid_sel ? BIT(16) : 0;
	r[15] |= pr->meter_sel ? BIT(27) : 0;
	r[15] |= pr->mir_sel ? BIT(15) : 0;
	r[15] |= pr->log_sel ? BIT(26) : 0;

	r[16] |= ((u32)(pr->fwd_data & 0xfff)) << 9;
/*	r[15] |= pr->log_octets ? BIT(31) : 0; */
	r[15] |= (u32)(pr->meter_data) >> 2;
	r[16] |= (((u32)(pr->meter_data) >> 7) & 0x3) << 29;

	r[16] |= ((u32)(pr->ivid_act & 0x3)) << 21;
	r[15] |= ((u32)(pr->ivid_data & 0xfff)) << 9;
	r[16] |= ((u32)(pr->ovid_act & 0x3)) << 30;
	r[16] |= ((u32)(pr->ovid_data & 0xfff)) << 16;
	r[16] |= ((u32)(pr->mir_data & 0x3)) << 6;
	r[17] |= ((u32)(pr->tagst_data & 0xf)) << 28;
	r[17] |= ((u32)(pr->nopri_data & 0x7)) << 25;
	r[17] |= pr->bypass_ibc_sc ? BIT(16) : 0;
}

static void rtl931x_pie_rule_dump_raw(u32 r[])
{
	pr_debug("Raw IACL table entry:\n");
	pr_debug("r 0 - 7: %08x %08x %08x %08x %08x %08x %08x %08x\n",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
	pr_debug("r 8 - 15: %08x %08x %08x %08x %08x %08x %08x %08x\n",
		r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15]);
	pr_debug("r 16 - 18: %08x %08x %08x\n", r[16], r[17], r[18]);
	pr_debug("Match  : %08x %08x %08x %08x %08x %08x\n", r[0], r[1], r[2], r[3], r[4], r[5]);
	pr_debug("Fixed  : %06x\n", r[6] >> 8);
	pr_debug("Match M: %08x %08x %08x %08x %08x %08x\n",
		(r[6] << 24) | (r[7] >> 8), (r[7] << 24) | (r[8] >> 8), (r[8] << 24) | (r[9] >> 8),
		(r[9] << 24) | (r[10] >> 8), (r[10] << 24) | (r[11] >> 8),
		(r[11] << 24) | (r[12] >> 8));
	pr_debug("R[13]:   %08x\n", r[13]);
	pr_debug("Fixed M: %06x\n", ((r[12] << 16) | (r[13] >> 16)) & 0xffffff);
	pr_debug("Valid / not / and1 / and2 : %1x\n", (r[13] >> 12) & 0xf);
	pr_debug("r 13-16: %08x %08x %08x %08x\n", r[13], r[14], r[15], r[16]);
}

/* Get the Destination-MAC of an L3 egress interface or the Source MAC for routed packets
 * from the SoC's L3_EGR_INTF_MAC table
 * Indexes 0-2047 are DMACs, 2048+ are SMACs
 */
static u64 rtl931x_get_l3_egress_mac(u32 idx)
{
	u64 mac;
	/* Read L3_EGR_INTF_MAC table (2) via register RTL9310_TBL_2 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 2);

	rtl_table_read(r, idx);
	/* The table has a size of 2 registers */
	mac = sw_r32(rtl_table_data(r, 0));
	mac <<= 32;
	mac |= sw_r32(rtl_table_data(r, 1));
	rtl_table_release(r);

	return mac;
}

/* Set the Destination-MAC of a route or the Source MAC of an L3 egress interface
 * in the SoC's L3_EGR_INTF_MAC table
 * Indexes 0-2047 are DMACs, 2048+ are SMACs
 */
static void rtl931x_set_l3_egress_mac(u32 idx, u64 mac)
{
	/* Access L3_EGR_INTF_MAC table (2) via register RTL9310_TBL_2 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_2, 2);

	/* The table has a size of 2 registers */
	sw_w32(mac >> 32, rtl_table_data(r, 0));
	sw_w32(mac, rtl_table_data(r, 1));

	pr_debug("%s: setting index %d to %016llx\n", __func__, idx, mac);
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

/* Sets up an egress interface for L3 actions
 * Actions for ip4/6_icmp_redirect, ip4/6_pbr_icmp_redirect are:
 * 0: FORWARD, 1: DROP, 2: TRAP2CPU, 3: COPY2CPU, 4: TRAP2MASTERCPU 5: COPY2MASTERCPU
 * 6: HARDDROP
 * idx is the index in the HW interface table: idx < 0x80
 */
static void rtl931x_set_l3_egress_intf(int idx, struct rtl838x_l3_intf *intf)
{
	u32 u, v;
	/* Read L3_EGR_INTF table (4) via register RTL9310_TBL_3 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 4);

	/* The table has 2 registers */
	u = (intf->vid & 0xfff) << 9;
	u |= (intf->smac_idx & 0x3f) << 3;
	u |= (intf->ip4_mtu_id & 0x7);

	v = (intf->ip6_mtu_id & 0x7) << 28;
	v |= (intf->ttl_scope & 0xff) << 20;
	v |= (intf->hl_scope & 0xff) << 12;
	v |= (intf->ip4_icmp_redirect & 0x7) << 9;
	v |= (intf->ip6_icmp_redirect & 0x7)<< 6;
	v |= (intf->ip4_pbr_icmp_redirect & 0x7) << 3;
	v |= (intf->ip6_pbr_icmp_redirect & 0x7);

	sw_w32(u, rtl_table_data(r, 0));
	sw_w32(v, rtl_table_data(r, 1));

	pr_debug("%s writing to index %d: %08x %08x\n", __func__, idx, u, v);
	rtl_table_write(r, idx & 0x7f);
	rtl_table_release(r);
}

static int rtl931x_pie_rule_write(struct rtl838x_switch_priv *priv, int idx, struct pie_rule *pr)
{
	/* Access IACL table (0) via register 1, the table size is 4096 */
	struct table_reg *q = rtl_table_get(RTL9310_TBL_1, 0);
	u32 r[22];
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL931X_PIE_BLK_TMPLTE_CTRL(block));

	pr_debug("%s: %d, t_select: %08x\n", __func__, idx, t_select);

	for (int i = 0; i < 22; i++)
		r[i] = 0;

	if (!pr->valid) {
		rtl_table_write(q, idx);
		rtl_table_release(q);
		return 0;
	}
	rtl931x_write_pie_fixed_fields(r, pr);

	pr_debug("%s: template %d\n", __func__, (t_select >> (pr->tid * 4)) & 0xf);
	rtl931x_write_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 4)) & 0xf]);

	rtl931x_write_pie_action(r, pr);

	rtl931x_pie_rule_dump_raw(r);

	for (int i = 0; i < 22; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);

	return 0;
}

static bool rtl931x_pie_templ_has(int t, enum template_field_id field_type)
{
	for (int i = 0; i < N_FIXED_FIELDS_RTL931X; i++) {
		enum template_field_id ft = fixed_templates[t][i];
		if (field_type == ft)
			return true;
	}

	return false;
}

/* Verify that the rule pr is compatible with a given template t in block block
 * Note that this function is SoC specific since the values of e.g. TEMPLATE_FIELD_SIP0
 * depend on the SoC
 */
static int rtl931x_pie_verify_template(struct rtl838x_switch_priv *priv,
				       struct pie_rule *pr, int t, int block)
{
	int i;

	if (!pr->is_ipv6 && pr->sip_m && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_SIP0))
		return -1;

	if (!pr->is_ipv6 && pr->dip_m && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_DIP0))
		return -1;

	if (pr->is_ipv6) {
		if ((pr->sip6_m.s6_addr32[0] ||
		     pr->sip6_m.s6_addr32[1] ||
		     pr->sip6_m.s6_addr32[2] ||
		     pr->sip6_m.s6_addr32[3]) &&
		    !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_SIP2))
			return -1;
		if ((pr->dip6_m.s6_addr32[0] ||
		     pr->dip6_m.s6_addr32[1] ||
		     pr->dip6_m.s6_addr32[2] ||
		     pr->dip6_m.s6_addr32[3]) &&
		    !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_DIP2))
			return -1;
	}

	if (ether_addr_to_u64(pr->smac) && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_SMAC0))
		return -1;

	if (ether_addr_to_u64(pr->dmac) && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_DMAC0))
		return -1;

	/* TODO: Check more */

	i = find_first_zero_bit(&priv->pie_use_bm[block * 4], PIE_BLOCK_SIZE);

	if (i >= PIE_BLOCK_SIZE)
		return -1;

	return i + PIE_BLOCK_SIZE * block;
}

static int rtl931x_pie_rule_add(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx, block, j;
	int min_block = 0;
	int max_block = priv->n_pie_blocks / 2;

	if (pr->is_egress) {
		min_block = max_block;
		max_block = priv->n_pie_blocks;
	}
	pr_debug("In %s\n", __func__);

	mutex_lock(&priv->pie_mutex);

	for (block = min_block; block < max_block; block++) {
		for (j = 0; j < 2; j++) {
			int t = (sw_r32(RTL931X_PIE_BLK_TMPLTE_CTRL(block)) >> (j * 4)) & 0xf;
			pr_debug("Testing block %d, template %d, template id %d\n", block, j, t);
			pr_debug("%s: %08x\n",
				__func__, sw_r32(RTL931X_PIE_BLK_TMPLTE_CTRL(block)));
			idx = rtl931x_pie_verify_template(priv, pr, t, block);
			if (idx >= 0)
				break;
		}
		if (j < 2)
			break;
	}

	if (block >= priv->n_pie_blocks) {
		mutex_unlock(&priv->pie_mutex);
		return -EOPNOTSUPP;
	}

	pr_debug("Using block: %d, index %d, template-id %d\n", block, idx, j);
	set_bit(idx, priv->pie_use_bm);

	pr->valid = true;
	pr->tid = j;  /* Mapped to template number */
	pr->tid_m = 0x1;
	pr->id = idx;

	rtl931x_pie_lookup_enable(priv, idx);
	rtl931x_pie_rule_write(priv, idx, pr);

	mutex_unlock(&priv->pie_mutex);

	return 0;
}

/* Delete a range of Packet Inspection Engine rules */
static int rtl931x_pie_rule_del(struct rtl838x_switch_priv *priv, int index_from, int index_to)
{
	u32 v = (index_from << 1)| (index_to << 13 ) | BIT(0);

	pr_debug("%s: from %d to %d\n", __func__, index_from, index_to);
	mutex_lock(&priv->reg_mutex);

	/* Write from-to and execute bit into control register */
	sw_w32(v, RTL931X_PIE_CLR_CTRL);

	/* Wait until command has completed */
	do {
	} while (sw_r32(RTL931X_PIE_CLR_CTRL) & BIT(0));

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtl931x_pie_rule_rm(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx = pr->id;

	rtl931x_pie_rule_del(priv, idx, idx);
	clear_bit(idx, priv->pie_use_bm);
}

static void rtl931x_pie_init(struct rtl838x_switch_priv *priv)
{
	u32 template_selectors;

	mutex_init(&priv->pie_mutex);

	pr_debug("%s\n", __func__);
	/* Enable ACL lookup on all ports, including CPU_PORT */
	for (int i = 0; i <= priv->cpu_port; i++)
		sw_w32(1, RTL931X_ACL_PORT_LOOKUP_CTRL(i));

	/* Include IPG in metering */
	sw_w32_mask(0, 1, RTL931X_METER_GLB_CTRL);

	/* Delete all present rules, block size is 128 on all SoC families */
	rtl931x_pie_rule_del(priv, 0, priv->n_pie_blocks * 128 - 1);

	/* Assign first half blocks 0-7 to VACL phase, second half to IACL */
	/* 3 bits are used for each block, values for PIE blocks are */
	/* 6: Disabled, 0: VACL, 1: IACL, 2: EACL */
	/* And for OpenFlow Flow blocks: 3: Ingress Flow table 0, */
	/* 4: Ingress Flow Table 3, 5: Egress flow table 0 */
	for (int i = 0; i < priv->n_pie_blocks; i++) {
		int pos = (i % 10) * 3;
		u32 r = RTL931X_PIE_BLK_PHASE_CTRL + 4 * (i / 10);

		if (i < priv->n_pie_blocks / 2)
			sw_w32_mask(0x7 << pos, 0, r);
		else
			sw_w32_mask(0x7 << pos, 1 << pos, r);
	}

	/* Enable predefined templates 0, 1 for first quarter of all blocks */
	template_selectors = 0 | (1 << 4);
	for (int i = 0; i < priv->n_pie_blocks / 4; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for second quarter of all blocks */
	template_selectors = 2 | (3 << 4);
	for (int i = priv->n_pie_blocks / 4; i < priv->n_pie_blocks / 2; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 0, 1 for third quater of all blocks */
	template_selectors = 0 | (1 << 4);
	for (int i = priv->n_pie_blocks / 2; i < priv->n_pie_blocks * 3 / 4; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for fourth quater of all blocks */
	template_selectors = 2 | (3 << 4);
	for (int i = priv->n_pie_blocks * 3 / 4; i < priv->n_pie_blocks; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

}

static int rtl931x_l3_setup(struct rtl838x_switch_priv *priv)
{
	return 0;
}

static void rtl931x_vlan_port_keep_tag_set(int port, bool keep_outer, bool keep_inner)
{
	sw_w32(FIELD_PREP(RTL931X_VLAN_PORT_TAG_EGR_OTAG_STS_MASK,
			  keep_outer ? RTL931X_VLAN_PORT_TAG_STS_TAGGED : RTL931X_VLAN_PORT_TAG_STS_UNTAG) |
	       FIELD_PREP(RTL931X_VLAN_PORT_TAG_EGR_ITAG_STS_MASK,
			  keep_inner ? RTL931X_VLAN_PORT_TAG_STS_TAGGED : RTL931X_VLAN_PORT_TAG_STS_UNTAG),
	       RTL931X_VLAN_PORT_TAG_CTRL(port));
}

static void rtl931x_vlan_port_pvidmode_set(int port, enum pbvlan_type type, enum pbvlan_mode mode)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0x3 << 12, mode << 12, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
	else
		sw_w32_mask(0x3 << 26, mode << 26, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
}

static void rtl931x_vlan_port_pvid_set(int port, enum pbvlan_type type, int pvid)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0xfff, pvid, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
	else
		sw_w32_mask(0xfff << 14, pvid << 14, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
}

static void rtl931x_set_igr_filter(int port, enum igr_filter state)
{
	sw_w32_mask(0x3 << ((port & 0xf)<<1), state << ((port & 0xf)<<1),
		    RTL931X_VLAN_PORT_IGR_FLTR + (((port >> 4) << 2)));
}

static void rtl931x_set_egr_filter(int port,  enum egr_filter state)
{
	sw_w32_mask(0x1 << (port % 0x20), state << (port % 0x20),
		    RTL931X_VLAN_PORT_EGR_FLTR + (((port >> 5) << 2)));
}

static void rtl931x_set_distribution_algorithm(int group, int algoidx, u32 algomsk)
{
	u32 l3shift = 0;
	u32 newmask = 0;

	/* TODO: for now we set algoidx to 0 */
	algoidx = 0;

	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SIP_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_DIP_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT;
	}

	if (l3shift == 4) {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT;
	} else {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT;
	}

	sw_w32(newmask << l3shift, RTL931X_TRK_HASH_CTRL + (algoidx << 2));
}

static void rtl931x_led_init(struct rtl838x_switch_priv *priv)
{
	u64 pm_copper = 0, pm_fiber = 0;
	struct device_node *node;

	pr_debug("%s called\n", __func__);
	node = of_find_compatible_node(NULL, NULL, "realtek,rtl9300-leds");
	if (!node) {
		pr_debug("%s No compatible LED node found\n", __func__);
		return;
	}

	for (int i = 0; i < priv->cpu_port; i++) {
		int pos = (i << 1) % 32;
		u32 set;

		sw_w32_mask(0x3 << pos, 0, RTL931X_LED_PORT_FIB_SET_SEL_CTRL(i));
		sw_w32_mask(0x3 << pos, 0, RTL931X_LED_PORT_COPR_SET_SEL_CTRL(i));

		if (!priv->ports[i].phy)
			continue;

		/* 0x0 = 1 led, 0x1 = 2 leds, 0x2 = 3 leds, 0x3 = 4 leds per port */
		sw_w32_mask(0x3 << pos, (priv->ports[i].leds_on_this_port - 1) << pos,
			    RTL931X_LED_PORT_NUM_CTRL(i));

		if (priv->ports[i].phy_is_integrated)
			pm_fiber |= BIT_ULL(i);
		else
			pm_copper |= BIT_ULL(i);

		set = priv->ports[i].led_set;
		sw_w32_mask(0, set << pos, RTL931X_LED_PORT_COPR_SET_SEL_CTRL(i));
		sw_w32_mask(0, set << pos, RTL931X_LED_PORT_FIB_SET_SEL_CTRL(i));
	}

	for (int i = 0; i < 4; i++) {
		const __be32 *led_set;
		char set_name[9];
		u32 setlen;
		u32 v;

		sprintf(set_name, "led_set%d", i);
		pr_debug(">%s<\n", set_name);
		led_set = of_get_property(node, set_name, &setlen);
		if (!led_set || setlen != 16)
			break;
		v = be32_to_cpup(led_set) << 16 | be32_to_cpup(led_set + 1);
		sw_w32(v, RTL931X_LED_SET0_0_CTRL - 4 - i * 8);
		v = be32_to_cpup(led_set + 2) << 16 | be32_to_cpup(led_set + 3);
		sw_w32(v, RTL931X_LED_SET0_0_CTRL - i * 8);
	}

	/* Set LED mode to serial (0x1) */
	sw_w32_mask(0x3, 0x1, RTL931X_LED_GLB_CTRL);

	rtl839x_set_port_reg_le(pm_copper, RTL931X_LED_PORT_COPR_MASK_CTRL);
	rtl839x_set_port_reg_le(pm_fiber, RTL931X_LED_PORT_FIB_MASK_CTRL);
	rtl839x_set_port_reg_le(pm_copper | pm_fiber, RTL931X_LED_PORT_COMBO_MASK_CTRL);

	for (int i = 0; i < 32; i++)
		pr_debug("%s %08x: %08x\n",__func__, 0xbb000600 + i * 4, sw_r32(0x0600 + i * 4));
}

u8 port_sds[RTL931X_CPU_PORT] = {2, 0, 0, 0, 0, 0, 0, 0,
				 3, 0, 0, 0, 0, 0, 0, 0,
				 4, 0, 0, 0, 0, 0, 0, 0,
				 5, 0, 0, 0, 0, 0, 0, 0,
				 6, 0, 0, 0, 0, 0, 0, 0,
				 7, 0, 0, 0, 0, 0, 0, 0,
				 8, 0, 9, 0, 0, 0, 0, 0};

static void rtl9310_sds_mode_usxgmii(int sds_num)
{
	u32 a[12] = {0x13a4, 0x13a4, 0x13a8, 0x13a8, 0x13AC, 0x13B0, 0x13B4, 0x13B4,
			0x13B8, 0x13B8, 0x13B8, 0x13B8};
	u32 o[12] = {0, 16, 0, 16, 0, 0, 0, 16, 0, 4, 8, 12};
	u32 v[12] = {0x5101, 0x5101, 0x5101, 0x5101, 0x5101, 0x50101, 0x55, 0x55, 0xf, 0xf, 0xf, 0xf};
// uboot uses
//	u32 v[12] = {0x4101, 0x4101, 0x4101, 0x4101, 0x4101, 0x40101, 0x45, 0x45, 0x9, 0x9, 0x9, 0x9};
	u32 m[12] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7ffff, 0xff, 0xff, 0xf, 0xf, 0xf, 0xf};
	int n = sds_num - 2;

	if (sds_num < 0 || sds_num > 13)
		return;

	sw_w32_mask(m[n] << o[n], v[n] << o[n], a[n]);
}

static void rtl931x_phylink_mac_config(struct dsa_switch *ds, int port,
					unsigned int mode,
					const struct phylink_link_state *state)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int sds_num;
	u32 reg, band;

	/* Nothing to be done for the CPU-port */
	if (port == priv->cpu_port)
		return;

	sds_num = priv->ports[port].sds_num;

	reg = sw_r32(rtl931x_mac_force_mode_ctrl(port));
	pr_info("%s: speed %d sds_num %d reg force %X\n", __func__, state->speed, sds_num, reg);

	reg &= ~(RTL931X_DUPLEX_MODE | RTL931X_FORCE_EN | RTL931X_FORCE_LINK_EN);

	switch (state->interface) {
	case PHY_INTERFACE_MODE_HSGMII:
		pr_info("%s setting mode PHY_INTERFACE_MODE_HSGMII\n", __func__);
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_HSGMII);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_HSGMII);
		band = rtl931x_sds_cmu_band_set(sds_num, true, 62, PHY_INTERFACE_MODE_HSGMII);
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_1000BASEX);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_1000BASEX);
		break;
	case PHY_INTERFACE_MODE_XGMII:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_XGMII);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_XGMII);
		break;
	case PHY_INTERFACE_MODE_10GBASER:
		fallthrough;
	case PHY_INTERFACE_MODE_10GKR:
//		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_10GBASER);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_10GBASER);
//		reg |= RTL931X_FORCE_EN | RTL931X_FORCE_LINK_EN;
		break;
	case PHY_INTERFACE_MODE_USXGMII:
		/* Translates to MII_USXGMII_10GSXGMII */
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_USXGMII);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_USXGMII);
		break;
	case PHY_INTERFACE_MODE_SGMII:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_SGMII);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_SGMII);
		band = rtl931x_sds_cmu_band_set(sds_num, true, 62, PHY_INTERFACE_MODE_SGMII);
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_QSGMII);
		rtl931x_sds_init(sds_num, port, PHY_INTERFACE_MODE_QSGMII);
		break;
	default:
		pr_err("%s: unknown serdes mode: %s\n",
			__func__, phy_modes(state->interface));
		return;
	}

	if (state->interface == PHY_INTERFACE_MODE_USXGMII)
		rtl9310_sds_mode_usxgmii(port_sds[port]);

	pr_info("%s: speed was %d\n", __func__, (reg >> 12) & 0xf); 
	reg &= ~(0xf << 12);
	reg |= RTL_SPEED_1000 << 12; /* Set SMI speed to 0x2 */

	reg |= RTL931X_TX_PAUSE_EN | RTL931X_RX_PAUSE_EN;

	if (priv->lagmembers & BIT_ULL(port))
		reg |= RTL931X_DUPLEX_MODE;

	if (state->duplex == DUPLEX_FULL)
		reg |= RTL931X_DUPLEX_MODE;

	sw_w32(reg, rtl931x_mac_force_mode_ctrl(port));

}

/* Reset the SerDes by powering it off, setting its mode to off (0x9f), setting the new mode
 * and turning it on again. Modes are:
 * 0x06: QSGMII		0x10: XSGMII		0x0d: USXGMII
 * 0x12: HISGMII	0x09: XSMII		0x02: SGMII
 * 0x1f: OFF
 */
static void rtl931x_sds_rst(int sds_num, phy_interface_t mode)
{
	u32 sds_ps, val;
//	int pos = sds_num % 4;

	int pos = ((sds_num & 0x3) << 3);

	switch (mode) {
	case PHY_INTERFACE_MODE_QSGMII:
		val = 0x6;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		val = 0x2;
		break;
	case PHY_INTERFACE_MODE_HSGMII:
		val = 0x12;
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		/* serdes mode FIBER1G */
		val = 0x9;
		break;

	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_10GKR:
		val = 0x35;
		break;
//	case MII_10GR1000BX_AUTO:
//		val = 0x39;
//		break;
	case PHY_INTERFACE_MODE_USXGMII:
		val = 0xd;
		break;
	default:
		val = 0x25;
	}

	// Switch Serdes Off
	sds_ps = sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	sw_w32(sds_ps | BIT(sds_num), RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	
	pr_info("%s: setdes mode was %X\n",__func__, RTL931X_SERDES_MODE_CTRL(sds_num));
	// Change SerDes mode to off and then set new mode
	sw_w32_mask(0xff << pos, 0x9f << pos, RTL931X_SERDES_MODE_CTRL(sds_num));
	msleep(10);
	// New mode, need to set BIT(7)
	val |= BIT(7);
	sw_w32_mask(0xff << pos, val << pos, RTL931X_SERDES_MODE_CTRL(sds_num));
	pr_info("%s: setdes mode is now %X\n",__func__, RTL931X_SERDES_MODE_CTRL(sds_num));

	// Return to inital power state
	sw_w32(sds_ps, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
}

int rtl931x_read_sds_phy(int phy_addr, int page, int phy_reg)
{
	int i;
	u32 cmd = phy_addr << 2 | page << 7 | phy_reg << 13 | 1;

	pr_debug("%s: phy_addr(SDS-ID) %d, phy_reg: %d\n", __func__, phy_addr, phy_reg);
	sw_w32(cmd, RTL931X_SERDES_INDRT_ACCESS_CTRL);

	for (i = 0; i < 100; i++) {
		if (!(sw_r32(RTL931X_SERDES_INDRT_ACCESS_CTRL) & 0x1))
			break;
		mdelay(1);
	}

	if (i >= 100)
		return -EIO;

	pr_debug("%s: returning %04x\n", __func__, sw_r32(RTL931X_SERDES_INDRT_DATA_CTRL) & 0xffff);

	return sw_r32(RTL931X_SERDES_INDRT_DATA_CTRL) & 0xffff;
}

int rtl931x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v)
{
	int i;
	u32 cmd;

	cmd = phy_addr << 2 | page << 7 | phy_reg << 13;
	sw_w32(cmd, RTL931X_SERDES_INDRT_ACCESS_CTRL);

	sw_w32(v, RTL931X_SERDES_INDRT_DATA_CTRL);

	cmd =  sw_r32(RTL931X_SERDES_INDRT_ACCESS_CTRL) | 0x3;
	sw_w32(cmd, RTL931X_SERDES_INDRT_ACCESS_CTRL);

	for (i = 0; i < 100; i++) {
		if (!(sw_r32(RTL931X_SERDES_INDRT_ACCESS_CTRL) & 0x1))
			break;
		mdelay(1);
	}

	if (i >= 100)
		return -EIO;

	return 0;
}

void rtl931x_sds_field_w(int sds, u32 page, u32 reg, int end_bit, int start_bit, u32 v)
{
	int l = end_bit - start_bit + 1;
	u32 data = v;

	if (l < 32) {
		u32 mask = BIT(l) - 1;

		data = rtl931x_read_sds_phy(sds, page, reg);
		data &= ~(mask << start_bit);
		data |= (v & mask) << start_bit;
	}

	rtl931x_write_sds_phy(sds, page, reg, data);
}

u32 rtl931x_sds_field_r(int sds, u32 page, u32 reg, int end_bit, int start_bit)
{
	int l = end_bit - start_bit + 1;
	u32 v = rtl931x_read_sds_phy(sds, page, reg);

	if (l >= 32)
		return v;

	return (v >> start_bit) & (BIT(l) - 1);
}

/*static void rtl931x_sds_rst(u32 sds)
{
	u32 o, v, o_mode;
	int shift = ((sds & 0x3) << 3);

	// TODO: We need to lock this!
	
	o = sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	v = o | BIT(sds);
	sw_w32(v, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);

	o_mode = sw_r32(RTL931X_SERDES_MODE_CTRL(sds));
	v = (1 << 7) | 0x1F;
	sw_w32_mask(0xff << shift, v << shift, RTL931X_SERDES_MODE_CTRL(sds));
	sw_w32(o_mode, RTL931X_SERDES_MODE_CTRL(sds));

	sw_w32(o, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
}*/

static void rtl931x_symerr_clear(u32 sds, phy_interface_t mode)
{

	switch (mode) {
	case PHY_INTERFACE_MODE_NA:
		break;
	case PHY_INTERFACE_MODE_XGMII:
		u32 xsg_sdsid_0, xsg_sdsid_1;

		if (sds < 2)
			xsg_sdsid_0 = sds;
		else
			xsg_sdsid_0 = (sds - 1) * 2;
		xsg_sdsid_1 = xsg_sdsid_0 + 1;

		for (int i = 0; i < 4; ++i) {
			rtl931x_sds_field_w(xsg_sdsid_0, 0x1, 24,  2, 0, i);
			rtl931x_sds_field_w(xsg_sdsid_0, 0x1,  3, 15, 8, 0x0);
			rtl931x_sds_field_w(xsg_sdsid_0, 0x1,  2, 15, 0, 0x0);
		}

		for (int i = 0; i < 4; ++i) {
			rtl931x_sds_field_w(xsg_sdsid_1, 0x1, 24,  2, 0, i);
			rtl931x_sds_field_w(xsg_sdsid_1, 0x1,  3, 15, 8, 0x0);
			rtl931x_sds_field_w(xsg_sdsid_1, 0x1,  2, 15, 0, 0x0);
		}

		rtl931x_sds_field_w(xsg_sdsid_0, 0x1, 0, 15, 0, 0x0);
		rtl931x_sds_field_w(xsg_sdsid_0, 0x1, 1, 15, 8, 0x0);
		rtl931x_sds_field_w(xsg_sdsid_1, 0x1, 0, 15, 0, 0x0);
		rtl931x_sds_field_w(xsg_sdsid_1, 0x1, 1, 15, 8, 0x0);
		break;
	default:
		break;
	}

	return;
}
#if 0
static void rtl931x_sds_fiber_disable(u32 sds)
{
	u32 v = 0x3F;
	u32 asds = rtl931x_get_analog_sds(sds);

	rtl931x_sds_field_w(asds, 0x1F, 0x9, 11, 6, v);
}
#endif
static void rtl931x_sds_fiber_mode_set(u32 sds, phy_interface_t mode)
{
	int pos;
	u32 val, asds = rtl931x_get_analog_sds(sds);

	/* clear symbol error count before changing mode */
	rtl931x_symerr_clear(sds, mode);

	pos = ((sds & 0x3) << 3);
	val = 0x9F;
	sw_w32(val, RTL931X_SERDES_MODE_CTRL(sds));

	switch (mode) {
	case PHY_INTERFACE_MODE_QSGMII:
		val = 0x6;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		val = 0x5;
		break;
	case PHY_INTERFACE_MODE_HSGMII:
		val = 0x25;
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		/* serdes mode FIBER1G */
		val = 0x9;
		break;
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_10GKR:
		val = 0x35;
		break;
//	case MII_10GR1000BX_AUTO:
//		val = 0x39;
//		break;
	case PHY_INTERFACE_MODE_USXGMII:
		val = 0x1b;
		break;
	default:
		val = 0x25;
	}

	rtl931x_sds_field_w(asds, 0x1F, 0x9, 11, 6, val);

	return;
}

static int rtl931x_sds_cmu_page_get(phy_interface_t mode)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_1000BASEX:	/* MII_1000BX_FIBER / 100BX_FIBER / 1000BX100BX_AUTO */
		return 0x24;
	case PHY_INTERFACE_MODE_HSGMII:
	case PHY_INTERFACE_MODE_2500BASEX:	/* MII_2500Base_X: */
		return 0x28;
/*	case MII_HISGMII_5G: */
/*		return 0x2a; */
	case PHY_INTERFACE_MODE_QSGMII:
		return 0x34;			/* Code also has 0x34 */
	case PHY_INTERFACE_MODE_XAUI:		/* MII_RXAUI_LITE: */
		return 0x2c;
	case PHY_INTERFACE_MODE_XGMII:		/* MII_XSGMII */
	case PHY_INTERFACE_MODE_10GKR:
	case PHY_INTERFACE_MODE_10GBASER:	/* MII_10GR */
		return 0x2e;
	default:
		return -1;
	}

	return -1;
}

static void rtl931x_cmu_type_set(u32 asds, phy_interface_t mode, int chiptype)
{
	int cmu_type = 0; /* Clock Management Unit */
	u32 cmu_page = 0;
	u32 frc_cmu_spd;
	u32 evenSds;
	u32 lane, frc_lc_mode_bitnum, frc_lc_mode_val_bitnum;

	switch (mode) {
	case PHY_INTERFACE_MODE_NA:
	case PHY_INTERFACE_MODE_10GKR:
	case PHY_INTERFACE_MODE_XGMII:
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_USXGMII:
		return;

/*	case MII_10GR1000BX_AUTO:
		if (chiptype)
			rtl931x_sds_field_w(asds, 0x24, 0xd, 14, 14, 0);
		return; */

	case PHY_INTERFACE_MODE_QSGMII:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break;

	case PHY_INTERFACE_MODE_HSGMII:
		cmu_type = 1;
		frc_cmu_spd = 1;
		break;

	case PHY_INTERFACE_MODE_1000BASEX:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break;

/*	case MII_1000BX100BX_AUTO:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break; */

	case PHY_INTERFACE_MODE_SGMII:
		cmu_type = 1;
		frc_cmu_spd = 0;
		break;

	case PHY_INTERFACE_MODE_2500BASEX:
		cmu_type = 1;
		frc_cmu_spd = 1;
		break;

	default:
		pr_warn("%s: SerDes %d mode is invalid\n", __func__, asds);
		return;
	}

	if (cmu_type == 1)
		cmu_page = rtl931x_sds_cmu_page_get(mode);

	lane = asds % 2;

	if (!lane) {
		frc_lc_mode_bitnum = 4;
		frc_lc_mode_val_bitnum = 5;
	} else {
		frc_lc_mode_bitnum = 6;
		frc_lc_mode_val_bitnum = 7;
	}

	evenSds = asds - lane;

	pr_debug("%s: cmu_type %0d cmu_page %x frc_cmu_spd %d lane %d asds %d\n",
	        __func__, cmu_type, cmu_page, frc_cmu_spd, lane, asds);

	if (cmu_type == 1) {
		pr_debug("%s A CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
		rtl931x_sds_field_w(asds, cmu_page, 0x7, 15, 15, 0);
		pr_debug("%s B CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
		if (chiptype) {
			rtl931x_sds_field_w(asds, cmu_page, 0xd, 14, 14, 0);
		}

		rtl931x_sds_field_w(evenSds, 0x20, 0x12, 3, 2, 0x3);
		rtl931x_sds_field_w(evenSds, 0x20, 0x12, frc_lc_mode_bitnum, frc_lc_mode_bitnum, 1);
		rtl931x_sds_field_w(evenSds, 0x20, 0x12, frc_lc_mode_val_bitnum, frc_lc_mode_val_bitnum, 0);
		rtl931x_sds_field_w(evenSds, 0x20, 0x12, 12, 12, 1);
		rtl931x_sds_field_w(evenSds, 0x20, 0x12, 15, 13, frc_cmu_spd);
	}

	pr_debug("%s CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
	return;
}

static void rtl931x_sds_rx_rst(u32 sds)
{
	u32 asds = rtl931x_get_analog_sds(sds);

	if (sds < 2)
		return;

	rtl931x_write_sds_phy(asds, 0x2e, 0x12, 0x2740);
	rtl931x_write_sds_phy(asds, 0x2f, 0x0, 0x0);
	rtl931x_write_sds_phy(asds, 0x2f, 0x2, 0x2010);
	rtl931x_write_sds_phy(asds, 0x20, 0x0, 0xc10);

	rtl931x_write_sds_phy(asds, 0x2e, 0x12, 0x27c0);
	rtl931x_write_sds_phy(asds, 0x2f, 0x0, 0xc000);
	rtl931x_write_sds_phy(asds, 0x2f, 0x2, 0x6010);
	rtl931x_write_sds_phy(asds, 0x20, 0x0, 0xc30);

	mdelay(50);
}

// Currently not used
// static void rtl931x_sds_disable(u32 sds)
// {
// 	u32 v = 0x1f;

// 	v |= BIT(7);
// 	sw_w32(v, RTL931X_SERDES_MODE_CTRL + (sds >> 2) * 4);
// }

static void rtl931x_sds_mii_mode_set(u32 sds, phy_interface_t mode)
{
	u32 val;

	int shift = ((sds & 0x3) << 3);

	// TODO: We need to lock this!
	
/*	o = sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	v = o | BIT(sds);
	sw_w32(v, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);

	o_mode = sw_r32(RTL931X_SERDES_MODE_CTRL(sds));
	v = (1 << 7) | 0x1F;
	sw_w32_mask(0xff << shift, v << shift, RTL931X_SERDES_MODE_CTRL(sds));*/

	switch (mode) {
	case PHY_INTERFACE_MODE_QSGMII:
		val = 0x6;
		break;
	case PHY_INTERFACE_MODE_XGMII:
		val = 0x10; /* serdes mode XSGMII */
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		val = 0xD;
		break;
	case PHY_INTERFACE_MODE_HSGMII:
		val = 0x12;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		val = 0x2;
		break;
	default:
		return;
	}

	val |= (1 << 7);

	sw_w32_mask(0xff << shift, val << shift, RTL931X_SERDES_MODE_CTRL(sds));
}

static sds_config sds_config_10p3125g_type1[] = {
	{ 0x2E, 0x00, 0x0107 }, { 0x2E, 0x01, 0x01A3 }, { 0x2E, 0x02, 0x6A24 },
	{ 0x2E, 0x03, 0xD10D }, { 0x2E, 0x04, 0x8000 }, { 0x2E, 0x05, 0xA17E },
	{ 0x2E, 0x06, 0xE31D }, { 0x2E, 0x07, 0x800E }, { 0x2E, 0x08, 0x0294 },
	{ 0x2E, 0x09, 0x0CE4 }, { 0x2E, 0x0A, 0x7FC8 }, { 0x2E, 0x0B, 0xE0E7 },
	{ 0x2E, 0x0C, 0x0200 }, { 0x2E, 0x0D, 0xDF80 }, { 0x2E, 0x0E, 0x0000 },
	{ 0x2E, 0x0F, 0x1FC2 }, { 0x2E, 0x10, 0x0C3F }, { 0x2E, 0x11, 0x0000 },
	{ 0x2E, 0x12, 0x27C0 }, { 0x2E, 0x13, 0x7E1D }, { 0x2E, 0x14, 0x1300 },
	{ 0x2E, 0x15, 0x003F }, { 0x2E, 0x16, 0xBE7F }, { 0x2E, 0x17, 0x0090 },
	{ 0x2E, 0x18, 0x0000 }, { 0x2E, 0x19, 0x4000 }, { 0x2E, 0x1A, 0x0000 },
	{ 0x2E, 0x1B, 0x8000 }, { 0x2E, 0x1C, 0x011F }, { 0x2E, 0x1D, 0x0000 },
	{ 0x2E, 0x1E, 0xC8FF }, { 0x2E, 0x1F, 0x0000 }, { 0x2F, 0x00, 0xC000 },
	{ 0x2F, 0x01, 0xF000 }, { 0x2F, 0x02, 0x6010 }, { 0x2F, 0x12, 0x0EE7 },
	{ 0x2F, 0x13, 0x0000 }
};

static sds_config sds_config_10p3125g_cmu_type1[] = {
	{ 0x2F, 0x03, 0x4210 }, { 0x2F, 0x04, 0x0000 }, { 0x2F, 0x05, 0x0019 },
	{ 0x2F, 0x06, 0x18A6 }, { 0x2F, 0x07, 0x2990 }, { 0x2F, 0x08, 0xFFF4 },
	{ 0x2F, 0x09, 0x1F08 }, { 0x2F, 0x0A, 0x0000 }, { 0x2F, 0x0B, 0x8000 },
	{ 0x2F, 0x0C, 0x4224 }, { 0x2F, 0x0D, 0x0000 }, { 0x2F, 0x0E, 0x0000 },
	{ 0x2F, 0x0F, 0xA470 }, { 0x2F, 0x10, 0x8000 }, { 0x2F, 0x11, 0x037B }
};

void rtl931x_sds_init(u32 sds, u32 port, phy_interface_t mode)
{
	u32 board_sds_tx_type1[] = {
		0x01c3, 0x01c3, 0x01c3, 0x01a3, 0x01a3, 0x01a3,
		0x0143, 0x0143, 0x0143, 0x0143, 0x0163, 0x0163,
	};
	u32 board_sds_tx[] = {
		0x1a00, 0x1a00, 0x0200, 0x0200, 0x0200, 0x0200,
		0x01a3, 0x01a3, 0x01a3, 0x01a3, 0x01e3, 0x01e3
	};
	u32 board_sds_tx2[] = {
		0x0dc0, 0x01c0, 0x0200, 0x0180, 0x0160, 0x0123,
		0x0123, 0x0163, 0x01a3, 0x01a0, 0x01c3, 0x09c3,
	};
	u32 asds, dSds, ori, model_info, val;
	int chiptype = 0;

	asds = rtl931x_get_analog_sds(sds);

	if (sds > 13)
		return;

	pr_debug("%s: set sds %d to mode %d\n", __func__, sds, mode);
	val = rtl931x_sds_field_r(asds, 0x1F, 0x9, 11, 6);

	pr_debug("%s: fibermode %08X stored mode 0x%x analog SDS %d", __func__,
			rtl931x_read_sds_phy(asds, 0x1f, 0x9), val, asds);
	pr_debug("%s: SGMII mode %08X in 0x24 0x9 analog SDS %d", __func__,
			rtl931x_read_sds_phy(asds, 0x24, 0x9), asds);
	pr_debug("%s: CMU mode %08X stored even SDS %d", __func__,
			rtl931x_read_sds_phy(asds & ~1, 0x20, 0x12), asds & ~1);
	pr_debug("%s: serdes_mode_ctrl %08X", __func__,  RTL931X_SERDES_MODE_CTRL(sds));
	pr_debug("%s CMU page 0x24 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x24, 0x7));
	pr_debug("%s CMU page 0x26 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x26, 0x7));
	pr_debug("%s CMU page 0x28 0x7 %08x\n", __func__, rtl931x_read_sds_phy(asds, 0x28, 0x7));
	pr_debug("%s XSG page 0x0 0xe %08x\n", __func__, rtl931x_read_sds_phy(dSds, 0x0, 0xe));
	pr_debug("%s XSG2 page 0x0 0xe %08x\n", __func__, rtl931x_read_sds_phy(dSds + 1, 0x0, 0xe));

	model_info = sw_r32(RTL93XX_MODEL_NAME_INFO);
	if ((model_info >> 4) & 0x1) {
		chiptype = 1;
	}

	if (sds < 2)
		dSds = sds;
	else
		dSds = (sds - 1) * 2;

	pr_debug("%s: 2.5gbit %08X dsds %d", __func__,
	        rtl931x_read_sds_phy(dSds, 0x1, 0x14), dSds);

	pr_debug("%s: RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR 0x%08X\n", __func__, sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR));
	ori = sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	val = ori | (1 << sds);
	sw_w32(val, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);

	switch (mode) {
	case PHY_INTERFACE_MODE_NA:
		break;

	case PHY_INTERFACE_MODE_XGMII: /* MII_XSGMII */

		if (chiptype) {
			u32 xsg_sdsid_1;
			xsg_sdsid_1 = dSds + 1;
			/* fifo inv clk */
			rtl931x_sds_field_w(dSds, 0x1, 0x1, 7, 4, 0xf);
			rtl931x_sds_field_w(dSds, 0x1, 0x1, 3, 0, 0xf);

			rtl931x_sds_field_w(xsg_sdsid_1, 0x1, 0x1, 7, 4, 0xf);
			rtl931x_sds_field_w(xsg_sdsid_1, 0x1, 0x1, 3, 0, 0xf);

		}

		rtl931x_sds_field_w(dSds, 0x0, 0xE, 12, 12, 1);
		rtl931x_sds_field_w(dSds + 1, 0x0, 0xE, 12, 12, 1);
		break;

	case PHY_INTERFACE_MODE_USXGMII: /* MII_USXGMII_10GSXGMII/10GDXGMII/10GQXGMII: */
		u32 op_code = 0x6003;
		u32 evenSds;

		if (chiptype) {
			rtl931x_sds_field_w(asds, 0x6, 0x2, 12, 12, 1);

			for (int i = 0; i < sizeof(sds_config_10p3125g_type1) / sizeof(sds_config); ++i) {
				rtl931x_write_sds_phy(asds, sds_config_10p3125g_type1[i].page - 0x4, sds_config_10p3125g_type1[i].reg, sds_config_10p3125g_type1[i].data);
			}

			evenSds = asds - (asds % 2);

			for (int i = 0; i < sizeof(sds_config_10p3125g_cmu_type1) / sizeof(sds_config); ++i) {
				rtl931x_write_sds_phy(evenSds,
				                      sds_config_10p3125g_cmu_type1[i].page - 0x4, sds_config_10p3125g_cmu_type1[i].reg, sds_config_10p3125g_cmu_type1[i].data);
			}

			rtl931x_sds_field_w(asds, 0x6, 0x2, 12, 12, 0);
		} else {

			rtl931x_sds_field_w(asds, 0x2e, 0xd, 6, 0, 0x0);
			rtl931x_sds_field_w(asds, 0x2e, 0xd, 7, 7, 0x1);

			rtl931x_sds_field_w(asds, 0x2e, 0x1c, 5, 0, 0x1E);
			rtl931x_sds_field_w(asds, 0x2e, 0x1d, 11, 0, 0x00);
			rtl931x_sds_field_w(asds, 0x2e, 0x1f, 11, 0, 0x00);
			rtl931x_sds_field_w(asds, 0x2f, 0x0, 11, 0, 0x00);
			rtl931x_sds_field_w(asds, 0x2f, 0x1, 11, 0, 0x00);

			rtl931x_sds_field_w(asds, 0x2e, 0xf, 12, 6, 0x7F);
			rtl931x_write_sds_phy(asds, 0x2f, 0x12, 0xaaa);

			rtl931x_sds_rx_rst(sds);

			rtl931x_write_sds_phy(asds, 0x7, 0x10, op_code);
			rtl931x_write_sds_phy(asds, 0x6, 0x1d, 0x0480);
			rtl931x_write_sds_phy(asds, 0x6, 0xe, 0x0400);
		}
		break;

	case PHY_INTERFACE_MODE_10GBASER: /* MII_10GR / MII_10GR1000BX_AUTO: */
	                                  /* configure 10GR fiber mode=1 */
	        /* init 10gr */
		rtl931x_sds_field_w(asds, 0x1f, 0xb, 1, 1, 1);

		/* init fiber_1g */
		rtl931x_sds_field_w(dSds, 0x3, 0x13, 15, 14, 0);

		rtl931x_sds_field_w(dSds, 0x2, 0x0, 12, 12, 1);
		rtl931x_sds_field_w(dSds, 0x2, 0x0, 6, 6, 1);
		rtl931x_sds_field_w(dSds, 0x2, 0x0, 13, 13, 0);

		/* init auto */
		rtl931x_sds_field_w(asds, 0x1f, 13, 15, 0, 0x109e);
		rtl931x_sds_field_w(asds, 0x1f, 0x6, 14, 10, 0x8);
		rtl931x_sds_field_w(asds, 0x1f, 0x7, 10, 4, 0x7f);

		/* sds rx config */
		rtl931x_write_sds_phy(asds, 0x2e, 0x12, 0x27c0);
		rtl931x_write_sds_phy(asds, 0x2f, 0x0, 0xc000);
		rtl931x_write_sds_phy(asds, 0x2f, 0x2, 0x6010);


		rtl931x_write_sds_phy(asds, 0x20, 0x0, 0xc30);		
		rtl931x_sds_field_w(asds, 0x20, 0x0, 9, 0, 0x30);
		rtl931x_sds_field_w(asds, 0x2A, 0x12, 7, 6, 0x3);
		rtl931x_sds_field_w(asds, 0x20, 0x0, 11, 10, 0x1);
		rtl931x_sds_field_w(asds, 0x20, 0x0, 11, 10, 0x3);
		
		//tx polarity change
//		rtl931x_sds_field_w(asds, 0x6, 0x2, 14, 14, 1);


		rtl931x_sds_rx_rst(sds);

		break;

	case PHY_INTERFACE_MODE_HSGMII:
		rtl931x_sds_field_w(dSds, 0x1, 0x14, 8, 8, 1);
		break;

	case PHY_INTERFACE_MODE_1000BASEX: /* MII_1000BX_FIBER */
		rtl931x_sds_field_w(dSds, 0x3, 0x13, 15, 14, 0);

		rtl931x_sds_field_w(dSds, 0x2, 0x0, 12, 12, 1);
		rtl931x_sds_field_w(dSds, 0x2, 0x0, 6, 6, 1);
		rtl931x_sds_field_w(dSds, 0x2, 0x0, 13, 13, 0);

		rtl931x_write_sds_phy(asds, 0x20, 0x0, 0xc30);		
		rtl931x_sds_field_w(asds, 0x20, 0x0, 9, 0, 0x30);
		rtl931x_sds_field_w(asds, 0x2A, 0x12, 7, 6, 0x3);
		rtl931x_sds_field_w(asds, 0x20, 0x0, 11, 10, 0x1);
		rtl931x_sds_field_w(asds, 0x20, 0x0, 11, 10, 0x3);

		break;

	case PHY_INTERFACE_MODE_SGMII:
		rtl931x_sds_field_w(asds, 0x24, 0x9, 15, 15, 0);
		break;

	case PHY_INTERFACE_MODE_2500BASEX:
		rtl931x_sds_field_w(dSds, 0x1, 0x14, 8, 8, 1);
		break;

	case PHY_INTERFACE_MODE_QSGMII:
	default:
		pr_warn("%s: PHY mode %s not supported by SerDes %d\n",
		        __func__, phy_modes(mode), sds);
		return;
	}

	rtl931x_cmu_type_set(asds, mode, chiptype);

	if (sds >= 2 && sds <= 13) {
		if (chiptype)
			rtl931x_write_sds_phy(asds, 0x2E, 0x1, board_sds_tx_type1[sds - 2]);
		else {
			val = 0xa0000;
			sw_w32(val, RTL931X_CHIP_INFO_ADDR);
			val = sw_r32(RTL931X_CHIP_INFO_ADDR);
			if (val & BIT(28)) /* consider 9311 etc. RTL9313_CHIP_ID == HWP_CHIP_ID(unit)) */
			{
				rtl931x_write_sds_phy(asds, 0x2E, 0x1, board_sds_tx2[sds - 2]);
			} else {
				rtl931x_write_sds_phy(asds, 0x2E, 0x1, board_sds_tx[sds - 2]);
			}
			val = 0;
			sw_w32(val, RTL931X_CHIP_INFO_ADDR);
		}
	}

	val = ori & ~BIT(sds);
	sw_w32(val, RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR);
	pr_debug("%s: RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR 0x%08X\n", __func__, sw_r32(RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR));
	if (mode == PHY_INTERFACE_MODE_HSGMII)
		rtl931x_sds_field_w(asds, 0x2a, 7, 15, 15, 0);
	
/*	if (mode == PHY_INTERFACE_MODE_XGMII ||
	    mode == PHY_INTERFACE_MODE_QSGMII ||
	    mode == PHY_INTERFACE_MODE_HSGMII ||
	    mode == PHY_INTERFACE_MODE_SGMII ||
	    mode == PHY_INTERFACE_MODE_10GBASER ||
	    mode == PHY_INTERFACE_MODE_USXGMII)*/ {
		if (mode == PHY_INTERFACE_MODE_XGMII)
			rtl931x_sds_mii_mode_set(sds, mode);
		else
			rtl931x_sds_fiber_mode_set(sds, mode);
	}

/*
	//check
        osal_time_mdelay(10);
        SDS_FIELD_W(unit, aSds, 0x20, 0x0, 11, 10, 0x1);
        SDS_FIELD_W(unit, aSds, 0x20, 0x0, 11, 10, 0x3);*/
}

int rtl931x_sds_cmu_band_set(int sds, bool enable, u32 band, phy_interface_t mode)
{
	u32 asds;
	int page = rtl931x_sds_cmu_page_get(mode);

	sds -= (sds % 2);
	sds = sds & ~1;
	asds = rtl931x_get_analog_sds(sds);
	page += 1;

	if (enable) {
		rtl931x_sds_field_w(asds, page, 0x7, 13, 13, 0);
		rtl931x_sds_field_w(asds, page, 0x7, 11, 11, 0);
	} else {
		rtl931x_sds_field_w(asds, page, 0x7, 13, 13, 0);
		rtl931x_sds_field_w(asds, page, 0x7, 11, 11, 0);
	}

	rtl931x_sds_field_w(asds, page, 0x7, 4, 0, band);

	rtl931x_sds_rst(sds, mode);

	return 0;
}

int rtl931x_sds_cmu_band_get(int sds, phy_interface_t mode)
{
	int page = rtl931x_sds_cmu_page_get(mode);
	u32 asds, band;

	sds -= (sds % 2);
	asds = rtl931x_get_analog_sds(sds);
	page += 1;
	rtl931x_write_sds_phy(asds, 0x1f, 0x02, 73);

	rtl931x_sds_field_w(asds, page, 0x5, 15, 15, 1);
	band = rtl931x_sds_field_r(asds, 0x1f, 0x15, 8, 3);

	return band;
}

static void rtl931x_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s port %d\n", __func__, port);
	mutex_lock(&priv->reg_mutex);
	sw_w32(port << 11, RTL931X_L2_TBL_FLUSH_CTRL + 4);

	sw_w32(BIT(24) | BIT(28), RTL931X_L2_TBL_FLUSH_CTRL);

	do { } while (sw_r32(RTL931X_L2_TBL_FLUSH_CTRL) & BIT (28));

	mutex_unlock(&priv->reg_mutex);
}

static void rtl931x_phylink_mac_link_up(struct dsa_switch *ds, int port,
				   unsigned int mode,
				   phy_interface_t interface,
				   struct phy_device *phydev,
				   int speed, int duplex,
				   bool tx_pause, bool rx_pause)
{
	u32 spdsel;

	if (speed == SPEED_10000)
		spdsel = RTL_SPEED_10000;
	else if (speed == SPEED_5000)
		spdsel = RTL_SPEED_5000;
	else if (speed == SPEED_2500)
		spdsel = RTL_SPEED_2500;
	else if (speed == SPEED_1000)
		spdsel = RTL_SPEED_1000;
	else if (speed == SPEED_100)
		spdsel = RTL_SPEED_100;
	else
		spdsel = RTL_SPEED_10;

	/* Restart TX/RX to port */
	sw_w32_mask(0, 0x3, rtl931x_mac_port_ctrl(port));

}

static void rtl931x_phylink_mac_link_down(struct dsa_switch *ds, int port,
				     unsigned int mode,
				     phy_interface_t interface)
{
	/* Stop TX/RX to port */
	sw_w32_mask(0x3, 0, rtl931x_mac_port_ctrl(port));
}

const struct rtl838x_reg rtl931x_reg = {
	.phylink_mac_link_down = rtl931x_phylink_mac_link_down,
	.phylink_mac_link_up = rtl931x_phylink_mac_link_up,
	.phylink_mac_config = rtl931x_phylink_mac_config,
	.mask_port_reg_be = rtl839x_mask_port_reg_be,
	.set_port_reg_be = rtl839x_set_port_reg_be,
	.get_port_reg_be = rtl839x_get_port_reg_be,
	.mask_port_reg_le = rtl839x_mask_port_reg_le,
	.set_port_reg_le = rtl839x_set_port_reg_le,
	.get_port_reg_le = rtl839x_get_port_reg_le,
	.stat_port_rst = RTL931X_STAT_PORT_RST,
	.stat_rst = RTL931X_STAT_RST,
	.stat_port_std_mib = 0,  /* Not defined */
	.traffic_enable = rtl931x_traffic_enable,
	.traffic_disable = rtl931x_traffic_disable,
	.traffic_get = rtl931x_traffic_get,
	.traffic_set = rtl931x_traffic_set,
	.l2_ctrl_0 = RTL931X_L2_CTRL,
	.l2_ctrl_1 = RTL931X_L2_AGE_CTRL,
	.l2_port_aging_out = RTL931X_L2_PORT_AGE_CTRL,
	.set_ageing_time = rtl931x_set_ageing_time,
	.smi_poll_ctrl = RTL931X_SMI_PORT_POLLING_CTRL,
	.l2_tbl_flush_ctrl = RTL931X_L2_TBL_FLUSH_CTRL,
	.exec_tbl0_cmd = rtl931x_exec_tbl0_cmd,
	.exec_tbl1_cmd = rtl931x_exec_tbl1_cmd,
	.tbl_access_data_0 = rtl931x_tbl_access_data_0,
	.isr_glb_src = RTL931X_ISR_GLB_SRC,
	.isr_port_link_sts_chg = RTL931X_ISR_PORT_LINK_STS_CHG,
	.imr_port_link_sts_chg = RTL931X_IMR_PORT_LINK_STS_CHG,
	/* imr_glb does not exist on RTL931X */
	.vlan_tables_read = rtl931x_vlan_tables_read,
	.vlan_set_tagged = rtl931x_vlan_set_tagged,
	.vlan_set_untagged = rtl931x_vlan_set_untagged,
	.vlan_profile_dump = rtl931x_vlan_profile_dump,
	.vlan_profile_setup = rtl931x_vlan_profile_setup,
	.vlan_fwd_on_inner = rtl931x_vlan_fwd_on_inner,
	.stp_get = rtl931x_stp_get,
	.stp_set = rtl931x_stp_set,
	.l2_port_new_salrn = rtl931x_l2_port_new_salrn,
	.l2_port_new_sa_fwd = rtl931x_l2_port_new_sa_fwd,
	.mir_ctrl = RTL931X_MIR_CTRL,
	.mir_dpm = RTL931X_MIR_DPM_CTRL,
	.mir_spm = RTL931X_MIR_SPM_CTRL,
	.mac_link_sts = RTL931X_MAC_LINK_STS,
	.mac_link_dup_sts = RTL931X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts = rtl931x_mac_link_spd_sts,
	.mac_rx_pause_sts = RTL931X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts = RTL931X_MAC_TX_PAUSE_STS,
	.read_l2_entry_using_hash = rtl931x_read_l2_entry_using_hash,
	.write_l2_entry_using_hash = rtl931x_write_l2_entry_using_hash,
	.read_cam = rtl931x_read_cam,
	.write_cam = rtl931x_write_cam,
	.init_eee = rtl931x_init_eee,
	.set_mac_eee = rtl931x_port_eee_set,
#if 0
	.eee_port_ability = rtl931x_eee_port_ability,
#endif
	.vlan_port_keep_tag_set = rtl931x_vlan_port_keep_tag_set,
	.vlan_port_pvidmode_set = rtl931x_vlan_port_pvidmode_set,
	.vlan_port_pvid_set = rtl931x_vlan_port_pvid_set,
	.trk_mbr_ctr = rtl931x_trk_mbr_ctr,
	.set_vlan_igr_filter = rtl931x_set_igr_filter,
	.set_vlan_egr_filter = rtl931x_set_egr_filter,
	.set_distribution_algorithm = rtl931x_set_distribution_algorithm,
	.l2_hash_key = rtl931x_l2_hash_key,
	.l2_hash_seed = rtl931x_l2_hash_seed,
	.read_mcast_pmask = rtl931x_read_mcast_pmask,
	.write_mcast_pmask = rtl931x_write_mcast_pmask,
	.pie_init = rtl931x_pie_init,
	.pie_rule_write = rtl931x_pie_rule_write,
	.pie_rule_add = rtl931x_pie_rule_add,
	.pie_rule_rm = rtl931x_pie_rule_rm,
	.l2_learning_setup = rtl931x_l2_learning_setup,
	.l3_setup = rtl931x_l3_setup,
	.led_init = rtl931x_led_init,
	.set_l3_nexthop = rtl931x_set_l3_nexthop,
	.get_l3_nexthop = rtl931x_get_l3_nexthop,
	.get_l3_egress_mac = rtl931x_get_l3_egress_mac,
	.set_l3_egress_mac = rtl931x_set_l3_egress_mac,
	.route_read = rtl931x_route_read,
	.route_write = rtl931x_route_write,
//	.host_route_write = rtl931x_host_route_write,
	.find_l3_slot = rtl931x_find_l3_slot,
	.route_lookup_hw = rtl931x_route_lookup_hw,
	.get_l3_router_mac = rtl931x_get_l3_router_mac,
	.set_l3_router_mac = rtl931x_set_l3_router_mac,
	.set_l3_egress_intf = rtl931x_set_l3_egress_intf,
	.fast_age = rtl931x_fast_age,
	.set_receive_management_action = rtl931x_set_receive_management_action,
};
