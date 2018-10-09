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
#include "mdadm.h"
#include "platform-intel.h"
#include "probe_roms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

static int devpath_to_ll(const char *dev_path, const char *entry,
			 unsigned long long *val);

static void free_sys_dev(struct sys_dev **list)
{
	while (*list) {
		struct sys_dev *next = (*list)->next;

		if ((*list)->path)
			free((*list)->path);
		free(*list);
		*list = next;
	}
}

struct sys_dev *find_driver_devices(const char *bus, const char *driver)
{
	/* search sysfs for devices driven by 'driver' */
	char path[PATH_MAX];
	char link[PATH_MAX];
	char *c, *p;
	DIR *driver_dir;
	struct dirent *de;
	struct sys_dev *head = NULL;
	struct sys_dev *list = NULL;
	struct sys_dev *vmd = NULL;
	enum sys_dev_type type;
	unsigned long long dev_id;
	unsigned long long class;

	if (strcmp(driver, "isci") == 0)
		type = SYS_DEV_SAS;
	else if (strcmp(driver, "ahci") == 0)
		type = SYS_DEV_SATA;
	else if (strcmp(driver, "nvme") == 0) {
		/* if looking for nvme devs, first look for vmd */
		vmd = find_driver_devices("pci", "vmd");
		type = SYS_DEV_NVME;
	} else if (strcmp(driver, "vmd") == 0)
		type = SYS_DEV_VMD;
	else
		type = SYS_DEV_UNKNOWN;

	sprintf(path, "/sys/bus/%s/drivers/%s", bus, driver);
	driver_dir = opendir(path);
	if (!driver_dir) {
		if (vmd)
			free_sys_dev(&vmd);
		return NULL;
	}
	for (de = readdir(driver_dir); de; de = readdir(driver_dir)) {
		int n;
		int skip = 0;

		/* is 'de' a device? check that the 'subsystem' link exists and
		 * that its target matches 'bus'
		 */
		sprintf(path, "/sys/bus/%s/drivers/%s/%s/subsystem",
			bus, driver, de->d_name);
		n = readlink(path, link, sizeof(link));
		if (n < 0 || n >= (int)sizeof(link))
			continue;
		link[n] = '\0';
		c = strrchr(link, '/');
		if (!c)
			continue;
		if (strncmp(bus, c+1, strlen(bus)) != 0)
			continue;

		sprintf(path, "/sys/bus/%s/drivers/%s/%s",
			bus, driver, de->d_name);

		/* if searching for nvme - skip vmd connected one */
		if (type == SYS_DEV_NVME) {
			struct sys_dev *dev;
			char *rp = realpath(path, NULL);
			for (dev = vmd; dev; dev = dev->next) {
				if ((strncmp(dev->path, rp, strlen(dev->path)) == 0))
					skip = 1;
			}
			free(rp);
		}

		/* if it's not Intel device or mark as VMD connected - skip it. */
		if (devpath_to_vendor(path) != 0x8086 || skip == 1)
			continue;

		if (devpath_to_ll(path, "device", &dev_id) != 0)
			continue;

		if (devpath_to_ll(path, "class", &class) != 0)
			continue;

		/*
		 * Each VMD device (domain) adds separate PCI bus, it is better
		 * to store path as a path to that bus (easier further
		 * determination which NVMe dev is connected to this particular
		 * VMD domain).
		 */
		if (type == SYS_DEV_VMD) {
			sprintf(path, "/sys/bus/%s/drivers/%s/%s/domain/device",
				bus, driver, de->d_name);
		}
		p = realpath(path, NULL);
		if (p == NULL) {
			pr_err("Unable to get real path for '%s'\n", path);
			continue;
		}

		/* start / add list entry */
		if (!head) {
			head = xmalloc(sizeof(*head));
			list = head;
		} else {
			list->next = xmalloc(sizeof(*head));
			list = list->next;
		}

		if (!list) {
			free_sys_dev(&head);
			break;
		}

		list->dev_id = (__u16) dev_id;
		list->class = (__u32) class;
		list->type = type;
		list->next = NULL;
		list->path = p;

		if ((list->pci_id = strrchr(list->path, '/')) != NULL)
			list->pci_id++;
	}
	closedir(driver_dir);

	if (vmd) {
		if (list)
			list->next = vmd;
		else
			head = vmd;
	}

	return head;
}

static struct sys_dev *intel_devices=NULL;
static time_t valid_time = 0;

struct sys_dev *device_by_id(__u16 device_id)
{
	struct sys_dev *iter;

	for (iter = intel_devices; iter != NULL; iter = iter->next)
		if (iter->dev_id == device_id)
			return iter;
	return NULL;
}

struct sys_dev *device_by_id_and_path(__u16 device_id, const char *path)
{
	struct sys_dev *iter;

	for (iter = intel_devices; iter != NULL; iter = iter->next)
		if ((iter->dev_id == device_id) && strstr(iter->path, path))
			return iter;
	return NULL;
}

static int devpath_to_ll(const char *dev_path, const char *entry, unsigned long long *val)
{
	char path[strlen(dev_path) + strlen(entry) + 2];
	int fd;
	int n;

	sprintf(path, "%s/%s", dev_path, entry);

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	n = sysfs_fd_get_ll(fd, val);
	close(fd);
	return n;
}

__u16 devpath_to_vendor(const char *dev_path)
{
	char path[strlen(dev_path) + strlen("/vendor") + 1];
	char vendor[7];
	int fd;
	__u16 id = 0xffff;
	int n;

	sprintf(path, "%s/vendor", dev_path);

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0xffff;

	n = read(fd, vendor, sizeof(vendor));
	if (n == sizeof(vendor)) {
		vendor[n - 1] = '\0';
		id = strtoul(vendor, NULL, 16);
	}
	close(fd);

	return id;
}

struct sys_dev *find_intel_devices(void)
{
	struct sys_dev *ahci, *isci, *nvme;

	if (valid_time > time(0) - 10)
		return intel_devices;

	if (intel_devices)
		free_sys_dev(&intel_devices);

	isci = find_driver_devices("pci", "isci");
	ahci = find_driver_devices("pci", "ahci");
	/* Searching for NVMe will return list of NVMe and VMD controllers */
	nvme = find_driver_devices("pci", "nvme");

	if (!isci && !ahci) {
		ahci = nvme;
	} else if (!ahci) {
		ahci = isci;
		struct sys_dev *elem = ahci;
		while (elem->next)
			elem = elem->next;
		elem->next = nvme;
	} else {
		struct sys_dev *elem = ahci;
		while (elem->next)
			elem = elem->next;
		elem->next = isci;
		while (elem->next)
			elem = elem->next;
		elem->next = nvme;
	}
	intel_devices = ahci;
	valid_time = time(0);
	return intel_devices;
}

/*
 * PCI Expansion ROM Data Structure Format */
struct pciExpDataStructFormat {
	__u8  ver[4];
	__u16 vendorID;
	__u16 deviceID;
	__u16 devListOffset;
	__u16 pciDataStructLen;
	__u8 pciDataStructRev;
} __attribute__ ((packed));

struct orom_entry *orom_entries;

const struct orom_entry *get_orom_entry_by_device_id(__u16 dev_id)
{
	struct orom_entry *entry;
	struct devid_list *devid;

	for (entry = orom_entries; entry; entry = entry->next) {
		for (devid = entry->devid_list; devid; devid = devid->next) {
			if (devid->devid == dev_id)
				return entry;
		}
	}

	return NULL;
}

const struct imsm_orom *get_orom_by_device_id(__u16 dev_id)
{
	const struct orom_entry *entry = get_orom_entry_by_device_id(dev_id);

	if (entry)
		return &entry->orom;

	return NULL;
}

static struct orom_entry *add_orom(const struct imsm_orom *orom)
{
	struct orom_entry *list;
	struct orom_entry *prev = NULL;

	for (list = orom_entries; list; prev = list, list = list->next)
		;

	list = xmalloc(sizeof(struct orom_entry));
	list->orom = *orom;
	list->devid_list = NULL;
	list->next = NULL;

	if (prev == NULL)
		orom_entries = list;
	else
		prev->next = list;

	return list;
}

static void add_orom_device_id(struct orom_entry *entry, __u16 dev_id)
{
	struct devid_list *list;
	struct devid_list *prev = NULL;

	for (list = entry->devid_list; list; prev = list, list = list->next) {
		if (list->devid == dev_id)
			return;
	}
	list = xmalloc(sizeof(struct devid_list));
	list->devid = dev_id;
	list->next = NULL;

	if (prev == NULL)
		entry->devid_list = list;
	else
		prev->next = list;
}

static int scan(const void *start, const void *end, const void *data)
{
	int offset;
	const struct imsm_orom *imsm_mem = NULL;
	int len = (end - start);
	struct pciExpDataStructFormat *ptr= (struct pciExpDataStructFormat *)data;

	if (data + 0x18 > end) {
		dprintf("cannot find pciExpDataStruct \n");
		return 0;
	}

	dprintf("ptr->vendorID: %lx __le16_to_cpu(ptr->deviceID): %lx \n",
		(ulong) __le16_to_cpu(ptr->vendorID),
		(ulong) __le16_to_cpu(ptr->deviceID));

	if (__le16_to_cpu(ptr->vendorID) != 0x8086)
		return 0;

	if (get_orom_by_device_id(ptr->deviceID))
		return 0;

	for (offset = 0; offset < len; offset += 4) {
		const void *mem = start + offset;

		if ((memcmp(mem, IMSM_OROM_SIGNATURE, 4) == 0)) {
			imsm_mem = mem;
			break;
		}
	}

	if (!imsm_mem)
		return 0;

	struct orom_entry *orom = add_orom(imsm_mem);

	/* only PciDataStructure with revision 3 and above supports devices list. */
	if (ptr->pciDataStructRev >= 3 && ptr->devListOffset) {
		const __u16 *dev_list = (void *)ptr + ptr->devListOffset;
		int i;

		for (i = 0; dev_list[i] != 0; i++)
			add_orom_device_id(orom, dev_list[i]);
	} else {
		add_orom_device_id(orom, __le16_to_cpu(ptr->deviceID));
	}

	return 0;
}

const struct imsm_orom *imsm_platform_test(struct sys_dev *hba)
{
	struct imsm_orom orom = {
		.signature = IMSM_OROM_SIGNATURE,
		.rlc = IMSM_OROM_RLC_RAID0 | IMSM_OROM_RLC_RAID1 |
					IMSM_OROM_RLC_RAID10 | IMSM_OROM_RLC_RAID5,
		.sss = IMSM_OROM_SSS_4kB | IMSM_OROM_SSS_8kB |
					IMSM_OROM_SSS_16kB | IMSM_OROM_SSS_32kB |
					IMSM_OROM_SSS_64kB | IMSM_OROM_SSS_128kB |
					IMSM_OROM_SSS_256kB | IMSM_OROM_SSS_512kB |
					IMSM_OROM_SSS_1MB | IMSM_OROM_SSS_2MB,
		.dpa = IMSM_OROM_DISKS_PER_ARRAY,
		.tds = IMSM_OROM_TOTAL_DISKS,
		.vpa = IMSM_OROM_VOLUMES_PER_ARRAY,
		.vphba = IMSM_OROM_VOLUMES_PER_HBA
	};
	orom.attr = orom.rlc | IMSM_OROM_ATTR_ChecksumVerify;

	if (check_env("IMSM_TEST_OROM_NORAID5")) {
		orom.rlc = IMSM_OROM_RLC_RAID0 | IMSM_OROM_RLC_RAID1 |
				IMSM_OROM_RLC_RAID10;
	}
	if (check_env("IMSM_TEST_AHCI_EFI_NORAID5") && (hba->type == SYS_DEV_SAS)) {
		orom.rlc = IMSM_OROM_RLC_RAID0 | IMSM_OROM_RLC_RAID1 |
				IMSM_OROM_RLC_RAID10;
	}
	if (check_env("IMSM_TEST_SCU_EFI_NORAID5") && (hba->type == SYS_DEV_SATA)) {
		orom.rlc = IMSM_OROM_RLC_RAID0 | IMSM_OROM_RLC_RAID1 |
				IMSM_OROM_RLC_RAID10;
	}

	struct orom_entry *ret = add_orom(&orom);

	add_orom_device_id(ret, hba->dev_id);

	return &ret->orom;
}

static const struct imsm_orom *find_imsm_hba_orom(struct sys_dev *hba)
{
	unsigned long align;

	if (check_env("IMSM_TEST_OROM"))
		return imsm_platform_test(hba);

	/* return empty OROM capabilities in EFI test mode */
	if (check_env("IMSM_TEST_AHCI_EFI") || check_env("IMSM_TEST_SCU_EFI"))
		return NULL;

	find_intel_devices();

	if (intel_devices == NULL)
		return NULL;

	/* scan option-rom memory looking for an imsm signature */
	if (check_env("IMSM_SAFE_OROM_SCAN"))
		align = 2048;
	else
		align = 512;
	if (probe_roms_init(align) != 0)
		return NULL;
	probe_roms();
	/* ignore return value - True is returned if both adapater roms are found */
	scan_adapter_roms(scan);
	probe_roms_exit();

	return get_orom_by_device_id(hba->dev_id);
}

#define GUID_STR_MAX	37  /* according to GUID format:
			     * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" */

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
((struct efi_guid) \
{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
  (b) & 0xff, ((b) >> 8) & 0xff, \
  (c) & 0xff, ((c) >> 8) & 0xff, \
  (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }})

#define SYS_EFI_VAR_PATH "/sys/firmware/efi/vars"
#define SYS_EFIVARS_PATH "/sys/firmware/efi/efivars"
#define SCU_PROP "RstScuV"
#define AHCI_PROP "RstSataV"
#define AHCI_SSATA_PROP "RstsSatV"
#define AHCI_CSATA_PROP "RstCSatV"
#define VMD_PROP "RstUefiV"

#define VENDOR_GUID \
	EFI_GUID(0x193dfefa, 0xa445, 0x4302, 0x99, 0xd8, 0xef, 0x3a, 0xad, 0x1a, 0x04, 0xc6)

#define PCI_CLASS_RAID_CNTRL 0x010400

static int read_efi_var(void *buffer, ssize_t buf_size, char *variable_name, struct efi_guid guid)
{
	char path[PATH_MAX];
	char buf[GUID_STR_MAX];
	int fd;
	ssize_t n;

	snprintf(path, PATH_MAX, "%s/%s-%s", SYS_EFIVARS_PATH, variable_name, guid_str(buf, guid));

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 1;

	/* read the variable attributes and ignore it */
	n = read(fd, buf, sizeof(__u32));
	if (n < 0) {
		close(fd);
		return 1;
	}

	/* read the variable data */
	n = read(fd, buffer, buf_size);
	close(fd);
	if (n < buf_size)
		return 1;

	return 0;
}

static int read_efi_variable(void *buffer, ssize_t buf_size, char *variable_name, struct efi_guid guid)
{
	char path[PATH_MAX];
	char buf[GUID_STR_MAX];
	int dfd;
	ssize_t n, var_data_len;

	/* Try to read the variable using the new efivarfs interface first.
	 * If that fails, fall back to the old sysfs-efivars interface. */
	if (!read_efi_var(buffer, buf_size, variable_name, guid))
		return 0;

	snprintf(path, PATH_MAX, "%s/%s-%s/size", SYS_EFI_VAR_PATH, variable_name, guid_str(buf, guid));

	dprintf("EFI VAR: path=%s\n", path);
	/* get size of variable data */
	dfd = open(path, O_RDONLY);
	if (dfd < 0)
		return 1;

	n = read(dfd, &buf, sizeof(buf));
	close(dfd);
	if (n < 0)
		return 1;
	buf[n] = '\0';

	errno = 0;
	var_data_len = strtoul(buf, NULL, 16);
	if ((errno == ERANGE && (var_data_len == LONG_MAX)) ||
	    (errno != 0 && var_data_len == 0))
		return 1;

	/* get data */
	snprintf(path, PATH_MAX, "%s/%s-%s/data", SYS_EFI_VAR_PATH, variable_name, guid_str(buf, guid));

	dprintf("EFI VAR: path=%s\n", path);
	dfd = open(path, O_RDONLY);
	if (dfd < 0)
		return 1;

	n = read(dfd, buffer, buf_size);
	close(dfd);
	if (n != var_data_len || n < buf_size) {
		return 1;
	}

	return 0;
}

const struct imsm_orom *find_imsm_efi(struct sys_dev *hba)
{
	struct imsm_orom orom;
	struct orom_entry *ret;
	int err;

	if (check_env("IMSM_TEST_AHCI_EFI") || check_env("IMSM_TEST_SCU_EFI"))
		return imsm_platform_test(hba);

	/* OROM test is set, return that there is no EFI capabilities */
	if (check_env("IMSM_TEST_OROM"))
		return NULL;

	if (hba->type == SYS_DEV_SATA && hba->class != PCI_CLASS_RAID_CNTRL)
		return NULL;

	err = read_efi_variable(&orom, sizeof(orom), hba->type == SYS_DEV_SAS ? SCU_PROP : AHCI_PROP, VENDOR_GUID);

	/* try to read variable for second AHCI controller */
	if (err && hba->type == SYS_DEV_SATA)
		err = read_efi_variable(&orom, sizeof(orom), AHCI_SSATA_PROP, VENDOR_GUID);

	/* try to read variable for combined AHCI controllers */
	if (err && hba->type == SYS_DEV_SATA) {
		static struct orom_entry *csata;

		err = read_efi_variable(&orom, sizeof(orom), AHCI_CSATA_PROP, VENDOR_GUID);
		if (!err) {
			if (!csata)
				csata = add_orom(&orom);
			add_orom_device_id(csata, hba->dev_id);
			csata->type = hba->type;
			return &csata->orom;
		}
	}

	if (hba->type == SYS_DEV_VMD) {
		err = read_efi_variable(&orom, sizeof(orom), VMD_PROP, VENDOR_GUID);
	}

	if (err)
		return NULL;

	ret = add_orom(&orom);
	add_orom_device_id(ret, hba->dev_id);
	ret->type = hba->type;

	return &ret->orom;
}

const struct imsm_orom *find_imsm_nvme(struct sys_dev *hba)
{
	static struct orom_entry *nvme_orom;

	if (hba->type != SYS_DEV_NVME)
		return NULL;

	if (!nvme_orom) {
		struct imsm_orom nvme_orom_compat = {
			.signature = IMSM_NVME_OROM_COMPAT_SIGNATURE,
			.rlc = IMSM_OROM_RLC_RAID0 | IMSM_OROM_RLC_RAID1 |
						IMSM_OROM_RLC_RAID10 | IMSM_OROM_RLC_RAID5,
			.sss = IMSM_OROM_SSS_4kB | IMSM_OROM_SSS_8kB |
						IMSM_OROM_SSS_16kB | IMSM_OROM_SSS_32kB |
						IMSM_OROM_SSS_64kB | IMSM_OROM_SSS_128kB,
			.dpa = IMSM_OROM_DISKS_PER_ARRAY_NVME,
			.tds = IMSM_OROM_TOTAL_DISKS_NVME,
			.vpa = IMSM_OROM_VOLUMES_PER_ARRAY,
			.vphba = IMSM_OROM_TOTAL_DISKS_NVME / 2 * IMSM_OROM_VOLUMES_PER_ARRAY,
			.attr = IMSM_OROM_ATTR_2TB | IMSM_OROM_ATTR_2TB_DISK,
			.driver_features = IMSM_OROM_CAPABILITIES_EnterpriseSystem
		};
		nvme_orom = add_orom(&nvme_orom_compat);
	}
	add_orom_device_id(nvme_orom, hba->dev_id);
	nvme_orom->type = SYS_DEV_NVME;
	return &nvme_orom->orom;
}

const struct imsm_orom *find_imsm_capability(struct sys_dev *hba)
{
	const struct imsm_orom *cap = get_orom_by_device_id(hba->dev_id);

	if (cap)
		return cap;

	if (hba->type == SYS_DEV_NVME)
		return find_imsm_nvme(hba);
	if ((cap = find_imsm_efi(hba)) != NULL)
		return cap;
	if ((cap = find_imsm_hba_orom(hba)) != NULL)
		return cap;

	return NULL;
}

char *devt_to_devpath(dev_t dev)
{
	char device[46];

	sprintf(device, "/sys/dev/block/%d:%d/device", major(dev), minor(dev));
	return realpath(device, NULL);
}

char *diskfd_to_devpath(int fd)
{
	/* return the device path for a disk, return NULL on error or fd
	 * refers to a partition
	 */
	struct stat st;

	if (fstat(fd, &st) != 0)
		return NULL;
	if (!S_ISBLK(st.st_mode))
		return NULL;

	return devt_to_devpath(st.st_rdev);
}

int path_attached_to_hba(const char *disk_path, const char *hba_path)
{
	int rc;

	if (check_env("IMSM_TEST_AHCI_DEV") ||
	    check_env("IMSM_TEST_SCU_DEV")) {
		return 1;
	}

	if (!disk_path || !hba_path)
		return 0;
	dprintf("hba: %s - disk: %s\n", hba_path, disk_path);
	if (strncmp(disk_path, hba_path, strlen(hba_path)) == 0)
		rc = 1;
	else
		rc = 0;

	return rc;
}

int devt_attached_to_hba(dev_t dev, const char *hba_path)
{
	char *disk_path = devt_to_devpath(dev);
	int rc = path_attached_to_hba(disk_path, hba_path);

	if (disk_path)
		free(disk_path);

	return rc;
}

int disk_attached_to_hba(int fd, const char *hba_path)
{
	char *disk_path = diskfd_to_devpath(fd);
	int rc = path_attached_to_hba(disk_path, hba_path);

	if (disk_path)
		free(disk_path);

	return rc;
}

char *vmd_domain_to_controller(struct sys_dev *hba, char *buf)
{
	struct dirent *ent;
	DIR *dir;
	char path[PATH_MAX];

	if (!hba)
		return NULL;

	if (hba->type != SYS_DEV_VMD)
		return NULL;

	dir = opendir("/sys/bus/pci/drivers/vmd");
	if (!dir)
		return NULL;

	for (ent = readdir(dir); ent; ent = readdir(dir)) {
		sprintf(path, "/sys/bus/pci/drivers/vmd/%s/domain/device",
			ent->d_name);

		if (!realpath(path, buf))
			continue;

		if (strncmp(buf, hba->path, strlen(buf)) == 0) {
			sprintf(path, "/sys/bus/pci/drivers/vmd/%s", ent->d_name);
			closedir(dir);
			return realpath(path, buf);
		}
	}

	closedir(dir);
	return NULL;
}
