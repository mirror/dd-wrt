/* Copyright (c) 2013-2018, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
#include <linux/scatterlist.h>

#include <nss_api_if.h>
#include <nss_crypto_api.h>
#include <nss_crypto_hlos.h>
#include <nss_crypto_defines.h>
#include <nss_crypto_hdr.h>

#define NSS_CRYPTO_MAX_IVLEN_AES 16
#define NSS_CRYPTO_MAX_IVLEN_DES 8

#define CRYPTO_BENCH_PERM_RO		0444
#define CRYPTO_BENCH_PERM_RW		0666

#define CRYPTO_BENCH_MAX_DATA_SZ	1536
#define CRYPTO_BENCH_MAX_BAM_SZ	(CRYPTO_BENCH_MAX_DATA_SZ + NSS_CRYPTO_MAX_IVLEN_AES)
#define CRYPTO_BENCH_RESULTS_SZ		12
#define CRYPTO_BENCH_DATA_ALIGN		4

#define CRYPTO_BENCH_PRN_LVL_DBG	3
#define CRYPTO_BENCH_PRN_LVL_INFO	2
#define CRYPTO_BENCH_PRN_LVL_ERR	1

#define CRYPTO_BENCH_OK		0
#define CRYPTO_BENCH_NOT_OK	-1


#define CRYPTO_BENCH_ASSERT(expr) do { \
	if (!(expr)) {	\
		printk("Operation failed - %d, %s\n", __LINE__, #expr); \
		panic("system is going down\n"); \
	} \
} while (0)

#define crypto_bench_print(level, fmt, args...)	do {	\
	if (CRYPTO_BENCH_PRN_LVL_##level <= param.print_mode) {	\
		printk(fmt, ##args);	\
	}	\
} while (0)

#define crypto_bench_error(fmt, args...)	crypto_bench_print(ERR, fmt, ##args)
#define crypto_bench_info(fmt, args...)		crypto_bench_print(INFO, fmt, ##args)
#define crypto_bench_debug(fmt, args...)	crypto_bench_print(DBG, fmt, ##args)

static DECLARE_WAIT_QUEUE_HEAD(tx_comp);
static DECLARE_WAIT_QUEUE_HEAD(tx_start);
static struct task_struct *tx_thread;

static struct timeval init_time;
static struct timeval comp_time;
static spinlock_t op_lock;
static struct nss_crypto_user *crypto_hdl;
static struct nss_crypto_user_ctx *ctx;

static struct kmem_cache *crypto_op_zone;

static struct dentry *droot;

static const uint8_t *help = "bench mcmp flush start default";

static atomic_t tx_reqs;

static uint8_t triple_des_cipher_key[] = {
	0xf5, 0x9b, 0xaa, 0x2f,
	0x32, 0x3e, 0x8c, 0xd6,
	0x48, 0xb5, 0x6e, 0x24,
	0x07, 0x73, 0x64, 0x67,
	0x18, 0xcf, 0x33, 0x79,
	0x8a, 0x43, 0x5c, 0xb0
};

static uint8_t triple_des_auth_key[] = {
	0x59, 0x7f, 0x88, 0x87,
	0xa4, 0x44, 0xec, 0x6b,
	0x28, 0xc5, 0x9 , 0x50,
	0x9f, 0x3e, 0x28, 0x3a,
	0xbb, 0xef, 0xca, 0xb5
};

static uint8_t triple_des_cipher_iv[] = {
	0x18, 0x73, 0xe9, 0xd9,
	0xff, 0xe4, 0x72, 0xe1
};

static uint8_t cipher_key[] = {
	0x8d, 0x57, 0xf2, 0x96,
	0xdc, 0x3c, 0x7c, 0x97,
	0xdf, 0xa9, 0xe3, 0xf3,
	0x03, 0x96, 0x0c, 0xe1,
	0x70, 0xcd, 0x48, 0xb7,
	0x38, 0x41, 0x6f, 0x5b,
	0x1f, 0xe1, 0x07, 0x01,
	0x18, 0x1a, 0xa2, 0x39
};

static uint8_t auth_key[] = {
	0xbd, 0xb6, 0xa6, 0xd3,
	0xbb, 0x1e, 0x7f, 0xbb,
	0x51, 0x13, 0x3f, 0xe8,
	0xcb, 0x0a, 0xb2, 0x1e,
	0x61, 0x6c, 0xf9, 0x41,
};

static uint8_t cipher_iv[NSS_CRYPTO_MAX_IVLEN_AES] = {
	0xb3, 0x2a, 0xf0, 0x3b,
	0xac, 0xfa, 0x03, 0x6b,
	0x09, 0xbe, 0x66, 0xf4,
	0x48, 0x99, 0x94, 0xbc
};

static uint8_t plain_text[] = {
	0x45, 0x00, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0x11, 0xf7, 0x18, 0xc0, 0xa8, 0x01, 0x0b,
	0xc0, 0xa8, 0x02, 0x0b, 0x00, 0x3f, 0x00, 0x3f,
	0x00, 0x5a, 0x0b, 0xbe, 0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
	0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
	0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
	0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
	0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43,
	0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
	0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x00, 0x04
};

static uint8_t aes128_enc_text[] = {
	0xfd, 0x6c, 0xdd, 0xc9, 0xa2, 0xe8, 0x24, 0xa8,
	0x55, 0x4a, 0xd9, 0xec, 0xaf, 0x06, 0xde, 0xbf,
	0xf6, 0x59, 0x5a, 0xa1, 0xd7, 0x24, 0x78, 0x56,
	0x15, 0x80, 0xcb, 0x54, 0xda, 0xb6, 0x6c, 0xc0,
	0x60, 0x45, 0xb0, 0x5f, 0x48, 0xdd, 0x96, 0xfb,
	0x1f, 0xfa, 0xf0, 0xd9, 0x96, 0xb3, 0x18, 0xd5,
	0xde, 0xd9, 0xe5, 0x2a, 0xe2, 0x2f, 0x7b, 0xb9,
	0x6b, 0x75, 0xeb, 0x63, 0x7d, 0x25, 0x1c, 0xe1,
	0xf3, 0x16, 0x26, 0x2a, 0x7b, 0x65, 0x2b, 0x07,
	0xe9, 0x47, 0xcf, 0x46, 0x7a, 0x5a, 0x53, 0xea,
	0xff, 0xd0, 0xff, 0x83, 0x6d, 0x7f, 0x4d, 0x9d,
	0xda, 0x88, 0xa2, 0x8d, 0x90, 0xcf, 0x47, 0x55,
	0x05, 0xc4, 0x86, 0xf1, 0x0a, 0x6f, 0x2c, 0x82,
	0x31, 0xb5, 0x59, 0x70, 0x61, 0xed, 0x58, 0xf1,
};

static uint8_t aes128_sha1_hash[] = {
	0xbd, 0xcd, 0x2f, 0x7f,
	0xf8, 0x9d, 0x04, 0xa9,
	0x9a, 0x30, 0x23, 0x3e
};

static uint8_t aes256_enc_text[] = {
	0x86, 0x3d, 0xe1, 0xa9, 0xfe, 0x57, 0xfd, 0xa5,
	0x11, 0x71, 0x5c, 0x3a, 0x59, 0x16, 0x02, 0x06,
	0x92, 0x6b, 0x24, 0x6e, 0xa5, 0x4f, 0xca, 0x1b,
	0x8c, 0xcd, 0x28, 0x22, 0x76, 0x43, 0x9f, 0x33,
	0xe4, 0x66, 0x29, 0x88, 0x56, 0x9f, 0xd0, 0x3b,
	0xfe, 0xde, 0xed, 0x2e, 0x85, 0x03, 0xd8, 0xf5,
	0x39, 0x9a, 0x5b, 0x2d, 0x46, 0x65, 0x5a, 0xa8,
	0x62, 0x8a, 0xb2, 0x37, 0x41, 0x98, 0x69, 0x71,
	0x1c, 0x7e, 0x8a, 0xc9, 0xd2, 0xd2, 0x99, 0xc2,
	0xf7, 0x5c, 0x31, 0xe3, 0x89, 0xb6, 0xc6, 0x70,
	0xd1, 0x50, 0xd9, 0x22, 0xc8, 0x62, 0xea, 0x10,
	0x5b, 0xe9, 0x22, 0x91, 0x6e, 0xc5, 0x79, 0x44,
	0xdc, 0x5d, 0xe9, 0xbe, 0x02, 0x84, 0x6f, 0xa8,
	0x7a, 0x3e, 0x6f, 0xdf, 0x61, 0x26, 0xef, 0xee
};

static uint8_t aes256_sha1_hash[] = {
	0x5d, 0x99, 0x78, 0xf5,
	0xf2, 0xae, 0x2f, 0xb4,
	0xfe, 0x01, 0x1e, 0x54
};

static uint8_t triple_des_enc_text[] = {
	0x08, 0xa9, 0x22, 0xf6, 0xb8, 0x7a, 0x84, 0x83,
	0x0a, 0x1d, 0x8b, 0xcc, 0x07, 0xc2, 0x0a, 0xaf,
	0x3f, 0x5f, 0xf5, 0x4a, 0xb6, 0xe1, 0x8c, 0xa5,
	0x4a, 0xe9, 0xaf, 0x59, 0x19, 0x64, 0x20, 0xdd,
	0xdf, 0xf6, 0x72, 0xbe, 0xb9, 0xf2, 0xa8, 0x20,
	0xbe, 0xda, 0x4b, 0x6c, 0xb1, 0x5e, 0x74, 0x7f,
	0x54, 0xa8, 0x4c, 0x7f, 0x14, 0xc6, 0xe7, 0x68,
	0x34, 0x11, 0xa4, 0xce, 0x02, 0x8c, 0xaf, 0x99,
	0xa5, 0x98, 0x53, 0x6e, 0x8e, 0x1d, 0xe8, 0x04,
	0x86, 0xa7, 0xdb, 0x5d, 0x7f, 0x0b, 0xe5, 0xd1,
	0x0b, 0xd1, 0x35, 0x52, 0x57, 0x06, 0xec, 0x53,
	0x0b, 0x62, 0xbb, 0x16, 0xe6, 0x9f, 0xa2, 0xe1,
	0x32, 0x38, 0x3a, 0xff, 0x04, 0x1e, 0x68, 0x34,
	0x62, 0x8c, 0x65, 0xf8, 0x25, 0x25, 0x20, 0x59
};

static uint8_t triple_des_sha1_hash[] = {
	0x93, 0xb2, 0xbf, 0xdf,
	0x2f, 0x2b, 0x41, 0x48,
	0xc8, 0x6f, 0xb9, 0xc8
};

static int32_t crypto_sid[NSS_CRYPTO_MAX_IDXS];
static uint32_t prep;
static uint8_t pattern_data[CRYPTO_BENCH_MAX_DATA_SZ];
static uint8_t *data_ptr;

/*
 * Prototypes
 */
nss_crypto_req_callback_t bench_done;

static int  crypto_bench_tx(void *arg);

struct crypto_op {
	struct list_head node;
	struct nss_crypto_hdr *ch;
	enum nss_crypto_op_dir op_dir;

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

	bool failed;
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

	uint32_t op_dir;	/**< encrypt(op=1) or decrypt(op=0) */

	uint32_t ciph_type;	/**< 1 for AES or 2 for DES */

	uint32_t algo_type;	/* different algo supported */
	uint32_t op_type;	/* operation type */

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
	.op_dir = 1,
	.ciph_type = 1,
	.algo_type = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC,
	.op_type = 3,
	.bam_align = 4,
	.num_reqs = 128,
	.hash_len = 12,
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
	.op_dir = 1,
	.ciph_type = 1,
	.algo_type = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC,
	.op_type = 3,
	.bam_align = 4,
	.num_reqs = 128,
	.hash_len = 12,
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

	for (i = 0; i < len; i = i+4)
		crypto_bench_debug("0x%02x, 0x%02x, 0x%02x, 0x%02x, %s", addr[i], addr[i+1], addr[i+2], addr[i+3], "\n");

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
} while (0)

#define chk_n_update(expr, field, def_val)	do {	\
	if ((expr)) {	\
		field |= def_val;	\
	}	\
} while (0)

static void crypto_bench_init_param(enum crypto_bench_type type)
{

	/*
	 * we don't support data more than 1536
	 */
	chk_n_set((param.cipher_len > CRYPTO_BENCH_MAX_DATA_SZ), param.cipher_len, CRYPTO_BENCH_MAX_DATA_SZ);
	chk_n_set((param.auth_len > CRYPTO_BENCH_MAX_DATA_SZ), param.auth_len, CRYPTO_BENCH_MAX_DATA_SZ);

	chk_n_set((param.cipher_skip < NSS_CRYPTO_MAX_IVLEN_AES), param.cipher_skip, NSS_CRYPTO_MAX_IVLEN_DES);

	chk_n_set((param.cpu_id > CONFIG_NR_CPUS), param.cpu_id, 0);

	chk_n_set((param.bam_align == 0), param.bam_align, CRYPTO_BENCH_DATA_ALIGN);
	chk_n_set((param.bam_align > 8), param.bam_align, CRYPTO_BENCH_DATA_ALIGN);
	chk_n_set((param.bam_len > CRYPTO_BENCH_MAX_BAM_SZ), param.bam_len, CRYPTO_BENCH_MAX_BAM_SZ);
	chk_n_set((param.algo_type == 0), param.algo_type, 1);
	chk_n_set((param.algo_type == 0), param.op_type, 1);

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

		if (crypto_sid[i] < 0)
			continue;

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
	struct nss_crypto_session_data data = {0};
	struct crypto_op *op = NULL;
	uint32_t iv_hash_len;
	uint16_t iv_len;
	uint8_t *str = "none";
	int i = 0;
	int status;

	if (prep)
		return CRYPTO_BENCH_NOT_OK;

	prep = 1;

	if ((param.ciph_type == 1) || (param.ciph_type == 2)) {
		data.cipher_key = &cipher_key[0];
		data.auth_key = &auth_key[0];
		data.auth_keylen = ARRAY_SIZE(auth_key);
	} else if (param.ciph_type == 3) {
		data.cipher_key = &triple_des_cipher_key[0];
		data.auth_key = &triple_des_auth_key[0];
		data.auth_keylen = ARRAY_SIZE(triple_des_auth_key);
	}

	if (param.op_type == 3) {
		switch (param.algo_type) {
		case NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC;
			str = "AES128_GCM_GMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC;
			str = "AES128_CBC_SHA160_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA256_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA256_HMAC;
			str = "AES128_CBC_SHA256_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES192_GCM_GMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES192_GCM_GMAC;
			str = "AES192_GCM_GMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA160_HMAC;
			str = "AES192_CBC_SHA160_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA256_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC_SHA256_HMAC;
			str = "AES192_CBC_SHA256_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES256_GCM_GMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES128_GCM_GMAC;
			str = "AES256_GCM_GMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC;
			str = "AES256_CBC_SHA160_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA256_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA256_HMAC;
			str = "AES256_CBC_SHA256_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC;
			str = "3DES_CBC_SHA160_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA256_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA256_HMAC;
			str = "3DES_CBC_SHA256_HMAC";
			break;

		default:
			str = param.algo_type ? "unsupported" : "none";
			break;
		}
	}

	if (param.op_type == 1) {
		switch (param.algo_type) {
		case NSS_CRYPTO_CMN_ALGO_AES128_CBC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES128_CBC;
			str = "AES128_CBC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES192_CBC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES192_CBC;
			str = "AES192_CBC";
			break;
		case NSS_CRYPTO_CMN_ALGO_AES256_CBC:
			data.algo = NSS_CRYPTO_CMN_ALGO_AES256_CBC;
			str = "AES256_CBC";
			break;
		default:
			str = param.ciph_type ? "unsupported" : "none";
			break;
		}
	}

	crypto_bench_info("cipher algo %s\n", str);

	if (param.op_type == 2) {
		switch (param.algo_type) {
		case NSS_CRYPTO_CMN_ALGO_MD5_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_MD5_HMAC;
			str = "MD5_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_SHA160_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_SHA160_HMAC;
			str = "SHA160_HMAC";
			break;
		case NSS_CRYPTO_CMN_ALGO_SHA256_HMAC:
			data.algo = NSS_CRYPTO_CMN_ALGO_SHA256_HMAC;
			str = "SHA256_HMAC";
			break;
		default:
			str = param.ciph_type ? "unsupported" : "none";
			break;
		}
	}

	crypto_bench_info("auth algo %s\n", str);

	if ((data.algo < NSS_CRYPTO_CMN_ALGO_NULL) && (data.algo >= NSS_CRYPTO_CMN_ALGO_MAX))
		return CRYPTO_BENCH_NOT_OK;

	for (i = 0; i < param.max_sids; i++) {
		status = nss_crypto_session_alloc(crypto_hdl, &data, &crypto_sid[i]);
		if (status < 0) {
			crypto_bench_error("UNABLE TO ALLOCATE CRYPTO SESSION\n");
			return CRYPTO_BENCH_NOT_OK;
		}
	}

	crypto_bench_info("preparing crypto bench\n");

	/* TODO */
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

		op->op_dir = param.op_dir;
		op->cipher_skip = param.cipher_skip;
		op->auth_skip = param.auth_skip;

		op->payload = kmalloc(op->data_len + SMP_CACHE_BYTES + CRYPTO_BENCH_RESULTS_SZ, GFP_DMA);
		if (!op->payload) {
			crypto_bench_error("unable to allocate payload after - %d allocs\n", i);
			crypto_bench_flush();
			kmem_cache_free(crypto_op_zone, op);
			return CRYPTO_BENCH_NOT_OK;
		}
		op->payload_len = op->data_len + SMP_CACHE_BYTES + CRYPTO_BENCH_RESULTS_SZ;
		op->data_vaddr = PTR_ALIGN(op->payload, SMP_CACHE_BYTES);

		iv_len = (param.ciph_type <= 2) ? NSS_CRYPTO_MAX_IVLEN_AES : NSS_CRYPTO_MAX_IVLEN_DES;

		op->iv_offset = op->cipher_skip - iv_len;
		op->hash_offset = op->data_len;

		if (param.ciph_type <= 2)
			memcpy(op->data_vaddr + op->iv_offset, cipher_iv, sizeof(cipher_iv));
		else
			memcpy(op->data_vaddr + op->iv_offset, triple_des_cipher_iv, sizeof(triple_des_cipher_iv));

		memcpy(op->data_vaddr + op->cipher_skip, data_ptr, op->cipher_len);

		list_add_tail(&op->node, &op_head);
	}

	tx_thread = kthread_create(crypto_bench_tx, (void *) &op_head, "crypto_bench");

	kthread_bind(tx_thread, param.cpu_id);

	return CRYPTO_BENCH_OK;
}

static int32_t crypto_bench_prep_buf(struct crypto_op *op)
{
	struct nss_crypto_hdr *ch;
	struct scatterlist src;
	static int curr_sid;
	uint32_t *iv_addr;
	uint16_t iv_len;
	uint16_t icv_len;
	int num_frag;
	uint16_t tot_len;
	uint16_t fixed_size;
	bool ahash = (op->op_dir == NSS_CRYPTO_OP_DIR_AUTH);

	iv_len = (param.ciph_type <= 2) ? NSS_CRYPTO_MAX_IVLEN_AES : NSS_CRYPTO_MAX_IVLEN_DES;
	icv_len = param.hash_len;

	sg_init_one(&src, op->data_vaddr, op->data_len + icv_len);
	num_frag = sg_nents(&src);

	ch = nss_crypto_hdr_alloc(crypto_hdl, crypto_sid[curr_sid], num_frag, iv_len, icv_len, ahash);
	if (ch == NULL) {
		crypto_bench_error("UNABLE TO ALLOCATE CRYPTO BUFFER\n");
		return CRYPTO_BENCH_NOT_OK;
	}

	nss_crypto_hdr_set_skip(ch, op->cipher_skip);
	nss_crypto_hdr_set_op(ch, op->op_dir);

	nss_crypto_hdr_map_sglist(ch, &src, icv_len);

	fixed_size = 2 * ch->in_frags * sizeof(struct nss_crypto_frag) + sizeof(struct nss_crypto_hdr);
	tot_len = ch->iv_len + ch->hmac_len + ch->buf_len + ch->priv_len + fixed_size;

	nss_crypto_hdr_set_tot_len(ch, tot_len);

	iv_addr = nss_crypto_hdr_get_iv(ch);

	if (param.ciph_type == 3)
		memcpy(iv_addr, triple_des_cipher_iv, sizeof(triple_des_cipher_iv));
	else
		memcpy(iv_addr, cipher_iv, sizeof(cipher_iv));

	op->ch = ch;
	op->failed = false;

	return CRYPTO_BENCH_OK;
}

void crypto_bench_mcmp(void)
{
	struct crypto_op *op;
	struct list_head *ptr;
	uint32_t encr_res, hash_res;
	uint8_t *encr_text;
	uint8_t *hash_text;

	chk_n_set((param.ciph_type <= 1), encr_text, &aes128_enc_text[0]);
	chk_n_set((param.ciph_type <= 1), hash_text, &aes128_sha1_hash[0]);

	chk_n_set((param.ciph_type == 2), encr_text, &aes256_enc_text[0]);
	chk_n_set((param.ciph_type == 2), hash_text, &aes256_sha1_hash[0]);

	chk_n_set((param.ciph_type > 2), encr_text, &triple_des_enc_text[0]);
	chk_n_set((param.ciph_type > 2), hash_text, &triple_des_sha1_hash[0]);

	list_for_each(ptr, &op_head) {
		op = list_entry(ptr, struct crypto_op, node);

		if (op->failed)
			continue;

		encr_res = memcmp(op->data_vaddr + op->cipher_skip, encr_text, op->cipher_len);
		param.mcmp_encr = param.mcmp_encr + !!(encr_res);

		if ((param.op_type == 3) || (param.op_type == 2)) {
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
	int32_t status;
	uint32_t loop_count = 0, total_mbps = 0;
	uint32_t reqs_completed = 0;

	param.peak_mbps = 0;
	param.avg_mbps = 0;

	for (;;) {
		/* Nothing to do */
		wait_event_interruptible(tx_start, (param.num_loops > 0) || kthread_should_stop());

		if (kthread_should_stop())
			break;

		atomic_set(&tx_reqs, param.num_reqs);

		crypto_bench_debug("#");

		/* get start time */
		do_gettimeofday(&init_time);

		/**
		 * Request submission
		 */

		list_for_each(ptr, &op_head) {

			op = list_entry(ptr, struct crypto_op, node);

			if (crypto_bench_prep_buf(op) != CRYPTO_BENCH_OK)
				continue;

			crypto_bench_dump_addr((uint8_t *)op->ch->data_addr, op->ch->data_len, "Before transformation");

			status = nss_crypto_transform_payload(crypto_hdl, op->ch, bench_done, op);
			if (status < 0) {
				param.tx_err++;
				nss_crypto_hdr_free(crypto_hdl, op->ch);
				op->failed = true;
				atomic_dec(&tx_reqs);
			}
		}

		wait_event_interruptible(tx_comp, (atomic_read(&tx_reqs) == 0));

		/**
		 * Calculate time and output the Mbps
		 */

		init_usecs  = (init_time.tv_sec * 1000 * 1000) + init_time.tv_usec;
		comp_usecs  = (comp_time.tv_sec * 1000 * 1000) + comp_time.tv_usec;
		delta_usecs = comp_usecs - init_usecs;

		reqs_completed = param.num_reqs - atomic_read(&tx_reqs);

		mbits   = (reqs_completed * param.bam_len * 8);

		if (delta_usecs)
			param.mbps = mbits / delta_usecs;

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
void crypto_bench_done(void *app_data, struct nss_crypto_hdr *ch, uint8_t error)
{
	struct nss_crypto_buf *buf;
	struct crypto_op *op;

	buf = nss_crypto_hdr_get_buf(ch);
	op = (struct crypto_op *)buf->app_data;

	crypto_bench_dump_addr(op->data_vaddr + op->cipher_skip, op->cipher_len, "bench_done: data");
	crypto_bench_dump_addr(hash_addr, 128, "bench_done: hash");

	nss_crypto_hdr_free(crypto_hdl, ch);

	if (atomic_dec_and_test(&tx_reqs)) {
		do_gettimeofday(&comp_time);

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
		if (status == CRYPTO_BENCH_OK)
			wake_up_process(tx_thread);
	} else {
		crypto_bench_error("<bench>: invalid cmd\n");
	}

	if (status == CRYPTO_BENCH_NOT_OK)
		crypto_bench_flush();

	return cnt;
}

static const struct file_operations cmd_ops = {
	.read = crypto_bench_cmd_read,
	.write = crypto_bench_cmd_write,
};

void  crypto_bench_attach(void *app_data, struct nss_crypto_user *user)
{
	spin_lock_init(&op_lock);

	crypto_op_zone = kmem_cache_create("crypto_bench", sizeof(struct crypto_op), 0, SLAB_HWCACHE_ALIGN, NULL);

	crypto_hdl  = user;

	/* R/W, Hex */
	debugfs_create_x8("pattern", CRYPTO_BENCH_PERM_RW, droot, &param.pattern);
	debugfs_create_x32("op_dir", CRYPTO_BENCH_PERM_RW, droot, &param.op_dir);

	/* R/W U32 */
	debugfs_create_u32("cpu_id", CRYPTO_BENCH_PERM_RW, droot, &param.cpu_id);
	debugfs_create_u32("reqs", CRYPTO_BENCH_PERM_RW, droot, &param.num_reqs);
	debugfs_create_u32("loops", CRYPTO_BENCH_PERM_RW, droot, &param.num_loops);
	debugfs_create_u32("print", CRYPTO_BENCH_PERM_RW, droot, &param.print_mode);

	debugfs_create_u32("bam_len", CRYPTO_BENCH_PERM_RW, droot, &param.bam_len);
	debugfs_create_u32("cipher_len", CRYPTO_BENCH_PERM_RW, droot, &param.cipher_len);
	debugfs_create_u32("auth_len", CRYPTO_BENCH_PERM_RW, droot, &param.auth_len);
	debugfs_create_u32("hash_len", CRYPTO_BENCH_PERM_RW, droot, &param.hash_len);

	debugfs_create_u32("bam_align", CRYPTO_BENCH_PERM_RW, droot, &param.bam_align);
	debugfs_create_u32("cipher_skip", CRYPTO_BENCH_PERM_RW, droot, &param.cipher_skip);
	debugfs_create_u32("auth_skip", CRYPTO_BENCH_PERM_RW, droot, &param.auth_skip);
	debugfs_create_u32("cipher_type", CRYPTO_BENCH_PERM_RW, droot, &param.ciph_type);
	debugfs_create_u32("algo_type", CRYPTO_BENCH_PERM_RW, droot, &param.algo_type);
	debugfs_create_u32("op_type", CRYPTO_BENCH_PERM_RW, droot, &param.op_type);

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
}

void crypto_bench_detach(void *app_data, struct nss_crypto_user *user)
{
	crypto_bench_flush();
	kmem_cache_destroy(crypto_op_zone);
}

int __init crypto_bench_init(void)
{
	ctx = kmalloc(sizeof(struct nss_crypto_user_ctx), GFP_KERNEL);
	if (!ctx) {
		return -1;
	}

	memset(ctx, 0, sizeof(struct nss_crypto_user_ctx));
	crypto_bench_info("crypto bench loaded - %s\n", NSS_CRYPTO_BUILD_ID);

	droot = debugfs_create_dir("crypto_bench", NULL);

	ctx->attach = crypto_bench_attach;
	ctx->detach = crypto_bench_detach;
	strlcpy(ctx->name, "bench", sizeof(ctx->name));
	ctx->hdr_pool_sz = 1024;
	ctx->default_hdr_sz = 512;
	ctx->timeout_ticks = 1;

	nss_crypto_register_user(ctx, NULL);

	bench_done = crypto_bench_done;
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
MODULE_AUTHOR("Qualcomm Atheros Inc");
