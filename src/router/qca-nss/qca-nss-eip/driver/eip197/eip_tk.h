/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __EIP_TK_H
#define __EIP_TK_H

#include "eip.h"

struct eip_tr;

#define EIP_TK_INST_CONTEXT_ACCESS_LEN(words) (((words) & 0xF) << 24)

/*
 * eip_tk_ctrl_op
 *	Token control word bits.
 */
enum eip_tk_ctrl_op {
	EIP_TK_CTRL_OP_NO_OP = 0x0,		/* No operation */
	EIP_TK_CTRL_OP_HMAC_ADD = 0x00000002,	/* Hmac outbound */
	EIP_TK_CTRL_OP_HMAC_CHK = 0x00000003,	/* Hmac inbound */
	EIP_TK_CTRL_OP_ENC = 0x00000004,	/* Encrypt outbound */
	EIP_TK_CTRL_OP_DEC = 0x00000005,	/* Decrypt inbound */
	EIP_TK_CTRL_OP_ENC_HMAC = 0x00000006,	/* Encrypt followed by hmac outbound */
	EIP_TK_CTRL_OP_DEC_HMAC = 0x00000007,	/* Decrypt followed by hmac inbound */
	EIP_TK_CTRL_OP_HMAC_ENC = 0x0000000E,	/* Hmac followed by encrypt outbound */
	EIP_TK_CTRL_OP_HMAC_DEC = 0x0000000F,	/* Hmac followed by decrypt inbound */
};

/*
 * eip_tk_inst
 *	Token instructions.
 */
enum eip_tk_inst {
	/*
	 * Valid Bits[31:17]
	 */
	EIP_TK_INST_BYPASS = 0x01000000,		/* Bypass direction */
	EIP_TK_INST_ENC = 0x05940000,			/* Encrypt */
	EIP_TK_INST_PAD_ENC = 0x2D060000,		/* Pad before encryption */
	EIP_TK_INST_DEC = 0x0D960000,			/* Decrypt */
	EIP_TK_INST_DEC_CHK_PAD = 0xD0060000,		/* Verify pad after decrypt */
	EIP_TK_INST_ENC_HMAC = 0x0F000000,		/* Encrypt then hash */
	EIP_TK_INST_ENC_GCM = 0x0F000000,		/* Encrypt then hash */
	EIP_TK_INST_NOPAD_ENC_HMAC = 0x2F020000,	/* Dont pad before encrypt/hash */
	EIP_TK_INST_HMAC_DEC = 0x0F020000,	/* Hash before decrypt */
	EIP_TK_INST_HMAC_DEC_CHK = 0xD1070000,	/* Verify hash, pad after decrypt */
	EIP_TK_INST_HMAC = 0x03000000,		/* Hash */
	EIP_TK_INST_GCM_HMAC = 0x0B000000,	/* Hash */
	EIP_TK_INST_HASH = 0x02920000,		/* HASH ONLY */
	EIP_TK_INST_NOP = 0x20000000,		/* NOP instruction */
	EIP_TK_INST_DIGEST = 0x02020000,	/* Used for Intermediate DIGEST ONLY */
	EIP_TK_INST_IPAD_GEN = 0x20E00000,	/* Generate inner digest */
	EIP_TK_INST_OPAD_GEN = 0x20E60000,	/* Generate outer digest */
	EIP_TK_INST_IPAD_ADD = 0xE0EE0800,	/* Write inner digest to context at offset */
	EIP_TK_INST_OPAD_ADD = 0xE0E60800,	/* Write outer digest to context at offset */
	EIP_TK_INST_DEL_DATA = 0xa0800000,	/* Used for removing data */
	EIP_TK_INST_ADD_DATA = 0x25000000,	/* Used for inserting data */
	EIP_TK_INST_HMAC_ADD = 0x21E60000,	/* Insert generated hash */
	EIP_TK_INST_HMAC_GET = 0x40E60000,	/* Retrieve hash */
	EIP_TK_INST_HMAC_CHK = 0xD0070000,	/* Disable pad verification */
};

/*
 * struct eip_tk
 */
struct eip_tk {
	uint32_t words[16];                 /* Maximum Space for Control, IV and instruction */
} __attribute__((aligned(L1_CACHE_BYTES)));


/*
 * struct eip_tk_params
 */
struct eip_tk_params {
	struct eip_tr *tr;	/* transform record */
	eip_req_t eip_req;	/* request */
	uint32_t tk_hdr;	/* token header */
};

typedef uint8_t (*eip_tk_proc_t)(struct eip_tk *tk, struct eip_tk_params *tk_params);


/*
 * Fill token APIs.
 */
uint8_t eip_tk_encauth_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_authdec_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_encauth_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_authdec_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_digest(struct eip_tk *tk, struct eip_tr *tr, struct scatterlist *sg, uint32_t *tk_hdr,
		uint8_t ipad_offst, uint8_t opad_offst);
uint8_t eip_tk_auth(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_enc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_dec(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_enc_3des(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_dec_3des(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_enc_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_dec_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_encauth_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_authdec_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_encauth_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_authdec_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);

#endif /* __EIP_TK_H */
