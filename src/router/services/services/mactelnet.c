/*
 * mactelnet.c
 *
 * Copyright (C) 2014 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_MACTELNET
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

char *mactelnetd_deps(void)
{
	return "mactelnetd_enable mactelnetd_passwd";
}

char *mactelnetd_proc(void)
{
	return "mactelnetd";
}

void start_mactelnetd(void)
{
	pid_t pid;
	int ret = 0;
	char *telnetd_argv[] = { "mactelnetd", "-n", NULL };
	stop_mactelnetd();

	if (!nvram_invmatchi("mactelnetd_enable", 0))
		return;
	if (!*(nvram_safe_get("mactelnetd_passwd")))
		return;
	FILE *fp = fopen("/tmp/mactelnetd.users", "wb");
	fprintf(fp, "root:%s\n", nvram_safe_get("mactelnetd_passwd"));
	fclose(fp);

#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		ret = _evalpid(telnetd_argv, NULL, 0, &pid);
#ifdef HAVE_REGISTER
	else
		return;
#endif
	dd_logstart("mactelnetd", ret);

	cprintf("done\n");
	return;
}

void stop_mactelnetd(void)
{
	stop_process("mactelnetd", "daemon");
	nvram_delstates(mactelnetd_deps());
}
#endif
