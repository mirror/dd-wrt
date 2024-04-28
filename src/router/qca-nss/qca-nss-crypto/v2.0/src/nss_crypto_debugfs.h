/* Copyright (c) 2014, 2017-2018, The Linux Foundation. All rights reserved.
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

/*
 * nss_crypto_debugfs_entry
 *	Engine debugfs entry
 */
struct nss_crypto_debugfs_entry {
	char *name;				/* name of debugfs file */
	struct file_operations ops;		/* file operation pointer */
};

extern void nss_crypto_debugfs_add_control(struct nss_crypto_ctrl *ctrl);
extern void nss_crypto_debugfs_add_node(struct nss_crypto_node *node, struct dentry *root, const char *name);
extern void nss_crypto_debugfs_add_engine(struct nss_crypto_engine *eng, struct dentry *root);
extern void nss_crypto_debugfs_del_engine(struct nss_crypto_engine *eng);
extern void nss_crypto_debugfs_add_ctx(struct nss_crypto_ctx *ctx, struct dentry *root);
extern void nss_crypto_debugfs_del_ctx(struct nss_crypto_ctx *ctx);

