/*
 * proftp.c
 *
 * Copyright (C) 2008 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_FTP
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
#include <samba3.h>

static void ftpsrv_umount()
{
	FILE *fp;
	char line[256];
	char mnt_dir[32] = "/tmp/proftpd/users";

	//cleanup
	if ((fp = fopen("/proc/mounts", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, "proftpd")) {
				sysprintf("/bin/umount %s/*/*", mnt_dir);
			}
		}
		fclose(fp);
	}
}

char *ftpsrv_deps(void)
{
	return "proftpd_acctserverport proftpd_authserverip proftpd_enable proftpd_anon_subdir proftpd_authserverport proftpd_anon proftpd_port proftpd_anon_dir proftpd_rad proftpd_wan proftpd_sharedkey samba3_users samba3_shares";
}

char *ftpsrv_proc(void)
{
	return "proftpd";
}

void stop_ftpsrv(void);

void start_ftpsrv(void)
{
	struct samba3_share *cs, *csnext;
	struct samba3_shareuser *csu, *csunext;
	struct samba3_user *samba3users, *cu, *cunext;
	struct samba3_share *samba3shares;

	if (!nvram_matchi("proftpd_enable", 1)) {
		stop_ftpsrv();
		return;
	}

	FILE *fp, *tmp;

	mkdir("/tmp/proftpd", 0700);
	mkdir("/tmp/proftpd/etc", 0700);
	mkdir("/tmp/proftpd/var", 0700);

	fp = fopen("/tmp/proftpd/etc/passwd", "wb");

	samba3users = getsamba3users();
	for (cu = samba3users; cu; cu = cunext) {
		if (*cu->username && cu->sharetype & SHARETYPE_FTP) {
			sysprintf("mkdir -p \"/tmp/proftpd/users/%s\"", cu->username);
			char passout[MD5_OUT_BUFSIZE];
			fprintf(fp, "%s:%s:0:0:Ftp User,,,:/tmp/proftpd/users/%s:/bin/sh\n", cu->username,
				zencrypt(cu->password, passout), cu->username);
		}
		cunext = cu->next;
		free(cu);
	}

	fclose(fp);

	// add ftp user (for anonymous access)
	if (nvram_matchi("proftpd_anon", 1)) {
		fp = fopen("/tmp/proftpd/etc/passwd", "ab");
		fprintf(fp, "ftp:x:0:0:Ftp Anon,,,:/tmp/root:/bin/sh\n");
		fclose(fp);
	}

	fp = fopen("/tmp/proftpd/etc/proftpd.conf", "wb");
	fprintf(fp,
		"ServerName      DD-WRT\n"
		"DefaultAddress  %s\n"
		"ServerType      standalone\n"
		"DefaultServer   on\n"
		"ScoreboardFile  /tmp/proftpd/etc/proftpd.scoreboard\n"
		"Port            %s\n"
		"Umask           022\n"
		"MaxInstances    10\n"
		"MaxLoginAttempts 3\n"
		"User            root\n"
		"Group           root\n"
		"UseReverseDNS   off\n"
		"RootLogin       on\n"
		"AllowOverwrite  on\n"
		"AllowRetrieveRestart  on\n"
		"AllowStoreRestart  on\n"
		"<Limit WRITE>\n  DenyAll\n</Limit>\n"
		"<Limit SITE_CHMOD>\n"
		"  DenyAll\n"
		"</Limit>\n"
		"DelayEngine     off\n"
		"WtmpLog         off\n"
		"DefaultRoot     ~\n",
		nvram_safe_get("lan_ipaddr"), nvram_safe_get("proftpd_port"));

	samba3shares = getsamba3shares();
	for (cs = samba3shares; cs; cs = csnext) {
		int hasuser = 0;
		for (csu = cs->users; csu; csu = csunext) {
			samba3users = getsamba3users();
			for (cu = samba3users; cu; cu = cunext) {
				if (!strcmp(csu->username, cu->username) && (cu->sharetype & SHARETYPE_FTP))
					hasuser = 1;
				cunext = cu->next;
				free(cu);
			}
			csunext = csu->next;
		}
		if (!hasuser) {
			for (csu = cs->users; csu; csu = csunext) {
				csunext = csu->next;
				free(csu);
			}
			goto nextshare;
		}

		fprintf(fp, "<Directory      \"~/%s\">\n", cs->label);

		if (!strcmp(cs->access_perms, "rw"))
			fprintf(fp, "  <Limit WRITE>\n    AllowAll\n  </Limit>\n");
		for (csu = cs->users; csu; csu = csunext) {
			samba3users = getsamba3users();
			for (cu = samba3users; cu; cu = cunext) {
				if (!strcmp(csu->username, cu->username) && (cu->sharetype & SHARETYPE_FTP)) {
					sysprintf("mkdir -p \"/tmp/proftpd/users/%s/%s\"", cu->username, cs->label);
					sysprintf("mount --bind \"%s/%s\" \"/tmp/proftpd/users/%s/%s\"", cs->mp, cs->sd,
						  cu->username, cs->label);
				}
				cunext = cu->next;
				free(cu);
			}
			csunext = csu->next;
			free(csu);
		}

		fprintf(fp, "</Directory>\n");

nextshare:;
		csnext = cs->next;
		free(cs);
	}

	if (nvram_matchi("proftpd_rad", 0))
		fprintf(fp, "AuthUserFile	/tmp/proftpd/etc/passwd\n");
	else {
		fprintf(fp,
			"AuthOrder mod_radius.c\n"
			"RadiusEngine	on\n"
			"RadiusAuthServer	%s:%s	%s 5\n"
			"RadiusAcctServer	%s:%s	%s 5\n",
			nvram_safe_get("proftpd_authserverip"), nvram_safe_get("proftpd_authserverport"),
			nvram_safe_get("proftpd_sharedkey"), nvram_safe_get("proftpd_authserverip"),
			nvram_safe_get("proftpd_acctserverport"), nvram_safe_get("proftpd_sharedkey"));
		fprintf(fp, "RadiusUserInfo 0 0 %s /bin/false\n",
			"/mnt"); //TODO allow to choose dir
	}
	// Anonymous ftp - read only
	if (nvram_matchi("proftpd_anon", 1)) {
		fprintf(fp,
			"<Anonymous      \"%s\">\n"
			"User           ftp\n"
			"Group          root\n"
			"UserAlias      anonymous ftp\n"
			"</Anonymous>\n",
			nvram_safe_get("proftpd_anon_dir"));
	}
	fclose(fp);
	chmod("/tmp/proftpd/etc/passwd", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (reload_process("proftpd")) {
#ifdef HAVE_SMP
		if (eval("/usr/bin/taskset", "0x2", "proftpd"))
#endif
			eval("proftpd");
		dd_loginfo("proftpd", "server successfully started");
	}
	nvram_set("usb_reason", "proftp_start");
	eval("service", "run_rc_usb", "start");

	return;
}

void restart_ftpsrv(void)
{
	start_ftpsrv();
}

void stop_ftpsrv(void)
{
	unlink("/tmp/proftpd/etc/passwd");
	unlink("/tmp/proftpd/etc/proftpd.conf");
	unlink("/tmp/proftpd/etc/proftpd.scoreboard");

	stop_process("proftpd", "FTP Server");
	ftpsrv_umount();
	nvram_delstates(ftpsrv_deps());
}
#endif
