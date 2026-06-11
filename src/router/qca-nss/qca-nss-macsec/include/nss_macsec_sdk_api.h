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

#ifndef _NSS_MACSEC_SDK_API_H_
#define _NSS_MACSEC_SDK_API_H_

#define NETLINK_SDK       30
#define SDK_CALL_MSG      0x88

#define SDK_MSG_VER   0x1

#define SDK_FAL_CMD  0x1
#define SDK_DAL_CMD  0x2

enum {
	SDK_RET_SUCCESS = 0,
	SDK_RET_PARAM_ERR,
	SDK_RET_NOT_SUPPORT,
	SDK_RET_UNKOWN_ERR
};

struct sdk_msg_header {
	unsigned short version;
	unsigned short cmd_type;
	unsigned short sub_type;
	unsigned short reserved0;
	unsigned int buf_len;	/* not include this header */
	unsigned int data_len;
	unsigned short ret;
	unsigned short reserved1;
} __attribute__ ((packed));

#endif
