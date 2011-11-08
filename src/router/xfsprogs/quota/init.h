/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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

extern char	*progname;
extern int	exitcode;
extern int	expert;

extern void	edit_init(void);
extern void	free_init(void);
extern void	path_init(void);
extern void	project_init(void);
extern void	quot_init(void);
extern void	quota_init(void);
extern void	report_init(void);
extern void	state_init(void);

extern void init_cvtnum(unsigned int *, unsigned int *);
