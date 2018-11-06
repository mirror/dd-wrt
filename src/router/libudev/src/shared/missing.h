/***
  This file is part of eudev, forked from systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#pragma once

/* Missing glibc definitions to access certain kernel APIs */

#include <sys/resource.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/oom.h>

#include "macro.h"
#include "config.h"

#ifdef ARCH_MIPS
#include <asm/sgidefs.h>
#endif

#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif

/* If RLIMIT_RTTIME is not defined, then we cannot use RLIMIT_NLIMITS as is */
#define _RLIMIT_MAX (RLIMIT_RTTIME+1 > RLIMIT_NLIMITS ? RLIMIT_RTTIME+1 : RLIMIT_NLIMITS)

#ifndef __NR_getrandom
#  if defined __x86_64__
#    define __NR_getrandom 318
#  elif defined(__i386__)
#    define __NR_getrandom 355
#  elif defined(__arm__) || defined(__aarch64__)
#    define __NR_getrandom 384
#  elif defined(__ia64__)
#    define __NR_getrandom 1339
#  elif defined(__m68k__)
#    define __NR_getrandom 352
#  elif defined(__s390x__)
#    define __NR_getrandom 349
#  else
#    warning "__NR_getrandom unknown for your architecture"
#    define __NR_getrandom 0xffffffff
#  endif
#endif

#if !HAVE_DECL_GETRANDOM
static inline int getrandom(void *buffer, size_t count, unsigned flags) {
        return syscall(__NR_getrandom, buffer, count, flags);
}
#endif

#ifndef GRND_NONBLOCK
#define GRND_NONBLOCK 0x0001
#endif

#ifndef GRND_RANDOM
#define GRND_RANDOM 0x0002
#endif

#ifndef BTRFS_IOCTL_MAGIC
#define BTRFS_IOCTL_MAGIC 0x94
#endif

#ifndef BTRFS_PATH_NAME_MAX
#define BTRFS_PATH_NAME_MAX 4087
#endif

#ifndef HAVE_LINUX_BTRFS_H
struct btrfs_ioctl_vol_args {
        int64_t fd;
        char name[BTRFS_PATH_NAME_MAX + 1];
};
#endif

#ifndef BTRFS_IOC_DEVICES_READY
#define BTRFS_IOC_DEVICES_READY _IOR(BTRFS_IOCTL_MAGIC, 39, \
                                 struct btrfs_ioctl_vol_args)
#endif

#ifndef MS_MOVE
#define MS_MOVE 8192
#endif

#ifndef MS_PRIVATE
#define MS_PRIVATE  (1 << 18)
#endif

#if !HAVE_DECL_GETTID
static inline pid_t gettid(void) {
        return (pid_t) syscall(SYS_gettid);
}
#endif

#ifndef MS_REC
#define MS_REC 16384
#endif

#ifndef MAX_HANDLE_SZ
#define MAX_HANDLE_SZ 128
#endif

#ifndef __NR_name_to_handle_at
#  if defined(__x86_64__)
#    define __NR_name_to_handle_at 303
#  elif defined(__i386__)
#    define __NR_name_to_handle_at 341
#  elif defined(__arm__)
#    define __NR_name_to_handle_at 370
#  elif defined(__powerpc__)
#    define __NR_name_to_handle_at 345
#  else
#    error "__NR_name_to_handle_at is not defined"
#  endif
#endif

#if !HAVE_DECL_NAME_TO_HANDLE_AT
struct file_handle {
        unsigned int handle_bytes;
        int handle_type;
        unsigned char f_handle[0];
};

static inline int name_to_handle_at(int fd, const char *name, struct file_handle *handle, int *mnt_id, int flags) {
        return syscall(__NR_name_to_handle_at, fd, name, handle, mnt_id, flags);
}
#endif

#ifndef INPUT_PROP_POINTING_STICK
#define INPUT_PROP_POINTING_STICK 0x05
#endif

#ifndef INPUT_PROP_ACCELEROMETER
#define INPUT_PROP_ACCELEROMETER  0x06
#endif

#ifndef O_PATH
#define O_PATH    010000000
#endif

#ifndef AT_EMPTY_PATH
#define AT_EMPTY_PATH 0x1000
#endif

#if !HAVE_DECL_STRNDUPA
#define strndupa(s, n) \
  ({ \
    const char *__old = (s); \
    size_t __len = strnlen(__old, (n)); \
    char *__new = (char *)alloca(__len + 1); \
    __new[__len] = '\0'; \
    (char *)memcpy(__new, __old, __len); \
  })
#endif
