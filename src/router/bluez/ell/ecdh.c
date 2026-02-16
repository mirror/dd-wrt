/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

#include "private.h"
#include "ecc-private.h"
#include "ecc.h"
#include "ecdh.h"
#include "random.h"
#include "useful.h"

/*
 * Some sane maximum for calculating the public key.
 */
#define ECDH_MAX_ITERATIONS 20

/*
 * IETF draft-jivsov-ecc-compact-00 Section 4.2.1
 *
 * The following algorithm calculates a key pair {k, Q=k*G=(x,y)}, where k is
 * the private key and Q=(x,y) is the public key.
 *
 * Black box generation:
 *     1. Generate a key pair {k, Q=k*G=(x,y)} with KG
 *     2. if( y != min(y,p-y) ) goto step 1
 *     3. output {k, Q=(x,y)} as a key pair
 */
LIB_EXPORT bool l_ecdh_generate_key_pair(const struct l_ecc_curve *curve,
					struct l_ecc_scalar **out_private,
					struct l_ecc_point **out_public)
{
	bool compliant = false;
	int iter = 0;
	uint64_t p2[L_ECC_MAX_DIGITS];

	if (unlikely(!curve || !out_private || !out_public))
		return false;

	_ecc_calculate_p2(curve, p2);

	*out_public = l_ecc_point_new(curve);

	while (!compliant && iter++ < ECDH_MAX_ITERATIONS) {
		/* aborts if no valid private key can be generated */
		*out_private = l_ecc_scalar_new_random(curve);

		_ecc_point_mult(*out_public, &curve->g, (*out_private)->c,
					NULL, curve->p);

		/* ensure public key is compliant */
		if (_vli_cmp((*out_public)->y, p2, curve->ndigits) >= 0) {
			compliant = true;
			break;
		}

		l_ecc_scalar_free(*out_private);
		*out_private = NULL;
	}

	if (!compliant) {
		l_ecc_point_free(*out_public);
		*out_public = NULL;
		return false;
	}

	return true;
}

LIB_EXPORT bool l_ecdh_generate_shared_secret(
				const struct l_ecc_scalar *private_key,
				const struct l_ecc_point *other_public,
				struct l_ecc_scalar **secret)
{
	const struct l_ecc_curve *curve = private_key->curve;
	struct l_ecc_scalar *z;
	struct l_ecc_point *product;

	if (unlikely(!private_key || !other_public || !secret))
		return false;

	/* aborts if no valid scalar can be generated */
	z = l_ecc_scalar_new_random(curve);

	product = l_ecc_point_new(curve);

	_ecc_point_mult(product, other_public, private_key->c, z->c, curve->p);

	*secret = _ecc_constant_new(curve, product->x, curve->ndigits * 8);

	l_ecc_point_free(product);
	l_ecc_scalar_free(z);

	return true;
}
