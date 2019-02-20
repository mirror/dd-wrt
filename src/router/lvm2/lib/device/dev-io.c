/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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
#include "lib/device/device.h"
#include "lib/metadata/metadata.h"
#include "lib/mm/memlock.h"

#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef __linux__
#  define u64 uint64_t		/* Missing without __KERNEL__ */
#  undef WNOHANG		/* Avoid redefinition */
#  undef WUNTRACED		/* Avoid redefinition */
#  include <linux/fs.h>		/* For block ioctl definitions */
#  define BLKSIZE_SHIFT SECTOR_SHIFT
#  ifndef BLKGETSIZE64		/* fs.h out-of-date */
#    define BLKGETSIZE64 _IOR(0x12, 114, size_t)
#  endif /* BLKGETSIZE64 */
#  ifndef BLKDISCARD
#    define BLKDISCARD	_IO(0x12,119)
#  endif
#else
#  include <sys/disk.h>
#  define BLKBSZGET DKIOCGETBLOCKSIZE
#  define BLKSSZGET DKIOCGETBLOCKSIZE
#  define BLKGETSIZE64 DKIOCGETBLOCKCOUNT
#  define BLKFLSBUF DKIOCSYNCHRONIZECACHE
#  define BLKSIZE_SHIFT 0
#endif

#ifdef O_DIRECT_SUPPORT
#  ifndef O_DIRECT
#    error O_DIRECT support configured but O_DIRECT definition not found in headers
#  endif
#endif

static unsigned _dev_size_seqno = 1;

static const char *_reasons[] = {
	"dev signatures",
	"PV labels",
	"VG metadata header",
	"VG metadata content",
	"extra VG metadata header",
	"extra VG metadata content",
	"LVM1 metadata",
	"pool metadata",
	"LV content",
	"logging",
};

static const char *_reason_text(dev_io_reason_t reason)
{
	return _reasons[(unsigned) reason];
}

/*-----------------------------------------------------------------
 * The standard io loop that keeps submitting an io until it's
 * all gone.
 *---------------------------------------------------------------*/
static int _io(struct device_area *where, char *buffer, int should_write, dev_io_reason_t reason)
{
	int fd = dev_fd(where->dev);
	ssize_t n = 0;
	size_t total = 0;

	if (fd < 0) {
		log_error("Attempt to read an unopened device (%s).",
			  dev_name(where->dev));
		return 0;
	}

	log_debug_io("%s %s:%8" PRIu64 " bytes (sync) at %" PRIu64 "%s (for %s)",
		     should_write ? "Write" : "Read ", dev_name(where->dev),
		     where->size, (uint64_t) where->start,
		     (should_write && test_mode()) ? " (test mode - suppressed)" : "", _reason_text(reason));

	/*
	 * Skip all writes in test mode.
	 */
	if (should_write && test_mode())
		return 1;

	if (where->size > SSIZE_MAX) {
		log_error("Read size too large: %" PRIu64, where->size);
		return 0;
	}

	if (lseek(fd, (off_t) where->start, SEEK_SET) == (off_t) -1) {
		log_error("%s: lseek %" PRIu64 " failed: %s",
			  dev_name(where->dev), (uint64_t) where->start,
			  strerror(errno));
		return 0;
	}

	while (total < (size_t) where->size) {
		do
			n = should_write ?
			    write(fd, buffer, (size_t) where->size - total) :
			    read(fd, buffer, (size_t) where->size - total);
		while ((n < 0) && ((errno == EINTR) || (errno == EAGAIN)));

		if (n < 0)
			log_error_once("%s: %s failed after %" PRIu64 " of %" PRIu64
				       " at %" PRIu64 ": %s", dev_name(where->dev),
				       should_write ? "write" : "read",
				       (uint64_t) total,
				       (uint64_t) where->size,
				       (uint64_t) where->start, strerror(errno));

		if (n <= 0)
			break;

		total += n;
		buffer += n;
	}

	return (total == (size_t) where->size);
}

/*-----------------------------------------------------------------
 * LVM2 uses O_DIRECT when performing metadata io, which requires
 * block size aligned accesses.  If any io is not aligned we have
 * to perform the io via a bounce buffer, obviously this is quite
 * inefficient.
 *---------------------------------------------------------------*/

/*
 * Get the physical and logical block size for a device.
 */
int dev_get_block_size(struct device *dev, unsigned int *physical_block_size, unsigned int *block_size)
{
	const char *name = dev_name(dev);
	int fd = dev->bcache_fd;
	int do_close = 0;
	int r = 1;

	if ((dev->phys_block_size > 0) && (dev->block_size > 0)) {
		*physical_block_size = (unsigned int)dev->phys_block_size;
		*block_size = (unsigned int)dev->block_size;
		return 1;
	}

	if (fd <= 0) {
		if (!dev->open_count) {
			if (!dev_open_readonly(dev))
				return_0;
			do_close = 1;
		}
		fd = dev_fd(dev);
	}

	if (dev->block_size == -1) {
		if (ioctl(fd, BLKBSZGET, &dev->block_size) < 0) {
			log_sys_error("ioctl BLKBSZGET", name);
			r = 0;
			goto out;
		}
		log_debug_devs("%s: Block size is %u bytes", name, dev->block_size);
	}

#ifdef BLKPBSZGET
	/* BLKPBSZGET is available in kernel >= 2.6.32 only */
	if (dev->phys_block_size == -1) {
		if (ioctl(fd, BLKPBSZGET, &dev->phys_block_size) < 0) {
			log_sys_error("ioctl BLKPBSZGET", name);
			r = 0;
			goto out;
		}
		log_debug_devs("%s: Physical block size is %u bytes", name, dev->phys_block_size);
	}
#elif defined (BLKSSZGET)
	/* if we can't get physical block size, just use logical block size instead */
	if (dev->phys_block_size == -1) {
		if (ioctl(fd, BLKSSZGET, &dev->phys_block_size) < 0) {
			log_sys_error("ioctl BLKSSZGET", name);
			r = 0;
			goto out;
		}
		log_debug_devs("%s: Physical block size can't be determined: Using logical block size of %u bytes", name, dev->phys_block_size);
	}
#else
	/* if even BLKSSZGET is not available, use default 512b */
	if (dev->phys_block_size == -1) {
		dev->phys_block_size = 512;
		log_debug_devs("%s: Physical block size can't be determined: Using block size of %u bytes instead", name, dev->phys_block_size);
	}
#endif

	*physical_block_size = (unsigned int) dev->phys_block_size;
	*block_size = (unsigned int) dev->block_size;
out:
	if (do_close && !dev_close_immediate(dev))
		stack;

	return r;
}

/*
 * Widens a region to be an aligned region.
 */
static void _widen_region(unsigned int block_size, struct device_area *region,
			  struct device_area *result)
{
	uint64_t mask = block_size - 1, delta;
	memcpy(result, region, sizeof(*result));

	/* adjust the start */
	delta = result->start & mask;
	if (delta) {
		result->start -= delta;
		result->size += delta;
	}

	/* adjust the end */
	delta = (result->start + result->size) & mask;
	if (delta)
		result->size += block_size - delta;
}

static int _aligned_io(struct device_area *where, char *buffer,
		       int should_write, dev_io_reason_t reason)
{
	char *bounce, *bounce_buf;
	unsigned int physical_block_size = 0;
	unsigned int block_size = 0;
	unsigned buffer_was_widened = 0;
	uintptr_t mask;
	struct device_area widened;
	int r = 0;

	if (!(where->dev->flags & DEV_REGULAR) &&
	    !dev_get_block_size(where->dev, &physical_block_size, &block_size))
		return_0;

	if (!block_size)
		block_size = lvm_getpagesize();
	mask = block_size - 1;

	_widen_region(block_size, where, &widened);

	/* Did we widen the buffer?  When writing, this means means read-modify-write. */
	if (where->size != widened.size || where->start != widened.start) {
		buffer_was_widened = 1;
		log_debug_io("Widening request for %" PRIu64 " bytes at %" PRIu64 " to %" PRIu64 " bytes at %" PRIu64 " on %s (for %s)",
			     where->size, (uint64_t) where->start, widened.size, (uint64_t) widened.start, dev_name(where->dev), _reason_text(reason));
	} else if (!((uintptr_t) buffer & mask))
		/* Perform the I/O directly. */
		return _io(where, buffer, should_write, reason);

	/* Allocate a bounce buffer with an extra block */
	if (!(bounce_buf = bounce = malloc((size_t) widened.size + block_size))) {
		log_error("Bounce buffer malloc failed");
		return 0;
	}

	/*
	 * Realign start of bounce buffer (using the extra sector)
	 */
	if (((uintptr_t) bounce) & mask)
		bounce = (char *) ((((uintptr_t) bounce) + mask) & ~mask);

	/* Do we need to read into the bounce buffer? */
	if ((!should_write || buffer_was_widened) &&
	    !_io(&widened, bounce, 0, reason)) {
		if (!should_write)
			goto_out;
		/* FIXME Handle errors properly! */
		/* FIXME pre-extend the file */
		memset(bounce, '\n', widened.size);
	}

	if (should_write) {
		memcpy(bounce + (where->start - widened.start), buffer,
		       (size_t) where->size);

		/* ... then we write */
		if (!(r = _io(&widened, bounce, 1, reason)))
			stack;
			
		goto out;
	}

	memcpy(buffer, bounce + (where->start - widened.start),
	       (size_t) where->size);

	r = 1;

out:
	free(bounce_buf);
	return r;
}

static int _dev_get_size_file(struct device *dev, uint64_t *size)
{
	const char *name = dev_name(dev);
	struct stat info;

	if (dev->size_seqno == _dev_size_seqno) {
		log_very_verbose("%s: using cached size %" PRIu64 " sectors",
				 name, dev->size);
		*size = dev->size;
		return 1;
	}

	if (stat(name, &info)) {
		log_sys_error("stat", name);
		return 0;
	}

	*size = info.st_size;
	*size >>= SECTOR_SHIFT;	/* Convert to sectors */
	dev->size = *size;
	dev->size_seqno = _dev_size_seqno;

	log_very_verbose("%s: size is %" PRIu64 " sectors", name, *size);

	return 1;
}

static int _dev_get_size_dev(struct device *dev, uint64_t *size)
{
	const char *name = dev_name(dev);
	int fd = dev->bcache_fd;
	int do_close = 0;

	if (dev->size_seqno == _dev_size_seqno) {
		log_very_verbose("%s: using cached size %" PRIu64 " sectors",
				 name, dev->size);
		*size = dev->size;
		return 1;
	}

	if (fd <= 0) {
		if (!dev_open_readonly(dev))
			return_0;
		fd = dev_fd(dev);
		do_close = 1;
	}

	if (ioctl(fd, BLKGETSIZE64, size) < 0) {
		log_sys_error("ioctl BLKGETSIZE64", name);
		if (do_close && !dev_close_immediate(dev))
			log_sys_error("close", name);
		return 0;
	}

	*size >>= BLKSIZE_SHIFT;	/* Convert to sectors */
	dev->size = *size;
	dev->size_seqno = _dev_size_seqno;

	log_very_verbose("%s: size is %" PRIu64 " sectors", name, *size);

	if (do_close && !dev_close_immediate(dev))
		log_sys_error("close", name);

	return 1;
}

static int _dev_read_ahead_dev(struct device *dev, uint32_t *read_ahead)
{
	long read_ahead_long;

	if (dev->read_ahead != -1) {
		*read_ahead = (uint32_t) dev->read_ahead;
		return 1;
	}

	if (!dev_open_readonly(dev))
		return_0;

	if (ioctl(dev->fd, BLKRAGET, &read_ahead_long) < 0) {
		log_sys_error("ioctl BLKRAGET", dev_name(dev));
		if (!dev_close_immediate(dev))
			stack;
		return 0;
	}

	*read_ahead = (uint32_t) read_ahead_long;
	dev->read_ahead = read_ahead_long;

	log_very_verbose("%s: read_ahead is %u sectors",
			 dev_name(dev), *read_ahead);

	if (!dev_close_immediate(dev))
		stack;

	return 1;
}

static int _dev_discard_blocks(struct device *dev, uint64_t offset_bytes, uint64_t size_bytes)
{
	uint64_t discard_range[2];

	if (!dev_open(dev))
		return_0;

	discard_range[0] = offset_bytes;
	discard_range[1] = size_bytes;

	log_debug_devs("Discarding %" PRIu64 " bytes offset %" PRIu64 " bytes on %s. %s",
		       size_bytes, offset_bytes, dev_name(dev),
		       test_mode() ? " (test mode - suppressed)" : "");

	if (!test_mode() && ioctl(dev->fd, BLKDISCARD, &discard_range) < 0) {
		log_error("%s: BLKDISCARD ioctl at offset %" PRIu64 " size %" PRIu64 " failed: %s.",
			  dev_name(dev), offset_bytes, size_bytes, strerror(errno));
		if (!dev_close_immediate(dev))
			stack;
		/* It doesn't matter if discard failed, so return success. */
		return 1;
	}

	if (!dev_close_immediate(dev))
		stack;

	return 1;
}

/*-----------------------------------------------------------------
 * Public functions
 *---------------------------------------------------------------*/
void dev_size_seqno_inc(void)
{
	_dev_size_seqno++;
}

int dev_get_size(struct device *dev, uint64_t *size)
{
	if (!dev)
		return 0;

	if ((dev->flags & DEV_REGULAR))
		return _dev_get_size_file(dev, size);

	return _dev_get_size_dev(dev, size);
}

int dev_get_read_ahead(struct device *dev, uint32_t *read_ahead)
{
	if (!dev)
		return 0;

	if (dev->flags & DEV_REGULAR) {
		*read_ahead = 0;
		return 1;
	}

	return _dev_read_ahead_dev(dev, read_ahead);
}

int dev_discard_blocks(struct device *dev, uint64_t offset_bytes, uint64_t size_bytes)
{
	if (!dev)
		return 0;

	if (dev->flags & DEV_REGULAR)
		return 1;

	return _dev_discard_blocks(dev, offset_bytes, size_bytes);
}

void dev_flush(struct device *dev)
{
	if (!(dev->flags & DEV_REGULAR) && ioctl(dev->fd, BLKFLSBUF, 0) >= 0)
		return;

	if (fsync(dev->fd) >= 0)
		return;

	sync();
}

int dev_open_flags(struct device *dev, int flags, int direct, int quiet)
{
	struct stat buf;
	const char *name;
	int need_excl = 0, need_rw = 0;

	if ((flags & O_ACCMODE) == O_RDWR)
		need_rw = 1;

	if ((flags & O_EXCL))
		need_excl = 1;

	if (dev->fd >= 0) {
		if (((dev->flags & DEV_OPENED_RW) || !need_rw) &&
		    ((dev->flags & DEV_OPENED_EXCL) || !need_excl)) {
			dev->open_count++;
			return 1;
		}

		if (dev->open_count && !need_excl)
			log_debug_devs("%s: Already opened read-only. Upgrading "
				       "to read-write.", dev_name(dev));

		/* dev_close_immediate will decrement this */
		dev->open_count++;

		if (!dev_close_immediate(dev))
			return_0;
		// FIXME: dev with DEV_ALLOCED is released
		// but code is referencing it
	}

	if (critical_section())
		/* FIXME Make this log_error */
		log_verbose("dev_open(%s) called while suspended",
			    dev_name(dev));

	if (!(name = dev_name_confirmed(dev, quiet)))
		return_0;

#ifdef O_DIRECT_SUPPORT
	if (direct) {
		if (!(dev->flags & DEV_O_DIRECT_TESTED))
			dev->flags |= DEV_O_DIRECT;

		if ((dev->flags & DEV_O_DIRECT))
			flags |= O_DIRECT;
	}
#endif

#ifdef O_NOATIME
	/* Don't update atime on device inodes */
	if (!(dev->flags & DEV_REGULAR) && !(dev->flags & DEV_NOT_O_NOATIME))
		flags |= O_NOATIME;
#endif

	if ((dev->fd = open(name, flags, 0777)) < 0) {
#ifdef O_NOATIME
		if ((errno == EPERM) && (flags & O_NOATIME)) {
			flags &= ~O_NOATIME;
			dev->flags |= DEV_NOT_O_NOATIME;
			if ((dev->fd = open(name, flags, 0777)) >= 0) {
				log_debug_devs("%s: Not using O_NOATIME", name);
				goto opened;
			}
		}
#endif

#ifdef O_DIRECT_SUPPORT
		if (direct && !(dev->flags & DEV_O_DIRECT_TESTED)) {
			flags &= ~O_DIRECT;
			if ((dev->fd = open(name, flags, 0777)) >= 0) {
				dev->flags &= ~DEV_O_DIRECT;
				log_debug_devs("%s: Not using O_DIRECT", name);
				goto opened;
			}
		}
#endif
		if (quiet)
			log_sys_debug("open", name);
		else
			log_sys_error("open", name);

		dev->flags |= DEV_OPEN_FAILURE;
		return 0;
	}

#ifdef O_DIRECT_SUPPORT
      opened:
	if (direct)
		dev->flags |= DEV_O_DIRECT_TESTED;
#endif
	dev->open_count++;
	dev->flags &= ~DEV_ACCESSED_W;

	if (need_rw)
		dev->flags |= DEV_OPENED_RW;
	else
		dev->flags &= ~DEV_OPENED_RW;

	if (need_excl)
		dev->flags |= DEV_OPENED_EXCL;
	else
		dev->flags &= ~DEV_OPENED_EXCL;

	if (!(dev->flags & DEV_REGULAR) &&
	    ((fstat(dev->fd, &buf) < 0) || (buf.st_rdev != dev->dev))) {
		log_error("%s: fstat failed: Has device name changed?", name);
		if (!dev_close_immediate(dev))
			stack;
		return 0;
	}

#ifndef O_DIRECT_SUPPORT
	if (!(dev->flags & DEV_REGULAR))
		dev_flush(dev);
#endif

	if ((flags & O_CREAT) && !(flags & O_TRUNC))
		dev->end = lseek(dev->fd, (off_t) 0, SEEK_END);

	log_debug_devs("Opened %s %s%s%s", dev_name(dev),
		       dev->flags & DEV_OPENED_RW ? "RW" : "RO",
		       dev->flags & DEV_OPENED_EXCL ? " O_EXCL" : "",
		       dev->flags & DEV_O_DIRECT ? " O_DIRECT" : "");

	dev->flags &= ~DEV_OPEN_FAILURE;
	return 1;
}

int dev_open_quiet(struct device *dev)
{
	return dev_open_flags(dev, O_RDWR, 1, 1);
}

int dev_open(struct device *dev)
{
	return dev_open_flags(dev, O_RDWR, 1, 0);
}

int dev_open_readonly(struct device *dev)
{
	return dev_open_flags(dev, O_RDONLY, 1, 0);
}

int dev_open_readonly_buffered(struct device *dev)
{
	return dev_open_flags(dev, O_RDONLY, 0, 0);
}

int dev_open_readonly_quiet(struct device *dev)
{
	return dev_open_flags(dev, O_RDONLY, 1, 1);
}

int dev_test_excl(struct device *dev)
{
	int flags = 0;

	flags |= O_EXCL;
	flags |= O_RDWR;

	return dev_open_flags(dev, flags, 1, 1);
}

static void _close(struct device *dev)
{
	if (close(dev->fd))
		log_sys_error("close", dev_name(dev));
	dev->fd = -1;
	dev->phys_block_size = -1;
	dev->block_size = -1;

	log_debug_devs("Closed %s", dev_name(dev));

	if (dev->flags & DEV_ALLOCED)
		dev_destroy_file(dev);
}

static int _dev_close(struct device *dev, int immediate)
{
	if (dev->fd < 0) {
		log_error("Attempt to close device '%s' "
			  "which is not open.", dev_name(dev));
		return 0;
	}

#ifndef O_DIRECT_SUPPORT
	if (dev->flags & DEV_ACCESSED_W)
		dev_flush(dev);
#endif

	if (dev->open_count > 0)
		dev->open_count--;

	if (immediate && dev->open_count)
		log_debug_devs("%s: Immediate close attempt while still referenced",
			       dev_name(dev));

	if (immediate || (dev->open_count < 1))
		_close(dev);

	return 1;
}

int dev_close(struct device *dev)
{
	return _dev_close(dev, 0);
}

int dev_close_immediate(struct device *dev)
{
	return _dev_close(dev, 1);
}

int dev_read(struct device *dev, uint64_t offset, size_t len, dev_io_reason_t reason, void *buffer)
{
	struct device_area where;
	int ret;

	if (!dev->open_count)
		return_0;

	where.dev = dev;
	where.start = offset;
	where.size = len;

	ret = _aligned_io(&where, buffer, 0, reason);

	return ret;
}

/*
 * Read from 'dev' into 'buf', possibly in 2 distinct regions, denoted
 * by (offset,len) and (offset2,len2).  Thus, the total size of
 * 'buf' should be len+len2.
 */
int dev_read_circular(struct device *dev, uint64_t offset, size_t len,
		      uint64_t offset2, size_t len2, dev_io_reason_t reason, char *buf)
{
	if (!dev_read(dev, offset, len, reason, buf)) {
		log_error("Read from %s failed", dev_name(dev));
		return 0;
	}

	/*
	 * The second region is optional, and allows for
	 * a circular buffer on the device.
	 */
	if (!len2)
		return 1;

	if (!dev_read(dev, offset2, len2, reason, buf + len)) {
		log_error("Circular read from %s failed",
			  dev_name(dev));
		return 0;
	}

	return 1;
}

/* FIXME If O_DIRECT can't extend file, dev_extend first; dev_truncate after.
 *       But fails if concurrent processes writing
 */

/* FIXME pre-extend the file */
int dev_append(struct device *dev, size_t len, dev_io_reason_t reason, char *buffer)
{
	int r;

	if (!dev->open_count)
		return_0;

	r = dev_write(dev, dev->end, len, reason, buffer);
	dev->end += (uint64_t) len;

#ifndef O_DIRECT_SUPPORT
	dev_flush(dev);
#endif
	return r;
}

int dev_write(struct device *dev, uint64_t offset, size_t len, dev_io_reason_t reason, void *buffer)
{
	struct device_area where;
	int ret;

	if (!dev->open_count)
		return_0;

	if (!len) {
		log_error(INTERNAL_ERROR "Attempted to write 0 bytes to %s at " FMTu64, dev_name(dev), offset);
		return 0;
	}

	where.dev = dev;
	where.start = offset;
	where.size = len;

	dev->flags |= DEV_ACCESSED_W;

	ret = _aligned_io(&where, buffer, 1, reason);

	return ret;
}

int dev_set(struct device *dev, uint64_t offset, size_t len, dev_io_reason_t reason, int value)
{
	size_t s;
	char buffer[4096] __attribute__((aligned(8)));

	if (!dev_open(dev))
		return_0;

	if ((offset % SECTOR_SIZE) || (len % SECTOR_SIZE))
		log_debug_devs("Wiping %s at %" PRIu64 " length %" PRIsize_t,
			       dev_name(dev), offset, len);
	else
		log_debug_devs("Wiping %s at sector %" PRIu64 " length %" PRIsize_t
			       " sectors", dev_name(dev), offset >> SECTOR_SHIFT,
			       len >> SECTOR_SHIFT);

	memset(buffer, value, sizeof(buffer));
	while (1) {
		s = len > sizeof(buffer) ? sizeof(buffer) : len;
		if (!dev_write(dev, offset, s, reason, buffer))
			break;

		len -= s;
		if (!len)
			break;

		offset += s;
	}

	dev->flags |= DEV_ACCESSED_W;

	if (!dev_close(dev))
		stack;

	return (len == 0);
}
