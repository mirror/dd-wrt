// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const field_t	dir2sf_flds[];
extern const field_t	dir2_inou_flds[];
extern const field_t	dir2_sf_hdr_flds[];
extern const field_t	dir2_sf_entry_flds[];

extern const field_t	dir3sf_flds[];
extern const field_t	dir3_sf_entry_flds[];

extern int	dir2sf_size(void *obj, int startoff, int idx);
extern int	dir2_inou_size(void *obj, int startoff, int idx);
extern int	dir2_sf_entry_size(void *obj, int startoff, int idx);
extern int	dir2_sf_hdr_size(void *obj, int startoff, int idx);
