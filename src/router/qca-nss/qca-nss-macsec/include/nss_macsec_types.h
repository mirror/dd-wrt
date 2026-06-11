/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __NSS_MACSEC_TYPES_H__
#define __NSS_MACSEC_TYPES_H__

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <net/sock.h>
#include <net/netlink.h>

#define osal_print printk

#else
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define osal_print printf

#endif


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

typedef enum {
	ERROR_OK = 0,
	ERROR_RESOURCE = 1,
	ERROR_PARAM = 2,
	ERROR_NOT_FOUND = 3,
	ERROR_CONFLICT = 4,
	ERROR_TIMEOUT = 5,
	ERROR_NOT_SUPPORT = 6,
	ERROR_ERROR = 0xffffffff
} g_error_t;

#ifndef OK
#define OK                   ERROR_OK
#endif

#ifndef ERROR
#define ERROR                ERROR_ERROR
#endif

#ifndef NULL
#define NULL                ((void *) 0)
#endif

#define SHR_RET_ON_ERR(f) \
    do{ g_error_t shr_rv = (f); \
        if ( shr_rv != OK) { \
            return shr_rv; \
        } \
    }while(0)

#define SHR_PARAM_CHECK(f) \
	do{ \
		if (!(f)) { \
			osal_print("%s[%d]: error parameter!\r\n", __FUNCTION__, __LINE__); \
			return ERROR_PARAM; \
		} \
	}while(0)

#endif /* __TYPES_H__ */
