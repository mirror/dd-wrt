/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef _REF_PORT_CTRL_H_
#define _REF_PORT_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

int
qca_ar8327_sw_get_port_link(struct switch_dev *dev, int port,
			                        struct switch_port_link *link);

void
qca_ar8327_sw_mac_polling_task(struct switch_dev *dev);

void
dess_rgmii_sw_mac_polling_task(struct switch_dev *dev);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _REF_PORT_CTRL_H_ */

