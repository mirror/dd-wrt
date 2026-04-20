/* Copyright (c) 2014-2017, The Linux Foundation. All rights reserved.
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

#include <nss_api_if.h>
#include <nss_crypto_hlos.h>
#include <nss_crypto_dbg.h>
#include <nss_crypto_if.h>
#include <nss_crypto_hw.h>
#include <nss_crypto_ctrl.h>
#include <nss_crypto_debugfs.h>

#define NSS_CRYPTO_DEBUGFS_NAME_SZ 64

/*
 * nss_crypto_debugs_add_stats()
 * 	Creates debugs entries for common statistics
 */
static void nss_crypto_debugfs_add_stats(struct dentry *parent, struct nss_crypto_stats *stats)
{
	debugfs_create_u32("queued", S_IRUGO, parent, &stats->queued);
	debugfs_create_u32("completed", S_IRUGO, parent, &stats->completed);
	debugfs_create_u32("dropped", S_IRUGO, parent, &stats->dropped);
}

/*
 * nss_crypto_debugfs_add_ctrl_stats()
 * 	Creates debugfs entries for Host maintained control statistics
 */
static void nss_crypto_debugfs_add_ctrl_stats(struct dentry *parent, struct nss_crypto_ctrl_stats *stats)
{
	debugfs_create_atomic_t("session_alloc", S_IRUGO, parent, &stats->session_alloc);
	debugfs_create_atomic_t("session_free", S_IRUGO, parent, &stats->session_free);
	debugfs_create_atomic_t("session_alloc_fail", S_IRUGO, parent, &stats->session_alloc_fail);
}

/*
 * nss_crypto_debugfs_init()
 * 	initiallize the crypto debugfs interface
 */
void nss_crypto_debugfs_init(struct nss_crypto_ctrl *ctrl)
{
	struct dentry *tstats_dentry, *cstats_dentry;

	ctrl->root_dentry = debugfs_create_dir("qca-nss-crypto", NULL);
	if (ctrl->root_dentry == NULL) {

		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with other initialization
		 */
		nss_crypto_info_always("Unable to create directory qca-nss-crypto in debugfs\n");
		return;
	}

	ctrl->stats_dentry = debugfs_create_dir("stats", ctrl->root_dentry);
	if (ctrl->stats_dentry == NULL) {

		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with other initialization
		 */
		nss_crypto_info_always("Unable to create directory qca-nss-crypto/stats in debugfs\n");
		debugfs_remove_recursive(ctrl->root_dentry);
		return;
	}

	ctrl->cfg_dentry = debugfs_create_dir("config", ctrl->root_dentry);
	if (ctrl->cfg_dentry == NULL) {

		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with other initialization
		 */
		nss_crypto_info_always("Unable to create directory qca-nss-crypto/config in debugfs\n");
		debugfs_remove_recursive(ctrl->root_dentry);
		return;
	}

	/*
	 *  Create a debufs entry corresponding total statistics
	 */
	tstats_dentry = debugfs_create_dir("total", ctrl->stats_dentry);
	if (tstats_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/stats/total directory in debugfs");
		return;
	}

	/*
	 * create total stats files
	 */
	nss_crypto_debugfs_add_stats(tstats_dentry, &ctrl->total_stats);

	/*
	 * Create a debugfs entry corresponding to host stats
	 */
	cstats_dentry = debugfs_create_dir("control", ctrl->stats_dentry);
	if (cstats_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/stats/control directory in debugfs");
		return;
	}

	/*
	 * create host stats files
	 */
	nss_crypto_debugfs_add_ctrl_stats(cstats_dentry, &ctrl->ctrl_stats);
}

/*
 * nss_crypto_debugfs_add_engine()
 * 	Creates per engine debugfs entries
 */
void nss_crypto_debugfs_add_engine(struct nss_crypto_ctrl *ctrl, uint32_t engine_num)
{
	char buf[NSS_CRYPTO_DEBUGFS_NAME_SZ];
	struct nss_crypto_ctrl_eng *e_ctrl = &ctrl->eng[engine_num];

	if (ctrl->root_dentry == NULL) {
		nss_crypto_err("root directories are not present: unable to add engine data\n");
		return;
	}

	memset(buf, 0, NSS_CRYPTO_DEBUGFS_NAME_SZ);
	scnprintf(buf, sizeof(buf), "engine%d", engine_num);

	e_ctrl->stats_dentry = debugfs_create_dir(buf, ctrl->stats_dentry);
	if (e_ctrl->stats_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/stats/%s directory in debugfs", buf);
		return;
	}

	/*
	 * create engine stats files
	 */
	nss_crypto_debugfs_add_stats(e_ctrl->stats_dentry, &e_ctrl->stats);
}

/*
 * nss_crypto_debugfs_add_session()
 * 	Creates per session debugfs entries
 */
void nss_crypto_debugfs_add_session(struct nss_crypto_ctrl *ctrl, uint32_t idx)
{
	char buf[NSS_CRYPTO_DEBUGFS_NAME_SZ];
	enum nss_crypto_cipher cipher_algo;
	enum nss_crypto_auth auth_algo;
	struct dentry *temp_dentry;
	struct nss_crypto_idx_info *idx_info = &ctrl->idx_info[idx];

	if (ctrl->root_dentry == NULL) {
		nss_crypto_err("root directories are not present: unable to add session data\n");
		return;
	}

	cipher_algo = nss_crypto_get_cipher(idx);
	auth_algo = nss_crypto_get_auth(idx);

	memset(buf, 0, NSS_CRYPTO_DEBUGFS_NAME_SZ);
	scnprintf(buf, sizeof(buf), "session%d", idx);

	idx_info->stats_dentry = debugfs_create_dir(buf, ctrl->stats_dentry);
	if (idx_info->stats_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/stats/%s directory in debugfs", buf);
		return;
	}

	/*
	 * create sessions stats files
	 */
	nss_crypto_debugfs_add_stats(idx_info->stats_dentry, &idx_info->stats);

	/*
	 * create session cfg info files
	 */
	idx_info->cfg_dentry = debugfs_create_dir(buf, ctrl->cfg_dentry);
	if (idx_info->cfg_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/config/%s directory in debugfs", buf);
		return;
	}

	/*
	 * populate the cipher information
	 */
	temp_dentry = debugfs_create_dir("cipher", idx_info->cfg_dentry);
	if (temp_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/config/%s/cipher directory in debugfs", buf);
		return;
	}

	/*
	 * A file whose name represent the session's cipher algorithm
	 * will be created. Reading the file will return key length
	 * corresponding to algorithm in bytes
	 */
	switch (cipher_algo) {
	case NSS_CRYPTO_CIPHER_NONE:
		debugfs_create_u32("NONE", S_IRUGO, temp_dentry, &idx_info->ckey.key_len);
		break;

	case NSS_CRYPTO_CIPHER_AES_CBC:
		debugfs_create_u32("AES-CBC", S_IRUGO, temp_dentry, &idx_info->ckey.key_len);
		break;

	case NSS_CRYPTO_CIPHER_AES_CTR:
		debugfs_create_u32("AES-CTR", S_IRUGO, temp_dentry, &idx_info->ckey.key_len);
		break;

	case NSS_CRYPTO_CIPHER_DES:
		debugfs_create_u32("DES", S_IRUGO, temp_dentry, &idx_info->ckey.key_len);
		break;

	case NSS_CRYPTO_CIPHER_NULL:
		debugfs_create_u32("NULL", S_IRUGO, temp_dentry, &idx_info->ckey.key_len);
		break;

	default:
		nss_crypto_err("Unknown cipher algorithm:%d\n", cipher_algo);
		break;
	}

	/*
	 * populate the authentication information
	 */
	temp_dentry = debugfs_create_dir("auth", idx_info->cfg_dentry);
	if (temp_dentry == NULL) {
		nss_crypto_err("Unable to create qca-nss-crypto/config/%s/auth directory in debugfs", buf);
		return;
	}

	/*
	 * A file whose name represent the session's authentication algorithm
	 * will be created. Reading the file will return key length
	 * corresponding to algorithm in bytes
	 */
	switch (auth_algo) {
	case NSS_CRYPTO_AUTH_NONE:
		debugfs_create_u32("NONE", S_IRUGO, temp_dentry, &idx_info->akey.key_len);
		break;

	case NSS_CRYPTO_AUTH_SHA1_HMAC:
		debugfs_create_u32("SHA1", S_IRUGO, temp_dentry, &idx_info->akey.key_len);
		break;

	case NSS_CRYPTO_AUTH_SHA256_HMAC:
		debugfs_create_u32("SHA256", S_IRUGO, temp_dentry, &idx_info->akey.key_len);
		break;

	case NSS_CRYPTO_AUTH_NULL:
		debugfs_create_u32("NULL", S_IRUGO, temp_dentry, &idx_info->akey.key_len);
		break;

	default:
		nss_crypto_err("Unknown authentication alogrithm :%d\n", auth_algo);
		break;
	}

}

/*
 * nss_crypto_debugfs_del_delete()
 * 	deletes per session debugfs entries
 */
void nss_crypto_debugfs_del_session(struct nss_crypto_ctrl *ctrl, uint32_t idx)
{
	struct nss_crypto_idx_info *idx_info = &ctrl->idx_info[idx];

	if (idx_info->stats_dentry == NULL)  {
		nss_crypto_err("Unable to find the directory\n");
		return;
	}

	debugfs_remove_recursive(idx_info->stats_dentry);
	idx_info->stats_dentry = NULL;

	if (idx_info->cfg_dentry == NULL)  {
		nss_crypto_err("Unable to find the file\n");
		return;
	}

	debugfs_remove_recursive(idx_info->cfg_dentry);
	idx_info->cfg_dentry = NULL;
}
