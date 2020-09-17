// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern char	**breakline(char *input, int *count);
extern void	doneline(char *input, char **vec);
extern char	*fetchline(void);
extern void	input_init(void);
extern void	pushfile(FILE *file);
