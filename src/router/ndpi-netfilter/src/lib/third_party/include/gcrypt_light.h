
#ifdef __cplusplus
extern "C" {
#endif

#ifndef GCRY_LIGHT_H
#define GCRY_LIGHT_H

#define HMAC_SHA256_DIGEST_SIZE 32  /* Same as SHA-256's output size. */
#define SHA256_DIGEST_SIZE 32
#define GCRY_MD_BUFF_SIZE 256

#define GCRY_CIPHER_AES128 7
#define GCRY_CIPHER_AES256 8
#define GCRY_CIPHER_MODE_ECB 1
#define GCRY_CIPHER_MODE_GCM 8 

#define GCRY_MD_SHA256 8
#define GCRY_MD_FLAG_HMAC 2
#define GPG_ERR_NO_ERROR 0
#define GPG_ERR_KEY -1
#define GPG_ERR_ANY -2
#define GPG_ERR_INV_ARG -3
#define GCRYCTL_INITIALIZATION_FINISHED_P 1
#define GCRYCTL_INITIALIZATION_FINISHED 2
#define GCRYCTL_RESET 3

#define GCRY_AES_KEY_SIZE 32
#define GCRY_AES_AUTH_SIZE 256
#define GCRY_AES_TAG_SIZE 32
#define GCRY_AES_IV_SIZE 32

typedef int gcry_error_t;
typedef gcry_error_t gpg_error_t;

struct gcry_md_hd {
	uint8_t  key[64],out[HMAC_SHA256_DIGEST_SIZE];
	uint8_t  data_buf[GCRY_MD_BUFF_SIZE];
	uint32_t key_len;
	uint32_t data_len;
};
typedef struct gcry_md_hd * gcry_md_hd_t;

struct gcry_cipher_hd {
    int     algo,mode;
    size_t  keylen,authlen,taglen,ivlen;
    uint8_t s_key:1,s_auth:1,s_iv:1,s_crypt_ok:1;
    uint8_t auth[GCRY_AES_AUTH_SIZE];
    uint8_t tag[GCRY_AES_TAG_SIZE];
    uint8_t iv[GCRY_AES_IV_SIZE];
    union {
        struct mbedtls_aes_context *ecb;
        struct mbedtls_gcm_context *gcm;
    }  ctx;
};

typedef struct gcry_cipher_hd * gcry_cipher_hd_t;

NDPI_STATIC int          gcry_control (int, int);
NDPI_STATIC const char   *gcry_check_version(void *);
NDPI_STATIC char         *gpg_strerror_r(gcry_error_t, char *, size_t);

NDPI_STATIC gcry_error_t gcry_md_open  (gcry_md_hd_t *h, int algo, int flags);
NDPI_STATIC void         gcry_md_close (gcry_md_hd_t h);
NDPI_STATIC void         gcry_md_reset (gcry_md_hd_t h);

NDPI_STATIC gcry_error_t gcry_md_setkey (gcry_md_hd_t h, const uint8_t *key, size_t key_len);
NDPI_STATIC gcry_error_t gcry_md_write  (gcry_md_hd_t h, const uint8_t *data, size_t data_len);
NDPI_STATIC uint8_t      *gcry_md_read  (gcry_md_hd_t h, int flag);

NDPI_STATIC size_t       gcry_md_get_algo_dlen (int algo);
NDPI_STATIC int          gcry_md_get_algo (gcry_md_hd_t h);

NDPI_STATIC gcry_error_t gcry_cipher_open  (gcry_cipher_hd_t *handle, int algo, int mode, unsigned int flags);
NDPI_STATIC void         gcry_cipher_close (gcry_cipher_hd_t h);
NDPI_STATIC gcry_error_t gcry_cipher_ctl    (gcry_cipher_hd_t h, int cmd, void *data, size_t len);
NDPI_STATIC gcry_error_t gcry_cipher_reset (gcry_cipher_hd_t h);
NDPI_STATIC gcry_error_t gcry_cipher_setiv (gcry_cipher_hd_t h, const void *iv, size_t ivlen);
NDPI_STATIC gcry_error_t gcry_cipher_setkey (gcry_cipher_hd_t h, const void *key, size_t keylen);
NDPI_STATIC gcry_error_t gcry_cipher_checktag (gcry_cipher_hd_t h, const void *intag, size_t taglen);
NDPI_STATIC gcry_error_t gcry_cipher_authenticate (gcry_cipher_hd_t h, const void *abuf, size_t abuflen);
NDPI_STATIC gcry_error_t gcry_cipher_encrypt (gcry_cipher_hd_t h, void *out, size_t outsize,
                                  const void *in, size_t inlen);
NDPI_STATIC gcry_error_t gcry_cipher_decrypt (gcry_cipher_hd_t h, void *out, size_t outsize,
                                  const void *in, size_t inlen);

NDPI_STATIC size_t       gcry_cipher_get_algo_keylen (int algo);

#endif /* GCRY_LIGHT_H */

#ifdef __cplusplus
}
#endif

/* vim: set ts=4 sw=4 et foldmethod=marker foldmarker={{{{,}}}}: */
