/* Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
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
 * nss_cryptoapi.c
 * 	Interface to communicate Native Linux crypto framework specific data
 * 	to Crypto core specific data
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
#include <linux/debugfs.h>

#include <crypto/ctr.h>
#include <crypto/des.h>
#include <crypto/aes.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/scatterwalk.h>

#include <nss_api_if.h>
#include <nss_crypto_if.h>
#include <nss_cfi_if.h>
#include "nss_cryptoapi_private.h"

/*
 * nss_cryptoapi_debugs_add_stats()
 * 	Creates debugfs entries for common statistics
 */
void nss_cryptoapi_debugfs_add_stats(struct dentry *parent, struct nss_cryptoapi_ctx *session_ctx)
{
	pr_info("add stats");
	debugfs_create_u64("queued", S_IRUGO, parent, &session_ctx->queued);
	debugfs_create_u64("completed", S_IRUGO, parent, &session_ctx->completed);
	debugfs_create_u64("queue_failed", S_IRUGO, parent, &session_ctx->queue_failed);
}

/*
 * nss_cryptoapi_debugfs_add_session()
 * 	Creates per session debugfs entries
 */
void nss_cryptoapi_debugfs_add_session(struct nss_cryptoapi *gbl_ctx, struct nss_cryptoapi_ctx *session_ctx)
{
	char buf[NSS_CRYPTOAPI_DEBUGFS_NAME_SZ];

	if (gbl_ctx->root_dentry == NULL) {
		nss_cfi_err("root directories are not present: unable to add session data\n");
		return;
	}

	memset(buf, 0, NSS_CRYPTOAPI_DEBUGFS_NAME_SZ);
	scnprintf(buf, sizeof(buf), "session%d", session_ctx->sid);

	session_ctx->session_dentry = debugfs_create_dir(buf, gbl_ctx->stats_dentry);
	if (session_ctx->session_dentry == NULL) {
		nss_cfi_err("Unable to create qca-nss-cryptoapi/stats/%s directory in debugfs", buf);
		return;
	}

	/*
	 * create session's stats files
	 */
	nss_cryptoapi_debugfs_add_stats(session_ctx->session_dentry, session_ctx);
}

/*
 * nss_cryptoapi_debugfs_del_session()
 * 	deletes per session debugfs entries
 */
void nss_cryptoapi_debugfs_del_session(struct nss_cryptoapi_ctx *session_ctx)
{
	if (session_ctx->session_dentry == NULL)  {
		nss_cfi_err("Unable to find the directory\n");
		return;
	}

	debugfs_remove_recursive(session_ctx->session_dentry);
}

/*
 * nss_cryptoapi_debugfs_init()
 * 	initiallize the cryptoapi debugfs interface
 */
void nss_cryptoapi_debugfs_init(struct nss_cryptoapi *gbl_ctx)
{
	gbl_ctx->root_dentry = debugfs_create_dir("qca-nss-cryptoapi", NULL);
	if (!gbl_ctx->root_dentry) {
		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with other initialization
		 */
		nss_cfi_err("Unable to create directory qca-nss-cryptoapi in debugfs\n");
		return;
	}

	gbl_ctx->stats_dentry = debugfs_create_dir("stats", gbl_ctx->root_dentry);
	if (!gbl_ctx->stats_dentry) {
		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with other initialization
		 */
		nss_cfi_err("Unable to create directory qca-nss-cryptoapi/stats in debugfs\n");
		debugfs_remove_recursive(gbl_ctx->root_dentry);
		return;
	}
}

/*
 * nss_cryptoapi_debugfs_exit()
 * 	cleanup the cryptoapi debugfs interface
 */
void nss_cryptoapi_debugfs_exit(struct nss_cryptoapi *gbl_ctx)
{
	if (!gbl_ctx->root_dentry) {
		nss_cfi_err("Unable to find root directory qca-nss-cryptoapi in debugfs\n");
		return;
	}

	debugfs_remove_recursive(gbl_ctx->root_dentry);
}
