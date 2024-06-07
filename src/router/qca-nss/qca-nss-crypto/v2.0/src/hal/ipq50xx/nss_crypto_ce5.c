/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 */

#include "nss_crypto_hlos.h"
#include <nss_api_if.h>
#include <nss_crypto_defines.h>
#include <nss_crypto_api.h>
#include <nss_crypto_hdr.h>

#include "nss_crypto_ctrl.h"
#include "nss_crypto_debugfs.h"
#include "nss_crypto_ce5.h"

/*
 * nss_crypto_ce5_ctx_ctrl
 *	ce5 context control structure
 */
struct nss_crypto_ce5_ctx_ctrl {
	uint32_t ctrl[NSS_CRYPTO_CE5_MAX_CTRL];
};

/*
 * g_ctx_info
 *	nss_crypto_ce5_ctx_ctrl type member
 *	contains the hw specific register values for NSS Firmware
 *	ctrl[0] = config_0
 *	ctrl[1] = encr_seg_cfg
 *	ctrl[2] = auth_seg_cfg
 */
static struct nss_crypto_ce5_ctx_ctrl g_ctx_info[NSS_CRYPTO_CMN_ALGO_MAX] = {
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES128 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES128 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CTR,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CTR,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_ECB] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES128 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_ECB,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_ECB] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_ECB,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_TRIPLE_DES | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_DES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = 0,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA160_HASH] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = 0,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST | NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA224_HASH] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = 0,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST | NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA256_HASH] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = 0,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST | NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA160_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = 0,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA224_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = 0,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_SHA256_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = 0,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA160_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES128 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA160_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA160_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES128 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CTR,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA160_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CTR,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA160_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_TRIPLE_DES | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_DES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA1 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_3DES_CBC_SHA256_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_TRIPLE_DES | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_DES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CBC_SHA256_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES128 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CBC_SHA256_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CBC,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES128_CTR_SHA256_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CTR,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
	[NSS_CRYPTO_CMN_ALGO_AES256_CTR_SHA256_HMAC] = {
		.ctrl[0] = NSS_CRYPTO_CE5_CONFIG_DOP_INTR | NSS_CRYPTO_CE5_CONFIG_DIN_INTR |
			NSS_CRYPTO_CE5_CONFIG_DOUT_INTR | NSS_CRYPTO_CE5_CONFIG_REQ_SIZE(NSS_CRYPTO_CE5_MAX_BEATS),
		.ctrl[1] = NSS_CRYPTO_CE5_ENCR_SEG_CFG_KEY_AES256 | NSS_CRYPTO_CE5_ENCR_SEG_CFG_ALG_AES |
			NSS_CRYPTO_CE5_ENCR_SEG_CFG_MODE_CTR,
		.ctrl[2] = NSS_CRYPTO_CE5_AUTH_SEG_CFG_ALG_SHA | NSS_CRYPTO_CE5_AUTH_SEG_CFG_SIZE_SHA2 |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_MODE_HMAC | NSS_CRYPTO_CE5_AUTH_SEG_CFG_FIRST |
			NSS_CRYPTO_CE5_AUTH_SEG_CFG_LAST,
	},
};

/*
 * nss_crypto_ce5_ctx_fill()
 *	Fill context record specific information
 */
int nss_crypto_ce5_ctx_fill(struct nss_crypto_ctx *ctx, struct nss_crypto_session_data *data,
				struct nss_crypto_cmn_ctx *msg)
{
	struct nss_crypto_ce5_ctx_ctrl *ctx_ctrl;
	int i;
	uint32_t min_size;

	if (data->algo > NSS_CRYPTO_CMN_ALGO_MAX)
		return -EINVAL;

	/*
	 * Fill context control words
	 */
	ctx->hw_info = ctx_ctrl = &g_ctx_info[data->algo];

	/*
	 * Find the minimum between the size of crypto context control
	 * words and message spare words.
	 */
	min_size = min(ARRAY_SIZE(ctx_ctrl->ctrl), ARRAY_SIZE(msg->spare));

	/*
	 * Fill the spare words with config0, encr_seg_cfg & auth_seg_cfg and mark the availability in flags
	 */
	for (i = 0; i < min_size; i++) {
		msg->spare[i] = ctx_ctrl->ctrl[i];
		msg->flags |= (NSS_CRYPTO_CMN_CTX_FLAGS_SPARE0 << i);
	}

	return 0;
}

/*
 * nss_crypto_ce5_engine_init()
 *	allocate & initialize engine
 */
int nss_crypto_ce5_engine_init(struct platform_device *pdev, struct resource *crypto_res,
		struct resource *bam_res, uint8_t eng_id)
{
	struct nss_crypto_node *node = platform_get_drvdata(pdev);
	struct nss_crypto_engine *eng;
	int status;

	/*
	 * allocate the engine class
	 */
	eng = nss_crypto_engine_alloc(pdev);
	if (!eng) {
		nss_crypto_warn("%px: unable to allocate engine\n", node);
		return -ENOMEM;
	}

	/*
	 * remap the I/O addresses for crypto
	 */
	eng->crypto_paddr = crypto_res->start;
	eng->crypto_vaddr = ioremap(crypto_res->start, resource_size(crypto_res));
	if (!eng->crypto_vaddr) {
		nss_crypto_warn("%px: unable to remap crypto_addr(0x%px)\n", node, (void *)eng->crypto_paddr);
		nss_crypto_engine_free(eng);
		return -EIO;
	}

	/*
	 * remap the I/O addresses for bam
	 */
	eng->dma_paddr = bam_res->start;
	eng->dma_vaddr = ioremap(bam_res->start, resource_size(bam_res));
	if (!eng->dma_vaddr) {
		iounmap(eng->crypto_vaddr);
		nss_crypto_warn("%px: unable to remap dma_addr(0x%px)\n", node, (void *)eng->dma_paddr);
		nss_crypto_engine_free(eng);
		return -EIO;
	}

	eng->node = node;
	eng->id = eng_id;

	status = nss_crypto_engine_init(node, eng);
	if (status < 0) {
		iounmap(eng->dma_vaddr);
		iounmap(eng->crypto_vaddr);
		nss_crypto_warn("%px: unable to initialize engine(%d),status(%d)", node, eng->id, status);
		nss_crypto_engine_free(eng);
		return status;
	}

	return 0;
}

/*
 * nss_crypto_ce5_node_init()
 *	allocate & initialize ce5 node
 */
int nss_crypto_ce5_node_init(struct platform_device *pdev, const char *name)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	struct nss_crypto_node *node;
	int status;

	/*
	 * allocate the CE5 node
	 */
	node = nss_crypto_node_alloc(pdev, NSS_CRYPTO_CMN_INTERFACE, name);
	if (!node) {
		nss_crypto_warn("%px: unable to allocate %s node\n", np, name);
		return -ENOMEM;
	}

	node->fill_ctx = nss_crypto_ce5_ctx_fill;

	status = nss_crypto_node_init(node, np);
	if (status < 0) {
		nss_crypto_warn("%px: unable to initialize the node, status(%d)\n", np, status);
		nss_crypto_node_free(node);
		return status;
	}

	platform_set_drvdata(pdev, node);
	return 0;
}

/*
 * nss_crypto_ce5_init()
 *	performs HW initialization, FW boot and DMA initialization
 */
int nss_crypto_ce5_init(struct platform_device *pdev)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	struct resource crypto_res = {0};
	struct resource bam_res = {0};
	struct device_node *child;
	int eng_id = 0;
	int status;

	/*
	 * TODO: Vote the CE5 crypto clocks for keeping them enabled
	 */

	/*
	 * initialize the CE5 node
	 */
	status = nss_crypto_ce5_node_init(pdev, "ce5v5");
	if (status < 0) {
		nss_crypto_warn("%px: unable to initialize node, status(%d)\n", pdev, status);
		return status;
	}

	/*
	 * Crypto Registers resource
	 */
	if (of_address_to_resource(np, 0, &crypto_res) != 0) {
		nss_crypto_warn("%px: unable to read crypto resource in %s\n", np, np->full_name);
		return -EINVAL;
	}

	/*
	 * BAM Registers resource
	 */
	if (of_address_to_resource(np, 1, &bam_res) != 0) {
		nss_crypto_warn("%px: unable to read bam resource in %s\n", np, np->full_name);
		return -EINVAL;
	}

	/*
	 * initialize each engine of CE5
	 */
	for_each_child_of_node(np, child) {
		status = nss_crypto_ce5_engine_init(pdev, &crypto_res, &bam_res, eng_id);
		if (status < 0) {
			nss_crypto_warn("%px: unable to initialize engine, status(%d)\n", child, status);
			break;
		}

		eng_id++;
	}

	return status;
}
