/* random.h - random functions
 *	Copyright (C) 1998, 2002 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef G10_RANDOM_H
#define G10_RANDOM_H

#include "types.h"

void _gcry_random_initialize (int);
void _gcry_register_random_progress (void (*cb)(void *,const char*,int,int,int),
                                     void *cb_data );
void _gcry_random_dump_stats(void);
void _gcry_secure_random_alloc(void);
int  _gcry_quick_random_gen( int onoff );
int  _gcry_random_is_faked(void);
void _gcry_set_random_seed_file (const char *name);
void _gcry_update_random_seed_file (void);

byte *_gcry_get_random_bits( size_t nbits, int level, int secure );
void _gcry_fast_random_poll( void );

#endif /*G10_RANDOM_H*/




