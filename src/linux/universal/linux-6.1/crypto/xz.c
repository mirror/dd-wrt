/*
 * Cryptographic API.
 *
 * XZ decompression support.
 *
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <crypto/algapi.h>
#include <linux/xz.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/net.h>

struct xz_comp_ctx {
	struct xz_dec	*decomp_state;
	struct xz_buf	decomp_buf;
};

static int crypto_xz_decomp_init(struct xz_comp_ctx *ctx)
{
	ctx->decomp_state = xz_dec_init(XZ_SINGLE, 0);
	if (!ctx->decomp_state)
		return -ENOMEM;

	return 0;
}

static void crypto_xz_decomp_exit(struct xz_comp_ctx *ctx)
{
	xz_dec_end(ctx->decomp_state);
}

static int crypto_xz_init(struct crypto_tfm *tfm)
{
	struct xz_comp_ctx *ctx = crypto_tfm_ctx(tfm);

	return crypto_xz_decomp_init(ctx);
}

static void crypto_xz_exit(struct crypto_tfm *tfm)
{
	struct xz_comp_ctx *ctx = crypto_tfm_ctx(tfm);

	crypto_xz_decomp_exit(ctx);
}

static int crypto_xz_compress(struct crypto_tfm *tfm, const u8 *src,
			      unsigned int slen, u8 *dst, unsigned int *dlen)
{
	return -EOPNOTSUPP;
}

static int crypto_xz_decompress(struct crypto_tfm *tfm, const u8 *src,
				unsigned int slen, u8 *dst, unsigned int *dlen)
{
	struct xz_comp_ctx *dctx = crypto_tfm_ctx(tfm);
	struct xz_buf *xz_buf = &dctx->decomp_buf;
	int ret;

	memset(xz_buf, '\0', sizeof(struct xz_buf));

	xz_buf->in = (u8 *) src;
	xz_buf->in_pos = 0;
	xz_buf->in_size = slen;
	xz_buf->out = (u8 *) dst;
	xz_buf->out_pos = 0;
	xz_buf->out_size = *dlen;

	ret = xz_dec_run(dctx->decomp_state, xz_buf);
	if (ret != XZ_STREAM_END) {
		ret = -EINVAL;
		goto out;
	}

	*dlen = xz_buf->out_pos;
	ret = 0;

out:
	return ret;
}

static struct crypto_alg crypto_xz_alg = {
	.cra_name		= "xz",
	.cra_flags		= CRYPTO_ALG_TYPE_COMPRESS,
	.cra_ctxsize		= sizeof(struct xz_comp_ctx),
	.cra_module		= THIS_MODULE,
	.cra_list		= LIST_HEAD_INIT(crypto_xz_alg.cra_list),
	.cra_init		= crypto_xz_init,
	.cra_exit		= crypto_xz_exit,
	.cra_u			= { .compress = {
	.coa_compress 		= crypto_xz_compress,
	.coa_decompress  	= crypto_xz_decompress } }
};

static int __init crypto_xz_mod_init(void)
{
	return crypto_register_alg(&crypto_xz_alg);
}

static void __exit crypto_xz_mod_exit(void)
{
	crypto_unregister_alg(&crypto_xz_alg);
}

module_init(crypto_xz_mod_init);
module_exit(crypto_xz_mod_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Crypto XZ decompression support");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
