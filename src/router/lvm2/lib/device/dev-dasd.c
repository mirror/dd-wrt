/*
 * Copyright (C) 2015 Red Hat, Inc. All rights reserved.
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
#include "lib/metadata/metadata.h"
#include "lib/device/dev-type.h"
#include <sys/ioctl.h>

#ifdef __linux__

/*
 * Interface taken from kernel header arch/s390/include/uapi/asm/dasd.h
 */

/* 
 * Author(s)......: Holger Smolinski <Holger.Smolinski@de.ibm.com>
 * Copyright IBM Corp. 1999, 2000
 * EMC Symmetrix ioctl Copyright EMC Corporation, 2008
 * Author.........: Nigel Hislop <hislop_nigel@emc.com>
 */

#define DASD_IOCTL_LETTER 'D'
#define DASD_API_VERSION 6

/* 
 * struct dasd_information2_t
 * represents any data about the device, which is visible to userspace.
 *  including foramt and featueres.
 */
typedef struct dasd_information2_t {
	unsigned int devno;		/* S/390 devno */
	unsigned int real_devno;	/* for aliases */
	unsigned int schid;		/* S/390 subchannel identifier */
	unsigned int cu_type  : 16;	/* from SenseID */
	unsigned int cu_model :  8;	/* from SenseID */
	unsigned int dev_type : 16;	/* from SenseID */
	unsigned int dev_model : 8;	/* from SenseID */
	unsigned int open_count;
	unsigned int req_queue_len;
	unsigned int chanq_len;		/* length of chanq */
	char type[4];			/* from discipline.name, 'none' for unknown */
	unsigned int status;		/* current device level */
	unsigned int label_block;	/* where to find the VOLSER */
	unsigned int FBA_layout;	/* fixed block size (like AIXVOL) */
	unsigned int characteristics_size;
	unsigned int confdata_size;
	char characteristics[64];	/* from read_device_characteristics */
	char configuration_data[256];	/* from read_configuration_data */
	unsigned int format;		/* format info like formatted/cdl/ldl/... */
	unsigned int features;		/* dasd features like 'ro',... */
	unsigned int reserved0;		/* reserved for further use ,... */
	unsigned int reserved1;		/* reserved for further use ,... */
	unsigned int reserved2;		/* reserved for further use ,... */
	unsigned int reserved3;		/* reserved for further use ,... */
	unsigned int reserved4;		/* reserved for further use ,... */
	unsigned int reserved5;		/* reserved for further use ,... */
	unsigned int reserved6;		/* reserved for further use ,... */
	unsigned int reserved7;		/* reserved for further use ,... */
} dasd_information2_t;

#define DASD_FORMAT_CDL  2

/* Get information on a dasd device (enhanced) */
#define BIODASDINFO2   _IOR(DASD_IOCTL_LETTER,3,dasd_information2_t)

/*
 * End of included interface.
 */

int dasd_is_cdl_formatted(struct device *dev)
{
	int ret = 0;
	dasd_information2_t dasd_info2;

	if (!dev_open_readonly(dev))
		return_0;

	if (ioctl(dev->fd, BIODASDINFO2, &dasd_info2)) {
		log_sys_error("ioctl BIODASDINFO2", dev_name(dev));
		goto out;
	}

	if (dasd_info2.format == DASD_FORMAT_CDL)
		ret = 1;

out:
	if (!dev_close(dev))
		stack;

	return ret;
}

#else

int dasd_is_cdl_formatted(struct device *dev)
{
	return 0;
}

#endif
