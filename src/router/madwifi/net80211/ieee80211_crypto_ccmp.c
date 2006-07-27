/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211_crypto_ccmp.c 1443 2006-02-06 20:20:57Z mrenzmann $
 */

/*
 * IEEE 802.11i AES-CCMP crypto support.
 *
 * Part of this module is derived from similar code in the Host
 * AP driver. The code is used with the consent of the author and
 * it's license is included below.
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/random.h>
#include <linux/init.h>

#include <linux/crypto.h>
#include <asm/scatterlist.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>

#define AES_BLOCK_LEN 16

struct ccmp_ctx {
	struct ieee80211vap *cc_vap;	/* for diagnostics + statistics */
	struct ieee80211com *cc_ic;
	struct crypto_tfm *cc_tfm;
};

static void *ccmp_attach(struct ieee80211vap *, struct ieee80211_key *);
static void ccmp_detach(struct ieee80211_key *);
static int ccmp_setkey(struct ieee80211_key *);
static int ccmp_encap(struct ieee80211_key *, struct sk_buff *, u_int8_t);
static int ccmp_decap(struct ieee80211_key *, struct sk_buff *, int);
static int ccmp_enmic(struct ieee80211_key *, struct sk_buff *, int);
static int ccmp_demic(struct ieee80211_key *, struct sk_buff *, int);

static const struct ieee80211_cipher ccmp = {
	.ic_name	= "AES-CCM",
	.ic_cipher	= IEEE80211_CIPHER_AES_CCM,
	.ic_header	= IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN +
			  IEEE80211_WEP_EXTIVLEN,
	.ic_trailer	= IEEE80211_WEP_MICLEN,
	.ic_miclen	= 0,
	.ic_attach	= ccmp_attach,
	.ic_detach	= ccmp_detach,
	.ic_setkey	= ccmp_setkey,
	.ic_encap	= ccmp_encap,
	.ic_decap	= ccmp_decap,
	.ic_enmic	= ccmp_enmic,
	.ic_demic	= ccmp_demic,
};

static int ccmp_encrypt(struct ieee80211_key *, struct sk_buff *, int);
static int ccmp_decrypt(struct ieee80211_key *, u_int64_t, struct sk_buff *, int);

static void *
ccmp_attach(struct ieee80211vap *vap, struct ieee80211_key *k)
{
	struct ccmp_ctx *ctx;

	_MOD_INC_USE(THIS_MODULE, return NULL);

	MALLOC(ctx, struct ccmp_ctx *, sizeof(struct ccmp_ctx),
		M_DEVBUF, M_NOWAIT | M_ZERO);
	if (ctx == NULL) {
		vap->iv_stats.is_crypto_nomem++;
		_MOD_DEC_USE(THIS_MODULE);
		return NULL;
	}

	ctx->cc_vap = vap;
	ctx->cc_ic = vap->iv_ic;
	return ctx;
}

static void
ccmp_detach(struct ieee80211_key *k)
{
	struct ccmp_ctx *ctx = k->wk_private;

	if (ctx->cc_tfm != NULL)
		crypto_free_tfm(ctx->cc_tfm);
	FREE(ctx, M_DEVBUF);

	_MOD_DEC_USE(THIS_MODULE);
}

static int
ccmp_setkey(struct ieee80211_key *k)
{
	struct ccmp_ctx *ctx = k->wk_private;

	if (k->wk_keylen != (128 / NBBY)) {
		IEEE80211_DPRINTF(ctx->cc_vap, IEEE80211_MSG_CRYPTO,
			"%s: Invalid key length %u, expecting %u\n",
			__func__, k->wk_keylen, 128 / NBBY);
		return 0;
	}
	
	if (k->wk_flags & IEEE80211_KEY_SWCRYPT) {
		if (ctx->cc_tfm == NULL)
			ctx->cc_tfm = crypto_alloc_tfm("aes", 0);
		
		if (ctx->cc_tfm == NULL) {
			IEEE80211_DPRINTF(ctx->cc_vap, IEEE80211_MSG_CRYPTO,
				"%s: Tried to add a software crypto key, but software crypto not available\n",
				__func__);
			return 0;
		}
		
		crypto_cipher_setkey(ctx->cc_tfm, k->wk_key, k->wk_keylen);
	}
	return 1;
}

/*
 * Add privacy headers appropriate for the specified key.
 */
static int
ccmp_encap(struct ieee80211_key *k, struct sk_buff *skb, u_int8_t keyid)
{
	struct ccmp_ctx *ctx = k->wk_private;
	struct ieee80211com *ic = ctx->cc_ic;
	u_int8_t *ivp;
	int hdrlen;

	hdrlen = ieee80211_hdrspace(ic, skb->data);

	/*
	 * Copy down 802.11 header and add the IV, KeyID, and ExtIV.
	 */
	ivp = skb_push(skb, ccmp.ic_header);
	memmove(ivp, ivp + ccmp.ic_header, hdrlen);
	ivp += hdrlen;

	k->wk_keytsc++;		/* XXX wrap at 48 bits */
	ivp[0] = k->wk_keytsc >> 0;		/* PN0 */
	ivp[1] = k->wk_keytsc >> 8;		/* PN1 */
	ivp[2] = 0;				/* Reserved */
	ivp[3] = keyid | IEEE80211_WEP_EXTIV;	/* KeyID | ExtID */
	ivp[4] = k->wk_keytsc >> 16;		/* PN2 */
	ivp[5] = k->wk_keytsc >> 24;		/* PN3 */
	ivp[6] = k->wk_keytsc >> 32;		/* PN4 */
	ivp[7] = k->wk_keytsc >> 40;		/* PN5 */

	/*
	 * Finally, do software encrypt if neeed.
	 */
	if ((k->wk_flags & IEEE80211_KEY_SWCRYPT) &&
	    !ccmp_encrypt(k, skb, hdrlen))
		return 0;

	return 1;
}

/*
 * Add MIC to the frame as needed.
 */
static int
ccmp_enmic(struct ieee80211_key *k, struct sk_buff *skb, int force)
{
	return 1;
}

static __inline uint64_t
READ_6(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5)
{
	uint32_t iv32 = (b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
	uint16_t iv16 = (b4 << 0) | (b5 << 8);
	return (((uint64_t)iv16) << 32) | iv32;
}

/*
 * Validate and strip privacy headers (and trailer) for a
 * received frame. The specified key should be correct but
 * is also verified.
 */
static int
ccmp_decap(struct ieee80211_key *k, struct sk_buff *skb, int hdrlen)
{
	struct ccmp_ctx *ctx = k->wk_private;
	struct ieee80211vap *vap = ctx->cc_vap;
	struct ieee80211_frame *wh;
	uint8_t *ivp;
	uint64_t pn;
	u_int8_t tid;

	/*
	 * Header should have extended IV and sequence number;
	 * verify the former and validate the latter.
	 */
	wh = (struct ieee80211_frame *)skb->data;
	ivp = skb->data + hdrlen;
	if ((ivp[IEEE80211_WEP_IVLEN] & IEEE80211_WEP_EXTIV) == 0) {
		/*
		 * No extended IV; discard frame.
		 */
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr2,
			"%s", "Missing ExtIV for AES-CCM cipher");
		vap->iv_stats.is_rx_ccmpformat++;
		return 0;
	}
	tid = 0;
	if (IEEE80211_QOS_HAS_SEQ(wh)) 
		tid = ((struct ieee80211_qosframe *)wh)->i_qos[0] & IEEE80211_QOS_TID;
	/* NB: assume IEEEE80211_WEP_MINLEN covers the extended IV */ 
	pn = READ_6(ivp[0], ivp[1], ivp[4], ivp[5], ivp[6], ivp[7]);
	if (pn <= k->wk_keyrsc[tid]) {
		/*
		 * Replay violation.
		 */
		ieee80211_notify_replay_failure(vap, wh, k, pn);
		vap->iv_stats.is_rx_ccmpreplay++;
		return 0;
	}

	/*
	 * Check if the device handled the decrypt in hardware.
	 * If so we just strip the header; otherwise we need to
	 * handle the decrypt in software.  Note that for the
	 * latter we leave the header in place for use in the
	 * decryption work.
	 */
	if ((k->wk_flags & IEEE80211_KEY_SWCRYPT) &&
	    !ccmp_decrypt(k, pn, skb, hdrlen))
		return 0;

	/*
	 * Copy up 802.11 header and strip crypto bits.
	 */
	memmove(skb->data + ccmp.ic_header, skb->data, hdrlen);
	skb_pull(skb, ccmp.ic_header);
	while (skb->next != NULL)
		skb = skb->next;
	skb_trim(skb, skb->len - ccmp.ic_trailer);

	/*
	 * Ok to update rsc now.
	 */
	k->wk_keyrsc[tid] = pn;

	return 1;
}

/*
 * Verify and strip MIC from the frame.
 */
static int
ccmp_demic(struct ieee80211_key *k, struct sk_buff *skb, int hdrlen)
{
	return 1;
}

static __inline void
xor_block(u8 *b, const u8 *a, size_t len)
{
	int i;
	for (i = 0; i < len; i++)
		b[i] ^= a[i];
}

static void
rijndael_encrypt(struct crypto_tfm *tfm, const void *src, void *dst)
{
	struct scatterlist sg_src;
	struct scatterlist sg_dst;

	sg_src.page = virt_to_page(src);
	sg_src.offset = offset_in_page(src);
	sg_src.length = AES_BLOCK_LEN;

	sg_dst.page = virt_to_page(dst);
	sg_dst.offset = offset_in_page(dst);
	sg_dst.length = AES_BLOCK_LEN;
	crypto_cipher_encrypt(tfm, &sg_dst, &sg_src, AES_BLOCK_LEN);
}

/*
 * Host AP crypt: host-based CCMP encryption implementation for Host AP driver
 *
 * Copyright (c) 2003-2004, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 */

static void
ccmp_init_blocks(struct crypto_tfm *tfm, struct ieee80211_frame *wh,
	u_int64_t pn, size_t dlen,
	uint8_t b0[AES_BLOCK_LEN], uint8_t aad[2 * AES_BLOCK_LEN],
	uint8_t auth[AES_BLOCK_LEN], uint8_t s0[AES_BLOCK_LEN])
{
#define	IS_4ADDRESS(wh) \
	((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) == IEEE80211_FC1_DIR_DSTODS)
#define	IS_QOS_DATA(wh)	IEEE80211_QOS_HAS_SEQ(wh)

	/* CCM Initial Block:
	 * Flag (Include authentication header, M=3 (8-octet MIC),
	 *       L=1 (2-octet Dlen))
	 * Nonce: 0x00 | A2 | PN
	 * Dlen */
	b0[0] = 0x59;
	/* NB: b0[1] set below */
	IEEE80211_ADDR_COPY(b0 + 2, wh->i_addr2);
	b0[8] = pn >> 40;
	b0[9] = pn >> 32;
	b0[10] = pn >> 24;
	b0[11] = pn >> 16;
	b0[12] = pn >> 8;
	b0[13] = pn >> 0;
	b0[14] = (dlen >> 8) & 0xff;
	b0[15] = dlen & 0xff;

	/* AAD:
	 * FC with bits 4..6 and 11..13 masked to zero; 14 is always one
	 * A1 | A2 | A3
	 * SC with bits 4..15 (seq#) masked to zero
	 * A4 (if present)
	 * QC (if present)
	 */
	aad[0] = 0;	/* AAD length >> 8 */
	/* NB: aad[1] set below */
	aad[2] = wh->i_fc[0] & 0x8f;	/* XXX magic #s */
	aad[3] = wh->i_fc[1] & 0xc7;	/* XXX magic #s */
	/* NB: we know 3 addresses are contiguous */
	memcpy(aad + 4, wh->i_addr1, 3 * IEEE80211_ADDR_LEN);
	aad[22] = wh->i_seq[0] & IEEE80211_SEQ_FRAG_MASK;
	aad[23] = 0; /* all bits masked */
	/*
	 * Construct variable-length portion of AAD based
	 * on whether this is a 4-address frame/QOS frame.
	 * We always zero-pad to 32 bytes before running it
	 * through the cipher.
	 *
	 * We also fill in the priority bits of the CCM
	 * initial block as we know whether or not we have
	 * a QOS frame.
	 */
	if (IS_4ADDRESS(wh)) {
		IEEE80211_ADDR_COPY(aad + 24,
			((struct ieee80211_frame_addr4 *)wh)->i_addr4);
		if (IS_QOS_DATA(wh)) {
			struct ieee80211_qosframe_addr4 *qwh4 =
				(struct ieee80211_qosframe_addr4 *) wh;
			aad[30] = qwh4->i_qos[0] & 0x0f;/* just priority bits */
			aad[31] = 0;
			b0[1] = aad[30];
			aad[1] = 22 + IEEE80211_ADDR_LEN + 2;
		} else {
			*(u_int16_t *)&aad[30] = 0;
			b0[1] = 0;
			aad[1] = 22 + IEEE80211_ADDR_LEN;
		}
	} else {
		if (IS_QOS_DATA(wh)) {
			struct ieee80211_qosframe *qwh =
				(struct ieee80211_qosframe*) wh;
			aad[24] = qwh->i_qos[0] & 0x0f;	/* just priority bits */
			aad[25] = 0;
			b0[1] = aad[24];
			aad[1] = 22 + 2;
		} else {
			*(u_int16_t *)&aad[24] = 0;
			b0[1] = 0;
			aad[1] = 22;
		}
		*(u_int16_t *)&aad[26] = 0;
		*(u_int32_t *)&aad[28] = 0;
	}

	/* Start with the first block and AAD */
	rijndael_encrypt(tfm, b0, auth);
	xor_block(auth, aad, AES_BLOCK_LEN);
	rijndael_encrypt(tfm, auth, auth);
	xor_block(auth, &aad[AES_BLOCK_LEN], AES_BLOCK_LEN);
	rijndael_encrypt(tfm, auth, auth);
	b0[0] &= 0x07;
	b0[14] = b0[15] = 0;
	rijndael_encrypt(tfm, b0, s0);
#undef	IS_QOS_DATA
#undef	IS_4ADDRESS
}

#define	CCMP_ENCRYPT(_i, _b, _b0, _pos, _e, _len) do {	\
	/* Authentication */				\
	xor_block(_b, _pos, _len);			\
	rijndael_encrypt(ctx->cc_tfm, _b, _b);		\
	/* Encryption, with counter */			\
	_b0[14] = (_i >> 8) & 0xff;			\
	_b0[15] = _i & 0xff;				\
	rijndael_encrypt(ctx->cc_tfm, _b0, _e);		\
	xor_block(_pos, _e, _len);			\
} while (0)

static int
ccmp_encrypt(struct ieee80211_key *key, struct sk_buff *skb0, int hdrlen)
{
	struct ccmp_ctx *ctx = key->wk_private;
	struct ieee80211_frame *wh = (struct ieee80211_frame *) skb0->data;
	struct sk_buff *skb;
	int data_len, i;
	uint8_t aad[2 * AES_BLOCK_LEN], b0[AES_BLOCK_LEN], b[AES_BLOCK_LEN];
	uint8_t e[AES_BLOCK_LEN], s0[AES_BLOCK_LEN];
	uint8_t *mic, *pos;
	u_int space;

	ctx->cc_vap->iv_stats.is_crypto_ccmp++;

	skb = skb0;
	data_len = skb->len;
	while (skb->next != NULL) {
		skb = skb->next;
		data_len += skb->len;
	}
	data_len -= hdrlen + ccmp.ic_header;
	if (skb_tailroom(skb) < ccmp.ic_trailer) {
		/* NB: should not happen */
		IEEE80211_NOTE_MAC(ctx->cc_vap, IEEE80211_MSG_CRYPTO,
			wh->i_addr1, "No room for %s MIC, tailroom %u",
			ccmp.ic_name, skb_tailroom(skb));
		/* XXX statistic */
		return 0;
	}
	ccmp_init_blocks(ctx->cc_tfm, wh, key->wk_keytsc,
		data_len, b0, aad, b, s0);

	i = 1;
	skb = skb0;
	pos = skb->data + hdrlen + ccmp.ic_header;
	/* NB: assumes header is entirely in first skbuf */
	space = skb->len - (hdrlen + ccmp.ic_header);
	for (;;) {
		if (space > data_len)
			space = data_len;
		/*
		 * Do full blocks.
		 */
		while (space >= AES_BLOCK_LEN) {
			CCMP_ENCRYPT(i, b, b0, pos, e, AES_BLOCK_LEN);
			pos += AES_BLOCK_LEN, space -= AES_BLOCK_LEN;
			data_len -= AES_BLOCK_LEN;
			i++;
		}
		if (data_len <= 0)		/* no more data */
			break;
		if (skb->next == NULL) {	/* last buffer */
			if (space != 0) {
				/*
				 * Short last block.
				 */
				CCMP_ENCRYPT(i, b, b0, pos, e, space);
			}
			break;
		}
		skb = skb->next;
		if (space != 0) {
			uint8_t *pos_next;
			u_int space_next;
			u_int len;

			/*
			 * Block straddles buffers, split references.  We
			 * do not handle splits that require >2 buffers.
			 */
			pos_next = skb->data;
			len = min(data_len, AES_BLOCK_LEN);
			space_next = len > space ? len - space : 0;
			KASSERT(skb->len >= space_next,
				("not enough data in following buffer, "
				"skb len %u need %u\n", skb->len, space_next));

			xor_block(b + space, pos_next, space_next);
			CCMP_ENCRYPT(i, b, b0, pos, e, space);
			xor_block(pos_next, e + space, space_next);
			data_len -= len;
			/* XXX could check for data_len <= 0 */
			i++;

			pos = pos_next + space_next;
			space = skb->len - space_next;
		} else {
			/*
			 * Setup for next buffer.
			 */
			pos = skb->data;
			space = skb->len;
		}
	}
	/* tack on MIC */
	mic = skb_put(skb, ccmp.ic_trailer);
	for (i = 0; i < ccmp.ic_trailer; i++)
		mic[i] = b[i] ^ s0[i];
	return 1;
}
#undef CCMP_ENCRYPT

#define	CCMP_DECRYPT(_i, _b, _b0, _pos, _a, _len) do {	\
	/* Decrypt, with counter */			\
	_b0[14] = (_i >> 8) & 0xff;			\
	_b0[15] = _i & 0xff;				\
	rijndael_encrypt(ctx->cc_tfm, _b0, _b);		\
	xor_block(_pos, _b, _len);			\
	/* Authentication */				\
	xor_block(_a, _pos, _len);			\
	rijndael_encrypt(ctx->cc_tfm, _a, _a);		\
} while (0)

static int
ccmp_decrypt(struct ieee80211_key *key, u_int64_t pn, struct sk_buff *skb0, int hdrlen)
{
	struct ccmp_ctx *ctx = key->wk_private;
	struct ieee80211_frame *wh = (struct ieee80211_frame *) skb0->data;
	struct sk_buff *skb;
	uint8_t aad[2 * AES_BLOCK_LEN];
	uint8_t b0[AES_BLOCK_LEN], b[AES_BLOCK_LEN], a[AES_BLOCK_LEN];
	size_t data_len;
	int i;
	uint8_t *pos, *mic;
	u_int space;

	ctx->cc_vap->iv_stats.is_crypto_ccmp++;

	skb = skb0;
	data_len = skb->len;
	while (skb->next != NULL) {
		skb = skb->next;
		data_len += skb->len;
	}
	data_len -= hdrlen + ccmp.ic_header + ccmp.ic_trailer;
	/* NB: skb left pointing at last in chain */
	ccmp_init_blocks(ctx->cc_tfm, wh, pn, data_len, b0, aad, a, b);
	/* NB: this is the last in the chain */
	/* XXX assert skb->len >= ccmp.ic_trailer */
	mic = skb->data + skb->len - ccmp.ic_trailer;
	xor_block(mic, b, ccmp.ic_trailer);

	i = 1;
	skb = skb0;
	pos = skb->data + hdrlen + ccmp.ic_header;
	space = skb->len - (hdrlen + ccmp.ic_header);
	for (;;) {
		if (space > data_len)
			space = data_len;
		while (space >= AES_BLOCK_LEN) {
			CCMP_DECRYPT(i, b, b0, pos, a, AES_BLOCK_LEN);
			pos += AES_BLOCK_LEN;
			space -= AES_BLOCK_LEN;
			data_len -= AES_BLOCK_LEN;
			i++;
		}
		if (data_len <= 0)		/* no more data */
			break;
		skb = skb->next;
		if (skb == NULL) {		/* last buffer */
			if (space != 0)		/* short last block */
				CCMP_DECRYPT(i, b, b0, pos, a, space);
			break;
		}
		if (space != 0) {
			uint8_t *pos_next;
			u_int space_next;
			u_int len;

			/*
			 * Block straddles buffers, split references.  We
			 * do not handle splits that require >2 buffers.
			 */
			pos_next = skb->data;
			len = min(data_len, (size_t) AES_BLOCK_LEN);
			space_next = len > space ? len - space : 0;
			KASSERT(skb->len >= space_next,
				("not enough data in following buffer, "
				"skb len %u need %u\n", skb->len, space_next));

			xor_block(b+space, pos_next, space_next);
			CCMP_DECRYPT(i, b, b0, pos, a, space);
			xor_block(pos_next, b+space, space_next);
			data_len -= len;
			i++;

			pos = pos_next + space_next;
			space = skb->len - space_next;
		} else {
			/*
			 * Setup for next buffer.
			 */
			pos = skb->data;
			space = skb->len;
		}
	}

	if (memcmp(mic, a, ccmp.ic_trailer) != 0) {
		IEEE80211_NOTE_MAC(ctx->cc_vap, IEEE80211_MSG_CRYPTO,
			wh->i_addr2,
			"AES-CCM decrypt failed; MIC mismatch (keyix %u, rsc %llu)",
			key->wk_keyix, pn);
		ctx->cc_vap->iv_stats.is_rx_ccmpmic++;
		return 0;
	}
	return 1;
}
#undef CCMP_DECRYPT

/*
 * Module glue.
 */

MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("802.11 wireless support: AES-CCM cipher");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int __init
init_crypto_ccmp(void)
{
	ieee80211_crypto_register(&ccmp);
	return 0;
}
module_init(init_crypto_ccmp);

static void __exit
exit_crypto_ccmp(void)
{
	ieee80211_crypto_unregister(&ccmp);
}
module_exit(exit_crypto_ccmp);
