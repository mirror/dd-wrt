/*
 * sshd.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#ifdef HAVE_SSHD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <services.h>

#define ED25519_HOST_KEY_FILE	"/tmp/root/.ssh/ssh_host_ed25519_key"
#define TMP_HOST_KEY_FILE	"/tmp/tmp_host_key"
#define AUTHORIZED_KEYS_FILE	"/tmp/root/.ssh/authorized_keys"
#define NVRAM_ED25519_KEY_NAME      "sshd_ed25519_host_key"

static void empty_dir_check(void);
static int write_key_file(char *keyname, char *keyfile, int chmodval);
static int generate_dropbear_ed25519_host_key(void);
void stop_sshd(void);

char *sshd_deps(void)
{

	return "sshd_enable http_username http_passwd " NVRAM_ED25519_KEY_NAME " sshd_authorized_keys sshd_port sshd_passwd_auth sshd_forwarding";
}

char *sshd_proc(void)
{
	return "dropbear";
}

void start_sshd(void)
{
	nvram_unset("sshd_rsa_host_key");
	stop_sshd();
	if (!nvram_invmatchi("sshd_enable", 0))
		return;
	if (nvram_match("http_username", DEFAULT_USER) && nvram_match("http_passwd", DEFAULT_PASS))
		return;
	empty_dir_check();
	int changed = 0;
	//egc
	if (!nvram_matchi("sshd_keyready", 1)) { //if private key has been downloaded delete
		unlink("/tmp/id_ed25519_ssh");
	}

	if (write_key_file(NVRAM_ED25519_KEY_NAME, ED25519_HOST_KEY_FILE, 0600) == -1) {
		generate_dropbear_ed25519_host_key();
		write_key_file(NVRAM_ED25519_KEY_NAME, ED25519_HOST_KEY_FILE, 0600);
		changed = 1;
	}
	eval("dropbearconvert", "openssh", "dropbear", ED25519_HOST_KEY_FILE, ED25519_HOST_KEY_FILE);
	nvram_unset("sshd_dss_host_key");
	if (changed)
		nvram_commit();
	write_key_file("sshd_authorized_keys", AUTHORIZED_KEYS_FILE, 0600);
	stop_sshd();
	int a = 0;
	char *sshd_argv[12];
	sshd_argv[a++] = "dropbear";
#ifndef HAVE_MAKSAT
	sshd_argv[a++] = "-b";
	sshd_argv[a++] = "/tmp/loginprompt";
#endif
	sshd_argv[a++] = "-r";
	sshd_argv[a++] = ED25519_HOST_KEY_FILE;
	sshd_argv[a++] = "-p";
	sshd_argv[a++] = nvram_safe_get("sshd_port");
	sshd_argv[a++] = "-W";
	sshd_argv[a++] = nvram_safe_get("sshd_rw");
	if (!nvram_matchi("sshd_passwd_auth", 1))
		sshd_argv[a++] = "-s";
	if (nvram_matchi("sshd_forwarding", 1))
		sshd_argv[a++] = "-a";
	sshd_argv[a++] = NULL;
	_log_evalpid(sshd_argv, NULL, 0, NULL);
	return;
}

void stop_sshd(void)
{
	stop_process("dropbear", "ssh daemon");
	nvram_delstates(sshd_deps());
	cprintf("done\n");

	return;
}

static void empty_dir_check(void)
{
	struct stat buf;

	if (stat("/tmp/root/.ssh", &buf) != 0)
		mkdir("/tmp/root/.ssh", 0700);

}

/**
 * Write the sshd key file making sure that it contains no LF 0x0D chars
 * and is terminated by a single CR 0x0A char
 *
 * @return zero on sucess, non-zero on error
 */
static int write_key_file(char *keyname, char *keyfile, int chmodval)
{
	FILE *fd = NULL;
	char *host_key = NULL;
	int i = 0;

	if (!keyname || !keyfile)
		return -1;
	host_key = nvram_safe_get(keyname);
	if (!*host_key)
		return -1;

	/*
	 * Update the named key file 
	 */
	if ((fd = fopen(keyfile, "wb")) == NULL) {
		cprintf("Can't open %s\n", keyfile);
		return -1;
	}
	fwritenvram(keyname, fd);
	// add CR at end
	if (host_key[i - 1] != 0x0A)
		fprintf(fd, "%c", 0x0A);

	fclose(fd);

	// set perms
	chmod(keyfile, chmodval);

	return 0;
}

static int generate_dropbear_ed25519_host_key(void)
{
	FILE *fp = (void *)0;
	char *buf = malloc(4096);
	int ret = -1;

	eval("dropbearkey", "-t", "ed25519", "-f", ED25519_HOST_KEY_FILE);

	eval("dropbearconvert", "dropbear", "openssh", ED25519_HOST_KEY_FILE, TMP_HOST_KEY_FILE);

	fp = fopen(TMP_HOST_KEY_FILE, "r");

	if (fp == (void *)0) {
		free(buf);
		return -1;
	}

	ret = fread(buf, 1, 4095, fp);

	if (ret <= 0) {
		fclose(fp);
		free(buf);
		return -1;
	}

	buf[ret] = 0;		// terminate by 0. buf isnt initialized
	nvram_set(NVRAM_ED25519_KEY_NAME, buf);
	free(buf);

	fclose(fp);

	unlink(TMP_HOST_KEY_FILE);

	return ret;
}
#endif				/* HAVE_SSHD */
