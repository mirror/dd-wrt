/*
 * Copyright (C) 2006 Red Hat, Inc. All rights reserved.
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

#include <unistd.h>
#include <fcntl.h>

#ifdef UDEV_SYNC_SUPPORT
#include <libudev.h>

struct udev *_udev;

int udev_init_library_context(void)
{
	if (_udev)
		udev_unref(_udev);

	if (!(_udev = udev_new())) {
		log_error("Failed to create udev library context.");
		return 0;
	}

	return 1;
}

void udev_fin_library_context(void)
{
	udev_unref(_udev);
	_udev = NULL;
}

int udev_is_running(void)
{
	struct udev_queue *udev_queue;
	int r;

	if (!_udev) {
		log_debug_activation("Udev library context not set.");
		goto bad;
	}

	if (!(udev_queue = udev_queue_new(_udev))) {
		log_debug_activation("Could not get udev state.");
		goto bad;
	}

	r = udev_queue_get_udev_is_active(udev_queue);
	udev_queue_unref(udev_queue);

	return r;

bad:
	log_debug_activation("Assuming udev is not running.");
	return 0;
}

void *udev_get_library_context(void)
{
	return _udev;
}

#else	/* UDEV_SYNC_SUPPORT */

int udev_init_library_context(void)
{
	return 1;
}

void *udev_get_library_context(void)
{
	return NULL;
}

void udev_fin_library_context(void)
{
}

int udev_is_running(void)
{
	return 0;
}

#endif

int lvm_getpagesize(void)
{
	return getpagesize();
}

int read_urandom(void *buf, size_t len)
{
	int fd;

	/* FIXME: we should stat here, and handle other cases */
	/* FIXME: use common _io() routine's open/read/close */
	if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
		log_sys_error("open", "read_urandom: /dev/urandom");
		return 0;
	}

	if (read(fd, buf, len) != (ssize_t) len) {
		log_sys_error("read", "read_urandom: /dev/urandom");
		if (close(fd))
			stack;
		return 0;
	}

	if (close(fd))
		stack;

	return 1;
}

/*
 * Return random integer in [0,max) interval
 *
 * The loop rejects numbers that come from an "incomplete" slice of the
 * RAND_MAX space.  Considering the number space [0, RAND_MAX] is divided
 * into some "max"-sized slices and at most a single smaller slice,
 * between [n*max, RAND_MAX] for suitable n, numbers from this last slice
 * are discarded because they could distort the distribution in favour of
 * smaller numbers.
 */
unsigned lvm_even_rand(unsigned *seed, unsigned max)
{
	unsigned r, ret;

	do {
		r = (unsigned) rand_r(seed);
		ret = r % max;
	} while (r - ret > RAND_MAX - max);

	return ret;
}

int clvmd_is_running(void)
{
#ifdef CLVMD_PIDFILE
	return dm_daemon_is_running(CLVMD_PIDFILE);
#else
	return 0;
#endif
}

int cmirrord_is_running(void)
{
#ifdef CMIRRORD_PIDFILE
	return dm_daemon_is_running(CMIRRORD_PIDFILE);
#else
	return 0;
#endif
}
