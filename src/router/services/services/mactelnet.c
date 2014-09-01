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

void start_mactelnetd(void)
{
	int ret = 0;
	pid_t pid;

	char *telnetd_argv[] = { "mactelnetd", NULL };
	stop_mactelnetd();

	if (!nvram_invmatch("mactelnetd_enable", "0"))
		return;
	if (strlen(nvram_safe_get("mactelnetd_passwd")) == 0)
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
	dd_syslog(LOG_INFO, "mactelnetd : MAC Telnet daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_mactelnetd(void)
{
	stop_process("mactelnetd", "MAC Telnet daemon");
}
#endif
