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
#ifndef _PPE_DRV_TUN_PRGM_PRSR_H_
#define _PPE_DRV_TUN_PRGM_PRSR_H_

#include <fal/fal_tunnel_program.h>
#include "ppe_drv_tun_udf.h"
#include "ppe_drv_tun_l2tp.h"
#include "ppe_drv_tun_prgm_prsr_gre.h"

/*
 *  Get Tunnel Type from Program Parser entry type
 */
#define PPE_DRV_TUN_GET_TUNNEL_TYPE_FROM_PGM_TYPE(a) (FAL_TUNNEL_TYPE_PROGRAM0 + (a))
#define PPE_DRV_TUN_PRGM_PRSR_MAX	6 /* Program Parser MAX value */
#define PPE_DRV_TUN_PRGM_UDF_MAX	3 /* Program UDF MAC value */

/*
 * decap key bitmap
 */
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_SRC_IP		0
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_DEST_IP		1
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_L4_PROTO	2
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_SPORT		3
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_DPORT		4
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_TLINFO		5
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_UDF0		6
#define PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_UDF1		7

/*
 * Program UDF Bitmap
 */
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF0		0
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF1		1
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF2		2
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_INVERSE	3

/*
 * Program UDF Action fields
 */
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_INNER_HDR_TYPE	0
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_HDR_LEN		1
#define PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_EXCEPTION_EN		2

/*
 * Macro to convert outer Program Header type to FAL type
 */
#define PPE_DRV_TUN_PRGM_PRSR_OUT_HDR_TO_FAL_OUT_HDR(a) (((a == 0) ? (0) : (a+1)))

/*
 * ppe_drv_tun_prgm_prsr_mode
 *	Tunnel program parser mode
 */
enum ppe_drv_tun_prgm_prsr_mode {
	PPE_DRV_TUN_PROGRAM_MODE_NONE,	/* Program parser not configured*/
	PPE_DRV_TUN_PROGRAM_MODE_GRE,		/* Program parser mode GRE */
	PPE_DRV_TUN_PROGRAM_MODE_L2TP_V2,	/* Program parser mode L2TP */
};

/*
 * ppe_drv_tun_prgm_prsr_ip_ver
 *	Outer header IP version
 */
enum ppe_drv_tun_prgm_prsr_ip_ver {
	PPE_DRV_TUN_PRGM_PRSR_IP_VER_IPV4 = 1,	/* Outer IP version IPv4  */
	PPE_DRV_TUN_PRGM_PRSR_IP_VER_IPV6,	/* Outer IP version IPv6 */
	PPE_DRV_TUN_PRGM_PRSR_IP_VER_IPV4_IPV6,	/* Outer IP version IPv4/IPv6 */
};

/*
 * ppe_drv_tun_prgm_prsr_outer_hdr_t
 *	Outer header Type
 */
enum ppe_drv_tun_prgm_prsr_outer_hdr_t {
	PPE_DRV_TUN_PRGM_PRSR_OHDR_ETH = 0,	/* Outer header type Ethernet */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_IPV4,	/* Outer header type IPv4 */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_IPV6,	/* Outer header type IPv6 */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_UDP,		/* Outer header type UDP */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_UDP_LITE,	/* Outer header type UDP lite */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_TCP,		/* Outer header type TCP */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_GRE,		/* Outer header type GRE */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_VXLAN,	/* Outer header type VXLAN */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_VXLAN_GPE,	/* Outer header type VXLAN-GPE */
	PPE_DRV_TUN_PRGM_PRSR_OHDR_GENEVE,	/* Outer header type GENEVE */
};

/*
 * ppe_drv_tun_prgm_prsr_pos_mode
 *	Parser position mode
 */
enum ppe_drv_tun_prgm_prsr_pos_mode {
	PPE_DRV_TUN_PRGM_PRSR_POS_MODE_END = 0,	/* Position from end of outer header */
	PPE_DRV_TUN_PRGM_PRSR_POS_MODE_START,	/* Position from start of outer header */
};

/*
 * ppe_drv_tun_prgm_prsr_inner_mode
 *	Outer header mode
 */
enum ppe_drv_tun_prgm_prsr_inner_mode {
	PPE_DRV_TUN_PRGM_PRSR_INNER_MODE_FIX = 0,	/* next header mode fixed */
	PPE_DRV_TUN_PRGM_PRSR_INNER_MODE_UDF,		/* next header mode UDF */
};

/*
 * ppe_drv_tun_prgm_prsr_inner_hdr_t
 *	Inner header type
 */
enum ppe_drv_tun_prgm_prsr_inner_hdr_t {
	PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_ETH = 0,	/* Inner header type Ethernet  */
	PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_ETH_TAG,	/* Inner header type Ethernet TAG */
	PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_IPV4,		/* Inner header type IPv4 */
	PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_IPV6,		/* Inner header type IPv6 */
};

/*
 * ppe_drv_tun_prgm_prsr_decap_key
 *	Tunnel program parser key structure
 */
struct ppe_drv_tun_prgm_prsr_decap_key {
	uint8_t udf1_id;	/* UDF1 index */
	uint8_t udf0_id;	/* UDF0 index */
	uint16_t key_bitmap;	/* Decap key Bitmap configuration */
	uint16_t udf0_mask;	/* UDF0 mask */
	uint16_t udf1_mask;	/* UDF1 mask */
	uint32_t tunnel_info_mask;	/* Tunnel info mask */
};

/*
 * ppe_drv_tun_prgm_prsr_prgm_udf_cfg
 *	Program UDF configuration
 */
struct ppe_drv_tun_prgm_prsr_prgm_udf_cfg {
	uint16_t hdr_len;	/* header length in bytes */
	uint8_t len_unit;	/* header length unit 0=1byte, 1=2 bytes, 2=4bytes, 3=8 bytes */
	uint8_t len_mask;	/* length mask */
	uint8_t udf_offset[PPE_DRV_TUN_PRGM_UDF_MAX];	/* udf offset configuration */
};

/*
 * ppe_drv_tun_prgm_prsr_prgm_udf
 *	Program UDF entry configuration
 */
struct ppe_drv_tun_prgm_prsr_prgm_udf {
	uint8_t udf_bitmap;				/* Bitmap for UDF configuration */
	uint16_t udf_val[PPE_DRV_TUN_PRGM_UDF_MAX];	/* UDF values */
	uint16_t udf_mask[PPE_DRV_TUN_PRGM_UDF_MAX];	/* UDF mask */
	uint8_t action_bitmap;				/* action bitmap to set header valid/ header length valid */
	enum ppe_drv_tun_prgm_prsr_inner_hdr_t inner_hdr;	/* inner header type */
	uint8_t hdr_len;				/* header leangth */
};

/*
 * ppe_drv_tun_prgm_prsr_l2tp
 * 	Program Parser l2tp tunnel specific data
 */
struct ppe_drv_tun_prgm_prsr_l2tp {
	struct ppe_drv_tun_prgm_prsr_prgm_udf ipv4_udf;	/* program UDF entry used for matching IPv4 L2TP inner payload */
	struct ppe_drv_tun_prgm_prsr_prgm_udf ipv6_udf;	/* program UDF entry used for matching IPv6 L2TP inner payload */
	struct ppe_drv_tun_udf *l2tp_udf;		/* UDF profile used for L2TP tunnel */
};

/*
 * ppe_drv_tun_prgm_prsr_cfg
 *	Tunnel program parser configuration structure
 */
struct ppe_drv_tun_prgm_prsr_cfg {
	enum ppe_drv_tun_prgm_prsr_ip_ver ip_ver;			/* Outer header IP version */
	enum ppe_drv_tun_prgm_prsr_outer_hdr_t outer_hdr;		/* Outer header type */
	enum ppe_drv_tun_prgm_prsr_inner_mode inner_mode;		/* program parser inner mode */
	enum ppe_drv_tun_prgm_prsr_pos_mode pos_mode;			/* program position mode */
	uint32_t protocol;						/* protocol value based on outer header (32 bits) */
	uint32_t protocol_mask;						/* protocol mask */
	union {
		enum ppe_drv_tun_prgm_prsr_inner_hdr_t inner_hdr;	/* Inner header type used in case of fix mode */
		struct ppe_drv_tun_prgm_prsr_prgm_udf_cfg udf;		/* Udf data used in case of udf mode */
	} conf;
};

/*
 * ppe_drv_tun_prgm_prsr_decap_cfg
 *      Tunnel program parser tunnel data
 */
struct ppe_drv_tun_prgm_prsr_decap_cfg {
	enum ppe_drv_tun_prgm_prsr_mode mode;		/* Program parser mode configured */
	struct ppe_drv_tun_prgm_prsr_decap_key key;	/* Program parser key configuration */
	struct ppe_drv_tun_prgm_prsr_cfg prsr_cfg;	/* Program parser configuration */
	union {
		struct ppe_drv_tun_prgm_prsr_l2tp l2tp;	/* L2TP specific data */
	} data;
};

/*
 * ppe_drv_tun_pgm_parser
 *	tun decap Program parser
 */
struct ppe_drv_tun_prgm_prsr {
	struct kref ref;				/* Reference Counter */
	uint8_t parser_idx;				/* parser index (0-5) denotes program0-program5 */
	struct ppe_drv_tun_prgm_prsr_decap_cfg ctx;	/* Program parser tunnel specific data */
};

/*
 * ppe_drv_tun_prgm_udf_bitmap_check()
 * 	check program udf bitmap
 */
static inline bool ppe_drv_tun_prgm_udf_bitmap_check(struct ppe_drv_tun_prgm_prsr_prgm_udf *udf, uint8_t flag)
{
	return !!(udf->udf_bitmap & (1 << flag));
}

/*
 * ppe_drv_tun_prgm_udf_bitmap_set()
 * 	set program udf bitmap
 */
static inline void ppe_drv_tun_prgm_udf_bitmap_set(struct ppe_drv_tun_prgm_prsr_prgm_udf *udf, uint8_t flag)
{
	udf->udf_bitmap |= (1 << flag);
}

/*
 * ppe_drv_tun_prgm_udf_bitmap_clear()
 * 	clear program udf bitmask
 */
static inline void ppe_drv_tun_prgm_udf_bitmap_clear(struct ppe_drv_tun_prgm_prsr_prgm_udf *udf, uint8_t flag)
{
	udf->udf_bitmap &= ~(1 << flag);
}

/*
 * ppe_drv_tun_prgm_udf_action_bitmap_check()
 * 	check program udf action bitmap
 */
static inline bool ppe_drv_tun_prgm_udf_action_bitmap_check(struct ppe_drv_tun_prgm_prsr_prgm_udf *udf, uint8_t flag)
{
	return !!(udf->action_bitmap & (1 << flag));
}

/*
 * ppe_drv_tun_prgm_udf_action_bitmap_set()
 * 	set program udf action bitmap
 */
static inline void ppe_drv_tun_prgm_udf_action_bitmap_set(struct ppe_drv_tun_prgm_prsr_prgm_udf *udf, uint8_t flag)
{
	udf->action_bitmap |= (1 << flag);
}

/*
 * ppe_drv_tun_prgm_udf_action_bitmap_clear()
 * 	clear program udf action bitmask
 */
static inline void ppe_drv_tun_prgm_udf_action_bitmap_clear(struct ppe_drv_tun_prgm_prsr_prgm_udf *udf, uint8_t flag)
{
	udf->action_bitmap &= ~(1 << flag);
}

/*
 * PPE Decap Program Tunnel Parser API's
 */
void ppe_drv_tun_prgm_prsr_free(struct ppe_drv_tun_prgm_prsr *program_parser);
struct ppe_drv_tun_prgm_prsr *ppe_drv_tun_prgm_prsr_alloc(struct ppe_drv *p);
struct ppe_drv_tun_prgm_prsr *ppe_drv_tun_prgm_prsr_entry_alloc(enum ppe_drv_tun_prgm_prsr_mode mode);
bool ppe_drv_tun_prgm_prsr_configured(struct ppe_drv_tun_prgm_prsr *program_parser);
bool ppe_drv_tun_prgm_prsr_deref(struct ppe_drv_tun_prgm_prsr *pgm);
void ppe_drv_tun_prgm_prsr_ref(struct ppe_drv_tun_prgm_prsr *pgm);
bool ppe_drv_tun_prgm_prsr_configure(struct ppe_drv_tun_prgm_prsr_cfg *prsr_cfg, struct ppe_drv_tun_prgm_prsr_decap_key *key, uint8_t parser_idx);
bool ppe_drv_tun_prgm_prsr_prgm_udf_fill(fal_tunnel_program_udf_t *pgm_udf, struct ppe_drv_tun_prgm_prsr_prgm_udf *udf);
bool ppe_drv_tun_prgm_prsr_deconfigure(struct ppe_drv_tun_prgm_prsr_cfg *prsr_cfg, uint8_t parser_idx);
bool ppe_drv_tun_prgm_prsr_type_allocated(enum ppe_drv_tun_prgm_prsr_mode prsr_mode);
bool ppe_drv_tun_prgm_prsr_prgm_udf_deconfigure(uint8_t parser_idx, struct ppe_drv_tun_prgm_prsr_prgm_udf *udf);
bool ppe_drv_tun_prgm_prsr_prgm_udf_configure(uint8_t parser_idx, struct ppe_drv_tun_prgm_prsr_prgm_udf *udf);
#endif /* _PPE_DRV_TUN_PRGM_PRSR_H_ */
