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

#include <xfs/libxfs.h>
#include <stdarg.h>
#include "command.h"
#include "output.h"
#include "sig.h"
#include "malloc.h"
#include "init.h"

static int	log_f(int argc, char **argv);

static const cmdinfo_t	log_cmd =
	{ "log", NULL, log_f, 0, 2, 0, N_("[stop|start <filename>]"),
	  N_("start or stop logging to a file"), NULL };

int		dbprefix;
static FILE	*log_file;
static char	*log_file_name;

int
dbprintf(const char *fmt, ...)
{
	va_list	ap;
	int	i;

	if (seenint())
		return 0;
	va_start(ap, fmt);
	blockint();
	i = 0;
	if (dbprefix)
		i += printf("%s: ", fsdevice);
	i += vprintf(fmt, ap);
	unblockint();
	va_end(ap);
	if (log_file) {
		va_start(ap, fmt);
		vfprintf(log_file, fmt, ap);
		va_end(ap);
	}
	return i;
}

static int
log_f(
	int		argc,
	char		**argv)
{
	if (argc == 1) {
		if (log_file)
			dbprintf(_("logging to %s\n"), log_file_name);
		else
			dbprintf(_("no log file\n"));
	} else if (argc == 2 && strcmp(argv[1], "stop") == 0) {
		if (log_file) {
			xfree(log_file_name);
			fclose(log_file);
			log_file = NULL;
		} else
			dbprintf(_("no log file\n"));
	} else if (argc == 3 && strcmp(argv[1], "start") == 0) {
		if (log_file)
			dbprintf(_("already logging to %s\n"), log_file_name);
		else {
			log_file = fopen(argv[2], "a");
			if (log_file == NULL)
				dbprintf(_("can't open %s for writing\n"),
					argv[2]);
			else
				log_file_name = xstrdup(argv[1]);
		}
	} else
		dbprintf(_("bad log command, ignored\n"));
	return 0;
}

void
logprintf(const char *fmt, ...)
{
	va_list	ap;

	if (log_file) {
		va_start(ap, fmt);
		(void)vfprintf(log_file, fmt, ap);
		va_end(ap);
	}
}

void
output_init(void)
{
	add_command(&log_cmd);
}
