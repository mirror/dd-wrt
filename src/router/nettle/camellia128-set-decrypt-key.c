/* camellia128-set-decrypt-key.c

   Inverse key setup for the camellia block cipher.

   Copyright (C) 2010, 2013 Niels Möller

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
   not, see http://www.gnu.org/licenses/.
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "camellia-internal.h"

void
camellia128_invert_key(struct camellia128_ctx *dst,
		       const struct camellia128_ctx *src)
{
  _nettle_camellia_invert_key (_CAMELLIA128_NKEYS, dst->keys, src->keys);
}

void
camellia128_set_decrypt_key(struct camellia128_ctx *ctx,
			    const uint8_t *key)
{
  camellia128_set_encrypt_key(ctx, key);
  camellia128_invert_key(ctx, ctx);
}
