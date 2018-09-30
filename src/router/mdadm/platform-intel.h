/*
 * Intel(R) Matrix Storage Manager hardware and firmware support routines
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <asm/types.h>
#include <strings.h>

/* The IMSM Capability (IMSM AHCI and ISCU OROM/EFI variable) Version Table definition */
struct imsm_orom {
	__u8 signature[4];
	#define IMSM_OROM_SIGNATURE "$VER"
	#define IMSM_NVME_OROM_COMPAT_SIGNATURE "$NVM"
	__u8 table_ver_major; /* Currently 2 (can change with future revs) */
	__u8 table_ver_minor; /* Currently 2 (can change with future revs) */
	__u16 major_ver; /* Example: 8 as in 8.6.0.1020 */
	__u16 minor_ver; /* Example: 6 as in 8.6.0.1020 */
	__u16 hotfix_ver; /* Example: 0 as in 8.6.0.1020 */
	__u16 build; /* Example: 1020 as in 8.6.0.1020 */
	__u8 len; /* number of bytes in this entire table */
	__u8 checksum; /* checksum of all the bytes in this table */
	__u16 rlc; /* RAID Level Capability */
	/* we assume the cpu is x86 as the orom should not be found
	 * anywhere else
	 */
	#define IMSM_OROM_RLC_RAID0 (1 << 0)
	#define IMSM_OROM_RLC_RAID1 (1 << 1)
	#define IMSM_OROM_RLC_RAID10 (1 << 2)
	#define IMSM_OROM_RLC_RAID1E (1 << 3)
	#define IMSM_OROM_RLC_RAID5 (1 << 4)
	#define IMSM_OROM_RLC_RAID_CNG (1 << 5)
	__u16 sss; /* Strip Size Supported */
	#define IMSM_OROM_SSS_2kB (1 << 0)
	#define IMSM_OROM_SSS_4kB (1 << 1)
	#define IMSM_OROM_SSS_8kB (1 << 2)
	#define IMSM_OROM_SSS_16kB (1 << 3)
	#define IMSM_OROM_SSS_32kB (1 << 4)
	#define IMSM_OROM_SSS_64kB (1 << 5)
	#define IMSM_OROM_SSS_128kB (1 << 6)
	#define IMSM_OROM_SSS_256kB (1 << 7)
	#define IMSM_OROM_SSS_512kB (1 << 8)
	#define IMSM_OROM_SSS_1MB (1 << 9)
	#define IMSM_OROM_SSS_2MB (1 << 10)
	#define IMSM_OROM_SSS_4MB (1 << 11)
	#define IMSM_OROM_SSS_8MB (1 << 12)
	#define IMSM_OROM_SSS_16MB (1 << 13)
	#define IMSM_OROM_SSS_32MB (1 << 14)
	#define IMSM_OROM_SSS_64MB (1 << 15)
	__u16 dpa; /* Disks Per Array supported */
	#define IMSM_OROM_DISKS_PER_ARRAY 6
	#define IMSM_OROM_DISKS_PER_ARRAY_NVME 12
	__u16 tds; /* Total Disks Supported */
	#define IMSM_OROM_TOTAL_DISKS 6
	#define IMSM_OROM_TOTAL_DISKS_NVME 12
	__u8 vpa; /* # Volumes Per Array supported */
	#define IMSM_OROM_VOLUMES_PER_ARRAY 2
	__u8 vphba; /* # Volumes Per Host Bus Adapter supported */
	#define IMSM_OROM_VOLUMES_PER_HBA 4
	#define IMSM_OROM_VOLUMES_PER_HBA_NVME 4
	/* Attributes supported. This should map to the
	 * attributes in the MPB. Also, lower 16 bits
	 * should match/duplicate RLC bits above.
	 */
	__u32 attr;
	#define IMSM_OROM_ATTR_RAID0 IMSM_OROM_RLC_RAID0
	#define IMSM_OROM_ATTR_RAID1 IMSM_OROM_RLC_RAID1
	#define IMSM_OROM_ATTR_RAID10 IMSM_OROM_RLC_RAID10
	#define IMSM_OROM_ATTR_RAID1E IMSM_OROM_RLC_RAID1E
	#define IMSM_OROM_ATTR_RAID5 IMSM_OROM_RLC_RAID5
	#define IMSM_OROM_ATTR_RAID_CNG IMSM_OROM_RLC_RAID_CNG
	#define IMSM_OROM_ATTR_2TB_DISK (1 << 26)
	#define IMSM_OROM_ATTR_2TB (1 << 29)
	#define IMSM_OROM_ATTR_PM (1 << 30)
	#define IMSM_OROM_ATTR_ChecksumVerify (1 << 31)
	__u32 capabilities;
	#define IMSM_OROM_CAPABILITIES_Ext_SATA (1 << 0)
	#define IMSM_OROM_CAPABILITIES_TurboMemory (1 << 1)
	#define IMSM_OROM_CAPABILITIES_HddPassword (1 << 2)
	#define IMSM_OROM_CAPABILITIES_DiskCoercion (1 << 3)
	__u32 driver_features;
	#define IMSM_OROM_CAPABILITIES_HDDUnlock (1 << 0)
	#define IMSM_OROM_CAPABILITIES_LEDLoc (1 << 1)
	#define IMSM_OROM_CAPABILITIES_EnterpriseSystem (1 << 2)
	#define IMSM_OROM_CAPABILITIES_Zpodd (1 << 3)
	#define IMSM_OROM_CAPABILITIES_LargeDramCache (1 << 4)
	#define IMSM_OROM_CAPABILITIES_Rohi (1 << 5)
	#define IMSM_OROM_CAPABILITIES_ReadPatrol (1 << 6)
	#define IMSM_OROM_CAPABILITIES_XorHw (1 << 7)
	#define IMSM_OROM_CAPABILITIES_SKUMode ((1 << 8)|(1 << 9))
	#define IMSM_OROM_CAPABILITIES_TPV (1 << 10)
} __attribute__((packed));

static inline int imsm_orom_has_raid0(const struct imsm_orom *orom)
{
	return !!(orom->rlc & IMSM_OROM_RLC_RAID0);
}
static inline int imsm_orom_has_raid1(const struct imsm_orom *orom)
{
	return !!(orom->rlc & IMSM_OROM_RLC_RAID1);
}
static inline int imsm_orom_has_raid1e(const struct imsm_orom *orom)
{
	return !!(orom->rlc & IMSM_OROM_RLC_RAID1E);
}
static inline int imsm_orom_has_raid10(const struct imsm_orom *orom)
{
	return !!(orom->rlc & IMSM_OROM_RLC_RAID10);
}
static inline int imsm_orom_has_raid5(const struct imsm_orom *orom)
{
	return !!(orom->rlc & IMSM_OROM_RLC_RAID5);
}

/**
 * imsm_orom_has_chunk - check if the orom supports the given chunk size
 * @orom: orom pointer from find_imsm_orom
 * @chunk: chunk size in kibibytes
 */
static inline int imsm_orom_has_chunk(const struct imsm_orom *orom, int chunk)
{
	int fs = ffs(chunk);
	if (!fs)
		return 0;
	fs--; /* bit num to bit index */
	if (chunk & (chunk-1))
		return 0; /* not a power of 2 */
	return !!(orom->sss & (1 << (fs - 1)));
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 * The funciton is borrowed from Linux kernel code
 * include/asm-generic/bitops/fls.h
 */
static inline int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

static inline int imsm_orom_is_enterprise(const struct imsm_orom *orom)
{
	return !!(orom->driver_features & IMSM_OROM_CAPABILITIES_EnterpriseSystem);
}

static inline int imsm_orom_is_nvme(const struct imsm_orom *orom)
{
	return memcmp(orom->signature, IMSM_NVME_OROM_COMPAT_SIGNATURE,
			sizeof(orom->signature)) == 0;
}

static inline int imsm_orom_has_tpv_support(const struct imsm_orom *orom)
{
	return !!(orom->driver_features & IMSM_OROM_CAPABILITIES_TPV);
}

enum sys_dev_type {
	SYS_DEV_UNKNOWN = 0,
	SYS_DEV_SAS,
	SYS_DEV_SATA,
	SYS_DEV_NVME,
	SYS_DEV_VMD,
	SYS_DEV_MAX
};

struct sys_dev {
	enum sys_dev_type type;
	char *path;
	char *pci_id;
	__u16  dev_id;
	__u32  class;
	struct sys_dev *next;
};

struct efi_guid {
	__u8 b[16];
};

struct devid_list {
	__u16 devid;
	struct devid_list *next;
};

struct orom_entry {
	struct imsm_orom orom;
	struct devid_list *devid_list;
	enum sys_dev_type type;
	struct orom_entry *next;
};

extern struct orom_entry *orom_entries;

static inline char *guid_str(char *buf, struct efi_guid guid)
{
	sprintf(buf, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		 guid.b[3], guid.b[2], guid.b[1], guid.b[0],
		 guid.b[5], guid.b[4], guid.b[7], guid.b[6],
		 guid.b[8], guid.b[9], guid.b[10], guid.b[11],
		 guid.b[12], guid.b[13], guid.b[14], guid.b[15]);
	return buf;
}

char *diskfd_to_devpath(int fd);
__u16 devpath_to_vendor(const char *dev_path);
struct sys_dev *find_driver_devices(const char *bus, const char *driver);
struct sys_dev *find_intel_devices(void);
const struct imsm_orom *find_imsm_capability(struct sys_dev *hba);
const struct imsm_orom *find_imsm_orom(void);
int disk_attached_to_hba(int fd, const char *hba_path);
int devt_attached_to_hba(dev_t dev, const char *hba_path);
char *devt_to_devpath(dev_t dev);
int path_attached_to_hba(const char *disk_path, const char *hba_path);
const char *get_sys_dev_type(enum sys_dev_type);
const struct orom_entry *get_orom_entry_by_device_id(__u16 dev_id);
const struct imsm_orom *get_orom_by_device_id(__u16 device_id);
struct sys_dev *device_by_id(__u16 device_id);
struct sys_dev *device_by_id_and_path(__u16 device_id, const char *path);
char *vmd_domain_to_controller(struct sys_dev *hba, char *buf);
