/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2024 LG Electronics Co., Ltd.
 */

#ifndef _UAPI_LINUX_NTFS_H
#define _UAPI_LINUX_NTFS_H
#include <linux/types.h>
#include <linux/ioctl.h>

/*
 * Shutdown the filesystem.
 */
#define FS_IOC_SHUTDOWN _IOR('X', 125, __u32)

/*
 * Flags for FS_IOC_SHUTDOWN
 */
#define FS_SHUTDOWN_FLAGS_DEFAULT	0x0
#define FS_SHUTDOWN_FLAGS_LOGFLUSH	0x1	/* flush log but not data*/
#define FS_SHUTDOWN_FLAGS_NOLOGFLUSH	0x2	/* don't flush log nor data */

#endif /* _UAPI_LINUX_NTFS_H */
