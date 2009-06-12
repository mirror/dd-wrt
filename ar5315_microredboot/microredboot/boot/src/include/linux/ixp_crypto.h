
#ifndef IX_CRYPTO_H
#define IX_CRYPTO_H

#define MAX_KEYLEN 64
#define NPE_CTX_LEN 80
#define AES_BLOCK128 16

#define NPE_OP_HASH_GEN_ICV   0x50
#define NPE_OP_ENC_GEN_KEY    0xc9


#define NPE_OP_HASH_VERIFY   0x01
#define NPE_OP_CCM_ENABLE    0x04
#define NPE_OP_CRYPT_ENABLE  0x08
#define NPE_OP_HASH_ENABLE   0x10
#define NPE_OP_NOT_IN_PLACE  0x20
#define NPE_OP_HMAC_DISABLE  0x40
#define NPE_OP_CRYPT_ENCRYPT 0x80

#define MOD_ECB     0x0000
#define MOD_CTR     0x1000
#define MOD_CBC_ENC 0x2000
#define MOD_CBC_DEC 0x3000
#define MOD_CCM_ENC 0x4000
#define MOD_CCM_DEC 0x5000

#define ALGO_AES    0x0800
#define CIPH_DECR   0x0000
#define CIPH_ENCR   0x0400

#define MOD_DES     0x0000
#define MOD_TDEA2   0x0100
#define MOD_TDEA3   0x0200
#define MOD_AES128  0x0000
#define MOD_AES192  0x0100
#define MOD_AES256  0x0200

#define KEYLEN_128  4
#define KEYLEN_192  6
#define KEYLEN_256  8

#define CIPHER_TYPE_NULL   0
#define CIPHER_TYPE_DES    1
#define CIPHER_TYPE_3DES   2
#define CIPHER_TYPE_AES    3

#define CIPHER_MODE_ECB    1
#define CIPHER_MODE_CTR    2
#define CIPHER_MODE_CBC    3
#define CIPHER_MODE_CCM    4

#define HASH_TYPE_NULL     0
#define HASH_TYPE_MD5      1
#define HASH_TYPE_SHA1     2
#define HASH_TYPE_CBCMAC   3

#define OP_REG_DONE  1
#define OP_REGISTER  2
#define OP_PERFORM   3

#define STATE_UNREGISTERED 0
#define STATE_REGISTERED   1
#define STATE_UNLOADING    2

struct crypt_ctl {
#ifndef CONFIG_NPE_ADDRESS_COHERENT
	u8 mode;    /* NPE operation */
	u8 init_len;
	u16 reserved;
#else
	u16 reserved;
	u8 init_len;
	u8 mode;    /* NPE operation */
#endif
	u8 iv[16];  /* IV for CBC mode or CTR IV for CTR mode */
	union {
		u32 icv;
		u32 rev_aes;
	} addr;
	u32 src_buf;
	u32 dest_buf;
#ifndef CONFIG_NPE_ADDRESS_COHERENT
	u16 auth_offs;  /* Authentication start offset */
	u16 auth_len;   /* Authentication data length */
	u16 crypt_offs; /* Cryption start offset */
	u16 crypt_len;  /* Cryption data length */
#else
	u16 auth_len;   /* Authentication data length */
	u16 auth_offs;  /* Authentication start offset */
	u16 crypt_len;  /* Cryption data length */
	u16 crypt_offs; /* Cryption start offset */
#endif
	u32 aadAddr;    /* Additional Auth Data Addr for CCM mode */
	u32 crypto_ctx; /* NPE Crypto Param structure address */

	/* Used by Host */
	struct ix_sa_ctx *sa_ctx;
	int oper_type;
};

struct npe_crypt_cont {
	union {
		struct crypt_ctl crypt;
		u8 rev_aes_key[NPE_CTX_LEN];
	} ctl;
	struct npe_crypt_cont *next;
	struct npe_crypt_cont *virt;
	dma_addr_t phys;
};

struct ix_hash_algo {
	char *name;
	u32 cfgword;
	int digest_len;
	int aad_len;
	unsigned char *icv;
	int type;
};

struct ix_cipher_algo {
	char *name;
	u32 cfgword_enc;
	u32 cfgword_dec;
	int block_len;
	int iv_len;
	int type;
	int mode;
};

struct ix_key {
	u8 key[MAX_KEYLEN];
	int len;
};

struct ix_sa_master {
	struct device *npe_dev;
	struct qm_queue *sendq;
	struct qm_queue *recvq;
	struct dma_pool *dmapool;
	struct npe_crypt_cont *pool;
	int pool_size;
	rwlock_t lock;
};

struct ix_sa_dir {
	unsigned char *npe_ctx;
	dma_addr_t npe_ctx_phys;
	int npe_ctx_idx;
	u8 npe_mode;
};

struct ix_sa_ctx {
	struct list_head list;
	struct ix_sa_master *master;

	const struct ix_hash_algo *h_algo;
	const struct ix_cipher_algo *c_algo;
	struct ix_key c_key;
	struct ix_key h_key;

	int digest_len;

	struct ix_sa_dir encrypt;
	struct ix_sa_dir decrypt;

	struct npe_crypt_cont *rev_aes;
	gfp_t gfp_flags;

	int state;
	void *priv;

	void(*reg_cb)(struct ix_sa_ctx*, int);
	void(*perf_cb)(struct ix_sa_ctx*, void*, int);
	atomic_t use_cnt;
};

const struct ix_hash_algo *ix_hash_by_id(int type);
const struct ix_cipher_algo *ix_cipher_by_id(int type, int mode);

struct ix_sa_ctx *ix_sa_ctx_new(int priv_len, gfp_t flags);
void ix_sa_ctx_free(struct ix_sa_ctx *sa_ctx);

int ix_sa_crypto_perform(struct ix_sa_ctx *sa_ctx, u8 *data, void *ptr,
		int datalen, int c_offs, int c_len, int a_offs, int a_len,
		int hmac, char *iv, int encrypt);

int ix_sa_ctx_setup_cipher_auth(struct ix_sa_ctx *sa_ctx,
		const struct ix_cipher_algo *cipher,
		const struct ix_hash_algo *auth, int len);

#endif
