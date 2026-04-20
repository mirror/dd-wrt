/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PPE_DRV_TUN_V6_H_
#define _PPE_DRV_TUN_V6_H_

ppe_drv_ret_t ppe_drv_v6_tun_add_ce_notify(struct ppe_drv_v6_rule_create *create);
ppe_drv_ret_t ppe_drv_v6_tun_del_ce_notify(struct ppe_drv_v6_rule_destroy *destroy);
struct ppe_drv_v6_conn *ppe_drv_v6_conn_tun_conn_get(struct ppe_drv_v6_5tuple *tuple);
void ppe_drv_tun_v6_parse_l2_hdr(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn *cn,
				 struct ppe_drv_tun_cmn_ctx_l2 *l2);
ppe_drv_ret_t ppe_drv_v6_tun_add_ce_validate(void *vcreate_rule, struct ppe_drv_v6_conn *cn);
ppe_drv_ret_t ppe_drv_v6_tun_del_ce_validate(void *vdestroy_rule, struct ppe_drv_v6_conn_sync **cns, struct ppe_drv_v6_conn **cn_v6);
bool ppe_drv_v6_tun_allow_tunnel_create(struct ppe_drv_v6_rule_create *create);
bool ppe_drv_tun_v6_get_conn_flow(struct ppe_drv_v6_5tuple *tuple, struct ppe_drv_v6_conn_flow **pcf, struct ppe_drv_v6_conn_flow **pcr);
bool ppe_drv_tun_v6_fse_entry_add(void *vcreate_rule, struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_v6_conn_flow *pcr);
bool ppe_drv_tun_v6_fse_entry_del(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_v6_conn_flow *pcr);
#endif /* _PPE_DRV_TUN_V6_H_ */
