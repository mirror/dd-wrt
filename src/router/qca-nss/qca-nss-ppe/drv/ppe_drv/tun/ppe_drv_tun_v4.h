/*
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

#ifndef _PPE_DRV_TUN_V4_H_
#define _PPE_DRV_TUN_V4_H_

ppe_drv_ret_t ppe_drv_v4_tun_add_ce_notify(struct ppe_drv_v4_rule_create *create);
ppe_drv_ret_t ppe_drv_v4_tun_del_ce_notify(struct ppe_drv_v4_rule_destroy *destroy);
struct ppe_drv_v4_conn *ppe_drv_v4_conn_tun_conn_get(struct ppe_drv_v4_5tuple *tuple);
void ppe_drv_tun_v4_parse_l2_hdr(struct ppe_drv_v4_rule_create *create, struct ppe_drv_v4_conn *cn,
				 struct ppe_drv_tun_cmn_ctx_l2 *l2);
ppe_drv_ret_t ppe_drv_v4_tun_add_ce_validate(void *vcreate_rule, struct ppe_drv_v4_conn *cn);
ppe_drv_ret_t ppe_drv_v4_tun_del_ce_validate(void *vdestroy_rule, struct ppe_drv_v4_conn_sync **cns, struct ppe_drv_v4_conn **cn_v4);
bool ppe_drv_v4_tun_allow_tunnel_create(struct ppe_drv_v4_rule_create *create);
bool ppe_drv_v4_tun_allow_tunnel_destroy(struct ppe_drv_v4_rule_destroy *destroy);
bool ppe_drv_tun_v4_fse_entry_add(void *vcreate_rule, struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_v4_conn_flow *pcr);
bool ppe_drv_tun_v4_fse_entry_del(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_v4_conn_flow *pcr);
#endif /* _PPE_DRV_TUN_V4_H_ */
