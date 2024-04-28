/* Copyright (c) 2013-2017, The Linux Foundation. All rights reserved.
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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/atomic.h>

#include <nss_crypto_if.h>
#include <nss_crypto_hlos.h>
#include <nss_api_if.h>


#define CRYPTO_BENCH_PERM_RO		0444
#define CRYPTO_BENCH_PERM_RW		0666

#define __ENCR_MEMCMP_SZ		64
/* #define __ENCR_MEMCMP_SZ		256 */

#define CRYPTO_BENCH_MAX_DATA_SZ	1536
#define CRYPTO_BENCH_MAX_BAM_SZ	(CRYPTO_BENCH_MAX_DATA_SZ + NSS_CRYPTO_MAX_IVLEN_AES)
#define CRYPTO_BENCH_RESULTS_SZ 	128
#define CRYPTO_BENCH_DATA_ALIGN 	4

#define CRYPTO_BENCH_PRN_LVL_DBG	3
#define CRYPTO_BENCH_PRN_LVL_INFO	2
#define CRYPTO_BENCH_PRN_LVL_ERR	1

#define CRYPTO_BENCH_OK 	0
#define CRYPTO_BENCH_NOT_OK	-1


#define CRYPTO_BENCH_ASSERT(expr) do { \
	if (!(expr)) {	\
		printk("Operation failed - %d, %s\n", __LINE__, #expr); \
		panic("system is going down\n"); \
	} \
} while(0)

#define crypto_bench_print(level, fmt, args...)	do {	\
	if (CRYPTO_BENCH_PRN_LVL_##level <= param.print_mode) {	\
		printk(fmt, ##args);	\
	}	\
} while(0)

#define crypto_bench_error(fmt, args...)	crypto_bench_print(ERR, fmt, ##args)
#define crypto_bench_info(fmt, args...)		crypto_bench_print(INFO, fmt, ##args)
#define crypto_bench_debug(fmt, args...)	crypto_bench_print(DBG, fmt, ##args)

static DECLARE_WAIT_QUEUE_HEAD(tx_comp);
static DECLARE_WAIT_QUEUE_HEAD(tx_start);
static struct task_struct *tx_thread = NULL;

static struct timespec64 init_time;
static struct timespec64 comp_time;
static spinlock_t op_lock;
static nss_crypto_handle_t crypto_hdl;

static struct kmem_cache *crypto_op_zone;

static struct dentry *droot;

static const uint8_t *help = "bench mcmp flush start default";

static atomic_t tx_reqs;

static uint8_t auth_key[NSS_CRYPTO_MAX_KEYLEN_SHA256]= {
	0x01, 0x02, 0x03, 0x04,
	0x05, 0x06, 0x07, 0x08,
	0x09, 0x0a, 0x0b, 0x0c,
	0x0d, 0x0e, 0x0f, 0x01,
	0x02, 0x03, 0x04, 0x05
};

static uint8_t cipher_key[NSS_CRYPTO_MAX_KEYLEN_AES] = {
	0x60, 0x3d, 0xeb, 0x10,
	0x15, 0xca, 0x71, 0xbe,
	0x2b, 0x73, 0xae, 0xf0,
	0x85, 0x7d, 0x77, 0x81,
	0x1f, 0x35, 0x2c, 0x07,
	0x3b, 0x61, 0x08, 0xd7,
	0x2d, 0x98, 0x10, 0xa3,
	0x09, 0x14, 0xdf, 0xf4
};

static uint8_t cipher_iv[NSS_CRYPTO_MAX_IVLEN_AES] = {
	0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b,
	0x0c, 0x0d, 0x0e, 0x0f
};


#if (__ENCR_MEMCMP_SZ == 64)

static uint8_t plain_text[__ENCR_MEMCMP_SZ] = {
	0x6b, 0xc1, 0xbe, 0xe2,
	0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11,
	0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57,
	0x1e, 0x03, 0xac, 0x9c,
        0x9e, 0xb7, 0x6f, 0xac,
	0x45, 0xaf, 0x8e, 0x51,
        0x30, 0xc8, 0x1c, 0x46,
	0xa3, 0x5c, 0xe4, 0x11,
        0xe5, 0xfb, 0xc1, 0x19,
	0x1a, 0x0a, 0x52, 0xef,
        0xf6, 0x9f, 0x24, 0x45,
	0xdf, 0x4f, 0x9b, 0x17,
        0xad, 0x2b, 0x41, 0x7b,
	0xe6, 0x6c, 0x37, 0x10
};

/*
 * 3DES (encrypted text)
 */
static uint8_t triple_des_encr_text[__ENCR_MEMCMP_SZ] = {
	0x5e, 0x45, 0x0a, 0x57,
	0x99, 0xa1, 0x77, 0x53,
	0x71, 0x83, 0x3f, 0xb0,
	0xba, 0xcd, 0xbb, 0xcf,
	0x60, 0x40, 0xa8, 0x94,
	0x1c, 0x09, 0x89, 0xab,
	0xc8, 0x30, 0x1f, 0x45,
	0xae, 0x42, 0x80, 0xf0,
	0x01, 0xd9, 0xa4, 0xd7,
	0x9c, 0xae, 0xd6, 0xc4,
	0x34, 0x50, 0x33, 0x63,
	0xb0, 0x71, 0x49, 0xd9,
	0xc3, 0xe7, 0x1f, 0xd4,
	0x2d, 0x6f, 0x16, 0xc2,
	0x0d, 0x93, 0xfe, 0x9a,
	0x0c, 0x21, 0x94, 0xfa,
};

/*
 * AES-256 (encrypted text)
 */
static uint8_t aes_encr_text[__ENCR_MEMCMP_SZ] = {
	0xf5, 0x8c, 0x4c, 0x04,
	0xd6, 0xe5, 0xf1, 0xba,
	0x77, 0x9e, 0xab, 0xfb,
	0x5f, 0x7b, 0xfb, 0xd6,
        0x9c, 0xfc, 0x4e, 0x96,
	0x7e, 0xdb, 0x80, 0x8d,
	0x67, 0x9f, 0x77, 0x7b,
	0xc6, 0x70, 0x2c, 0x7d,
        0x39, 0xf2, 0x33, 0x69,
	0xa9, 0xd9, 0xba, 0xcf,
	0xa5, 0x30, 0xe2, 0x63,
	0x04, 0x23, 0x14, 0x61,
        0xb2, 0xeb, 0x05, 0xe2,
	0xc3, 0x9b, 0xe9, 0xfc,
	0xda, 0x6c, 0x19, 0x07,
	0x8c, 0x6a, 0x9d, 0x1b
};

/*
 * AES hash
 */
static uint8_t aes_sha1_hash[NSS_CRYPTO_MAX_KEYLEN_SHA1] = {
	0xc9, 0xdd, 0x94, 0xfb,
	0xc8, 0x9f, 0x81, 0x12,
	0x68, 0x1b, 0x8f, 0xfb,
	0xb5, 0xfd, 0x27, 0x69,
	0x76, 0xa1, 0x2e, 0x99
};

/*
 * 3DES hash
 */
static uint8_t triple_des_sha1_hash[NSS_CRYPTO_MAX_KEYLEN_SHA1] = {
	0x4a, 0xae, 0xc0, 0x59,
	0x39, 0x40, 0x6b, 0x85,
	0xb1, 0x03, 0xad, 0x7e,
};

#elif (__ENCR_MEMCMP_SZ == 256)
static uint8_t plain_text[__ENCR_MEMCMP_SZ] = {0};

static uint8_t encr_text[__ENCR_MEMCMP_SZ] = {
	0xb7, 0xbf, 0x3a, 0x5d,
	0xf4, 0x39, 0x89, 0xdd,
	0x97, 0xf0, 0xfa, 0x97,
	0xeb, 0xce, 0x2f, 0x4a,
	0xe1, 0xc6, 0x56, 0x30,
	0x5e, 0xd1, 0xa7, 0xa6,
	0x56, 0x38, 0x05, 0x74,
	0x6f, 0xe0, 0x3e, 0xdc,
	0x41, 0x63, 0x5b, 0xe6,
	0x25, 0xb4, 0x8a, 0xfc,
	0x16, 0x66, 0xdd, 0x42,
	0xa0, 0x9d, 0x96, 0xe7,
	0xf7, 0xb9, 0x30, 0x58,
	0xb8, 0xbc, 0xe0, 0xff,
	0xfe, 0xa4, 0x1b, 0xf0,
	0x01, 0x2c, 0xd3, 0x94,
	0x21, 0xdf, 0xa2, 0xcf,
	0x15, 0x47, 0x29, 0x33,
	0x64, 0x9f, 0x5d, 0xd1,
	0x3d, 0xde, 0x5a, 0xc0,
	0xa9, 0xe0, 0x7b, 0xce,
	0xc9, 0x4a, 0xb5, 0xb9,
	0x20, 0x61, 0xab, 0xf1,
	0x4a, 0x57, 0xb8, 0xfe,
	0xf1, 0xd5, 0xd3, 0xae,
	0xa9, 0x4a, 0x05, 0x5a,
	0xde, 0x50, 0x1f, 0x8c,
	0x5d, 0x37, 0x4e, 0x68,
	0xb7, 0xd6, 0x8d, 0xb4,
	0xae, 0xd4, 0x47, 0x0e,
	0xac, 0x03, 0xc2, 0x18,
	0x9b, 0x78, 0xea, 0xca,
	0xe8, 0xdf, 0x42, 0xda,
	0x66, 0x9e, 0x01, 0xbb,
	0x4b, 0x89, 0x2a, 0xb8,
	0x31, 0x87, 0x1a, 0x88,
	0xc0, 0xfd, 0x91, 0x08,
	0x01, 0x03, 0x7d, 0x89,
	0x8c, 0xc1, 0x7a, 0x8d,
	0x72, 0x73, 0xd2, 0x93,
	0x26, 0x07, 0x78, 0x03,
	0x96, 0x87, 0xb5, 0x38,
	0x19, 0x96, 0x9c, 0x06,
	0xa1, 0xaf, 0x8f, 0x9d,
	0xd8, 0xca, 0x42, 0x80,
	0x6c, 0x8d, 0xe6, 0xd7,
	0xa8, 0x5d, 0xbb, 0x82,
	0x03, 0x7a, 0x2b, 0x29,
	0xdb, 0x07, 0x5f, 0xd4,
	0x80, 0x6f, 0xee, 0xc5,
	0x68, 0x30, 0x69, 0xbf,
	0xe8, 0x48, 0x80, 0xd8,
	0x27, 0x75, 0x70, 0x6a,
	0x40, 0xf9, 0x98, 0x73,
	0xa8, 0x1c, 0xae, 0x6d,
	0xbb, 0x8a, 0xbd, 0x0e,
	0xf5, 0xc2, 0xc3, 0x83,
	0x3a, 0xd7, 0x99, 0xcd,
	0xec, 0x6c, 0xec, 0x4b,
	0xab, 0x4c, 0x8a, 0x87,
	0xce, 0xeb, 0x4c, 0x37,
	0x74, 0xab, 0xdb, 0x47,
	0x60, 0x0f, 0x02, 0x39,
	0xbf, 0x3c, 0xef, 0x9c,
};

static uint8_t sha1_hash[NSS_CRYPTO_MAX_KEYLEN_SHA1] = {
	0xf1, 0x71, 0x4b, 0xb9,
	0xeb, 0x76, 0x21, 0x47,
	0x9e, 0xa0, 0x90, 0x7f,
};

#else
#error "incorrect ENCR_MCMP_SZ"
#endif
static int32_t crypto_sid[NSS_CRYPTO_MAX_IDXS];
static uint32_t prep = 0;
static uint8_t pattern_data[CRYPTO_BENCH_MAX_DATA_SZ] = {0};
static uint8_t *data_ptr;

/*
 * Prototypes
 */
static void crypto_bench_done(struct nss_crypto_buf *buf);
static int  crypto_bench_tx(void *arg);

struct crypto_op {
	struct list_head node;

	struct nss_crypto_buf *buf;

	uint8_t *payload;
	uint32_t payload_len;

	uint8_t *data_vaddr;
	uint32_t data_paddr;
	uint32_t data_len;

	uint16_t cipher_skip;
	uint16_t auth_skip;

	uint16_t cipher_len;
	uint16_t auth_len;

	uint16_t hash_offset;
	uint16_t iv_offset;

};


LIST_HEAD(op_head);

/*
 * Debug interface symbols
 */
enum crypto_bench_type {
	TYPE_BENCH = 0,
	TYPE_MCMP  = 1,
	TYPE_CAL   = 1,
	TYPE_MAX
};

struct crypto_bench_param {
	uint32_t print_mode;	/**< enable prints(=1) or disable prints(=0) */
	uint32_t bench_mode;	/**< run mode bench */
	uint32_t mcmp_mode;	/**< run mode memory compare */

	uint8_t  pattern;	/**< pattern to fill */

	uint32_t bam_len;	/**< size of the data buffer */
	uint32_t cipher_len;	/**< size of cipher operation */
	uint32_t auth_len;	/**< size of auth operation */
	uint32_t hash_len;	/**< size of hash to use for mcmp */
	uint32_t key_len;	/**< cipher key length */

	uint32_t cipher_op;	/**< encrypt(op=1) or decrypt(op=0) */
	uint32_t auth_op;	/**< auth(op=1) or none(op=0) */

	uint32_t ciph_algo;	/**< 1 for AES or 2 for DES */
	uint32_t auth_algo;	/**< 1 for SHA-1 or 2 for SHA-256 */

	uint32_t sid;		/**< session index to use */

	uint32_t num_reqs;	/**< number of requests in "1" pass */
	uint32_t num_loops;	/**< number of loops of num_reqs */
	uint32_t cpu_id;	/**< CPU to run the test from */
	uint32_t mbps;		/**< current throughput */

	uint32_t bam_align;	/**< start data from align boundary */
	uint32_t cipher_skip;	/**< start cipher after skipping */
	uint32_t auth_skip;	/**< start cipher after skipping */

	uint32_t max_sids;	/**< maximum Sessions supported */
	uint32_t tx_err; /**< number of errors while enqueuing buffer to NSS */

	uint32_t mcmp_encr;	/**< encryption failures in mcmp */
	uint32_t mcmp_hash;	/**< hash match failures in mcmp */
	uint32_t avg_mbps;	/**< avg throughput */
	uint32_t peak_mbps;	/**<peak throughput */
};

/*
 * Add default values here, rest will be zero'ed out
 */
static struct crypto_bench_param def_param = {
	.pattern = 0x33,
	.bam_len = 256,
	.cipher_op = 1,
	.auth_op = 1,
	.ciph_algo = 1,
	.auth_algo = 1,
	.bam_align = 4,
	.num_reqs = 128,
	.hash_len = NSS_CRYPTO_MAX_HASHLEN_SHA1,
	.key_len = 16,
	.print_mode = 2,
	.num_loops = 999999,
	.cipher_len = 200,
	.auth_len = 220,
	.auth_skip = 16,
	.cipher_skip = 16,
	.max_sids = 4,
};

static struct crypto_bench_param param = {
	.pattern = 0x33,
	.bam_len = 256,
	.cipher_op = 1,
	.auth_op = 1,
	.ciph_algo = 1,
	.auth_algo = 1,
	.bam_align = 4,
	.num_reqs = 128,
	.hash_len = NSS_CRYPTO_MAX_HASHLEN_SHA1,
	.key_len = 16,
	.print_mode = 2,
	.num_loops = 999999,
	.cipher_len = 200,
	.auth_len = 220,
	.auth_skip = 16,
	.cipher_skip = 16,
	.max_sids = 4,
};

#if defined (CONFIG_NSS_CRYPTO_TOOL_DBG)
static void crypto_bench_dump_addr(uint8_t *addr, uint32_t len, uint8_t *str)
{
	int i;

	crypto_bench_debug("%s:\n", str);

	/* crypto_bench_debug("0x%02x,%s", addr[0]," "); */

	for (i = 0; i < len; i = i+4) {
		crypto_bench_debug("0x%02x, 0x%02x, 0x%02x, 0x%02x, %s", addr[i], addr[i+1], addr[i+2], addr[i+3], "\n");
	}
	crypto_bench_debug("\n");
}
#else
#define crypto_bench_dump_addr(addr, len, str)
#endif


static inline uint32_t crypto_bench_align(uint32_t val, uint32_t align)
{
	uint32_t offset;

	offset = val % align;

	return val - offset + align;
}

#define chk_n_set(expr, field, def_val)	do {	\
	if ((expr)) {	\
		field = def_val;	\
	}	\
} while(0)

#define chk_n_update(expr, field, def_val)	do {	\
	if ((expr)) {	\
		field |= def_val;	\
	}	\
} while(0)


static void crypto_bench_init_param(enum crypto_bench_type type)
{
	chk_n_set((param.auth_op == 0), param.auth_len, 0);
	chk_n_set((param.auth_op == 0), param.auth_skip, 0);

	chk_n_set((param.cipher_op == 0), param.cipher_len, 0);
	chk_n_set((param.cipher_op == 0), param.cipher_skip, 0);

	/*
	 * we don't support data more than 1536
	 */
	chk_n_set((param.cipher_len > CRYPTO_BENCH_MAX_DATA_SZ), param.cipher_len, CRYPTO_BENCH_MAX_DATA_SZ);
	chk_n_set((param.auth_len > CRYPTO_BENCH_MAX_DATA_SZ), param.auth_len, CRYPTO_BENCH_MAX_DATA_SZ);

	chk_n_set((param.cipher_skip < NSS_CRYPTO_MAX_IVLEN_AES), param.cipher_skip, NSS_CRYPTO_MAX_IVLEN_AES);

	chk_n_set((param.cpu_id > CONFIG_NR_CPUS), param.cpu_id, 0);

	chk_n_set((param.bam_align == 0), param.bam_align, CRYPTO_BENCH_DATA_ALIGN);
	chk_n_set((param.bam_align > 8), param.bam_align, CRYPTO_BENCH_DATA_ALIGN);
	chk_n_set((param.bam_len > CRYPTO_BENCH_MAX_BAM_SZ), param.bam_len, CRYPTO_BENCH_MAX_BAM_SZ);

	chk_n_set((param.ciph_algo == 0), param.ciph_algo, 1);
	/* chk_n_set((param.auth_algo == 0), param.auth_algo, 1); */

	chk_n_set((param.max_sids > NSS_CRYPTO_MAX_IDXS), param.max_sids, def_param.max_sids);
	chk_n_set((param.max_sids == 0), param.max_sids, def_param.max_sids);

	switch (type) {
	case TYPE_BENCH:
		param.bench_mode = 1;
		param.mcmp_mode = 0;

		memset(&pattern_data[0], param.pattern, param.cipher_len);
		data_ptr = &pattern_data[0];

		break;

	case TYPE_MCMP:
		param.bench_mode = 0;
		param.mcmp_mode = 1;

		/* chk_n_set((param.auth_op == 0), param.auth_op, 1); */
		chk_n_set((param.cipher_op == 0), param.cipher_op, 1);

		param.hash_len = 12;

		data_ptr = &plain_text[0];

		break;

	default:
		crypto_bench_error("Invalid command passed\n");
		break;
	}

}

static void crypto_bench_flush(void)
{
	struct crypto_op *op;
	int i = 0;
	prep = 0;

	if (tx_thread != NULL) {
		kthread_stop(tx_thread);
		tx_thread = NULL;
	}
	wait_event_interruptible(tx_comp, (atomic_read(&tx_reqs) == 0));

	while (!list_empty(&op_head)) {
		op = list_first_entry(&op_head, struct crypto_op, node);

		list_del(&op->node);

		kfree(op->payload);

		kmem_cache_free(crypto_op_zone, op);
	}

	for (i = 0; i < param.max_sids; i++) {

		if (crypto_sid[i] < 0) {
			continue;
		}

		nss_crypto_session_free(crypto_hdl, crypto_sid[i]);

		crypto_sid[i] = -1;
	}

	memcpy(&param, &def_param, sizeof(struct crypto_bench_param));

	param.num_loops = 0;
}

/*
 * NOTE: Allocating extra 128 bytes to acccomodate result dump
 */
static int32_t crypto_bench_prep_op(void)
{
	struct nss_crypto_key c_key = {0};
	struct nss_crypto_key a_key = {0};
	struct nss_crypto_key *c_key_ptr = NULL;
	struct nss_crypto_key *a_key_ptr = NULL;
	struct nss_crypto_params crypto_params = {0};
	struct crypto_op *op = NULL;
	nss_crypto_status_t status;
	uint32_t iv_hash_len;
	uint8_t *str;
	int i = 0;

	if (prep) {
		return CRYPTO_BENCH_NOT_OK;
	}

	prep = 1;

	c_key.algo	= NSS_CRYPTO_CIPHER_AES_CBC;
	c_key.key 	= &cipher_key[0];
	c_key.key_len   = param.key_len;

	switch(param.auth_algo) {
	case 1:
		a_key.algo = NSS_CRYPTO_AUTH_SHA1_HMAC;
		a_key.key_len = NSS_CRYPTO_MAX_KEYLEN_SHA1;
		a_key.key = &auth_key[0];
		a_key_ptr = &a_key;

		str = "SHA1_HMAC";
		break;

	case 2:
		a_key.algo = NSS_CRYPTO_AUTH_SHA256_HMAC;
		a_key.key_len = NSS_CRYPTO_MAX_KEYLEN_SHA256;
		a_key.key = &auth_key[0];

		str = "SHA256_HMAC";
		break;

	default:
		a_key_ptr = NULL;
		str = param.auth_op ? "unsupported" : "none";
		break;
	}

	crypto_bench_info("auth algo %s\n", str);

	switch (param.ciph_algo) {
	case 1:
		c_key.algo = NSS_CRYPTO_CIPHER_AES_CBC;
		c_key.key = &cipher_key[0];

		chk_n_set((param.key_len <= 16), param.key_len, 16);
		chk_n_set((param.key_len > 16), param.key_len, 32);
		c_key.key_len = param.key_len;

		c_key_ptr = &c_key;

		str = "AES";
		break;

	case 2:
		c_key.algo = NSS_CRYPTO_CIPHER_DES;
		c_key.key = &cipher_key[0];

		chk_n_set((param.key_len <= 8), param.key_len, 8);
		chk_n_set((param.key_len > 8), param.key_len, 24);
		c_key.key_len = param.key_len;

		str = "DES";
		break;

	default:
		c_key_ptr = NULL;
		str = param.ciph_algo ? "unsupported" : "none";
		break;
	}

	crypto_bench_info("cipher algo %s\n", str);

	crypto_params.cipher_skip = param.cipher_skip;
	crypto_params.auth_skip = param.auth_skip;
	chk_n_set(param.cipher_op, crypto_params.req_type, NSS_CRYPTO_REQ_TYPE_ENCRYPT);
	chk_n_set((param.cipher_op > 1), crypto_params.req_type, NSS_CRYPTO_REQ_TYPE_DECRYPT);
	chk_n_update(param.auth_op, crypto_params.req_type, NSS_CRYPTO_REQ_TYPE_AUTH);


	if ((c_key.algo == NSS_CRYPTO_CIPHER_NONE) && (a_key.algo == NSS_CRYPTO_AUTH_NONE)) {
		return CRYPTO_BENCH_NOT_OK;
	}

	for (i = 0; i < param.max_sids; i++) {
		status = nss_crypto_session_alloc(crypto_hdl, c_key_ptr, a_key_ptr, &crypto_sid[i]);
		if (status != NSS_CRYPTO_STATUS_OK) {
			crypto_bench_error("UNABLE TO ALLOCATE CRYPTO SESSION\n");
			return CRYPTO_BENCH_NOT_OK;
		}

		nss_crypto_session_update(crypto_hdl, crypto_sid[i], &crypto_params);
	}

	crypto_bench_info("preparing crypto bench\n");

	iv_hash_len = NSS_CRYPTO_MAX_IVLEN_AES + CRYPTO_BENCH_RESULTS_SZ + sizeof(uint32_t);

	for (i = 0; i < param.num_reqs; i++) {

		op = kmem_cache_alloc(crypto_op_zone, GFP_KERNEL);
		if (op == NULL) {
			crypto_bench_error("UNABLE TO ALLOCATE SLAB MEMORY\n");
			return CRYPTO_BENCH_NOT_OK;
		}

		memset(op, 0x0, sizeof(struct crypto_op));

		op->data_len = param.bam_len;
		op->cipher_len = param.cipher_len;
		op->auth_len = param.auth_len;

		op->cipher_skip = param.cipher_skip;
		op->auth_skip = param.auth_skip;

		op->payload = kmalloc(op->data_len + param.bam_align + CRYPTO_BENCH_RESULTS_SZ, GFP_DMA);
		if (!op->payload) {
			crypto_bench_error("unable to allocate payload after - %d allocs\n", i);
			crypto_bench_flush();
			kmem_cache_free(crypto_op_zone, op);
			return CRYPTO_BENCH_NOT_OK;
		}
		op->payload_len = op->data_len + param.bam_align + CRYPTO_BENCH_RESULTS_SZ;
		op->data_vaddr = (uint8_t *)crypto_bench_align((uint32_t)op->payload, param.bam_align);

		op->iv_offset = op->cipher_skip - NSS_CRYPTO_MAX_IVLEN_AES;
		op->hash_offset = op->data_len;

		memcpy(op->data_vaddr + op->iv_offset, cipher_iv, NSS_CRYPTO_MAX_IVLEN_AES);

		memcpy(op->data_vaddr + op->cipher_skip, data_ptr, op->cipher_len);

		list_add_tail(&op->node, &op_head);
	}

	tx_thread = kthread_create(crypto_bench_tx, (void *) &op_head, "crypto_bench");

	kthread_bind(tx_thread, param.cpu_id);

	return CRYPTO_BENCH_OK;
}

static int32_t crypto_bench_prep_buf(struct crypto_op *op)
{
	struct nss_crypto_buf *buf;
	static int curr_sid;
	uint8_t *iv_addr;
	uint16_t iv_len;

	buf = nss_crypto_buf_alloc(crypto_hdl);
	if (buf == NULL) {
		crypto_bench_error("UNABLE TO ALLOCATE CRYPTO BUFFER\n");
		return CRYPTO_BENCH_NOT_OK;
	}

	nss_crypto_set_cb(buf, crypto_bench_done, op);
	nss_crypto_set_session_idx(buf, crypto_sid[curr_sid]);

	iv_addr = nss_crypto_get_ivaddr(buf);
	iv_len = (param.ciph_algo == 2) ? NSS_CRYPTO_MAX_IVLEN_DES : NSS_CRYPTO_MAX_IVLEN_AES;
	memcpy(iv_addr, cipher_iv, iv_len);

	nss_crypto_set_data(buf, op->data_vaddr, op->data_vaddr, op->data_len + param.hash_len);
	nss_crypto_set_transform_len(buf, op->cipher_len, op->auth_len);

	op->buf = buf;

	return CRYPTO_BENCH_OK;
}

void crypto_bench_mcmp(void)
{
	struct crypto_op *op;
	struct list_head *ptr;
	uint32_t encr_res, hash_res;
	uint8_t *encr_text;
	uint8_t *hash_text;

	chk_n_set((param.ciph_algo <=1), encr_text, &aes_encr_text[0]);
	chk_n_set((param.ciph_algo <=1), hash_text, &aes_sha1_hash[0]);

	chk_n_set((param.ciph_algo > 1), encr_text, &triple_des_encr_text[0]);
	chk_n_set((param.ciph_algo > 1), hash_text, &triple_des_sha1_hash[0]);


	list_for_each(ptr, &op_head) {
		op = list_entry(ptr, struct crypto_op, node);

		encr_res = memcmp(op->data_vaddr + op->cipher_skip, encr_text, op->cipher_len);
		param.mcmp_encr = param.mcmp_encr + !!(encr_res);

		if (param.auth_op) {
			hash_res = memcmp(op->data_vaddr + op->hash_offset, hash_text, param.hash_len);
			param.mcmp_hash = param.mcmp_hash + !!(hash_res);
		}

		memcpy(op->data_vaddr + op->cipher_skip, data_ptr, op->cipher_len);
	}
}

static int crypto_bench_tx(void *arg)
{
	uint32_t init_usecs, comp_usecs, delta_usecs, mbits;
	struct crypto_op *op;
	struct list_head *ptr;
	nss_crypto_status_t status;
	uint32_t loop_count = 0, total_mbps = 0;
	uint32_t reqs_completed = 0;

	param.peak_mbps = 0;
	param.avg_mbps = 0;

	for (;;) {
		/* Nothing to do */
		wait_event_interruptible(tx_start, (param.num_loops > 0) || kthread_should_stop());

		if (kthread_should_stop()) {
			break;
		}

		atomic_set(&tx_reqs, param.num_reqs);

		crypto_bench_debug("#");

		/* get start time */
		ktime_get_real_ts64(&init_time);

		/**
		 * Request submission
		 */

		list_for_each(ptr, &op_head) {

			op = list_entry(ptr, struct crypto_op, node);

			if (crypto_bench_prep_buf(op) != CRYPTO_BENCH_OK) {
				continue;
			}

			crypto_bench_dump_addr((uint8_t *)op->buf->data_addr, op->buf->data_len, "Before transformation");

			status = nss_crypto_transform_payload(crypto_hdl, op->buf);
			if (status != NSS_CRYPTO_STATUS_OK) {
				param.tx_err++;
				nss_crypto_buf_free(crypto_hdl, op->buf);
				atomic_dec(&tx_reqs);
			}
		}

		wait_event_interruptible(tx_comp, (atomic_read(&tx_reqs) == 0));

		/**
		 * Calculate time and output the Mbps
		 */

		init_usecs  = (init_time.tv_sec * 1000 * 1000) + (init_time.tv_nsec / NSEC_PER_USEC);
		comp_usecs  = (comp_time.tv_sec * 1000 * 1000) + (comp_time.tv_nsec / NSEC_PER_USEC);
		delta_usecs = comp_usecs - init_usecs;

		reqs_completed = param.num_reqs - atomic_read(&tx_reqs);

		mbits   = (reqs_completed * param.bam_len * 8);

		if (delta_usecs) {
			param.mbps = mbits / delta_usecs;
		}

		chk_n_set((param.peak_mbps < param.mbps), param.peak_mbps, param.mbps);

		crypto_bench_debug("bench: completed (reqs = %d, size = %d, time = %d, mbps = %d",
				reqs_completed, param.bam_len, delta_usecs, param.mbps);

		total_mbps += param.mbps;

		if (param.mcmp_mode) {
			crypto_bench_mcmp();
			crypto_bench_debug(", encr_fail = %d, hash_fail = %d", param.mcmp_encr, param.mcmp_hash);
		}
		crypto_bench_debug(")\n");

		loop_count++;

		if (param.num_loops == 0) {
			param.avg_mbps = (total_mbps / loop_count);
			crypto_bench_info("crypto bench is done\n");
		}
	}


	return 0;
}


/*
 * Context should be ATOMIC
 */
static void crypto_bench_done(struct nss_crypto_buf *buf)
{
	struct crypto_op *op;
	uint8_t *hash_addr;

	op = (struct crypto_op *)nss_crypto_get_cb_ctx(buf);

	hash_addr = nss_crypto_get_hash_addr(buf);
	memcpy(op->data_vaddr + op->hash_offset, hash_addr, param.hash_len);

	crypto_bench_dump_addr(op->data_vaddr + op->cipher_skip, op->cipher_len, "bench_done: data");
	crypto_bench_dump_addr(hash_addr, 128, "bench_done: hash");


	nss_crypto_buf_free(crypto_hdl, buf);

	if (atomic_dec_and_test(&tx_reqs)) {
		ktime_get_real_ts64(&comp_time);

		wake_up_interruptible(&tx_comp);
		param.num_loops--;
	}
}


static ssize_t crypto_bench_cmd_read(struct file *fp, char __user *ubuf, size_t cnt, loff_t *pos)
{
	return simple_read_from_buffer(ubuf, cnt, pos, help, strlen(help));
}


static ssize_t crypto_bench_cmd_write(struct file *fp, const char __user *ubuf, size_t cnt, loff_t *pos)
{
	uint8_t buf[64] = {0};
	int32_t status = CRYPTO_BENCH_OK;

	cnt = simple_write_to_buffer(buf, sizeof(buf), pos, ubuf, cnt);

	if (!strncmp(buf, "start", strlen("start")) && (tx_thread != NULL)) {	/* start */
		wake_up_process(tx_thread);
	} else if (!strncmp(buf, "flush", strlen("flush"))) {	/* flush */
		crypto_bench_flush();
	} else if (!strncmp(buf, "bench", strlen("bench"))) {	/* bench */
		crypto_bench_init_param(TYPE_BENCH);
		status = crypto_bench_prep_op();
	} else if (!strncmp(buf, "mcmp", strlen("mcmp"))) {	/* mcmp */
		crypto_bench_init_param(TYPE_MCMP);
		status = crypto_bench_prep_op();
	} else if (!strncmp(buf, "cal", strlen("cal"))) {	/* mcmp */
		crypto_bench_init_param(TYPE_CAL);
		status = crypto_bench_prep_op();
	} else if (!strncmp(buf, "default", strlen("default"))) {
		memcpy(&param, &def_param, sizeof(struct crypto_bench_param));
		crypto_bench_init_param(TYPE_BENCH);

		status = crypto_bench_prep_op();
		if (status == CRYPTO_BENCH_OK) {
			wake_up_process(tx_thread);
		}
	} else {
		crypto_bench_error("<bench>: invalid cmd\n");
	}

	if (status == CRYPTO_BENCH_NOT_OK) {
		crypto_bench_flush();
	}

	return cnt;
}

static const struct file_operations cmd_ops = {
	.read = crypto_bench_cmd_read,
	.write = crypto_bench_cmd_write,
};

nss_crypto_user_ctx_t crypto_bench_attach(nss_crypto_handle_t crypto)
{
	spin_lock_init(&op_lock);

	crypto_op_zone = kmem_cache_create("crypto_bench", sizeof(struct crypto_op), 0, SLAB_HWCACHE_ALIGN, NULL);

	crypto_hdl  = crypto;

	/* R/W, Hex */
	debugfs_create_x8("pattern", CRYPTO_BENCH_PERM_RW, droot, &param.pattern);
	debugfs_create_x32("encr", CRYPTO_BENCH_PERM_RW, droot, &param.cipher_op);
	debugfs_create_x32("auth", CRYPTO_BENCH_PERM_RW, droot, &param.auth_op);

	/* R/W U32 */
	debugfs_create_u32("cpu_id", CRYPTO_BENCH_PERM_RW, droot, &param.cpu_id);
	debugfs_create_u32("reqs", CRYPTO_BENCH_PERM_RW, droot, &param.num_reqs);
	debugfs_create_u32("loops", CRYPTO_BENCH_PERM_RW, droot, &param.num_loops);
	debugfs_create_u32("print", CRYPTO_BENCH_PERM_RW, droot, &param.print_mode);

	debugfs_create_u32("bam_len", CRYPTO_BENCH_PERM_RW, droot, &param.bam_len);
	debugfs_create_u32("cipher_len", CRYPTO_BENCH_PERM_RW, droot, &param.cipher_len);
	debugfs_create_u32("auth_len", CRYPTO_BENCH_PERM_RW, droot, &param.auth_len);
	debugfs_create_u32("hash_len", CRYPTO_BENCH_PERM_RW, droot, &param.hash_len);
	debugfs_create_u32("key_len", CRYPTO_BENCH_PERM_RW, droot, &param.key_len);

	debugfs_create_u32("bam_align", CRYPTO_BENCH_PERM_RW, droot, &param.bam_align);
	debugfs_create_u32("cipher_skip", CRYPTO_BENCH_PERM_RW, droot, &param.cipher_skip);
	debugfs_create_u32("auth_skip", CRYPTO_BENCH_PERM_RW, droot, &param.auth_skip);

	debugfs_create_u32("cipher_algo", CRYPTO_BENCH_PERM_RW, droot, &param.ciph_algo);
	debugfs_create_u32("auth_algo", CRYPTO_BENCH_PERM_RW, droot, &param.auth_algo);

	debugfs_create_u32("max_session", CRYPTO_BENCH_PERM_RW, droot, &param.max_sids);

	/* R/W buffer */
	debugfs_create_file("cmd", CRYPTO_BENCH_PERM_RW, droot, &op_head, &cmd_ops);

	/* Read, U32 */
	debugfs_create_u32("mbps", CRYPTO_BENCH_PERM_RO, droot, &param.mbps);
	debugfs_create_u32("bench", CRYPTO_BENCH_PERM_RO, droot, &param.bench_mode);
	debugfs_create_u32("mcmp", CRYPTO_BENCH_PERM_RO, droot, &param.mcmp_mode);
	debugfs_create_u32("hash_fail", CRYPTO_BENCH_PERM_RO, droot, &param.mcmp_hash);
	debugfs_create_u32("encr_fail", CRYPTO_BENCH_PERM_RO, droot, &param.mcmp_encr);
	debugfs_create_u32("peak_mbps", CRYPTO_BENCH_PERM_RO, droot, &param.peak_mbps);
	debugfs_create_u32("avg_mbps", CRYPTO_BENCH_PERM_RO, droot, &param.avg_mbps);
	debugfs_create_u32("enqueue_errors", CRYPTO_BENCH_PERM_RO, droot, &param.tx_err);

	return (nss_crypto_user_ctx_t)&op_head;
}

void crypto_bench_detach(nss_crypto_user_ctx_t ctx)
{
	crypto_bench_flush();
	kmem_cache_destroy(crypto_op_zone);
}

int __init crypto_bench_init(void)
{
	crypto_bench_info("crypto bench loaded - %s\n", NSS_CRYPTO_BUILD_ID);

	droot = debugfs_create_dir("crypto_bench", NULL);

	nss_crypto_register_user(crypto_bench_attach, crypto_bench_detach, "bench");

	return 0;
}

void __exit crypto_bench_exit(void)
{
	crypto_bench_info("Crypto bench unloaded\n");
	nss_crypto_unregister_user(crypto_hdl);
}

module_init(crypto_bench_init);
module_exit(crypto_bench_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("QCA NSS Crypto driver");
