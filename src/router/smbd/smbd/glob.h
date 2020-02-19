/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __KSMBD_GLOB_H
#define __KSMBD_GLOB_H

#include <linux/ctype.h>
#include <linux/version.h>

#include "unicode.h"
#include "vfs_cache.h"
#include "smberr.h"

#define KSMBD_VERSION	"3.1.4"

/* @FIXME clean up this code */

extern int ksmbd_debugging;
extern int ksmbd_caseless_search;

#define DATA_STREAM	1
#define DIR_STREAM	2

#ifndef ksmbd_pr_fmt
#ifdef SUBMOD_NAME
#define ksmbd_pr_fmt(fmt)	"ksmbd: " SUBMOD_NAME ": " fmt
#else
#define ksmbd_pr_fmt(fmt)	"ksmbd: " fmt
#endif
#endif

#ifdef CONFIG_SMB_SERVER_DEBUGGING
#define ksmbd_debug(fmt, ...)					\
	do {							\
		if (ksmbd_debugging)				\
			pr_info(ksmbd_pr_fmt("%s:%d: " fmt),	\
				__func__,			\
				__LINE__,			\
				##__VA_ARGS__);			\
	} while (0)
#else
#define ksmbd_debug(fmt, ...) do { } while(0)
#endif

#define ksmbd_info(fmt, ...)					\
			pr_info(ksmbd_pr_fmt(fmt), ##__VA_ARGS__)

#define ksmbd_err(fmt, ...)					\
			pr_err(ksmbd_pr_fmt("%s:%d: " fmt),	\
				__func__,			\
				__LINE__,			\
				##__VA_ARGS__)

#define UNICODE_LEN(x)		((x) * 2)

/* @FIXME clean up this code */
/* @FIXME clean up this code */
/* @FIXME clean up this code */

/* ksmbd misc functions */
extern void ntstatus_to_dos(__le32 ntstatus, __u8 *eclass, __le16 *ecode);
#endif /* __KSMBD_GLOB_H */
