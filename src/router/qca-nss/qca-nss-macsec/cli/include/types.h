/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __TYPES_H__
#define __TYPES_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

/*
 * Basic data types
 */
typedef unsigned long long sa_u64_t;
typedef long long sa_i64_t;
typedef unsigned int sa_u32_t;
typedef int sa_i32_t;
typedef unsigned short sa_u16_t;
typedef short sa_i16_t;
typedef unsigned char sa_u8_t;
typedef signed char sa_i8_t;

typedef char sa_ch_t;
typedef long sa_il_t;
typedef unsigned long sa_ul_t;
typedef char sa_bool_t;

enum {
	CLI_ERROR_OK = 0,
	CLI_ERROR_RESOURCE = 1,
	CLI_ERROR_PARAM = 2,
	CLI_ERROR_NOT_FOUND = 3,
	CLI_ERROR_CONFLICT = 4,
	CLI_ERROR_TIMEOUT = 5,
	CLI_ERROR_NOT_SUPPORT = 6,
	CLI_ERROR_ERROR = 0xffffffff
};

#define SA_FALSE 0
#define SA_TRUE  1

#endif /* __TYPES_H__ */
