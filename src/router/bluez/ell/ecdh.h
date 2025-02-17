/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_ECDH_H
#define __ELL_ECDH_H

#ifdef __cplusplus
extern "C" {
#endif

struct l_ecc_curve;
struct l_ecc_point;
struct l_ecc_scalar;

/*
 * Generate a private/public key pair. private/public are out parameters and
 * must be freed.
 */
bool l_ecdh_generate_key_pair(const struct l_ecc_curve *curve,
					struct l_ecc_scalar **out_private,
					struct l_ecc_point **out_public);
/*
 * Generate a shared secret from a private/public key. secret is an out
 * parameters and must be freed.
 */
bool l_ecdh_generate_shared_secret(const struct l_ecc_scalar *private_key,
				const struct l_ecc_point *other_public,
				struct l_ecc_scalar **secret);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_ECDH_H */
