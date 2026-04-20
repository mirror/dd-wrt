/* Copyright (c) 2013, 2015-2017, The Linux Foundation. All rights reserved.
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
#ifndef __NSS_CRYPTO_CTRL_H
#define __NSS_CRYPTO_CTRL_H

/**
 * @brief session free timeout parameters
 */
#define NSS_CRYPTO_RESP_TIMEO_TICKS msecs_to_jiffies(3000)	/* Timeout for NSS reponses to Host messages */
#define NSS_CRYPTO_PERF_LEVEL_TIMEO_TICKS msecs_to_jiffies(10000)	/* Timeout for NSS reponses to Host messages */

#define NSS_CRYPTO_SESSION_BITMAP BITS_TO_LONGS(NSS_CRYPTO_MAX_IDXS)

/**
 * @brief max key lengths supported for various algorithms
 */
enum nss_crypto_keylen_supp {
	NSS_CRYPTO_KEYLEN_AES128 = 16,		/**< AES-128 bit */
	NSS_CRYPTO_KEYLEN_AES256 = 32,		/**< AES-256 bit */
	NSS_CRYPTO_KEYLEN_SHA1HMAC = 20,	/**< SHA1-HMAC */
	NSS_CRYPTO_KEYLEN_SHA256HMAC = 32,	/**< SHA256-HMAC */
	NSS_CRYPTO_KEYLEN_DES = 8,		/**< DES-64 bit */
	NSS_CRYPTO_KEYLEN_3DES = 24,		/**< 3DES-192 bit */
};

/**
 * @brief NSS Crypto state
 */
enum nss_crypto_state {
	NSS_CRYPTO_STATE_NOT_READY = 0,      /**< Crypto state is not Ready */
	NSS_CRYPTO_STATE_READY,              /**< Crypto state is ready */
	NSS_CRYPTO_STATE_INITIALIZED,        /**< Crypto engines are initialized */
	NSS_CRYPTO_STATE_MAX
};

struct nss_crypto_work {
	struct delayed_work work;	/* Work Structure */
	uint32_t session_idx;		/* session for which work is scheduled */
};

struct nss_crypto_encr_cfg {
	uint32_t cfg;
	uint8_t key[NSS_CRYPTO_CKEY_SZ];
};

struct nss_crypto_auth_cfg {
	uint32_t cfg;
	uint32_t *iv;
	uint8_t key[NSS_CRYPTO_AKEY_SZ];
};

struct nss_crypto_ctrl_idx {
	struct nss_crypto_idx idx;
	struct nss_crypto_cmd_block *cblk;
};

/**
 * @brief Crypto control specific structure that describes an Engine
 */
struct nss_crypto_ctrl_eng {
	uint32_t cmd_base;	/**< base address for command descriptors (BAM prespective) */
	uint8_t *crypto_base;	/**< base address for crypto register writes */
	uint32_t bam_pbase;	/**< physical base address for BAM register writes */
	uint8_t *bam_base;	/**< base address for BAM regsiter writes */
	uint32_t bam_ee;	/**< BAM execution enivironment for the crypto engine */
	struct device *dev;	/**< HLOS device type for the crypto engine */

	struct nss_crypto_desc *hw_desc[NSS_CRYPTO_BAM_PP]; 		/**< H/W descriptors BAM rings, command descriptors */
	struct nss_crypto_ctrl_idx idx_tbl[NSS_CRYPTO_MAX_IDXS];	/**< index table */

	struct nss_crypto_stats stats;	/**< engine stats */
	struct dentry *stats_dentry;	/**< debufs entry corresponding to stats directory */
};

/**
 * @brief Per index information required for getting information
 */
struct nss_crypto_idx_info {
	struct nss_crypto_key ckey;			/**< cipher key */
	struct nss_crypto_key akey;			/**< auth key */

	struct nss_crypto_stats stats;			/**< session stats */

	enum nss_crypto_session_state state;		/**< Indicates whether session is active or not */

	struct dentry *stats_dentry;			/**< debufs entry corresponding to stats */
	struct dentry *cfg_dentry;			/**< debufs entry corresponding to config */

	uint16_t req_type;				/**< transform is for encryption or decryption */
	enum nss_crypto_max_blocklen cipher_blk_len;	/**< cipher block len */
	enum nss_crypto_max_ivlen iv_len;		/**< iv len */
};

/**
 * @brief Host maintained control stats
 */
struct nss_crypto_ctrl_stats {
	atomic_t session_alloc;			/**< Sessions allocated */
	atomic_t session_free;			/**< Sessions freed */
	atomic_t session_alloc_fail;		/**< Session alloc failures */
};

/**
 * @brief NSS crypto clock control
 */
struct nss_crypto_clock {
	struct clk *clk;			/**< Linux clock */
	uint32_t turbo_freq;			/**< turbo frequency */
	uint32_t nominal_freq;			/**< nominal frequency */
};

/**
 * @brief Main Crypto Control structure, holds information about number of session indexes
 * number of engines etc.,
 *
 * @note currently we support 4 indexes, in future it will allocate more
 */
struct nss_crypto_ctrl {
	unsigned long idx_bitmap[NSS_CRYPTO_SESSION_BITMAP];
				/**< session allocation bitmap, upto NSS_CRYPTO_MAX_IDXS can be used */
	unsigned long idx_state_bitmap[NSS_CRYPTO_SESSION_BITMAP];
				/**< session state bitmap, upto NSS_CRYPTO_MAX_IDXS can be used */

	uint32_t num_idxs;			/**< number of allocated indexes */
	uint32_t num_eng;			/**< number of available engines */

	atomic_t crypto_state;			/**< crypto devices initialized or not */
	atomic_t perf_level;			/**< crypto PM perf level */

	spinlock_t lock;			/**< lock */
	struct mutex mutex;			/**< mutex lock */
	struct semaphore sem;			/**< semaphore lock */

	struct completion complete;		/**< completion for NSS message */
	struct completion perf_complete;	/**< completion for NSS message */

	atomic_t complete_timeo;		/**< indicates whether completion has timeout */

	struct delayed_work crypto_work;	/**< crypto_work structure */

	struct nss_crypto_ctrl_eng *eng;	/**< pointer to engines control data information */

	struct nss_crypto_stats total_stats;	/**< crypto total stats */
	struct nss_crypto_ctrl_stats ctrl_stats;
						/**< crypto control stats */

	struct dentry *root_dentry;		/**< debufs entry corresponding to qca-nss-crypto directory */
	struct dentry *stats_dentry;		/**< debufs entry corresponding to stats directory */
	struct dentry *cfg_dentry;		/**< debufs entry corresponding to config directory */

	struct nss_crypto_clock *clocks;	/**< array of crypto clocks */
	size_t max_clocks;			/**< maximum no. of available clocks */

	struct nss_crypto_idx_info idx_info[NSS_CRYPTO_MAX_IDXS];
						/**< per index info */
};

/**
 * @brief Driver context structure
 */
struct nss_crypto_drv_ctx {
	struct nss_ctx_instance *drv_hdl;	/**< NSS driver handle */
	void *pm_hdl;				/**< NSS PM handle */
};

/*
 * @brief set crypto state
 */
static inline void nss_crypto_set_state(struct nss_crypto_ctrl *ctrl, enum nss_crypto_state state)
{
	atomic_set(&ctrl->crypto_state, state);
}

/*
 * @brief check crypto state
 */
static inline bool nss_crypto_check_state(struct nss_crypto_ctrl *ctrl, enum nss_crypto_state state)
{
	return atomic_read(&ctrl->crypto_state) == state;
}

/*
 * @brief reset crypto state
 */
static inline void nss_crypto_reset_state(struct nss_crypto_ctrl *ctrl)
{
	atomic_set(&ctrl->crypto_state, NSS_CRYPTO_STATE_NOT_READY);
}

/**
 * @brief Initialize and allocate descriptor memory for a given pipe
 *
 * @param eng[IN] Engine context for control operation
 * @param idx[IN] Pipe pair index number
 * @param desc_paddr[IN] physical address of H/W descriptor
 * @param desc_vaddr[IN] virtual address of H/W descriptor
 *
 */
void nss_crypto_pipe_init(struct nss_crypto_ctrl_eng *eng, uint32_t idx, uint32_t *desc_paddr, struct nss_crypto_desc **desc_vaddr);

/**
 * @brief initiallize the index table per engine
 *
 * @param eng[IN] per engine state
 * @param msg[OUT] message to NSS for each allocated index
 *
 * @return status of the call
 */
nss_crypto_status_t nss_crypto_idx_init(struct nss_crypto_ctrl_eng *eng, struct nss_crypto_idx *msg);

/**
 * @brief Initiallize the generic control entities in nss_crypto_ctrl
 */
void nss_crypto_ctrl_init(void);

/**
 * @brief update IV parameters
 *
 * @param session_idx[IN] session index
 * @param state[IN] crypto state (active/free)
 * @param cipher[IN] cipher algorithm
 *
 * @return status of the call
 */
nss_crypto_status_t nss_crypto_send_session_update(uint32_t session_idx, enum nss_crypto_session_state state, enum nss_crypto_cipher cipher);

/**
 * @brief reallocate memory.
 *
 * @param src[IN] pointer to src memory
 * @param src_len[IN]  length of src memory
 * @param dst_len[IN]  length of new meory required
 *
 * @return pointer to new memory allocated
 */
void *nss_crypto_mem_realloc(void *src, size_t src_len, size_t dst_len);

/**
 * @brief deallocate crypto session
 *
 * @param session_idx[IN] session index
 *
 * @return result of the call
 */
void nss_crypto_idx_free(uint32_t session_idx);

/*
 * @brief checks session's current state
 *
 * @param idx_info[IN] pointer to index info structure
 *
 * @return true if session is free
 */
bool nss_crypto_chk_idx_isfree(struct nss_crypto_idx_info *idx_info);
#endif /* __NSS_CRYPTO_CTRL_H*/
