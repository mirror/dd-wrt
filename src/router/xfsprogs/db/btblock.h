/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

extern const struct field	bmapbta_flds[];
extern const struct field	bmapbta_hfld[];
extern const struct field	bmapbta_key_flds[];
extern const struct field	bmapbta_rec_flds[];

extern const struct field	bmapbtd_flds[];
extern const struct field	bmapbtd_hfld[];
extern const struct field	bmapbtd_key_flds[];
extern const struct field	bmapbtd_rec_flds[];

extern const struct field	inobt_flds[];
extern const struct field	inobt_hfld[];
extern const struct field	inobt_key_flds[];
extern const struct field	inobt_rec_flds[];

extern const struct field	bnobt_flds[];
extern const struct field	bnobt_hfld[];
extern const struct field	bnobt_key_flds[];
extern const struct field	bnobt_rec_flds[];

extern const struct field	cntbt_flds[];
extern const struct field	cntbt_hfld[];
extern const struct field	cntbt_key_flds[];
extern const struct field	cntbt_rec_flds[];

extern int	btblock_size(void *obj, int startoff, int idx);
