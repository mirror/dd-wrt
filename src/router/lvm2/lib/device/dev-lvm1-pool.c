/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"
#include "lib/device/dev-type.h"
#include "lib/mm/xlate.h"

/*
 * These lvm1 structs just used NAME_LEN in the previous format1 lvm2 code, but
 * NAME_LEN was defined as 128 in generic lvm2 code that was not lvm1-specific
 * and not disk-format-specific.
 */

#define LVM1_NAME_LEN 128

struct data_area {
	uint32_t base;
	uint32_t size;
} __attribute__ ((packed));

struct pv_disk {
	int8_t id[2];
	uint16_t version;       /* lvm version */
	struct data_area pv_on_disk;
	struct data_area vg_on_disk;
	struct data_area pv_uuidlist_on_disk;
	struct data_area lv_on_disk;
	struct data_area pe_on_disk;
	int8_t pv_uuid[LVM1_NAME_LEN];
	int8_t vg_name[LVM1_NAME_LEN];
	int8_t system_id[LVM1_NAME_LEN];     /* for vgexport/vgimport */
	uint32_t pv_major;
	uint32_t pv_number;
	uint32_t pv_status;
	uint32_t pv_allocatable;
	uint32_t pv_size;
	uint32_t lv_cur;
	uint32_t pe_size;
	uint32_t pe_total;
	uint32_t pe_allocated;

	/* only present on version == 2 pv's */
	uint32_t pe_start;
} __attribute__ ((packed));


int dev_is_lvm1(struct device *dev, char *buf, int buflen)
{
	struct pv_disk *pvd = (struct pv_disk *) buf;
	uint32_t version;
	int ret;

	version = xlate16(pvd->version);

	if (pvd->id[0] == 'H' && pvd->id[1] == 'M' &&
	    (version == 1 || version == 2))
		ret = 1;
	else
		ret = 0;

	return ret;
}


#define POOL_MAGIC 0x011670
#define POOL_NAME_SIZE 256

#define NSPMajorVersion        4
#define NSPMinorVersion        1
#define NSPUpdateLevel 3

/* When checking for version matching, the first two numbers **
** are important for metadata formats, a.k.a pool labels.   **
** All the numbers are important when checking if the user  **
** space tools match up with the kernel module............. */

#define POOL_VERSION           (NSPMajorVersion << 16 | \
				NSPMinorVersion <<  8 | \
				NSPUpdateLevel)

struct pool_disk {
	uint64_t pl_magic;      /* Pool magic number */
	uint64_t pl_pool_id;    /* Unique pool identifier */
	char pl_pool_name[POOL_NAME_SIZE];      /* Name of pool */
	uint32_t pl_version;    /* Pool version */
	uint32_t pl_subpools;   /* Number of subpools in this pool */
	uint32_t pl_sp_id;      /* Subpool number within pool */
	uint32_t pl_sp_devs;    /* Number of data partitions in this subpool */
	uint32_t pl_sp_devid;   /* Partition number within subpool */
	uint32_t pl_sp_type;    /* Partition type */
	uint64_t pl_blocks;     /* Number of blocks in this partition */
	uint32_t pl_striping;   /* Striping size within subpool */
	/*
	 * If the number of DMEP devices is zero, then the next field **
	 * ** (pl_sp_dmepid) becomes the subpool ID for redirection.  In **
	 * ** other words, if this subpool does not have the capability  **
	 * ** to do DMEP, then it must specify which subpool will do it  **
	 * ** in it's place
	 */

	/*
	 * While the next 3 field are no longer used, they must stay to keep **
	 * ** backward compatibility...........................................
	 */
	uint32_t pl_sp_dmepdevs;/* Number of dmep devices in this subpool */
	uint32_t pl_sp_dmepid;  /* Dmep device number within subpool */
	uint32_t pl_sp_weight;  /* if dmep dev, pref to using it */

	uint32_t pl_minor;      /* the pool minor number */
	uint32_t pl_padding;    /* reminder - think about alignment */

	/*
	 * Even though we're zeroing out 8k at the front of the disk before
	 * writing the label, putting this in
	 */
	char pl_reserve[184];   /* bump the structure size out to 512 bytes */
};

#define CPIN_8(x, y, z) {memcpy((x), (y), (z));}
#define CPIN_16(x, y) {(x) = xlate16_be((y));}
#define CPIN_32(x, y) {(x) = xlate32_be((y));}
#define CPIN_64(x, y) {(x) = xlate64_be((y));}

static void pool_label_in(struct pool_disk *pl, void *buf)
{
	struct pool_disk *bufpl = (struct pool_disk *) buf;

	CPIN_64(pl->pl_magic, bufpl->pl_magic);
	CPIN_64(pl->pl_pool_id, bufpl->pl_pool_id);
	CPIN_8(pl->pl_pool_name, bufpl->pl_pool_name, POOL_NAME_SIZE);
	CPIN_32(pl->pl_version, bufpl->pl_version);
	CPIN_32(pl->pl_subpools, bufpl->pl_subpools);
	CPIN_32(pl->pl_sp_id, bufpl->pl_sp_id);
	CPIN_32(pl->pl_sp_devs, bufpl->pl_sp_devs);
	CPIN_32(pl->pl_sp_devid, bufpl->pl_sp_devid);
	CPIN_32(pl->pl_sp_type, bufpl->pl_sp_type);
	CPIN_64(pl->pl_blocks, bufpl->pl_blocks);
	CPIN_32(pl->pl_striping, bufpl->pl_striping);
	CPIN_32(pl->pl_sp_dmepdevs, bufpl->pl_sp_dmepdevs);
	CPIN_32(pl->pl_sp_dmepid, bufpl->pl_sp_dmepid);
	CPIN_32(pl->pl_sp_weight, bufpl->pl_sp_weight);
	CPIN_32(pl->pl_minor, bufpl->pl_minor);
	CPIN_32(pl->pl_padding, bufpl->pl_padding);
	CPIN_8(pl->pl_reserve, bufpl->pl_reserve, 184);
}

int dev_is_pool(struct device *dev, char *buf, int buflen)
{
	struct pool_disk pd;
	int ret;

	pool_label_in(&pd, buf);

	/* can ignore 8 rightmost bits for ondisk format check */
	if ((pd.pl_magic == POOL_MAGIC) &&
	    (pd.pl_version >> 8 == POOL_VERSION >> 8))
		ret = 1;
	else
		ret = 0;

	return ret;
}

