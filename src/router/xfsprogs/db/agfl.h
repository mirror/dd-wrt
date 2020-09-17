// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern const struct field	agfl_flds[];
extern const struct field	agfl_hfld[];
extern const struct field	agfl_crc_flds[];
extern const struct field	agfl_crc_hfld[];

extern void	agfl_init(void);
extern int	agfl_size(void *obj, int startoff, int idx);
