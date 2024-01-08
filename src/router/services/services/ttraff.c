/*
 * ttraff.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h> /* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

char *ttraff_deps(void)
{
	return "ttraff_enable ttraff_iface wan_proto";
}

char *ttraff_proc(void)
{
	return "ttraff";
}

void stop_ttraff(void);
void start_ttraff(void)
{
	stop_ttraff();
	if (!nvram_matchi("ttraff_enable", 1))
		return;

	if ((nvram_match("ttraff_iface", "") || !nvram_exists("ttraff_iface")) &&
	    (nvram_match("wan_proto", "disabled") || getWET()))
		return;

	char *argv[] = { "ttraff", NULL };
	_log_evalpid(argv, NULL, 0, NULL);

	return;
}

void stop_ttraff(void)
{
	stop_process("ttraff", "traffic counter daemon");
	nvram_delstates(ttraff_deps());
	cprintf("done\n");

	return;
}
