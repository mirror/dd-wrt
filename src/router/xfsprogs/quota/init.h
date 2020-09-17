// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern char	*progname;
extern int	exitcode;
extern int	expert;
extern bool	foreign_allowed;

extern void	edit_init(void);
extern void	free_init(void);
extern void	path_init(void);
extern void	project_init(void);
extern void	quot_init(void);
extern void	quota_init(void);
extern void	report_init(void);
extern void	state_init(void);

extern void init_cvtnum(unsigned int *, unsigned int *);
