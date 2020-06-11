/* ecc-gost-curve.h

   Copyright (C) 2013 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see https://www.gnu.org/licenses/.
*/

/* Development of Nettle's ECC support was funded by the .SE Internet Fund. */

#ifndef GNUTLS_LIB_NETTLE_GOST_ECC_GOST_CURVE_H
#define GNUTLS_LIB_NETTLE_GOST_ECC_GOST_CURVE_H

#ifdef __cplusplus
extern "C" {
#endif

/* The contents of this struct is internal. */
struct ecc_curve;

#ifndef NETTLE_PURE
#ifdef __GNUC__
#define NETTLE_PURE __attribute__((pure))
#else
#define NETTLE_PURE
#endif
#endif

#define gost_point_mul_g _gnutls_nettle_ecc_gost_point_mul_g
#define gost_point_set _gnutls_nettle_ecc_gost_point_set
#define gostdsa_generate_keypair _gnutls_nettle_ecc_gostdsa_generate_keypair
void
gost_point_mul_g (struct ecc_point *r, const struct ecc_scalar *n);
int
gost_point_set (struct ecc_point *p, const mpz_t x, const mpz_t y);
void
gostdsa_generate_keypair (struct ecc_point *pub,
			  struct ecc_scalar *key,
			  void *random_ctx, nettle_random_func *random);

#define nettle_get_gost_gc256b _gnutls_nettle_ecc_get_gost_gc256b
#define nettle_get_gost_gc512a _gnutls_nettle_ecc_get_gost_gc512a
const struct ecc_curve * NETTLE_PURE nettle_get_gost_gc256b(void);
const struct ecc_curve * NETTLE_PURE nettle_get_gost_gc512a(void);

#ifdef __cplusplus
}
#endif

#endif /* GNUTLS_LIB_NETTLE_GOST_ECC_GOST_CURVE_H */
