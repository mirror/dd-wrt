/*
 * wireguard_config.c
 *
 * Copyright (C) 2005 - 2021 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <utils.h>
#include <broadcom.h>
#include <dd_defs.h>
#include <revision.h>

#define cprintf(fmt, args...)

static int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
static char *wfgets(char *buf, int len, webs_t fp, int *eof);
size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
int wfflush(webs_t fp);
static int do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);

static int download_wireguard_config(unsigned char method, struct mime_handler *handler, char *path, webs_t wp)
{
	char fname[128];
	snprintf(fname, sizeof(fname), "%s", path);
	char dname[128];
	snprintf(dname, sizeof(dname), "%s", path);
	char *p = strstr(dname, "..");
	if (p)
		return -1;
	p = strstr(dname, "/");
	if (p)
		return -1;

	p = strstr(dname, ".");
	if (p)
		*p = '_';
	char location[128];
	snprintf(location, sizeof(location), "/tmp/wireguard/%s", &dname[sizeof("wireguard_config")]);
	return do_file_attach(handler, location, wp, fname);
}
