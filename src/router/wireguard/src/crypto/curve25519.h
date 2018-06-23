/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#ifndef _WG_CURVE25519_H
#define _WG_CURVE25519_H

#include <linux/types.h>

enum curve25519_lengths {
	CURVE25519_POINT_SIZE = 32
};

static bool __must_check curve25519(u8 mypublic[CURVE25519_POINT_SIZE], const u8 secret[CURVE25519_POINT_SIZE], const u8 basepoint[CURVE25519_POINT_SIZE]);
static void curve25519_generate_secret(u8 secret[CURVE25519_POINT_SIZE]);
static bool __must_check curve25519_generate_public(u8 pub[CURVE25519_POINT_SIZE], const u8 secret[CURVE25519_POINT_SIZE]);

static void curve25519_fpu_init(void);

#ifdef DEBUG
static bool curve25519_selftest(void);
#endif

#endif /* _WG_CURVE25519_H */
