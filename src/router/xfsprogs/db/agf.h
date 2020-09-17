// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	agf_flds[];
extern const struct field	agf_hfld[];

extern void	agf_init(void);
extern int	agf_size(void *obj, int startoff, int idx);
