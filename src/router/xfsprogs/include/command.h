/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <sys/time.h>

/*
 * A "oneshot" command ony runs once per command execution. It does
 * not iterate the command args function callout and so can be used
 * for functions like "help" that should only ever be run once.
 */
#define CMD_FLAG_ONESHOT	(1u << 31)
#define CMD_FLAG_FOREIGN_OK	(1u << 30) /* command not restricted to XFS */
#define CMD_FLAG_LIBRARY	(1u << 29) /* command provided by libxcmd */

typedef int (*cfunc_t)(int argc, char **argv);
typedef void (*helpfunc_t)(void);

typedef struct cmdinfo {
	const char	*name;
	const char	*altname;
	cfunc_t		cfunc;
	int		argmin;
	int		argmax;
	int		canpush;
	int		flags;
	const char	*args;
	const char	*oneline;
	helpfunc_t      help;
} cmdinfo_t;

extern cmdinfo_t	*cmdtab;
extern int		ncmds;

extern void		help_init(void);
extern void		quit_init(void);

typedef int (*iterfunc_t)(int index);
typedef int (*checkfunc_t)(const cmdinfo_t *ci);

extern void		add_command(const cmdinfo_t *ci);
extern void		add_user_command(char *optarg);
extern void		add_oneshot_user_command(char *optarg);
extern void		add_command_iterator(iterfunc_t func);
extern void		add_check_command(checkfunc_t cf);

extern const cmdinfo_t	*find_command(const char *cmd);

extern void		command_loop(void);
extern int		command_usage(const cmdinfo_t *ci);
extern int		command(const cmdinfo_t *ci, int argc, char **argv);

extern void		report_io_times(const char *verb, struct timeval *t2,
					long long offset, long long count,
					long long total, int ops, int compact);

#endif	/* __COMMAND_H__ */
