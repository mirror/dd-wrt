/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_ECC_H
#define __ELL_ECC_H

#include <sys/types.h>
#include <ell/cleanup.h>

#ifdef __cplusplus
extern "C" {
#endif

#define L_ECC_MAX_DIGITS 9
#define L_ECC_SCALAR_MAX_BYTES		L_ECC_MAX_DIGITS * 8
#define L_ECC_POINT_MAX_BYTES		L_ECC_SCALAR_MAX_BYTES * 2

struct l_ecc_curve;
struct l_ecc_point;
struct l_ecc_scalar;

enum l_ecc_point_type {
	L_ECC_POINT_TYPE_COMPLIANT = 0x01,
	L_ECC_POINT_TYPE_COMPRESSED_BIT0 = 0x02,
	L_ECC_POINT_TYPE_COMPRESSED_BIT1 = 0x03,
	L_ECC_POINT_TYPE_FULL = 0x04,
};

const unsigned int *l_ecc_supported_ike_groups(void);
const unsigned int *l_ecc_supported_tls_groups(void);

const struct l_ecc_curve *l_ecc_curve_from_name(const char *name);
const struct l_ecc_curve *l_ecc_curve_from_ike_group(unsigned int group);
const struct l_ecc_curve *l_ecc_curve_from_tls_group(unsigned int group);

const char *l_ecc_curve_get_name(const struct l_ecc_curve *curve);
unsigned int l_ecc_curve_get_ike_group(const struct l_ecc_curve *curve);
unsigned int l_ecc_curve_get_tls_group(const struct l_ecc_curve *curve);
struct l_ecc_scalar *l_ecc_curve_get_order(const struct l_ecc_curve *curve);
struct l_ecc_scalar *l_ecc_curve_get_prime(const struct l_ecc_curve *curve);
size_t l_ecc_curve_get_scalar_bytes(const struct l_ecc_curve *curve);

struct l_ecc_point *l_ecc_point_new(const struct l_ecc_curve *curve);
struct l_ecc_point *l_ecc_point_from_data(const struct l_ecc_curve *curve,
					enum l_ecc_point_type type,
					const void *data, size_t len);
struct l_ecc_point *l_ecc_point_from_sswu(const struct l_ecc_scalar *u);
struct l_ecc_point *l_ecc_point_clone(const struct l_ecc_point *p);

const struct l_ecc_curve *l_ecc_point_get_curve(const struct l_ecc_point *p);
ssize_t l_ecc_point_get_x(const struct l_ecc_point *p, void *x, size_t xlen);
ssize_t l_ecc_point_get_y(const struct l_ecc_point *p, void *y, size_t ylen);
bool l_ecc_point_y_isodd(const struct l_ecc_point *p);

ssize_t l_ecc_point_get_data(const struct l_ecc_point *p, void *buf, size_t len);
void l_ecc_point_free(struct l_ecc_point *p);
DEFINE_CLEANUP_FUNC(l_ecc_point_free);

struct l_ecc_scalar *l_ecc_scalar_new(const struct l_ecc_curve *curve,
						const void *buf, size_t len);
struct l_ecc_scalar *l_ecc_scalar_clone(const struct l_ecc_scalar *s);
struct l_ecc_scalar *l_ecc_scalar_new_random(
					const struct l_ecc_curve *curve);
struct l_ecc_scalar *l_ecc_scalar_new_modp(const struct l_ecc_curve *curve,
						const void *buf, size_t len);
struct l_ecc_scalar *l_ecc_scalar_new_modn(const struct l_ecc_curve *curve,
						const void *buf, size_t len);
struct l_ecc_scalar *l_ecc_scalar_new_reduced_1_to_n(
					const struct l_ecc_curve *curve,
					const void *buf, size_t len);
ssize_t l_ecc_scalar_get_data(const struct l_ecc_scalar *c, void *buf,
					size_t len);
void l_ecc_scalar_free(struct l_ecc_scalar *c);
DEFINE_CLEANUP_FUNC(l_ecc_scalar_free);

/* Constant operations */
bool l_ecc_scalar_add(struct l_ecc_scalar *ret, const struct l_ecc_scalar *a,
				const struct l_ecc_scalar *b,
				const struct l_ecc_scalar *mod);

/* Point operations */
bool l_ecc_point_multiply(struct l_ecc_point *ret,
				const struct l_ecc_scalar *scalar,
				const struct l_ecc_point *point);
bool l_ecc_point_multiply_g(struct l_ecc_point *ret,
				const struct l_ecc_scalar *scalar);
bool l_ecc_point_add(struct l_ecc_point *ret, const struct l_ecc_point *a,
				const struct l_ecc_point *b);
bool l_ecc_point_inverse(struct l_ecc_point *p);

/* extra operations needed for SAE */
bool l_ecc_scalar_multiply(struct l_ecc_scalar *ret,
				const struct l_ecc_scalar *a,
				const struct l_ecc_scalar *b);
int l_ecc_scalar_legendre(struct l_ecc_scalar *value);
bool l_ecc_scalar_sum_x(struct l_ecc_scalar *ret, const struct l_ecc_scalar *x);

bool l_ecc_scalars_are_equal(const struct l_ecc_scalar *a,
				const struct l_ecc_scalar *b);

bool l_ecc_points_are_equal(const struct l_ecc_point *a,
				const struct l_ecc_point *b);
bool l_ecc_point_is_infinity(const struct l_ecc_point *p);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_ECC_H */
