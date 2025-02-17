/*
 * Copyright (c) 2013, Kenneth MacKay
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdbool.h>

#include "private.h"
#include "ecc.h"
#include "ecc-private.h"
#include "random.h"

typedef struct {
	uint64_t m_low;
	uint64_t m_high;
} uint128_t;

static void vli_clear(uint64_t *vli, unsigned int ndigits)
{
	unsigned int i;

	for (i = 0; i < ndigits; i++)
		vli[i] = 0;
}

/* Returns true if vli == 0, false otherwise. */
static bool vli_is_zero(const uint64_t *vli, unsigned int ndigits)
{
	unsigned int i;

	for (i = 0; i < ndigits; i++) {
		if (vli[i])
			return false;
	}

	return true;
}

/* Returns nonzero if bit bit of vli is set. */
static uint64_t vli_test_bit(const uint64_t *vli, unsigned int bit)
{
	return (vli[bit / 64] & ((uint64_t) 1 << (bit % 64)));
}

/* Sets dest = src. */
static void vli_set(uint64_t *dest, const uint64_t *src, unsigned int ndigits)
{
	unsigned int i;

	for (i = 0; i < ndigits; i++)
		dest[i] = src[i];
}

/* Returns sign of left - right. */
int _vli_cmp(const uint64_t *left, const uint64_t *right, unsigned int ndigits)
{
	int i;

	for (i = ndigits - 1; i >= 0; i--) {
		if (left[i] > right[i])
			return 1;
		if (left[i] < right[i])
			return -1;
	}

	return 0;
}

/* Computes result = in << c, returning carry. Can modify in place
 * (if result == in). 0 < shift < 64.
 */
static uint64_t vli_lshift(uint64_t *result, const uint64_t *in,
							unsigned int shift,
							unsigned int ndigits)
{
	uint64_t carry = 0;
	unsigned int i;

	for (i = 0; i < ndigits; i++) {
		uint64_t temp = in[i];

		result[i] = (temp << shift) | carry;
		carry = temp >> (64 - shift);
	}

	return carry;
}

/* Computes vli = vli >> 1. */
void _vli_rshift1(uint64_t *vli, unsigned int ndigits)
{
	uint64_t *end = vli;
	uint64_t carry = 0;

	vli += ndigits;

	while (vli-- > end) {
		uint64_t temp = *vli;
		*vli = (temp >> 1) | carry;
		carry = temp << 63;
	}
}

/* Computes result = left + right, returning carry. Can modify in place. */
uint64_t _vli_add(uint64_t *result, const uint64_t *left,
				const uint64_t *right, unsigned int ndigits)
{
	uint64_t carry = 0;
	unsigned int i;

	for (i = 0; i < ndigits; i++) {
		uint64_t sum;

		sum = left[i] + right[i] + carry;
		if (sum != left[i])
			carry = (sum < left[i]);

		result[i] = sum;
	}

	return carry;
}

/* Computes result = left - right, returning borrow. Can modify in place. */
uint64_t _vli_sub(uint64_t *result, const uint64_t *left,
				const uint64_t *right, unsigned int ndigits)
{
	uint64_t borrow = 0;
	unsigned int i;

	for (i = 0; i < ndigits; i++) {
		uint64_t diff;

		diff = left[i] - right[i] - borrow;
		if (diff != left[i])
			borrow = (diff > left[i]);

		result[i] = diff;
	}

	return borrow;
}

static uint128_t mul_64_64(uint64_t left, uint64_t right)
{
	uint64_t a0 = left & 0xffffffffull;
	uint64_t a1 = left >> 32;
	uint64_t b0 = right & 0xffffffffull;
	uint64_t b1 = right >> 32;
	uint64_t m0 = a0 * b0;
	uint64_t m1 = a0 * b1;
	uint64_t m2 = a1 * b0;
	uint64_t m3 = a1 * b1;
	uint128_t result;

	m2 += (m0 >> 32);
	m2 += m1;

	/* Overflow */
	if (m2 < m1)
		m3 += 0x100000000ull;

	result.m_low = (m0 & 0xffffffffull) | (m2 << 32);
	result.m_high = m3 + (m2 >> 32);

	return result;
}

static uint128_t add_128_128(uint128_t a, uint128_t b)
{
	uint128_t result;

	result.m_low = a.m_low + b.m_low;
	result.m_high = a.m_high + b.m_high + (result.m_low < a.m_low);

	return result;
}

static void vli_mult(uint64_t *result, const uint64_t *left,
							const uint64_t *right,
							unsigned int ndigits)
{
	uint128_t r01 = { 0, 0 };
	uint64_t r2 = 0;
	unsigned int i, k;

	/* Compute each digit of result in sequence, maintaining the
	 * carries.
	 */
	for (k = 0; k < ndigits * 2 - 1; k++) {
		unsigned int min;

		if (k < ndigits)
			min = 0;
		else
			min = (k + 1) - ndigits;

		for (i = min; i <= k && i < ndigits; i++) {
			uint128_t product;

			product = mul_64_64(left[i], right[k - i]);

			r01 = add_128_128(r01, product);
			r2 += (r01.m_high < product.m_high);
		}

		result[k] = r01.m_low;
		r01.m_low = r01.m_high;
		r01.m_high = r2;
		r2 = 0;
	}

	result[ndigits * 2 - 1] = r01.m_low;
}

static void vli_square(uint64_t *result, const uint64_t *left,
			unsigned int ndigits)
{
	uint128_t r01 = { 0, 0 };
	uint64_t r2 = 0;
	unsigned int i, k;

	for (k = 0; k < ndigits * 2 - 1; k++) {
		unsigned int min;

		if (k < ndigits)
			min = 0;
		else
			min = (k + 1) - ndigits;

		for (i = min; i <= k && i <= k - i; i++) {
			uint128_t product;

			product = mul_64_64(left[i], left[k - i]);

			if (i < k - i) {
				r2 += product.m_high >> 63;
				product.m_high = (product.m_high << 1) |
							(product.m_low >> 63);
				product.m_low <<= 1;
			}

			r01 = add_128_128(r01, product);
			r2 += (r01.m_high < product.m_high);
		}

		result[k] = r01.m_low;
		r01.m_low = r01.m_high;
		r01.m_high = r2;
		r2 = 0;
	}

	result[ndigits * 2 - 1] = r01.m_low;
}

/* Computes result = (left + right) % mod.
 * Assumes that left < mod and right < mod, result != mod.
 */
void _vli_mod_add(uint64_t *result, const uint64_t *left,
				const uint64_t *right, const uint64_t *mod,
				unsigned int ndigits)
{
	uint64_t carry;

	carry = _vli_add(result, left, right, ndigits);

	/* result > mod (result = mod + remainder), so subtract mod to
	 * get remainder.
	 */
	if (carry || _vli_cmp(result, mod, ndigits) >= 0)
		_vli_sub(result, result, mod, ndigits);
}

/* Computes result = (left - right) % mod.
 * Assumes that left < mod and right < mod, result != mod.
 */
void _vli_mod_sub(uint64_t *result, const uint64_t *left,
				const uint64_t *right, const uint64_t *mod,
				unsigned int ndigits)
{
	uint64_t borrow = _vli_sub(result, left, right, ndigits);

	/* In this case, p_result == -diff == (max int) - diff.
	 * Since -x % d == d - x, we can get the correct result from
	 * result + mod (with overflow).
	 */
	if (borrow)
		_vli_add(result, result, mod, ndigits);
}

/* Counts the number of 64-bit "digits" in vli. */
static unsigned int _vli_num_digits(const uint64_t *vli, unsigned int ndigits)
{
	int i;

	/* Search from the end until we find a non-zero digit.
	 * We do it in reverse because we expect that most digits will
	 * be nonzero.
	 */
	for (i = ndigits - 1; i >= 0 && vli[i] == 0; i--);

	return (i + 1);
}

/* Counts the number of bits required for vli. */
static unsigned int _vli_num_bits(const uint64_t *vli, unsigned int ndigits)
{
	unsigned int i, num_digits;
	uint64_t digit;

	num_digits = _vli_num_digits(vli, ndigits);
	if (num_digits == 0)
		return 0;

	digit = vli[num_digits - 1];
	for (i = 0; digit; i++)
		digit >>= 1;

	return ((num_digits - 1) * 64 + i);
}

/* Computes result = product % mod, where product is 2N words long.
 * Currently only designed to work for curve_p or curve_n.
 */
void _vli_mmod_slow(uint64_t *result, const uint64_t *product,
				const uint64_t *mod, unsigned int ndigits)
{
	uint64_t mod_m[2 * L_ECC_MAX_DIGITS];
	uint64_t tmp[2 * L_ECC_MAX_DIGITS];
	uint64_t *v[2] = { tmp, (uint64_t *) product };
	uint64_t carry = 0;
	unsigned int i;
	/* Shift mod so its highest set bit is at the maximum position. */
	int shift = (ndigits * 2 * 64) - _vli_num_bits(mod, ndigits);
	int word_shift = shift / 64;
	int bit_shift = shift % 64;

	vli_clear(mod_m, word_shift);
	if (bit_shift > 0) {
		for (i = 0; i < ndigits; ++i) {
			mod_m[word_shift + i] = (mod[i] << bit_shift) | carry;
			carry = mod[i] >> (64 - bit_shift);
		}
	} else
		vli_set(mod_m + word_shift, mod, ndigits);

	for (i = 1; shift >= 0; --shift) {
		uint64_t borrow = 0;
		unsigned int j;

		for (j = 0; j < ndigits * 2; ++j) {
			uint64_t diff = v[i][j] - mod_m[j] - borrow;

			if (diff != v[i][j])
				borrow = (diff > v[i][j]);
			v[1 - i][j] = diff;
		}
		i = !(i ^ borrow); /* Swap the index if there was no borrow */
		_vli_rshift1(mod_m, ndigits);
		mod_m[ndigits - 1] |= mod_m[ndigits] << (64 - 1);
		_vli_rshift1(mod_m + ndigits, ndigits);
	}
	vli_set(result, v[i], ndigits);
}

/* Computes p_result = p_product % curve_p.
 * See algorithm 5 and 6 from
 * http://www.isys.uni-klu.ac.at/PDF/2001-0126-MT.pdf
 */
static void vli_mmod_fast_192(uint64_t *result, const uint64_t *product,
				const uint64_t *curve_prime, uint64_t *tmp)
{
	const unsigned int ndigits = 3;
	int carry;

	vli_set(result, product, ndigits);

	vli_set(tmp, &product[3], ndigits);
	carry = _vli_add(result, result, tmp, ndigits);

	tmp[0] = 0;
	tmp[1] = product[3];
	tmp[2] = product[4];
	carry += _vli_add(result, result, tmp, ndigits);

	tmp[0] = tmp[1] = product[5];
	tmp[2] = 0;
	carry += _vli_add(result, result, tmp, ndigits);

	while (carry || _vli_cmp(curve_prime, result, ndigits) != 1)
		carry -= _vli_sub(result, result, curve_prime, ndigits);
}

/* Computes result = product % curve_prime
 * from http://www.nsa.gov/ia/_files/nist-routines.pdf
 */
static void vli_mmod_fast_256(uint64_t *result, const uint64_t *product,
				const uint64_t *curve_prime, uint64_t *tmp)
{
	int carry;
	const unsigned int ndigits = 4;

	/* t */
	vli_set(result, product, ndigits);

	/* s1 */
	tmp[0] = 0;
	tmp[1] = product[5] & 0xffffffff00000000ull;
	tmp[2] = product[6];
	tmp[3] = product[7];
	carry = vli_lshift(tmp, tmp, 1, ndigits);
	carry += _vli_add(result, result, tmp, ndigits);

	/* s2 */
	tmp[1] = product[6] << 32;
	tmp[2] = (product[6] >> 32) | (product[7] << 32);
	tmp[3] = product[7] >> 32;
	carry += vli_lshift(tmp, tmp, 1, ndigits);
	carry += _vli_add(result, result, tmp, ndigits);

	/* s3 */
	tmp[0] = product[4];
	tmp[1] = product[5] & 0xffffffff;
	tmp[2] = 0;
	tmp[3] = product[7];
	carry += _vli_add(result, result, tmp, ndigits);

	/* s4 */
	tmp[0] = (product[4] >> 32) | (product[5] << 32);
	tmp[1] = (product[5] >> 32) | (product[6] & 0xffffffff00000000ull);
	tmp[2] = product[7];
	tmp[3] = (product[6] >> 32) | (product[4] << 32);
	carry += _vli_add(result, result, tmp, ndigits);

	/* d1 */
	tmp[0] = (product[5] >> 32) | (product[6] << 32);
	tmp[1] = (product[6] >> 32);
	tmp[2] = 0;
	tmp[3] = (product[4] & 0xffffffff) | (product[5] << 32);
	carry -= _vli_sub(result, result, tmp, ndigits);

	/* d2 */
	tmp[0] = product[6];
	tmp[1] = product[7];
	tmp[2] = 0;
	tmp[3] = (product[4] >> 32) | (product[5] & 0xffffffff00000000ull);
	carry -= _vli_sub(result, result, tmp, ndigits);

	/* d3 */
	tmp[0] = (product[6] >> 32) | (product[7] << 32);
	tmp[1] = (product[7] >> 32) | (product[4] << 32);
	tmp[2] = (product[4] >> 32) | (product[5] << 32);
	tmp[3] = (product[6] << 32);
	carry -= _vli_sub(result, result, tmp, ndigits);

	/* d4 */
	tmp[0] = product[7];
	tmp[1] = product[4] & 0xffffffff00000000ull;
	tmp[2] = product[5];
	tmp[3] = product[6] & 0xffffffff00000000ull;
	carry -= _vli_sub(result, result, tmp, ndigits);

	if (carry < 0) {
		do {
			carry += _vli_add(result, result, curve_prime, ndigits);
		} while (carry < 0);
	} else {
		while (carry || _vli_cmp(curve_prime, result, ndigits) != 1)
			carry -= _vli_sub(result, result, curve_prime, ndigits);
	}
}

/*
 * The NIST algorithms define S values, which are comprised of 32 bit C values
 * of the original product we are trying to reduce. Since we are working with
 * 64 bit 'digits', we need to convert these C values into 64 bit chunks. This
 * macro mainly makes code readability easier since we can directly pass the
 * two C indexes (h and l). Some of these C values are zero, which is also a
 * value C index. In this case -1 should be passed to indicate zero.
 */
#define ECC_SET_S(prod, h, l) ({ 				\
	uint64_t r = 0;						\
	if (h == -1) {						\
		/* zero, don't do anything */			\
	} else if (h & 1)					\
		r |= (prod[h / 2] & 0xffffffff00000000ull);	\
	else							\
		r |= (prod[h / 2] << 32);			\
	if (l == -1) {						\
		/* zero, don't do anything */			\
	} else if (l & 1)					\
		r |= (prod[l / 2] >> 32);			\
	else							\
		r |= (prod[l / 2] & 0xffffffff);		\
	r;							\
})

static void vli_mmod_fast_384(uint64_t *result, const uint64_t *product,
				const uint64_t *curve_prime, uint64_t *tmp)
{
	int carry;
	const unsigned int ndigits = 6;

	/* t */
	vli_set(result, product, ndigits);

	/* s1 */
	tmp[0] = 0;
	tmp[1] = 0;
	tmp[2] = ECC_SET_S(product, 22, 21);
	tmp[3] = ECC_SET_S(product, -1, 23);
	tmp[4] = 0;
	tmp[5] = 0;
	carry = vli_lshift(tmp, tmp, 1, ndigits);
	carry += _vli_add(result, result, tmp, ndigits);

	/* s2 */
	tmp[0] = product[6];
	tmp[1] = product[7];
	tmp[2] = product[8];
	tmp[3] = product[9];
	tmp[4] = product[10];
	tmp[5] = product[11];
	carry += _vli_add(result, result, tmp, ndigits);

	/* s3 */
	tmp[0] = ECC_SET_S(product, 22, 21);
	tmp[1] = ECC_SET_S(product, 12, 23);
	tmp[2] = ECC_SET_S(product, 14, 13);
	tmp[3] = ECC_SET_S(product, 16, 15);
	tmp[4] = ECC_SET_S(product, 18, 17);
	tmp[5] = ECC_SET_S(product, 20, 19);
	carry += _vli_add(result, result, tmp, ndigits);

	/* s4 */
	tmp[0] = ECC_SET_S(product, 23, -1);
	tmp[1] = ECC_SET_S(product, 20, -1);
	tmp[2] = ECC_SET_S(product, 13, 12);
	tmp[3] = ECC_SET_S(product, 15, 14);
	tmp[4] = ECC_SET_S(product, 17, 16);
	tmp[5] = ECC_SET_S(product, 19, 18);
	carry += _vli_add(result, result, tmp, ndigits);

	/* s5 */
	tmp[0] = 0;
	tmp[1] = 0;
	tmp[2] = ECC_SET_S(product, 21, 20);
	tmp[3] = ECC_SET_S(product, 23, 22);
	tmp[4] = 0;
	tmp[5] = 0;
	carry += _vli_add(result, result, tmp, ndigits);

	/* s6 */
	tmp[0] = ECC_SET_S(product, -1, 20);
	tmp[1] = ECC_SET_S(product, 21, -1);
	tmp[2] = ECC_SET_S(product, 23, 22);
	tmp[3] = 0;
	tmp[4] = 0;
	tmp[5] = 0;
	carry += _vli_add(result, result, tmp, ndigits);

	/* s7 */
	tmp[0] = ECC_SET_S(product, 12, 23);
	tmp[1] = ECC_SET_S(product, 14, 13);
	tmp[2] = ECC_SET_S(product, 16, 15);
	tmp[3] = ECC_SET_S(product, 18, 17);
	tmp[4] = ECC_SET_S(product, 20, 19);
	tmp[5] = ECC_SET_S(product, 22, 21);
	carry -= _vli_sub(result, result, tmp, ndigits);

	/* s8 */
	tmp[0] = ECC_SET_S(product, 20, -1);
	tmp[1] = ECC_SET_S(product, 22, 21);
	tmp[2] = ECC_SET_S(product, -1, 23);
	tmp[3] = 0;
	tmp[4] = 0;
	tmp[5] = 0;
	carry -= _vli_sub(result, result, tmp, ndigits);

	/* s9 */
	tmp[0] = 0;
	tmp[1] = ECC_SET_S(product, 23, -1);
	tmp[2] = ECC_SET_S(product, -1, 23);
	tmp[3] = 0;
	tmp[4] = 0;
	tmp[5] = 0;
	carry -= _vli_sub(result, result, tmp, ndigits);

	if (carry < 0) {
		do {
			carry += _vli_add(result, result, curve_prime, ndigits);
		} while (carry < 0);
	} else {
		while (carry || _vli_cmp(curve_prime, result, ndigits) != 1)
			carry -= _vli_sub(result, result, curve_prime, ndigits);
	}
}

/* Computes result = product % curve_prime
 *  from http://www.nsa.gov/ia/_files/nist-routines.pdf
*/
bool _vli_mmod_fast(uint64_t *result, const uint64_t *product,
			const uint64_t *curve_prime, unsigned int ndigits)
{
	uint64_t tmp[2 * L_ECC_MAX_DIGITS];

	switch (ndigits) {
	case 3:
		vli_mmod_fast_192(result, product, curve_prime, tmp);
		break;
	case 4:
		vli_mmod_fast_256(result, product, curve_prime, tmp);
		break;
	case 6:
		vli_mmod_fast_384(result, product, curve_prime, tmp);
		break;
	default:
		return false;
	}

	return true;
}

/* Computes result = (left * right) % curve_p. */
void _vli_mod_mult_fast(uint64_t *result, const uint64_t *left,
			const uint64_t *right, const uint64_t *curve_prime,
			unsigned int ndigits)
{
	uint64_t product[2 * L_ECC_MAX_DIGITS];

	vli_mult(product, left, right, ndigits);
	_vli_mmod_fast(result, product, curve_prime, ndigits);
}

/* Computes result = left^2 % curve_p. */
void _vli_mod_square_fast(uint64_t *result, const uint64_t *left,
					const uint64_t *curve_prime,
					unsigned int ndigits)
{
	uint64_t product[2 * L_ECC_MAX_DIGITS];

	vli_square(product, left, ndigits);
	_vli_mmod_fast(result, product, curve_prime, ndigits);
}

#define EVEN(vli) (!(vli[0] & 1))
/* Computes result = (1 / p_input) % mod. All VLIs are the same size.
 * See "From Euclid's GCD to Montgomery Multiplication to the Great Divide"
 * https://labs.oracle.com/techrep/2001/smli_tr-2001-95.pdf
 */
void _vli_mod_inv(uint64_t *result, const uint64_t *input,
						const uint64_t *mod,
						unsigned int ndigits)
{
	uint64_t a[L_ECC_MAX_DIGITS], b[L_ECC_MAX_DIGITS];
	uint64_t u[L_ECC_MAX_DIGITS], v[L_ECC_MAX_DIGITS];
	uint64_t carry;
	int cmp_result;

	if (vli_is_zero(input, ndigits)) {
		vli_clear(result, ndigits);
		return;
	}

	vli_set(a, input, ndigits);
	vli_set(b, mod, ndigits);
	vli_clear(u, ndigits);
	u[0] = 1;
	vli_clear(v, ndigits);

	while ((cmp_result = _vli_cmp(a, b, ndigits)) != 0) {
		carry = 0;

		if (EVEN(a)) {
			_vli_rshift1(a, ndigits);

			if (!EVEN(u))
				carry = _vli_add(u, u, mod, ndigits);

			_vli_rshift1(u, ndigits);
			if (carry)
				u[ndigits - 1] |= 0x8000000000000000ull;
		} else if (EVEN(b)) {
			_vli_rshift1(b, ndigits);

			if (!EVEN(v))
				carry = _vli_add(v, v, mod, ndigits);

			_vli_rshift1(v, ndigits);
			if (carry)
				v[ndigits - 1] |= 0x8000000000000000ull;
		} else if (cmp_result > 0) {
			_vli_sub(a, a, b, ndigits);
			_vli_rshift1(a, ndigits);

			if (_vli_cmp(u, v, ndigits) < 0)
				_vli_add(u, u, mod, ndigits);

			_vli_sub(u, u, v, ndigits);
			if (!EVEN(u))
				carry = _vli_add(u, u, mod, ndigits);

			_vli_rshift1(u, ndigits);
			if (carry)
				u[ndigits - 1] |= 0x8000000000000000ull;
		} else {
			_vli_sub(b, b, a, ndigits);
			_vli_rshift1(b, ndigits);

			if (_vli_cmp(v, u, ndigits) < 0)
				_vli_add(v, v, mod, ndigits);

			_vli_sub(v, v, u, ndigits);
			if (!EVEN(v))
				carry = _vli_add(v, v, mod, ndigits);

			_vli_rshift1(v, ndigits);
			if (carry)
				v[ndigits - 1] |= 0x8000000000000000ull;
		}
	}

	vli_set(result, u, ndigits);
}

/* ------ Point operations ------ */

/* Point multiplication algorithm using Montgomery's ladder with co-Z
 * coordinates. From http://eprint.iacr.org/2011/338.pdf
 */

/* Double in place */
static void ecc_point_double_jacobian(uint64_t *x1, uint64_t *y1, uint64_t *z1,
					const uint64_t *curve_prime,
					unsigned int ndigits)
{
	/* t1 = x, t2 = y, t3 = z */
	uint64_t t4[L_ECC_MAX_DIGITS];
	uint64_t t5[L_ECC_MAX_DIGITS];

	if (vli_is_zero(z1, ndigits))
		return;

	/* t4 = y1^2 */
	_vli_mod_square_fast(t4, y1, curve_prime, ndigits);
	/* t5 = x1*y1^2 = A */
	_vli_mod_mult_fast(t5, x1, t4, curve_prime, ndigits);
	/* t4 = y1^4 */
	_vli_mod_square_fast(t4, t4, curve_prime, ndigits);
	/* t2 = y1*z1 = z3 */
	_vli_mod_mult_fast(y1, y1, z1, curve_prime, ndigits);
	/* t3 = z1^2 */
	_vli_mod_square_fast(z1, z1, curve_prime, ndigits);

	/* t1 = x1 + z1^2 */
	_vli_mod_add(x1, x1, z1, curve_prime, ndigits);
	/* t3 = 2*z1^2 */
	_vli_mod_add(z1, z1, z1, curve_prime, ndigits);
	/* t3 = x1 - z1^2 */
	_vli_mod_sub(z1, x1, z1, curve_prime, ndigits);
	/* t1 = x1^2 - z1^4 */
	_vli_mod_mult_fast(x1, x1, z1, curve_prime, ndigits);

	/* t3 = 2*(x1^2 - z1^4) */
	_vli_mod_add(z1, x1, x1, curve_prime, ndigits);
	/* t1 = 3*(x1^2 - z1^4) */
	_vli_mod_add(x1, x1, z1, curve_prime, ndigits);
	if (vli_test_bit(x1, 0)) {
		uint64_t carry = _vli_add(x1, x1, curve_prime, ndigits);
		_vli_rshift1(x1, ndigits);
		x1[ndigits - 1] |= carry << 63;
	} else {
		_vli_rshift1(x1, ndigits);
	}
	/* t1 = 3/2*(x1^2 - z1^4) = B */

	/* t3 = B^2 */
	_vli_mod_square_fast(z1, x1, curve_prime, ndigits);
	/* t3 = B^2 - A */
	_vli_mod_sub(z1, z1, t5, curve_prime, ndigits);
	/* t3 = B^2 - 2A = x3 */
	_vli_mod_sub(z1, z1, t5, curve_prime, ndigits);
	/* t5 = A - x3 */
	_vli_mod_sub(t5, t5, z1, curve_prime, ndigits);
	/* t1 = B * (A - x3) */
	_vli_mod_mult_fast(x1, x1, t5, curve_prime, ndigits);
	/* t4 = B * (A - x3) - y1^4 = y3 */
	_vli_mod_sub(t4, x1, t4, curve_prime, ndigits);

	vli_set(x1, z1, ndigits);
	vli_set(z1, y1, ndigits);
	vli_set(y1, t4, ndigits);
}

/* Modify (x1, y1) => (x1 * z^2, y1 * z^3) */
static void apply_z(uint64_t *x1, uint64_t *y1, uint64_t *z,
			const uint64_t *curve_prime, unsigned int ndigits)
{
	uint64_t t1[L_ECC_MAX_DIGITS];

	_vli_mod_square_fast(t1, z, curve_prime, ndigits);    /* z^2 */
	_vli_mod_mult_fast(x1, x1, t1, curve_prime, ndigits); /* x1 * z^2 */
	_vli_mod_mult_fast(t1, t1, z, curve_prime, ndigits);  /* z^3 */
	_vli_mod_mult_fast(y1, y1, t1, curve_prime, ndigits); /* y1 * z^3 */
}

/* P = (x1, y1) => 2P, (x2, y2) => P' */
static void xycz_initial_double(uint64_t *x1, uint64_t *y1, uint64_t *x2,
					uint64_t *y2, uint64_t *p_initial_z,
					const uint64_t *curve_prime,
					unsigned int ndigits)
{
	uint64_t z[L_ECC_MAX_DIGITS];

	vli_set(x2, x1, ndigits);
	vli_set(y2, y1, ndigits);

	vli_clear(z, ndigits);
	z[0] = 1;

	if (p_initial_z)
		vli_set(z, p_initial_z, ndigits);

	apply_z(x1, y1, z, curve_prime, ndigits);

	ecc_point_double_jacobian(x1, y1, z, curve_prime, ndigits);

	apply_z(x2, y2, z, curve_prime, ndigits);
}

/* Input P = (x1, y1, Z), Q = (x2, y2, Z)
 * Output P' = (x1', y1', Z3), P + Q = (x3, y3, Z3)
 * or P => P', Q => P + Q
 */
static void xycz_add(uint64_t *x1, uint64_t *y1, uint64_t *x2, uint64_t *y2,
			const uint64_t *curve_prime, unsigned int ndigits)
{
	/* t1 = X1, t2 = Y1, t3 = X2, t4 = Y2 */
	uint64_t t5[L_ECC_MAX_DIGITS];

	/* t5 = x2 - x1 */
	_vli_mod_sub(t5, x2, x1, curve_prime, ndigits);
	/* t5 = (x2 - x1)^2 = A */
	_vli_mod_square_fast(t5, t5, curve_prime, ndigits);
	/* t1 = x1*A = B */
	_vli_mod_mult_fast(x1, x1, t5, curve_prime, ndigits);
	/* t3 = x2*A = C */
	_vli_mod_mult_fast(x2, x2, t5, curve_prime, ndigits);
	/* t4 = y2 - y1 */
	_vli_mod_sub(y2, y2, y1, curve_prime, ndigits);
	/* t5 = (y2 - y1)^2 = D */
	_vli_mod_square_fast(t5, y2, curve_prime, ndigits);

	/* t5 = D - B */
	_vli_mod_sub(t5, t5, x1, curve_prime, ndigits);
	/* t5 = D - B - C = x3 */
	_vli_mod_sub(t5, t5, x2, curve_prime, ndigits);
	/* t3 = C - B */
	_vli_mod_sub(x2, x2, x1, curve_prime, ndigits);
	/* t2 = y1*(C - B) */
	_vli_mod_mult_fast(y1, y1, x2, curve_prime, ndigits);
	/* t3 = B - x3 */
	_vli_mod_sub(x2, x1, t5, curve_prime, ndigits);
	/* t4 = (y2 - y1)*(B - x3) */
	_vli_mod_mult_fast(y2, y2, x2, curve_prime, ndigits);
	/* t4 = y3 */
	_vli_mod_sub(y2, y2, y1, curve_prime, ndigits);

	vli_set(x2, t5, ndigits);
}

/* Input P = (x1, y1, Z), Q = (x2, y2, Z)
 * Output P + Q = (x3, y3, Z3), P - Q = (x3', y3', Z3)
 * or P => P - Q, Q => P + Q
 */
static void xycz_add_c(uint64_t *x1, uint64_t *y1, uint64_t *x2, uint64_t *y2,
			const uint64_t *curve_prime, unsigned int ndigits)
{
	/* t1 = X1, t2 = Y1, t3 = X2, t4 = Y2 */
	uint64_t t5[L_ECC_MAX_DIGITS];
	uint64_t t6[L_ECC_MAX_DIGITS];
	uint64_t t7[L_ECC_MAX_DIGITS];

	/* t5 = x2 - x1 */
	_vli_mod_sub(t5, x2, x1, curve_prime, ndigits);
	/* t5 = (x2 - x1)^2 = A */
	_vli_mod_square_fast(t5, t5, curve_prime, ndigits);
	/* t1 = x1*A = B */
	_vli_mod_mult_fast(x1, x1, t5, curve_prime, ndigits);
	/* t3 = x2*A = C */
	_vli_mod_mult_fast(x2, x2, t5, curve_prime, ndigits);
	/* t4 = y2 + y1 */
	_vli_mod_add(t5, y2, y1, curve_prime, ndigits);
	/* t4 = y2 - y1 */
	_vli_mod_sub(y2, y2, y1, curve_prime, ndigits);

	/* t6 = C - B */
	_vli_mod_sub(t6, x2, x1, curve_prime, ndigits);
	/* t2 = y1 * (C - B) */
	_vli_mod_mult_fast(y1, y1, t6, curve_prime, ndigits);
	/* t6 = B + C */
	_vli_mod_add(t6, x1, x2, curve_prime, ndigits);
	/* t3 = (y2 - y1)^2 */
	_vli_mod_square_fast(x2, y2, curve_prime, ndigits);
	/* t3 = x3 */
	_vli_mod_sub(x2, x2, t6, curve_prime, ndigits);

	/* t7 = B - x3 */
	_vli_mod_sub(t7, x1, x2, curve_prime, ndigits);
	/* t4 = (y2 - y1)*(B - x3) */
	_vli_mod_mult_fast(y2, y2, t7, curve_prime, ndigits);
	/* t4 = y3 */
	_vli_mod_sub(y2, y2, y1, curve_prime, ndigits);

	/* t7 = (y2 + y1)^2 = F */
	_vli_mod_square_fast(t7, t5, curve_prime, ndigits);
	/* t7 = x3' */
	_vli_mod_sub(t7, t7, t6, curve_prime, ndigits);
	/* t6 = x3' - B */
	_vli_mod_sub(t6, t7, x1, curve_prime, ndigits);
	/* t6 = (y2 + y1)*(x3' - B) */
	_vli_mod_mult_fast(t6, t6, t5, curve_prime, ndigits);
	/* t2 = y3' */
	_vli_mod_sub(y1, t6, y1, curve_prime, ndigits);

	vli_set(x1, t7, ndigits);
}

void _ecc_point_mult(struct l_ecc_point *result,
			const struct l_ecc_point *point, const uint64_t *scalar,
			uint64_t *initial_z, const uint64_t *curve_prime)
{
	/* R0 and R1 */
	const struct l_ecc_curve *curve = point->curve;
	uint64_t rx[2][L_ECC_MAX_DIGITS];
	uint64_t ry[2][L_ECC_MAX_DIGITS];
	uint64_t z[L_ECC_MAX_DIGITS];
	uint64_t sk[2][L_ECC_MAX_DIGITS];
	int i, nb;
	unsigned int ndigits = curve->ndigits;
	int num_bits;
	int carry;

	carry = _vli_add(sk[0], scalar, curve->n, ndigits);
	_vli_add(sk[1], sk[0], curve->n, ndigits);
	scalar = sk[!carry];
	num_bits = sizeof(uint64_t) * ndigits * 8 + 1;

	vli_set(rx[1], point->x, ndigits);
	vli_set(ry[1], point->y, ndigits);

	xycz_initial_double(rx[1], ry[1], rx[0], ry[0], initial_z, curve_prime,
				ndigits);

	for (i = num_bits - 2; i > 0; i--) {
		nb = !vli_test_bit(scalar, i);
		xycz_add_c(rx[1 - nb], ry[1 - nb], rx[nb], ry[nb], curve_prime,
				ndigits);
		xycz_add(rx[nb], ry[nb], rx[1 - nb], ry[1 - nb], curve_prime,
				ndigits);
	}

	nb = !vli_test_bit(scalar, 0);
	xycz_add_c(rx[1 - nb], ry[1 - nb], rx[nb], ry[nb], curve_prime,
			ndigits);

	/* Find final 1/Z value. */
	/* X1 - X0 */
	_vli_mod_sub(z, rx[1], rx[0], curve_prime, ndigits);
	/* Yb * (X1 - X0) */
	_vli_mod_mult_fast(z, z, ry[1 - nb], curve_prime, ndigits);
	/* xP * Yb * (X1 - X0) */
	_vli_mod_mult_fast(z, z, point->x, curve_prime, ndigits);

	/* 1 / (xP * Yb * (X1 - X0)) */
	_vli_mod_inv(z, z, curve_prime, ndigits);

	/* yP / (xP * Yb * (X1 - X0)) */
	_vli_mod_mult_fast(z, z, point->y, curve_prime, ndigits);
	/* Xb * yP / (xP * Yb * (X1 - X0)) */
	_vli_mod_mult_fast(z, z, rx[1 - nb], curve_prime, ndigits);
	/* End 1/Z calculation */

	xycz_add(rx[nb], ry[nb], rx[1 - nb], ry[1 - nb], curve_prime, ndigits);

	apply_z(rx[0], ry[0], z, curve_prime, ndigits);

	vli_set(result->x, rx[0], ndigits);
	vli_set(result->y, ry[0], ndigits);
}

/* Returns true if p_point is the point at infinity, false otherwise. */
bool _ecc_point_is_zero(const struct l_ecc_point *point)
{
	return (vli_is_zero(point->x, point->curve->ndigits) &&
		vli_is_zero(point->y, point->curve->ndigits));
}
