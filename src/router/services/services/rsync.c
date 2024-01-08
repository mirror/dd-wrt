/*
 * rsync.c
 *
 * Copyright (C) 2019 dd-wrt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_RSYNC
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>
#include <rsync.h>

char *rsync_deps(void)
{
	return "rsyncd_enable lan_ipaddr rsync_allowed rsync_shares";
}

char *rsync_proc(void)
{
	return "rsyncd";
}

void stop_rsync(void);
void start_rsync(void)
{
	struct rsync_share *cs, *csnext;
	struct rsync_share *rsyncshares;
	stop_rsync();
	if (!nvram_matchi("rsyncd_enable", 1))
		return;
	FILE *fp = fopen("/tmp/rsyncd.conf", "wb");

	fprintf(fp, "gid = root\n");
	fprintf(fp, "uid = root\n");
	fprintf(fp, "read only = no\n");
	fprintf(fp, "use chroot = true\n");
	fprintf(fp, "transfer logging = false\n");
	fprintf(fp, "log format = %%h %%o %%f %%l %%b\n");
	fprintf(fp, "log file = /var/log/rsyncd.log\n");
	fprintf(fp, "pid file = /var/run/rsyncd.pid\n");
	char *add = nvram_safe_get("rsync_allowed");
	fprintf(fp, "hosts allow = %s/%d%s%s\n", nvram_safe_get("lan_ipaddr"), getmask(nvram_safe_get("lan_netmask")),
		*add ? " " : "", add);
	fprintf(fp, "slp refresh = 300\n");
	fprintf(fp, "use slp = false\n");

	if (fp) {
		rsyncshares = getrsyncshares();
		for (cs = rsyncshares; cs; cs = csnext) {
			if (*cs->label && *cs->mp) {
				fprintf(fp, "[%s]\n", cs->label);
				fprintf(fp, "\tpath = %s/%s\n", cs->mp, cs->sd);
			}
			csnext = cs->next;
			free(cs);
		}
		fclose(fp);
	}
	log_eval("rsyncd", "--daemon", "--config=/tmp/rsyncd.conf");
	return;
}

void stop_rsync(void)
{
	stop_process("rsyncd", "rsync daemon");
	nvram_delstates(rsync_deps());
}
#endif
