#ifndef _RAETH_IOCTL_H
#define _RAETH_IOCTL_H

/* ioctl commands */
#define RAETH_ESW_REG_READ		0x89F1
#define RAETH_ESW_REG_WRITE		0x89F2
#define RAETH_MII_READ			0x89F3
#define RAETH_MII_WRITE			0x89F4

/* rt3052 embedded ethernet switch registers */
#define REG_ESW_VLAN_ID_BASE		0x50
#define REG_ESW_VLAN_MEMB_BASE		0x70
#define REG_ESW_TABLE_SEARCH		0x24
#define REG_ESW_TABLE_STATUS0		0x28
#define REG_ESW_TABLE_STATUS1		0x2C
#define REG_ESW_TABLE_STATUS2		0x30
#define REG_ESW_WT_MAC_AD0		0x34
#define REG_ESW_WT_MAC_AD1		0x38
#define REG_ESW_WT_MAC_AD2		0x3C

#define REG_ESW_MAX			0xFC

typedef struct rt3052_esw_reg {
	unsigned int off;
	unsigned int val;
} esw_reg;

typedef struct ralink_mii_ioctl_data {
	__u32 phy_id;
	__u32 reg_num;
	__u32 val_in;
	__u32 val_out;
} ra_mii_ioctl_data;

#endif
