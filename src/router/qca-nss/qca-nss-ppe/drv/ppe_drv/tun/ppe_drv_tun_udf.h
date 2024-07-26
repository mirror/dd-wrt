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
#ifndef _PPE_DRV_TUN_UDF_H_
#define _PPE_DRV_TUN_UDF_H_

#define PPE_DRV_TUN_UDF_PROFILE_ID_MAX	8 /* Max UDF profiles */
#define PPE_DRV_TUN_UDF_MAX	4 /* Max number of UDF feilds within a UDF ID */

#define PPE_DRV_TUN_L2TP_TUNNEL_ID_UDF	0 /*UDF index for l2tp tunnel ID  */
#define PPE_DRV_TUN_L2TP_SESSION_ID_UDF	1 /* UDF index for l2tp session ID */

/*
 * ppe_drv_tun_udf_l3_type
 *	tun udf l3 type
 */
enum ppe_drv_tun_udf_l3_type {
	PPE_DRV_TUN_UDF_L3_TYPE_OTHERS = 0,	/* L3 type others */
	PPE_DRV_TUN_UDF_L3_TYPE_IPV4,		/* L3 type IPv4 */
	PPE_DRV_TUN_UDF_L3_TYPE_ARP,		/* L3 type ARP */
	PPE_DRV_TUN_UDF_L3_TYPE_IPV6,		/* L3 type IPv6 */
};

/*
 * ppe_drv_tun_udf_l4_type
 *	tun l4 type
 */
enum ppe_drv_tun_udf_l4_type {
	PPE_DRV_TUN_UDF_L4_TYPE_OTHERS = 0,	/* L4 type others */
	PPE_DRV_TUN_UDF_L4_TYPE_TCP,		/* L4 type TCP */
	PPE_DRV_TUN_UDF_L4_TYPE_UDP,		/* L4 type UDP */
	PPE_DRV_TUN_UDF_L4_TYPE_UDP_LITE,	/* L4 type UDP LITE */
	PPE_DRV_TUN_UDF_L4_TYPE_ICMP,		/* L4 type ICMP */
	PPE_DRV_TUN_UDF_L4_TYPE_GRE,		/* L4 type GRE */
};

/*
 * ppe_drv_tun_udf_offset_type
 *      tun udf offset type
 */
enum ppe_drv_tun_udf_offset_type {
	PPE_DRV_TUN_UDF_OFFSET_TYPE_L2 = 0,	/* offset from L2 */
	PPE_DRV_TUN_UDF_OFFSET_TYPE_L3,		/* offset from L3 */
	PPE_DRV_TUN_UDF_OFFSET_TYPE_L4,		/* offset from L4 */
	PPE_DRV_TUN_UDF_OFFSET_TYPE_OVERLAY,	/* offset from overlay */
	PPE_DRV_TUN_UDF_OFFSET_TYPE_PROGRAM,	/* offset from program header */
	PPE_DRV_TUN_UDF_OFFSET_TYPE_PAYLOAD,	/* offset from payload */
};

/*
 * ppe_drv_tun_udf_data
 *	tunnel udf data
 */
struct ppe_drv_tun_udf_data {
	enum ppe_drv_tun_udf_offset_type offset_type;	/* offset type */
	uint32_t offset;				/* offset value */
};

/*
 * ppe_drv_tun_udf_profile
 *	tunnel udf profile
 */
struct ppe_drv_tun_udf_profile {
	bool l3_match;					/* l3 match enable */
	enum ppe_drv_tun_udf_l3_type l3_type;	/* L3 type to match */
	bool l4_match;					/* L4 match enable */
	enum ppe_drv_tun_udf_l4_type l4_type;	/* L4 type to match */
	bool program_match;				/* program type match enable */
	uint8_t program_type;				/* program type */
	uint8_t udf_bitmap;				/* udf enable bitmap */
	struct ppe_drv_tun_udf_data udf[PPE_DRV_TUN_UDF_MAX];	/* UDF data */
};

/*
 * ppe_drv_tun_udf
 *      tunnel udf structure
 */
struct ppe_drv_tun_udf {
	uint8_t udf_index;		/* Index */
	struct kref ref;	/* Reference Counter */
	struct ppe_drv_tun_udf_profile udf;
};

/*
 * ppe_drv_tun_udf_bitmask_check()
 * 	check tunnel udf bitmask
 */
static inline bool ppe_drv_tun_udf_bitmask_check(struct ppe_drv_tun_udf_profile *udf, uint8_t flag)
{
	return !!(udf->udf_bitmap & (1 << flag));
}

/*
 * ppe_drv_tun_udf_bitmask_set()
 * 	set tunnel udf bitmask
 */
static inline void ppe_drv_tun_udf_bitmask_set(struct ppe_drv_tun_udf_profile *udf, uint8_t flag)
{
	udf->udf_bitmap |= (1 << flag);
}

/*
 * ppe_drv_tun_udf_bitmask_clear()
 * 	clear tunnel udf bitmask
 */
static inline void ppe_drv_tun_udf_bitmask_clear(struct ppe_drv_tun_udf_profile *udf, uint8_t flag)
{
	udf->udf_bitmap &= ~(1 << flag);
}

struct ppe_drv_tun_udf *ppe_drv_tun_udf_alloc(struct ppe_drv *p);
void ppe_drv_tun_udf_free(struct ppe_drv_tun_udf *pgm_udf);
bool ppe_drv_tun_udf_entry_dref(struct ppe_drv_tun_udf  *pgm_udf);
struct ppe_drv_tun_udf *ppe_drv_tun_udf_entry_configure(struct ppe_drv_tun_udf_profile *udf_pf);
struct ppe_drv_tun_udf *ppe_drv_tun_udf_entry_alloc(struct ppe_drv *p);
#endif /* _PPE_DRV_TUN_UDF_H_ */