// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern void	add_strvec(char ***vecp, char *str);
extern char	**copy_strvec(char **vec);
extern void	free_strvec(char **vec);
extern char	**new_strvec(int count);
extern void	print_strvec(char **vec);
