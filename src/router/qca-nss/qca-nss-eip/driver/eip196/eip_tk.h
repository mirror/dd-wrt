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
#define EIP_TK_IPSEC_ESP_TRAILER_LEN    (2)
#define EIP_TK_IPSEC_ESP_NXT_HDR(proto) (proto << 9)

#define EIP_TK_DTLS_PAD_LEN_SZ 1	/* 1 Byte Pad length field */
#define EIP_TK_PKT_TYPE(t) ((t) << 9)	/* Packet type for type instruction */
#define EIP_TK_IRR_LEN(l) ((l) << 19)	/* Length in IRR instruction */

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
	EIP_TK_CTRL_OP_MASK = 0x0000000F,	/* Maximum Bit mask */
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

	/*
	 * Ipsec specific instuctions
	 */
	EIP_TK_INST_ESP_HDR_HMAC = 0x23900000,		/* Only hash ESP header */
	EIP_TK_INST_ENC_HMAC_IPSEC = 0x07000000,	/* Encrypt then hash intermediate block */
	EIP_TK_INST_PAD_ENC_HMAC = 0x2F220000,		/* Pad before encryption then hash */
	EIP_TK_INST_CHK_SEQ_NO_ROLLOVER = 0xD0060000,	/* Verify sequence no rollover */
	EIP_TK_INST_SEQ_NO_AND_MASK_UPDT = 0xE02E1800,	/* Update sequence no /and mask in context record */

	EIP_TK_INST_ESP_HDR_CHK = 0x42900000,		/* ESP header authentication */
	EIP_TK_INST_HMAC_GET_IPSEC = 0x40E60000,	/* Retrieve hash */
	EIP_TK_INST_ICV_SPI_SEQ_NO_CHK = 0xDD070000,	/* Verify hash, SPI, Seq no */

	EIP_TK_INST_ESP_HDR_GCM = 0x2B900000,		/* Only hash ESP header for GCM */
	EIP_TK_INST_INS_IV_GCM = 0x21A80000,		/* Insert IV without authentication */
	EIP_TK_INST_SCH_RM_Y0_OUT = 0xA0800000,		/* Schedule removal of Y0 */

	EIP_TK_INST_ESP_HDR_CHK_GCM = 0x4A900000,	/* ESP header authentication */
	EIP_TK_INST_REM_IV_GCM = 0x40A80000,		/* Remove IV without authentication */

	/*
	 * DTLS specific instruction
	 */
	EIP_TK_INST_DATA = 0x0,	/* Token contains only Data */
	EIP_TK_INST_EPOCH_SEQ_HMAC = 0x22580004,	/* Hash Epoch and sequence number */
	EIP_TK_INST_LO_SEQ_HMAC = 0x22500004,	/* Hash lower sequence number */
	EIP_TK_INST_TYPE_HMAC_N_OUT = 0x23100001,	/* Hash pkt type & write to output */
	EIP_TK_INST_VER_HMAC_N_OUT = 0x23900002,	/* Hash version and also write version to output */
	EIP_TK_INST_EPOCH_SEQ_OUT = 0x21580004,	/* Write Epoch and sequence to output */
	EIP_TK_INST_LO_SEQ_OUT = 0x21500004,	/* Write lower sequence to output */
	EIP_TK_INST_DATA_LEN_HMAC = 0x22D80002,	/* Hash input data length field */
	EIP_TK_INST_FRAG_LEN_OUT = 0x21D80002,	/* Write fragment length to output */
	EIP_TK_INST_IV_OUT = 0x21A00000,	/* Write IV used for operation to output */
	EIP_TK_INST_DATA_HMAC_ENC_OUT = 0x07020000,	/* Hash, encrypt and write to output */
	EIP_TK_INST_DATA_ENC_HMAC_OUT = 0x0F020000,	/* Hash, encrypt (Last bit set for CTR) */
	EIP_TK_INST_HMAC_OUT = 0x25E20000,	/* Write generated MAC to output */
	EIP_TK_INST_PAD_ENC_OUT = 0x252E0000,	/* Encrypt padding and write result to output */
	EIP_TK_INST_SEQ64_NO_UPDT = 0xE2561800,	/* Update sequence number in context */
	EIP_TK_INST_REM_OUT_WORD = 0xA0220000,	/* Remove last 1 word of output data when available */
	EIP_TK_INST_2BLK_DEC = 0x24D80000,	/* Insert len bytes to cipher */
	EIP_TK_INST_WORD_DEC_OUT = 0x25D80004,	/* Insert last 4 byte to cipher and output */
	EIP_TK_INST_TYPE_N_VER_UPDT = 0x40900003,	/* Read type & version from input and store in context */
	EIP_TK_INST_EPOCH_N_HSEQ_HMAC = 0x42480004,	/* Read & hash epoch & hi sequence number */
	EIP_TK_INST_LSEQ_HMAC = 0x42400004,	/* Read and hash lo sequence number */
	EIP_TK_INST_TYPE_N_VER_HMAC = 0x22C00003,	/* Hash type & version from context */
	EIP_TK_INST_ENC_LEN_UPDT = 0x22780000,	/* Hash encrypted Payload length */
	EIP_TK_INST_FRAG_LEN_REM_STORE = 0x40000002,	/* Remove fragment length field from input */
	EIP_TK_INST_FRAG_LEN_REM = 0x40D80002,	/* Remove fragment length field from input */
	EIP_TK_INST_IV_UPDT = 0x40A00000,	/* Read IV from packet & store in context */
	EIP_TK_INST_INPUT_HMAC = 0xA0020000,	/* Pointer to start of HMAC */
	EIP_TK_INST_DATA_DEC_HMAC_N_OUT = 0x077A0000,	/* Decrypt payload and write to hash & output */
	EIP_TK_INST_DATA_LAST_DEC_HMAC_N_OUT = 0x0F020000,	/* Decrypt payload and write to hash & output */
	EIP_TK_INST_HMAC_DEC_N_OUT = 0x0D7E0000,	/* Decrypt MAC-Padding bytes and write to output */
	EIP_TK_INST_SEQ_PAD_HMAC_CHK = 0xD9070000,	/* Verify sequence, padding & HMAC */
	EIP_TK_INST_SEQ_HMAC_CHK = 0xD8070000,	/* Verify sequence & HMAC */
	EIP_TK_INST_SEQ_UPDT = 0xE4561800,	/* Update sequence number in context */
	EIP_TK_INST_LAST_BIT = 0x08000000,	/* End of current auth or crypto operation */
};

/*
 * struct eip_tk
 */
struct eip_tk {
	uint32_t words[32];                 /* Maximum Space for Control, IV and instruction */
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

uint8_t eip_tk_ipsec_encauth_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_ipsec_authdec_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_ipsec_encauth_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_ipsec_authdec_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params);

uint8_t eip_tk_dtls_encauth_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_dtls_authdec_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_dtls_encauth_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params);
uint8_t eip_tk_dtls_authdec_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params);


#endif /* __EIP_TK_H */
