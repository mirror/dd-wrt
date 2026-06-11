/*
 * Copyright (c) 2014, 2016, 2019 The Linux Foundation. All rights reserved.
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

#ifndef __CLI_PARSE_LIB_H__
#define __CLI_PARSE_LIB_H__

#include <sys_base.h>
#include "array.h"

#define CLI_MAC_STR_LEN_MAX     14	/* aaaa.bbbb.cccc */
#define CLI_MAC_STR_LEN_MIN     5	/* 1.1.1 */

#define CLI_IPV4_STR_LEN_MAX    15	/* aaa.bbb.ccc.ddd */
#define CLI_IPV4_STR_LEN_MIN    7	/* a.b.c.d */

#define CLI_IPV6_STR_LEN_MAX    39	/* xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx */
#define CLI_IPV6_STR_LEN_MIN    2	/* :: */

#define cli_str_2_enable(str) (strncmp(str, "enable", 1) == 0) ? 1 : 0
#define cli_enable_2_str(enable) enable? "enable" : "disable"

typedef struct {
	sa_u32_t flag;
	char *str;
	int cmp;
} flag_desc_t;

int cli_str_2_hex(const char *str, sa_u32_t *pHex);

int cli_str_2_long_hex(const char *str, sa_u64_t *pU64);

int cli_str_2_mac(const char *str, sa_u8_t *mac);

int cli_mac_2_str(const sa_u8_t *mac, char *str, unsigned int maxLen);

int cli_u8_array_2_str(const sa_u8_t *array, int arrayNum, char *str,
		       int strLen);
/*
int cli_str_2_u8_array(const char *str, sa_u8_t *array, int *pArrayNum);
*/
int cli_str_2_ipv4(const char *str, sa_u8_t *ip);

int cli_str_2_ipv6(const char *str, sa_u8_t *ip);

int cli_str_2_list(const char *str, const sa_u32_t min, const sa_u32_t max,
		   ARRAY_T *pPortArray);

int cli_str_2_sci(const char *str, sa_u8_t *sci);

int cli_sci_2_str(const sa_u8_t *sci, char *str, unsigned int maxLen);

int cli_str_2_sak(const char *str, sa_u8_t *key);

int cli_sak_2_str(const sa_u8_t *key, char *str, int strLen);

#endif
