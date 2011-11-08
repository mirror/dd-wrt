/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include <sys/mman.h>
#include "init.h"
#include "io.h"

static cmdinfo_t file_cmd;
static cmdinfo_t print_cmd;

fileio_t	*filetable;
int		filecount;
fileio_t	*file;

static void
print_fileio(
	fileio_t	*file,
	int		index,
	int		braces)
{
	printf(_("%c%03d%c %-14s (%s,%s,%s,%s%s%s%s)\n"),
		braces? '[' : ' ', index, braces? ']' : ' ', file->name,
		file->flags & IO_FOREIGN ? _("foreign") : _("xfs"),
		file->flags & IO_OSYNC ? _("sync") : _("non-sync"),
		file->flags & IO_DIRECT ? _("direct") : _("non-direct"),
		file->flags & IO_READONLY ? _("read-only") : _("read-write"),
		file->flags & IO_REALTIME ? _(",real-time") : "",
		file->flags & IO_APPEND ? _(",append-only") : "",
		file->flags & IO_NONBLOCK ? _(",non-block") : "");
}

int
filelist_f(void)
{
	int		i;

	for (i = 0; i < filecount; i++)
		print_fileio(&filetable[i], i, &filetable[i] == file);
	return 0;
}

static int
print_f(
	int		argc,
	char		**argv)
{
	filelist_f();
	maplist_f();
	return 0;
}

static int
file_f(
	int		argc,
	char		**argv)
{
	int		i;

	if (argc <= 1)
		return filelist_f();
	i = atoi(argv[1]);
	if (i < 0 || i >= filecount) {
		printf(_("value %d is out of range (0-%d)\n"), i, filecount-1);
	} else {
		file = &filetable[i];
		filelist_f();
	}
	return 0;
}

void
file_init(void)
{
	file_cmd.name = "file";
	file_cmd.altname = "f";
	file_cmd.args = _("[N]");
	file_cmd.cfunc = file_f;
	file_cmd.argmin = 0;
	file_cmd.argmax = 1;
	file_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	file_cmd.oneline = _("set the current file");

	print_cmd.name = "print";
	print_cmd.altname = "p";
	print_cmd.cfunc = print_f;
	print_cmd.argmin = 0;
	print_cmd.argmax = 0;
	print_cmd.flags = CMD_NOMAP_OK | CMD_NOFILE_OK | CMD_FOREIGN_OK |
				CMD_FLAG_GLOBAL;
	print_cmd.oneline = _("list current open files and memory mappings");

	add_command(&file_cmd);
	add_command(&print_cmd);
}
