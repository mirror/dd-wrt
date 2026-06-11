/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef _DAL_UTILITY_H_
#define _DAL_UTILITY_H_


enum access_layer_t {
	PHY_MMD3         = 0,
	PHY_MMD7         = 1,
	ETH_MAC          = 2,
};

#define REG_FIELD_READ(port, reg_layer, reg_addr, field, pFieldValue) \
	reg_acces_field_read(port, reg_layer, reg_addr, \
			field##_OFFSET, field##_LEN, (u16 *)pFieldValue)

#define REG_FIELD_WRITE(port, reg_layer, reg_addr, field, fieldValue) \
	reg_acces_field_write(port, reg_layer, reg_addr, \
			field##_OFFSET, field##_LEN, (u16)fieldValue)


g_error_t reg_access_write(
	struct macsec_port   *port,
	enum access_layer_t  reg_layer,
	u32                  reg_addr,
	u16                  val16
);

g_error_t reg_access_read(
	struct macsec_port   *port,
	enum access_layer_t  reg_layer,
	u32                  reg_addr,
	u16                  *p_val16
);

g_error_t reg_acces_field_read(
	struct macsec_port   *port,
	enum access_layer_t  reg_layer,
	u32                  reg_addr,
	u8                   offset,
	u8                   length,
	u16                  *p_val16
);

g_error_t reg_acces_field_write(
	struct macsec_port   *port,
	enum access_layer_t  reg_layer,
	u32                  reg_addr,
	u8                   offset,
	u8                   length,
	u16                  val16
);

g_error_t reg_access_mask_write(
	struct macsec_port   *port,
	enum access_layer_t  reg_layer,
	u32                  reg_addr,
	u16                  mask,
	u16                  val16
);



#endif /* _DAL_UTILITY_H_ */

