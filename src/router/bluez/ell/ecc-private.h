/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdbool.h>
#include <stdint.h>

#include "ecc.h"
#include "util.h"

struct l_ecc_curve;

struct l_ecc_point {
	uint64_t x[L_ECC_MAX_DIGITS];
	uint64_t y[L_ECC_MAX_DIGITS];
	const struct l_ecc_curve *curve;
};

struct l_ecc_curve {
	unsigned int ndigits;
	unsigned int ike_group;
	unsigned int tls_group;
	const char *name;
	struct l_ecc_point g;
	uint64_t p[L_ECC_MAX_DIGITS];
	uint64_t n[L_ECC_MAX_DIGITS];
	uint64_t b[L_ECC_MAX_DIGITS];
	int z;
};

struct l_ecc_scalar {
	uint64_t c[L_ECC_MAX_DIGITS];
	const struct l_ecc_curve *curve;
};

/*
 * Performs a secure memory comparison of two uint64_t buffers of size bytes
 * representing an integer. Blobs are ordered in little endian. It returns
 * a negative, zero or positif value if a < b, a == b or a > b respectively.
 */
static inline int secure_memcmp_64(const uint64_t *a, const uint64_t *b,
					size_t size)
{
	uint64_t aa_64, bb_64;

	int res = 0, mask;

	size_t i = 0;

	if (size) {
		/*
		 * Arrays store blobs in LE, we will process each blob as a
		 * byte array of size 8 using l_secure_memcmp. We need to make
		 * sure to feed a BE byte array to avoid unexpected behavior
		 * on different architectures.
		 */
		do {
			aa_64 = L_CPU_TO_BE64(a[i]);
			bb_64 = L_CPU_TO_BE64(b[i]);
			mask = l_secure_memcmp(&aa_64, &bb_64, 8);
			res = (mask & res) | mask;
			i++;
		} while (i != size);
	}

	return res;
}

void _ecc_be2native(uint64_t *dest, const uint64_t *bytes,
							unsigned int ndigits);

void _ecc_native2be(uint64_t *dest, const uint64_t *native,
							unsigned int ndigits);

void _vli_mod_inv(uint64_t *result, const uint64_t *input, const uint64_t *mod,
			unsigned int ndigits);

void _vli_mod_sub(uint64_t *result, const uint64_t *left, const uint64_t *right,
		const uint64_t *mod, unsigned int ndigits);

void _vli_mod_add(uint64_t *result, const uint64_t *left, const uint64_t *right,
			const uint64_t *mod, unsigned int ndigits);

void _vli_rshift1(uint64_t *vli, unsigned int ndigits);

void _vli_mmod_slow(uint64_t *result, const uint64_t *product,
				const uint64_t *mod, unsigned int ndigits);

bool _vli_mmod_fast(uint64_t *result, const uint64_t *product,
			const uint64_t *curve_prime, unsigned int ndigits);

void _vli_mod_mult_fast(uint64_t *result, const uint64_t *left,
		const uint64_t *right, const uint64_t *curve_prime,
		unsigned int ndigits);
void _vli_mod_square_fast(uint64_t *result, const uint64_t *left,
					const uint64_t *curve_prime,
					unsigned int ndigits);
void _vli_mod_exp(uint64_t *result, const uint64_t *base, const uint64_t *exp,
		const uint64_t *mod, unsigned int ndigits);

int _vli_cmp(const uint64_t *left, const uint64_t *right, unsigned int ndigits);
bool _vli_is_zero_or_one(const uint64_t *vli, unsigned int ndigits);

uint64_t _vli_add(uint64_t *result, const uint64_t *left,
				const uint64_t *right, unsigned int ndigits);
uint64_t _vli_sub(uint64_t *result, const uint64_t *left,
				const uint64_t *right, unsigned int ndigits);

int _vli_legendre(uint64_t *val, const uint64_t *p, unsigned int ndigits);

bool _ecc_point_is_zero(const struct l_ecc_point *point);

void _ecc_calculate_p2(const struct l_ecc_curve *curve, uint64_t *p2);

bool _ecc_compute_y(const struct l_ecc_curve *curve, uint64_t *y,
							const uint64_t *x);

void _ecc_point_mult(struct l_ecc_point *result,
			const struct l_ecc_point *point, const uint64_t *scalar,
			uint64_t *initial_z, const uint64_t *curve_prime);
void _ecc_point_add(struct l_ecc_point *ret, const struct l_ecc_point *p,
			const struct l_ecc_point *q,
			const uint64_t *curve_prime);
struct l_ecc_scalar *_ecc_constant_new(const struct l_ecc_curve *curve,
						const void *buf, size_t len);
