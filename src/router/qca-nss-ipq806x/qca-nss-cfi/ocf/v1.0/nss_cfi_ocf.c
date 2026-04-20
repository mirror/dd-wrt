/* Copyright (c) 2014,2015-2020 The Linux Foundation. All rights reserved.
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
#include <cryptodev.h>

#include <nss_api_if.h>
#include <nss_crypto_if.h>
#include <nss_cfi_if.h>
#include <nss_cfi_ocf.h>

static struct nss_cfi_ocf g_cfi_ocf = {{{0}}};

static struct nss_cfi_crypto_info gbl_crypto_info[NSS_CRYPTO_MAX_IDXS];

/*
 * cfi supported cipher and auth algorithms and max key, iv and hash lengths.
 * Note: Algorithm definitions start from 1 and hence, we need one additional
 * slot in the array to accommodate all algos. Slot 0 will be empty and unused.
 */
static struct nss_cfi_ocf_algo cfi_algo[CRYPTO_ALGORITHM_MAX + 1] = {
	[CRYPTO_AES_CBC] = 	{
					NSS_CRYPTO_CIPHER_AES_CBC,
					NSS_CRYPTO_MAX_KEYLEN_AES,
					NSS_CRYPTO_MAX_IVLEN_AES,
					NSS_CRYPTO_MAX_BLKLEN_AES,
					0,
					NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER
			   	},
	[CRYPTO_SHA1_HMAC] = 	{
					NSS_CRYPTO_AUTH_SHA1_HMAC,
					NSS_CRYPTO_MAX_KEYLEN_SHA1,
					0,
					0,
					NSS_CRYPTO_MAX_HASHLEN_SHA1,
					NSS_CFI_OCF_ALGO_TYPE_IS_AUTH
				},
	[CRYPTO_DES_CBC] =	{
					NSS_CRYPTO_CIPHER_DES,
					(NSS_CRYPTO_MAX_KEYLEN_DES)/3,
					NSS_CRYPTO_MAX_IVLEN_DES,
					NSS_CRYPTO_MAX_BLKLEN_DES,
					0,
					NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER
				},
	[CRYPTO_3DES_CBC] =	{
					NSS_CRYPTO_CIPHER_DES,
					NSS_CRYPTO_MAX_KEYLEN_DES,
					NSS_CRYPTO_MAX_IVLEN_DES,
					NSS_CRYPTO_MAX_BLKLEN_DES,
					0,
					NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER
				},
	[CRYPTO_NULL_HMAC] =    {
					NSS_CRYPTO_AUTH_NULL,
					0,
					0,
					0,
					0,
					NSS_CFI_OCF_ALGO_TYPE_IS_AUTH
				},
	[CRYPTO_NULL_CBC] =     {
					NSS_CRYPTO_CIPHER_NULL,
					0,
					0,
					NSS_CRYPTO_MAX_BLKLEN_NULL,
					0,
					NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER
				}
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
 * Returns true if given skb can be processed by crypto
 */
static bool nss_cfi_ocf_chk_skb_valid(struct sk_buff *skb)
{

	/* Skb fragmentation support unimplemented */
	if (skb_shinfo(skb)->nr_frags || skb_has_frag_list(skb)) {
		nss_cfi_err("SKB has fragments, crypto cannot process these buffers\n");
		return false;
	}

	return true;
}

/*
 * Returns success if cfi can handle the type of buffer sent by OCF
 */
static bool nss_cfi_ocf_get_buf(struct cryptop *crp, uint8_t **data, uint32_t *len)
{
	struct sk_buff *skb = NULL;

	/* IOV buffer type support unimplemented */
	nss_cfi_assert(!(crp->crp_flags & CRYPTO_F_IOV));

	if (crp->crp_flags & CRYPTO_F_SKBUF) {
		skb = (struct sk_buff *)crp->crp_buf;
		*data = skb->data;
		*len = skb->len;

		return nss_cfi_ocf_chk_skb_valid(skb);
	}

	*data = crp->crp_buf;
	*len  = crp->crp_ilen;

	return true;
}

/*
 * Generate a new software session.
 */
static int nss_cfi_ocf_newsession(device_t dev, uint32_t *sidp, struct cryptoini *cri)
{
	struct nss_cfi_crypto_info cfi_crypto = {.cipher_algo = NSS_CRYPTO_CIPHER_NULL,
						 .auth_algo = NSS_CRYPTO_AUTH_NULL};
	struct nss_cfi_ocf *sc = device_get_softc(dev);
	struct nss_crypto_key cip = {0};
	struct nss_crypto_key auth = {0};
	struct nss_crypto_key *cip_ptr = NULL;
	struct nss_crypto_key *auth_ptr = NULL;
	struct cryptoini *cip_ini = NULL;
	struct cryptoini *auth_ini = NULL;
	nss_crypto_status_t status;
	int alg;

	nss_cfi_assert(sidp);
	nss_cfi_assert(cri);
	nss_cfi_assert(sc);

	/*
	 * Check that the first cri parameter has a valid key length
	 * based on the crypto algorithm.
	 */
	alg = cri->cri_alg;
	cri->cri_klen = NSS_CRYPTO_BITS2BYTES(cri->cri_klen);
	if (cri->cri_klen > cfi_algo[alg].max_keylen) {
		nss_cfi_err("wrong keylen %d max keylen %d alg %d\n",
				cri->cri_klen, cfi_algo[alg].max_keylen, alg);
		return EINVAL;
	}

	switch (cfi_algo[alg].flag) {
	case NSS_CFI_OCF_ALGO_TYPE_IS_AUTH:
		auth_ini = cri;
		cip_ini = cri->cri_next;

		/* Only have auth, no cipher algorithm. */
		if (!cip_ini) {
			break;
		}

		if (cfi_algo[cip_ini->cri_alg].flag != NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER) {
			nss_cfi_err("invalid algo %d\n", cip_ini->cri_alg);
			return EINVAL;
		}

		cip_ini->cri_klen = NSS_CRYPTO_BITS2BYTES(cip_ini->cri_klen);
		if (cip_ini->cri_klen > cfi_algo[cip_ini->cri_alg].max_keylen) {
			nss_cfi_err("wrong keylen %d max keylen %d alg %d \n",
					cri->cri_klen, cfi_algo[cip_ini->cri_alg].max_keylen, cip_ini->cri_alg);
			return EINVAL;
		}

		break;

	case NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER:
		cip_ini = cri;
		auth_ini = cri->cri_next;

		/* Only have cipher, no auth algorithm. */
		if (!auth_ini) {
			break;
		}

		if (cfi_algo[auth_ini->cri_alg].flag != NSS_CFI_OCF_ALGO_TYPE_IS_AUTH) {
			nss_cfi_err("invalid algo %d\n", auth_ini->cri_alg);
			return EINVAL;
		}

		auth_ini->cri_klen = NSS_CRYPTO_BITS2BYTES(auth_ini->cri_klen);
		if (auth_ini->cri_klen > cfi_algo[auth_ini->cri_alg].max_keylen) {
			nss_cfi_err("wrong keylen %d max keylen %d alg %d\n",
					cri->cri_klen, cfi_algo[auth_ini->cri_alg].max_keylen, auth_ini->cri_alg);
			return EINVAL;
		}

		break;

	default:
		nss_cfi_err("wrong algo %d\n", alg);
		return EINVAL;
	}

	if (cip_ini) {
		cip.key_len = cip_ini->cri_klen;
		cip.algo = cfi_algo[cip_ini->cri_alg].core_algo;
		cip.key = cip_ini->cri_key;

		cfi_crypto.cipher_algo = cip.algo;

		cip_ptr = &cip;

		nss_cfi_info("cipher algo received %d, sent %d\n",cip_ini->cri_alg, cip.algo);
	}

	if (auth_ini) {
		auth.key_len = auth_ini->cri_klen;
		auth.algo = cfi_algo[auth_ini->cri_alg].core_algo;
		auth.key = auth_ini->cri_key;

		cfi_crypto.auth_algo = auth.algo;
		cfi_crypto.hash_len = cfi_algo[auth_ini->cri_alg].max_hashlen;

		auth_ptr = &auth;

		nss_cfi_info("auth algo received %d, sent %d\n",auth_ini->cri_alg, auth.algo);
	}

	status = nss_crypto_session_alloc(sc->crypto, cip_ptr, auth_ptr, sidp);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("unable to allocate session: status %d\n", status);
		return EINVAL;
	}

	cfi_crypto.sid = *sidp;
	memcpy(&gbl_crypto_info[*sidp], &cfi_crypto, sizeof(struct nss_cfi_crypto_info));

	return 0;
}

/*
 * Deallocate a session.
 */
static int nss_cfi_ocf_freesession(device_t dev, uint64_t tid)
{
	struct nss_cfi_ocf *sc = device_get_softc(dev);
	uint32_t sid = ((uint32_t) tid) & NSS_CFI_OCF_SES_MASK;
	nss_crypto_status_t status;

	sc->session_fn(sid);

	memset(&gbl_crypto_info[sid], 0, sizeof(struct nss_cfi_crypto_info));
	nss_cfi_info("freeing index %d\n",sid);

	status = nss_crypto_session_free(sc->crypto, sid);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("unable to free session: idx %d\n", sid);
		return EINVAL;
	}

	return 0;
}

/*
 * Cipher/Auth operation completion callback function
 */
static void nss_cfi_ocf_process_done(struct nss_crypto_buf *buf)
{
	struct cryptop *crp = NULL;
	uint8_t *hash_addr;
	uint16_t hash_len;
	uint32_t len;
	uint8_t *data;
	struct cryptodesc *auth_crd = NULL;
	struct cryptodesc *cip_crd = NULL;
	struct cryptodesc *crd = NULL;
	uint32_t req_type = nss_crypto_get_reqtype(buf->session_idx);

	nss_cfi_assert(buf);

	crp = (struct cryptop *)buf->cb_ctx;
	crd = crp->crp_desc;

	switch (cfi_algo[crd->crd_alg].flag) {
	case NSS_CFI_OCF_ALGO_TYPE_IS_AUTH:
		auth_crd = crd;
		cip_crd  = crd->crd_next;
		break;

	case NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER:
		cip_crd = crd;
		auth_crd = crd->crd_next;
		break;

	default:
		nss_cfi_err("wrong cipher or auth algo %d\n", crd->crd_alg);
	}

	if (auth_crd && cip_crd) {
		hash_addr = nss_crypto_get_hash_addr(buf);
		hash_len = (auth_crd->crd_mlen == 0) ? cfi_algo[auth_crd->crd_alg].max_hashlen : auth_crd->crd_mlen;

		nss_cfi_ocf_get_buf(crp, &data, &len);

		if (cip_crd->crd_flags & CRD_F_ENCRYPT) {
			memcpy(data + auth_crd->crd_inject, hash_addr, hash_len);
		} else {
			if (memcmp(data + auth_crd->crd_inject, hash_addr, hash_len)) {
				nss_cfi_err("Authentication failed\n");
				crp->crp_etype = ENOENT;
				goto done;
			}
		}
	}

	if (((req_type & NSS_CRYPTO_REQ_TYPE_DECRYPT) == NSS_CRYPTO_REQ_TYPE_DECRYPT) && (crp->crp_flags & CRYPTO_F_SKBUF)) {
		g_cfi_ocf.decrypt_fn((struct sk_buff *)crp->crp_buf, &gbl_crypto_info[buf->session_idx]);
	}

done:
	nss_crypto_buf_free(g_cfi_ocf.crypto, buf);
	crypto_done(crp);
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
	struct nss_cfi_ocf *sc = device_get_softc(dev);
	struct nss_crypto_buf *buf;
	struct cryptodesc *auth_crd = NULL;
	struct cryptodesc *cip_crd = NULL;
	struct cryptodesc *crd = NULL;
	uint32_t len;
	uint32_t sid;
	uint32_t hash_len = 0;
	uint8_t *data;
	int flag = 0;
	int ivsize;
	uint16_t cipher_len = 0, auth_len = 0;
	struct nss_crypto_params params;
	nss_crypto_status_t status;

	nss_cfi_assert(crp);
	nss_cfi_assert(sc);

	crp->crp_etype = 0;
	len  = 0;

	sid = NSS_CFI_OCF_SESSION(crp->crp_sid);
	if (sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("session id %d not valid\n", sid);
		crp->crp_etype = ENOENT;
		crypto_done(crp);

		return 0;
	}

	/*
	 * finding out cipher and auth descriptor.
	 */
	crd = crp->crp_desc;
	nss_cfi_assert(crd);

	switch (cfi_algo[crd->crd_alg].flag) {
	case NSS_CFI_OCF_ALGO_TYPE_IS_AUTH:
		auth_crd = crd;
		cip_crd  = crd->crd_next;

		if (unlikely(!cip_crd)) {
			nss_cfi_dbg("Cipher info is not present in pkt\n");
			crp->crp_etype = ENOENT;
			crypto_done(crp);

			return 0;
		}

		nss_cfi_assert((cfi_algo[cip_crd->crd_alg].flag == NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER));
		nss_cfi_assert((!(cip_crd->crd_flags & CRD_F_ENCRYPT)));

		break;

	case NSS_CFI_OCF_ALGO_TYPE_IS_CIPHER:
		cip_crd = crd;
		auth_crd = crd->crd_next;

		if (unlikely(!auth_crd)) {
			nss_cfi_dbg("Authentication info is not present in pkt\n");
			crp->crp_etype = ENOENT;
			crypto_done(crp);

			return 0;
		}

		nss_cfi_assert((cfi_algo[auth_crd->crd_alg].flag == NSS_CFI_OCF_ALGO_TYPE_IS_AUTH));
		nss_cfi_assert((crd->crd_flags & CRD_F_ENCRYPT));

		break;

	default:
		nss_cfi_err("wrong cipher or auth algo %d\n",crd->crd_alg);
		nss_cfi_assert(0);

		crp->crp_etype = ENOENT;
		crypto_done(crp);

		return 0;
	}

	if (nss_cfi_ocf_get_buf(crp, &data, &len) == false) {
		nss_cfi_dbg("not able to get buffer parameters\n");

		crp->crp_etype = ENOENT;
		crypto_done(crp);

		return 0;
	}

	/*
	 *  Allocate nss_crypto_buffer
	 */
	buf = nss_crypto_buf_alloc(sc->crypto);
	if (buf == NULL) {
		nss_cfi_dbg("not able to allocate crypto buffer\n");
		crp->crp_etype = ENOENT;
		crypto_done(crp);

		return 0;
	}

	/*
	 *  filling up buffer entries
	 */

	nss_crypto_set_cb(buf, nss_cfi_ocf_process_done, crp);
	nss_crypto_set_session_idx(buf, NSS_CFI_OCF_SESSION(crp->crp_sid));

	memset(&params, 0, sizeof(struct nss_crypto_params));

	if (cip_crd) {
		uint8_t *iv_addr;
		flag = NSS_CRYPTO_REQ_TYPE_DECRYPT;
		ivsize = cfi_algo[cip_crd->crd_alg].max_ivlen;

		nss_cfi_assert(!(cip_crd->crd_flags & CRD_F_IV_EXPLICIT));

		if (cip_crd->crd_flags & CRD_F_ENCRYPT) {
			if (!(cip_crd->crd_flags & CRD_F_IV_PRESENT)) {
				get_random_bytes((data + cip_crd->crd_inject), ivsize);
			}
			flag = NSS_CRYPTO_REQ_TYPE_ENCRYPT;
		}

		cipher_len = cip_crd->crd_len;
		iv_addr = nss_crypto_get_ivaddr(buf);
		memcpy(iv_addr, data + cip_crd->crd_inject, ivsize);

		params.cipher_skip = cip_crd->crd_skip;

		nss_cfi_dbg("cipher len %d cipher skip %d iv_addr %px\n",
				cipher_len, params.cipher_skip, iv_addr);
	}

	if (auth_crd) {
		flag |= NSS_CRYPTO_REQ_TYPE_AUTH;

		hash_len = (auth_crd->crd_mlen == 0) ?
				cfi_algo[auth_crd->crd_alg].max_hashlen : auth_crd->crd_mlen;

		auth_len = auth_crd->crd_len;
		gbl_crypto_info[buf->session_idx].hash_len = hash_len;
		params.auth_skip = auth_crd->crd_skip;

		nss_cfi_dbg("auth len %d auth skip %d \n",
				auth_len, params.auth_skip);
	}

	/*
	 * If packet is for encryption call the registered trap function
	 */
	if ((cip_crd->crd_flags & CRD_F_ENCRYPT) && (crp->crp_flags & CRYPTO_F_SKBUF)) {
		sc->encrypt_fn((struct sk_buff *)crp->crp_buf, &gbl_crypto_info[buf->session_idx]);
	}

	params.req_type = flag;

	/*
	 * The physical buffer data length provided to crypto will include
	 * space for authentication hash
	 */
	nss_crypto_set_data(buf, data, data, len + hash_len);
	nss_crypto_set_transform_len(buf, cipher_len, auth_len);

	/*
	 * Update the crypto session data
	 */
	status = nss_crypto_session_update(sc->crypto, buf->session_idx, &params);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_dbg("Invalid crypto session parameters\n");
		nss_crypto_buf_free(sc->crypto, buf);

		crp->crp_etype = ENOENT;
		crypto_done(crp);
		return 0;
	}

	/*
	 *  Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, buf) !=  NSS_CRYPTO_STATUS_OK) {
		nss_cfi_dbg("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);

		crp->crp_etype = ENOENT;
		crypto_done(crp);
	}

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
static nss_crypto_user_ctx_t nss_cfi_ocf_register(nss_crypto_handle_t crypto)
{
	struct nss_cfi_ocf *sc = &g_cfi_ocf;
	int i;

	sc->crypto = crypto;

	/* register algorithms with the framework */
	for (i = 1; i <= CRYPTO_ALGORITHM_MAX; i++) {
		if (cfi_algo[i].core_algo) {
			nss_cfi_info("registering ALGO %d with OCF\n",i);
			crypto_register(sc->cid, i, 0, 0);
		}
	}

	return sc;
}

/*
 * Unregister crypto core with OCF CFI layer
 */
static void nss_cfi_ocf_unregister(nss_crypto_user_ctx_t cfi)
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
	nss_cfi_data_trap_t encrypt;
	nss_cfi_data_trap_t decrypt;
	nss_cfi_data_trap_t offload;
	nss_cfi_session_trap_t session;

	softc_device_init(sc, NSS_CFI_DRV_NAME, 0, nss_cfi_ocf_methods);

	sc->cid = crypto_get_driverid(softc_get_device(sc), CRYPTOCAP_F_HARDWARE);
	if (sc->cid < 0) {
		nss_cfi_err("could not get crypto driver id\n");
		return -1;
	}

	encrypt = xchg(&sc->encrypt_fn, nss_cfi_ocf_encrypt_trap);
	decrypt = xchg(&sc->decrypt_fn, nss_cfi_ocf_decrypt_trap);
	session = xchg(&sc->session_fn, nss_cfi_ocf_session_trap);
	offload = xchg(&sc->offload_fn, nss_cfi_ocf_offload_trap);

	nss_crypto_register_user(nss_cfi_ocf_register, nss_cfi_ocf_unregister, "nss_cfi_ocf");

	return 0;
}

/*
 * De-Initialize OCF CFI layer
 */
void nss_cfi_ocf_exit(void)
{
	nss_cfi_info("exiting nss_cfi_ocf\n");
	crypto_unregister_all(g_cfi_ocf.cid);
}

EXPORT_SYMBOL(nss_cfi_ocf_register_ipsec);
EXPORT_SYMBOL(nss_cfi_ocf_unregister_ipsec);

module_init(nss_cfi_ocf_init);
module_exit(nss_cfi_ocf_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Interface module between OCF and NSS crypto");
