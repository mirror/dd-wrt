/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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
 */
#ifndef __NSS_CRYPTO_API_H
#define __NSS_CRYPTO_API_H

#include "nss_crypto_defines.h"

struct nss_crypto_hdr;

#define NSS_CRYPTO_SESSION_INDEX(x, max) ((x) & ((max) - 1))

/**
 * nss_crypto_register_user
 *	register user attach/detach routines to the crypto driver
 *
 * @datatype
 * nss_crypto_user_ctx
 *
 * @param[in] ctx user context
 * @param[in] app_data opaque data to passed to caller during attach/detach
 *
 * @return
 * pointer to NSS crypto user.
 */
extern struct nss_crypto_user *nss_crypto_register_user(struct nss_crypto_user_ctx *ctx, void *app_data);

/**
 * nss_crypto_unregister_user
 *	unregister user from the list of crypto device users
 *
 * @datatype
 * nss_crypto_user
 *
 * @param[in] crypto handle of the crypto device
 *
 * @return
 * None.
 */
extern void nss_crypto_unregister_user(struct nss_crypto_user *user);

/**
 * nss_crypto_get_cipher_block_len
 *	helper api to get cipher block len
 *
 * @param[in] session
 *
 * @return
 * uint8_t type.
 */
extern uint16_t nss_crypto_get_cipher_block_len(uint32_t session);

/**
 * nss_crypto_get_iv_len
 *	helper api to get iv len
 *
 * @param[in] session
 *
 * @return
 * int8_t type.
 */
extern uint16_t nss_crypto_get_iv_len(uint32_t session);

/**
 * nss_crypto_session_alloc
 *	allocates a new session for the requested configuration
 *
 * @datatype
 * nss_crypto_user \
 * nss_crypto_session_data
 *
 * @param[in] user crypto user handle
 * @param[in] data session specific data {keys, algo etc.,}
 * @param[out] session number assigned for this configuration
 *
 * @return
 * Linux error codes.
 */
extern int nss_crypto_session_alloc(struct nss_crypto_user *user, struct nss_crypto_session_data *data, uint32_t *session);

/**
 * nss_crypto_session_free
 *	Free an existing session, this flushes all state related to the session
 *	including keys, algorithms
 *
 * @datatype
 * nss_crypto_user
 *
 * @param[in] crypto crypto device handle
 * @param[in] session_idx session index to free
 *
 * @return
 * None.
 *
 * @note When changing/altering the configuration of a session such as new keys,
 *       algorithm etc. the procedure should be to free the older session and
 *       then allocate a newer session with the new parameters
 */
extern void nss_crypto_session_free(struct nss_crypto_user *user, uint32_t session);

/**
 * nss_crypto_transform_payload
 *	submit a transform request to crypto
 *
 * @datatype
 * nss_crypto_user \n
 * nss_crypto_hdr \n
 * nss_crypto_req_callback_t
 *
 * @param[in] crypto crypto device handle
 * @param[in] ch crypto header
 * @param[in] cb callback function to call upon completion
 * @param[in] app_data context of the caller
 *
 * @return
 * refer errno bits for details
 */
extern int nss_crypto_transform_payload(struct nss_crypto_user *user, struct nss_crypto_hdr *ch,
				nss_crypto_req_callback_t cb, void *app_data);
#endif
