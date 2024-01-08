/*
 * mkfiles.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

#define HOME_DIR "/tmp/root"
#define PASSWD_DIR "/tmp/etc"
#define SSH_DIR "/tmp/root/.ssh"
#define PASSWD_FILE "/tmp/etc/passwd"
#define GROUP_FILE "/tmp/etc/group"

int isregistered_real(void);

void setPassword(char *passwd)
{
	FILE *fp;
	struct stat buf;

	/*
	 * Create password's and group's database directory 
	 */
	if (stat(PASSWD_DIR, &buf) != 0) {
		mkdir(PASSWD_DIR, 0700);
	}
	if (!(fp = fopen(PASSWD_FILE, "w"))) {
		perror(PASSWD_FILE);
		return;
	}
#ifdef HAVE_ERC
	// fprintf(fp, "Admin:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", passwd);
	fprintf(fp, "root:*NOLOGIN*:0:0:Root User,,,:/tmp/root:/bin/sh\n");
	fprintf(fp, "SuperAdmin:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", nvram_safe_get("newhttp_passwd"));
	fprintf(fp, "reeapi:$1$oBrBCDd2$zLGC6enVwcGWigRVWzc9f0:0:0:Reeapi User,,,:/tmp/root:/bin/sh\n");
#elif HAVE_IPR
	fprintf(fp, "SuperAdmin:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", nvram_safe_get("newhttp_passwd"));
#elif HAVE_NDTRADE
	fprintf(fp, "ndtrade:$1$N2vARlRA$bI0Exx9L.3A103888c7gk.:0:0:ndtrade User,,,:/tmp/root:/bin/sh\n");
	fprintf(fp, "root:x:98:98:Root:/:/bin/false\n");
#else
	fprintf(fp, "root:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", passwd);
#endif
	fprintf(fp, "nobody:x:99:99:Nobody:/:/bin/false\n");
	fclose(fp);
}

void start_mkfiles(void)
{
	FILE *fp;
	struct stat buf;

	cprintf("%s:%d", __func__, __LINE__);
#ifdef HAVE_SKYTRON
	char *http_passwd = nvram_safe_get("skyhttp_passwd");
#elif HAVE_34TELECOM
	char *http_passwd = nvram_safe_get("newhttp_passwd");
#else
	char *http_passwd = nvram_safe_get("http_passwd");
#endif
	cprintf("%s:%d", __func__, __LINE__);
	if (stat(HOME_DIR, &buf) != 0) {
		mkdir(HOME_DIR, 0700);
	}
	cprintf("%s:%d", __func__, __LINE__);

#ifdef HAVE_SSHD
	if (stat(SSH_DIR, &buf) != 0) {
		mkdir(SSH_DIR, 0700);
	}
#endif
	cprintf("%s:%d", __func__, __LINE__);

	/*
	 * Create password's and group's database directory 
	 */
	if (stat(PASSWD_DIR, &buf) != 0) {
		mkdir(PASSWD_DIR, 0700);
	}
	cprintf("%s:%d", __func__, __LINE__);

	/*
	 * Write password file with username root and password 
	 */
	if (!(fp = fopen(PASSWD_FILE, "w"))) {
		perror(PASSWD_FILE);
		return;
	}
	cprintf("%s:%d", __func__, __LINE__);
#ifdef HAVE_ERC
	// fprintf(fp, "Admin:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n",
	//      http_passwd);
	fprintf(fp, "root:*NOLOGIN*:0:0:Root User,,,:/tmp/root:/bin/sh\n");
	fprintf(fp, "SuperAdmin:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", nvram_safe_get("newhttp_passwd"));
	fprintf(fp, "reeapi:$1$oBrBCDd2$zLGC6enVwcGWigRVWzc9f0:0:0:Reeapi User,,,:/tmp/root:/bin/sh\n");
#elif HAVE_WIKINGS
	// default username and password for Excel Networks
	fprintf(fp, "ExNet:$1$tkH3Bh9Z$/op5lnArS3Cba4eiruJMV/:0:0:Root User,,,:/tmp/root:/bin/sh\n");
#elif HAVE_IPR
	fprintf(fp, "SuperAdmin:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n", nvram_safe_get("newhttp_passwd"));
#elif HAVE_NDTRADE
	fprintf(fp, "ndtrade:$1$N2vARlRA$bI0Exx9L.3A103888c7gk.:0:0:ndtrade User,,,:/tmp/root:/bin/sh\n");
	fprintf(fp, "root:x:98:98:Root:/:/bin/false\n");
#else
	fprintf(fp,
		"root:%s:0:0:Root User,,,:/tmp/root:/bin/sh\n"
		"reboot:%s:0:0:Root User,,,:/tmp/root:/sbin/reboot\n",
		http_passwd, http_passwd);
#endif
	fprintf(fp, "nobody:x:99:99:Nobody:/:/bin/false\n");
	fclose(fp);
	cprintf("%s:%d", __func__, __LINE__);
	/*
	 * Write group file with group 'root' 
	 */
	if (!(fp = fopen(GROUP_FILE, "w"))) {
		perror(GROUP_FILE);
		return;
	}
	cprintf("%s:%d", __func__, __LINE__);
	fprintf(fp, "root:x:0:\n");
	fprintf(fp, "nobody:x:99:\n");
	fclose(fp);

	mkdir("/var/spool", 0700);

	mkdir("/var/spool/cron", 0700);
	mkdir("/var/lock", 0700);
	mkdir("/var/lock/subsys", 0700);
	mkdir("/var/spool/cron/crontabs", 0700);
	eval("touch", "/var/spool/cron/crontabs/root");
	mkdir("/var/lib", 0700);
	mkdir("/var/lib/misc", 0700);
	mkdir("/var/tmp", 0700);
	mkdir("/var/log", 0700);
	eval("touch", "/var/log/messages");
	cprintf("%s:%d", __func__, __LINE__);
	chmod("/tmp", 0777);
	cprintf("%s:%d", __func__, __LINE__);

	dns_to_resolv();
	cprintf("%s:%d", __func__, __LINE__);
	if (nvram_matchi("unblock", 1)) {
		nvram_unset("unblock");
#ifdef HAVE_TELNET
		start_telnetd(); //password has been changed, now we can start telnet or ssh (if enabled)
#endif
#ifdef HAVE_SSHD
		start_sshd();
#endif
	}

	return;
}

int setpasswd_main(int argc, char *argv[])
{
	start_mkfiles();
	return 0;
}
