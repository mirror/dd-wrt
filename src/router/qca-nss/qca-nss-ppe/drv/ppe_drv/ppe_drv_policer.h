/*
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

#define PPE_DRV_PORT_POLICER_MAX 8
#ifdef NSS_PPE_IPQ53XX
#define PPE_DRV_ACL_POLICER_MAX 128
#else
#define PPE_DRV_ACL_POLICER_MAX 512
#endif

/*
 * ppe_drv_policer_port
 *	 Policer Port interface information
 */
struct ppe_drv_policer_port {
	uint16_t index;				/* Port policer index */
	bool in_use;				/* Entry in use */
};

/*
 * ppe_drv_policer_acl
 *	 Policer ACL interface information
 */
struct ppe_drv_policer_acl {
	uint16_t acl_index;			/* Policer index */
	bool in_use;				/* Entry in use */
};

/*
 * ppe_drv_policer
 *	Global context
 */
struct ppe_drv_policer_ctx {
	struct ppe_drv_policer_port port_pol[PPE_DRV_PORT_POLICER_MAX];
	struct ppe_drv_policer_acl acl_pol[PPE_DRV_ACL_POLICER_MAX];
	ppe_drv_policer_flow_callback_t flow_add_cb;
	ppe_drv_policer_flow_callback_t flow_del_cb;
	void *flow_app_data;
	int user2hw_map[PPE_DRV_ACL_POLICER_MAX];
};

/*
 * ppe_drv_policer_port_get_index
 *	Return port policer index
 */
static inline uint16_t ppe_drv_policer_port_get_index(struct ppe_drv_policer_port *pol)
{
	return pol->index;
}

/*
 * ppe_drv_policer_acl_get_index
 *	Return ACL policer index
 */
static inline uint16_t ppe_drv_policer_acl_get_index(struct ppe_drv_policer_acl *pol)
{
	return pol->acl_index;
}

int ppe_drv_policer_user2hw_id(int index);

uint16_t ppe_drv_policer_acl_get_index(struct ppe_drv_policer_acl *pol);
uint16_t ppe_drv_policer_port_get_index(struct ppe_drv_policer_port *pol);

void ppe_drv_policer_entries_free(struct ppe_drv_policer_ctx *pol);
struct ppe_drv_policer_ctx *ppe_drv_policer_entries_alloc(void);

