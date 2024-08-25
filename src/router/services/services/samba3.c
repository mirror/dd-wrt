/*
 * samba3.c
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
#ifdef HAVE_SAMBA3
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>
#include <samba3.h>
#include <fcntl.h>

static int has_sambauser(struct samba3_shareuser *csu, struct samba3_user *samba3users)
{
	struct samba3_user *cu, *cunext;
	int hasuser = 0;
	for (cu = samba3users; cu; cu = cunext) {
		if (!strcmp(csu->username, cu->username) && (cu->sharetype & SHARETYPE_SAMBA))
			hasuser = 1;
		cunext = cu->next;
		free(cu);
	}
	return hasuser;
}

static void free_users(struct samba3_share *cs)
{
	struct samba3_shareuser *csu, *csunext;
	for (csu = cs->users; csu; csu = csunext) {
		csunext = csu->next;
		free(csu);
	}
}

void stop_samba3(void);

void start_samba3(void)
{
	char path[64];
	struct samba3_share *cs, *csnext;
	struct samba3_shareuser *csu, *csunext;
	struct samba3_user *samba3users, *cu, *cunext;
	struct samba3_share *samba3shares;
	int uniqueuserid = 1000;
	FILE *fp;
	/*#ifdef HAVE_NORTHSTAR
	if (!nvram_matchi("samba3_enable", 1)) {	// not set txworkq 
		set_smp_affinity(163, 2);
		set_smp_affinity(169, 2);
	} else {
		set_smp_affinity(163, 1);
		set_smp_affinity(169, 2);
	}
#endif*/
	if (!nvram_matchi("samba3_enable", 1)) {
		if (nvram_matchi("txworkq", 1)) {
			nvram_unset("txworkq");
			nvram_async_commit();
		}
		stop_samba3();
		return;
	}

	update_timezone();

	if (!nvram_matchi("txworkq", 1)) {
		nvram_seti("txworkq", 1);
		nvram_async_commit();
	}
	start_mkfiles();
	sysprintf("echo \"nobody:*:65534:65534:nobody:/var:/bin/false\" >> /etc/passwd");
	mkdir("/var/samba", 0700);
	mkdir("/var/run", 0700);
	mkdir("/var/run/samba", 0700);
	mkdir("/var/cache", 0700);
	mkdir("/var/cache/samba", 0700);
	eval("touch", "/var/samba/smbpasswd");
	eval("smbpasswd", "nobody", "nobody");
#ifdef HAVE_SMBD
	eval("ksmbd.adduser", "-a", "nobody", "-p", "nobody", "-i", "/tmp/smb.db");
	eval("ksmbd.adduser", "-u", "nobody", "-p", "nobody", "-i", "/tmp/smb.db");
#endif
	if (nvram_matchi("samba3_advanced", 1)) {
		write_nvram("/tmp/smb.conf", "samba3_conf");
	} else {
		samba3users = getsamba3users();
		for (cu = samba3users; cu; cu = cunext) {
			if (*cu->username && cu->sharetype & SHARETYPE_SAMBA) {
				sysprintf("echo \"%s\"\":*:%d:1000:\"%s\":/var:/bin/false\" >> /etc/passwd", cu->username,
					  uniqueuserid++, cu->username);
				eval("smbpasswd", cu->username, cu->password);
#ifdef HAVE_SMBD
				eval("ksmbd.adduser", "-a", cu->username, "-p", cu->password, "-i", "/tmp/smb.db");
				eval("ksmbd.adduser", "-u", cu->username, "-p", cu->password, "-i", "/tmp/smb.db");
#endif
			}
			cunext = cu->next;
			free(cu);
		}
		char *smbmaxproto = nvram_safe_get("samba3_max_proto");
		fp = fopen("/tmp/smb.conf", "wb");
		fprintf(fp,
			"[global]\n" //
			"log level = 1\n" //
			"netbios name = %s\n" //
			"server string = %s\n" //
#ifdef HAVE_SAMBA4
			"unix charset = UTF-8\n"
#endif
			"syslog = 10\n" //
			"encrypt passwords = true\n" //
			"preferred master = yes\n" //
			"use sendfile = yes\n" //
			"aio read size = 2048\n" //
			"aio write size = 2048\n" //
			"large readwrite = yes\n" //
			"security = user\n" //
			"oplocks = no\n" //
			"mangled names = no\n" //
			"max stat cache size = 64\n" //
			"workgroup = %s\n" //
			"bind interfaces only = yes\n" //
			"guest account = nobody\n" //
			"map to guest = %s\n" //
			"smb passwd file = /var/samba/smbpasswd\n" //
			"private dir = /var/samba\n" //
			"passdb backend = smbpasswd\n" //
			"log file = /var/smbd.log\n" //
			"max log size = 1000\n" //
			"socket options = TCP_NODELAY IPTOS_LOWDELAY SO_SNDBUF=262144 SO_RCVBUF=262144\n" //
			"read raw = yes\n" //
			"write raw = yes\n" //
			"max xmit = 65536\n" //
			"dead time = 15\n" //
			"getwd cache = yes\n" //
			"lpq cache time = 30\n" //
#if defined(HAVE_SMBD) || defined(HAVE_SAMBA4)
			"server min protocol = %s\n" //
			"server max protocol = %s\n" //
			"vfs objects = streams_xattr\n"
#else
			"min protocol = %s\n" //
			"max protocol = %s\n" //
#endif
#ifndef HAVE_SAMBA4
			"printing = none\n"
#endif
			"load printers = No\n" //
			"usershare allow guests = Yes\n",
			nvram_safe_get("router_name"), nvram_safe_get("samba3_srvstr"), nvram_safe_get("samba3_workgrp"),
			nvram_safe_get("samba3_guest"), nvram_safe_get("samba3_min_proto"), smbmaxproto);

#ifdef HAVE_SMBD
		if (!strncmp(smbmaxproto, "SMB3", 4))
			fprintf(fp, "smb3 encryption = %s\n", !strcmp(nvram_safe_get("samba3_encrypt"), "off") ? "no" : "yes");
#elif defined(HAVE_SAMBA4)
		if (!strncmp(smbmaxproto, "SMB3", 4))
			fprintf(fp, "smb encrypt = %s\n", nvram_safe_get("samba3_encrypt"));
#endif
		samba3shares = getsamba3shares();
		for (cs = samba3shares; cs; cs = csnext) {
			if (!cs->public) {
				int hasuser = 0;
				for (csu = cs->users; csu; csu = csunext) {
					hasuser = has_sambauser(csu, getsamba3users());
					csunext = csu->next;
				}
				if (!hasuser) {
					free_users(cs);
					goto nextshare;
				}
			}
			if (*cs->label) {
				fprintf(fp, "[%s]\n", cs->label);
				char *sd = cs->sd;
				if (*sd == '/')
					sd++; // kill leading slash if there is any

#ifdef HAVE_SMBD
				fprintf(fp, "comment = %s\n", cs->label);
				fprintf(fp, "path = %s%s%s\n", cs->mp, strlen(sd) ? "/" : "", sd);
#else
				fprintf(fp, "comment = \"%s\"\n", cs->label);
				fprintf(fp, "path = \"%s%s%s\"\n", cs->mp, strlen(sd) ? "/" : "", sd);
#endif
				fprintf(fp, "read only = %s\n", !strcmp(cs->access_perms, "ro") ? "yes" : "no");
				fprintf(fp, "guest ok = %s\n", cs->public == 1 ? "yes" : "no");
				if (!cs->public) {
					fprintf(fp, "valid users =");
					int first = 0;
					for (csu = cs->users; csu; csu = csunext) {
						if (!has_sambauser(csu, getsamba3users()))
							goto nextuser;
						if (first)
							fprintf(fp, ",");
						first = 1;
						fprintf(fp, " %s", csu->username);
nextuser:;
						csunext = csu->next;
						free(csu);
					}
				} else {
					free_users(cs);
				}
				fprintf(fp, "\n");
				fprintf(fp, "force user = root\n");
			} else {
				free_users(cs);
			}
nextshare:;
			csnext = cs->next;
			free(cs);
		}
		fclose(fp);
	}
	chmod("/jffs", 0777);

#ifndef HAVE_SMBD
	char conffile[64];
	if (reload_process("smbd")) {
#ifdef HAVE_SMP
		if (eval("/usr/bin/taskset", "0x2", "/usr/sbin/smbd", "-D", "-s",
			 getdefaultconfig(NULL, path, sizeof(path), "smb.conf")))
#endif
			log_eval("smbd", "-D", "-s", getdefaultconfig(NULL, path, sizeof(path), "smb.conf"));

		if (pidof("smbd") <= 0) {
#ifdef HAVE_SMP
			if (eval("/usr/bin/taskset", "0x2", "/usr/sbin/smbd", "-D", "-s",
				 getdefaultconfig(NULL, path, sizeof(path), "smb.conf")))
#endif
				log_eval("smbd", "-D", "-s", getdefaultconfig(NULL, path, sizeof(path), "smb.conf"));
		}
	}
	if (reload_process("nmbd")) {
		log_eval("nmbd", "-D", "-s", getdefaultconfig(NULL, path, sizeof(path), "smb.conf"));
		if (pidof("nmbd") <= 0) {
			log_eval("nmbd", "-D", "-s", getdefaultconfig(NULL, path, sizeof(path), "smb.conf"));
		}
	}
#ifdef HAVE_SAMBA4
	log_eval("winbindd", "-D", "-s", getdefaultconfig(NULL, path, sizeof(path), "smb.conf"));
#endif
#else
	insmod("oid_registry nls_base nls_utf8 crypto_hash crypto_null aead aead2 sha256_generic sha512_generic geniv seqiv arc4 ecb" //
	       " hmac cmac md4 md5 gf128mul ctr ghash-generic gcm ccm libdes des_generic aes_generic aes-arm" //
	       " aes-arm-ce aes-arm-bs sha256-arm sha512-arm ghash-ce aes-ce-cipher aes-ce-ccm" //
	       " aes-ce-blk aes-neon-blk aes-i586 aes-x86_64 aesni-intel ghash-clmulni-intel sha256-ssse3 sha512-ssse3 sha256-mb sha512-mb libcrc32c asn1_decoder");
	char param[32];
	sprintf(param, "connlimit=%d\n", nvram_default_geti("samba3_connlimit", 16));
	if (nvram_matchi("module_testing", 1))
		eval("insmod", "/jffs/modules_debug/ksmbd.ko", param);
	else
		eval("insmod", "ksmbd", param);
	struct utsname uts;
	/* Uname can fail only if you pass a bad pointer to it. */
	uname(&uts);
	char *nbname = !uts.nodename[0] ? NULL : uts.nodename;
	char *wgname = nvram_safe_get("samba3_workgrp");
	if (*wgname == 0)
		wgname = "WORKGROUP";
	if (*nbname) {
		char parm[128];
		sprintf(parm, "vendor:dd-wrt,model:%s,sku:%s", nvram_safe_get("DD_BOARD"), nvram_safe_get("os_version"));
		stop_process("wsdd2", "windows service discovery daemon");
		log_eval("wsdd2", "-d", "-N", nbname, "-G", wgname, "-b", parm);
	}
	char c1[64];
	char c2[64];
	if (reload_process("ksmbd.mountd")) {
		log_eval("ksmbd.mountd", "-c", getdefaultconfig(NULL, c1, sizeof(c1), "smb.conf"), "-u",
			 getdefaultconfig(NULL, c2, sizeof(c2), "smb.db"));
	}
#endif
	return;
}

void stop_samba3(void)
{
#ifdef HAVE_SMBD
	stop_process("wsdd2", "windows service discovery daemon");
	stop_process("ksmbd.mountd", "samba daemon");
	sysprintf("echo hard > /sys/class/ksmbd-control/kill_server");
	rmmod("ksmbd");
	unlink("/tmp/ksmbd.lock");
#else
	stop_process("smbd", "samba daemon");
	stop_process("nmbd", "daemon");
	//samba has changed the way pidfiles are named, thus stop process will not kill smbd and nmbd pidfiles, see pidfile.c in samba
	sysprintf("rm -rf /var/run/*smb.conf.pid");
#endif
}
#endif

void restart_samba3(void)
{
	start_samba3();
}

#ifdef TEST
void main(int argc, char *argv[])
{
	start_samba3();
}
#endif
