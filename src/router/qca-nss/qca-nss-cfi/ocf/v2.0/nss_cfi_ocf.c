/* Copyright (c) 2014 - 2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

/**
 * nss_cfi_ocf.c
 * 	Interface to communicate OCF specific data to Crypto core specifc data
 */

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,38) && !defined(AUTOCONF_INCLUDED)
#include<linux/config.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/random.h>
#include <linux/skbuff.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
#include <asm/scatterlist.h>
#endif
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/md5.h>
#include <cryptodev.h>

#include <nss_api_if.h>
#include <nss_crypto_hdr.h>
#include <nss_cfi_if.h>
#include <nss_cfi_ocf.h>

#define NSS_CRYPTO_BUF_TAILROOM 128
#define ESP_HDR_LEN 8

static struct nss_cfi_ocf g_cfi_ocf = {{{0}}};

static struct nss_cfi_crypto_info gbl_crypto_info[NSS_CRYPTO_MAX_IDXS];

/*
 * cfi supported cipher and auth algorithms and max key, iv and hash lengths.
 * Note: Algorithm definitions start from 1 and hence, we need one additional
 * slot in the array to accommodate all algos. Slot 0 will be empty and unused.
 */
static struct nss_cfi_ocf_algo_map cfi_algo[NSS_CRYPTO_CMN_ALGO_MAX] = {
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC] =	{
								{CRYPTO_AES_CBC, CRYPTO_SHA1_HMAC},
								AES_KEYSIZE_128,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								SHA1_DIGEST_SIZE,
								1,
			   				},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC] =	{
								{CRYPTO_AES_CBC, CRYPTO_SHA1_HMAC},
								AES_KEYSIZE_192,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								SHA1_DIGEST_SIZE,
								1,
			   				},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC] =	{
								{CRYPTO_AES_CBC, CRYPTO_SHA1_HMAC},
								AES_KEYSIZE_256,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								SHA1_DIGEST_SIZE,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC] =	{
								{CRYPTO_3DES_CBC, CRYPTO_SHA1_HMAC},
								DES3_EDE_KEY_SIZE,
								DES_BLOCK_SIZE,
								DES3_EDE_BLOCK_SIZE,
								SHA1_DIGEST_SIZE,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_MD5_HMAC] =	{
								{CRYPTO_AES_CBC, CRYPTO_MD5_HMAC},
								AES_KEYSIZE_128,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								MD5_DIGEST_SIZE,
								1,
			   				},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC_MD5_HMAC] =	{
								{CRYPTO_AES_CBC, CRYPTO_MD5_HMAC},
								AES_KEYSIZE_192,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								MD5_DIGEST_SIZE,
								1,
			   				},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_MD5_HMAC] =	{
								{CRYPTO_AES_CBC, CRYPTO_MD5_HMAC},
								AES_KEYSIZE_256,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								MD5_DIGEST_SIZE,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_MD5_HMAC] =	{
								{CRYPTO_3DES_CBC, CRYPTO_MD5_HMAC},
								DES3_EDE_KEY_SIZE,
								DES_BLOCK_SIZE,
								DES3_EDE_BLOCK_SIZE,
								MD5_DIGEST_SIZE,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC] =		{
								{CRYPTO_3DES_CBC, 0},
								DES3_EDE_KEY_SIZE,
								DES_BLOCK_SIZE,
								DES3_EDE_BLOCK_SIZE,
								0,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC] =		{
								{CRYPTO_AES_CBC, 0},
								AES_KEYSIZE_128,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								0,
								1,
			   				},
	[NSS_CRYPTO_CMN_ALGO_AES192_CBC] =		{
								{CRYPTO_AES_CBC, 0},
								AES_KEYSIZE_192,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								0,
								1,
			   				},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC] =		{
								{CRYPTO_AES_CBC, 0},
								AES_KEYSIZE_256,
								AES_BLOCK_SIZE,
								AES_BLOCK_SIZE,
								0,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_SHA160_HMAC] =		{
								{CRYPTO_SHA1_HMAC, 0},
								0,
								0,
							 	0,
								SHA1_DIGEST_SIZE,
								1,
							},
	[NSS_CRYPTO_CMN_ALGO_MD5_HMAC] =		{
								{CRYPTO_MD5_HMAC, 0},
								0,
								0,
							 	0,
								MD5_DIGEST_SIZE,
								1,
							},
};

/*
 * Dummy trap function for Offload
 */
static int32_t nss_cfi_ocf_offload_trap(struct sk_buff *skb, struct nss_cfi_crypto_info *crypto)
{
	return -1;
}

/*
 * Dummy trap function for Session
 */
static int32_t nss_cfi_ocf_session_trap(uint32_t session_idx)
{
	return 0;
}

/*
 * Dummy trap function for IPsec encryption.
 */
static int32_t nss_cfi_ocf_encrypt_trap(struct sk_buff *skb, uint32_t session_idx)
{
	return 0;
}

/*
 * Dummy trap function for IPsec decryption.
 */
static int32_t nss_cfi_ocf_decrypt_trap(struct sk_buff *skb, uint32_t session_idx)
{
	return 0;
}

/*
 * Returns success if cfi can handle the type of buffer sent by OCF
 */
static struct sk_buff *nss_cfi_ocf_get_skb(struct cryptop *crp)
{
	/* IOV buffer type support unimplemented */
	nss_cfi_assert(!(crp->crp_flags & CRYPTO_F_IOV));

	if (crp->crp_flags & CRYPTO_F_SKBUF)
		return (struct sk_buff *)crp->crp_buf;

	return NULL;
}

/*
 * nss_cfi_ocf_cri2algo_aead()
 *	Gets an algorith number based on algos filled in cri for aead
 */
enum nss_crypto_cmn_algo nss_cfi_ocf_cri2algo_aead(struct cryptoini *cri)
{
	struct nss_cfi_ocf_algo_map *map = &cfi_algo[0];
	enum nss_crypto_cmn_algo algo;
	int cri_alg, cri_next_alg;
	int cipher_alg, auth_alg;
	int ckey_len, akey_len;

	BUG_ON(!cri->cri_next);

	cri_alg = cri->cri_alg;
	cri_next_alg = cri->cri_next->cri_alg;

	for (algo = 0; algo < NSS_CRYPTO_CMN_ALGO_MAX; algo++, map++) {
		cipher_alg = map->algo[0];
		auth_alg = map->algo[1];

		/*
		 * Skip non-AEAD algorithms
		 */
		if (!cipher_alg || !auth_alg) {
			continue;
		}

		/*
		 * Test whether AEAD is
		 * a) encrytion followed by authentication or
		 * b) authentication followed by decryption
		 */
		if ((cipher_alg == cri_alg) && (auth_alg == cri_next_alg)) {
			ckey_len = NSS_CFI_OCF_BITS2BYTES(cri->cri_klen);
			akey_len = NSS_CFI_OCF_BITS2BYTES(cri->cri_next->cri_klen);
		} else if ((auth_alg == cri_alg) && (cipher_alg == cri_next_alg)) {
			akey_len = NSS_CFI_OCF_BITS2BYTES(cri->cri_klen);
			ckey_len = NSS_CFI_OCF_BITS2BYTES(cri->cri_next->cri_klen);
		} else {
			continue;
		}

		if ((map->max_keylen == ckey_len) && (map->max_hashlen == akey_len)) {
			return algo;
		}
	}

	return NSS_CRYPTO_CMN_ALGO_MAX;
}

/*
 * nss_cfi_ocf_cri2algo()
 *	Gets an algorith number based on algos filled in cri
 */
enum nss_crypto_cmn_algo nss_cfi_ocf_cri2algo(struct cryptoini *cri)
{
	struct nss_cfi_ocf_algo_map *map = &cfi_algo[0];
	enum nss_crypto_cmn_algo algo;

	if (cri->cri_next) {
		return nss_cfi_ocf_cri2algo_aead(cri);
	}

	/*
	 * Only for non-AEAD algorithms
	 */
	for (algo = 0; algo < NSS_CRYPTO_CMN_ALGO_MAX; algo++, map++) {
		if (!map->algo[1] && (map->algo[0] == cri->cri_alg)) {
			return algo;
		}
	}

	return NSS_CRYPTO_CMN_ALGO_MAX;
}

/*
 * Generate a new software session.
 */
static int nss_cfi_ocf_newsession(device_t dev, uint32_t *sidp, struct cryptoini *cri)
{
	struct nss_cfi_crypto_info cfi_crypto;
	struct nss_cfi_ocf *sc = device_get_softc(dev);
	struct nss_crypto_session_data session_data;
	struct cryptoini *cip_ini = NULL;
	struct cryptoini *auth_ini = NULL;
	uint32_t session;
	int alg, status;

	nss_cfi_assert(sidp);
	nss_cfi_assert(cri);
	nss_cfi_assert(sc);

	alg = nss_cfi_ocf_cri2algo(cri);
	if (alg == NSS_CRYPTO_CMN_ALGO_MAX) {
		return EINVAL;
	}

	memset(&cfi_crypto, 0, sizeof(struct nss_cfi_crypto_info));
	memset(&session_data, 0, sizeof(struct nss_crypto_session_data));

	switch (cri->cri_alg) {
	case CRYPTO_MD5_HMAC:
	case CRYPTO_SHA1_HMAC:
		auth_ini = cri;
		auth_ini->cri_klen = NSS_CFI_OCF_BITS2BYTES(auth_ini->cri_klen); /* Convert len in bits to bytes */

		cip_ini = cri->cri_next;

		if (!cip_ini)
			break;

		cip_ini->cri_klen = NSS_CFI_OCF_BITS2BYTES(cip_ini->cri_klen); /* Convert len in bits to bytes */

		break;

	case CRYPTO_AES_CBC:
	case CRYPTO_3DES_CBC:
		cip_ini = cri;
		auth_ini = cri->cri_next;

		if (!auth_ini)
			break;

		auth_ini->cri_klen = NSS_CFI_OCF_BITS2BYTES(auth_ini->cri_klen); /* Convert len in bits to bytes */

		break;

	default:
		nss_cfi_err("%px: wrong algo %d\n", sc, alg);
		return EINVAL;
	}

	if (cip_ini) {
		session_data.cipher_key = cip_ini->cri_key;
	}

	if (auth_ini) {
		session_data.auth_key = auth_ini->cri_key;
		session_data.auth_keylen = auth_ini->cri_klen;
		cfi_crypto.hash_len = cfi_algo[alg].max_hashlen;
	}

	cfi_crypto.algo = alg;

	session_data.algo = alg;

	status = nss_crypto_session_alloc(sc->crypto, &session_data, &session);
	if (status < 0) {
		nss_cfi_err("%px: unable to allocate session: status %d\n", sc, cfi_crypto.sid);
		return EINVAL;
	}

	/*
	 * Here we store the index in lower 16 bits and populate
	 * algo in higher 16 bits of sidp structure. Then in process
	 * and process done, we will use the sidp passed by OCF to
	 * get back the algo
	 */
	*sidp = NSS_CFI_OCF_SESSION(session);

	/*
	 * Check if the index returned is within the bounds
	 */
	cfi_crypto.sid = *sidp;
	if (cfi_crypto.sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_crypto_session_free(sc->crypto, cfi_crypto.sid);
		return E2BIG;
	}

	*sidp |= NSS_CFI_OCF_ALGO_SHIFT(alg);

	memcpy(&gbl_crypto_info[cfi_crypto.sid], &cfi_crypto, sizeof(struct nss_cfi_crypto_info));
	return 0;
}

/*
 * Deallocate a session.
 */
static int nss_cfi_ocf_freesession(device_t dev, uint64_t tid)
{
	struct nss_cfi_ocf *sc = device_get_softc(dev);
	uint32_t sid = ((uint32_t) tid);
	uint16_t idx = sid & NSS_CFI_OCF_SES_MASK;

	sc->session_fn(sid);

	memset(&gbl_crypto_info[idx], 0, sizeof(struct nss_cfi_crypto_info));
	nss_cfi_info("freeing index = %d, sid = %d\n",idx, sid);

	nss_crypto_session_free(sc->crypto, idx);

	return 0;
}

/*
 * Cipher/Auth operation completion callback function
 */
static void nss_cfi_ocf_process_done(void *app_data, struct nss_crypto_hdr *ch, uint8_t error)
{
	struct cryptop *crp = NULL;
	struct cryptodesc *auth_crd = NULL;
	struct cryptodesc *cip_crd = NULL;
	struct cryptodesc *crd = NULL;
	enum nss_crypto_cmn_algo algo;
	uint32_t sid;

	nss_cfi_assert(ch);

	crp = (struct cryptop *)app_data;
	crd = crp->crp_desc;
	sid = ch->index;
	algo = NSS_CFI_OCF_GET_ALGO(crp->crp_sid);

	switch (crd->crd_alg) {
	case CRYPTO_SHA1_HMAC:
	case CRYPTO_MD5_HMAC:
		auth_crd = crd;
		cip_crd  = crd->crd_next;

		if (unlikely(!cip_crd)) {
			nss_cfi_dbg("Cipher info is not present in pkt\n");
			goto crypto_done;
		}

		break;

	case CRYPTO_AES_CBC:
	case CRYPTO_3DES_CBC:
		cip_crd = crd;
		auth_crd = crd->crd_next;

		if (unlikely(!auth_crd)) {
			nss_cfi_dbg("Authentication info is not present in pkt\n");
			goto crypto_done;
		}

		break;

	default:
		nss_cfi_err("%px: wrong cipher or auth algo %d\n", crp, crd->crd_alg);
		goto crypto_done;
	}

	if ((ch->op == NSS_CRYPTO_OP_DIR_AUTH_DEC) && (crp->crp_flags & CRYPTO_F_SKBUF)) {
		g_cfi_ocf.decrypt_fn((struct sk_buff *)crp->crp_buf, &gbl_crypto_info[sid]);
	}

	nss_crypto_hdr_free(g_cfi_ocf.crypto, ch);
	crypto_done(crp);

	return;

crypto_done:
	crp->crp_etype = ENOENT;
	crypto_done(crp);

	return;
}

/*
 * Process the request sent by OCF layer
 *
 * For Encryption + Authenticatin case, cipher desc should always be first desc
 * and auth desc should be second desc.
 * For Decryption + Authentication case, auth desc should always be fisrt desc
 * and cipher desc should be second desc.
 * In case of Authentication, buffer provided by framework should accomodate
 * extra 128 bytes for result dump.
 * In IPSEC case, IV is not provided by ipsec stack in case of encryption. Hence
 * we genrate a random IV at proper offset in buffer.
 */
static int nss_cfi_ocf_process(device_t dev, struct cryptop *crp, int hint)
{
	nss_crypto_req_callback_t process_done = nss_cfi_ocf_process_done;
	struct nss_cfi_ocf *sc = device_get_softc(dev);
	struct cryptodesc *auth_crd = NULL;
	struct cryptodesc *cip_crd = NULL;
	struct nss_crypto_hdr *ch = NULL;
	struct cryptodesc *crd = NULL;
	struct sk_buff *skb;
	uint32_t len, sid;
	int ivsize, algo;
	uint16_t cipher_len = 0;
	uint16_t auth_len = 0;
	uint32_t hash_len = 0;
	bool encrypt = false;
	bool decrypt = false;
	bool auth = false;
	int esp_offset = 0;

	nss_cfi_assert(crp);
	nss_cfi_assert(sc);

	crp->crp_etype = 0;
	len  = 0;

	sid = NSS_CFI_OCF_SESSION(crp->crp_sid);
	if (sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("%px: session id %d not valid\n", crp, sid);
		goto crypto_done;
	}

	algo = NSS_CFI_OCF_GET_ALGO(crp->crp_sid);

	/*
	 * finding out cipher and auth descriptor.
	 */
	crd = crp->crp_desc;
	nss_cfi_assert(crd);

	switch (crd->crd_alg) {
	case CRYPTO_MD5_HMAC:
	case CRYPTO_SHA1_HMAC:
		auth_crd = crd;
		cip_crd  = crd->crd_next;

		if (unlikely(!cip_crd))
			goto crypto_done;

		break;

	case CRYPTO_AES_CBC:
	case CRYPTO_3DES_CBC:
		cip_crd = crd;
		auth_crd = crd->crd_next;

		if (unlikely(!auth_crd))
			goto crypto_done;

		break;

	default:
		nss_cfi_err("%px: wrong cipher or auth algo %d\n", crp, crd->crd_alg);
		goto crypto_done;
	}

	skb = nss_cfi_ocf_get_skb(crp);
	if (!skb)
		goto crypto_done;

	ivsize = cfi_algo[algo].max_ivlen;

	/*
	 *  Allocate nss_crypto_buffer
	 */
	ch = nss_crypto_hdr_alloc(sc->crypto, sid, 1, 1, ivsize, hash_len, false);
	if (!ch) {
		nss_cfi_dbg("%px: not able to allocate crypto buffer\n");
		goto crypto_done;
	}

	if (cip_crd) {
		uint32_t *iv_addr;
		ivsize = cfi_algo[algo].max_ivlen;

		nss_cfi_assert(!(cip_crd->crd_flags & CRD_F_IV_EXPLICIT));

		if (cip_crd->crd_flags & CRD_F_ENCRYPT) {
			if (!(cip_crd->crd_flags & CRD_F_IV_PRESENT)) {
				get_random_bytes((skb->data + cip_crd->crd_inject), ivsize);
			}
			encrypt = true;
		} else {
			decrypt = true;
		}

		cipher_len = cip_crd->crd_len;
		iv_addr = nss_crypto_hdr_get_iv(ch);
		memcpy(iv_addr, skb->data + cip_crd->crd_inject, ivsize);

		nss_cfi_dbg("enc=(%d), dec=(%d), ciph_len(%d), ciph_skip(%d), inject(%d), iv_addr(%px)\n",
				encrypt, decrypt, cipher_len, cip_crd->crd_skip, cip_crd->crd_inject, iv_addr);
	}

	if (auth_crd) {
		auth = true;
		hash_len = (auth_crd->crd_mlen == 0) ?
				cfi_algo[algo].max_hashlen : auth_crd->crd_mlen;

		auth_len = auth_crd->crd_len;
		gbl_crypto_info[ch->index].hash_len = hash_len;
	}

	ch->hmac_len = hash_len;

	esp_offset = skb->len - (hash_len + cipher_len + ivsize + ESP_HDR_LEN);

	if (encrypt && auth) {
		nss_crypto_hdr_set_skip(ch, cip_crd->crd_skip - esp_offset);
		nss_crypto_hdr_set_op(ch, NSS_CRYPTO_OP_DIR_ENC_AUTH);
	} else if (decrypt && auth) {
		nss_crypto_hdr_set_skip(ch, cip_crd->crd_skip);
		nss_crypto_hdr_set_op(ch, NSS_CRYPTO_OP_DIR_AUTH_DEC);
	} else {
		nss_cfi_err("%px: invalid operation\n", ch);
	}

	/*
	 * For encryption, skb has outer header attached at the start
	 * of the packet and hash len reserved at the end of the packet
	 * EIP expects the packet to start at ESP header. And data len
	 * in EIP is expected to exclude the hash len. Hence before
	 * sending the skb for mapping into crypto header structure
	 * we move skb->data by ip header length and reduce the length
	 * by (hash_len + ip header length). We will restore both
	 * skb->data and len after mapping is done.
	 */
	if (encrypt) {
		len = skb->len;
		skb->data += esp_offset;
		skb->len -= (hash_len + esp_offset);
	}

	nss_crypto_hdr_map_skb(ch, skb);

	if (encrypt) {
		skb->data -= esp_offset;
		skb->len = len;
	}

	/*
	 * If packet is for encryption call the registered trap function
	 */
	if ((cip_crd->crd_flags & CRD_F_ENCRYPT) && (crp->crp_flags & CRYPTO_F_SKBUF)) {
		if (sc->encrypt_fn((struct sk_buff *)crp->crp_buf, &gbl_crypto_info[ch->index])) {
			goto no_offload;
		}

		/*
		 * Call offload function, if offload is disabled (non zero return value) continue normal transformation.
		 * else set EINPROGRESS in crp to indicate packet has been consumed by crypto.
		 */
		if (sc->offload_fn((struct sk_buff *)crp->crp_buf, &gbl_crypto_info[ch->index])) {
			goto no_offload;
		}

		crp->crp_etype = EINPROGRESS;
		nss_crypto_hdr_free(sc->crypto, ch);
		crypto_done(crp);
		return 0;
	}

no_offload:
	/*
	 *  Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, ch, process_done, crp)) {
		nss_crypto_hdr_free(sc->crypto, ch);
		goto crypto_done;
	}

	return 0;

crypto_done:
	crp->crp_etype = ENOENT;
	crypto_done(crp);

	return 0;
}

/*
 * crypto device methods
 */
device_method_t nss_cfi_ocf_methods = {
	DEVMETHOD(cryptodev_newsession, nss_cfi_ocf_newsession),
	DEVMETHOD(cryptodev_freesession, nss_cfi_ocf_freesession),
	DEVMETHOD(cryptodev_process, nss_cfi_ocf_process),
};

/*
 * register crypto core with the CFI
 */
void nss_cfi_ocf_register(void *app_data, struct nss_crypto_user *user)
{
	struct nss_cfi_ocf *sc = &g_cfi_ocf;

	/* register algorithms with the framework */
	if (nss_crypto_algo_is_supp(NSS_CRYPTO_CMN_ALGO_AES128_CBC)) {
		crypto_register(sc->cid, CRYPTO_AES_CBC, 0, 0);
		nss_cfi_info("%px: crypto_register for AES_CBC algo\n", sc);
	}

	if (nss_crypto_algo_is_supp(NSS_CRYPTO_CMN_ALGO_3DES_CBC)) {
		crypto_register(sc->cid, CRYPTO_3DES_CBC, 0, 0);
		nss_cfi_info("%px: crypto_register for 3DES_CBC algo\n", sc);
	}

	if (nss_crypto_algo_is_supp(NSS_CRYPTO_CMN_ALGO_SHA160_HMAC)) {
		crypto_register(sc->cid, CRYPTO_SHA1_HMAC, 0, 0);
		nss_cfi_info("%px: crypto_register for SHA1_HMAC algo\n", sc);
	}

	if (nss_crypto_algo_is_supp(NSS_CRYPTO_CMN_ALGO_MD5_HMAC)) {
		crypto_register(sc->cid, CRYPTO_MD5_HMAC, 0, 0);
		nss_cfi_info("%px: crypto_register for MD5_HMAC algo\n", sc);
	}
}

/*
 * Unregister crypto core with OCF CFI layer
 */
void nss_cfi_ocf_unregister(void *app_data, struct nss_crypto_user *user)
{
	nss_cfi_info("unregister nss_cfi_ocf\n");
}

/*
 * Register IPsec trap handlers with CFI_OCF
 */
void nss_cfi_ocf_register_ipsec(nss_cfi_data_trap_t encrypt_fn,
				nss_cfi_data_trap_t decrypt_fn,
				nss_cfi_session_trap_t session_fn,
				nss_cfi_data_trap_t offload_fn)
{
	struct nss_cfi_ocf *sc = &g_cfi_ocf;
	nss_cfi_data_trap_t encrypt;
	nss_cfi_data_trap_t decrypt;
	nss_cfi_data_trap_t offload;
	nss_cfi_session_trap_t session;

	encrypt = xchg(&sc->encrypt_fn, encrypt_fn);
	decrypt = xchg(&sc->decrypt_fn, decrypt_fn);
	session = xchg(&sc->session_fn, session_fn);
	offload = xchg(&sc->offload_fn, offload_fn);
}

/*
 * Unregister IPsec trap handlers with CFI_OCF
 */
void nss_cfi_ocf_unregister_ipsec(void)
{
	struct nss_cfi_ocf *sc = &g_cfi_ocf;
	nss_cfi_data_trap_t encrypt;
	nss_cfi_data_trap_t decrypt;
	nss_cfi_data_trap_t offload;
	nss_cfi_session_trap_t session;

	nss_cfi_info("Unregistering IPsec trap handlers\n");

	offload = xchg(&sc->offload_fn, nss_cfi_ocf_offload_trap);
	session = xchg(&sc->session_fn, nss_cfi_ocf_session_trap);
	encrypt = xchg(&sc->encrypt_fn, nss_cfi_ocf_encrypt_trap);
	decrypt = xchg(&sc->decrypt_fn, nss_cfi_ocf_decrypt_trap);

	msleep(10000);
}

/*
 * Initialtizing crypto core layer
 */
int nss_cfi_ocf_init(void)
{
	struct nss_cfi_ocf *sc = &g_cfi_ocf;
	struct nss_crypto_user_ctx *user_ctx;
	nss_cfi_data_trap_t encrypt;
	nss_cfi_data_trap_t decrypt;
	nss_cfi_data_trap_t offload;
	nss_cfi_session_trap_t session;

	softc_device_init(sc, NSS_CFI_DRV_NAME, 0, nss_cfi_ocf_methods);

	sc->cid = crypto_get_driverid(softc_get_device(sc), CRYPTOCAP_F_HARDWARE);
	if (sc->cid < 0) {
		nss_cfi_err("%px: could not get crypto driver id\n", sc);
		return -1;
	}

	user_ctx = vzalloc(sizeof(struct nss_crypto_user_ctx));
	if (!user_ctx) {
		return -ENOMEM;
	}

	sc->user_ctx = user_ctx;

	user_ctx->attach = nss_cfi_ocf_register;
	user_ctx->detach = nss_cfi_ocf_unregister;
	strlcpy(user_ctx->name, "nss_cfi_ocf", NSS_CRYPTO_NAME_MAX);
	user_ctx->hdr_pool_sz = 1024;
	user_ctx->default_hdr_sz = 512;
	user_ctx->timeout_ticks = 1;

	encrypt = xchg(&sc->encrypt_fn, nss_cfi_ocf_encrypt_trap);
	decrypt = xchg(&sc->decrypt_fn, nss_cfi_ocf_decrypt_trap);
	session = xchg(&sc->session_fn, nss_cfi_ocf_session_trap);
	offload = xchg(&sc->offload_fn, nss_cfi_ocf_offload_trap);

	sc->crypto = nss_crypto_register_user(user_ctx, "nss_cfi_ocf");
	if (!sc->crypto) {
		nss_cfi_err("Failed to register NSS CRYPTOAPI driver\n");
		vfree(user_ctx);
		return -ENODEV;
	}

	return 0;
}

/*
 * De-Initialize OCF CFI layer
 */
void nss_cfi_ocf_exit(void)
{
	struct nss_cfi_ocf *sc = &g_cfi_ocf;

	if (sc->crypto)
		nss_crypto_unregister_user(sc->crypto);

	nss_cfi_info("exiting nss_cfi_ocf\n");
	kfree(sc->user_ctx);
	crypto_unregister_all(g_cfi_ocf.cid);
}

EXPORT_SYMBOL(nss_cfi_ocf_register_ipsec);
EXPORT_SYMBOL(nss_cfi_ocf_unregister_ipsec);

module_init(nss_cfi_ocf_init);
module_exit(nss_cfi_ocf_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Interface module between OCF and NSS crypto");
