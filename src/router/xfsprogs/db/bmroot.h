// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	bmroota_flds[];
extern const struct field	bmroota_key_flds[];
extern const struct field	bmrootd_flds[];
extern const struct field	bmrootd_key_flds[];

extern int	bmroota_size(void *obj, int startoff, int idx);
extern int	bmrootd_size(void *obj, int startoff, int idx);
