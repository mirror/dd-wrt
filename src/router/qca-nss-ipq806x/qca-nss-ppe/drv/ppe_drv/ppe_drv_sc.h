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

/*
 * Inline EIP hardware services.
 */
#define PPE_DRV_EIP_HWSERVICE_IPSEC 0x3

/*
 * ppe_drv_sc_in_l2_dir_type
 *	Direction types for IN_L2_SERVICE_TBL (impacts how Rx pre-hdr is populated)
 */
enum ppe_drv_sc_in_l2_dir_type {
	PPE_DRV_SC_IN_L2_DIR_DST,	/* Treat port specified in IN_L2_SERVICE_TBL as destination port */
	PPE_DRV_SC_IN_L2_DIR_SRC	/* Treat port specified in IN_L2_SERVICE_TBL as source port */
};

/*
 * ppe_drv_sc_in_l2_off_sel
 *	Offset selector for IN_L2_SERVICE_TBL
 */
enum ppe_drv_sc_in_l2_off_sel {
	PPE_DRV_SC_IN_L2_OFF_L3,	/* Set payload offset to L3 header in metadata to EIP */
	PPE_DRV_SC_IN_L2_OFF_L4		/* Set payload offset to L4 header in metadata to EIP */
};

/*
 * ppe_drv_sc_vp_info
 * 	VP info for a service code
 */
struct ppe_drv_sc_vp_info {
	ppe_drv_sc_callback_t cb;	/* sc registered callback of VP*/
	void *app_data;			/* Associated app data */
};

/*
 * ppe_drv_sc
 *	Instance structure for service code management
 */
struct ppe_drv_sc {
	struct ppe_drv_sc_vp_info __rcu *vp_info[PPE_DRV_VIRTUAL_MAX];
					/* Per VP info for the service code */
	ppe_drv_sc_callback_t cb;	/* Per sc registered callback */
	void *app_data;			/* Associated app data */
};

/*
 * ppe_drv_sc_check_and_set()
 *	Check if service code is already set before setting a new service code.
 */
static inline bool ppe_drv_sc_check_and_set(ppe_drv_sc_t *scp, ppe_drv_sc_t sc)
{
	return (*scp == PPE_DRV_SC_NONE) ? (*scp = sc) : false;
}

void ppe_drv_sc_ucast_queue_set(ppe_drv_sc_t sc, uint8_t queue_id, uint8_t profile_id);
void ppe_drv_sc_entries_free(struct ppe_drv_sc *sc);
struct ppe_drv_sc *ppe_drv_sc_entries_alloc(void);
