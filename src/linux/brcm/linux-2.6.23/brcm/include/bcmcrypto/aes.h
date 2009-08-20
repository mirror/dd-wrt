/*
 * aes.h
 * AES encrypt/decrypt wrapper functions used around Rijndael reference
 * implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: aes.h,v 1.19 2008/02/15 21:12:14 Exp $
 */

#ifndef _AES_H_
#define _AES_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif
#include <bcmcrypto/rijndael-alg-fst.h>

/* forward declaration */
typedef struct dot11_header dot11_header_t;

#define AES_BLOCK_SZ    	16
#define AES_BLOCK_BITLEN	(AES_BLOCK_SZ * 8)
#define AES_KEY_BITLEN(kl)  	(kl * 8)
#define AES_ROUNDS(kl)		((AES_KEY_BITLEN(kl) / 32) + 6)
#define AES_MAXROUNDS		14

enum {
	NO_PADDING,
	PAD_LEN_PADDING /* padding with padding length  */
};


void BCMROMFN(aes_encrypt)(const size_t KL, const uint8 *K, const uint8 *ptxt, uint8 *ctxt);
void BCMROMFN(aes_decrypt)(const size_t KL, const uint8 *K, const uint8 *ctxt, uint8 *ptxt);

#define aes_block_encrypt(nr, rk, ptxt, ctxt) rijndaelEncrypt(rk, nr, ptxt, ctxt)
#define aes_block_decrypt(nr, rk, ctxt, ptxt) rijndaelDecrypt(rk, nr, ctxt, ptxt)

int
BCMROMFN(aes_cbc_encrypt_pad)(uint32 *rk, const size_t key_len, const uint8 *nonce,
                              const size_t data_len, const uint8 *ptxt, uint8 *ctxt,
                              uint8 pad_type);
int BCMROMFN(aes_cbc_encrypt)(uint32 *rk, const size_t key_len, const uint8 *nonce,
                              const size_t data_len, const uint8 *ptxt, uint8 *ctxt);
int BCMROMFN(aes_cbc_decrypt)(uint32 *rk, const size_t key_len, const uint8 *nonce,
                              const size_t data_len, const uint8 *ctxt, uint8 *ptxt);
int BCMROMFN(aes_cbc_decrypt_pad)(uint32 *rk, const size_t key_len, const uint8 *nonce,
                                  const size_t data_len, const uint8 *ctxt, uint8 *ptxt,
                                  uint8 pad_type);

#define AES_CTR_MAXBLOCKS	(1<<16)

int BCMROMFN(aes_ctr_crypt)(unsigned int *rk, const size_t key_len, const uint8 *nonce,
                            const size_t data_len, const uint8 *ptxt, uint8 *ctxt);

/* only support the 2 octet AAD length encoding */
#define AES_CCM_LEN_FIELD_LEN	2
#define AES_CCM_AAD_MAX_LEN	0xFEFF
#define AES_CCM_NONCE_LEN	13

#define AES_CCM_CRYPT_FLAGS	(AES_CCM_LEN_FIELD_LEN-1)

/* only support the 8 octet auth field */
#define AES_CCM_AUTH_LEN	8

#define AES_CCM_AUTH_FLAGS	(4*(AES_CCM_AUTH_LEN-2) + (AES_CCM_LEN_FIELD_LEN-1))
#define AES_CCM_AUTH_AAD_FLAG	0x40

#define AES_CCMP_AAD_MIN_LEN	22
#define AES_CCMP_AAD_MAX_LEN	30
#define AES_CCMP_NONCE_LEN	AES_CCM_NONCE_LEN
#define AES_CCMP_AUTH_LEN	AES_CCM_AUTH_LEN

#define AES_CCMP_FC_RETRY		0x800
#define AES_CCMP_FC_PM			0x1000
#define AES_CCMP_FC_MOREDATA		0x2000
#define AES_CCMP_FRAGNUM_MASK		0x000f
#define AES_CCMP_QOS_TID_MASK		0x000f

/* mute PM */
#define	AES_CCMP_LEGACY_FC_MASK		~(AES_CCMP_FC_RETRY)
#define	AES_CCMP_LEGACY_SEQ_MASK	0x0000
#define	AES_CCMP_LEGACY_QOS_MASK	0xffff

/* mute MoreData, PM, Retry, Low 3 bits of subtype */
#define AES_CCMP_SUBTYPE_LOW_MASK	0x0070
#define	AES_CCMP_FC_MASK 		~(AES_CCMP_SUBTYPE_LOW_MASK | AES_CCMP_FC_RETRY | \
					  AES_CCMP_FC_PM | AES_CCMP_FC_MOREDATA)
#define	AES_CCMP_SEQ_MASK		AES_CCMP_FRAGNUM_MASK
#define	AES_CCMP_QOS_MASK		AES_CCMP_QOS_TID_MASK

#define AES_CCMP_ENCRYPT_SUCCESS	0
#define AES_CCMP_ENCRYPT_ERROR		-1

/* AES-CCMP mode encryption algorithm
	- encrypts in-place
	- packet buffer must be contiguous
	- packet buffer must have sufficient tailroom for CCMP MIC
	- returns AES_CCMP_ENCRYPT_SUCCESS on success
	- returns AES_CCMP_ENCRYPT_ERROR on error
*/
int aes_ccmp_encrypt(unsigned int *rk, const size_t key_len,
	const size_t data_len, uint8 *p, bool legacy, uint8 nonce_1st_byte);

#define AES_CCMP_DECRYPT_SUCCESS	0
#define AES_CCMP_DECRYPT_ERROR		-1
#define AES_CCMP_DECRYPT_MIC_FAIL	-2

/* AES-CCMP mode decryption algorithm
	- decrypts in-place
	- packet buffer must be contiguous
	- packet buffer must have sufficient tailroom for CCMP MIC
	- returns AES_CCMP_DECRYPT_SUCCESS on success
	- returns AES_CCMP_DECRYPT_ERROR on decrypt protocol/format error
	- returns AES_CCMP_DECRYPT_MIC_FAIL on message integrity check failure
*/
int BCMROMFN(aes_ccmp_decrypt)(unsigned int *rk, const size_t key_len,
	const size_t data_len, uint8 *p, bool legacy, uint8 nonce_1st_byte);

void BCMROMFN(aes_ccmp_cal_params)(dot11_header_t *h, bool legacy, uint8 nonce_1st_byte,
                               uint8 *nonce, uint8 *aad, uint *la, uint *lh);

int BCMROMFN(aes_ccm_mac)(unsigned int *rk, const size_t key_len, const uint8 *nonce,
                          const size_t aad_len, const uint8 *aad, const size_t data_len,
                          const uint8 *ptxt, uint8 *mac);
int BCMROMFN(aes_ccm_encrypt)(unsigned int *rk, const size_t key_len, const uint8 *nonce,
                              const size_t aad_len, const uint8 *aad, const size_t data_len,
                              const uint8 *ptxt, uint8 *ctxt, uint8 *mac);
int BCMROMFN(aes_ccm_decrypt)(unsigned int *rk, const size_t key_len, const uint8 *nonce,
                              const size_t aad_len, const uint8 *aad, const size_t data_len,
                              const uint8 *ctxt, uint8 *ptxt);


INLINE void xor_128bit_block(const uint8 *src1, const uint8 *src2, uint8 *dst);

#endif /* _AES_H_ */
