/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "ecc.h"
#include "ecc-private.h"
#include "random.h"
#include "useful.h"
#include "private.h"
#include "missing.h"

/*
 * RFC 5114 - Section 2.6 256-bit Random ECP Group
 */
#define P256_CURVE_P { 0xFFFFFFFFFFFFFFFFull, 0x00000000FFFFFFFFull, \
			0x0000000000000000ull, 0xFFFFFFFF00000001ull }
#define P256_CURVE_GX { 0xF4A13945D898C296ull, 0x77037D812DEB33A0ull,   \
			0xF8BCE6E563A440F2ull, 0x6B17D1F2E12C4247ull }
#define P256_CURVE_GY { 0xCBB6406837BF51F5ull, 0x2BCE33576B315ECEull,   \
			0x8EE7EB4A7C0F9E16ull, 0x4FE342E2FE1A7F9Bull }
#define P256_CURVE_N { 0xF3B9CAC2FC632551ull, 0xBCE6FAADA7179E84ull,   \
			0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFF00000000ull }
#define P256_CURVE_B { 0x3BCE3C3E27D2604Bull, 0x651D06B0CC53B0F6ull,   \
			0xB3EBBD55769886BCull, 0x5AC635D8AA3A93E7ull }

static const struct l_ecc_curve p256 = {
	.name = "secp256r1",
	.ike_group = 19,
	.tls_group = 23,
	.ndigits = 4,
	.g = {
		.x = P256_CURVE_GX,
		.y = P256_CURVE_GY,
		.curve = &p256
	},
	.p = P256_CURVE_P,
	.n = P256_CURVE_N,
	.b = P256_CURVE_B,
	.z = -10,
};

/*
 * RFC 5114 - Section 2.7 384-bit Random ECP Group
 */
#define P384_CURVE_P {	0x00000000FFFFFFFFull, 0xFFFFFFFF00000000ull, \
			0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull, \
			0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull }
#define P384_CURVE_GX {	0x3A545E3872760AB7ull, 0x5502F25DBF55296Cull, \
			0x59F741E082542A38ull, 0x6E1D3B628BA79B98ull, \
			0x8EB1C71EF320AD74ull, 0xAA87CA22BE8B0537ull }
#define P384_CURVE_GY {	0x7A431D7C90EA0E5Full, 0x0A60B1CE1D7E819Dull, \
			0xE9DA3113B5F0B8C0ull, 0xF8F41DBD289A147Cull, \
			0x5D9E98BF9292DC29ull, 0x3617DE4A96262C6Full }
#define P384_CURVE_N {	0xECEC196ACCC52973ull, 0x581A0DB248B0A77Aull, \
			0xC7634D81F4372DDFull, 0xFFFFFFFFFFFFFFFFull, \
			0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull }
#define P384_CURVE_B {	0x2A85C8EDD3EC2AEFull, 0xC656398D8A2ED19Dull, \
			0x0314088F5013875Aull, 0x181D9C6EFE814112ull, \
			0x988E056BE3F82D19ull, 0xB3312FA7E23EE7E4ull }

static const struct l_ecc_curve p384 = {
	.name = "secp384r1",
	.ike_group = 20,
	.tls_group = 24,
	.ndigits = 6,
	.g = {
		.x = P384_CURVE_GX,
		.y = P384_CURVE_GY,
		.curve = &p384
	},
	.p = P384_CURVE_P,
	.n = P384_CURVE_N,
	.b = P384_CURVE_B,
	.z = -12,
};

static const struct l_ecc_curve *curves[] = {
	&p384,
	&p256,
};

/* Returns supported IKE groups, sorted by the highest effective key size */
LIB_EXPORT const unsigned int *l_ecc_supported_ike_groups(void)
{
	static unsigned int supported_ike_groups[L_ARRAY_SIZE(curves) + 1];
	static bool ike_first = true;

	if (ike_first) {
		unsigned int i;

		for (i = 0; i < L_ARRAY_SIZE(curves); i++)
			supported_ike_groups[i] = curves[i]->ike_group;

		supported_ike_groups[i] = 0;
		ike_first = false;
	}

	return supported_ike_groups;
}

/* Returns supported TLS groups, sorted by the highest effective key size */
LIB_EXPORT const unsigned int *l_ecc_supported_tls_groups(void)
{
	static unsigned int supported_tls_groups[L_ARRAY_SIZE(curves) + 1];
	static bool tls_first = true;

	if (tls_first) {
		unsigned int i;

		for (i = 0; i < L_ARRAY_SIZE(curves); i++)
			supported_tls_groups[i] = curves[i]->tls_group;

		supported_tls_groups[i] = 0;
		tls_first = false;
	}

	return supported_tls_groups;
}

LIB_EXPORT const struct l_ecc_curve *l_ecc_curve_from_name(const char *name)
{
	int i;

	if (unlikely(!name))
		return NULL;

	for (i = 0; curves[i]; i++) {
		if (!strcmp(curves[i]->name, name))
			return curves[i];
	}

	return NULL;
}

LIB_EXPORT const struct l_ecc_curve *l_ecc_curve_from_ike_group(
							unsigned int group)
{
	unsigned int i;

	for (i = 0; i < L_ARRAY_SIZE(curves); i++) {
		if (curves[i]->ike_group == group)
			return curves[i];
	}

	return NULL;
}

LIB_EXPORT const struct l_ecc_curve *l_ecc_curve_from_tls_group(
							unsigned int group)
{
	unsigned int i;

	for (i = 0; i < L_ARRAY_SIZE(curves); i++) {
		if (curves[i]->tls_group == group)
			return curves[i];
	}

	return NULL;
}

LIB_EXPORT const char *l_ecc_curve_get_name(const struct l_ecc_curve *curve)
{
	if (unlikely(!curve))
		return NULL;

	return curve->name;
}

LIB_EXPORT unsigned int l_ecc_curve_get_ike_group(
					const struct l_ecc_curve *curve)
{
	if (unlikely(!curve))
		return 0;

	return curve->ike_group;
}

LIB_EXPORT unsigned int l_ecc_curve_get_tls_group(
					const struct l_ecc_curve *curve)
{
	if (unlikely(!curve))
		return 0;

	return curve->tls_group;
}

LIB_EXPORT struct l_ecc_scalar *l_ecc_curve_get_order(
						const struct l_ecc_curve *curve)
{
	return _ecc_constant_new(curve, curve->n, curve->ndigits * 8);
}

LIB_EXPORT struct l_ecc_scalar *l_ecc_curve_get_prime(
						const struct l_ecc_curve *curve)
{
	if (unlikely(!curve))
		return NULL;

	return _ecc_constant_new(curve, curve->p, curve->ndigits * 8);
}

LIB_EXPORT size_t l_ecc_curve_get_scalar_bytes(const struct l_ecc_curve *curve)
{
	if (unlikely(!curve))
		return 0;

	return curve->ndigits * 8;
}

static bool ecc_valid_point(struct l_ecc_point *point)
{
	const struct l_ecc_curve *curve = point->curve;
	uint64_t tmp1[L_ECC_MAX_DIGITS];
	uint64_t tmp2[L_ECC_MAX_DIGITS];
	uint64_t _3[L_ECC_MAX_DIGITS] = { 3 };	/* -a = 3 */
	unsigned int ndigits = curve->ndigits;

	/* The point at infinity is invalid. */
	if (_ecc_point_is_zero(point))
		return false;

	/* x and y must be smaller than p. */
	if (_vli_cmp(curve->p, point->x, ndigits) != 1 ||
			_vli_cmp(curve->p, point->y, ndigits) != 1)
		return false;

	/* Computes result = y^2. */
	_vli_mod_square_fast(tmp1, point->y, curve->p, ndigits);

	/* Computes result = x^3 + ax + b. result must not overlap x. */
	/* r = x^2 */
	_vli_mod_square_fast(tmp2, point->x, curve->p, ndigits);
	/* r = x^2 - 3 */
	_vli_mod_sub(tmp2, tmp2, _3, curve->p, ndigits);
	/* r = x^3 - 3x */
	_vli_mod_mult_fast(tmp2, tmp2, point->x, curve->p, ndigits);
	/* r = x^3 - 3x + b */
	_vli_mod_add(tmp2, tmp2, curve->b, curve->p, ndigits);
	/* Make sure that y^2 == x^3 + ax + b */
	return (_vli_cmp(tmp1, tmp2, ndigits) == 0);
}

void _ecc_be2native(uint64_t *dest, const uint64_t *bytes,
							unsigned int ndigits)
{
	unsigned int i;
	uint64_t tmp[2 * L_ECC_MAX_DIGITS];

	for (i = 0; i < ndigits; i++)
		tmp[ndigits - 1 - i] = l_get_be64(&bytes[i]);

	memcpy(dest, tmp, ndigits * 8);
}

void _ecc_native2be(uint64_t *dest, const uint64_t *native,
							unsigned int ndigits)
{
	unsigned int i;
	uint64_t tmp[L_ECC_MAX_DIGITS];

	for (i = 0; i < ndigits; i++)
		l_put_be64(native[ndigits - 1 - i], &tmp[i]);

	memcpy(dest, tmp, ndigits * 8);
}

static void ecc_compute_y_sqr(const struct l_ecc_curve *curve,
					uint64_t *y_sqr, const uint64_t *x)
{
	uint64_t sum[L_ECC_MAX_DIGITS] = { 0 };
	uint64_t tmp[L_ECC_MAX_DIGITS] = { 0 };
	uint64_t _3[L_ECC_MAX_DIGITS] = { 3ull }; /* -a = 3 */

	/* x^3 */
	_vli_mod_square_fast(sum, x, curve->p, curve->ndigits);
	_vli_mod_mult_fast(sum, sum, x, curve->p, curve->ndigits);
	/* x^3 - ax */
	_vli_mod_mult_fast(tmp, _3, x, curve->p, curve->ndigits);
	_vli_mod_sub(sum, sum, tmp, curve->p, curve->ndigits);
	/* x^3 - ax + b */
	_vli_mod_add(sum, sum, curve->b, curve->p, curve->ndigits);

	memcpy(y_sqr, sum, curve->ndigits * 8);
}

/*
 * Compute sqrt(y^2)
 * Since our prime p satisfies p = 3 (mod 4), we can say:
 *
 * y = (y^2)^((p + 1) / 4)
 *
 * This avoids the need for a square root function.
 */
static void ecc_compute_sqrt(const struct l_ecc_curve *curve,
					uint64_t *y, const uint64_t *y_sqr)
{
	uint64_t expo[L_ECC_MAX_DIGITS];
	uint64_t one[L_ECC_MAX_DIGITS] = { 1ull };

	memcpy(expo, curve->p, curve->ndigits * 8);

	/* (p + 1) / 4  == (p >> 2) + 1 */
	_vli_rshift1(expo, curve->ndigits);
	_vli_rshift1(expo, curve->ndigits);
	_vli_mod_add(expo, expo, one, curve->p, curve->ndigits);
	/* sum ^ ((p + 1) / 4) */
	_vli_mod_exp(y, y_sqr, expo, curve->p, curve->ndigits);
}

bool _ecc_compute_y(const struct l_ecc_curve *curve, uint64_t *y,
							const uint64_t *x)
{
	uint64_t sum[L_ECC_MAX_DIGITS] = { 0 };
	uint64_t check[L_ECC_MAX_DIGITS] = { 0 };

	/* y = sqrt(x^3 + ax + b) (mod p) */
	ecc_compute_y_sqr(curve, sum, x);
	ecc_compute_sqrt(curve, y, sum);

	/* square y to ensure we have a correct value */
	_vli_mod_mult_fast(check, y, y, curve->p, curve->ndigits);

	if (_vli_cmp(check, sum, curve->ndigits) != 0)
		return false;

	return true;
}

/*
 * IETF - Compact representation of an elliptic curve point:
 * https://tools.ietf.org/id/draft-jivsov-ecc-compact-00.xml
 *
 * "min(y,p-y) can be calculated with the help of the pre-calculated value
 *  p2=(p-1)/2. min(y,p-y) is y if y<p2 and p-y otherwise."
 */
void _ecc_calculate_p2(const struct l_ecc_curve *curve, uint64_t *p2)
{
	uint64_t one[L_ECC_MAX_DIGITS] = { 1 };

	_vli_mod_sub(p2, curve->p, one, curve->p, curve->ndigits);
	_vli_rshift1(p2, curve->ndigits);
}

/*
 * IETF draft-jivsov-ecc-compact-00 Section 4.1
 * Encoding and decoding of an elliptic curve point
 * ...
 * Decoding:
 * Given the compact representation of Q, return canonical representation
 * of Q=(x,y) as follows:
 *     1. y' = sqrt( x^3 + a*x + b ), where y'>0
 *     2. y = min(y',p-y')
 *     3. Q=(x,y) is the canonical representation of the point
 */
static bool decode_point(const struct l_ecc_curve *curve, uint64_t *x,
				struct l_ecc_point *point)
{
	uint64_t y_min[L_ECC_MAX_DIGITS];
	uint64_t p2[L_ECC_MAX_DIGITS];

	if (!_ecc_compute_y(curve, y_min, (uint64_t *)x))
		return false;

	_ecc_calculate_p2(curve, p2);

	if (_vli_cmp(y_min, p2, curve->ndigits) >= 0)
		_vli_mod_sub(point->y, curve->p, y_min,
					curve->p, curve->ndigits);
	else
		memcpy(point->y, y_min, curve->ndigits * 8);

	memcpy(point->x, x, curve->ndigits * 8);

	return true;
}

/* (rx, ry) = (px, py) + (qx, qy) */
void _ecc_point_add(struct l_ecc_point *ret, const struct l_ecc_point *p,
			const struct l_ecc_point *q,
			const uint64_t *curve_prime)
{
	/*
	* s = (py - qy)/(px - qx)
	*
	* rx = s^2 - px - qx
	* ry = s(px - rx) - py
	*/
	uint64_t s[L_ECC_MAX_DIGITS];
	uint64_t kp1[L_ECC_MAX_DIGITS];
	uint64_t kp2[L_ECC_MAX_DIGITS];
	uint64_t resx[L_ECC_MAX_DIGITS];
	uint64_t resy[L_ECC_MAX_DIGITS];
	unsigned int ndigits = p->curve->ndigits;

	memset(s, 0, ndigits * 8);

	/* kp1 = py - qy */
	_vli_mod_sub(kp1, q->y, p->y, curve_prime, ndigits);
	/* kp2 = px - qx */
	_vli_mod_sub(kp2, q->x, p->x, curve_prime, ndigits);
	/* s = kp1/kp2 */
	_vli_mod_inv(kp2, kp2, curve_prime, ndigits);
	_vli_mod_mult_fast(s, kp1, kp2, curve_prime, ndigits);
	/* rx = s^2 - px - qx */
	_vli_mod_mult_fast(kp1, s, s, curve_prime, ndigits);
	_vli_mod_sub(kp1, kp1, p->x, curve_prime, ndigits);
	_vli_mod_sub(resx, kp1, q->x, curve_prime, ndigits);
	/* ry = s(px - rx) - py */
	_vli_mod_sub(kp1, p->x, resx, curve_prime, ndigits);
	_vli_mod_mult_fast(kp1, s, kp1, curve_prime, ndigits);
	_vli_mod_sub(resy, kp1, p->y, curve_prime, ndigits);

	memcpy(ret->x, resx, ndigits * 8);
	memcpy(ret->y, resy, ndigits * 8);
}

/* result = (base ^ exp) % p */
void _vli_mod_exp(uint64_t *result, const uint64_t *base, const uint64_t *exp,
			const uint64_t *mod, unsigned int ndigits)
{
	unsigned int i;
	int bit;
	uint64_t n[L_ECC_MAX_DIGITS];
	uint64_t r[L_ECC_MAX_DIGITS] = { 1 };

	memcpy(n, base, ndigits * 8);

	for (i = 0; i < ndigits; i++) {
		for (bit = 0; bit < 64; bit++) {
			uint64_t tmp[L_ECC_MAX_DIGITS];

			if (exp[i] & (1ull << bit)) {
				_vli_mod_mult_fast(tmp, r, n, mod, ndigits);
				memcpy(r, tmp, ndigits * 8);
			}

			_vli_mod_mult_fast(tmp, n, n, mod, ndigits);
			memcpy(n, tmp, ndigits * 8);
		}
	}

	memcpy(result, r, ndigits * 8);
}

__attribute__((noinline)) static int vli_equal(const uint64_t *a,
							const uint64_t *b,
							unsigned int ndigits)
{
	uint64_t diff = 0;
	unsigned int i;

	for (i = 0; i < ndigits; i++) {
		diff |= a[i] ^ b[i];
		__asm__ ("" : "=r" (diff) : "0" (diff));
	}

	return (~diff & (diff - 1)) >> 63;
}

int _vli_legendre(uint64_t *val, const uint64_t *p, unsigned int ndigits)
{
	uint64_t tmp[L_ECC_MAX_DIGITS];
	uint64_t exp[L_ECC_MAX_DIGITS];
	uint64_t _1[L_ECC_MAX_DIGITS] = { 1ull };
	uint64_t _0[L_ECC_MAX_DIGITS] = { 0 };

	/* check that val ^ ((p - 1) / 2) == [1, 0 or -1] */

	_vli_sub(exp, p, _1, ndigits);
	_vli_rshift1(exp, ndigits);
	_vli_mod_exp(tmp, val, exp, p, ndigits);

	if (_vli_cmp(tmp, _1, ndigits) == 0)
		return 1;
	if (_vli_cmp(tmp, _0, ndigits) == 0)
		return 0;
	return -1;
}

bool _vli_is_zero_or_one(const uint64_t *vli, unsigned int ndigits)
{
	uint64_t _1[L_ECC_MAX_DIGITS] = { 1ull };
	int ret;

	ret = secure_select(vli_equal(vli, _1, ndigits), true, false);
	ret = secure_select(l_secure_memeq(vli, ndigits * 8, 0), true, ret);

	return ret;
}

LIB_EXPORT struct l_ecc_point *l_ecc_point_new(const struct l_ecc_curve *curve)
{
	struct l_ecc_point *p = l_new(struct l_ecc_point, 1);

	p->curve = curve;

	return p;
}

LIB_EXPORT struct l_ecc_point *l_ecc_point_from_data(
					const struct l_ecc_curve *curve,
					enum l_ecc_point_type type,
					const void *data, size_t len)
{
	struct l_ecc_point *p;
	size_t bytes = curve->ndigits * 8;
	uint64_t tmp[L_ECC_MAX_DIGITS];
	bool sub;

	if (!data)
		return NULL;

	/* Verify the data length matches a full point or X coordinate */
	if (type == L_ECC_POINT_TYPE_FULL) {
		if (len != bytes * 2)
			return NULL;
	} else if (len != bytes)
		return NULL;

	p = l_ecc_point_new(curve);

	_ecc_be2native(p->x, (void *) data, curve->ndigits);

	switch (type) {
	case L_ECC_POINT_TYPE_COMPLIANT:
		if (!decode_point(curve, p->x, p))
			goto failed;

		break;
	case L_ECC_POINT_TYPE_COMPRESSED_BIT0:
	case L_ECC_POINT_TYPE_COMPRESSED_BIT1:
		if (!_ecc_compute_y(curve, p->y, p->x))
			goto failed;

		/*
		 * This is determining whether or not to subtract the Y
		 * coordinate from P. According to ANSI X9.62 an even Y should
		 * be prefixed with 02 (BIT0) and an odd Y should be prefixed
		 * with 03 (BIT1). If this is not the case, subtract Y from P.
		 *
		 * ANSI X9.62
		 * 4.3.6 Point-to-Octet-String Conversion
		 *
		 * 2. If the compressed form is used, then do the following:
		 *     2.1. Compute the bit ~Yp . (See Section 4.2.)
		 *     2.2. Assign the value 02 to the single octet PC if ~Yp
		 *          is 0, or the value 03 if ~Yp is 1.
		 *     2.3. The result is the octet string PO = PC || X
		 */

		sub = secure_select(type == L_ECC_POINT_TYPE_COMPRESSED_BIT0,
					p->y[0] & 1, !(p->y[0] & 1));

		_vli_mod_sub(tmp, curve->p, p->y, curve->p, curve->ndigits);

		l_secure_select(sub, tmp, p->y, p->y, curve->ndigits * 8);

		break;
	case L_ECC_POINT_TYPE_FULL:
		_ecc_be2native(p->y, (void *) data + bytes, curve->ndigits);

		if (!ecc_valid_point(p))
			goto failed;

		break;
	}

	return p;

failed:
	l_free(p);
	return NULL;
}

LIB_EXPORT struct l_ecc_point *l_ecc_point_from_sswu(
						const struct l_ecc_scalar *u)
{
	const struct l_ecc_curve *curve = u->curve;
	unsigned int ndigits = curve->ndigits;
	uint64_t z[L_ECC_MAX_DIGITS] = { abs(curve->z) };
	uint64_t _3[L_ECC_MAX_DIGITS] = { 3ull }; /* -a = 3 */
	uint64_t u2z[L_ECC_MAX_DIGITS];
	uint64_t t1[L_ECC_MAX_DIGITS];
	uint64_t t2[L_ECC_MAX_DIGITS];
	uint64_t m[L_ECC_MAX_DIGITS];
	uint64_t t[L_ECC_MAX_DIGITS];
	uint64_t x1l[L_ECC_MAX_DIGITS];
	uint64_t x1r[L_ECC_MAX_DIGITS];
	uint64_t x1[L_ECC_MAX_DIGITS];
	uint64_t gx1[L_ECC_MAX_DIGITS];
	uint64_t x2[L_ECC_MAX_DIGITS];
	uint64_t gx2[L_ECC_MAX_DIGITS];
	/* reuse m/t/x1l,x1r, they are unused by the time x/v/y/p-y is needed */
	uint64_t *x = m;
	uint64_t *v = t;
	uint64_t *yl = x1l;
	uint64_t *yr = x1r;
	bool l;
	struct l_ecc_point *P;

	/*
	 * m = (z^2 * u^4 + z * u^2) modulo p
	 * u2z = u^2 * z
	 * t2 = u2z^2
	 * m = t2 - u2z since for all our curves z is negative
	 */
	_vli_mod_square_fast(u2z, u->c, curve->p, ndigits);
	_vli_mod_mult_fast(u2z, u2z, z, curve->p, ndigits);
	_vli_mod_square_fast(t2, u2z, curve->p, ndigits);
	_vli_mod_sub(m, t2, u2z, curve->p, ndigits);

	/*
	 * l = CEQ(m, 0)
	 * t = inv0(m) where inv0(x) is calculated as x^(p-2) modulo p
	 */
	l = l_secure_memeq(m, sizeof(m), 0);

	memset(t2, 0, sizeof(t2));
	t2[0] = 2ull;
	_vli_mod_sub(t1, curve->p, t2, curve->p, ndigits);
	_vli_mod_exp(t, m, t1, curve->p, ndigits);

	/* Calculate: b / z*a, both z and a are negative */
	_vli_mod_mult_fast(t1, z, _3, curve->p, ndigits);
	_vli_mod_inv(t1, t1, curve->p, ndigits);
	_vli_mod_mult_fast(x1l, curve->b, t1, curve->p, ndigits);

	/* t = 1 + t */
	memset(t2, 0, sizeof(t2));
	t2[0] = 1ull;
	_vli_mod_add(t, t, t2, curve->p, ndigits);

	/* t1 = 1 / a */
	_vli_mod_inv(t1, _3, curve->p, ndigits);

	/* x1r = b * t1 * t */
	_vli_mod_mult_fast(x1r, curve->b, t1, curve->p, ndigits);
	_vli_mod_mult_fast(x1r, x1r, t, curve->p, ndigits);

	/* x1 = CSEL(l, (b / (z*a) modulo p), ((-b/a) * (1 + t)) modulo p) */
	l_secure_select(l, x1l, x1r, x1, ndigits * 8);

	/* gx1 = (x1^3 + a*x1 + b) modulo p */
	ecc_compute_y_sqr(curve, gx1, x1);

	/* x2 = (z*u^2*x1) modulo p, z is negative, hence the second op */
	_vli_mod_mult_fast(x2, u2z, x1, curve->p, ndigits);
	_vli_mod_sub(x2, curve->p, x2, curve->p, ndigits);

	/* gx2 = (x2^3 + a*x2 + b) modulo p */
	ecc_compute_y_sqr(curve, gx2, x2);

	/*
	 * l = gx1 is a quadratic residue modulo p
	 * x is a quadratic residue if x^((p-1)/2) modulo p is zero or one
	 */
	_vli_mod_sub(t1, curve->p, t2, curve->p, ndigits);
	_vli_rshift1(t1, ndigits);
	_vli_mod_exp(t2, gx1, t1, curve->p, ndigits);
	l = _vli_is_zero_or_one(t2, ndigits);

	/* v = CSEL(l, gx1, gx2) */
	l_secure_select(l, gx1, gx2, v, ndigits * 8);
	/* x = CSEL(l, x1, x2) */
	l_secure_select(l, x1, x2, x, ndigits * 8);
	/* y = sqrt(v) */
	ecc_compute_sqrt(curve, yl, v);
	/* l = CEQ(LSB(u), LSB(y)) */
	l = !((u->c[0] & 1ull) ^ (yl[0] & 1ull));

	/* p - y */
	_vli_mod_sub(yr, curve->p, yl, curve->p, ndigits);

	/* P = CSEL(l, (x,y), (x, p-y)) */
	P = l_ecc_point_new(curve);
	memcpy(P->x, x, ndigits * 8);
	l_secure_select(l, yl, yr, P->y, ndigits * 8);

	return P;
}

LIB_EXPORT struct l_ecc_point *l_ecc_point_clone(const struct l_ecc_point *p)
{
	if (!p)
		return NULL;

	return l_memdup(p, sizeof(*p));
}

LIB_EXPORT const struct l_ecc_curve *l_ecc_point_get_curve(
						const struct l_ecc_point *p)
{
	if (!p)
		return NULL;

	return p->curve;
}

LIB_EXPORT ssize_t l_ecc_point_get_x(const struct l_ecc_point *p, void *x,
					size_t xlen)
{
	if (xlen < p->curve->ndigits * 8)
		return -EMSGSIZE;

	_ecc_native2be(x, p->x, p->curve->ndigits);

	return p->curve->ndigits * 8;
}

LIB_EXPORT ssize_t l_ecc_point_get_y(const struct l_ecc_point *p, void *y,
					size_t ylen)
{
	if (ylen < p->curve->ndigits * 8)
		return -EMSGSIZE;

	_ecc_native2be(y, p->y, p->curve->ndigits);

	return p->curve->ndigits * 8;
}

LIB_EXPORT bool l_ecc_point_y_isodd(const struct l_ecc_point *p)
{
	return p->y[0] & 1;
}

LIB_EXPORT ssize_t l_ecc_point_get_data(const struct l_ecc_point *p, void *buf,
					size_t len)
{
	if (len < (p->curve->ndigits * 8) * 2)
		return -EMSGSIZE;

	_ecc_native2be(buf, (uint64_t *) p->x, p->curve->ndigits);
	_ecc_native2be(buf + (p->curve->ndigits * 8), (uint64_t *) p->y,
				p->curve->ndigits);

	return (p->curve->ndigits * 8) * 2;
}

LIB_EXPORT void l_ecc_point_free(struct l_ecc_point *p)
{
	if (unlikely(!p))
		return;

	explicit_bzero(p->x, p->curve->ndigits * 8);
	explicit_bzero(p->y, p->curve->ndigits * 8);
	l_free(p);
}

struct l_ecc_scalar *_ecc_constant_new(const struct l_ecc_curve *curve,
						const void *buf, size_t len)
{
	struct l_ecc_scalar *c;

	if (unlikely(!curve))
		return NULL;

	if (buf && len != curve->ndigits * 8)
		return NULL;

	c = l_new(struct l_ecc_scalar, 1);

	c->curve = curve;

	if (buf)
		memcpy(c->c, buf, len);

	return c;
}

LIB_EXPORT struct l_ecc_scalar *l_ecc_scalar_new(
					const struct l_ecc_curve *curve,
					const void *buf, size_t len)
{
	struct l_ecc_scalar *c;

	c = _ecc_constant_new(curve, NULL, 0);
	if (!c)
		return NULL;

	if (!buf)
		return c;

	_ecc_be2native(c->c, buf, curve->ndigits);

	if (!_vli_is_zero_or_one(c->c, curve->ndigits) &&
			secure_memcmp_64(curve->n, c->c, curve->ndigits) > 0)
		return c;

	l_ecc_scalar_free(c);

	return NULL;
}

LIB_EXPORT struct l_ecc_scalar *l_ecc_scalar_clone(const struct l_ecc_scalar *s)
{
	if (!s)
		return NULL;

	return l_memdup(s, sizeof(*s));
}

/*
 * Build a scalar = value modulo p where p is the prime number for a given
 * curve.  bytes can contain a number with up to 2x number of digits as the
 * curve.  This is used in Hash to Curve calculations.
 */
LIB_EXPORT struct l_ecc_scalar *l_ecc_scalar_new_modp(
					const struct l_ecc_curve *curve,
					const void *bytes, size_t len)
{
	struct l_ecc_scalar *c;
	uint64_t tmp[2 * L_ECC_MAX_DIGITS];
	unsigned int ndigits = len / 8;

	if (!bytes)
		return NULL;

	if (len % 8)
		return NULL;

	if (ndigits > curve->ndigits * 2)
		return NULL;

	c = _ecc_constant_new(curve, NULL, 0);
	if (!c)
		return NULL;

	memset(tmp, 0, sizeof(tmp));
	_ecc_be2native(tmp, bytes, ndigits);

	_vli_mmod_fast(c->c, tmp, curve->p, curve->ndigits);

	if (!_vli_is_zero_or_one(c->c, curve->ndigits) &&
			secure_memcmp_64(curve->n, c->c, curve->ndigits) > 0)
		return c;

	l_ecc_scalar_free(c);

	return NULL;
}

LIB_EXPORT struct l_ecc_scalar *l_ecc_scalar_new_modn(
					const struct l_ecc_curve *curve,
					const void *bytes, size_t len)
{
	struct l_ecc_scalar *c;
	uint64_t tmp[2 * L_ECC_MAX_DIGITS];
	unsigned int ndigits = len / 8;

	if (!bytes)
		return NULL;

	if (len % 8)
		return NULL;

	if (ndigits > curve->ndigits * 2)
		return NULL;

	c = _ecc_constant_new(curve, NULL, 0);
	if (!c)
		return NULL;

	memset(tmp, 0, sizeof(tmp));
	_ecc_be2native(tmp, bytes, ndigits);

	_vli_mmod_slow(c->c, tmp, curve->n, curve->ndigits);

	if (!_vli_is_zero_or_one(c->c, curve->ndigits) &&
			secure_memcmp_64(curve->n, c->c, curve->ndigits) > 0)
		return c;

	l_ecc_scalar_free(c);

	return NULL;
}

/*
 * Takes a buffer of the same size as the curve and scales it to a range
 * 1..n using value = (value mod (n - 1)) + 1.  For the curves we support
 * this can be done using a subtraction operation due to the size of n
 */
LIB_EXPORT struct l_ecc_scalar *l_ecc_scalar_new_reduced_1_to_n(
					const struct l_ecc_curve *curve,
					const void *buf, size_t len)
{
	uint64_t _1[L_ECC_MAX_DIGITS] = { 1ull };
	uint64_t tmp[L_ECC_MAX_DIGITS];
	struct l_ecc_scalar *c;

	if (!buf)
		return NULL;

	if (len != curve->ndigits * 8)
		return NULL;

	c = _ecc_constant_new(curve, NULL, 0);
	if (!c)
		return NULL;

	_vli_sub(tmp, curve->n, _1, curve->ndigits);
	_ecc_be2native(c->c, buf, curve->ndigits);

	if (_vli_cmp(c->c, tmp, curve->ndigits) >= 0)
                _vli_sub(c->c, c->c, tmp, curve->ndigits);

	_vli_add(c->c, c->c, _1, curve->ndigits);

	return c;
}

LIB_EXPORT struct l_ecc_scalar *l_ecc_scalar_new_random(
					const struct l_ecc_curve *curve)
{
	uint64_t r[L_ECC_MAX_DIGITS];

	l_getrandom(r, curve->ndigits * 8);

	while (_vli_cmp(r, curve->p, curve->ndigits) > 0 ||
			_vli_cmp(r, curve->n, curve->ndigits) > 0 ||
			_vli_is_zero_or_one(r, curve->ndigits))
		l_getrandom(r, curve->ndigits * 8);

	return _ecc_constant_new(curve, r, curve->ndigits * 8);
}

LIB_EXPORT ssize_t l_ecc_scalar_get_data(const struct l_ecc_scalar *c,
						void *buf, size_t len)
{
	if (len < c->curve->ndigits * 8)
		return -EMSGSIZE;

	_ecc_native2be(buf, (uint64_t *) c->c, c->curve->ndigits);

	return c->curve->ndigits * 8;
}

LIB_EXPORT void l_ecc_scalar_free(struct l_ecc_scalar *c)
{
	if (unlikely(!c))
		return;

	explicit_bzero(c->c, c->curve->ndigits * 8);
	l_free(c);
}

LIB_EXPORT bool l_ecc_scalar_add(struct l_ecc_scalar *ret,
					const struct l_ecc_scalar *a,
					const struct l_ecc_scalar *b,
					const struct l_ecc_scalar *mod)
{
	if (unlikely(!ret || !a || !b || !mod))
		return false;

	_vli_mod_add(ret->c, a->c, b->c, mod->c, a->curve->ndigits);

	return true;
}

LIB_EXPORT bool l_ecc_point_multiply(struct l_ecc_point *ret,
					const struct l_ecc_scalar *scalar,
					const struct l_ecc_point *point)
{
	if (unlikely(!ret || !scalar || !point))
		return false;

	_ecc_point_mult(ret, point, scalar->c, NULL, scalar->curve->p);

	return true;
}

LIB_EXPORT bool l_ecc_point_multiply_g(struct l_ecc_point *ret,
					const struct l_ecc_scalar *scalar)
{
	if (unlikely(!ret || !scalar))
		return false;

	_ecc_point_mult(ret, &scalar->curve->g, scalar->c, NULL,
						scalar->curve->p);

	return true;
}

LIB_EXPORT bool l_ecc_point_add(struct l_ecc_point *ret,
					const struct l_ecc_point *a,
					const struct l_ecc_point *b)
{
	if (unlikely(!ret || !a || !b))
		return false;

	_ecc_point_add(ret, a, b, a->curve->p);

	return true;
}

LIB_EXPORT bool l_ecc_point_inverse(struct l_ecc_point *p)
{
	if (unlikely(!p))
		return false;

	_vli_mod_sub(p->y, p->curve->p, p->y, p->curve->p, p->curve->ndigits);

	return true;
}

LIB_EXPORT bool l_ecc_scalar_multiply(struct l_ecc_scalar *ret,
					const struct l_ecc_scalar *a,
					const struct l_ecc_scalar *b)
{
	if (unlikely(!ret || !a || !b))
		return false;

	_vli_mod_mult_fast(ret->c, a->c, b->c, a->curve->p, a->curve->ndigits);

	return true;
}

LIB_EXPORT int l_ecc_scalar_legendre(struct l_ecc_scalar *value)
{
	if (unlikely(!value))
		return -1;

	return _vli_legendre(value->c, value->curve->p, value->curve->ndigits);
}

LIB_EXPORT bool l_ecc_scalar_sum_x(struct l_ecc_scalar *ret,
					const struct l_ecc_scalar *x)
{
	if (unlikely(!ret || !x))
		return false;

	ecc_compute_y_sqr(x->curve, ret->c, x->c);

	return true;
}

LIB_EXPORT bool l_ecc_scalars_are_equal(const struct l_ecc_scalar *a,
						const struct l_ecc_scalar *b)
{
	if (unlikely(!a || !b))
		return false;

	return (memcmp(a->c, b->c, a->curve->ndigits * 8) == 0);
}

LIB_EXPORT bool l_ecc_points_are_equal(const struct l_ecc_point *a,
						const struct l_ecc_point *b)
{
	if (unlikely(!a || !b))
		return false;

	return ((memcmp(a->x, b->x, a->curve->ndigits * 8) == 0) &&
			(memcmp(a->y, b->y, a->curve->ndigits * 8) == 0));
}

LIB_EXPORT bool l_ecc_point_is_infinity(const struct l_ecc_point *p)
{
	return _ecc_point_is_zero(p);
}
