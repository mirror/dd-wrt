// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const field_t	attr_sf_entry_flds[];
extern const field_t	attr_sf_hdr_flds[];
extern const field_t	attr_shortform_flds[];
extern const field_t	attrshort_hfld[];

extern int	attr_sf_entry_size(void *obj, int startoff, int idx);
extern int	attrshort_size(void *obj, int startoff, int idx);
