/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#ifndef _WG_ZINC_H
#define _WG_ZINC_H

static int chacha20_mod_init(void);
static int poly1305_mod_init(void);
static int chacha20poly1305_mod_init(void);
static int blake2s_mod_init(void);
static int curve25519_mod_init(void);

#endif
