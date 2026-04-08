/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#ifndef _UAPI_LINUX_NTFS_H
#define _UAPI_LINUX_NTFS_H
#include <linux/types.h>
#include <linux/ioctl.h>

/*
 * ntfs-specific ioctl commands
 */
#define NTFS_IOC_SHUTDOWN _IOR('X', 125, __u32)

/*
 * Flags used by NTFS_IOC_SHUTDOWN
 */
#define NTFS_GOING_DOWN_DEFAULT        0x0     /* default with full sync */
#define NTFS_GOING_DOWN_FULLSYNC       0x1     /* going down with full sync*/
#define NTFS_GOING_DOWN_NOSYNC         0x2     /* going down */

#endif /* _UAPI_LINUX_NTFS_H */
