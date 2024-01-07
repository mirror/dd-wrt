/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@kernel.org>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __KSMBD_GLOB_H
#define __KSMBD_GLOB_H

#include <linux/ctype.h>
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#include "unicode.h"
#include "vfs_cache.h"

#define KSMBD_VERSION	"3.4.8"


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
/**
 * strchrnul - Find and return a character in a string, or end of string
 * @s: The string to be searched
 * @c: The character to search for
 *
 * Returns pointer to first occurrence of 'c' in s. If c is not found, then
 * return a pointer to the null byte at the end of s.
 */
static char *strchrnul(const char *s, int c)
{
	while (*s && *s != (char)c)
		s++;
	return (char *)s;
}

static inline int can_lookup(struct inode *inode)
{
	if (likely(inode->i_opflags & IOP_LOOKUP))
		return 1;
	if (likely(!inode->i_op->lookup))
		return 0;

	/* We do this once for the lifetime of the inode */
	spin_lock(&inode->i_lock);
	inode->i_opflags |= IOP_LOOKUP;
	spin_unlock(&inode->i_lock);
	return 1;
}


static inline bool d_can_lookup(const struct dentry *dentry)
{
	return can_lookup(dentry->d_inode);
}

#endif

#define KSMBD_DEBUG_SMB		BIT(0)
#define KSMBD_DEBUG_AUTH	BIT(1)
#define KSMBD_DEBUG_VFS		BIT(2)
#define KSMBD_DEBUG_OPLOCK      BIT(3)
#define KSMBD_DEBUG_IPC         BIT(4)
#define KSMBD_DEBUG_CONN        BIT(5)
#define KSMBD_DEBUG_RDMA        BIT(6)
#define KSMBD_DEBUG_ALL         (KSMBD_DEBUG_SMB | KSMBD_DEBUG_AUTH |	\
				KSMBD_DEBUG_VFS | KSMBD_DEBUG_OPLOCK |	\
				KSMBD_DEBUG_IPC | KSMBD_DEBUG_CONN |	\
				KSMBD_DEBUG_RDMA)

#ifdef pr_fmt
#undef pr_fmt
#endif

#ifdef SUBMOD_NAME
#define pr_fmt(fmt)	"ksmbd: " SUBMOD_NAME ": " fmt
#else
#define pr_fmt(fmt)	"ksmbd: " fmt
#endif

#if 0
#define ksmbd_debug(type, fmt, ...)				\
	do {							\
		if (ksmbd_debug_types & KSMBD_DEBUG_##type)	\
			pr_info(fmt, ##__VA_ARGS__);		\
	} while (0)
#else
#define ksmbd_debug(type, fmt, ...)				\
	do {							\
	} while (0)
#endif
#define UNICODE_LEN(x)		((x) * 2)

#ifdef CONFIG_SMB_INSECURE_SERVER
/* ksmbd misc functions */
static void ntstatus_to_dos(__le32 ntstatus, __u8 *eclass, __le16 *ecode);
#endif

#ifndef LOOKUP_NO_SYMLINKS
#define LOOKUP_NO_SYMLINKS 0
#endif

#endif /* __KSMBD_GLOB_H */
