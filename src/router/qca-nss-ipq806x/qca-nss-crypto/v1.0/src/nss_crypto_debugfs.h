/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 * @brief initiallize the crypto debugfs interface
 *
 * @param ctrl[IN] pointer to crypto control structure
 */
void nss_crypto_debugfs_init(struct nss_crypto_ctrl *ctrl);

/**
 * @brief creates per engine debugfs entries
 *
 * @param ctrl[IN] pointer to crypto control structure
 * @param engine_id[IN] engine id
 */
void nss_crypto_debugfs_add_engine(struct nss_crypto_ctrl *ctrl, uint32_t engine_num);

/**
 * @brief creates per session debugfs entries
 *
 * @param ctrl[IN] pointer to crypto control structure
 * @param session_idx[IN] session index.
 */
void nss_crypto_debugfs_add_session(struct nss_crypto_ctrl *ctrl, uint32_t idx);

/**
 * @brief deletes per session debugfs entries
 *
 * @param ctrl[IN] pointer to crypto control structure
 * @param session_idx[IN] session index.
 */
void nss_crypto_debugfs_del_session(struct nss_crypto_ctrl *ctrl, uint32_t idx);

