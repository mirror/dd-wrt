/*
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#ifndef NPE_DEVICE_H
#define NPE_DEVICE_H

#include <linux/miscdevice.h>
#include <asm/hardware.h>

#ifdef __ARMEB__
#undef CONFIG_NPE_ADDRESS_COHERENT
#else
#define CONFIG_NPE_ADDRESS_COHERENT
#endif

#if defined(__ARMEB__) || defined (CONFIG_NPE_ADDRESS_COHERENT)
#define npe_to_cpu32(x) (x)
#define npe_to_cpu16(x) (x)
#define cpu_to_npe32(x) (x)
#define cpu_to_npe16(x) (x)
#else
#error NPE_DATA_COHERENT
#define NPE_DATA_COHERENT
#define npe_to_cpu32(x) be32_to_cpu(x)
#define npe_to_cpu16(x) be16_to_cpu(x)
#define cpu_to_npe32(x) cpu_to_be32(x)
#define cpu_to_npe16(x) cpu_to_be16(x)
#endif


struct npe_info {
	struct resource *res;
	void __iomem *addr;
	struct npe_plat_data *plat;
	u8 img_info[4];
	int usage;
	int loaded;
	u32 exec_count;
	u32 ctx_reg2;
};


static inline void npe_reg_write(struct npe_info *npe, u32 reg, u32 val)
{
	*(volatile u32*)((u8*)(npe->addr) + reg) = val;
}

static inline u32 npe_reg_read(struct npe_info *npe, u32 reg)
{
	return *(volatile u32*)((u8*)(npe->addr) + reg);
}

static inline u32 npe_status(struct npe_info *npe)
{
	return npe_reg_read(npe, IX_NPEDL_REG_OFFSET_EXCTL);
}

/* ixNpeDlNpeMgrCommandIssue */
static inline void npe_write_exctl(struct npe_info *npe, u32 cmd)
{
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXCTL, cmd);
}
/* ixNpeDlNpeMgrWriteCommandIssue */
static inline void
npe_write_cmd(struct npe_info *npe, u32 addr, u32 data, int cmd)
{
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXDATA, data);
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXAD, addr);
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXCTL, cmd);
}
/* ixNpeDlNpeMgrReadCommandIssue */
static inline u32
npe_read_cmd(struct npe_info *npe, u32 addr, int cmd)
{
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXAD, addr);
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_EXCTL, cmd);
	/* Intel reads the data twice - so do we... */
	npe_reg_read(npe, IX_NPEDL_REG_OFFSET_EXDATA);
	return npe_reg_read(npe, IX_NPEDL_REG_OFFSET_EXDATA);
}

/* ixNpeDlNpeMgrExecAccRegWrite */
static inline void npe_write_ecs_reg(struct npe_info *npe, u32 addr, u32 data)
{
	npe_write_cmd(npe, addr, data, IX_NPEDL_EXCTL_CMD_WR_ECS_REG);
}
/* ixNpeDlNpeMgrExecAccRegRead */
static inline u32 npe_read_ecs_reg(struct npe_info *npe, u32 addr)
{
	return npe_read_cmd(npe, addr, IX_NPEDL_EXCTL_CMD_RD_ECS_REG);
}

extern void npe_stop(struct npe_info *npe);
extern void npe_start(struct npe_info *npe);
extern void npe_reset(struct npe_info *npe);

extern struct device *get_npe_by_id(int id);
extern void return_npe_dev(struct device *dev);

/* NPE Messages */
extern int
npe_mh_status(struct npe_info *npe);
extern int
npe_mh_setportaddr(struct npe_info *npe, struct mac_plat_info *mp, u8 *macaddr);
extern int
npe_mh_disable_firewall(struct npe_info *npe, struct mac_plat_info *mp);
extern int
npe_mh_set_rxqid(struct npe_info *npe, struct mac_plat_info *mp, int qid);
extern int
npe_mh_npe_loopback_mode(struct npe_info *npe, struct mac_plat_info *mp, int enable);
extern int
npe_mh_get_stats(struct npe_info *npe, struct mac_plat_info *mp, u32 phys, int reset);
static inline u32 ix_fuse(void)
{
	unsigned int fuses = ~(*IXP4XX_EXP_CFG2);
	if (!cpu_is_ixp46x())
		fuses &= ~IX_FUSE_IXP46X_ONLY;

	return fuses;
}

#endif
