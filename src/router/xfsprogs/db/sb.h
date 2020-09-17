// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	sb_flds[];
extern const struct field	sb_hfld[];

extern void	sb_init(void);
extern int	sb_logcheck(void);
extern int	sb_size(void *obj, int startoff, int idx);
