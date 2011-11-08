/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

typedef int (*cfunc_t)(int argc, char **argv);
typedef void (*helpfunc_t)(void);

typedef struct cmdinfo
{
	const char	*name;
	const char	*altname;
	cfunc_t		cfunc;
	int		argmin;
	int		argmax;
	int		canpush;
	const char	*args;
	const char	*oneline;
	helpfunc_t      help;
} cmdinfo_t;

extern cmdinfo_t	*cmdtab;
extern int		ncmds;

extern void		add_command(const cmdinfo_t *ci);
extern int		command(int argc, char **argv);
extern const cmdinfo_t	*find_command(const char *cmd);
extern void		init_commands(void);
