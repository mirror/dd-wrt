/* Copyright (c) 2013, 2017, The Linux Foundation. All rights reserved.
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
#ifndef __NSS_CRYPTO_HW_H
#define __NSS_CRYPTO_HW_H

/**
 * H/W registers & values
 */
#define CRYPTO_MASK_ALL				~((uint32_t)(0))
#define CRYPTO_MAX_BURST			64

#define CRYPTO_BURST2BEATS(x)			((x >> 3) - 1)

#define CRYPTO_CONFIG_REQ_SIZE(x)		(x << 17)
#define CRYPTO_CONFIG_DOUT_INTR			(0x1 << 3)
#define CRYPTO_CONFIG_DIN_INTR			(0x1 << 2)
#define CRYPTO_CONFIG_DOP_INTR			(0x1 << 1)
#define CRYPTO_CONFIG_HIGH_SPEED_EN		(0x1 << 4)
#define CRYPTO_CONFIG_PIPE_SEL(x)		(x << 5)
#define CRYPTO_CONFIG_LITTLE_ENDIAN		(0x1 << 9)
#define CRYPTO_CONFIG_MAX_REQS(x)		(x << 14)
#define CRYPTO_CONFIG_RESET			0xE001F

#define CRYPTO_ENCR_SEG_CFG_ALG_DES		0x1
#define CRYPTO_ENCR_SEG_CFG_ALG_AES		0x2
#define CRYPTO_ENCR_SEG_CFG_KEY_AES128		(0x0 << 3)
#define CRYPTO_ENCR_SEG_CFG_KEY_AES256		(0x2 << 3)
#define CRYPTO_ENCR_SEG_CFG_KEY_SINGLE_DES	(0x0 << 3)
#define CRYPTO_ENCR_SEG_CFG_KEY_TRIPLE_DES	(0x1 << 3)
#define CRYPTO_ENCR_SEG_CFG_MODE_ECB		(0x0 << 6)
#define CRYPTO_ENCR_SEG_CFG_MODE_CBC		(0x1 << 6)
#define CRYPTO_ENCR_SEG_CFG_MODE_CTR		(0x2 << 6)
#define CRYPTO_ENCR_SEG_CFG_MODE_XTS		(0x3 << 6)
#define CRYPTO_ENCR_SEG_CFG_MODE_CCM		(0x4 << 6)
#define CRYPTO_ENCR_SEG_CFG_ENC			(0x1 << 10)
#define CRYPTO_ENCR_SEG_CFG_PIPE_KEYS		(0x1 << 15)

#define CRYPTO_SET_ENCRYPT(cfg)			((cfg) |= CRYPTO_ENCR_SEG_CFG_ENC)
#define CRYPTO_SET_DECRYPT(cfg)			((cfg) &= ~CRYPTO_ENCR_SEG_CFG_ENC)

#define CRYPTO_GOPROC_SET			0x1
#define CRYPTO_GOPROC_CLR_CNTXT			(0x1 << 1)
#define CRYPTO_GOPROC_RESULTS_DUMP		(0x1 << 2)

#define CRYPTO_AUTH_SEG_CFG_ALG_SHA		0x1
#define CRYPTO_AUTH_SEG_CFG_MODE_HMAC		(0x1 << 6)
#define CRYPTO_AUTH_SEG_CFG_SIZE_SHA1		(0x0 << 9)
#define CRYPTO_AUTH_SEG_CFG_SIZE_SHA2		(0x1 << 9)
#define CRYPTO_AUTH_SEG_CFG_POS_BEFORE		(0x0 << 14)
#define CRYPTO_AUTH_SEG_CFG_POS_AFTER		(0x1 << 14)
#define CRYPTO_AUTH_SEG_CFG_LAST		(0x1 << 16)
#define CRYPTO_AUTH_SEG_CFG_FIRST		(0x1 << 17)
#define CRYPTO_AUTH_SEG_CFG_PIPE_KEYS		(0x1 << 19)

#define CRYPTO_BAM_DESC_INT			(0x1 << 15)
#define CRYPTO_BAM_DESC_EOT			(0x1 << 14)
#define CRYPTO_BAM_DESC_EOB			(0x1 << 13)
#define CRYPTO_BAM_DESC_NWD			(0x1 << 12)
#define CRYPTO_BAM_DESC_CMD			(0x1 << 11)
#define CRYPTO_BAM_DESC_LOCK			(0x1 << 10)
#define CRYPTO_BAM_DESC_UNLOCK			(0x1 << 9)

#define CRYPTO_BAM_P_CTRL_EN			(0x1 << 1)
#define CRYPTO_BAM_P_CTRL_SYS_MODE		(0x1 << 5)
#define CRYPTO_BAM_P_CTRL_DIRECTION(n)		((n & 0x1) << 3)
#define CRYPTO_BAM_P_CTRL_LOCK_GROUP(n)		(n << 16)
#define CRYPTO_BAM_DESC_CNT_TRSHLD_VAL		64
#define	CRYPTO_BAM_CNFG_BITS_BAM_FULL_PIPE	(1 << 11)

#define CRYPTO_ADDR_MSK				0x00FFFFFF
#define CRYPTO_CMD_ADDR(addr)		((addr) & CRYPTO_ADDR_MSK)

#define CRYPTO_BASE			0x1A000
#define CRYPTO_SEG_SIZE			(CRYPTO_BASE + 0x110)
#define CRYPTO_GO_PROC			(CRYPTO_BASE + 0x120)
#define CRYPTO_ENCR_SEG_CFG		(CRYPTO_BASE + 0x200)
#define CRYPTO_ENCR_SEG_SIZE		(CRYPTO_BASE + 0x204)
#define CRYPTO_ENCR_SEG_START		(CRYPTO_BASE + 0x208)
#define CRYPTO_ENCR_CNTRn_IVn(n)	(CRYPTO_BASE + 0x20C + (0x4 * n))
#define CRYPTO_ENCR_CNTR_MASK		(CRYPTO_BASE + 0x21c)
#define CRYPTO_ENCR_CCM_INIT_CNTRn(n)	(CRYPTO_BASE + 0x220 + (0x4 * n))
#define CRYPTO_AUTH_SEG_CFG		(CRYPTO_BASE + 0x300)
#define CRYPTO_AUTH_SEG_SIZE 		(CRYPTO_BASE + 0x304)
#define CRYPTO_AUTH_SEG_START		(CRYPTO_BASE + 0X308)
#define CRYPTO_AUTH_IVn(n)		(CRYPTO_BASE + 0x310 + (0x4 * n))
#define CRYPTO_AUTH_INFO_NONCEn(n)	(CRYPTO_BASE + 0x350 + (0x4 * n))
#define CRYPTO_CONFIG			(CRYPTO_BASE + 0x400)
#define CRYPTO_ENCR_KEYn(n)		(CRYPTO_BASE + 0x3000 + (0x4 * n))
#define CRYPTO_AUTH_KEYn(n)		(CRYPTO_BASE + 0x3040 + (0x4 * n))
#define CRYPTO_ENCR_PIPEm_KEYn(m, n)	(CRYPTO_BASE + 0x4000 + (0x20 * m) + (0x4 * n))
#define CRYPTO_AUTH_PIPEm_KEYn(m, n)	(CRYPTO_BASE + 0x4800 + (0x80 * m) + (0x4 * n))
#define CRYPTO_STATUS			(CRYPTO_BASE + 0x100)
#define CRYPTO_STATUS2			(CRYPTO_BASE + 0x104)
#define CRYPTO_DEBUG_ENABLE		(CRYPTO_BASE + 0x5000)
#define CRYPTO_DEBUG_STATUS		(CRYPTO_BASE + 0x5004)

#define CRYPTO_BAM_P_EVNT_REG(n)	(0x1818 + (0x1000 * n))
#define CRYPTO_BAM_P_DESC_FIFO_ADDR(n)	(0x181c + (0x1000 * n))
#define CRYPTO_BAM_P_FIFO_SIZES(n)	(0x1820 + (0x1000 * n))
#define CRYPTO_BAM_P_SW_OFSTS(n)	(0x1800 + (0x1000 * n))
#define CRYPTO_BAM_P_CTRL(n)            (0x1000 + (0x1000 * n))
#define CRYPTO_BAM_P_RST(n)             (0x1004 + (0x1000 * n))

#define CRYPTO_BAM_CTRL			0x0
#define CRYPTO_BAM_DESC_CNT_TRSHLD	0x8
#define CRYPTO_BAM_CNFG_BITS		0x7c

#define CRYPTO_BAM_CTRL_SW_RST		(0x1 << 0)
#define CRYPTO_BAM_CTRL_BAM_EN		(0x1 << 1)

/**
 * H/W specific information
 */
#define NSS_CRYPTO_CIPHER_IV_REGS	4 /**< cipher IV regs*/
#define NSS_CRYPTO_AUTH_IV_REGS		8 /**< auth IV regs*/

#define NSS_CRYPTO_CKEY_REGS		8 /**< cipher key regs*/
#define NSS_CRYPTO_AKEY_REGS		8 /**< auth key regs*/

#define NSS_CRYPTO_BCNT_REGS		2

#define NSS_CRYPTO_CKEY_SZ		(NSS_CRYPTO_CKEY_REGS * sizeof(uint32_t))
#define NSS_CRYPTO_AKEY_SZ		(NSS_CRYPTO_AKEY_REGS * sizeof(uint32_t))

#define NSS_CRYPTO_RESULTS_SZ		sizeof(struct nss_crypto_res_dump)
#define NSS_CRYPTO_INDESC_SZ		sizeof(struct nss_crypto_in_trans)
#define NSS_CRYPTO_OUTDESC_SZ		sizeof(struct nss_crypto_out_trans)
#define NSS_CRYPTO_BAM_DESC_SZ		sizeof(struct nss_crypto_bam_desc)
#define NSS_CRYPTO_BAM_CMD_SZ		sizeof(struct nss_crypto_bam_cmd)
#define NSS_CRYPTO_DESC_SZ		sizeof(struct nss_crypto_desc)
#define NSS_CRYPTO_DESC_ALIGN		8
#define NSS_CRYPTO_CACHE_CMD_SZ		offsetof(struct nss_crypto_cmd_config, keys)
#define NSS_CRYPTO_UNCACHE_CMD_SZ	sizeof(struct nss_crypto_cmd_config)
#define NSS_CRYPTO_CMD_REQ_SZ		offsetof(struct nss_crypto_cmd_request, unlock)
#define NSS_CRYPTO_CMD_UNLOCK_SZ	NSS_CRYPTO_BAM_CMD_SZ

#define NSS_CRYPTO_NUM_INDESC		(NSS_CRYPTO_INDESC_SZ / NSS_CRYPTO_BAM_DESC_SZ)
#define NSS_CRYPTO_NUM_OUTDESC		(NSS_CRYPTO_OUTDESC_SZ / NSS_CRYPTO_BAM_DESC_SZ)
#define NSS_CRYPTO_NUM_AUTH_IV		16
#define NSS_CRYPTO_NUM_AUTH_BYTECNT	4
#define NSS_CRYPTO_NUM_ENCR_CTR		4
#define NSS_CRYPTO_NUM_STATUS		2

#define NSS_CRYPTO_INPIPE(_pipe)	((_pipe) - ((_pipe) & 0x1))
#define NSS_CRYPTO_OUTPIPE(_pipe)	(NSS_CRYPTO_INPIPE((_pipe)) + 1)


/**
 * @brief BAM pipe id constants for different pipe pairs as defined
 * by the H/W specification 'NSS_CRYPTO_BAM_{IN/OUT}PIPE_<pipe pair index>'
 */
enum nss_crypto_bam_pipe_id {
	NSS_CRYPTO_BAM_INPIPE_0 = 0,
	NSS_CRYPTO_BAM_OUTPIPE_0 = 1,
	NSS_CRYPTO_BAM_INPIPE_1 = 2,
	NSS_CRYPTO_BAM_OUTPIPE_1 = 3,
	NSS_CRYPTO_BAM_INPIPE_2 = 4,
	NSS_CRYPTO_BAM_OUTPIPE_2 = 5,
	NSS_CRYPTO_BAM_INPIPE_3 = 6,
	NSS_CRYPTO_BAM_OUTPIPE_3 = 7
};

/**
 * @brief crypto BAM descriptor
 */
struct nss_crypto_bam_desc {
	uint32_t data_start;	/**< Start of Data*/
	uint16_t data_len;	/**< Length of Data*/
	uint16_t flags;		/**< Status Flags*/
};

/**
 * @brief crypto BAM cmd descriptor
 */
struct nss_crypto_bam_cmd {
	uint32_t addr;		/**< Address to access*/
	uint32_t value;		/**< Addr (for read) or Data (for write)*/
	uint32_t mask;		/**< mask to identify valid bits*/
	uint32_t reserve;	/**< reserved*/
};

/**
 * @brief keys command block, both encryption and authentication keys
 * 	  are kept for uncached mode
 */
struct nss_crypto_cmd_keys {
	struct nss_crypto_bam_cmd encr[NSS_CRYPTO_CKEY_REGS];	/**< encryption keys */
	struct nss_crypto_bam_cmd auth[NSS_CRYPTO_AKEY_REGS];	/**< authentication keys */
};

/**
 * @brief common configuration command block
 */
struct nss_crypto_cmd_config {
	struct nss_crypto_bam_cmd config_0;				/**< config */

	struct nss_crypto_bam_cmd encr_seg_cfg;				/**< encryption config */
	struct nss_crypto_bam_cmd auth_seg_cfg;				/**< authentication config */

	struct nss_crypto_bam_cmd encr_seg_start;			/**< encryption start offset */
	struct nss_crypto_bam_cmd auth_seg_start;			/**< authentication start offset */

	struct nss_crypto_bam_cmd encr_ctr_msk;				/**< encryption counter mask */
	struct nss_crypto_bam_cmd auth_iv[NSS_CRYPTO_AUTH_IV_REGS];	/**< authentication IVs */

	struct nss_crypto_cmd_keys keys;				/**< cipher & auth keys for uncached */
};

/**
 * @brief per request configuration command block
 */
struct nss_crypto_cmd_request {
	struct nss_crypto_bam_cmd seg_size;				/**< total segment size */
	struct nss_crypto_bam_cmd encr_seg_size;			/**< encryption size */
	struct nss_crypto_bam_cmd auth_seg_size;			/**< authentication size */

	struct nss_crypto_bam_cmd encr_iv[NSS_CRYPTO_CIPHER_IV_REGS];	/**< encryption IVs */

	struct nss_crypto_bam_cmd config_1;				/**< config, used to switch into little endian */
	struct nss_crypto_bam_cmd go_proc;				/**< crypto trigger, marking config loaded */
	struct nss_crypto_bam_cmd unlock;				/**< dummy write for unlock*/
};

/**
 * @brief crypto command block structure, there is '1' instance of common configuration as it
 * 	  doesn't change per request and there is 'n' request command blocks that change on
 * 	  each request
 */
struct nss_crypto_cmd_block {
	struct nss_crypto_cmd_config cfg;
	struct nss_crypto_cmd_request req[NSS_CRYPTO_MAX_QDEPTH];
};


/**
 * @brief results dump format, generated at the end of the operation
 * 	  as an optimization we place the results dump at the end of
 * 	  the packet. The IV contains the generated hash of the operation
 */
struct nss_crypto_res_dump {
	uint32_t auth_iv[NSS_CRYPTO_NUM_AUTH_IV];
	uint32_t byte_cnt[NSS_CRYPTO_NUM_AUTH_BYTECNT];
	uint32_t encr_ctr[NSS_CRYPTO_NUM_ENCR_CTR];
	uint32_t status[NSS_CRYPTO_NUM_STATUS];
	uint8_t burst_pad[24]; 				/**< XXX:pad based upon the burst size*/
};

/**
 * @brief input transaction set
 */
struct nss_crypto_in_trans {
	struct nss_crypto_bam_desc cmd0_lock;	/**< main cmds & lock pipe*/
	struct nss_crypto_bam_desc cmd1;	/**< secondary commands */
	struct nss_crypto_bam_desc data;	/**< data*/
	struct nss_crypto_bam_desc cmd2_unlock;	/**< unlock pipe */
};

/**
 * @brief output transaction set
 */
struct nss_crypto_out_trans {
	struct nss_crypto_bam_desc data;
	struct nss_crypto_bam_desc results;
};

/**
 * @brief set of descriptors, this gets allocated in one chunk
 *
 * @note  future implementation will decouple all this into separate memory
 *        allocations done from various memories (like DDR, NSS_TCM, Krait L2)
 */
struct nss_crypto_desc {
	struct nss_crypto_in_trans in[NSS_CRYPTO_MAX_QDEPTH];
	struct nss_crypto_out_trans out[NSS_CRYPTO_MAX_QDEPTH];
};

#define nss_crypto_idx_to_inpipe(_idx)		((_idx) << 1)
#define nss_crypto_idx_to_outpipe(_idx)		(((_idx) << 1) + 1)
#define nss_crypto_inpipe_to_idx(_in)		((_in) >> 1)

#endif /* __NSS_CRYPTO_HW_H*/
