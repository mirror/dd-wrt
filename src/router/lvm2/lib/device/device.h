/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_DEVICE_H
#define _LVM_DEVICE_H

#include "lib/uuid/uuid.h"

#include <fcntl.h>

#define DEV_ACCESSED_W		0x00000001	/* Device written to? */
#define DEV_REGULAR		0x00000002	/* Regular file? */
#define DEV_ALLOCED		0x00000004	/* malloc used */
#define DEV_OPENED_RW		0x00000008	/* Opened RW */
#define DEV_OPENED_EXCL		0x00000010	/* Opened EXCL */
#define DEV_O_DIRECT		0x00000020	/* Use O_DIRECT */
#define DEV_O_DIRECT_TESTED	0x00000040	/* DEV_O_DIRECT is reliable */
#define DEV_OPEN_FAILURE	0x00000080	/* Has last open failed? */
#define DEV_USED_FOR_LV		0x00000100	/* Is device used for an LV */
#define DEV_ASSUMED_FOR_LV	0x00000200	/* Is device assumed for an LV */
#define DEV_NOT_O_NOATIME	0x00000400	/* Don't use O_NOATIME */
#define DEV_IN_BCACHE		0x00000800      /* dev fd is open and used in bcache */
#define DEV_BCACHE_EXCL		0x00001000      /* bcache_fd should be open EXCL */
#define DEV_FILTER_AFTER_SCAN	0x00002000	/* apply filter after bcache has data */
#define DEV_FILTER_OUT_SCAN	0x00004000	/* filtered out during label scan */
#define DEV_BCACHE_WRITE	0x00008000      /* bcache_fd is open with RDWR */

/*
 * Support for external device info.
 * Any new external device info source needs to be
 * registered using EXT_REGISTER macro in dev-ext.c.
 */
typedef enum dev_ext_e {
	DEV_EXT_NONE,
	DEV_EXT_UDEV,
	DEV_EXT_NUM
} dev_ext_t;

struct dev_ext {
	int enabled;
	dev_ext_t src;
	void *handle;
};

/*
 * All devices in LVM will be represented by one of these.
 * pointer comparisons are valid.
 */
struct device {
	struct dm_list aliases;	/* struct dm_str_list */
	dev_t dev;

	/* private */
	int fd;
	int open_count;
	int phys_block_size;
	int block_size;
	int read_ahead;
	int bcache_fd;
	uint32_t flags;
	unsigned size_seqno;
	uint64_t size;
	uint64_t end;
	struct dev_ext ext;
	const char *duplicate_prefer_reason;

	const char *vgid; /* if device is an LV */
	const char *lvid; /* if device is an LV */

	char pvid[ID_LEN + 1]; /* if device is a PV */
	char _padding[7];
};

/*
 * All I/O is annotated with the reason it is performed.
 */
typedef enum dev_io_reason {
	DEV_IO_SIGNATURES = 0,	/* Scanning device signatures */
	DEV_IO_LABEL,		/* LVM PV disk label */
	DEV_IO_MDA_HEADER,	/* Text format metadata area header */
	DEV_IO_MDA_CONTENT,	/* Text format metadata area content */
	DEV_IO_MDA_EXTRA_HEADER,	/* Header of any extra metadata areas on device */
	DEV_IO_MDA_EXTRA_CONTENT,	/* Content of any extra metadata areas on device */
	DEV_IO_FMT1,		/* Original LVM1 metadata format */
	DEV_IO_POOL,		/* Pool metadata format */
	DEV_IO_LV,		/* Content written to an LV */
	DEV_IO_LOG		/* Logging messages */
} dev_io_reason_t;

struct device_list {
	struct dm_list list;
	struct device *dev;
};

struct device_area {
	struct device *dev;
	uint64_t start;		/* Bytes */
	uint64_t size;		/* Bytes */
};

/*
 * Support for external device info.
 */
const char *dev_ext_name(struct device *dev);
int dev_ext_enable(struct device *dev, dev_ext_t src);
int dev_ext_disable(struct device *dev);
struct dev_ext *dev_ext_get(struct device *dev);
int dev_ext_release(struct device *dev);

/*
 * Increment current dev_size_seqno.
 * This is used to control lifetime
 * of cached device size.
 */
void dev_size_seqno_inc(void);

/*
 * All io should use these routines.
 */
int dev_get_block_size(struct device *dev, unsigned int *phys_block_size, unsigned int *block_size);
int dev_get_size(struct device *dev, uint64_t *size);
int dev_get_read_ahead(struct device *dev, uint32_t *read_ahead);
int dev_discard_blocks(struct device *dev, uint64_t offset_bytes, uint64_t size_bytes);

/* Use quiet version if device number could change e.g. when opening LV */
int dev_open(struct device *dev);
int dev_open_quiet(struct device *dev);
int dev_open_flags(struct device *dev, int flags, int direct, int quiet);
int dev_open_readonly(struct device *dev);
int dev_open_readonly_buffered(struct device *dev);
int dev_open_readonly_quiet(struct device *dev);
int dev_close(struct device *dev);
int dev_close_immediate(struct device *dev);
int dev_test_excl(struct device *dev);

int dev_fd(struct device *dev);
const char *dev_name(const struct device *dev);

int dev_read(struct device *dev, uint64_t offset, size_t len, dev_io_reason_t reason, void *buffer);
int dev_read_circular(struct device *dev, uint64_t offset, size_t len,
		      uint64_t offset2, size_t len2, dev_io_reason_t reason, char *buf);
int dev_write(struct device *dev, uint64_t offset, size_t len, dev_io_reason_t reason, void *buffer);
int dev_append(struct device *dev, size_t len, dev_io_reason_t reason, char *buffer);
int dev_set(struct device *dev, uint64_t offset, size_t len, dev_io_reason_t reason, int value);
void dev_flush(struct device *dev);

struct device *dev_create_file(const char *filename, struct device *dev,
			       struct dm_str_list *alias, int use_malloc);
void dev_destroy_file(struct device *dev);

/* Return a valid device name from the alias list; NULL otherwise */
const char *dev_name_confirmed(struct device *dev, int quiet);

#endif
