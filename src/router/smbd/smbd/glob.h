/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_GLOB_H
#define __SMBD_GLOB_H

#include <linux/ctype.h>
#include <linux/version.h>

#include "unicode.h"
#include "vfs_cache.h"
#include "smberr.h"

#define SMBD_VERSION	"3.0.1"

/* @FIXME clean up this code */

extern int smbd_debugging;
extern int smbd_caseless_search;

#define DATA_STREAM	1
#define DIR_STREAM	2

#ifndef smbd_pr_fmt
#ifdef SUBMOD_NAME
#define smbd_pr_fmt(fmt)	"ksmbd: " SUBMOD_NAME ": " fmt
#else
#define smbd_pr_fmt(fmt)	"ksmbd: " fmt
#endif
#endif

#ifdef CONFIG_SMB_SERVER_DEBUGGING
#define smbd_debug(fmt, ...)					\
	do {							\
		if (smbd_debugging)				\
			pr_info(smbd_pr_fmt("%s:%d: " fmt),	\
				__func__,			\
				__LINE__,			\
				##__VA_ARGS__);			\
	} while (0)
#else
#define smbd_debug(fmt, ...) do { } while(0)
#endif

#define smbd_info(fmt, ...)					\
			pr_info(smbd_pr_fmt(fmt), ##__VA_ARGS__)

#define smbd_err(fmt, ...)					\
			pr_err(smbd_pr_fmt("%s:%d: " fmt),	\
				__func__,			\
				__LINE__,			\
				##__VA_ARGS__)

#define UNICODE_LEN(x)		((x) * 2)

/* @FIXME clean up this code */
/* @FIXME clean up this code */
/* @FIXME clean up this code */

/* smbd misc functions */
extern void ntstatus_to_dos(__u32 ntstatus, __u8 *eclass, __u16 *ecode);
#endif /* __SMBD_GLOB_H */
