/*
 * Copyright (c) 2012, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _REF_UCI_H_
#define _REF_UCI_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */


#if defined(IN_SWCONFIG)
int
qca_ar8327_sw_switch_ext(struct switch_dev *dev,
			 	const struct switch_attr *attr,
			 	struct switch_val *val);

int parse_uci_option(struct switch_val *val, const char *option_names[], const int length);

#if defined(IN_TUNNEL)
int parse_tunnel_udfprofileentry(a_uint32_t dev_id, struct switch_val *val);
int parse_tunnel_udfprofilecfg(struct switch_val *val);
#endif

#if defined(IN_MAPT)
int parse_mapt(const char *command_name, struct switch_val *val);
#endif

#if defined(IN_VPORT)
int parse_vport(const char *command_name, struct switch_val *val);
#endif

#if defined(IN_ATHTAG)
int parse_athtag(const char *command_name, struct switch_val *val);
#endif
#endif

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _REF_FDB_H_ */

