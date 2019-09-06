#ifndef _BACKPORT_CRYPTO_SKCIPHER_H
#define _BACKPORT_CRYPTO_SKCIPHER_H
#include_next <crypto/skcipher.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
/**
 *	struct skcipher_request - Symmetric key cipher request
 *	@cryptlen: Number of bytes to encrypt or decrypt
 *	@iv: Initialisation Vector
 *	@src: Source SG list
 *	@dst: Destination SG list
 *	@base: Underlying async request request
 *	@__ctx: Start of private context data
 */
#define skcipher_request LINUX_BACKPORT(skcipher_request)
struct skcipher_request {
	unsigned int cryptlen;

	u8 *iv;

	struct scatterlist *src;
	struct scatterlist *dst;

	struct crypto_async_request base;

	void *__ctx[] CRYPTO_MINALIGN_ATTR;
};

#define crypto_skcipher LINUX_BACKPORT(crypto_skcipher)
struct crypto_skcipher {
	int (*setkey)(struct crypto_skcipher *tfm, const u8 *key,
	              unsigned int keylen);
	int (*encrypt)(struct skcipher_request *req);
	int (*decrypt)(struct skcipher_request *req);

	unsigned int ivsize;
	unsigned int reqsize;
	unsigned int keysize;

	struct crypto_tfm base;
};

#ifndef SKCIPHER_REQUEST_ON_STACK
#define SKCIPHER_REQUEST_ON_STACK(name, tfm) \
	char __##name##_desc[sizeof(struct skcipher_request) + \
		crypto_skcipher_reqsize(tfm)] CRYPTO_MINALIGN_ATTR; \
	struct skcipher_request *name = (void *)__##name##_desc
#endif

/**
 * DOC: Symmetric Key Cipher API
 *
 * Symmetric key cipher API is used with the ciphers of type
 * CRYPTO_ALG_TYPE_SKCIPHER (listed as type "skcipher" in /proc/crypto).
 *
 * Asynchronous cipher operations imply that the function invocation for a
 * cipher request returns immediately before the completion of the operation.
 * The cipher request is scheduled as a separate kernel thread and therefore
 * load-balanced on the different CPUs via the process scheduler. To allow
 * the kernel crypto API to inform the caller about the completion of a cipher
 * request, the caller must provide a callback function. That function is
 * invoked with the cipher handle when the request completes.
 *
 * To support the asynchronous operation, additional information than just the
 * cipher handle must be supplied to the kernel crypto API. That additional
 * information is given by filling in the skcipher_request data structure.
 *
 * For the symmetric key cipher API, the state is maintained with the tfm
 * cipher handle. A single tfm can be used across multiple calls and in
 * parallel. For asynchronous block cipher calls, context data supplied and
 * only used by the caller can be referenced the request data structure in
 * addition to the IV used for the cipher request. The maintenance of such
 * state information would be important for a crypto driver implementer to
 * have, because when calling the callback function upon completion of the
 * cipher operation, that callback function may need some information about
 * which operation just finished if it invoked multiple in parallel. This
 * state information is unused by the kernel crypto API.
 */

#define __crypto_skcipher_cast LINUX_BACKPORT(__crypto_skcipher_cast)
static inline struct crypto_skcipher *__crypto_skcipher_cast(
	struct crypto_tfm *tfm)
{
	return container_of(tfm, struct crypto_skcipher, base);
}

/**
 * crypto_alloc_skcipher() - allocate symmetric key cipher handle
 * @alg_name: is the cra_name / name or cra_driver_name / driver name of the
 *	      skcipher cipher
 * @type: specifies the type of the cipher
 * @mask: specifies the mask for the cipher
 *
 * Allocate a cipher handle for an skcipher. The returned struct
 * crypto_skcipher is the cipher handle that is required for any subsequent
 * API invocation for that skcipher.
 *
 * Return: allocated cipher handle in case of success; IS_ERR() is true in case
 *	   of an error, PTR_ERR() returns the error code.
 */
#define crypto_alloc_skcipher LINUX_BACKPORT(crypto_alloc_skcipher)
struct crypto_skcipher *crypto_alloc_skcipher(const char *alg_name,
					      u32 type, u32 mask);

#define crypto_skcipher_tfm LINUX_BACKPORT(crypto_skcipher_tfm)
static inline struct crypto_tfm *crypto_skcipher_tfm(
	struct crypto_skcipher *tfm)
{
	return &tfm->base;
}

/**
 * crypto_free_skcipher() - zeroize and free cipher handle
 * @tfm: cipher handle to be freed
 */
#define crypto_free_skcipher LINUX_BACKPORT(crypto_free_skcipher)
static inline void crypto_free_skcipher(struct crypto_skcipher *tfm)
{
	crypto_destroy_tfm(tfm, crypto_skcipher_tfm(tfm));
}

/**
 * crypto_has_skcipher() - Search for the availability of an skcipher.
 * @alg_name: is the cra_name / name or cra_driver_name / driver name of the
 *	      skcipher
 * @type: specifies the type of the cipher
 * @mask: specifies the mask for the cipher
 *
 * Return: true when the skcipher is known to the kernel crypto API; false
 *	   otherwise
 */
#define crypto_has_skcipher LINUX_BACKPORT(crypto_has_skcipher)
static inline int crypto_has_skcipher(const char *alg_name, u32 type,
					u32 mask)
{
	return crypto_has_alg(alg_name, crypto_skcipher_type(type),
			      crypto_skcipher_mask(mask));
}

#define crypto_skcipher_driver_name LINUX_BACKPORT(crypto_skcipher_driver_name)
static inline const char *crypto_skcipher_driver_name(
	struct crypto_skcipher *tfm)
{
	return crypto_tfm_alg_driver_name(crypto_skcipher_tfm(tfm));
}

/**
 * crypto_skcipher_ivsize() - obtain IV size
 * @tfm: cipher handle
 *
 * The size of the IV for the skcipher referenced by the cipher handle is
 * returned. This IV size may be zero if the cipher does not need an IV.
 *
 * Return: IV size in bytes
 */
#define crypto_skcipher_ivsize LINUX_BACKPORT(crypto_skcipher_ivsize)
static inline unsigned int crypto_skcipher_ivsize(struct crypto_skcipher *tfm)
{
	return tfm->ivsize;
}

/**
 * crypto_skcipher_blocksize() - obtain block size of cipher
 * @tfm: cipher handle
 *
 * The block size for the skcipher referenced with the cipher handle is
 * returned. The caller may use that information to allocate appropriate
 * memory for the data returned by the encryption or decryption operation
 *
 * Return: block size of cipher
 */
#define crypto_skcipher_blocksize LINUX_BACKPORT(crypto_skcipher_blocksize)
static inline unsigned int crypto_skcipher_blocksize(
	struct crypto_skcipher *tfm)
{
	return crypto_tfm_alg_blocksize(crypto_skcipher_tfm(tfm));
}

#define crypto_skcipher_alignmask LINUX_BACKPORT(crypto_skcipher_alignmask)
static inline unsigned int crypto_skcipher_alignmask(
	struct crypto_skcipher *tfm)
{
	return crypto_tfm_alg_alignmask(crypto_skcipher_tfm(tfm));
}

#define crypto_skcipher_get_flags LINUX_BACKPORT(crypto_skcipher_get_flags)
static inline u32 crypto_skcipher_get_flags(struct crypto_skcipher *tfm)
{
	return crypto_tfm_get_flags(crypto_skcipher_tfm(tfm));
}

#define crypto_skcipher_set_flags LINUX_BACKPORT(crypto_skcipher_set_flags)
static inline void crypto_skcipher_set_flags(struct crypto_skcipher *tfm,
					       u32 flags)
{
	crypto_tfm_set_flags(crypto_skcipher_tfm(tfm), flags);
}

#define crypto_skcipher_clear_flags LINUX_BACKPORT(crypto_skcipher_clear_flags)
static inline void crypto_skcipher_clear_flags(struct crypto_skcipher *tfm,
						 u32 flags)
{
	crypto_tfm_clear_flags(crypto_skcipher_tfm(tfm), flags);
}

/**
 * crypto_skcipher_setkey() - set key for cipher
 * @tfm: cipher handle
 * @key: buffer holding the key
 * @keylen: length of the key in bytes
 *
 * The caller provided key is set for the skcipher referenced by the cipher
 * handle.
 *
 * Note, the key length determines the cipher type. Many block ciphers implement
 * different cipher modes depending on the key size, such as AES-128 vs AES-192
 * vs. AES-256. When providing a 16 byte key for an AES cipher handle, AES-128
 * is performed.
 *
 * Return: 0 if the setting of the key was successful; < 0 if an error occurred
 */
#define crypto_skcipher_setkey LINUX_BACKPORT(crypto_skcipher_setkey)
static inline int crypto_skcipher_setkey(struct crypto_skcipher *tfm,
					 const u8 *key, unsigned int keylen)
{
	return tfm->setkey(tfm, key, keylen);
}

#define crypto_skcipher_has_setkey LINUX_BACKPORT(crypto_skcipher_has_setkey)
static inline bool crypto_skcipher_has_setkey(struct crypto_skcipher *tfm)
{
	return tfm->keysize;
}

#define crypto_skcipher_default_keysize LINUX_BACKPORT(crypto_skcipher_default_keysize)
static inline unsigned int crypto_skcipher_default_keysize(
	struct crypto_skcipher *tfm)
{
	return tfm->keysize;
}

/**
 * crypto_skcipher_reqtfm() - obtain cipher handle from request
 * @req: skcipher_request out of which the cipher handle is to be obtained
 *
 * Return the crypto_skcipher handle when furnishing an skcipher_request
 * data structure.
 *
 * Return: crypto_skcipher handle
 */
#define crypto_skcipher_reqtfm LINUX_BACKPORT(crypto_skcipher_reqtfm)
static inline struct crypto_skcipher *crypto_skcipher_reqtfm(
	struct skcipher_request *req)
{
	return __crypto_skcipher_cast(req->base.tfm);
}

/**
 * crypto_skcipher_encrypt() - encrypt plaintext
 * @req: reference to the skcipher_request handle that holds all information
 *	 needed to perform the cipher operation
 *
 * Encrypt plaintext data using the skcipher_request handle. That data
 * structure and how it is filled with data is discussed with the
 * skcipher_request_* functions.
 *
 * Return: 0 if the cipher operation was successful; < 0 if an error occurred
 */
#define crypto_skcipher_encrypt LINUX_BACKPORT(crypto_skcipher_encrypt)
static inline int crypto_skcipher_encrypt(struct skcipher_request *req)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);

	return tfm->encrypt(req);
}

/**
 * crypto_skcipher_decrypt() - decrypt ciphertext
 * @req: reference to the skcipher_request handle that holds all information
 *	 needed to perform the cipher operation
 *
 * Decrypt ciphertext data using the skcipher_request handle. That data
 * structure and how it is filled with data is discussed with the
 * skcipher_request_* functions.
 *
 * Return: 0 if the cipher operation was successful; < 0 if an error occurred
 */
#define crypto_skcipher_decrypt LINUX_BACKPORT(crypto_skcipher_decrypt)
static inline int crypto_skcipher_decrypt(struct skcipher_request *req)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);

	return tfm->decrypt(req);
}

/**
 * DOC: Symmetric Key Cipher Request Handle
 *
 * The skcipher_request data structure contains all pointers to data
 * required for the symmetric key cipher operation. This includes the cipher
 * handle (which can be used by multiple skcipher_request instances), pointer
 * to plaintext and ciphertext, asynchronous callback function, etc. It acts
 * as a handle to the skcipher_request_* API calls in a similar way as
 * skcipher handle to the crypto_skcipher_* API calls.
 */

/**
 * crypto_skcipher_reqsize() - obtain size of the request data structure
 * @tfm: cipher handle
 *
 * Return: number of bytes
 */
#define crypto_skcipher_reqsize LINUX_BACKPORT(crypto_skcipher_reqsize)
static inline unsigned int crypto_skcipher_reqsize(struct crypto_skcipher *tfm)
{
	return tfm->reqsize;
}

/**
 * skcipher_request_set_tfm() - update cipher handle reference in request
 * @req: request handle to be modified
 * @tfm: cipher handle that shall be added to the request handle
 *
 * Allow the caller to replace the existing skcipher handle in the request
 * data structure with a different one.
 */
#define skcipher_request_set_tfm LINUX_BACKPORT(skcipher_request_set_tfm)
static inline void skcipher_request_set_tfm(struct skcipher_request *req,
					    struct crypto_skcipher *tfm)
{
	req->base.tfm = crypto_skcipher_tfm(tfm);
}

#define skcipher_request_cast LINUX_BACKPORT(skcipher_request_cast)
static inline struct skcipher_request *skcipher_request_cast(
	struct crypto_async_request *req)
{
	return container_of(req, struct skcipher_request, base);
}

/**
 * skcipher_request_alloc() - allocate request data structure
 * @tfm: cipher handle to be registered with the request
 * @gfp: memory allocation flag that is handed to kmalloc by the API call.
 *
 * Allocate the request data structure that must be used with the skcipher
 * encrypt and decrypt API calls. During the allocation, the provided skcipher
 * handle is registered in the request data structure.
 *
 * Return: allocated request handle in case of success; IS_ERR() is true in case
 *	   of an error, PTR_ERR() returns the error code.
 */
#define skcipher_request LINUX_BACKPORT(skcipher_request)
static inline struct skcipher_request *skcipher_request_alloc(
	struct crypto_skcipher *tfm, gfp_t gfp)
{
	struct skcipher_request *req;

	req = kmalloc(sizeof(struct skcipher_request) +
		      crypto_skcipher_reqsize(tfm), gfp);

	if (likely(req))
		skcipher_request_set_tfm(req, tfm);

	return req;
}

/**
 * skcipher_request_free() - zeroize and free request data structure
 * @req: request data structure cipher handle to be freed
 */
#define skcipher_request_free LINUX_BACKPORT(skcipher_request_free)
static inline void skcipher_request_free(struct skcipher_request *req)
{
	kzfree(req);
}

/**
 * skcipher_request_set_callback() - set asynchronous callback function
 * @req: request handle
 * @flags: specify zero or an ORing of the flags
 *         CRYPTO_TFM_REQ_MAY_BACKLOG the request queue may back log and
 *	   increase the wait queue beyond the initial maximum size;
 *	   CRYPTO_TFM_REQ_MAY_SLEEP the request processing may sleep
 * @compl: callback function pointer to be registered with the request handle
 * @data: The data pointer refers to memory that is not used by the kernel
 *	  crypto API, but provided to the callback function for it to use. Here,
 *	  the caller can provide a reference to memory the callback function can
 *	  operate on. As the callback function is invoked asynchronously to the
 *	  related functionality, it may need to access data structures of the
 *	  related functionality which can be referenced using this pointer. The
 *	  callback function can access the memory via the "data" field in the
 *	  crypto_async_request data structure provided to the callback function.
 *
 * This function allows setting the callback function that is triggered once the
 * cipher operation completes.
 *
 * The callback function is registered with the skcipher_request handle and
 * must comply with the following template
 *
 *	void callback_function(struct crypto_async_request *req, int error)
 */
#define skcipher_request_set_callback LINUX_BACKPORT(skcipher_request_set_callback)
static inline void skcipher_request_set_callback(struct skcipher_request *req,
						 u32 flags,
						 crypto_completion_t compl,
						 void *data)
{
	req->base.complete = compl;
	req->base.data = data;
	req->base.flags = flags;
}

/**
 * skcipher_request_set_crypt() - set data buffers
 * @req: request handle
 * @src: source scatter / gather list
 * @dst: destination scatter / gather list
 * @cryptlen: number of bytes to process from @src
 * @iv: IV for the cipher operation which must comply with the IV size defined
 *      by crypto_skcipher_ivsize
 *
 * This function allows setting of the source data and destination data
 * scatter / gather lists.
 *
 * For encryption, the source is treated as the plaintext and the
 * destination is the ciphertext. For a decryption operation, the use is
 * reversed - the source is the ciphertext and the destination is the plaintext.
 */
#define skcipher_request_set_crypt LINUX_BACKPORT(skcipher_request_set_crypt)
static inline void skcipher_request_set_crypt(
	struct skcipher_request *req,
	struct scatterlist *src, struct scatterlist *dst,
	unsigned int cryptlen, void *iv)
{
	req->src = src;
	req->dst = dst;
	req->cryptlen = cryptlen;
	req->iv = iv;
}
static unsigned int crypto_skcipher_extsize(struct crypto_alg *alg)
{
	if (alg->cra_type == &crypto_blkcipher_type)
		return sizeof(struct crypto_blkcipher *);

	BUG_ON(alg->cra_type != &crypto_ablkcipher_type &&
	       alg->cra_type != &crypto_givcipher_type);

	return sizeof(struct crypto_ablkcipher *);
}

static int skcipher_setkey_blkcipher(struct crypto_skcipher *tfm,
				     const u8 *key, unsigned int keylen)
{
	struct crypto_blkcipher **ctx = crypto_skcipher_ctx(tfm);
	struct crypto_blkcipher *blkcipher = *ctx;
	int err;

	crypto_blkcipher_clear_flags(blkcipher, ~0);
	crypto_blkcipher_set_flags(blkcipher, crypto_skcipher_get_flags(tfm) &
					      CRYPTO_TFM_REQ_MASK);
	err = crypto_blkcipher_setkey(blkcipher, key, keylen);
	crypto_skcipher_set_flags(tfm, crypto_blkcipher_get_flags(blkcipher) &
				       CRYPTO_TFM_RES_MASK);

	return err;
}

static int skcipher_crypt_blkcipher(struct skcipher_request *req,
				    int (*crypt)(struct blkcipher_desc *,
						 struct scatterlist *,
						 struct scatterlist *,
						 unsigned int))
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);
	struct crypto_blkcipher **ctx = crypto_skcipher_ctx(tfm);
	struct blkcipher_desc desc = {
		.tfm = *ctx,
		.info = req->iv,
		.flags = req->base.flags,
	};


	return crypt(&desc, req->dst, req->src, req->cryptlen);
}

static int skcipher_encrypt_blkcipher(struct skcipher_request *req)
{
	struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
	struct crypto_tfm *tfm = crypto_skcipher_tfm(skcipher);
	struct blkcipher_alg *alg = &tfm->__crt_alg->cra_blkcipher;

	return skcipher_crypt_blkcipher(req, alg->encrypt);
}

static int skcipher_decrypt_blkcipher(struct skcipher_request *req)
{
	struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
	struct crypto_tfm *tfm = crypto_skcipher_tfm(skcipher);
	struct blkcipher_alg *alg = &tfm->__crt_alg->cra_blkcipher;

	return skcipher_crypt_blkcipher(req, alg->decrypt);
}

static void crypto_exit_skcipher_ops_blkcipher(struct crypto_tfm *tfm)
{
	struct crypto_blkcipher **ctx = crypto_tfm_ctx(tfm);

	crypto_free_blkcipher(*ctx);
}

static int crypto_init_skcipher_ops_blkcipher(struct crypto_tfm *tfm)
{
	struct crypto_alg *calg = tfm->__crt_alg;
	struct crypto_skcipher *skcipher = __crypto_skcipher_cast(tfm);
	struct crypto_blkcipher **ctx = crypto_tfm_ctx(tfm);
	struct crypto_blkcipher *blkcipher;
	struct crypto_tfm *btfm;

	if (!crypto_mod_get(calg))
		return -EAGAIN;

	btfm = __crypto_alloc_tfm(calg, CRYPTO_ALG_TYPE_BLKCIPHER,
					CRYPTO_ALG_TYPE_MASK);
	if (IS_ERR(btfm)) {
		crypto_mod_put(calg);
		return PTR_ERR(btfm);
	}

	blkcipher = __crypto_blkcipher_cast(btfm);
	*ctx = blkcipher;
	tfm->exit = crypto_exit_skcipher_ops_blkcipher;

	skcipher->setkey = skcipher_setkey_blkcipher;
	skcipher->encrypt = skcipher_encrypt_blkcipher;
	skcipher->decrypt = skcipher_decrypt_blkcipher;

	skcipher->ivsize = crypto_blkcipher_ivsize(blkcipher);
	skcipher->has_setkey = calg->cra_blkcipher.max_keysize;

	return 0;
}

static int skcipher_setkey_ablkcipher(struct crypto_skcipher *tfm,
				      const u8 *key, unsigned int keylen)
{
	struct crypto_ablkcipher **ctx = crypto_skcipher_ctx(tfm);
	struct crypto_ablkcipher *ablkcipher = *ctx;
	int err;

	crypto_ablkcipher_clear_flags(ablkcipher, ~0);
	crypto_ablkcipher_set_flags(ablkcipher,
				    crypto_skcipher_get_flags(tfm) &
				    CRYPTO_TFM_REQ_MASK);
	err = crypto_ablkcipher_setkey(ablkcipher, key, keylen);
	crypto_skcipher_set_flags(tfm,
				  crypto_ablkcipher_get_flags(ablkcipher) &
				  CRYPTO_TFM_RES_MASK);

	return err;
}

static int skcipher_crypt_ablkcipher(struct skcipher_request *req,
				     int (*crypt)(struct ablkcipher_request *))
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);
	struct crypto_ablkcipher **ctx = crypto_skcipher_ctx(tfm);
	struct ablkcipher_request *subreq = skcipher_request_ctx(req);

	ablkcipher_request_set_tfm(subreq, *ctx);
	ablkcipher_request_set_callback(subreq, skcipher_request_flags(req),
					req->base.complete, req->base.data);
	ablkcipher_request_set_crypt(subreq, req->src, req->dst, req->cryptlen,
				     req->iv);

	return crypt(subreq);
}

static int skcipher_encrypt_ablkcipher(struct skcipher_request *req)
{
	struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
	struct crypto_tfm *tfm = crypto_skcipher_tfm(skcipher);
	struct ablkcipher_alg *alg = &tfm->__crt_alg->cra_ablkcipher;

	return skcipher_crypt_ablkcipher(req, alg->encrypt);
}

static int skcipher_decrypt_ablkcipher(struct skcipher_request *req)
{
	struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
	struct crypto_tfm *tfm = crypto_skcipher_tfm(skcipher);
	struct ablkcipher_alg *alg = &tfm->__crt_alg->cra_ablkcipher;

	return skcipher_crypt_ablkcipher(req, alg->decrypt);
}

static void crypto_exit_skcipher_ops_ablkcipher(struct crypto_tfm *tfm)
{
	struct crypto_ablkcipher **ctx = crypto_tfm_ctx(tfm);

	crypto_free_ablkcipher(*ctx);
}

static int crypto_init_skcipher_ops_ablkcipher(struct crypto_tfm *tfm)
{
	struct crypto_alg *calg = tfm->__crt_alg;
	struct crypto_skcipher *skcipher = __crypto_skcipher_cast(tfm);
	struct crypto_ablkcipher **ctx = crypto_tfm_ctx(tfm);
	struct crypto_ablkcipher *ablkcipher;
	struct crypto_tfm *abtfm;

	if (!crypto_mod_get(calg))
		return -EAGAIN;

	abtfm = __crypto_alloc_tfm(calg, 0, 0);
	if (IS_ERR(abtfm)) {
		crypto_mod_put(calg);
		return PTR_ERR(abtfm);
	}

	ablkcipher = __crypto_ablkcipher_cast(abtfm);
	*ctx = ablkcipher;
	tfm->exit = crypto_exit_skcipher_ops_ablkcipher;

	skcipher->setkey = skcipher_setkey_ablkcipher;
	skcipher->encrypt = skcipher_encrypt_ablkcipher;
	skcipher->decrypt = skcipher_decrypt_ablkcipher;

	skcipher->ivsize = crypto_ablkcipher_ivsize(ablkcipher);
	skcipher->reqsize = crypto_ablkcipher_reqsize(ablkcipher) +
			    sizeof(struct ablkcipher_request);
	skcipher->has_setkey = calg->cra_ablkcipher.max_keysize;

	return 0;
}

static int crypto_skcipher_init_tfm(struct crypto_tfm *tfm)
{
	if (tfm->__crt_alg->cra_type == &crypto_blkcipher_type)
		return crypto_init_skcipher_ops_blkcipher(tfm);

	BUG_ON(tfm->__crt_alg->cra_type != &crypto_ablkcipher_type &&
	       tfm->__crt_alg->cra_type != &crypto_givcipher_type);

	return crypto_init_skcipher_ops_ablkcipher(tfm);
}

static const struct crypto_type crypto_skcipher_type2 = {
	.extsize = crypto_skcipher_extsize,
	.init_tfm = crypto_skcipher_init_tfm,
	.maskclear = ~CRYPTO_ALG_TYPE_MASK,
	.maskset = CRYPTO_ALG_TYPE_BLKCIPHER_MASK,
	.type = CRYPTO_ALG_TYPE_BLKCIPHER,
	.tfmsize = offsetof(struct crypto_skcipher, base),
};

void *crypto_alloc_tfm(const char *alg_name,
		       const struct crypto_type *frontend, u32 type, u32 mask);

#define crypto_alloc_skcipher_crypt LINUX_BACKPORT(crypto_alloc_skcipher)
struct crypto_skcipher *crypto_alloc_skcipher(const char *alg_name,
					      u32 type, u32 mask)
{
	return crypto_alloc_tfm(alg_name, &crypto_skcipher_type2, type, mask);
}


#endif /* < 4.3 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
#define skcipher_request_zero LINUX_BACKPORT(skcipher_request_zero)
static inline void skcipher_request_zero(struct skcipher_request *req)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);

	memzero_explicit(req, sizeof(*req) + crypto_skcipher_reqsize(tfm));
}
#endif /* < 4.6 */

#endif /* _BACKPORT_CRYPTO_SKCIPHER_H */
